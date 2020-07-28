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

package com.mediatek.camera.mode.pip.pipwrapping;


import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.util.Log;

/**
 * This class is used to communicate with pip wrapping package.
 * <p>
 * Basic operating flow:
 * <p>
 * 1. start preview path:
 * <p>
 * Note: upadtexxxx can be called any time during preview
 * <p>
 * initPIPRenderer ->updateEffectTemplates --> setPreviewTextureSize
 * --> setUpSurfaceTextures -->setPreviewSurface-->getBottomSurfaceTexture
 * --> getTopSurfaceTexture --> updateTopGraphicPostion --> updateOrientation-->start preview
 * <p>
 * 2. take picture path:
 *    <p>
 *    Firstly, you should register PIPOperator.Listener to be notified when jpeg is done.
 *    <p>
 *    After preview is started, send two jpeg by: takePicture --> takePicture and then wait
 *    callback comes back.
 * <p>
 * 3. video recording path:
 * <p>
 *    prepareRecording -->  setRecordingSurface --> startPushVideoBuffer
 * <p>
 * 4. video snap shot path:
 * <p>
 *    takeVideoSnapshot
 */
public class PIPOperator implements PipCaptureExecutor.ImageCallback {
    private static final String TAG = PIPOperator.class.getSimpleName();
    private Activity mActivity;
    private RendererManager mRendererManager;
    private PipCaptureExecutor mPipCaptureExecutor;
    private Listener mListener;

    public interface Listener {
        void onPIPPictureTaken(byte[] jpegData);
        void unlockNextCapture();
        AnimationRect getPreviewAnimationRect();
    }

    public PIPOperator(Activity activity, Listener listener) {
        mActivity = activity;
        mListener = listener;
    }

    @Override
    public void onPictureTaken(byte[] jpegData) {
        Log.d(TAG, "onPIPPictureTaken jpegData = " + jpegData);
        if (mListener != null) {
            mListener.onPIPPictureTaken(jpegData);
        }
    }

    @Override
    public void unlockNextCapture() {
        Log.d(TAG, "canDoStartPreview");
        if (mListener != null) {
            mListener.unlockNextCapture();
        }
    }

    @Override
    public RendererManager getRendererManager() {
        return mRendererManager;
    }

    @Override
    public AnimationRect getPreviewAnimationRect() {
        if (mListener != null) {
            return mListener.getPreviewAnimationRect();
        }
        return null;
    }

    /**
     * initialize pip wrapping renderer, pip GL thread will be created here.
     * <p>
     * when GL thread is already exist, will not create it again.
     */
    public void initPIPRenderer() {
        Log.d(TAG, "initPIPRenderer");
        if (mRendererManager == null) {
            mRendererManager = new RendererManager(mActivity);
        }
        mRendererManager.init();
        if (mPipCaptureExecutor == null) {
            mPipCaptureExecutor = new PipCaptureExecutor(mActivity, mRendererManager, this);
        }
        mPipCaptureExecutor.init();
    }

    /**
     * update pip template resource.
     * Note: This function must be called before setUpSurfaceTextures.
     * @param backResourceId bottom graphic template
     * @param frontResourceId top graphic template
     * @param effectFrontHighlightId top graphic highlight template
     * @param editButtonResourceId top graphic edit template
     */
    public void updateEffectTemplates(int backResourceId, int frontResourceId,
            int effectFrontHighlightId, int editButtonResourceId) {
        Log.d(TAG, "updateEffectTemplates");
        mRendererManager.updateEffectTemplates(backResourceId, frontResourceId,
                effectFrontHighlightId, editButtonResourceId);
    }

    /**
     * when exit pip mode, should call this function to recycle resources.
     */
    public void unInitPIPRenderer() {
        if (mPipCaptureExecutor != null) {
            mPipCaptureExecutor.unInit();
        }
        mRendererManager.unInit();
    }

    /**
     * Set the bottom/top texture size.
     * Note: This function must be called before setUpSurfaceTextures.
     * @param width  preview texture's width
     * @param height preview texture's height
     */
    public void setPreviewTextureSize(int width, int height) {
        Log.d(TAG, "setTextureSize width = " + width + " height = " + height);
        mRendererManager.setPreviewSize(width, height);
    }

    /**
     * create two surface textures, switch pip by needSwitchPIP
     * <p>
     * By default, bottom surface texture is drawn in bottom graphic.
     * top surface texture is drawn in top graphic.
     */
    public void setUpSurfaceTextures() {
        Log.d(TAG, "setUpSurfaceTextures");
        mRendererManager.setUpSurfaceTextures();
    }

    /**
     * Set a surface to receive pip preview buffer from pip wrapping
     * @param surface used to receive pip preview buffer
     */
    public void setPreviewSurface(Surface surface) {
        Log.d(TAG, "setPreviewSurface surface = " + surface);
        mRendererManager.setPreviewSurfaceSync(surface);
    }

    /**
     * notify surface view has been destroyed
     *
     */
    public void notifySurfaceViewDestroyed(Surface surface) {
        Log.d(TAG, "notifySurfaceViewDestroyed");
        mRendererManager.notifySurfaceViewDestroyed(surface);
    }

    /**
     * This surface texture is used to receive bottom camera device's preview buffer.
     * <p>
     * It will update preview buffer to pip GL thread for processing when onFrameAvailabe
     * @return pip bottom surface texture
     */
    public SurfaceTexture getBottomSurfaceTexture() {
        return mRendererManager.getBottomPvSt();
    }

