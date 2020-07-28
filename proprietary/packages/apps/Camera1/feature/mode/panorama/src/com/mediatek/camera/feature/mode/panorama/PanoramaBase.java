/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.mode.panorama;

import android.hardware.Camera;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;

import com.google.common.base.Preconditions;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUi.AnimationData;
import com.mediatek.camera.common.IAppUiListener.ISurfaceStatusListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraModeBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.photo.PhotoModeHelper;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.relation.StatusMonitor.StatusChangeListener;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingManagerFactory;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.mode.panorama.IPanoramaDeviceController.CameraStateCallback;
import com.mediatek.camera.feature.mode.panorama.IPanoramaDeviceController.PreviewCallback;
import com.mediatek.camera.feature.mode.panorama.IPanoramaDeviceController.PreviewSizeCallback;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * Panorama base.
 */
public class PanoramaBase extends CameraModeBase implements PreviewCallback, CameraStateCallback,
        PreviewSizeCallback {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(PanoramaBase.class.getSimpleName());

    private static final String CAMERA_FACING_BACK = "back";
    private static final String CAMERA_FACING_FRONT = "front";
    private static final String KEY_PICTURE_SIZE = "key_picture_size";

    protected IPanoramaDeviceController mIPanoramaDeviceController;
    protected PhotoModeHelper mPhotoModeHelper;
    protected int mCaptureWidth;
    protected int mCaptureHeight;
    protected String mCameraId;
    //the reason is if surface is ready, let it to set to device controller, otherwise
    //if surface is ready but activity is not into resume ,will found the preview
    //can not start preview.
    private volatile boolean mIsResumed = true;

    private ISurfaceStatusListener mISurfaceStatusListener = new SurfaceChangeListener();
    protected ISettingManager mISettingManager;
    private volatile SwitchCameraTask mSwitchCameraTask;
    private ISettingManager.SettingController mSettingController;
    private byte[] mPreviewData;
    private int mPreviewFormat;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private Object mPreviewDataSync = new Object();
    private HandlerThread mAnimationHandlerThread;
    private Handler mAnimationHandler;
    private StatusChangeListener mStatusChangeListener = new MyStatusChangeListener();

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
                     boolean isFromLaunch) {
        LogHelper.d(TAG, "[init]+");
        Preconditions.checkNotNull(app);
        Preconditions.checkNotNull(cameraContext);
        super.init(app, cameraContext, isFromLaunch);
        mPhotoModeHelper = new PhotoModeHelper(cameraContext);
        createAnimationHandler();

        mCameraId = getCameraIdByFacing(mDataStore.getValue(
                KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));
        mIPanoramaDeviceController = new PanoramaDeviceController(app.getActivity(),
                mICameraContext);
        mIPanoramaDeviceController.setCameraStateCallback(this);
        initSettingManager(mCameraId);
        prepareAndOpenCamera(false);
        LogHelper.d(TAG, "[init]-");
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        LogHelper.d(TAG, "[resume]+");
        super.resume(deviceUsage);
        synchronized ((Object) mIsResumed) {
            mIsResumed = true;
        }
        mCameraId = getCameraIdByFacing(mDataStore.getValue(
                KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));
        mIPanoramaDeviceController.queryCameraDeviceManager();
        initSettingManager(mCameraId);
        prepareAndOpenCamera(false);
        LogHelper.d(TAG, "[resume]-");
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        LogHelper.d(TAG, "[pause]+");
        super.pause(nextModeDeviceUsage);
        boolean needCloseCamera =
                mNeedCloseCameraIds == null ? true : mNeedCloseCameraIds.size() > 0;
        boolean isSwitchMode = mNeedCloseCameraIds != null;
        synchronized ((Object) mIsResumed) {
            mIsResumed = false;
        }
        //clear the surface listener
        mIApp.getAppUi().clearPreviewStatusListener(mISurfaceStatusListener);
        //camera animation.
        synchronized (mPreviewDataSync) {
            if (isSwitchMode && mPreviewData != null) {
                startChangeModeAnimation();
            }
        }
        if (needCloseCamera) {
            prePareAndCloseCamera(needCloseCameraSync());
            recycleSettingManager(mCameraId);
        } else {
            clearAllCallbacks();
            //do stop preview
            mIPanoramaDeviceController.stopPreview();
        }

        LogHelper.d(TAG, "[pause]-");
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "[unInit]+");
        super.unInit();
        destroyAnimationHandler();
        mIPanoramaDeviceController.destroyDeviceController();
        LogHelper.d(TAG, "[unInit]-");
    }

    @Override
    public boolean onCameraSelected(@Nonnull String newCameraId) {
        LogHelper.i(TAG, "[onCameraSelected] ,new id:" + newCameraId + ",current id:" + mCameraId);
        //first need check whether can switch camera or not.
        if (canSelectCamera(newCameraId)) {
            //trigger switch camera animation in here
            //must before mCamera = newCameraId, otherwise the animation's orientation and
            // whether need mirror is error.
            synchronized (mPreviewDataSync) {
                startSwitchCameraAnimation();
            }
            recycleSettingManager(mCameraId);
            //we must unregister before change camera id.
            clearAllCallbacks();

            //update the camera id
            mCameraId = newCameraId;
            initSettingManager(mCameraId);
            //use async task to switch camera
            doCameraSelect();
            return true;
        } else {
            return false;
        }
    }

    @Override
    protected ISettingManager getSettingManager() {
        return mISettingManager;
    }

    @Override
    public boolean onShutterButtonClick() {
        return true;
    }

    @Override
    public void onPreviewCallback(byte[] data, int format) {
        synchronized ((Object) mIsResumed) {
            if (!mIsResumed) {
                return;
            }
        }
        // Because we want use the one preview frame for doing switch camera animation
        // so will dismiss the later frames.
        // The switch camera data will be restore to null when camera close done.
        synchronized (mPreviewDataSync) {
            if (mPreviewData == null) {
                //Notify preview started.
                String current = getCameraIdByFacing(mDataStore.getValue(
                        KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));
                mIApp.getAppUi().applyAllUIEnabled(true);
                mIApp.getAppUi().onPreviewStarted(current);
                updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
                stopAllAnimations();
            }
            mPreviewData = data;
            mPreviewFormat = format;
        }
    }


    @Override
    public void onCameraOpened() {
        LogHelper.i(TAG, "[onCameraOpened]");
        updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
    }

    @Override
    public void beforeCloseCamera() {
        updateModeDeviceState(MODE_DEVICE_STATE_CLOSED);
    }

    @Override
    public void onCameraPreviewStarted() {

    }

    @Override
    public void onCameraPreviewStopped() {
        updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
    }

    @Override
    public void onPreviewSizeReady(Size previewSize) {
        LogHelper.d(TAG, "[onPreviewSizeReady] previewSize: " + previewSize.toString());
        int width = previewSize.getWidth();
        int height = previewSize.getHeight();
        if (width != mPreviewWidth || height != mPreviewHeight) {
            onPreviewSizeChanged(width, height);
        }
    }

    private void onPreviewSizeChanged(int width, int height) {
        //Need reset the preview data to null if the preview size is changed.
        synchronized (mPreviewDataSync) {
            mPreviewData = null;
        }
        mPreviewWidth = width;
        mPreviewHeight = height;
        mIApp.getAppUi().setPreviewSize(mPreviewWidth, mPreviewHeight, mISurfaceStatusListener);
    }

    private void prepareAndOpenCamera(boolean needOpenCameraSync) {
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        statusMonitor.registerValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);

        //before open camera, prepare the preview callback and size changed callback.
        mIPanoramaDeviceController.setPreviewCallback(this);
        mIPanoramaDeviceController.setPreviewSizeReadyCallback(this);
        //prepare device info.
        PanoramaDeviceInfo info = new PanoramaDeviceInfo();
        info.setCameraId(mCameraId);
        info.setSettingManager(mISettingManager);
        info.setNeedOpenCameraSync(needOpenCameraSync);
        mIPanoramaDeviceController.openCamera(info);
    }

    private void prePareAndCloseCamera(boolean needSync) {
        clearAllCallbacks();
        mIPanoramaDeviceController.closeCamera(needSync);
        //reset the preview size and preview data.
        mPreviewData = null;
        mPreviewWidth = 0;
        mPreviewHeight = 0;
    }

    private void clearAllCallbacks() {
        mIPanoramaDeviceController.setPreviewSizeReadyCallback(null);
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        statusMonitor.unregisterValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
    }

    private void initSettingManager(String cameraId) {
        SettingManagerFactory smf = mICameraContext.getSettingManagerFactory();
        mISettingManager = smf.getInstance(
                cameraId,
                getModeKey(),
                ModeType.PHOTO,
                mCameraApi);
        mSettingController = mISettingManager.getSettingController();
    }

    private void recycleSettingManager(String cameraId) {
        mICameraContext.getSettingManagerFactory().recycle(cameraId);
    }

    private void createAnimationHandler() {
        mAnimationHandlerThread = new HandlerThread("Animation_handler");
        mAnimationHandlerThread.start();
        mAnimationHandler = new Handler(mAnimationHandlerThread.getLooper());
    }

    private void destroyAnimationHandler() {
        if (mAnimationHandlerThread.isAlive()) {
            mAnimationHandlerThread.quit();
            mAnimationHandler = null;
        }
    }

    private void stopAllAnimations() {
        LogHelper.d(TAG, "[stopAllAnimations]");
        //clear the old one.
        mAnimationHandler.removeCallbacksAndMessages(null);
        mAnimationHandler.post(new Runnable() {
            @Override
            public void run() {
                LogHelper.d(TAG, "[stopAllAnimations] run");
                //means preview is started, so need notify switch camera animation need stop.
                stopSwitchCameraAnimation();
                //need notify change mode animation need stop if is doing change mode.
                stopChangeModeAnimation();
            }
        });
    }

    private void startSwitchCameraAnimation() {
        // Prepare the animation data.
        AnimationData data = prepareAnimationData(mPreviewData, mPreviewWidth,
                mPreviewHeight, mPreviewFormat);
        // Trigger animation start.
        mIApp.getAppUi().animationStart(IAppUi.AnimationType.TYPE_SWITCH_CAMERA, data);
    }

    private void stopSwitchCameraAnimation() {
        mIApp.getAppUi().animationEnd(IAppUi.AnimationType.TYPE_SWITCH_CAMERA);
    }

    private void startChangeModeAnimation() {
        AnimationData data = prepareAnimationData(mPreviewData, mPreviewWidth,
                mPreviewHeight, mPreviewFormat);
        mIApp.getAppUi().animationStart(IAppUi.AnimationType.TYPE_SWITCH_MODE, data);
    }

    private void stopChangeModeAnimation() {
        mIApp.getAppUi().animationEnd(IAppUi.AnimationType.TYPE_SWITCH_MODE);
    }

    private AnimationData prepareAnimationData(byte[] data, int width, int height, int format) {
        // Prepare the animation data.
        AnimationData animationData = new AnimationData();
        animationData.mData = data;
        animationData.mWidth = width;
        animationData.mHeight = height;
        animationData.mFormat = format;
        animationData.mOrientation = mPhotoModeHelper.getCameraInfoOrientation(mCameraId);
        animationData.mIsMirror = mPhotoModeHelper.isMirror(mCameraId);
        return animationData;
    }

    /**
     * surface changed listener.
     */
    private class SurfaceChangeListener implements ISurfaceStatusListener {

        @Override
        public void surfaceAvailable(Object surfaceObject, int width, int height) {
            LogHelper.i(TAG, "surfaceAvailable,device controller = " + mIPanoramaDeviceController
                    + ",w = " + width + ",h = " + height);
            synchronized ((Object) mIsResumed) {
                if (mIPanoramaDeviceController != null && mIsResumed) {
                    mIPanoramaDeviceController.updatePreviewSurface(surfaceObject);
                }
            }
        }

        @Override
        public void surfaceChanged(Object surfaceObject, int width, int height) {
            LogHelper.i(TAG, "surfaceChanged, device controller = " + mIPanoramaDeviceController
                    + ",w = " + width + ",h = " + height);
            synchronized ((Object) mIsResumed) {
                if (mIPanoramaDeviceController != null && mIsResumed) {
                    mIPanoramaDeviceController.updatePreviewSurface(surfaceObject);
                }
            }
        }

        @Override
        public void surfaceDestroyed(Object surfaceObject, int width, int height) {
            LogHelper.i(TAG, "surfaceDestroyed,device controller = " + mIPanoramaDeviceController);
            // if destroy,do noting.
            if (mIPanoramaDeviceController != null && mIsResumed) {
                mIPanoramaDeviceController.updatePreviewSurface(null);
            }
        }
    }

    /**
     * this class used for switch camera.
     */
    private class SwitchCameraTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            LogHelper.d(TAG, "[onPreExecute], will disable all UI");
            synchronized ((Object) mIsResumed) {
                if (mIsResumed) {
                    mIApp.getAppUi().applyAllUIEnabled(false);
                }
            }
        }

        @Override
        protected Void doInBackground(Void... voids) {
            LogHelper.d(TAG, "[doInBackground]");
            synchronized ((Object) mIsResumed) {
                if (mIsResumed) {
                    prePareAndCloseCamera(true);
                    prepareAndOpenCamera(false);
                }
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            LogHelper.d(TAG, "[onPostExecute], will enable all UI");
            synchronized ((Object) mIsResumed) {
                //here need enable all the UI
                mIApp.getAppUi().applyAllUIEnabled(true);
            }
            clearSwitchTask();
        }

        @Override
        protected void onCancelled(Void aVoid) {
            super.onCancelled(aVoid);
            LogHelper.d(TAG, "[onCancelled]");
            clearSwitchTask();
        }

        private void clearSwitchTask() {
            if (mSwitchCameraTask != null) {
                synchronized (mSwitchCameraTask) {
                    mSwitchCameraTask = null;
                }
            }
        }
    }

    /**
     * Status change listener implement.
     */
    private class MyStatusChangeListener implements StatusChangeListener {
        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.i(TAG, "[onStatusChanged] key = " + key + ",value = " + value);
            if (value != null && KEY_PICTURE_SIZE.equalsIgnoreCase(key)) {
                String[] sizes = value.split("x");
                mCaptureWidth = Integer.parseInt(sizes[0]);
                mCaptureHeight = Integer.parseInt(sizes[1]);
                Size previewSize = mIPanoramaDeviceController.getPreviewSize((double)
                        mCaptureWidth / mCaptureHeight);
                int width = previewSize.getWidth();
                int height = previewSize.getHeight();
                if (width != mPreviewWidth || height != mPreviewHeight) {
                    onPreviewSizeChanged(width, height);
                }
            }
        }
    }

    private String getCamerasFacing(int cameraId) {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);

        String facing = null;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
            facing = CAMERA_FACING_BACK;
        } else if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            facing = CAMERA_FACING_FRONT;
        }
        return facing;
    }

    private boolean canSelectCamera(@Nonnull String newCameraId) {
        boolean value = true;
        boolean isInSwitchCamera = false;
        if (mSwitchCameraTask != null) {
            synchronized (mSwitchCameraTask) {
                isInSwitchCamera = AsyncTask.Status.RUNNING == mSwitchCameraTask.getStatus();
            }
        }
        if (newCameraId == null || mCameraId.equalsIgnoreCase(newCameraId) || isInSwitchCamera) {
            value = false;
        }
        try {
            int cameraNum = Integer.valueOf(newCameraId);
            boolean isFacingFront = CAMERA_FACING_FRONT.equals(getCamerasFacing(cameraNum));
            if (isFacingFront) {
                value = false;
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[canSelectCamera] +: " + value);
        return value;
    }

    private void doCameraSelect() {
        handleOldSwitchCameraTask();
        //create a switch camera task.
        mSwitchCameraTask = new SwitchCameraTask();
        mSwitchCameraTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    private void handleOldSwitchCameraTask() {
        if (mSwitchCameraTask != null) {
            //if the old task is pending or running state,need cancel the old one.
            synchronized (mSwitchCameraTask) {
                mSwitchCameraTask.cancel(true);
            }
        }
    }

}