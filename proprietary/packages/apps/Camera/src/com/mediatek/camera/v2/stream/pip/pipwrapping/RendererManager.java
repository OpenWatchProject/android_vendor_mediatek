/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.v2.stream.pip.pipwrapping;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import junit.framework.Assert;

import java.util.HashMap;
import java.util.Map;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;


/**
 * Managers SurfaceTextures, creates SurfaceTexture and TexureRender Objects,
 * and do SW Sync that ensure two SurfaceTextures are sync.
 */
public class RendererManager {
    private static final Tag TAG = new Tag(RendererManager.class.getSimpleName());
    private static final int         EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    private static final String      BOTTOM = "pip_bottom";
    private static final String      TOP    = "pip_top";
    private final Activity           mActivity;
    private final RendererCallback   mRendererCallback;

    private EGLConfig                mEglConfig;
    private EGLDisplay               mEglDisplay;
    private EGLContext               mEglContext;
    private EGLSurface               mEglSurface;
    private EGL10                    mEgl;

    private Object                   mRenderLock = new Object();
    private ConditionVariable        mEglThreadBlockVar = new ConditionVariable();
    private HandlerThread            mEglThread;
    private EglHandler               mEglHandler;
    private HandlerThread            mStFrameListener = new HandlerThread("PIP-StFrameListener");
    private Handler                  mSurfaceTextureHandler;

    private FrameBuffer              mPreviewFrameBuffer;
    private BottomGraphicRenderer    mBottomGraphicRenderer;
    private TopGraphicRenderer       mTopGraphicRenderer;
    private ScreenRenderer           mScreenRenderer; // for PV
    private CaptureRenderer          mCaptureRenderer; // for CAP/VSS
    private RecorderRenderer         mRecorderRenderer; // for VR
    // bottom/top use the same preview texture size
    private int                      mRendererTexWidth = -1;
    private int                      mRendererTexHeight = -1;

    private SurfaceTextureWrapper    mMainPv_StWrapper; // for PV/VR
    private SurfaceTextureWrapper    mSubPv_StWrapper; // for PV/VR
    private boolean                  isBottomHasHighFrameRate = true;

    private SurfaceTextureWrapper    mMainCap_StWrapper; // for CAP/VSS
    private SurfaceTextureWrapper    mSubCap_StWrapper; // for PV
    private Surface                  mCaptureSurface; // for capture and video snap shot
    private int                      mCurrentOrientation; // this orientation is for g-sensor
    private FrameBuffer              mPictureFb;
    private AnimationRect            mPreviewTopGraphicRect = null;

    // effect resource id
    private int                      mBackTempResId = 0;
    private int                      mFrontTempResId = 0;
    private int                      mHighlightTempResId = 0;
    private int                      mEditResId = 0;

    // switch pip false: main camera in bottom graphic and sub camera in top graphic
    private boolean                  mPipSwitched = false;

    private boolean                  mBlockingForPvSizeChange = false;
    private boolean                  mNeedNotifyFirstFrameForSurfaceChanged = true;
    public interface RendererCallback {
        public void onFristFrameAvailable(long timestamp);
    }

    public RendererManager(Activity activity, RendererCallback callback) {
        mActivity = activity;
        mRendererCallback = callback;
    }

    /**
     * new a handler thread and create EGL Context in it.
     * We called this thread to "PIP GL Thread".
     * <p>
     * Note: if "PIP GL Thread" exits, skip initialization
     */
    public synchronized void init() {
        LogHelper.d(TAG, "[init]+ mEglHandler = " + mEglHandler);
        if (mEglHandler != null) {
            LogHelper.i(TAG, "[init]- ");
            return;
        }
        initializePreviewRendererThread();
        mEglHandler.sendMessageSync(EglHandler.MSG_CREATE_RENDERERS);

        LogHelper.d(TAG, "[init]- mEglHandler = " + mEglHandler);
    }

    /**
     * release surface textures, related renderers and "PIP GL Thread"
     */
    public synchronized void unInit() {
        LogHelper.d(TAG, "[unInit]+ mEglHandler = " + mEglHandler);
        if (mEglHandler != null) {
            // remove all previous messages and resume mEglThreadBlockVar
            mEglHandler.removeCallbacksAndMessages(null);
            mEglThreadBlockVar.open();
            mEglHandler.sendMessageSync(EglHandler.MSG_RELEASE);
            // release thread
            Looper looper = mEglThread.getLooper();
            if (looper != null) {
                looper.quit();
            }
            mSurfaceTextureHandler = null;
            mEglHandler = null;
            mEglThread = null;
        }
        LogHelper.d(TAG, "[unInit]-");
    }

