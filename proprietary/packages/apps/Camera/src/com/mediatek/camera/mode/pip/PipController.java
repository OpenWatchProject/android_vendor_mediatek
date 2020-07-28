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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.camera.mode.pip;

import java.util.concurrent.ConcurrentHashMap;

import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Size;
import android.view.Surface;

import com.android.camera.R;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.mode.pip.pipwrapping.AnimationRect;
import com.mediatek.camera.mode.pip.pipwrapping.PIPOperator;
import com.mediatek.camera.platform.ICameraAppUi.SpecViewType;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.util.Log;

public class PipController implements PipGestureManager.Listener, PIPOperator.Listener,
        PipView.Listener {
    private static final String           TAG = "PipController";

    private Activity                      mActivity;
    private static ConcurrentHashMap<Context, PipController> pipControllerList =
            new ConcurrentHashMap<Context, PipController>();

    /********************Communicate with  PIP Wrapping***************/
    private PIPOperator                   mPipOperator;

    /********************Communicate with PIP Gesture*****************/
    private PipGestureManager             mPipGestureManager;

    /********************Communicate with PIP View********************/
    private ICameraView                   mPipView = null;

    /********************Listener communicate with mode***************/
    private Listener                      mListener;
    /**
     * photo pip mode and video pip mode should implements this listener
     */

    private Object mSyncLock = new Object();
    private State mCurState = State.STATE_IDLE;

    private MainHandler mMainHandler;


    public interface Listener {
        int getGSensorOrientation();
        int getViewRotation();
        void onPIPPictureTaken(byte[] jpegData);
        void canDoStartPreview();
        int getButtomGraphicCameraId();
        void switchPIP();
    }

    public enum State {
        STATE_SWITCHING, STATE_IDLE, STATE_RECORD_STARTING, STATE_UNINIT,
    }

    @Override
    public int getGSensorOrientation() {
        return mListener.getGSensorOrientation();
    }

    @Override
    public int getButtomGraphicCameraId() {
        return mListener.getButtomGraphicCameraId();
    }

    @Override
    public void notifyTopGraphicIsEdited() {
        if (mPipView != null) {
            mPipView.refresh();
        }
    }

    @Override
    public void switchPIP() {
        Log.d(TAG, "switchPIP");
        mMainHandler.removeMessages(MSG_SWITCH_PIP);
        if (getState() == State.STATE_UNINIT) {
            return;
        }
        mMainHandler.sendEmptyMessage(MSG_SWITCH_PIP);
    }

    public void stopSwitchPip() {
        mMainHandler.removeMessages(MSG_SWITCH_PIP);
    }

    /**
     * call back from pip operator
     */
    @Override
    public void onPIPPictureTaken(byte[] jpegData) {
        Log.d(TAG, "onPIPPictureTaken jpegData = " + jpegData +
                " mListener = " + mListener);
        if (mListener != null) {
            mListener.onPIPPictureTaken(jpegData);
        }
    }

    @Override
    public void unlockNextCapture() {
        Log.d(TAG, "canDoStartPreview mListener = " + mListener);
        if (mListener != null) {
            mListener.canDoStartPreview();
        }
    }

    @Override
    public AnimationRect getPreviewAnimationRect() {
        if (mPipGestureManager != null) {
            return mPipGestureManager.getTopGraphicRect();
        }
        return null;
    }

    @Override
    public void onUpdateEffect(int backResourceId, int frontResourceId,
            int effectFrontHighlightId, int editButtonResourceId) {
        Log.d(TAG, "onUpdateEffect mListener = " + mListener);
        updateEffectTemplates(backResourceId, frontResourceId, effectFrontHighlightId,
                editButtonResourceId);
    }

    public static synchronized PipController instance(Context context) {
        Log.d(TAG, "instance pipControllerList size = " + pipControllerList.size());
        PipController pipController = pipControllerList.get(context);
        if (pipController == null) {
            pipController = new PipController();
            pipControllerList.put(context, pipController);
        }
        return pipController;
    }

    public void setListener(Listener listener) {
        Log.d(TAG, "setListener");
        mListener = listener;
    }

    /**
     * Set a surface to receive pip preview buffer from pip wrapping
     * @param surface used to receive pip preview buffer
     */
    public void setPreviewSurface(Surface surface) {
        Log.d(TAG, "setPreviewSurface mPipOperator = " + mPipOperator);
        synchronized (mSyncLock) {
            if (mPipOperator != null) {
                mPipOperator.setPreviewSurface(surface);
            }
        }
    }

    /**
     * notify surface view has destroyed
     *
     */
    public void notifySurfaceViewDestroyed(Surface surface) {
        Log.d(TAG, "notifySurfaceViewDestroyed mPipOperator = " + mPipOperator);
        if (mPipOperator != null) {
            mPipOperator.notifySurfaceViewDestroyed(surface);
        }
    }

    /**
     * Set the bottom/top texture size.
     * Note: This function must be called before setUpSurfaceTextures.
     * @param width  preview texture's width
     * @param height preview texture's height
     */
    public void setPreviewTextureSize(int width, int height) {
        Log.d(TAG, "setTextureSize width = " + width + " height = " + height);
        synchronized (mSyncLock) {
            if (mPipOperator != null && mPipGestureManager != null) {
                mPipOperator.setUpSurfaceTextures();
                mPipOperator.setPreviewTextureSize(width, height);
                mPipGestureManager.setRendererSize(width, height);
                mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
            }
        }
    }

    /**
     * This surface texture is used to receive bottom camera device's preview buffer.
     * <p>
     * It will update preview buffer to pip GL thread for processing when onFrameAvailabe
     * @return pip bottom surface texture
     */
    public SurfaceTexture getBottomSurfaceTexture() {
        synchronized (mSyncLock) {
            if (mPipOperator != null) {
                return mPipOperator.getBottomSurfaceTexture();
            }
        }
        return null;
    }

    /**
     * This surface texture is used to receive top camera device's preview buffer.
     * <p>
     * It will update preview buffer to pip GL thread for processing when onFrameAvailabe
     * @return pip top surface texture
     */
    public SurfaceTexture getTopSurfaceTexture() {
        synchronized (mSyncLock) {
            if (mPipOperator != null) {
                return mPipOperator.getTopSurfaceTexture();
            }
        }
        return null;
    }

    /********************PIP Capture************************/

    public void setPictureSize(Size bottom, Size top) {
        if (mPipOperator != null) {
            mPipOperator.setPictureSize(bottom, top);
        }
    }

    /**
     * take a picture, pip capture should call twice: one is for bottom the other is for top.
     * <p>
     * Note: this way is just for API1. API2 we will user surface to get jpeg more efficiency
     * @param jpeg the jpeg data
     * @param width jpeg's width
     * @param height jpeg's height
     * @param isBottomCamera indicate whether this jpeg is from bottom
     */
    public void takePicture(byte[] jpeg, int width, int height, boolean isBottomCamera,
            int captureOrientation) {
        Log.d(TAG, "takePicture jpeg = " + jpeg +              " width = " + width +
                   " height = " + height +
                   " isBottomCamera = " + isBottomCamera);
        if (mPipOperator != null) {
            mPipOperator.offerJpegData(jpeg, width, height, isBottomCamera, captureOrientation);
        }
    }

    /********************PIP Recording************************/
    /**
     * Prepare recording renderer, will new a recording thread.
     * <p>
     * Note: before prepareRecording, the recording surface must be set.
     */
    public void prepareRecording() {
        Log.d(TAG, "prepareRecording");
        if (mPipOperator != null) {
            mPipOperator.prepareRecording();
        }
    }

    /**
     * Set a recording surface to receive pip buffer from pip wrapping.
     * @param surface a recording surface used to receive pip buffer.
     * @param orientation the vertex orientation.
     */
    public void setRecordingSurface(Surface surface, int orientation) {
        Log.d(TAG, "setRecordingSurface surface = " + surface + " orientation = " + orientation);
        if (mPipOperator != null) {
            mPipOperator.setRecordingSurface(surface, orientation);
        }
    }

    /**
     * Begin to push pip frame to video recording surface.
     */
    public void startPushVideoBuffer() {
        Log.d(TAG, "startPushVideoBuffer");
        if (mPipOperator != null) {
            mPipOperator.startPushVideoBuffer();
        }
    }

    /**
     * Stop to push pip frame to video recording surface.
     */
    public void stopPushVideoBuffer() {
        Log.d(TAG, "stopPushVideoBuffer");
        if (mPipOperator != null) {
            mPipOperator.stopPushVideoBuffer();
        }
    }

    /**
     * Take a video snap shot by orientation
     * @param orientation video snap shot orientation
     */
    public void takeVideoSnapshot(int orientation, boolean isBackBottom) {
        Log.d(TAG, "takeVideoSnapshot orientation = " + orientation + " isBackBottom = "
                + isBackBottom);
        if (mPipOperator != null) {
            mPipOperator.takeVideoSnapshot(orientation, isBackBottom);
        }
    }

    /********************Gesture related**************************/
    /**
     * When onDown comes, receive it
     * @return true this gesture can not pass to other views;
     *         false this gesture can pass to other views
     */
    public boolean onDown(float x, float y, int width, int height) {
        boolean enable = false;
        if (mPipGestureManager != null && mPipOperator != null) {
            enable = mPipGestureManager.onDown(x, y, width, height);
            mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
        }
        return enable;
    }

    public boolean onUp() {
        boolean enable = false;
        if (mPipGestureManager != null && mPipOperator != null) {
            enable = mPipGestureManager.onUp();
            mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
        }
        return enable;
    }

    public boolean onSingleTapUp(float x, float y) {
        boolean enable = false;
        if (mPipView != null) {
            mPipView.update(PipView.TYPE_HIDE_EFFECT);
        }
        if (mPipGestureManager != null && mPipOperator != null) {
            enable = mPipGestureManager.onSingleTapUp(x, y);
            mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
        }
        return enable;
    }

    public boolean onLongPress(float x, float y) {
        boolean enable = false;
        if (mPipGestureManager != null && mPipOperator != null) {
            enable = mPipGestureManager.onLongPress(x, y);
            mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
        }
        return enable;
    }

    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        boolean enable = false;
        if (mPipGestureManager != null && mPipOperator != null) {
            enable = mPipGestureManager.onScroll(dx, dy, totalX, totalY);
            mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
        }
        return enable;
    }

    public void onGSensorOrientationChanged(int orientation) {
        Log.d(TAG, "onGSensorOrientationChanged orientation = " + orientation);
        // notify pip wrapping orientation changed
        if (mPipOperator != null) {
            mPipOperator.updateGSensorOrientation(orientation);
        }
    }

    public void onViewOrienationChanged(int viewOrientation) {
        Log.d(TAG, "onViewOrienationChanged orientation = " + viewOrientation);
        // notify pip gesture orientation changed
        if (mPipGestureManager != null) {
            mPipGestureManager.onViewOrientationChanged(viewOrientation);
        }
        if (mPipOperator != null) {
            mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
        }
        // notify pip view orientation changed
        if (mPipView != null) {
            mPipView.update(PipView.TYPE_ORIENTATION_CHANGED, viewOrientation);
        }
    }

    public void setDisplayRotation(int displayRotation) {
        Log.d(TAG, "setDisplayRotation displayRotation = " + displayRotation);
        if (mPipGestureManager != null && mPipOperator != null) {
            mPipGestureManager.setDisplayRotation(displayRotation);
            mPipGestureManager.onViewOrientationChanged(mListener.getViewRotation());
            mPipOperator.updateGSensorOrientation(getGSensorOrientation());
            mPipOperator.updateTopGraphic(mPipGestureManager.getTopGraphicRect());
        }
    }

    /********************pip view methods************************/
    public void hideModeViews(boolean hide) {
        Log.d(TAG, "hideModeViews hide = " + hide);
        if (mPipView != null) {
            if (hide) {
                mPipView.hide();
            } else {
                mPipView.show();
            }
        }
    }

    public void enableView(boolean enable) {
        Log.d(TAG, "enableView enable = " + enable);
        if (mPipView != null) {
            mPipView.setEnabled(enable);
        }
    }

    public boolean isPipEffectShowing() {
        if (mPipView != null && mPipView.isShowing()) {
            return true;
        }
        return false;
    }

    public void closeEffects() {
        if (mPipView != null) {
            mPipView.refresh();
        }
    }

    /********************life cycle methods************************/
    public void init(ICameraContext cameraContext, Listener listener) {
        Log.d(TAG, "init mPipOperator = " + mPipOperator);
        mActivity = cameraContext.getActivity();
        mMainHandler = new MainHandler(mActivity.getMainLooper());
        mListener = listener;
        synchronized (mSyncLock) {
            if (mPipGestureManager == null) {
                mPipGestureManager = new PipGestureManager(mActivity, this);
            }
            if (mPipOperator == null) {
                mPipOperator = new PIPOperator(mActivity, this);
                mPipOperator.initPIPRenderer();
            }
//            if (mPipView == null) {
//                mPipView = cameraContext.getCameraAppUi().getCameraView(SpecViewType.MODE_PIP);
//                mPipView.init(mActivity, cameraContext.getCameraAppUi(),
//                        cameraContext.getModuleController());
//                mPipView.setListener(this);
//                mPipView.update(PipView.TYPE_ORIENTATION_CHANGED, mListener.getViewRotation());
//            }
            // use this template as default.
            updateEffectTemplates(R.drawable.rear_01, R.drawable.front_01,
                    R.drawable.front_01_focus, R.drawable.plus);
            if (mPipView != null) {
                mPipView.show();
            }
        }
    }

    public void pause() {
        Log.i(TAG, "pause");
        setState(State.STATE_UNINIT);
        mMainHandler.removeMessages(MSG_SWITCH_PIP);
        synchronized (mSyncLock) {
            if (mPipOperator != null) {
                mPipOperator.unInitPIPRenderer();
            }
            if (mPipView != null) {
                mPipView.hide();
            }
        }
    }

    public void resume() {
        Log.d(TAG, "resume mPipOperator = " + mPipOperator);
        synchronized (mSyncLock) {
            if (mPipOperator == null) {
                mPipOperator = new PIPOperator(mActivity, this);
            }
            mPipOperator.initPIPRenderer();
            if (mPipView != null) {
                mPipView.show();
            }
        }
        setState(State.STATE_IDLE);
    }

    public void unInit(Context context) {
        Log.d(TAG, "unInit pipControllerList size = " + pipControllerList.size());
        synchronized (mSyncLock) {
            if (mPipOperator != null) {
                mPipOperator.unInitPIPRenderer();
                mPipOperator = null;
            }
            if (mPipView != null) {
                mPipView.uninit();
            }
            mPipGestureManager = null;
            pipControllerList.remove(context);
        }
    }

    public void setState(State state) {
        Log.d(TAG, "setState state = " + state);
        mCurState = state;
    }

    public State getState() {
        Log.d(TAG, "getState");
        return mCurState;
    }

    private PipController() {
        Log.d(TAG, "PIPController");
    }

    /**
     * update pip template resource.
     * Note: This function must be called before setUpSurfaceTextures.
     * @param backResourceId bottom graphic template
     * @param frontResourceId top graphic template
     * @param effectFrontHighlightId top graphic highlight template
     * @param editButtonResourceId top graphic edit template
     */
    private void updateEffectTemplates(int backResourceId, int frontResourceId,
            int effectFrontHighlightId, int editButtonResourceId) {
        Log.d(TAG, "updateEffectTemplates");
        if (mPipOperator != null) {
            mPipOperator.updateEffectTemplates(backResourceId, frontResourceId,
                    effectFrontHighlightId, editButtonResourceId);
        }
    }

    private static final int MSG_SWITCH_PIP = 1;
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage] msg:" + msg.what);

            switch (msg.what) {
            case MSG_SWITCH_PIP:
                mCurState = State.STATE_SWITCHING;
                synchronized (mSyncLock) {
                    if (mPipOperator != null) {
                        mPipOperator.switchPIP();
                    }
                    if (mListener != null) {
                        mListener.switchPIP();
                    }
                }
                Log.d(TAG, "switchPIP end");
                break;

            default:
                break;
            }
        }
    }
}
