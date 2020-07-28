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

package com.mediatek.camera.common.mode.video.device.v2;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.CamcorderProfile;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Range;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy.StateCallback;
import com.mediatek.camera.common.mode.video.device.IDeviceController;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.relation.StatusMonitor.StatusChangeListener;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.portability.CamcorderProfileEx;
import com.mediatek.camera.portability.AdvancedCamera;

import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * the implement for IDeviceController for api2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class VideoDevice2Controller implements IDeviceController,
        CaptureSurface.ImageCallback, ISettingManager.SettingDevice2Requester {

    private static final LogUtil.Tag TAG = new LogUtil.Tag(VideoDevice2Controller.class
            .getSimpleName());
    private static final String KEY_MATRIX_DISPLAY = "key_matrix_display_show";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final double ASPECT_TOLERANCE = 0.001;
    private static final int CAPTURE_FORMAT = ImageFormat.JPEG;
    private static final int HANDLER_UPDATE_PREVIEW_SURFACE = 1;
    private static final int CAPTURE_MAX_NUMBER = 2;
    private static final int WAIT_TIME = 5;
    // add for eis and maybe other features which need know whether is recording or not
    private static final int STREAMING_FEATURE_STATE_PREVIEW = 0;
    private static final int STREAMING_FEATURE_STATE_RECORDING = 1;
    private static final String AVAILABLE_RECORD_STATES
            = "com.mediatek.streamingfeature.availableRecordStates";
    private static final String RECORD_STATE_REQUEST
            = "com.mediatek.streamingfeature.recordState";
    private CaptureRequest.Key<int[]> mRecordStateKey;
    private CameraCharacteristics.Key<int[]> mAvailableRecordStates;

    private boolean mIsRecorderSurfaceConfigured = false;
    private boolean mIsMatrixDisplayShow = false;
    private boolean mNeedRConfigSession = false;
    private boolean mFirstFrameArrived = false;
    private boolean mIsRecording = false;
    private int mJpegRotation;

    private StateCallback mDeviceCallback = new DeviceStateCallback();
    private CameraState mCameraState = CameraState.CAMERA_UNKNOWN;
    private Object mPreviewSurfaceSync = new Object();
    private Object mWaitOpenCamera = new Object();
    private Lock mDeviceLock = new ReentrantLock();
    private Lock mLockState = new ReentrantLock();
    CaptureRequest.Builder mBuilder = null;

    private StatusChangeListener mStatusChangeListener = new MyStatusChangeListener();
    private ISettingManager.SettingDevice2Configurator mSettingDevice2Configurator;
    private CameraCharacteristics mCameraCharacteristics;
    private DeviceCallback mModeDeviceCallback;
    private CameraDeviceManager mCameraDeviceManager;
    private RestrictionProvider mRestrictionProvider;
    private SettingConfigCallback mSettingConfig;
    private Camera2CaptureSessionProxy mSession;
    private PreviewCallback mPreviewCallback;
    private VideoDeviceHandler mVideoHandler;
    private ISettingManager mSettingManager;
    private CaptureSurface mCaptureSurface;
    private ICameraContext mICameraContext;
    private StatusMonitor mStatusMonitor;
    private CameraManager mCameraManager;
    private Camera2Proxy mCamera2Proxy;
    private JpegCallback mJpegCallback;
    private CamcorderProfile mProfile;
    private Surface mPreviewSurface;
    private Handler mSessionHandler;
    private Surface mRecordSurface;
    private Activity mActivity;
    private String mCameraId;

    /**
     * this enum is used for tag native camera open state.
     */
    private enum CameraState {
        CAMERA_UNKNOWN,
        CAMERA_OPENING,
        CAMERA_OPENED,
        CAMERA_CLOSING,
    }
    /**
     * VideoDevice2Controller may use activity to get display rotation.
     * @param activity the camera activity.
     * @param context the camera context.
     */
    public VideoDevice2Controller(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        LogHelper.d(TAG, "[VideoDevice2Controller] Construct");
        mActivity = activity;
        mICameraContext = context;
        mCaptureSurface = new CaptureSurface();
        mCaptureSurface.setCaptureCallback(this);
        initDeviceHandler();
        prepareSessionHandler();
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void createAndChangeRepeatingRequest() {
        try {
            if (mSession != null) {
                synchronized (mSession) {
                    if (mSession != null) {
                        CaptureRequest.Builder builder = doCreateAndConfigRequest(mIsRecording);
                        setRepeatingRequest(builder);
                    }
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public CaptureRequest.Builder createAndConfigRequest(int templateType) {
        CaptureRequest.Builder builder = null;
        try {
            builder = doCreateAndConfigRequest(mIsRecording);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return builder;
    }

    @Override
    public Camera2CaptureSessionProxy getCurrentCaptureSession() {
        return mSession;
    }

    @Override
    public void requestRestartSession() {
        try {
            abortOldSession();
            updatePictureSize();
            mNeedRConfigSession = true;
            updatePreviewSize();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public int getRepeatingTemplateType() {
        return Camera2Proxy.TEMPLATE_RECORD;
    }

    @Override
    public void openCamera(@Nonnull ISettingManager settingManager, @Nonnull String cameraId,
                           boolean sync, RestrictionProvider relation) {
        LogHelper.i(TAG, "[openCamera] + cameraId : " + cameraId + "sync = " + sync);
        mRestrictionProvider = relation;
        if (CameraState.CAMERA_UNKNOWN != getCameraState() ||
                (mCameraId != null && cameraId.equalsIgnoreCase(mCameraId))) {
            LogHelper.e(TAG, "[openCamera] mCameraState = " + mCameraState);
            return;
        }
        updateCameraState(CameraState.CAMERA_OPENING);
        mCameraId = cameraId;
        initSettingManager(settingManager);
        try {
            mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
            initDeviceInfo();
            initSettings();
            doOpenCamera(sync);
        } catch (CameraOpenException | InterruptedException e) {
            e.printStackTrace();
        } finally {
            mDeviceLock.unlock();
        }
        LogHelper.i(TAG, "[openCamera] - ");
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        if (surfaceObject != null) {
            if (surfaceObject instanceof SurfaceHolder) {
                mPreviewSurface = surfaceObject == null ? null :
                        ((SurfaceHolder) surfaceObject).getSurface();
            } else if (surfaceObject instanceof SurfaceTexture) {
                mPreviewSurface = surfaceObject == null ? null :
                        new Surface((SurfaceTexture) surfaceObject);
            }
            mVideoHandler.sendEmptyMessage(HANDLER_UPDATE_PREVIEW_SURFACE);
        } else {
            mPreviewSurface = null;
        }
    }

    private void doUpdatePreviewSurface() {
        LogHelper.d(TAG, "[doUpdatePreviewSurface] mPreviewSurface = " + mPreviewSurface
                + " state = " + mCameraState + " mNeedRConfigSession = " + mNeedRConfigSession
                + " mRecordSurface = " + mRecordSurface);
        synchronized (mPreviewSurfaceSync) {
            if (CameraState.CAMERA_OPENED == getCameraState() && mPreviewSurface != null
                    && mNeedRConfigSession && mIsRecorderSurfaceConfigured) {
                configureSession();
                mNeedRConfigSession = false;
            }
        }
    }

    @Override
    public void stopPreview() {
        abortOldSession();
    }

    @Override
    public void startPreview() {

    }

    @Override
    public void takePicture(@Nonnull JpegCallback callback) {
        LogHelper.e(TAG, "[takePicture] +");
        mJpegCallback = callback;
        CaptureRequest.Builder builder;
        try {
            builder = mCamera2Proxy.createCaptureRequest(Camera2Proxy.TEMPLATE_VIDEO_SNAPSHOT);
            builder.addTarget(mPreviewSurface);
            builder.addTarget(mRecordSurface);
            builder.addTarget(mCaptureSurface.getSurface());
            int rotation = CameraUtil.getJpegRotation(Integer.parseInt(mCameraId), mJpegRotation);
            builder.set(CaptureRequest.JPEG_ORIENTATION, rotation);
            mSettingDevice2Configurator.configCaptureRequest(builder);
            mSession.capture(builder.build(), null, mSessionHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.e(TAG, "[takePicture] -");
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mJpegRotation = orientation;
    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.i(TAG, "[closeCamera] + sync = " + sync + " current state : " + mCameraState);
        if (CameraState.CAMERA_UNKNOWN != mCameraState) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                waitOpenDoneForClose();
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                abortOldSession();
                mModeDeviceCallback.beforeCloseCamera();
                doCloseCamera(sync);
                recycleVariables();
                mCaptureSurface.releaseCaptureSurface();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                mDeviceLock.unlock();
            }
            releaseVariables();
        }
        mCameraId = null;
        LogHelper.i(TAG, "[closeCamera] - ");
    }

    @Override
    public void lockCamera() {

    }

    @Override
    public void unLockCamera() {

    }

    @Override
    public void startRecording() {
        LogHelper.i(TAG, "[startRecording] + ");
        mIsRecording = true;
        mICameraContext.getSoundPlayback().play(ISoundPlayback.START_VIDEO_RECORDING);
        try {
            CaptureRequest.Builder builder = doCreateAndConfigRequest(true);
            setRepeatingRequest(builder);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[startRecording] - ");
    }

    @Override
    public void stopRecording() {
        LogHelper.i(TAG, "[stopRecording] +");
        mICameraContext.getSoundPlayback().play(ISoundPlayback.STOP_VIDEO_RECORDING);
        setStopRecordingToCamera();
        mIsRecording = false;
        LogHelper.d(TAG, "[stopRecording] -");
    }

    @Override
    public CameraProxy getCamera() {
        return null;
    }

    @Override
    public void configCamera(Surface surface, boolean isNeedWaitConfigSession) {
        LogHelper.i(TAG, "[configCamera] + ");
        if (surface != null && !surface.equals(mRecordSurface)) {
            mNeedRConfigSession = true;
        }
        mRecordSurface = surface;
        mIsRecorderSurfaceConfigured = true;
        mVideoHandler.sendEmptyMessage(HANDLER_UPDATE_PREVIEW_SURFACE);
        if (isNeedWaitConfigSession && mNeedRConfigSession) {
            synchronized (mPreviewSurfaceSync) {
                try {
                    LogHelper.d(TAG, "[configCamera] wait config session + ");
                    mPreviewSurfaceSync.wait();
                    LogHelper.d(TAG, "[configCamera] wait config session - ");
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
        LogHelper.i(TAG, "[configCamera] - ");
    }

    @Override
    public void setPreviewCallback(PreviewCallback callback1, DeviceCallback callback2) {
        mPreviewCallback = callback1;
        mModeDeviceCallback = callback2;
    }

    @Override
    public void setSettingConfigCallback(SettingConfigCallback callback) {
        mSettingConfig = callback;
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public Camera.CameraInfo getCameraInfo(int cameraId) {
        return null;
    }

    @Override
    public boolean isVssSupported(int cameraId) {
        return true;
    }

    @Override
    public CamcorderProfile getCamcorderProfile() {
        if (mProfile == null) {
            initProfile();
        }
        return mProfile;
    }

    @Override
    public void release() {
        mVideoHandler.getLooper().quit();
        if (mStatusMonitor != null) {
            mStatusMonitor.unregisterValueChangedListener(KEY_SCENE_MODE, mStatusChangeListener);
            mStatusMonitor.unregisterValueChangedListener(
                    KEY_MATRIX_DISPLAY, mStatusChangeListener);
        }
        if (mSessionHandler != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                mSessionHandler.getLooper().quitSafely();
            } else {
                mSessionHandler.getLooper().quit();
            }
        }
        if (mCaptureSurface != null) {
            mCaptureSurface.release();
        }
        updateCameraState(CameraState.CAMERA_UNKNOWN);
    }

    @Override
    public void preventChangeSettings() {

    }

    @Override
    public boolean isReadyForCapture() {
        boolean canCapture = mCamera2Proxy != null && getCameraState() == CameraState.CAMERA_OPENED;
        LogHelper.i(TAG, "[isReadyForCapture] canCapture = " + canCapture);
        return canCapture;
    }

    @Override
    public void onPictureCallback(byte[] data) {
        LogHelper.i(TAG, "[onPictureCallback]");
        if (mJpegCallback != null) {
            mJpegCallback.onDataReceived(data);
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
                    doUpdatePreviewSurface();
                    break;
                default:
                    break;
            }
        }
    }

    private void initDeviceHandler() {
        HandlerThread ht = new HandlerThread("Video Device2 Handler Thread");
        ht.start();
        mVideoHandler = new VideoDeviceHandler(ht.getLooper());
    }

    private void initSettingManager(ISettingManager settingManager) {
        mSettingManager = settingManager;
        mSettingManager.updateModeDevice2Requester(this);
        mSettingDevice2Configurator = settingManager.getSettingDevice2Configurator();
        mStatusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        mStatusMonitor.registerValueChangedListener(KEY_SCENE_MODE, mStatusChangeListener);
        mStatusMonitor.registerValueChangedListener(KEY_MATRIX_DISPLAY, mStatusChangeListener);
    }

    private void initSettings() {
        mSettingManager.createAllSettings();
        mSettingDevice2Configurator.setCameraCharacteristics(mCameraCharacteristics);
        mSettingManager.getSettingController().postRestriction(
                mRestrictionProvider.getRestriction());
        mSettingManager.getSettingController().addViewEntry();
        mSettingManager.getSettingController().refreshViewEntry();
    }

    private void doOpenCamera(boolean sync) throws CameraOpenException {
        if (sync) {
            mCameraDeviceManager.openCameraSync(mCameraId, mDeviceCallback, null);
        } else {
            mCameraDeviceManager.openCamera(mCameraId, mDeviceCallback, null);
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

    private void recycleVariables() {
        if (mStatusMonitor != null) {
            mStatusMonitor.unregisterValueChangedListener(KEY_SCENE_MODE, mStatusChangeListener);
            mStatusMonitor.unregisterValueChangedListener(
                    KEY_MATRIX_DISPLAY, mStatusChangeListener);
        }
        mIsMatrixDisplayShow = false;
    }

    private void doCloseCamera(boolean sync) {
        if (mCamera2Proxy != null) {
            if (sync) {
                mCamera2Proxy.close();
            } else {
                mCamera2Proxy.closeAsync();
            }
        }
    }

    private void releaseVariables() {
        mIsRecorderSurfaceConfigured = false;
        mCameraId = null;
        mStatusMonitor = null;
        mRecordSurface = null;
        mPreviewSurface = null;
        mCamera2Proxy = null;
        mIsRecorderSurfaceConfigured = false;
    }

    private void configAeFpsRange(CaptureRequest.Builder requestBuilder,
                                    CamcorderProfile profile) {
        LogHelper.d(TAG, "[configAeFpsRange] + ");
        try {
            Range<Integer>[] a = mCameraCharacteristics.get(
                    CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
            int low = profile.videoFrameRate;
            for (int i = 0; i < a.length; i++) {
                if (a[i].contains(profile.videoFrameRate) && a[i].getLower() <= low) {
                    low = a[i].getLower();
                }
            }
            Range aeFps = new Range(low, profile.videoFrameRate);
            requestBuilder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, aeFps);
            LogHelper.i(TAG, "[configAeFpsRange] - " + aeFps.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void prepareSessionHandler() {
        HandlerThread mSessionHandlerThread = new HandlerThread("camera_session");
        mSessionHandlerThread.start();
        mSessionHandler = new Handler(mSessionHandlerThread.getLooper());
    }

    private void updateCameraState(CameraState state) {
        LogHelper.d(TAG, "[updateCameraState] new state = " + state + " old =" + mCameraState);
        mLockState.lock();
        try {
            mCameraState = state;
        } finally {
            mLockState.unlock();
        }
    }

    private CameraState getCameraState() {
        mLockState.lock();
        try {
            return mCameraState;
        } finally {
            mLockState.unlock();
        }
    }

    @Override
    public void postRecordingRestriction(List<Relation> relations , boolean isNeedConfigRequest) {
        if (CameraState.CAMERA_OPENED != getCameraState() || mCamera2Proxy == null) {
            LogHelper.e(TAG, "[postRecordingRestriction] state is not right");
            return;
        }
        for (Relation relation : relations) {
            mSettingManager.getSettingController().postRestriction(relation);
        }
        if (isNeedConfigRequest) {
            repeatingPreview();
        }
    }

    private class DeviceStateCallback extends StateCallback {

        @Override
        public void onOpened(@Nonnull Camera2Proxy camera2proxy) {
            LogHelper.i(TAG, "[onOpened] + camera2proxy = " + camera2proxy);
            try {
                mCamera2Proxy = camera2proxy;
                if (CameraState.CAMERA_OPENING == getCameraState() && camera2proxy != null) {
                    mModeDeviceCallback.onCameraOpened(mCameraId);
                    updateCameraState(CameraState.CAMERA_OPENED);
                    updatePictureSize();
                    mNeedRConfigSession = true;
                    updatePreviewSize();
                }
            } catch (CameraAccessException | RuntimeException e) {
                e.printStackTrace();
            } finally {
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
            }
            LogHelper.d(TAG, "[onOpened] -");
        }

        @Override
        public void onDisconnected(@Nonnull Camera2Proxy camera2proxy) {
            LogHelper.i(TAG, "[onDisconnected] camera2proxy = " + camera2proxy);
            if (mCamera2Proxy != null && mCamera2Proxy == camera2proxy) {
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);
            }
        }

        @Override
        public void onError(@Nonnull Camera2Proxy camera2Proxy, int error) {
            LogHelper.i(TAG, "[onError] camera2proxy = " + camera2Proxy + " error = " + error);
            if ((mCamera2Proxy != null && mCamera2Proxy == camera2Proxy)
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

    private void abortOldSession() {
        LogHelper.i(TAG, "[abortOldSession] + ");
        if (mSession != null) {
            synchronized (mSession) {
                if (mSession != null) {
                    try {
                        mSession.abortCaptures();
                        mSession.close();
                        mSession = null;
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        LogHelper.d(TAG, "[abortOldSession] - ");
    }

    private void configureSession() {
        LogHelper.i(TAG, "[configureSession] + ");
        abortOldSession();
        List<Surface> surfaces = new LinkedList<>();
        surfaces.add(mPreviewSurface);
        Surface capture = mCaptureSurface.getSurface();
        if (capture != null) {
            surfaces.add(capture);
        }
        if (mRecordSurface != null) {
            surfaces.add(mRecordSurface);
        }
        try {
            preSetSession();
            mSettingDevice2Configurator.configSessionSurface(surfaces);
            mCamera2Proxy.createCaptureSession(surfaces, mSessionCallback, mSessionHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[configureSession] - ");
    }

    private void preSetSession() {
        try {
            AdvancedCamera.preSetSession(
                    Integer.parseInt(mCameraId), doCreateAndConfigRequest(false).build());
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private final Camera2CaptureSessionProxy.StateCallback mSessionCallback = new
            Camera2CaptureSessionProxy.StateCallback() {

                @Override
                public void onConfigured(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigured] + session = " + session
                        + ", mCameraState = " + mCameraState);
                    mDeviceLock.lock();
                    try {
                        if (CameraState.CAMERA_OPENED == getCameraState()) {
                            mSession = session;
                            synchronized (mPreviewSurfaceSync) {
                                if (mPreviewSurface != null && !mIsRecording) {
                                    repeatingPreview();
                                }
                                mPreviewSurfaceSync.notify();
                            }
                            if (!mIsMatrixDisplayShow) {
                                mModeDeviceCallback.onPreviewStart();
                            }
                        }
                    } finally {
                        mDeviceLock.unlock();
                    }
                    LogHelper.d(TAG, "[onConfigured] -");
                }

                @Override
                public void onConfigureFailed(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigureFailed] session = " + session);
                    if (mSession == session) {
                        mSession = null;
                    }
                    synchronized (mPreviewSurfaceSync) {
                        mPreviewSurfaceSync.notify();
                    }
                }
            };

    private void initDeviceInfo() {
        try {
            mCameraManager = (CameraManager) mActivity.getSystemService(Context.CAMERA_SERVICE);
            mCameraCharacteristics = mCameraManager.getCameraCharacteristics(mCameraId);
            initRecordStateKey();
        } catch (RuntimeException e) {
            e.printStackTrace();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void setRepeatingRequest(CaptureRequest.Builder builder) {
        if (mSession != null) {
            synchronized (mSession) {
                if (mSession != null) {
                    try {
                        mSession.setRepeatingRequest(builder.build(),
                                mPreviewCapProgressCallback, null);
                    } catch (CameraAccessException e) {
                        LogHelper.e(TAG, "[setRepeatingBurst] fail");
                        e.printStackTrace();
                    }
                } else {
                    LogHelper.e(TAG, "[setRepeatingBurst] mSession is null");
                }
            }
        }
    }

    private void repeatingPreview() {
        LogHelper.i(TAG, "[repeatingPreview] + ");
        try {
            mFirstFrameArrived = false;
            CaptureRequest.Builder builder = doCreateAndConfigRequest(false);
            setRepeatingRequest(builder);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[repeatingPreview] - ");
    }

    private CaptureCallback mPreviewCapProgressCallback = new CaptureCallback() {
                @Override
                public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                                               TotalCaptureResult result) {
                    super.onCaptureCompleted(session, request, result);
                    mSettingDevice2Configurator.getRepeatingCaptureCallback().onCaptureCompleted(
                            session, request, result);
                    if (mPreviewCallback != null && !mFirstFrameArrived && mCameraId != null) {
                        mFirstFrameArrived = true;
                        mPreviewCallback.onPreviewCallback(null, 0, mCameraId);
                    }
                }
            };

    private void initProfile() {
        int cameraId = Integer.parseInt(mCameraId);
        int quality = Integer.parseInt(
                mSettingManager.getSettingController().queryValue("key_video_quality"));
        LogHelper.i(TAG, "[initProfile] + cameraId = " + cameraId + " quality = " + quality);
        mProfile = CamcorderProfileEx.getProfile(cameraId, quality);
        reviseVideoCapability();
    }

    private void updatePreviewSize() throws CameraAccessException {
        Size size = getSupportedPreviewSizes(
                (double) mProfile.videoFrameWidth / mProfile.videoFrameHeight);
        mSettingConfig.onConfig(new com.mediatek.camera.common.utils.Size(
                size.getWidth(), size.getHeight()));
    }

    private void updatePictureSize() {
        initProfile();
        StreamConfigurationMap configurationMap =
                mCameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        android.util.Size[] highSizes = null;
        Size[] result = null;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            highSizes = configurationMap.getHighResolutionOutputSizes(CAPTURE_FORMAT);
        }
        if (highSizes == null) {
            highSizes = configurationMap.getOutputSizes(CAPTURE_FORMAT);
        }
        if (highSizes != null) {
            result = new Size[highSizes.length];
            for (int i = 0; i < highSizes.length; i++) {
                result[i] = new Size(highSizes[i].getWidth(), highSizes[i].getHeight());
            }
        }
        Size pictureSize = getOptimalVideoSnapshotPictureSize(
                result, (double) mProfile.videoFrameWidth / mProfile.videoFrameHeight);
        if (pictureSize != null) {
            mCaptureSurface.updatePictureInfo(pictureSize.getWidth(),
                    pictureSize.getHeight(), CAPTURE_FORMAT, CAPTURE_MAX_NUMBER);
            LogHelper.i(TAG, "[updatePictureSize] pictureSize = " + pictureSize.toString());
        } else {
            mCaptureSurface.updatePictureInfo(mProfile.videoFrameWidth,
                    mProfile.videoFrameHeight, CAPTURE_FORMAT, CAPTURE_MAX_NUMBER);
            LogHelper.e(TAG, "[updatePictureSize] pictureSize is null");
        }
    }

    // Returns the largest picture size which matches the given aspect ratio.
    private static Size getOptimalVideoSnapshotPictureSize(Size[] sizes, double targetRatio) {
        // Use a very small tolerance because we want an exact match.
        // final double ASPECT_TOLERANCE = 0.003;
        if (sizes == null) {
            return null;
        }
        Size optimalSize = null;
        // Try to find a size matches aspect ratio and has the largest width
        for (Size size : sizes) {
            double ratio = (double) size.getWidth() / size.getHeight();
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            if (optimalSize == null || size.getWidth() > optimalSize.getHeight()) {
                optimalSize = size;
            }
        }
        return optimalSize;
    }

    private CaptureRequest.Builder doCreateAndConfigRequest(boolean isRecording)
            throws CameraAccessException {
        CaptureRequest.Builder builder = null;
        if (mCamera2Proxy != null) {
            builder = mCamera2Proxy.createCaptureRequest(Camera2Proxy.TEMPLATE_RECORD);
            configAeFpsRange(builder, mProfile);
            builder.addTarget(mPreviewSurface);
            if (isRecording) {
                builder.addTarget(mRecordSurface);
            }
            mSettingDevice2Configurator.configCaptureRequest(builder);
        }
        mBuilder = builder;
        return builder;
    }

    private Size getSupportedPreviewSizes(double ratio) throws CameraAccessException {
        Size[] rawSizes = getSupportedSizeForClass(android.view.SurfaceHolder.class);
        List<Size> sizes = new ArrayList<Size>();
        for (Size sz: rawSizes) {
            sizes.add(sz);
        }
        Size values = CameraUtil.getOptimalPreviewSize(mActivity, sizes, ratio, true);
        LogHelper.d(TAG, "[getSupportedPreviewSizes] values = " + values.toString());
        return values;
    }

    private Size[] getSupportedSizeForClass(Class klass) throws CameraAccessException {
        StreamConfigurationMap configMap =
                mCameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        android.util.Size[] availableSizes = configMap.getOutputSizes(klass);
        Size[] result = new Size[availableSizes.length];
        for (int i = 0; i < availableSizes.length; i++) {
            result[i] = new Size(availableSizes[i].getWidth(), availableSizes[i].getHeight());
        }
        return result;
    }

    private void reviseVideoCapability() {
        LogHelper.d(TAG, "[reviseVideoCapability] + videoFrameRate = " + mProfile.videoFrameRate);
        String sceneMode = mSettingManager.getSettingController().queryValue(KEY_SCENE_MODE);
        if (Camera.Parameters.SCENE_MODE_NIGHT.equals(sceneMode)) {
            mProfile.videoFrameRate /= 2;
            mProfile.videoBitRate /= 2;
        }
        LogHelper.d(TAG, "[reviseVideoCapability] - videoFrameRate = " + mProfile.videoFrameRate);
    }
    /**
     * Status change listener implement.
     */
    private class MyStatusChangeListener implements StatusChangeListener {
        /**
         * Callback when feature value is changed.
         * @param key The string used to indicate value changed feature.
         * @param value The changed value of feature.
         */
        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.i(TAG, "[onStatusChanged] key = " + key
                    + "value = " + value + " mCameraState = " + getCameraState());
            if (KEY_SCENE_MODE.equalsIgnoreCase(key)
                    && CameraState.CAMERA_OPENED == getCameraState()) {
                initProfile();
            } else if (KEY_MATRIX_DISPLAY.equals(key)) {
                mIsMatrixDisplayShow = "true".equals(value);
            }
        }
    }

    // notify camera will stop recording,some features need know it before real stop recording.
    private void setStopRecordingToCamera() {
        List<Integer> states = getSupportedRecordStates();
        if (isRecordStateSupported() && states.contains(STREAMING_FEATURE_STATE_PREVIEW)) {
            int[] state = {STREAMING_FEATURE_STATE_PREVIEW};
            mBuilder.set(mRecordStateKey, state);
            mBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_IDLE);
            mBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_IDLE);
            setRepeatingRequest(mBuilder);
        }
    }

    private List<Integer> getSupportedRecordStates() {
        if (mCameraCharacteristics == null) {
            return Collections.emptyList();
        }
        List<CameraCharacteristics.Key<?>> keys = mCameraCharacteristics.getKeys();
        for (CameraCharacteristics.Key<?> key : keys) {
            if (AVAILABLE_RECORD_STATES.equals(key.getName())) {
                mAvailableRecordStates = (CameraCharacteristics.Key<int[]>) key;
            }
        }
        if (mAvailableRecordStates == null) {
            return Collections.emptyList();
        }
        int[] availableStates = getValueFromKey(mAvailableRecordStates);
        List<Integer> supportedValues = null;
        if (availableStates != null) {
            supportedValues = new ArrayList<Integer>();
            int length = availableStates.length;
            for (int i = 0; i < length; i++) {
                supportedValues.add(availableStates[i]);
                LogHelper.d(TAG, "AVAILABLE_RECORD_STATES support value is " + availableStates[i]);
            }
        }
        return supportedValues;
    }

    private void initRecordStateKey() {
        List<CaptureRequest.Key<?>> keys = mCameraCharacteristics.getAvailableCaptureRequestKeys();
        for (CaptureRequest.Key<?> key : keys) {
            if (RECORD_STATE_REQUEST.equals(key.getName())) {
                mRecordStateKey = (CaptureRequest.Key<int[]>) key;
                break;
            }
        }
    }

    private boolean isRecordStateSupported() {
        return mRecordStateKey != null && mAvailableRecordStates != null &&
                getSupportedRecordStates().size() > 1;
    }

    private <T> T getValueFromKey(CameraCharacteristics.Key<T> key) {
        T value = null;
        try {
            value = mCameraCharacteristics.get(key);
            if (value == null) {
                LogHelper.e(TAG, key.getName() + "was null");
            }
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, key.getName() + " was not supported by this device");
        }
        return value;
    }

}