    /**
     * update pip template resource.
     * Note: if resource id is the same with previous, call this function has no use.
     * @param backResourceId bottom graphic template
     * @param frontResourceId top graphic template
     * @param effectFrontHighlightId top graphic highlight template
     * @param editButtonResourceId top graphic edit template
     */
    public void updateEffectTemplates(int backResourceId, int frontResourceId,
            int effectFrontHighlightId, int editButtonResourceId) {
        LogHelper.d(TAG, "[updateEffectTemplates]+");
        if ((mBackTempResId) == backResourceId && (mFrontTempResId == frontResourceId)
                && (mHighlightTempResId == effectFrontHighlightId)) {
            LogHelper.w(TAG, "[updateEffectTemplates]- no need to update effect");
            return;
        }
        // when switch pip template quickly, skip pending update template
        // and do not block ui thread.
        if (mEglHandler != null) {
            mBackTempResId = backResourceId;
            mFrontTempResId = frontResourceId;
            mHighlightTempResId = effectFrontHighlightId;
            mEditResId = editButtonResourceId;
            mEglHandler.removeMessages(EglHandler.MSG_UPDATE_TEMPLATE);
            mEglHandler.obtainMessage(EglHandler.MSG_UPDATE_TEMPLATE).sendToTarget();
        }
        LogHelper.d(TAG, "[updateEffectTemplates]-");
    }

    /**
     * update top graphic's position
     * @param topGraphic
     */
    public void updateTopGraphic(AnimationRect topGraphic) {
        synchronized (mRenderLock) {
            mPreviewTopGraphicRect = topGraphic;
        }
    }

    /**
     * when G-sensor's orientation changed, should update it to PIPOperator
     * @param newOrientation G-sensor's new orientation
     */
    public void updateGSensorOrientation(int newOrientation) {
        mCurrentOrientation = newOrientation;
    }

    public void switchPIP() {
        LogHelper.d(TAG, "switchPIP , mPipSwitched:" + mPipSwitched);
        mPipSwitched = !mPipSwitched;
    }

    /**
     * Set pip preview texture's size
     * <p>
     * Note: pip bottom and top texture's size must be the same for switch pip
     * @param width bottom/top texture's width
     * @param height bottom/top texture's height
     */
    public void setPreviewSize(Size previewSize) {
        Assert.assertNotNull(previewSize);
        LogHelper.d(TAG, "[setPreviewSize]+ width = " + previewSize.getWidth()
                + " height = " + previewSize.getHeight());
//        if (mRendererTexWidth == previewSize.getWidth() && mRendererTexHeight
//                == previewSize.getHeight()) {
//            LogHelper.w(TAG, "[setPreviewTextureSize]- the same size set, ignore!");
//            return;
//        }
        if (mEglHandler != null) {
            mBlockingForPvSizeChange = true;
            mEglHandler.sendMessageSync(EglHandler.MSG_UPDATE_RENDERER_SIZE, previewSize);
        }
        LogHelper.d(TAG, "[setPreviewSize]-");
    }

    /**
     * Set preview surface to receive pip preview buffer
     * Note: this must be called after setPreviewTextureSize
     * @param surface
     */
    public void setPreviewSurface(Surface surface) {
        LogHelper.d(TAG, "[setPreviewSurface]+");
        if (mEglHandler != null && surface != null) {
            mEglHandler.sendMessageSync(EglHandler.MSG_SET_PREVIEW_SURFACE, surface);
        }
        LogHelper.d(TAG, "[setPreviewSurface]-");
    }


    /**
     * Notify activity pause.
     * <p>
     * Note: this for notify Surface destroy when pip preview
     */
    public void onActivityPause() {
        if (mEglHandler != null) {
            mEglHandler.sendMessageSync(EglHandler.MSG_NOTIFY_ACTIVITY_PAUSE);
        }
    }

    /**
     * Notify activity resume.
     */
    public void onActivityResume() {
        mNeedNotifyFirstFrameForSurfaceChanged = true;
    }

    /**
     * Get bottom surface texture
     * @return bottom graphic surface texture
     */
    public SurfaceTexture getMainCamPvSt() {
        LogHelper.d(TAG, "getMainCamPvSt");
        return _getMainPvSurfaceTexture();
    }

    /**
     * Get top surface texture
     * @return top graphic surface texture
     */
    public SurfaceTexture getSubCamPvSt() {
        LogHelper.d(TAG, "getSubCamPvSt");
        return _getSubPvSurfaceTexture();
    }


    /**
     * Set recording surface to receive pip buffer
     * <p>
     * Note: this must be called after setPreviewTextureSize
     * @param surface the surface got from encoder, who will receive pip buffer for VR.
     */
    public void setRecordingSurface(Surface surface) {
        LogHelper.d(TAG, "setRecordingSurface surfacee:" + surface);
        if (surface != null && mEglHandler != null) {
            mEglHandler.sendMessageSync(EglHandler.MSG_SET_RECORDING_SURFACE, surface);
        }
    }