    /**
     * This surface texture is used to receive top camera device's preview buffer.
     * <p>
     * It will update preview buffer to pip GL thread for processing when onFrameAvailabe
     * @return pip top surface texture
     */
    public SurfaceTexture getTopSurfaceTexture() {
        return mRendererManager.getTopPvSt();
    }

    /**
     * update top graphic's position
     * @param topGraphic is an instance of AnimationRect
     */
    public void updateTopGraphic(AnimationRect topGraphic) {
        Log.d(TAG, "updateTopGraphicPostion topGraphic = " + topGraphic);
        mRendererManager.updateTopGraphic(topGraphic);
    }

    /**
     * when G-sensor's orientation changed, should update it to PIPOperator
     * @param newOrientation G-sensor's new orientation
     */
    public void updateGSensorOrientation(int newOrientation) {
        mRendererManager.updateGSensorOrientation(newOrientation);
    }

    public int getPreviewTexWidth() {
        return mRendererManager.getPreviewTextureWidth();
    }

    public int getPreviewTexHeight() {
        return mRendererManager.getPreviewTextureHeight();
    }

    public void switchPIP() {
        mRendererManager.switchPipSync();
    }

    public void setPictureSize(Size bottomCaptureSize, Size topCaptureSize) {
        mPipCaptureExecutor.setUpCapture(bottomCaptureSize, topCaptureSize);
    }

    /**
     * take a picture,pip capture should call twice: one is for bottom the other is for top.
     */
    public void offerJpegData(byte[] jpeg, int width, int height,
                                     boolean isBottomCamera, int captureOrienation) {
        mPipCaptureExecutor.offerJpegData(jpeg, new Size(width, height), isBottomCamera);
    }

    /**
     * Prepare recording renderer, will new a recording thread.
     * <p>
     * Note: before prepareRecording, the recording surface must be set.
     */
    public void prepareRecording() {
        Log.d(TAG, "prepareRecording");
        mRendererManager.prepareRecordSync();
    }

    /**
     * Set a recording surface to receive pip buffer from pip wrapping
     * @param surface a recording surface used to receive pip buffer.
     * @param orientation recorder orientation
     */
    public void setRecordingSurface(Surface surface, int orientation) {
        Log.d(TAG, "setRecordingSurface surface = " + surface);
        mRendererManager.setRecordSurfaceSync(surface, orientation);
    }

    /**
     * Begin to push pip frame to video recording surface.
     */
    public void startPushVideoBuffer() {
        Log.d(TAG, "startPushVideoBuffer");
        mRendererManager.startRecordSync();
    }

    /**
     * Stop to push pip frame to video recording surface.
     */
    public void stopPushVideoBuffer() {
        Log.d(TAG, "stopPushVideoBuffer");
        mRendererManager.stopRecordSync();
    }

    /**
     * Take a video snap shot by orientation
     * @param orientation video snap shot orientation
     */
    public void takeVideoSnapshot(int orientation, boolean isBackBottom) {
        Log.d(TAG, "takeVideoSnapshot orientation = " + orientation);
        boolean isLandscape = (orientation % 180 != 0);
        int max = Math.max(mRendererManager.getPreviewTextureHeight(),
                mRendererManager.getPreviewTextureWidth());
        int min = Math.min(mRendererManager.getPreviewTextureHeight(),
                mRendererManager.getPreviewTextureWidth());
        int width = isLandscape ? max : min;
        int height = isLandscape ? min : max;
        mRendererManager.takeVideoSnapShot(orientation,
                mPipCaptureExecutor.getVssSurface(width, height));
    }

    public static class PIPCustomization {
        private static final String TAG = "PIPCustomization";
        public static final String MAIN_CAMERA = "main_camera";
        public static final String SUB_CAMERA = "sub_camera";
        // scale
        public static final float TOP_GRAPHIC_MAX_SCALE_VALUE = 1.4f;
        public static final float TOP_GRAPHIC_MIN_SCALE_VALUE = 0.6f;
        // rotate
        public static final float TOP_GRAPHIC_MAX_ROTATE_VALUE = 180f;
        // top graphic edge, default is min(width, height) / 2
        public static final float TOP_GRAPHIC_DEFAULT_EDGE_RELATIVE_VALUE = 1f / 2;
        // edit button edge, default is min(width, height) / 10
        public static final int TOP_GRAPHIC_EDIT_BUTTON_RELATIVE_VALUE = 10;
        public static final float TOP_GRAPHIC_LEFT_TOP_RELATIVE_VALUE =
                TOP_GRAPHIC_DEFAULT_EDGE_RELATIVE_VALUE - 1f /
                    TOP_GRAPHIC_EDIT_BUTTON_RELATIVE_VALUE;
        // top graphic crop preview position
        public static final float TOP_GRAPHIC_CROP_RELATIVE_POSITION_VALUE = 3f / 4;
        // which camera enable FD, default is main camera support fd
        public static final String ENABLE_FACE_DETECTION = MAIN_CAMERA;
        // when take picture, whether sub camera need mirror
        public static final boolean SUB_CAMERA_NEED_HORIZONTAL_FLIP = true;
        private PIPCustomization() {
        }
        public static boolean isMainCameraEnableFD() {
            boolean enable = false;
            enable = ENABLE_FACE_DETECTION.endsWith(MAIN_CAMERA);
            Log.d(TAG, "isMainCameraEnableFD enable = " + enable);
            return enable;
        }
    }
}
