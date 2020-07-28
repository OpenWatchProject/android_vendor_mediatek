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
package com.mediatek.camera.feature.mode.slowmotion;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.CamcorderProfile;
import android.os.Build;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
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
import com.mediatek.camera.common.mode.video.device.IDeviceController;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.CamcorderProfileEx;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * Control camera device for slow motion.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class SlowMotionDevice implements IDeviceController,
                                  ISettingManager.SettingDevice2Requester {

    private static final LogUtil.Tag TAG = new LogUtil.Tag(SlowMotionDevice.class.getSimpleName());
    private static final int HANDLER_UPDATE_PREVIEW_SURFACE = 1;
    private static final int SLOW_MOTION_QUALITY_HIGH = 2222;
    private static final int SLOW_MOTION_QUALITY_LOW = 2220;
    private static final int PREPARE_TIMEOUT_MS = 10000; // 10s
    private static final int WAIT_TIME = 5;

    private final HashMap<Camera2CaptureSessionProxy, List<Surface>>
                                 mPreparedSurfaces = new HashMap<>();
    private final SessionFuture mSessionFuture = new SessionFuture();

    private boolean mIsRecorderSurfaceConfigured = false;
    private boolean mNeedRConfigSession = false;
    private boolean mFirstFrameArrived = false;
    private boolean mIsRecording = false;


    private Camera2Proxy.StateCallback mDeviceCallback = new DeviceStateCallback();
    private CameraState mCameraState = CameraState.CAMERA_UNKNOWN;
    private Object mPreviewSurfaceSync = new Object();
    private Object mWaitOpenCamera = new Object();
    private Lock mDeviceLock = new ReentrantLock();
    private Lock mLockState = new ReentrantLock();

    private ISettingManager.SettingDevice2Configurator mSettingDevice2Configurator;
    private DeviceCallback mModeDeviceCallback;
    private CameraDeviceManager mCameraDeviceManager;
    private RestrictionProvider mRestrictionProvider;
    private CameraCharacteristics mCharacteristics;
    private SettingConfigCallback mSettingConfig;
    private Camera2CaptureSessionProxy mSession;
    private VideoDeviceHandler mVideoHandler;
    private PreviewCallback mPreviewCallback;
    private ISettingManager mSettingManager;
    private ICameraContext mICameraContext;
    private CameraManager mCameraManager;
    private Camera2Proxy mCamera2Proxy;
    private CamcorderProfile mProfile;
    private Surface mPreviewSurface;
    private Handler mSessionHandler;
    private Surface mRecordSurface;
    private Activity mActivity;
    private String mCameraId;

    private static final int[] sMtkSlowQualities = new int[] {
            SLOW_MOTION_QUALITY_LOW,
            SLOW_MOTION_QUALITY_HIGH
    };

    private static final int[] sSlowQualities = new int[] {
            CamcorderProfile.QUALITY_HIGH_SPEED_480P,
            CamcorderProfile.QUALITY_HIGH_SPEED_720P,
            CamcorderProfile.QUALITY_HIGH_SPEED_1080P,
            CamcorderProfile.QUALITY_HIGH_SPEED_2160P
    };
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
     * SlowMotionDevice may use activity to get display rotation.
     * @param activity the camera activity.
     * @param context  the camera context.
     */
    public SlowMotionDevice(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        LogHelper.d(TAG, "[SlowMotionDevice] Construct");
        mActivity = activity;
        mICameraContext = context;
        initDeviceHandler();
        prepareSessionHandler();
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void createAndChangeRepeatingRequest() {
        try {
            CaptureRequest.Builder builder = doCreateAndConfigRequest(mIsRecording);
            setRepeatingBurst(builder);
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
    public void openCamera(@Nonnull ISettingManager settingManager, @Nonnull String cameraId,
                           boolean sync, RestrictionProvider relation) {
        LogHelper.i(TAG, "[openCamera] + cameraId : " + cameraId + ",sync = " + sync);
        if (CameraState.CAMERA_UNKNOWN != getCameraState() ||
                (mCameraId != null && cameraId.equalsIgnoreCase(mCameraId))) {
            LogHelper.e(TAG, "[openCamera] mCameraState = " + mCameraState);
            return;
        }
        updateCameraState(CameraState.CAMERA_OPENING);
        mCameraId = cameraId;
        mRestrictionProvider = relation;
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
        LogHelper.i(TAG, "[openCamera] -");
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        LogHelper.d(TAG, "[updatePreviewSurface] surfaceHolder = " + surfaceObject);
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

    @Override
    public void stopPreview() {
        abortOldSession();
    }

    @Override
    public void startPreview() {

    }

    @Override
    public void takePicture(@Nonnull JpegCallback callback) {

    }

    @Override
    public void updateGSensorOrientation(int orientation) {

    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.i(TAG, "[closeCamera] sync = " + sync + " mCameraState = " + mCameraState);
        if (CameraState.CAMERA_UNKNOWN != getCameraState()) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                waitOpenDoneForClose();
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                mModeDeviceCallback.beforeCloseCamera();
                abortOldSession();
                doCloseCamera(sync);
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
        try {
            prepareRecorderSurface();
            mIsRecording = true;
            CaptureRequest.Builder builder = doCreateAndConfigRequest(true);
            setRepeatingBurst(builder);
            mICameraContext.getSoundPlayback().play(ISoundPlayback.START_VIDEO_RECORDING);
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[startRecording] fail");
            return;
        }
        LogHelper.d(TAG, "[startRecording] - ");
    }

    @Override
    public void stopRecording() {
        LogHelper.i(TAG, "[stopRecording] + ");
        mICameraContext.getSoundPlayback().play(ISoundPlayback.STOP_VIDEO_RECORDING);
        mIsRecording = false;
        repeatingPreview();
        LogHelper.d(TAG, "[stopRecording] - ");
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
        LogHelper.d(TAG, "[configCamera] - ");
    }


    @Override
    public Camera.CameraInfo getCameraInfo(int cameraId) {
        return null;
    }

    @Override
    public boolean isVssSupported(int cameraId) {
        return false;
    }

    @Override
    public CamcorderProfile getCamcorderProfile() {
        return mProfile;
    }

    @Override
    public void release() {
        mVideoHandler.getLooper().quit();
        mSessionHandler.getLooper().quit();
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
    public void postRecordingRestriction(List<Relation> relations, boolean isNeedConfigRequest) {

    }

    private void setRepeatingBurst(CaptureRequest.Builder builder) {
        if (mSession != null) {
            synchronized (mSession) {
                if (mSession != null) {
                    try {
                        mSession.setRepeatingBurst(
                                mSession.createHighSpeedRequestList(builder.build()),
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

    private void prepareRecorderSurface() throws CameraAccessException {
        List<Surface> preparedSurfaces = mPreparedSurfaces.get(mSession);
        if ((preparedSurfaces != null && !preparedSurfaces.contains(mRecordSurface))
                || preparedSurfaces == null) {
            LogHelper.i(TAG, "waitForSurfacePrepared prepare and wait");
            mSession.prepare(mRecordSurface);
            waitForSurfacePrepared(mSession, mRecordSurface, PREPARE_TIMEOUT_MS);
        }
    }

    private void initDeviceHandler() {
        HandlerThread ht = new HandlerThread("Slow motion device Handler Thread");
        ht.start();
        mVideoHandler = new VideoDeviceHandler(ht.getLooper());
    }

    private void initSettings() {
        mSettingManager.createAllSettings();
        mSettingDevice2Configurator.setCameraCharacteristics(mCharacteristics);
        mSettingManager.getSettingController().postRestriction(
                mRestrictionProvider.getRestriction());
        mSettingManager.getSettingController().addViewEntry();
        mSettingManager.getSettingController().refreshViewEntry();
    }

    private void initSettingManager(ISettingManager settingManager) {
        mSettingManager = settingManager;
        mSettingManager.updateModeDevice2Requester(this);
        mSettingDevice2Configurator =  mSettingManager.getSettingDevice2Configurator();
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

    private void doCloseCamera(boolean sync) {
        if (mCamera2Proxy != null) {
            synchronized (mCamera2Proxy) {
                if (mCamera2Proxy != null) {
                    if (sync) {
                        mCamera2Proxy.close();
                    } else {
                        mCamera2Proxy.closeAsync();
                    }
                }
                mCamera2Proxy = null;
            }
        }
    }

    private void releaseVariables() {
        mPreparedSurfaces.clear();
        mCameraId = null;
        mPreviewSurface = null;
        mRecordSurface = null;
        mIsRecorderSurfaceConfigured = false;
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

    private void doUpdatePreviewSurface() {
        LogHelper.d(TAG, "[doUpdatePreviewSurface] mPreviewSurface = " + mPreviewSurface
                + " state = " + mCameraState + " mNeedRConfigSession = " + mNeedRConfigSession
                + " mRecordSurface = " + mRecordSurface
                + " mIsRecorderSurfaceConfigured = " + mIsRecorderSurfaceConfigured);
        synchronized (mPreviewSurfaceSync) {
            if (CameraState.CAMERA_OPENED == mCameraState && mPreviewSurface != null
                    && mNeedRConfigSession && mIsRecorderSurfaceConfigured) {
                configureSession();
                mNeedRConfigSession = false;
            }
        }
    }

    private void prepareSessionHandler() {
        HandlerThread mSessionHandlerThread = new HandlerThread("camera_session");
        mSessionHandlerThread.start();
        mSessionHandler = new Handler(mSessionHandlerThread.getLooper());
    }

    private class DeviceStateCallback extends Camera2Proxy.StateCallback {
        @Override
        public void onOpened(@Nonnull Camera2Proxy camera2proxy) {
            LogHelper.i(TAG, "[onOpened] + camera2proxy = " + camera2proxy);
            try {
                mCamera2Proxy = camera2proxy;
                if (CameraState.CAMERA_OPENING == getCameraState() && camera2proxy != null) {
                    updateCameraState(CameraState.CAMERA_OPENED);
                    mModeDeviceCallback.onCameraOpened(mCameraId);
                    mNeedRConfigSession = true;
                    updatePreviewSize();
                }
            } catch (RuntimeException | CameraAccessException e) {
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
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            synchronized (mWaitOpenCamera) {
                mWaitOpenCamera.notifyAll();
            }
            CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);
        }

        @Override
        public void onError(@Nonnull Camera2Proxy camera2Proxy, int error) {
            LogHelper.i(TAG, "[onError] camera2proxy = " + camera2Proxy + " error = " + error);
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            mModeDeviceCallback.onError();
            synchronized (mWaitOpenCamera) {
                mWaitOpenCamera.notifyAll();
            }
            CameraUtil.showErrorInfoAndFinish(mActivity, error);
        }
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

    private void configureSession() {
        LogHelper.i(TAG, "[configureSession] + ");
        abortOldSession();
        List<Surface> surfaces = new LinkedList<>();
        surfaces.add(mPreviewSurface);
        if (mRecordSurface != null) {
            surfaces.add(mRecordSurface);
        }
        try {
            mSettingDevice2Configurator.configSessionSurface(surfaces);
            mCamera2Proxy.createConstrainedHighSpeedCaptureSession(
                    surfaces, mSessionCallback, mSessionHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[configureSession] - ");
    }

    private void abortOldSession() {
        if (mSession != null) {
            synchronized (mSession) {
                if (mSession != null) {
                    try {
                        mSession.abortCaptures();
                        mSession.close();
                        mSession = null;
                    } catch (CameraAccessException e) {
                        LogHelper.e(TAG, "[abortOldSession] exception", e);
                    }
                }
            }
        }
    }
    private final Camera2CaptureSessionProxy.StateCallback mSessionCallback = new
            Camera2CaptureSessionProxy.StateCallback() {

                @Override
                public void onConfigured(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigured] session = " + session
                            + "mCameraState = " + mCameraState);
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
                            mModeDeviceCallback.onPreviewStart();
                        }
                    } finally {
                        mDeviceLock.unlock();
                    }
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

                @Override
                public void onSurfacePrepared(@Nonnull Camera2CaptureSessionProxy session,
                                              @Nonnull Surface surface) {
                    mSessionFuture.setSession(session);
                    synchronized (mPreparedSurfaces) {
                        LogHelper.i(TAG, "onSurfacePrepared");
                        List<Surface> preparedSurfaces = mPreparedSurfaces.get(session);
                        if (preparedSurfaces == null) {
                            preparedSurfaces = new ArrayList<Surface>();
                        }
                        preparedSurfaces.add(surface);
                        mPreparedSurfaces.put(session, preparedSurfaces);
                        mPreparedSurfaces.notifyAll();
                    }
                }
            };

    private void initDeviceInfo() {
        try {
            mCameraManager = (CameraManager) mActivity.getSystemService(Context.CAMERA_SERVICE);
            mCharacteristics = mCameraManager.getCameraCharacteristics(mCameraId);
        } catch (RuntimeException e) {
            e.printStackTrace();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void repeatingPreview() {
        LogHelper.i(TAG, "[repeatingPreview] + ");
        try {
            mFirstFrameArrived = false;
            CaptureRequest.Builder builder = doCreateAndConfigRequest(false);
            setRepeatingBurst(builder);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[repeatingPreview] - ");
    }

    private CameraCaptureSession.CaptureCallback mPreviewCapProgressCallback =
                                                new CameraCaptureSession.CaptureCallback() {
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
        mProfile = checkerProfile();
        LogHelper.i(TAG, "profile" + mProfile.videoFrameWidth + " * " + mProfile.videoFrameHeight);
    }

    private CamcorderProfile findProfileForRange(int[] qualityMatrix) {
        CamcorderProfile profile = null;
        for (int i = 0; i < qualityMatrix.length; i++) {
            if (CamcorderProfile.hasProfile(Integer.parseInt(mCameraId), qualityMatrix[i])) {
                profile = CamcorderProfileEx.getProfile(
                        Integer.parseInt(mCameraId), qualityMatrix[i]);
                Range<Integer> range = getHighSpeedFixedFpsRangeForSize(new android.util.Size(
                        profile.videoFrameWidth, profile.videoFrameHeight), true);
                if (range != null && range.getLower() == profile.videoFrameRate) {
                    LogHelper.i(TAG, "find slow motion FrameRate is "
                            + profile.videoFrameRate + "Camera id = " + mCameraId + "size = "
                            + profile.videoFrameWidth + " x" + profile.videoFrameHeight);
                    return profile;
                }
            }
        }
        return profile;
    }

    private CamcorderProfile checkerProfile() {
        CamcorderProfile profile = findProfileForRange(sMtkSlowQualities);
        if (profile == null) {
            profile = findProfileForRange(sSlowQualities);
        }
        LogHelper.d(TAG, "[checkerProfile] profile = " + profile);
        return profile;
    }


    private void updatePreviewSize() throws CameraAccessException {
        initProfile();
        mSettingConfig.onConfig(new com.mediatek.camera.common.utils.Size(
                mProfile.videoFrameWidth, mProfile.videoFrameHeight));
    }

    private void configAeFpsRange(CaptureRequest.Builder requestBuilder,
                                    CamcorderProfile profile, boolean isRecording) {
        try {
            android.util.Size size =
                    new android.util.Size(profile.videoFrameWidth, profile.videoFrameHeight);
            Range<Integer> fixedFpsRanges = getHighSpeedFixedFpsRangeForSize(size, isRecording);
            if (fixedFpsRanges != null) {
                requestBuilder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, fixedFpsRanges);
                LogHelper.i(TAG, "[configAeFpsRange] = " + fixedFpsRanges.toString());
            } else {
                LogHelper.e(TAG, "[configAeFpsRange] error fps range not found");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private Range<Integer> getHighSpeedFixedFpsRangeForSize(
            android.util.Size size, boolean isRecording) {
        StreamConfigurationMap config =
                mCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        try {
            Range<Integer>[] availableFpsRanges = config.getHighSpeedVideoFpsRangesFor(size);
            for (Range<Integer> range : availableFpsRanges) {
                if (isRecording) {
                    if (range.getLower().equals(range.getUpper())) {
                        LogHelper.d(TAG, "[getHighSpeedFpsRange] range = " + range.toString());
                        return range;
                    }
                } else if (!range.getLower().equals(range.getUpper())) {
                    LogHelper.d(TAG, "[getHighSpeedFpsRangeForSize] range = " + range.toString());
                    return range;
                }
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        return null;
    }

    private CaptureRequest.Builder doCreateAndConfigRequest(boolean isRecording)
                                                             throws CameraAccessException {
        LogHelper.d(TAG, "[doCreateAndConfigRequest] isRecording = " + isRecording);
        CaptureRequest.Builder builder = null;
        if (mCamera2Proxy != null) {
            builder = mCamera2Proxy.createCaptureRequest(Camera2Proxy.TEMPLATE_RECORD);
            configAeFpsRange(builder, mProfile, isRecording);
            if (isRecording) {
                builder.addTarget(mRecordSurface);
            }
            builder.addTarget(mPreviewSurface);
            mSettingDevice2Configurator.configCaptureRequest(builder);
        }
        return builder;
    }

    private void waitForSurfacePrepared(
            Camera2CaptureSessionProxy session, Surface surface, long timeoutMs) {
        synchronized (mPreparedSurfaces) {
            List<Surface> preparedSurfaces = mPreparedSurfaces.get(session);
            if (preparedSurfaces != null && preparedSurfaces.contains(surface)) {
                LogHelper.i(TAG, "waitForSurfacePrepared no need to wait");
                return;
            }
            try {
                long waitTimeRemaining = timeoutMs;
                while (waitTimeRemaining > 0) {
                    long waitStartTime = SystemClock.elapsedRealtime();
                    mPreparedSurfaces.wait(timeoutMs);
                    long waitTime = SystemClock.elapsedRealtime() - waitStartTime;
                    waitTimeRemaining -= waitTime;
                    preparedSurfaces = mPreparedSurfaces.get(session);
                    if (waitTimeRemaining >= 0 && preparedSurfaces != null &&
                            preparedSurfaces.contains(surface)) {
                        LogHelper.i(TAG, "waitForSurfacePrepared wait done");
                        return;
                    }
                }
                LogHelper.i(TAG, "waitForSurfacePrepared wait time");
            } catch (InterruptedException ie) {
                throw new AssertionError();
            }
        }
    }

    /**
     * This class use to restore surface which is prepared.
     */
    private static class SessionFuture implements Future<Camera2CaptureSessionProxy> {
        private volatile Camera2CaptureSessionProxy mSession;
        ConditionVariable mCondVar = new ConditionVariable(/*opened*/false);

        public void setSession(Camera2CaptureSessionProxy session) {
            mSession = session;
            mCondVar.open();
        }

        @Override
        public boolean cancel(boolean mayInterruptIfRunning) {
            return false; // don't allow canceling this task
        }

        @Override
        public boolean isCancelled() {
            return false; // can never cancel this task
        }

        @Override
        public boolean isDone() {
            return mSession != null;
        }

        @Override
        public Camera2CaptureSessionProxy get() {
            mCondVar.block();
            return mSession;
        }

        @Override
        public Camera2CaptureSessionProxy get(
                long timeout, TimeUnit unit) throws TimeoutException {
            long timeoutMs = unit.convert(timeout, TimeUnit.MILLISECONDS);
            if (!mCondVar.block(timeoutMs)) {
                throw new TimeoutException(
                        "Failed to receive session after " + timeout + " " + unit);
            }

            if (mSession == null) {
                throw new AssertionError();
            }
            return mSession;
        }

    }
}