    public void prepareRecording() {
        LogHelper.d(TAG, "[prepareRecording]+ mRecorderRenderer:" +
                mRecorderRenderer +
                " mEglHandler:" + mEglHandler);
        if (mRecorderRenderer == null && mEglHandler != null) {
            mEglHandler.removeMessages(EglHandler.MSG_SETUP_VIDEO_RENDER);
            mEglHandler.sendMessageSync(EglHandler.MSG_SETUP_VIDEO_RENDER);
            synchronized (mRenderLock) {
                if (mRecorderRenderer == null) {
                    try {
                        mRenderLock.wait();
                    } catch (InterruptedException e) {
                        LogHelper.e(TAG, "unexpected interruption");
                    }
                }
            }
        }
        LogHelper.d(TAG, "[prepareRecording]-");
    }

    public void startRecording() {
        LogHelper.d(TAG, "[startRecording]+ mRecorderRenderer:" + mRecorderRenderer);
        if (mRecorderRenderer != null) {
            mRecorderRenderer.startRecording();
        }
        LogHelper.d(TAG, "[startRecording]-");
    }

    public void stopRecording() {
        LogHelper.d(TAG, "[stopRecording]+ mRecorderRenderer:" + mRecorderRenderer +
                " mEglHandler:" + mEglHandler);
        if (mRecorderRenderer != null && mEglHandler != null) {
            mRecorderRenderer.stopRecording();
            mEglHandler.removeMessages(EglHandler.MSG_RELEASE_VIDEO_RENDER);
            mEglHandler.sendMessageSync(EglHandler.MSG_RELEASE_VIDEO_RENDER);
        }
        LogHelper.d(TAG, "[stopRecording]-");
    }

    public void setPictureSize(Size bottomCaptureSize, Size topCaptureSize) {
        LogHelper.d(TAG, "[setPictureSize]+ bottomCaptureSize:" + bottomCaptureSize +
                " topCaptureSize:" + topCaptureSize);
        Assert.assertNotNull(bottomCaptureSize);
        Assert.assertNotNull(topCaptureSize);
        if (mEglHandler != null) {
            Map<String, Size> pictureSizeMap = new HashMap<String, Size>();
            pictureSizeMap.put(BOTTOM, bottomCaptureSize);
            pictureSizeMap.put(TOP, topCaptureSize);
            mEglHandler.sendMessageSync(EglHandler.MSG_SETUP_PICTURE_TEXTURES, pictureSizeMap);
        }
        LogHelper.d(TAG, "[setPictureSize]-");
    }

    /**
     * Set picture surface ,this must be set before take picture
     * @param surface a surface used to receive pip picture buffer
     */
    public void setCaptureOutputSurface(Surface surface) {
        LogHelper.d(TAG, "setCaptureOutputSurface surface:" + surface);
        mCaptureSurface = surface;
    }

    public SurfaceTexture getMainCamCapSt() {
        LogHelper.d(TAG, "getMainCamCapSt");
        if (mMainCap_StWrapper == null) {
            throw new IllegalStateException("please call setPictureSize firstly!");
        }
        return mMainCap_StWrapper.getSurfaceTexture();
    }

    public SurfaceTexture getSubCamCapSt() {
        LogHelper.d(TAG, "getSubCamCapSt");
        if (mSubCap_StWrapper == null) {
            throw new IllegalStateException("please call setPictureSize firstly!");
        }
        return mSubCap_StWrapper.getSurfaceTexture();
    }

    private void initializePreviewRendererThread() {
        synchronized (mRenderLock) {
            mEglThread = new HandlerThread("PIP-PreviewRealtimeRenderer");
            mEglThread.start();
            Looper looper = mEglThread.getLooper();
            if (looper == null) {
                throw new RuntimeException("why looper is null?");
            }
            mEglHandler = new EglHandler(looper);
            initialize();
        }
    }

    private SurfaceTexture _getMainPvSurfaceTexture() {
        synchronized (mRenderLock) {
            if (mEglHandler == null) {
                LogHelper.w(TAG, "call _getSubPvSurfaceTexture after init/un-init");
                return null;
            }
            if (mMainPv_StWrapper == null) {
                try {
                    mRenderLock.wait();
                } catch (InterruptedException e) {
                    LogHelper.e(TAG, "unexpected interruption");
                }
            }
            LogHelper.d(TAG, "_getMainPvSurfaceTexture mPreviewBottomSurfaceTexture = "
                    + mMainPv_StWrapper.getSurfaceTexture()
                    + " mEglHandler = " + mEglHandler);
            return mMainPv_StWrapper.getSurfaceTexture();
        }
    }

    private SurfaceTexture _getSubPvSurfaceTexture() {
        synchronized (mRenderLock) {
            if (mEglHandler == null) {
                LogHelper.w(TAG, "call _getSubPvSurfaceTexture after init/un-init");
                return null;
            }
            if (mSubPv_StWrapper == null) {
                try {
                    mRenderLock.wait();
                } catch (InterruptedException e) {
                    LogHelper.e(TAG, "unexpected interruption");
                }
            }
            mBlockingForPvSizeChange = false;
            LogHelper.d(TAG, "_getSubPvSurfaceTexture mPreviewTopSurfaceTexture = "
                    + mSubPv_StWrapper.getSurfaceTexture()
                    + " mEglHandler = " + mEglHandler);
            return mSubPv_StWrapper.getSurfaceTexture();
        }
    }

