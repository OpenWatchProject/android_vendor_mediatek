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

package com.mediatek.camera.common.mode.video.device.v1;

import android.app.Activity;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;

import android.media.CamcorderProfile;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.google.common.base.Preconditions;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.device.v1.CameraProxy.StateCallback;
import com.mediatek.camera.common.mode.video.device.IDeviceController;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceConfigurator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.portability.CamcorderProfileEx;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * An implementation of {@link IDeviceController} with CameraProxy.
 */
public class VideoDeviceController implements IDeviceController,
                                         ISettingManager.SettingDeviceRequester {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(VideoDeviceController.class.getSimpleName());
    private static final String KEY_VIDEO_QUALITY = "key_video_quality";
    private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String FALSE = "false";

    private static final double ASPECT_TOLERANCE = 0.001;
    private static final int HANDLER_UPDATE_PREVIEW_SURFACE = 1;
    private static final int UN_KNOW_ORIENTATION = -1;
    private static final int WAIT_TIME = 5;

    private volatile CameraState mCameraState = CameraState.CAMERA_UNKNOWN;
    private volatile CameraProxy mCameraProxy;
    private boolean mCanConfigParameter = false;
    private boolean mIsDuringRecording = false;
    private boolean mNeedRestartPreview;
    private int mJpegRotation = -1;
    private int mPreviewFormat;

    private StatusMonitor.StatusChangeListener mStatusListener = new MyStatusChangeListener();
    private StateCallback mCameraProxyStateCallback = new CameraDeviceProxyStateCallback();
    private Object mWaitOpenCamera = new Object();
    private Lock mLockCameraAndRequestSettingsLock = new ReentrantLock();
    private Lock mLockState = new ReentrantLock();
    private Lock mLock = new ReentrantLock();

    private SettingDeviceConfigurator mSettingDeviceConfigurator;
    private CameraDeviceManager mCameraDeviceManager;
    private RestrictionProvider mRestrictionProvider;
    private SettingConfigCallback mSettingConfig;
    private DeviceCallback mModeDeviceCallback;
    private JpegCallback mJpegReceivedCallback;
    private VideoDeviceHandler mVideoHandler;
    private PreviewCallback mPreviewCallback;
    private ISettingManager mSettingManager;
    private ICameraContext mICameraContext;
    private String mRememberSceneModeValue;
    private StatusMonitor mStatusMonitor;
    private Object mSurfaceObject;
    private CamcorderProfile mProfile;
    private CameraInfo[] mInfo;
    private Activity mActivity;
    private Size mPreviewSize;
    private String mCameraId;
    /**
     * this enum is used for tag native camera open state.
     */
    private enum CameraState {
        CAMERA_UNKNOWN, //initialize state.
        CAMERA_OPENING, //between open camera and open done callback.
        CAMERA_OPENED, //when camera open done.
        CAMERA_CLOSING, //when close camera is call.
    }
    /**
     * VideoDeviceController may use activity to get display rotation.
     * @param activity the camera activity.
     * @param context the camera context.
     */
    public VideoDeviceController(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        Preconditions.checkNotNull(activity);
        Preconditions.checkNotNull(context);
        mActivity = activity;
        mICameraContext = context;
        HandlerThread ht = new HandlerThread("Video Device Handler Thread");
        ht.start();
        mVideoHandler = new VideoDeviceHandler(ht.getLooper());
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API1);
        initializeCameraInfo();
    }

    @Override
    public void setPreviewCallback(PreviewCallback callback1, DeviceCallback callback2) {
        mPreviewCallback = callback1;
        mModeDeviceCallback = callback2;
        if (mCameraProxy != null && mPreviewCallback == null) {
            mCameraProxy.setPreviewCallback(null);
        }
    }

    @Override
    public void requestChangeSettingValueJustSelf(String key) {
    }

    @Override
    public void setSettingConfigCallback(SettingConfigCallback callback) {
        mSettingConfig = callback;
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API1);
    }

    @Override
    public void openCamera(@Nonnull ISettingManager settingManager,
                           @Nonnull String cameraId, boolean sync, RestrictionProvider relation) {
        LogHelper.i(TAG, "[openCamera] + proxy = " + mCameraProxy + " id = " + cameraId
                + " sync = " + sync + " mCameraState = " + mCameraState);
        Preconditions.checkNotNull(cameraId);
        mRestrictionProvider = relation;
        try {
            mLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
            if (canDoOpenCamera(cameraId)) {
                updateCameraState(CameraState.CAMERA_OPENING);
                mSettingManager = settingManager;
                mSettingManager.updateModeDeviceRequester(this);
                mSettingDeviceConfigurator = mSettingManager.getSettingDeviceConfigurator();
                mCameraId = cameraId;
                if (sync) {
                    mCameraDeviceManager.openCameraSync(mCameraId, mCameraProxyStateCallback, null);
                } else {
                    mCameraDeviceManager.openCamera(mCameraId, mCameraProxyStateCallback, null);
                }
            }
        } catch (CameraOpenException e) {
            if (CameraOpenException.ExceptionType.SECURITY_EXCEPTION == e.getExceptionType()) {
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_HARDWARE_EXCEPTION);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            mLock.unlock();
        }
        LogHelper.i(TAG, "[openCamera] -");
    }

    protected void updateCameraState(CameraState state) {
        LogHelper.d(TAG, "[updateCameraState] new = " + state + " old state =" + mCameraState);
        mLockState.lock();
        try {
            mCameraState = state;
        } finally {
            mLockState.unlock();
        }
    }
    /**
     * use safe to get VideoState.
     * @return the video state.
     */
    protected CameraState getCameraState() {
        mLockState.lock();
        try {
            return mCameraState;
        } finally {
            mLockState.unlock();
        }
    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.d(TAG, "[closeCamera] + mCameraState =" + mCameraState);
        if (CameraState.CAMERA_UNKNOWN != getCameraState()) {
            try {
                mLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                waitOpenDoneForClose();
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                if (mStatusMonitor != null) {
                    mStatusMonitor.unregisterValueChangedListener(KEY_SCENE_MODE, mStatusListener);
                }
                mModeDeviceCallback.beforeCloseCamera();
                doCloseCamera(sync);
                releaseVariables();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                mLock.unlock();
            }
        }
        mCameraId = null;
        LogHelper.d(TAG, "[closeCamera] - mCameraState =" + mCameraState);
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        mVideoHandler.obtainMessage(HANDLER_UPDATE_PREVIEW_SURFACE, surfaceObject).sendToTarget();
    }

    @Override
    public void stopPreview() {
        if (mCameraProxy != null) {
            LogHelper.i(TAG, "[stopPreview]");
            mCameraProxy.stopPreview();
            mModeDeviceCallback.afterStopPreview();
            mSettingDeviceConfigurator.onPreviewStopped();
        }
    }

    @Override
    public void startPreview() {
        if (mCameraProxy != null) {
            LogHelper.i(TAG, "[startPreview]");
            mCameraProxy.startPreview();
            mSettingDeviceConfigurator.onPreviewStarted();
        }
    }

    @Override
    public void takePicture(@Nonnull JpegCallback callback) {
        LogHelper.i(TAG, "[takePicture]");
        mJpegReceivedCallback = callback;
        setJpegRotation();
        mCameraProxy.takePicture(null, null, null, mJpegCallback);
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mJpegRotation = orientation;
    }

    @Override
    public void postRecordingRestriction(List<Relation> relations, boolean isNeedConfigRequest) {
        if (CameraState.CAMERA_OPENED != getCameraState() || mCameraProxy == null) {
            LogHelper.e(TAG, "[postRecordingRestriction] state is not right");
            return;
        }
        for (Relation relation : relations) {
            mSettingManager.getSettingController().postRestriction(relation);
        }
        Parameters parameters = mCameraProxy.getParameters();
        if (parameters != null) {
            mSettingDeviceConfigurator.configParameters(parameters);
            mCameraProxy.setParameters(parameters);
        }
    }

    @Override
    public void startRecording() {
        mIsDuringRecording = true;
        mCanConfigParameter = true;
        mCameraProxy.lock(false);
    }

    @Override
    public void stopRecording() {
        mIsDuringRecording = false;
    }

    @Override
    public CameraProxy getCamera() {
        return mCameraProxy;
    }

    @Override
    public void configCamera(Surface surface, boolean isNeedWaitConfigSession) {

    }

    @Override
    public CameraInfo getCameraInfo(int cameraId) {
        return mInfo[cameraId];
    }

    @Override
    public boolean isVssSupported(int cameraId) {
        if (CameraInfo.CAMERA_FACING_FRONT == mInfo[cameraId].facing) {
            return false;
        } else {
            return mCameraProxy.getOriginalParameters(false).isVideoSnapshotSupported();
        }
    }

    @Override
    public CamcorderProfile getCamcorderProfile() {
        return mProfile;
    }

    @Override
    public void lockCamera() {
        LogHelper.i(TAG, "[lockCamera]");
        mCanConfigParameter = true;
        if (mCameraProxy != null) {
            mCameraProxy.lock(true);
        }
    }

    @Override
    public void unLockCamera() {
        mLockCameraAndRequestSettingsLock.lock();
        LogHelper.i(TAG, "[unLockCamera]");
        if (mCameraProxy != null) {
            try {
                mCanConfigParameter = false;
                mCameraProxy.unlock();
            } finally {
                mLockCameraAndRequestSettingsLock.unlock();
            }
        }
    }

    @Override
    public void release() {
        mLockCameraAndRequestSettingsLock.lock();
        try {
            mVideoHandler.getLooper().quit();
            if (mStatusMonitor != null) {
                LogHelper.d(TAG, "[release] unregisterValueChangedListener");
                mStatusMonitor.unregisterValueChangedListener(KEY_SCENE_MODE, mStatusListener);
            }
            updateCameraState(CameraState.CAMERA_UNKNOWN);
        } finally {
            mLockCameraAndRequestSettingsLock.unlock();
        }
    }

    @Override
    public void preventChangeSettings() {
        mLockCameraAndRequestSettingsLock.lock();
        LogHelper.i(TAG, "[preventChangeSettings]");
        try {
            mCanConfigParameter = false;
        } finally {
            mLockCameraAndRequestSettingsLock.unlock();
        }
    }

    @Override
    public boolean isReadyForCapture() {
        boolean canCapture = mCameraProxy != null && getCameraState() == CameraState.CAMERA_OPENED;
        LogHelper.i(TAG, "[isReadyForCapture] canCapture = " + canCapture);
        return canCapture;
    }

    @Override
    public void requestChangeSettingValue(String key) {
        mLockCameraAndRequestSettingsLock.lock();
        try {
            LogHelper.i(TAG, "[requestChangeSettingValue] key = " + key);
            if (canChangeSettings()) {
                Parameters parameters = getParameters();
                //set recording hint must before configParameters(),because lome effect will modify
                //recording hint.
                parameters.setRecordingHint(true);
                setParameterRotation(parameters);
                if (mProfile != null) {
                    parameters.setPreviewFrameRate(mProfile.videoFrameRate);
                }
                //set preview size must before configParameters(),because lome effect will modify
                //preview size.
                updatePreviewSize(parameters);
                updatePictureSize(parameters);
                boolean isReStartPreview = mSettingDeviceConfigurator.configParameters(parameters);
                if (isReStartPreview && !mIsDuringRecording) {
                    stopPreview();
                    mCameraProxy.setParameters(parameters);
                    mNeedRestartPreview = true;
                    doStartPreview(mSurfaceObject);
                } else if (KEY_VIDEO_QUALITY.equals(key) && !mIsDuringRecording) {
                    mNeedRestartPreview = true;
                    mCameraProxy.stopPreview();
                    mCameraProxy.setParameters(parameters);
                    mSettingDeviceConfigurator.onPreviewStopped();
                    mModeDeviceCallback.afterStopPreview();
                    mSettingConfig.onConfig(mPreviewSize);
                } else {
                    mCameraProxy.setParameters(parameters);
                }
            }
        } finally {
            mLockCameraAndRequestSettingsLock.unlock();
        }
    }

    @Override
    public void requestChangeCommand(String key) {
        if (mCameraState == CameraState.CAMERA_OPENED && mCameraProxy != null) {
            mSettingDeviceConfigurator.configCommand(key, mCameraProxy);
        }
    }

    @Override
    public void requestChangeCommandImmediately(String key) {
        if (mCameraState == CameraState.CAMERA_OPENED && mCameraProxy != null) {
            mSettingDeviceConfigurator.configCommand(key, mCameraProxy);
        }
    }

    /**
     * use to handle some thing for video device control.
     */
    private class VideoDeviceHandler extends Handler {
        /**
         * the construction method.
         */
        VideoDeviceHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage] what = " + msg.what);
            switch (msg.what) {
                case HANDLER_UPDATE_PREVIEW_SURFACE:
                    doUpdatePreviewSurface(msg.obj);
                    break;
                default:
                    break;
            }
        }
    }

    private void waitOpenDoneForClose() {
        if (CameraState.CAMERA_OPENING == getCameraState()) {
            synchronized (mWaitOpenCamera) {
                try {
                    LogHelper.i(TAG, "[waitOpenDoneForClose] wait open camera begin");
                    updateCameraState(CameraState.CAMERA_CLOSING);
                    mWaitOpenCamera.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            LogHelper.i(TAG, "[waitOpenDoneForClose] wait open camera end");
        }
    }

    private void doCloseCamera(boolean sync) {
        if (mCameraProxy != null) {
            mCanConfigParameter = false;
            if (sync) {
                mCameraProxy.close();
            } else {
                mCameraProxy.closeAsync();
            }
        }
    }

    private void releaseVariables() {
        mStatusMonitor = null;
        mCameraProxy = null;
        mCameraId = null;
        mSurfaceObject = null;
        mPreviewSize = null;
    }

    private void doUpdatePreviewSurface(Object surfaceObject) {
        LogHelper.d(TAG, "[doUpdatePreviewSurface] +");
        mSurfaceObject = surfaceObject;
        mLock.lock();
        try {
            if (mCameraProxy != null) {
                doStartPreview(surfaceObject);
            }
        } finally {
            mLock.unlock();
        }
        LogHelper.d(TAG, "[doUpdatePreviewSurface] -");
    }

    private void doStartPreview(Object surfaceObject) {
        LogHelper.d(TAG, "[doStartPreview] surfaceHolder = " + surfaceObject + " state : "
                + mCameraState + " proxy = " + mCameraProxy + " mNeedRestartPreview "
                + mNeedRestartPreview + " mIsDuringRecording = " + mIsDuringRecording);
        boolean isStateReady = CameraState.CAMERA_OPENED == getCameraState();
        if (isStateReady && surfaceObject != null && mCameraProxy != null && !mIsDuringRecording) {
            try {
                mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
                if (surfaceObject instanceof SurfaceHolder) {
                    mCameraProxy.setPreviewDisplay((SurfaceHolder) surfaceObject);
                } else if (surfaceObject instanceof SurfaceTexture) {
                    mCameraProxy.setPreviewTexture((SurfaceTexture) surfaceObject);
                } else if (surfaceObject == null) {
                    mCameraProxy.setPreviewDisplay(null);
                }
                setDisplayOrientation();
                if (mNeedRestartPreview) {
                    mCameraProxy.startPreview();
                    mSettingDeviceConfigurator.onPreviewStarted();
                    mNeedRestartPreview = false;
                }
            } catch (IOException e) {
                throw new RuntimeException("set preview display exception");
            }
        }
    }

    private void initializeCameraInfo() {
        int numberOfCameras = Camera.getNumberOfCameras();
        mInfo = new CameraInfo[numberOfCameras];
        for (int i = 0; i < numberOfCameras; i++) {
            mInfo[i] = new CameraInfo();
            Camera.getCameraInfo(i, mInfo[i]);
            LogHelper.d(TAG, "[initializeCameraInfo] mInfo[" + i + "]= " + mInfo[i]);
        }
    }

    private void setJpegRotation() {
        Camera.Parameters parameters = mCameraProxy.getParameters();
        setParameterRotation(parameters);
        Size thumbnailSize = CameraUtil.getSizeByTargetSize(
                parameters.getSupportedJpegThumbnailSizes(),
                parameters.getPictureSize(), true);
        if (thumbnailSize != null
                && thumbnailSize.getWidth() != 0
                && thumbnailSize.getHeight() != 0) {
            parameters.setJpegThumbnailSize(thumbnailSize.getWidth(),
                    thumbnailSize.getHeight());
        }
        mCameraProxy.setParameters(parameters);
    }

    private void setParameterRotation(Parameters parameter) {
        if (mCameraId != null && mJpegRotation != UN_KNOW_ORIENTATION) {
            int rotation = CameraUtil.getJpegRotation(Integer.parseInt(mCameraId), mJpegRotation);
            parameter.setRotation(rotation);
        }
    }

    private PictureCallback mJpegCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] bytes, Camera camera) {
            LogHelper.d(TAG, "[onPictureTaken]");
            mJpegReceivedCallback.onDataReceived(bytes);
        }
    };

    private void setDisplayOrientation() {
        int displayRotation = CameraUtil.getDisplayRotation(mActivity);
        int displayOrientation
                = CameraUtil.getDisplayOrientation(displayRotation, Integer.parseInt(mCameraId));
        mCameraProxy.setDisplayOrientation(displayOrientation);
        LogHelper.d(TAG, "[setDisplayOrientation] Rotation  = " + displayRotation
                + "displayOrientation" + " = " + displayOrientation);
    }

    private boolean canDoOpenCamera(String newCameraId) {
        boolean value = true;
        boolean isStateError = CameraState.CAMERA_UNKNOWN != getCameraState();
        boolean isSameCamera = (mCameraId != null && newCameraId.equalsIgnoreCase(mCameraId));
        if (isStateError || isSameCamera) {
            value = false;
        }
        LogHelper.d(TAG, "[canDoOpenCamera] mCameraState = " + mCameraState + " new Camera: " +
                newCameraId + " current camera : " + mCameraId + " value = " + value);
        return value;
    }

    private Parameters getParameters() {
        if (mIsDuringRecording) {
            return mCameraProxy.getParameters();
        } else {
            Parameters parameter = mCameraProxy.getOriginalParameters(true);
            if (mPreviewSize != null) {
                parameter.setPreviewSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
            }
            return parameter;
        }
    }

    private boolean canChangeSettings() {
        boolean isCanChange;
        isCanChange = mCameraState == CameraState.CAMERA_OPENED
                && mCameraProxy != null && mCanConfigParameter;
        LogHelper.d(TAG, "[canChangeSettings] mCameraState = " + mCameraState + " mCameraProxy = "
                + mCameraProxy + " mCanConfigParameter = " + mCanConfigParameter
                + " isCanChange = " + isCanChange + " mIsDuringRecording = " + mIsDuringRecording);
        return isCanChange;
    }

    private class CameraDeviceProxyStateCallback extends StateCallback {

        @Override
        public void onOpened(@Nonnull CameraProxy cameraProxy) {
            LogHelper.i(TAG, "[onOpened] + cameraProxy = " + cameraProxy);
            mCameraProxy = cameraProxy;
            if (CameraState.CAMERA_OPENING != getCameraState() || cameraProxy == null) {
                LogHelper.d(TAG, "[onOpened] state = " + mCameraState);
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
                return;
            }
            try {
                doAfterOpenCamera(cameraProxy);
                Parameters parameters = mCameraProxy.getParameters();
                parameters.setRecordingHint(true);
                updateCameraState(CameraState.CAMERA_OPENED);
                mSettingDeviceConfigurator.configParameters(parameters);
                updatePreviewSize(parameters);
                updatePictureSize(parameters);
                mNeedRestartPreview = true;
                parameters.setPreviewFrameRate(mProfile.videoFrameRate);
                mPreviewFormat = parameters.getPreviewFormat();
                mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
                mCameraProxy.setParameters(parameters);
                mSettingConfig.onConfig(mPreviewSize);
            } catch (RuntimeException e) {
                e.printStackTrace();
            } finally {
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
            }
            LogHelper.d(TAG, "[onOpened] - ");
        }

        @Override
        public void onClosed(@Nonnull CameraProxy cameraProxy) {
            LogHelper.d(TAG, "[onClosed] proxy = " + cameraProxy);
            if (mCameraProxy != null && mCameraProxy == cameraProxy) {
                updateCameraState(CameraState.CAMERA_UNKNOWN);
            }
        }

        @Override
        public void onDisconnected(@Nonnull CameraProxy cameraProxy) {
            LogHelper.d(TAG, "[onDisconnected] proxy = " + cameraProxy);
            if (mCameraProxy != null && mCameraProxy == cameraProxy) {
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);
            }

        }

        @Override
        public void onError(@Nonnull CameraProxy cameraProxy, int error) {
            LogHelper.d(TAG, "[onError] proxy = " + cameraProxy + " error = " + error);
            if ((mCameraProxy != null && mCameraProxy == cameraProxy)
                    || error == CameraUtil.CAMERA_OPEN_FAIL) {
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                mModeDeviceCallback.onError();
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
                CameraUtil.showErrorInfoAndFinish(mActivity, error);
            }
        }
    }
    private void doAfterOpenCamera(CameraProxy cameraProxy) {
        mModeDeviceCallback.onCameraOpened(mCameraId);
        mCanConfigParameter = true;
        mICameraContext.getFeatureProvider().updateCameraParameters(
                mCameraId, cameraProxy.getOriginalParameters(false));
        mSettingManager.createAllSettings();
        mSettingDeviceConfigurator.setOriginalParameters(cameraProxy.getOriginalParameters(false));
        mSettingManager.getSettingController().postRestriction(
                mRestrictionProvider.getRestriction());
        mStatusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        mStatusMonitor.registerValueChangedListener(KEY_SCENE_MODE, mStatusListener);
        mSettingManager.getSettingController().addViewEntry();
        mSettingManager.getSettingController().refreshViewEntry();
    }

    private Camera.PreviewCallback mFrameworkPreviewCallback = new Camera.PreviewCallback() {
        @Override
        public void onPreviewFrame(byte[] bytes, Camera camera) {
            if (mPreviewCallback != null) {
                mPreviewCallback.onPreviewCallback(bytes, mPreviewFormat, mCameraId);
                mModeDeviceCallback.onPreviewStart();
            }
        }
    };

    private void initProfile() {
        String stringQuality = parseIntent();
        if (stringQuality == null) {
            stringQuality = mSettingManager.getSettingController().queryValue(KEY_VIDEO_QUALITY);
        }
        if (stringQuality != null && mCameraId != null) {
            int quality = Integer.parseInt(stringQuality) ;
            int cameraId = Integer.parseInt(mCameraId);
            mProfile = CamcorderProfileEx.getProfile(cameraId, quality);
            reviseVideoCapability(mProfile);
        }
        LogHelper.d(TAG, "[initProfile] + cameraId = " + mCameraId + " quality = " + stringQuality);
    }

    private void updatePreviewSize(Camera.Parameters parameters) {
        initProfile();
        mPreviewSize = computeDesiredPreviewSize(mProfile, parameters, mActivity);
        parameters.setPreviewSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
        setVideoSize(parameters);
        LogHelper.d(TAG, "[updatePreviewSize]" + mPreviewSize.toString());
    }

    private void setVideoSize(Camera.Parameters parameters) {
        String size = "" + mProfile.videoFrameWidth + "x" + mProfile.videoFrameHeight;
        parameters.set("video-size", size);
    }

    private void updatePictureSize(Camera.Parameters parameters) {
        if (parameters.isVideoSnapshotSupported()) {
            List<Camera.Size> supported = parameters.getSupportedPictureSizes();
            Camera.Size optimalSize = getOptimalVideoSnapshotPictureSize(supported,
                    (double) mPreviewSize.getWidth() / mPreviewSize.getHeight());
            Camera.Size original = parameters.getPictureSize();
            if (optimalSize != null) {
                if (!original.equals(optimalSize)) {
                    parameters.setPictureSize(optimalSize.width, optimalSize.height);
                }
                LogHelper.d(TAG, "[updatePictureSize]" + optimalSize.toString());
            } else {
                LogHelper.e(TAG, "[updatePictureSize] error optimalSize is null");
            }
        }

    }

    private void reviseVideoCapability(CamcorderProfile profile) {
        LogHelper.d(TAG, "[reviseVideoCapability] + VideoFrameRate = " + profile.videoFrameRate);
        String sceneMode = mSettingManager.getSettingController().queryValue(KEY_SCENE_MODE);
        mRememberSceneModeValue = sceneMode;
        if (Parameters.SCENE_MODE_NIGHT.equals(sceneMode)) {
            profile.videoFrameRate /= 2;
            profile.videoBitRate /= 2;
        }
        LogHelper.d(TAG, "[reviseVideoCapability] - videoFrameRate = " + profile.videoFrameRate);
    }

    private String parseIntent() {
        String quality = null;
        Intent intent = mActivity.getIntent();
        String action = intent.getAction();
        if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            boolean userLimitQuality = intent.hasExtra(MediaStore.EXTRA_VIDEO_QUALITY);
            if (userLimitQuality) {
                int extraVideoQuality = intent.getIntExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0);
                if (extraVideoQuality > 0 &&
                        CamcorderProfile.hasProfile(
                                Integer.parseInt(mCameraId), extraVideoQuality))  {
                    quality = Integer.toString(extraVideoQuality);
                } else {
                    quality = Integer.toString(CamcorderProfile.QUALITY_LOW);
                }
            }
        }
        return quality;
    }
    /**
     * Status change listener implement.
     */
    private class MyStatusChangeListener implements StatusMonitor.StatusChangeListener {
        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.i(TAG, "[onStatusChanged] key = " + key + " value = "
                    + value + " CameraState = " + mCameraState + " mRememberSceneModeValue = "
                    + mRememberSceneModeValue);
            if (KEY_SCENE_MODE.equalsIgnoreCase(key)
                    && CameraState.CAMERA_OPENED == getCameraState()
                    && (Parameters.SCENE_MODE_NIGHT.equals(mRememberSceneModeValue)
                    || Parameters.SCENE_MODE_NIGHT.equals(value))) {
                initProfile();
            }
        }
    }

    private Size computeDesiredPreviewSize(
            CamcorderProfile profile, Camera.Parameters parameters, Activity activity) {
        Camera.Size pre = parameters.getPreviewSize();
        Size previewSize = new Size(pre.width, pre.height);
        if (parameters.getSupportedVideoSizes() == null) { // should be rechecked
            previewSize = new Size(profile.videoFrameWidth, profile.videoFrameHeight);
        } else { // Driver supports separates outputs for preview and video.
            List<Camera.Size> sizes = parameters.getSupportedPreviewSizes();
            boolean isRotate = isDisplayRotateSupported(parameters);
            if (!isRotate) {
                int product = 0;
                Camera.Size preferred = parameters.getPreferredPreviewSizeForVideo();
                product = preferred.width * preferred.height;
                Iterator<Camera.Size> it = sizes.iterator();
                // Remove the preview sizes that are not preferred.
                while (it.hasNext()) {
                    Camera.Size size = it.next();
                    if (size.width * size.height > product) {
                        it.remove();
                    }
                }
            }

            int length = sizes.size();
            List<Size> supportedSize = new ArrayList<>(length);
            for (int i = 0; i < length; i++) {
                supportedSize.add(i, new Size(sizes.get(i).width, sizes.get(i).height));
            }

            Size optimalSize = CameraUtil.getOptimalPreviewSize(activity, supportedSize,
                    (double) profile.videoFrameWidth / profile.videoFrameHeight, isRotate);
            if (optimalSize != null) {
                previewSize = new Size(optimalSize.getWidth(), optimalSize.getHeight());
            } else {
                previewSize = new Size(profile.videoFrameWidth, profile.videoFrameHeight);
            }
        }
        LogHelper.i(TAG, "[computeDesiredPreviewSize] preview size " + previewSize.toString());
        return previewSize;
    }

    private boolean isDisplayRotateSupported(Camera.Parameters parameters) {
        String supported = parameters.get(KEY_DISP_ROT_SUPPORTED);
        return !(supported == null || FALSE.equals(supported));
    }

    // Returns the largest picture size which matches the given aspect ratio.
    private Camera.Size getOptimalVideoSnapshotPictureSize(
            List<Camera.Size> sizes, double targetRatio) {
        if (sizes == null) {
            return null;
        }
        Camera.Size optimalSize = null;
        // Try to find a size matches aspect ratio and has the largest width
        for (Camera.Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            if (optimalSize == null || size.width > optimalSize.width) {
                optimalSize = size;
            }
        }
        // Cannot find one that matches the aspect ratio. This should not
        // happen.
        // Ignore the requirement.
        if (optimalSize == null) {
            for (Camera.Size size : sizes) {
                if (optimalSize == null || size.width > optimalSize.width) {
                    optimalSize = size;
                }
            }
        }
        return optimalSize;
    }

}
