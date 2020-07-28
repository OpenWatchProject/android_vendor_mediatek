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

package com.mediatek.camera.feature.mode.vsdof.video.device;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.media.CamcorderProfile;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
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
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceConfigurator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.mode.vsdof.video.view.SdofVideoQualitySettingView;
import com.mediatek.camera.portability.CamcorderProfileEx;
import com.mediatek.camera.portability.CameraEx;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * An implementation of {@link ISdofDeviceController} with CameraProxy.
 */
public class SdofVideoDeviceController implements ISdofDeviceController,
                                         ISettingManager.SettingDeviceRequester {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(SdofVideoDeviceController.class.getSimpleName());
    private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String FALSE = "false";
    private static final String PROPERTY_KEY_CLIENT_APP_MODE = "vendor.mtk.client.appmode";
    private static final String APP_MODE_NAME_MTK_DUAL_CAMERA = "MtkStereo";

    private static final double ASPECT_TOLERANCE = 0.001;
    private static final int HANDLER_UPDATE_PREVIEW_SURFACE = 1;
    private static final int UN_KNOW_ORIENTATION = -1;
    private static final int WAIT_TIME = 5;

    private volatile CameraState mCameraState = CameraState.CAMERA_UNKNOWN;
    private volatile CameraProxy mCameraProxy;
    private boolean mIsDuringRecording = false;
    private boolean mCanConfigParameter = false;
    private boolean mNeedRestartPreview;
    private int mJpegRotation = -1;
    private int mPreviewFormat;
    private String mLevel;

    private StatusMonitor.StatusChangeListener mStatusListener = new MyStatusChangeListener();
    private StateCallback mCameraProxyStateCallback = new CameraDeviceProxyStateCallback();
    private Object mWaitOpenCamera = new Object();
    private Lock mLockCameraAndRequestSettingsLock = new ReentrantLock();
    private Lock mLockState = new ReentrantLock();
    private Lock mLock = new ReentrantLock();

    private SettingDeviceConfigurator mSettingDeviceConfigurator;
    private DeviceCallback mModeDeviceCallback;
    private CameraDeviceManager mCameraDeviceManager;
    private RestrictionProvider mRestrictionProvider;
    private SettingConfigCallback mSettingConfig;
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
    private HashMap<String, String> mQualityMap = new HashMap<>();

    @Override
    public void requestChangeSettingValue(String key) {
        mLockCameraAndRequestSettingsLock.lock();
        try {
            LogHelper.i(TAG, "[requestChangeSettingValue] key = " + key + " mCameraState = "
                    + mCameraState + " mCanConfigParameter = " + mCanConfigParameter);
            if (mCameraState == CameraState.CAMERA_OPENED
                    && mCameraProxy != null && mCanConfigParameter) {
                Parameters parameters;
                LogHelper.d(TAG, "[requestChangeSettingValue]" +
                        " mIsDuringRecording = " + mIsDuringRecording);
                if (mIsDuringRecording) {
                    parameters = mCameraProxy.getParameters();
                } else {
                    parameters = mCameraProxy.getOriginalParameters(true);
                    if (mPreviewSize != null) {
                        parameters.setPreviewSize(
                                mPreviewSize.getWidth(), mPreviewSize.getHeight());
                    }
                }
                parameters.setRecordingHint(true);
                setParameterRotation(parameters);
                if (mProfile != null) {
                    parameters.setPreviewFrameRate(mProfile.videoFrameRate);
                }
                if (mSettingDeviceConfigurator.configParameters(parameters)) {
                    setVsdofEnable(parameters);
                    stopPreview();
                    mCameraProxy.setParameters(parameters);
                    doStartPreview(mSurfaceObject);
                } else {
                    setVsdofEnable(parameters);
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

    @Override
    public void requestChangeSettingValueJustSelf(String key) {
    }

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
     * SdofVideoSdofDeviceController may use activity to get display rotation.
     *
     * @param activity the camera activity.
     * @param context  the camera context.
     */
    public SdofVideoDeviceController(@Nonnull Activity activity, @Nonnull ICameraContext context) {
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

    @Override
    public void setPreviewCallback(PreviewCallback callback1, DeviceCallback callback2) {
        mPreviewCallback = callback1;
        mModeDeviceCallback = callback2;
        if (mCameraProxy != null && mPreviewCallback == null) {
            mCameraProxy.setPreviewCallback(null);
        }
    }

    /**
     * set a call back for setting config.
     * @param callback the call back.
     */
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
            if (!canDoOpenCamera(cameraId)) {
                LogHelper.e(TAG, "[openCamera] -  condition is not ready return");
                return;
            }
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
        } catch (CameraOpenException e) {
            if (CameraOpenException.ExceptionType.SECURITY_EXCEPTION == e.getExceptionType()) {
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_HARDWARE_EXCEPTION);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            mLock.unlock();
        }
        LogHelper.d(TAG, "[openCamera]-");
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
     *
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
        if (CameraState.CAMERA_UNKNOWN == getCameraState()) {
            LogHelper.e(TAG, "[closeCamera] mCameraState is nuKnow");
            return;
        }

        try {
            mLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
            if (CameraState.CAMERA_OPENING == getCameraState()) {
                synchronized (mWaitOpenCamera) {
                    try {
                        LogHelper.i(TAG, "[closeCamera] wait open camera begin");
                        updateCameraState(CameraState.CAMERA_CLOSING);
                        mWaitOpenCamera.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                LogHelper.i(TAG, "[closeCamera] wait open camera end");
            }
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            if (mStatusMonitor != null) {
                mStatusMonitor.unregisterValueChangedListener(KEY_SCENE_MODE, mStatusListener);
            }
            mModeDeviceCallback.beforeCloseCamera();
            if (mCameraProxy != null) {
                mCanConfigParameter = false;
                if (sync) {
                    mCameraProxy.close();
                } else {
                    mCameraProxy.closeAsync();
                }
            }
            mStatusMonitor = null;
            mCameraProxy = null;
            mCameraId = null;
            mSurfaceObject = null;
            mPreviewSize = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            mLock.unlock();
        }
        LogHelper.d(TAG, "[closeCamera] - mCameraState =" + mCameraState);
    }

    /**
     * lock camera.
     */
    @Override
    public void lockCamera() {
        LogHelper.i(TAG, "[lockCamera]");
        mCanConfigParameter = true;
        if (mCameraProxy != null) {
            mCameraProxy.lock(true);
        }
    }

    /**
     * unlock camera.
     */
    @Override
    public void unLockCamera() {
        LogHelper.i(TAG, "[unLockCamera]");
        if (mCameraProxy != null) {
            mLockCameraAndRequestSettingsLock.lock();
            try {
                mCanConfigParameter = false;
                mCameraProxy.unlock();
            } finally {
                mLockCameraAndRequestSettingsLock.unlock();
            }
        }
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        mVideoHandler.obtainMessage(HANDLER_UPDATE_PREVIEW_SURFACE, surfaceObject)
                .sendToTarget();
    }

    @Override
    public void updateVideoQuality(String videoQuality) {
        String videoQualityOld = mICameraContext.getDataStore().getValue(
                SdofVideoQualitySettingView.KEY_SDOF_VIDEO_QUALITY,
                QUALITY_FHD,
                mICameraContext.getDataStore().getGlobalScope());
        if (videoQuality == null || videoQualityOld.equals(videoQuality)) {
            LogHelper.i(TAG, "[updateVideoQuality] the same video quality");
            return;
        }
        Parameters parameters = mCameraProxy.getOriginalParameters(true);
        updatePreviewSize(parameters);
        mCameraProxy.stopPreview();
        mCameraProxy.setParameters(parameters);
        mSettingDeviceConfigurator.onPreviewStopped();
        mModeDeviceCallback.afterStopPreview();
        mSettingConfig.onConfig(mPreviewSize);
    }

    private void doUpdatePreviewSurface(Object surfaceObject) {
        LogHelper.d(TAG, "[doUpdatePreviewSurface] + surfaceHolder = " + surfaceObject
                + " state : " + mCameraState + " proxy = " + mCameraProxy + " mNeedRestartPreview "
                + mNeedRestartPreview + " mIsDuringRecording =" + mIsDuringRecording);
        mSurfaceObject = surfaceObject;
        boolean isStateReady = CameraState.CAMERA_OPENED == getCameraState();
        if (isStateReady && surfaceObject != null && mCameraProxy != null && !mIsDuringRecording) {
            mLock.lock();
            try {
                synchronized (mCameraProxy) {
                    if (mCameraProxy != null) {
                        setDisplayOrientation();
                        mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
                        if (surfaceObject instanceof SurfaceHolder) {
                            mCameraProxy.setPreviewDisplay((SurfaceHolder) surfaceObject);
                        } else if (surfaceObject instanceof SurfaceTexture) {
                            mCameraProxy.setPreviewTexture((SurfaceTexture) surfaceObject);
                        } else if (surfaceObject == null) {
                            mCameraProxy.setPreviewDisplay(null);
                        }
                        if (mNeedRestartPreview) {
                            mCameraProxy.startPreview();
                            mSettingDeviceConfigurator.onPreviewStarted();
                            mNeedRestartPreview = false;
                        }
                    }
                }
            } catch (IOException e) {
                throw new RuntimeException("set preview display exception");
            } finally {
                mLock.unlock();
            }
        }
        LogHelper.d(TAG, "[doUpdatePreviewSurface] -");
    }

    @Override
    public void stopPreview() {
        if (mCameraProxy != null) {
            LogHelper.i(TAG, "[stopPreview]");
            mCameraProxy.stopPreview();
            mModeDeviceCallback.afterStopPreview();
        }
    }

    @Override
    public void startPreview() {
        if (mCameraProxy != null) {
            LogHelper.i(TAG, "[startPreview]");
            mCameraProxy.startPreview();
        }
    }

    @Override
    public void takePicture(@Nonnull JpegCallback callback) {
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mJpegRotation = orientation;
    }

    @Override
    public void postRecordingRestriction(List<Relation> relations, boolean isNeedConfigRequest) {
        for (Relation relation : relations) {
            mSettingManager.getSettingController().postRestriction(relation);
        }
        Parameters parameters = mCameraProxy.getParameters();
        mSettingDeviceConfigurator.configParameters(parameters);
        setVsdofEnable(parameters);
        mCameraProxy.setParameters(parameters);
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
    public void release() {
        mVideoHandler.getLooper().quit();
        if (mStatusMonitor != null) {
            LogHelper.d(TAG, "[release] unregisterValueChangedListener");
            mStatusMonitor.unregisterValueChangedListener(KEY_SCENE_MODE, mStatusListener);
        }
        updateCameraState(CameraState.CAMERA_UNKNOWN);
    }

    @Override
    public void setVsDofLevelParameter(String level) {
        mLevel = level;
        if (mCameraProxy != null) {
            Camera.Parameters parameters = mCameraProxy.getParameters();
            parameters.set(KEY_VS_DOF_LEVEL, level);
            mCameraProxy.setParameters(parameters);
        }
    }

    @Override
    public void onFirstFrameAvailable() {
        if (mModeDeviceCallback != null) {
            mModeDeviceCallback.onPreviewStart();
        }
    }

    private void initializeCameraInfo() {
        int numberOfCameras = Camera.getNumberOfCameras();
        mInfo = new CameraInfo[numberOfCameras];
        LogHelper.d(TAG, "[initializeCameraInfo] mNumberOfCameras = " + numberOfCameras);
        for (int i = 0; i < numberOfCameras; i++) {
            mInfo[i] = new CameraInfo();
            Camera.getCameraInfo(i, mInfo[i]);
            LogHelper.d(TAG, "[initializeCameraInfo] mInfo[" + i + "]= " + mInfo[i]);
        }
    }

    private void setParameterRotation(Parameters parameter) {
        if (mCameraId != null && mJpegRotation != UN_KNOW_ORIENTATION) {
            int rotation = CameraUtil.getJpegRotation(
                    Integer.parseInt(mCameraId), mJpegRotation);
            parameter.setRotation(rotation);
        }
    }

    private void doStartPreview(Object surfaceObject) {
        LogHelper.d(TAG, "[doStartPreview] state :" + mCameraState);
        if (mCameraProxy == null || surfaceObject == null) {
            LogHelper.d(TAG, "[doStartPreview] surfaceHolder = "
                    + surfaceObject + " mCameraProxy = " + mCameraProxy);
            return;
        }
        try {
            if (surfaceObject instanceof SurfaceHolder) {
                mCameraProxy.setPreviewDisplay((SurfaceHolder) surfaceObject);
            } else if (surfaceObject instanceof SurfaceTexture) {
                mCameraProxy.setPreviewTexture((SurfaceTexture) surfaceObject);
            } else if (surfaceObject == null) {
                mCameraProxy.setPreviewDisplay(null);
            }
            setDisplayOrientation();
            mCameraProxy.startPreview();
            mSettingDeviceConfigurator.onPreviewStarted();
            mNeedRestartPreview = false;
        } catch (IOException e) {
            throw new RuntimeException("set preview display exception");
        }
    }

    private void setDisplayOrientation() {
        int displayRotation = CameraUtil.getDisplayRotation(mActivity);
        int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation, Integer
                .parseInt(mCameraId));
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

    /**
     * Open camera device state callback, this callback is send to camera device manager
     * by open camera interface.
     */
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
                mCanConfigParameter = true;
                mICameraContext.getFeatureProvider().updateCameraParameters(mCameraId,
                        cameraProxy.getOriginalParameters(false));
                mSettingManager.createAllSettings();
                mSettingDeviceConfigurator.setOriginalParameters(
                        cameraProxy.getOriginalParameters(false));
                mSettingManager.getSettingController().postRestriction(
                        mRestrictionProvider.getRestriction());
                mModeDeviceCallback.onCameraOpened(mCameraId);
                mStatusMonitor = mICameraContext.getStatusMonitor(mCameraId);
                mStatusMonitor.registerValueChangedListener(KEY_SCENE_MODE, mStatusListener);
                mSettingManager.getSettingController().addViewEntry();
                mSettingManager.getSettingController().refreshViewEntry();
                Parameters parameters = mCameraProxy.getParameters();
                parameters.setRecordingHint(true);
                updateCameraState(CameraState.CAMERA_OPENED);
                mSettingDeviceConfigurator.configParameters(parameters);
                initVideoQualitys();
                updatePreviewSize(parameters);
                mNeedRestartPreview = true;
                parameters.setPreviewFrameRate(mProfile.videoFrameRate);
                mPreviewFormat = parameters.getPreviewFormat();
                mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
                setVsdofEnable(parameters);
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
        public void onRetry() {
            LogHelper.i(TAG, "[onRetry]");
            CameraEx.setProperty(PROPERTY_KEY_CLIENT_APP_MODE,
                    APP_MODE_NAME_MTK_DUAL_CAMERA);
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
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            synchronized (mWaitOpenCamera) {
                mWaitOpenCamera.notifyAll();
            }
            CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);

        }

        @Override
        public void onError(@Nonnull CameraProxy cameraProxy, int error) {
            LogHelper.d(TAG, "[onError] proxy = " + cameraProxy + " error = " + error);
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            synchronized (mWaitOpenCamera) {
                mWaitOpenCamera.notifyAll();
            }
            CameraUtil.showErrorInfoAndFinish(mActivity, error);
        }
    }

    private Camera.PreviewCallback mFrameworkPreviewCallback = new Camera.PreviewCallback() {
        @Override
        public void onPreviewFrame(byte[] bytes, Camera camera) {
            if (mPreviewCallback != null) {
                mPreviewCallback.onPreviewCallback(bytes, mPreviewFormat, mCameraId);
            }
        }
    };

    private void initProfile() {
        // find default video quality
        String videoQuality = mICameraContext.getDataStore().getValue(
                SdofVideoQualitySettingView.KEY_SDOF_VIDEO_QUALITY,
                QUALITY_FHD,
                mICameraContext.getDataStore().getGlobalScope());
        if (videoQuality == null || mCameraId == null) {
            return;
        }
        int quality = Integer.parseInt(mQualityMap.get(videoQuality));
        int cameraId = Integer.parseInt(mCameraId);
        LogHelper.d(TAG, "[initProfile] cameraId = " + mCameraId + " quality = " + quality);
        mProfile = CamcorderProfileEx.getProfile(cameraId, quality);
        reviseVideoCapability(mProfile);
    }

    private void initVideoQualitys() {
        if (!mQualityMap.containsKey(QUALITY_FHD)) {
            mQualityMap.put(QUALITY_FHD, String.valueOf(VSDOF_QUALITY_HIGH));
        }
    }

    private boolean updatePreviewSize(Parameters parameters) {
        boolean isChanged = false;
        initProfile();
        Size oldSize = mPreviewSize;
        mPreviewSize = computeDesiredPreviewSize(mProfile, parameters, mActivity);
        parameters.setPreviewSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
        if (mPreviewSize != null && !mPreviewSize.equals(oldSize)) {
            isChanged = true;
            mNeedRestartPreview = true;
        }
        LogHelper.d(TAG, "[updatePreviewSize]" + mPreviewSize.toString() + " oldSize = " + oldSize);
        return isChanged;
    }

    private void setVsdofEnable(Camera.Parameters params) {
        params.set(KEY_IMAGE_REFOCUS_MODE, "on");
        params.set(KEY_VSDOF_MODE, "on");
        params.set(KEY_STEREO_DENOISE_MODE, "off");
        if (mLevel != null) {
            params.set(KEY_VS_DOF_LEVEL, mLevel);
        }
    }

    private Size computeDesiredPreviewSize(
            CamcorderProfile profile, Parameters parameters, Activity activity) {
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

    private boolean isDisplayRotateSupported(Parameters parameters) {
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
}