    private class EglHandler extends Handler {
        public static final int MSG_NEW_PREVIEW_FRAME_ARRIVED = 17;
        public static final int MSG_SETUP_VIDEO_RENDER = 5;
        public static final int MSG_RELEASE_VIDEO_RENDER = 6;
        public static final int MSG_UPDATE_TEMPLATE = 7;
        public static final int MSG_RELEASE_PIP_TEXTURES = 9;
        public static final int MSG_RELEASE = 10;
        public static final int MSG_CREATE_RENDERERS = 11;
        public static final int MSG_UPDATE_RENDERER_SIZE = 12;
        public static final int MSG_SET_PREVIEW_SURFACE = 13;
        public static final int MSG_SET_RECORDING_SURFACE = 14;
        public static final int MSG_SETUP_PICTURE_TEXTURES = 15;
        public static final int MSG_NEW_PICTURE_FRAME_ARRIVED = 16;
        public static final int MSG_NOTIFY_ACTIVITY_PAUSE = 18;
        private long mBottomCamTimeStamp = 0L;
        private long mTopCamTimeStamp = 0L;

        private SurfaceTexture.OnFrameAvailableListener mBottomCamFrameAvailableListener
            = new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                if (mMainCap_StWrapper != null
                        && surfaceTexture == mMainCap_StWrapper.getSurfaceTexture()) {
                    LogHelper.d(TAG, "TakePicture: Main Camera onFrameAvailable ");
                    mEglHandler.obtainMessage(EglHandler.MSG_NEW_PICTURE_FRAME_ARRIVED,
                            mMainCap_StWrapper).sendToTarget();
                } else if (surfaceTexture == mMainPv_StWrapper.getSurfaceTexture()) {
                    mEglHandler.obtainMessage(EglHandler.MSG_NEW_PREVIEW_FRAME_ARRIVED,
                            mMainPv_StWrapper).sendToTarget();
                }
            }
        };

        private SurfaceTexture.OnFrameAvailableListener mTopCamFrameAvailableListener
            = new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                if (mSubCap_StWrapper != null
                        && surfaceTexture == mSubCap_StWrapper.getSurfaceTexture()) {
                    LogHelper.d(TAG, "TakePicture: Sub Camera onFrameAvailable ");
                    mEglHandler.obtainMessage(EglHandler.MSG_NEW_PICTURE_FRAME_ARRIVED,
                            mSubCap_StWrapper).sendToTarget();
                } else if (surfaceTexture == mSubPv_StWrapper.getSurfaceTexture()) {
                    mEglHandler.obtainMessage(EglHandler.MSG_NEW_PREVIEW_FRAME_ARRIVED,
                            mSubPv_StWrapper).sendToTarget();
                }
            }
        };

        public EglHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_CREATE_RENDERERS:
                createRenderers();
                mEglThreadBlockVar.open();
                break;
            case MSG_UPDATE_RENDERER_SIZE:
                Size rendererSize = (Size) msg.obj;
                mRendererTexWidth = rendererSize.getWidth();
                mRendererTexHeight = rendererSize.getHeight();
                doUpdateRenderSize();
                mEglThreadBlockVar.open();
                break;
            case MSG_SETUP_PICTURE_TEXTURES:
                @SuppressWarnings("unchecked")
                Map<String, Size> pictureSizeMap = (HashMap<String, Size>) msg.obj;
                doSetupPictureTextures(pictureSizeMap.get(BOTTOM), pictureSizeMap.get(TOP));
                mEglThreadBlockVar.open();
                break;
            case MSG_SET_PREVIEW_SURFACE:
                doUpdatePreviewSurface((Surface) msg.obj);
                mEglThreadBlockVar.open();
                break;
            case MSG_SET_RECORDING_SURFACE:
                doUpdateRecordingSurface((Surface) msg.obj);
                mEglThreadBlockVar.open();
                break;
            case MSG_NEW_PREVIEW_FRAME_ARRIVED:
                SurfaceTextureWrapper stPreviewWrapper = (SurfaceTextureWrapper) msg.obj;
                if (stPreviewWrapper == null) {
                    return;
                }
                stPreviewWrapper.updateTexImage();
                draw();
                break;
            case MSG_NEW_PICTURE_FRAME_ARRIVED:
                SurfaceTextureWrapper stPicWrapper = (SurfaceTextureWrapper) msg.obj;
                if (stPicWrapper == null) {
                    return;
                }
                stPicWrapper.updateTexImage();
                tryTakePicutre();
                break;
            case MSG_SETUP_VIDEO_RENDER:
                doSetUpRenderForRecord();
                mEglThreadBlockVar.open();
                break;
            case MSG_RELEASE_VIDEO_RENDER:
                doReleaseRenderForRecord();
                mEglThreadBlockVar.open();
                break;
            case MSG_UPDATE_TEMPLATE:
                doUpdateTemplate();
                break;
            case MSG_RELEASE_PIP_TEXTURES:
                releaseRenderers();
                mEglThreadBlockVar.open();
                break;
            case MSG_RELEASE:
                doRelease();
                mEglThreadBlockVar.open();
                break;
            case MSG_NOTIFY_ACTIVITY_PAUSE:
                if (mScreenRenderer != null) {
                    mScreenRenderer.onActivityPause();
                }
                mEglThreadBlockVar.open();
                break;
            }
        }

        private void createRenderers() {
            LogHelper.d(TAG, "[createRenderers]+ mSurfaceTextureHandler = "
                    + mSurfaceTextureHandler);
            synchronized (mRenderLock) {
                if (mSurfaceTextureHandler == null) {
                    // frame availabe thread
                    if (!mStFrameListener.isAlive()) {
                        mStFrameListener.start();
                    }
                    mSurfaceTextureHandler = new Handler(mStFrameListener.getLooper());
                    // preview fbo
                    mPreviewFrameBuffer = new FrameBuffer();
                    mPreviewFrameBuffer.init();
                    // bottom graphic renderer
                    mBottomGraphicRenderer = new BottomGraphicRenderer(mActivity);
                    // top graphic renderer
                    mTopGraphicRenderer = new TopGraphicRenderer(mActivity);
                    // screen preview renderer
                    mScreenRenderer = new ScreenRenderer(mActivity);
                    mScreenRenderer.init();
                    // capture renderer
                    mCaptureRenderer = new CaptureRenderer(mActivity);
                    mCaptureRenderer.init();
                    // update template for pause -> resume case
                    doUpdateTemplate();
                }
                isBottomHasHighFrameRate = true; // Util.isBottomHasHighFrameRate(mContext);
            }
            LogHelper.d(TAG, "[createRenderers]-");
        }

        private void doUpdateRenderSize() {
            LogHelper.d(TAG, "[doUpdateRenderSize]+ mPreviewTexWidth = " +
                    mRendererTexWidth + " mPreviewTexHeight = " + mRendererTexHeight
                    + " mPreviewFrameBuffer = " + mPreviewFrameBuffer);
            synchronized (mRenderLock) {
                // initialize bottom surface texture wrapper
                // When change buffer size, we may receive previous size's buffer,
                // There are two ways to prevent this occur:
                // One way: be sure SurfaceTexture's producer is disconnected.
                // (ex. two camera's session has already aborted)
                // The second way: make a new SurfaceTexture for new size.
                // In order to make a little changes, here we choose the second way.
                if (mMainPv_StWrapper != null) {
                    mMainPv_StWrapper.release();
                    mMainPv_StWrapper = null;
                }
                mMainPv_StWrapper = new SurfaceTextureWrapper();
                mMainPv_StWrapper.setDefaultBufferSize(mRendererTexWidth, mRendererTexHeight);
                mMainPv_StWrapper.setOnFrameAvailableListener(mBottomCamFrameAvailableListener,
                        mSurfaceTextureHandler);
                // initialize top surface texture
                if (mSubPv_StWrapper != null) {
                    mSubPv_StWrapper.release();
                    mSubPv_StWrapper = null;
                }
                mSubPv_StWrapper = new SurfaceTextureWrapper();
                mSubPv_StWrapper.setDefaultBufferSize(mRendererTexWidth, mRendererTexHeight);
                mSubPv_StWrapper.setOnFrameAvailableListener(mTopCamFrameAvailableListener,
                        mSurfaceTextureHandler);

                if (mPreviewFrameBuffer != null) {
                    mPreviewFrameBuffer.setRendererSize(mRendererTexWidth, mRendererTexHeight);
                }
                if (mBottomGraphicRenderer != null) {
                    mBottomGraphicRenderer.setRendererSize(mRendererTexWidth,
                            mRendererTexHeight, false);
                }
                if (mTopGraphicRenderer != null) {
                    mTopGraphicRenderer.setRendererSize(
                            mRendererTexWidth, mRendererTexHeight, true);
                }
                if (mScreenRenderer != null) {
                    mScreenRenderer.setRendererSize(mRendererTexWidth, mRendererTexHeight);
                }
                mRenderLock.notifyAll();
            }
            LogHelper.d(TAG, "[doUpdateRenderSize]-");
        }

        private void doUpdatePreviewSurface(Surface surface) {
            if (mScreenRenderer != null) {
                mScreenRenderer.setSurface(surface);
                mNeedNotifyFirstFrameForSurfaceChanged = true;
            }
        }

        private void releaseRenderers() {
            LogHelper.d(TAG, "[releaseRenderers]+");
            synchronized (mRenderLock) {
                // release frame buffer
                if (mPreviewFrameBuffer != null) {
                    mPreviewFrameBuffer.unInit();
                    mPreviewFrameBuffer = null;
                }
                if (mPictureFb != null) {
                    mPictureFb.unInit();
                    mPictureFb = null;
                }
                // release renderer
                if (mBottomGraphicRenderer != null) {
                    mBottomGraphicRenderer.release();
                    mBottomGraphicRenderer = null;
                }
                if (mTopGraphicRenderer != null) {
                    mTopGraphicRenderer.release();
                    mTopGraphicRenderer = null;
                }
                if (mCaptureRenderer != null) {
                    mCaptureRenderer.release();
                    mCaptureRenderer = null;
                }
                if (mRecorderRenderer != null) {
                    mRecorderRenderer.releaseSurface();
                    mRecorderRenderer = null;
                }
                if (mScreenRenderer != null) {
                    mScreenRenderer.release();
                    mScreenRenderer = null;
                }
                mRenderLock.notifyAll();
            }
            LogHelper.d(TAG, "[releaseRenderers]-");
        }

        private void doSetupPictureTextures(Size bPictureSize, Size tPictureSize) {
            LogHelper.d(TAG, "doSetupPictureTextures");
            // initialize capture frame buffer
            if (mPictureFb == null) {
                mPictureFb = new FrameBuffer();
                mPictureFb.init();
            }
            // initialize / update bottom Surface Texture
            if (mMainCap_StWrapper == null) {
                mMainCap_StWrapper = new SurfaceTextureWrapper();
                mMainCap_StWrapper.setDefaultBufferSize(bPictureSize.getWidth(),
                        bPictureSize.getHeight());
                mMainCap_StWrapper.setOnFrameAvailableListener(mBottomCamFrameAvailableListener,
                        mSurfaceTextureHandler);
            }
            // initialize / update top Surface Texture
            mMainCap_StWrapper.setDefaultBufferSize(bPictureSize.getWidth(),
                    bPictureSize.getHeight());
            if (mSubCap_StWrapper == null) {
                mSubCap_StWrapper = new SurfaceTextureWrapper();
                mSubCap_StWrapper.setDefaultBufferSize(tPictureSize.getWidth(),
                        tPictureSize.getHeight());
                mSubCap_StWrapper.setOnFrameAvailableListener(mTopCamFrameAvailableListener,
                        mSurfaceTextureHandler);
            }
            mSubCap_StWrapper.setDefaultBufferSize(tPictureSize.getWidth(),
                    tPictureSize.getHeight());
        }

        private void releaseTextures() {
            LogHelper.d(TAG, "[releaseTextures]+");
            if (mSubPv_StWrapper != null) {
                mSubPv_StWrapper.release();
                mSubPv_StWrapper = null;
            }
            if (mMainPv_StWrapper != null) {
                mMainPv_StWrapper.release();
                mMainPv_StWrapper = null;
            }
            mRendererTexWidth = -1;
            mRendererTexHeight = -1;
            // delete picture textures
            if (mMainCap_StWrapper != null) {
                mMainCap_StWrapper.release();
                mMainCap_StWrapper = null;
            }
            if (mSubCap_StWrapper != null) {
                mSubCap_StWrapper.release();
                mSubCap_StWrapper = null;
            }
            LogHelper.d(TAG, "[releaseTextures]-");
        }

        private void doSetUpRenderForRecord() {
            synchronized (mRenderLock) {
                mRecorderRenderer = new RecorderRenderer(mActivity);
                mRecorderRenderer.init();
                mRenderLock.notifyAll();
            }
        }

        private void doUpdateRecordingSurface(Surface recordingSurface) {
            synchronized (mRenderLock) {
                if (mRecorderRenderer == null) {
                    throw new IllegalStateException("Before update record surface, " +
                            "please call prepareRecording firstly!");
                }
                mRecorderRenderer.setRecrodingSurface(recordingSurface, !mPipSwitched);
                mRenderLock.notifyAll();
            }
        }

        private void doReleaseRenderForRecord() {
            if (mRecorderRenderer != null) {
                mRecorderRenderer.releaseSurface();
                mRecorderRenderer = null;
            }
        }

        private void doUpdateTemplate() {
            if (mTopGraphicRenderer != null && mBackTempResId > 0 && mFrontTempResId > 0) {
                mTopGraphicRenderer.initTemplateTexture(mBackTempResId, mFrontTempResId);
            }
            if (mScreenRenderer != null && mHighlightTempResId > 0 && mEditResId > 0) {
                mScreenRenderer.updateScreenEffectTemplate(mHighlightTempResId, mEditResId);
            }
        }

        private void doRelease() {
            LogHelper.d(TAG, "[doRelease]+");
            // release surface textures
            releaseTextures();
            // release renderer
            releaseRenderers();
            // release EGL context
            releaseEgl();
            LogHelper.d(TAG, "[doRelease]-");
        }

        private boolean doTimestampSync() {
            if (mMainPv_StWrapper == null || mSubPv_StWrapper == null) {
                return false;
            }
            // pip mode, sync first frame
            if (mMainPv_StWrapper.getBufferTimeStamp() == 0L ||
                    mSubPv_StWrapper.getBufferTimeStamp() == 0L) {
                return false;
            }
            // after sync first frame, when high fps's surface
            // texture new frame arrives, draw the frame
            // bottom is the high frame
            if (isBottomHasHighFrameRate && (mBottomCamTimeStamp !=
                    mMainPv_StWrapper.getBufferTimeStamp())) {
                mBottomCamTimeStamp = mMainPv_StWrapper.getBufferTimeStamp();
                mTopCamTimeStamp = mSubPv_StWrapper.getBufferTimeStamp();
                return true;
            }
            // top is the high frame
            if (!isBottomHasHighFrameRate && (mTopCamTimeStamp !=
                    mSubPv_StWrapper.getBufferTimeStamp())) {
                mBottomCamTimeStamp = mMainPv_StWrapper.getBufferTimeStamp();
                mTopCamTimeStamp = mSubPv_StWrapper.getBufferTimeStamp();
                return true;
            }
            return false;
        }

        private void draw() {
            synchronized (mRenderLock) {
                if (doTimestampSync() && !mBlockingForPvSizeChange) {
                    // wrapping pip texture
                    mPreviewFrameBuffer.setupFrameBufferGraphics(
                            mRendererTexWidth, mRendererTexHeight);
                    SurfaceTextureWrapper sub_StWrapper = mSubPv_StWrapper;
                    SurfaceTextureWrapper main_StWrapper = mMainPv_StWrapper;
                    boolean pipSwitched = mPipSwitched;
                    if (pipSwitched) {
                        sub_StWrapper = mMainPv_StWrapper;
                        main_StWrapper = mSubPv_StWrapper;
                    }
                    mBottomGraphicRenderer.draw(
                        main_StWrapper.getTextureId(), /**input texture id**/
                        main_StWrapper.getBufferTransformMatrix(), /**texture transform matrix**/
                        GLUtil.createIdentityMtx(), /**texture rotate transform matrix**/
                        false  /**whether need flip**/
                        );
                    mTopGraphicRenderer.draw(
                        sub_StWrapper.getTextureId(), /**input texture id**/
                        sub_StWrapper.getBufferTransformMatrix(), /**texture transform matrix**/
                        GLUtil.createIdentityMtx(), /**texture rotate transform matrix**/
                        mPreviewTopGraphicRect.copy(), /**texture vertex rect**/
                        mCurrentOrientation, /**rotation**/
                        false /**whether need flip**/
                        );
                    mPreviewFrameBuffer.setScreenBufferGraphics();
                    // ensure time stamp will not backup, always
                    // choose one camera's buffer time stamp
                    long effectBufferTimestamp =
                            pipSwitched ? sub_StWrapper.getBufferTimeStamp()
                                    : main_StWrapper.getBufferTimeStamp();
                    // draw to encoder
                    if (mRecorderRenderer != null) {
                        mRecorderRenderer.draw(
                                mPreviewFrameBuffer.getFboTexId(),
                                effectBufferTimestamp);
                    }
                    // draw to screen
                    if (mNeedNotifyFirstFrameForSurfaceChanged) {
                        mRendererCallback.onFristFrameAvailable(
                                main_StWrapper.getBufferTimeStamp());
                        mNeedNotifyFirstFrameForSurfaceChanged = false;
                    }
                    mScreenRenderer.draw(
                            mPreviewTopGraphicRect.copy(),
                            mPreviewFrameBuffer.getFboTexId(),
                            mPreviewTopGraphicRect.getHighLightStatus());
                }
            }
        }

        private void tryTakePicutre() {
            if (mMainCap_StWrapper != null && mMainCap_StWrapper.getBufferTimeStamp() > 0
                    && mSubCap_StWrapper != null && mSubCap_StWrapper.getBufferTimeStamp() > 0) {
                boolean isPipSwitched = mPipSwitched;
                SurfaceTextureWrapper topStWrapper = mSubCap_StWrapper;
                SurfaceTextureWrapper bottomStWrapper = mMainCap_StWrapper;
                if (isPipSwitched) {
                    topStWrapper = mMainCap_StWrapper;
                    bottomStWrapper = mSubCap_StWrapper;
                }
                // set to picture render size
                mPictureFb.setRendererSize(
                        bottomStWrapper.getWidth(),
                        bottomStWrapper.getHeight());
                mBottomGraphicRenderer.setRendererSize(
                        bottomStWrapper.getWidth(),
                        bottomStWrapper.getHeight(),
                        isPipSwitched);
                mTopGraphicRenderer.setRendererSize(
                        bottomStWrapper.getWidth(),
                        bottomStWrapper.getHeight(),
                        false);
                mCaptureRenderer.setRendererSize(
                        bottomStWrapper.getWidth(),
                        bottomStWrapper.getHeight());
                mCaptureRenderer.setCaptureSurface(mCaptureSurface);

                // wrapping pip texture
                mPictureFb.setupFrameBufferGraphics(
                        bottomStWrapper.getWidth(),
                        bottomStWrapper.getHeight());
                float[] texRotateMtxByOrientation = GLUtil.createIdentityMtx();
                if (mCurrentOrientation % 180 != 0) {
                    android.opengl.Matrix.translateM(texRotateMtxByOrientation, 0,
                            texRotateMtxByOrientation, 0, .5f, .5f, 0);
                    // sub camera should rotated in 90/270 g-sensor
                    android.opengl.Matrix.rotateM(texRotateMtxByOrientation, 0,
                            -(mPipSwitched ? 180 : 0), 0, 0, 1);
                    android.opengl.Matrix.translateM(texRotateMtxByOrientation, 0, -.5f, -.5f, 0);
                }

                AnimationRect pictureTopGraphicRect = mPreviewTopGraphicRect.copy();
                int coordinateRotation = 90;
                // sub camera in bottom graphic, the cooridnate is 270 degree
                if (mCurrentOrientation % 180 == 0 && mPipSwitched) {
                    coordinateRotation = 270;
                }
                pictureTopGraphicRect.changeToLandscapeCooridnateSystem(
                        bottomStWrapper.getWidth(),
                        bottomStWrapper.getHeight(),
                        coordinateRotation);
                mBottomGraphicRenderer.draw(
                        bottomStWrapper.getTextureId(),
                        GLUtil.createIdentityMtx(), // keep sensor raw buffer's orientation
                        texRotateMtxByOrientation, // rotate to landscape
                        false);
                mTopGraphicRenderer.draw(
                        topStWrapper.getTextureId(),
                        GLUtil.createIdentityMtx(), // keep sensor raw buffer's orientation
                        texRotateMtxByOrientation, // rotate to landscape
                        pictureTopGraphicRect.copy(),
                        // -90/270 for rotate to portrait
                        (mCurrentOrientation - (mPipSwitched ? 270 : 90) + 360) % 360,
                        mPipSwitched);
                mPictureFb.setScreenBufferGraphics();
                // draw to capture
                if (mCaptureRenderer != null) {
                    mCaptureRenderer.draw(mPictureFb.getFboTexId());
                }
                // back to preview render size
                mBottomGraphicRenderer.setRendererSize(mRendererTexWidth,
                        mRendererTexHeight, false);
                mTopGraphicRenderer.setRendererSize(mRendererTexWidth,
                        mRendererTexHeight, true);
                mMainCap_StWrapper.resetSTStatus();
                mSubCap_StWrapper.resetSTStatus();
            }
        }

        // Should be called from other thread.
        public void sendMessageSync(int msg) {
            mEglThreadBlockVar.close();
            sendEmptyMessage(msg);
            mEglThreadBlockVar.block();
        }

        public void sendMessageSync(int msg, Object obj) {
            mEglThreadBlockVar.close();
            obtainMessage(msg, obj).sendToTarget();
            mEglThreadBlockVar.block();
        }
    }

    private void initialize() {
        mEglHandler.post(new Runnable() {
            @Override
            public void run() {
                mEgl = (EGL10) EGLContext.getEGL();
                mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
                if (mEglDisplay == EGL10.EGL_NO_DISPLAY) {
                    throw new RuntimeException("eglGetDisplay failed");
                }
                int[] version = new int[2];
                if (!mEgl.eglInitialize(mEglDisplay, version)) {
                    throw new RuntimeException("eglInitialize failed");
                } else {
                    LogHelper.v(TAG, "<initialize> EGL version: " + version[0] + '.' + version[1]);
                }
                int[] attribList = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
                mEglConfig = PipEGLConfigWrapper.getInstance().getEGLConfigChooser().chooseConfig(
                        mEgl, mEglDisplay);
                mEglContext = mEgl.eglCreateContext(mEglDisplay, mEglConfig, EGL10.EGL_NO_CONTEXT,
                        attribList);
                if (mEglContext == null || mEglContext == EGL10.EGL_NO_CONTEXT) {
                    throw new RuntimeException("failed to createContext");
                } else {
                    LogHelper.v(TAG, "<initialize> EGL context: create success");
                }
                mEglSurface = mEgl.eglCreatePbufferSurface(mEglDisplay, mEglConfig, null);
                if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE) {
                    LogHelper.w(TAG, "createWindowSurface error eglError = " + mEgl.eglGetError());
                    throw new RuntimeException("failed to createWindowSurface mEglSurface = "
                            + mEglSurface + " EGL_NO_SURFACE = " + EGL10.EGL_NO_SURFACE);
                } else {
                    LogHelper.v(TAG, "<initialize> EGL surface: create success");
                }
                if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
                    throw new RuntimeException("failed to eglMakeCurrent");
                } else {
                    LogHelper.v(TAG, "<initialize> EGL make current: success");
                }
            }
        });
    }

    // this method must be called in GL Thread
    private void releaseEgl() {
        LogHelper.d(TAG, "[releaseEgl]+");
        mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
        mEgl.eglDestroyContext(mEglDisplay, mEglContext);
        mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE,
                EGL10.EGL_NO_CONTEXT);
        mEgl.eglTerminate(mEglDisplay);
        mEglSurface = null;
        mEglContext = null;
        mEglDisplay = null;
        LogHelper.d(TAG, "[releaseEgl]-");
    }
}
