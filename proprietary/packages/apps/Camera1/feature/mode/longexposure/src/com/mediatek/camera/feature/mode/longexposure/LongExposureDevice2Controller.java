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
 *     MediaTek Inc. (C) 2017. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DIStCLAIMS ANY AND ALL
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

package com.mediatek.camera.feature.mode.longexposure;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy.StateCallback;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Configurator;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.portability.AdvancedCamera;


import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * Class used to control device related operation in long exposure.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
class LongExposureDevice2Controller implements
        ILongExposureDeviceController,
        CaptureSurface.ImageCallback,
        ISettingManager.SettingDevice2Requester {
    private static final Tag TAG = new Tag(LongExposureDevice2Controller.class.getSimpleName());
    private static final int CAPTURE_FORMAT = ImageFormat.JPEG;
    private static final int CAPTURE_MAX_NUMBER = 2;
    private static final int WAIT_TIME = 5;
    private final Activity mActivity;
    private final CameraManager mCameraManager;
    private final CaptureSurface mCaptureSurface;
    private final ICameraContext mICameraContext;
    private final StateCallback mDeviceCallback = new DeviceStateCallback();
    private Object mSurfaceObject;
    private final Lock mLockState = new ReentrantLock();
    private final Object mWaitOpenCamera = new Object();
    private final Object mSurfaceHolderSync = new Object();

    private ISettingManager mSettingManager;
    private SettingDevice2Configurator mSettingDevice2Configurator;
    private SettingController mSettingController;

    private int mJpegRotation;
    private String mCurrentCameraId;
    private CameraDeviceManager mCameraDeviceManager;
    private volatile Camera2Proxy mCamera2Proxy;
    private Surface mPreviewSurface;
    private CameraState mCameraState = CameraState.CAMERA_UNKNOWN;
    private volatile Camera2CaptureSessionProxy mSession;
    private Handler mHandler;

    private JpegCallback mJpegCallback;
    private DeviceCallback mModeDeviceCallback;
    private PreviewSizeCallback mPreviewSizeCallback;

    private volatile int mPreviewWidth;
    private volatile int mPreviewHeight;
    private boolean mFirstFrameArrived = false;
    private boolean mIsPictureSizeChanged = false;
    private final Lock mDeviceLock = new ReentrantLock();
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final String KEY_SHUTTER_SPEED = "key_shutter_speed";

    private volatile boolean mIsSessionAbortCalled = false;
    private final LongExposureHandler mLongExposureHandler;

    private static final int MSG_IMAGE_RECEIVED = 0;

    private static final int MSG_ABORT = 0;

    /**
     * The enum is used for tag native camera state.
     */
    private enum CameraState {
        CAMERA_UNKNOWN, //initialize state.
        CAMERA_OPENING, //between open camera and open done callback.
        CAMERA_OPENED, //when camera open done.
        CAMERA_CLOSING,
    }

    /**
     * LongExposureDevice2Controller constructor, may use activity to get display rotation.
     *
     * @param activity the camera activity.
     */
    LongExposureDevice2Controller(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        mActivity = activity;
        mLongExposureHandler = new LongExposureHandler(Looper.myLooper());
        mCameraManager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        mICameraContext = context;
        mCaptureSurface = new CaptureSurface();
        mCaptureSurface.setCaptureCallback(this);
        prepareHandler();
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void openCamera(DeviceInfo info) {
        String cameraId = info.getCameraId();
        LogHelper.i(TAG, "[openCamera] cameraId : " + cameraId);
        initSettingManager(info.getSettingManager());
        if (canOpenCamera(cameraId)) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                mCurrentCameraId = cameraId;
                initSettings();
                updateCameraState(CameraState.CAMERA_OPENING);
                mCameraDeviceManager.openCamera(mCurrentCameraId, mDeviceCallback, null);
            } catch (CameraOpenException e) {
                if (CameraOpenException.ExceptionType.SECURITY_EXCEPTION == e.getExceptionType()) {
                    CameraUtil.showErrorInfoAndFinish(mActivity,
                            CameraUtil.CAMERA_HARDWARE_EXCEPTION);
                }
            } catch (CameraAccessException e) {
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_HARDWARE_EXCEPTION);
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                mDeviceLock.unlock();
            }
        }
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        LogHelper.d(TAG, "[updatePreviewSurface] surfaceHolder = " + surfaceObject + ",state = "
                + mCameraState);
        synchronized (mSurfaceHolderSync) {
            mSurfaceObject = surfaceObject;
            if (surfaceObject instanceof SurfaceHolder) {
                mPreviewSurface = surfaceObject == null ? null :
                        ((SurfaceHolder) surfaceObject).getSurface();
            } else if (surfaceObject instanceof SurfaceTexture) {
                mPreviewSurface = surfaceObject == null ? null :
                        new Surface((SurfaceTexture) surfaceObject);
            }
            boolean isStateReady = CameraState.CAMERA_OPENED == mCameraState;
            if (isStateReady && mCamera2Proxy != null) {
                if (surfaceObject != null) {
                    configureSession();
                } else {
                    stopPreview();
                }
            }
        }
    }

    @Override
    public void setDeviceCallback(DeviceCallback callback) {
        mModeDeviceCallback = callback;
    }

    @Override
    public void setPreviewSizeReadyCallback(PreviewSizeCallback callback) {
        mPreviewSizeCallback = callback;
    }

    /**
     * Set the new picture size.
     *
     * @param size current picture size.
     */
    @Override
    public void setPictureSize(Size size) {
        mIsPictureSizeChanged = mCaptureSurface.updatePictureInfo(size.getWidth(),
                size.getHeight(), CAPTURE_FORMAT, CAPTURE_MAX_NUMBER);
    }

    /**
     * Check whether can take picture or not.
     *
     * @return true means can take picture; otherwise can not take picture.
     */
    @Override
    public boolean isReadyForCapture() {
        boolean canCapture = mSession != null
                && mCamera2Proxy != null && getCameraState() == CameraState.CAMERA_OPENED;
        LogHelper.i(TAG, "[isReadyForCapture] canCapture = " + canCapture);
        return canCapture;
    }

    @Override
    public void destroyDeviceController() {
        if (mHandler != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                mHandler.getLooper().quitSafely();
            } else {
                mHandler.getLooper().quit();
            }
        }
        if (mCaptureSurface != null) {
            mCaptureSurface.releaseCaptureSurface();
            mCaptureSurface.release();
        }
    }

    @Override
    public void startPreview() {
        LogHelper.i(TAG, "[startPreview]");
        configureSession();
    }

    @Override
    public void stopPreview() {
        LogHelper.i(TAG, "[stopPreview]");
        abortOldSession();
    }

    @Override
    public void takePicture(@Nonnull JpegCallback callback) {
        if (mSession == null) {
            return;
        }
        mJpegCallback = callback;
        Builder builder;
        try {
            builder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_STILL_CAPTURE);
            builder.addTarget(mCaptureSurface.getSurface());
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[takePicture] error create build fail.");
            return;
        }

        //set the jpeg orientation
        int rotation = CameraUtil.getJpegRotationFromDeviceSpec(Integer.parseInt(mCurrentCameraId),
                mJpegRotation, mActivity);
        builder.set(CaptureRequest.JPEG_ORIENTATION, rotation);

        //set take picture command.
        try {
            mSession.capture(builder.build(), mCaptureCallback, mHandler);
            ISettingManager.SettingController controller = mSettingManager
                    .getSettingController();
            String speed = controller.queryValue("key_shutter_speed");
            if (!LongExposureModeHelper.EXPOSURE_TIME_AUTO.equals(speed)) {
                mICameraContext.getSoundPlayback().play(ISoundPlayback.SHUTTER_CLICK);
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void stopCapture() {
        LogHelper.i(TAG, "[stopCapture] mSession= " + mSession + ",isAborting " + mSession);
        mIsSessionAbortCalled = true;
        mHandler.sendEmptyMessage(MSG_ABORT);
    }

    @Override
    public void setNeedWaitPictureDone(boolean needWaitPictureDone) {

    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mJpegRotation = orientation;
    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.i(TAG, "[closeCamera] + sync = " + sync + ",current state : " + mCameraState);
        if (CameraState.CAMERA_UNKNOWN != mCameraState) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                waitOpenDoneForClose();
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                abortOldSession();
                if (mModeDeviceCallback != null) {
                    mModeDeviceCallback.beforeCloseCamera();
                }
                doCloseCamera(sync);
                recycleVariables();
                mCaptureSurface.releaseCaptureSurface();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                mDeviceLock.unlock();
            }
            recycleVariables();
        }
        LogHelper.i(TAG, "[closeCamera] -");
    }

    @Override
    public Size getPreviewSize(double targetRatio) {
        int oldPreviewWidth = mPreviewWidth;
        int oldPreviewHeight = mPreviewHeight;
        updateTargetPreviewSize(targetRatio);
        boolean isSameSize = oldPreviewHeight == mPreviewHeight && oldPreviewWidth == mPreviewWidth;
        LogHelper.i(TAG, "[getPreviewSize], old size : " + oldPreviewWidth + " X " +
                oldPreviewHeight + ", new  size :" + mPreviewWidth + " X " + mPreviewHeight);
        //if preview size don't change, but picture size changed,need do configure the surface.
        //if preview size changed,do't care the picture size changed,because surface will be
        //changed.
        if (isSameSize && mIsPictureSizeChanged) {
            configureSession();
        }
        return new Size(mPreviewWidth, mPreviewHeight);
    }

    @Override
    public void onPictureCallback(byte[] data) {
        mLongExposureHandler.obtainMessage(MSG_IMAGE_RECEIVED, data).sendToTarget();
    }

    @Override
    public void createAndChangeRepeatingRequest() {
        if (mCamera2Proxy == null || mCameraState != CameraState.CAMERA_OPENED) {
            LogHelper.e(TAG, "camera is closed or in opening state, can't request ");
            return;
        }
        repeatingPreview();
    }

    @Override
    public CaptureRequest.Builder createAndConfigRequest(int templateType) {
        CaptureRequest.Builder builder = null;
        try {
            builder = doCreateAndConfigRequest(templateType);
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
        configureSession();
    }

    @Override
    public int getRepeatingTemplateType() {
        return Camera2Proxy.TEMPLATE_PREVIEW;
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
            if (sync) {
                mCamera2Proxy.close();
            } else {
                mCamera2Proxy.closeAsync();
            }
        }
    }

    private void recycleVariables() {
        mCurrentCameraId = null;
        updatePreviewSurface(null);
        mCamera2Proxy = null;
        mIsPictureSizeChanged = false;
    }

    private void prepareHandler() {
        HandlerThread handlerThread = new HandlerThread("camera_session");
        handlerThread.start();
        mHandler = new Handler(handlerThread.getLooper()) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_ABORT:
                        try {
                            LogHelper.d(TAG, "[stopCapture] Go to abortCaptures");
                            mSession.abortCaptures();
                        } catch (CameraAccessException e) {
                            LogHelper.e(TAG, "[stopCapture] CameraAccessException " + e);
                        }
                        break;
                    default:
                        break;
                }
            }
        };
    }

    private boolean canOpenCamera(String newCameraId) {
        boolean isSameCamera = newCameraId.equalsIgnoreCase(mCurrentCameraId);
        boolean isStateReady = mCameraState == CameraState.CAMERA_UNKNOWN;
        //if is same camera id, don't need open the camera.
        boolean value = !isSameCamera && isStateReady;
        LogHelper.i(TAG, "[canOpenCamera] new id: " + newCameraId + ",current camera :" +
                mCurrentCameraId + ",isSameCamera = " + isSameCamera + ", current state : " +
                mCameraState + ",isStateReady = " + isStateReady + ",can open : " + value);
        return value;
    }

    private void initSettingManager(ISettingManager settingManager) {
        mSettingManager = settingManager;
        settingManager.updateModeDevice2Requester(this);
        mSettingDevice2Configurator = settingManager.getSettingDevice2Configurator();
        mSettingController = settingManager.getSettingController();
    }

    private void initSettings() throws CameraAccessException {
        mSettingManager.createAllSettings();
        mSettingDevice2Configurator.setCameraCharacteristics(
                mCameraManager.getCameraCharacteristics(mCurrentCameraId));
        Relation relation = LongExposureRestriction.getRestriction().getRelation("on",
                false);
        mSettingManager.getSettingController().postRestriction(relation);
        mSettingController.addViewEntry();
        mSettingController.refreshViewEntry();
    }

    private void configureSession() {
        if (mCamera2Proxy == null) {
            return;
        }
        //if the old session is in processing, need abort before one.
        abortOldSession();
        //Prepare the new session.
        List<Surface> surfaces = new LinkedList<>();
        surfaces.add(mPreviewSurface);
        //Prepare the capture surface.
        surfaces.add(mCaptureSurface.getSurface());
        try {
            preSetSession();
            mSettingDevice2Configurator.configSessionSurface(surfaces);
            LogHelper.d(TAG, "[configureSession] surface size : " + surfaces.size());
            mCamera2Proxy.createCaptureSession(surfaces, mSessionCallback, mHandler);
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[configureSession] error");
        }
        mIsPictureSizeChanged = false;
    }

    private void preSetSession() {
        try {
            AdvancedCamera.preSetSession(Integer.parseInt(mCurrentCameraId),
                    doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_PREVIEW).build());
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void abortOldSession() {
        LogHelper.d(TAG, "[abortOldSession]");
        if (mSession != null) {
            try {
                mSession.abortCaptures();
                mSession.close();
                mSession = null;
            } catch (CameraAccessException e) {
                LogHelper.e(TAG, "[abortOldSession] CameraAccessException ", e);
            }
        }
    }

    private void repeatingPreview() {
        LogHelper.i(TAG, "[repeatingPreview] mSession = " + mSession);
        if (mSession == null) {
            return;
        }
        try {
            mFirstFrameArrived = false;
            Builder builder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_PREVIEW);
            mSession.setRepeatingRequest(builder.build(), mCaptureCallback, null);
        } catch (CameraAccessException | RuntimeException e) {
            LogHelper.e(TAG, "[repeatingPreview] error");
        }
    }

    private Builder doCreateAndConfigRequest(int templateType) throws CameraAccessException {
        CaptureRequest.Builder builder = null;
        if (mCamera2Proxy != null) {
            builder = mCamera2Proxy.createCaptureRequest(templateType);
            mSettingDevice2Configurator.configCaptureRequest(builder);
            builder.addTarget(mPreviewSurface);
        }
        return builder;
    }

    private void updateTargetPreviewSize(double ratio) {
        try {
            CameraCharacteristics cs = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
            StreamConfigurationMap streamConfigurationMap =
                    cs.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            android.util.Size previewSizes[] =
                    streamConfigurationMap.getOutputSizes(SurfaceHolder.class);
            int length = previewSizes.length;
            List<Size> sizes = new ArrayList<>(length);
            for (int i = 0; i < length; i++) {
                sizes.add(i, new Size(previewSizes[i].getWidth(), previewSizes[i].getHeight()));
            }
            Size values = CameraUtil.getOptimalPreviewSize(mActivity, sizes, ratio, true);
            mPreviewWidth = values.getWidth();
            mPreviewHeight = values.getHeight();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[updateTargetPreviewSize] " + mPreviewWidth + " X " + mPreviewHeight);
    }

    private void updatePreviewSize() {
        ISettingManager.SettingController controller = mSettingManager.getSettingController();
        String pictureSize = controller.queryValue(KEY_PICTURE_SIZE);
        LogHelper.i(TAG, "[updatePreviewSize] :" + pictureSize);
        if (pictureSize != null) {
            String[] pictureSizes = pictureSize.split("x");
            int width = Integer.parseInt(pictureSizes[0]);
            int height = Integer.parseInt(pictureSizes[1]);
            double ratio = (double) width / height;
            updateTargetPreviewSize(ratio);
        }
    }

    /**
     * This is device state callback.
     */
    private class DeviceStateCallback extends StateCallback {

        @Override
        public void onOpened(@Nonnull Camera2Proxy camera2proxy) {
            LogHelper.i(TAG, "[onOpened]  camera2proxy = " + camera2proxy + " preview surface = "
                    + mPreviewSurface + "  mCameraState = " + mCameraState);

            try {
                mCamera2Proxy = camera2proxy;
                if (CameraState.CAMERA_OPENING == getCameraState() && camera2proxy != null) {
                    if (mModeDeviceCallback != null) {
                        mModeDeviceCallback.onCameraOpened(mCurrentCameraId);
                    }
                    updateCameraState(CameraState.CAMERA_OPENED);
                    updatePreviewSize();
                    if (mPreviewSizeCallback != null) {
                        mPreviewSizeCallback.onPreviewSizeReady(new Size(mPreviewWidth,
                                mPreviewHeight));
                    }
                }
            } catch (RuntimeException e) {
                e.printStackTrace();
            } finally {
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
            }
        }

        @Override
        public void onClosed(@Nonnull Camera2Proxy camera2Proxy) {
            LogHelper.i(TAG, "[onClosed]  proxy = " + camera2Proxy);
            if (mCamera2Proxy != null && mCamera2Proxy == camera2Proxy) {
                mCamera2Proxy = null;
            }
            //reset the surface holder to null
            synchronized (mSurfaceHolderSync) {
                mSurfaceObject = null;
            }
        }

        @Override
        public void onDisconnected(@Nonnull Camera2Proxy camera2proxy) {
            LogHelper.i(TAG, "[onDisconnected]  camera2proxy = " + camera2proxy);
            if (mCamera2Proxy != null && mCamera2Proxy == camera2proxy) {
                synchronized (mWaitOpenCamera) {
                    mWaitOpenCamera.notifyAll();
                }
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);
            }
        }

        @Override
        public void onError(@Nonnull Camera2Proxy camera2Proxy, int error) {
            LogHelper.i(TAG, "[onError]  camera2proxy = " + camera2Proxy + " error = " + error);
            if ((mCamera2Proxy != null && mCamera2Proxy == camera2Proxy)
                    || error == CameraUtil.CAMERA_OPEN_FAIL) {
                synchronized (mWaitOpenCamera) {
                    updateCameraState(CameraState.CAMERA_UNKNOWN);
                    mWaitOpenCamera.notifyAll();
                }
                CameraUtil.showErrorInfoAndFinish(mActivity, error);
            }
        }
    }

    /**
     * Camera session callback.
     */
    private final Camera2CaptureSessionProxy.StateCallback mSessionCallback = new
            Camera2CaptureSessionProxy.StateCallback() {

                @Override
                public void onConfigured(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigured],session = " + session);
                    mSession = session;
                    mDeviceLock.lock();
                    try {
                        if (CameraState.CAMERA_OPENED == getCameraState()) {
                            mSession = session;
                            synchronized (mSurfaceHolderSync) {
                                if (mPreviewSurface != null) {
                                    repeatingPreview();
                                }
                            }
                        }
                    } finally {
                        mDeviceLock.unlock();
                    }
                }

                @Override
                public void onConfigureFailed(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigureFailed],session = " + session);
                    if (mSession == session) {
                        mSession = null;
                    }
                }

                @Override
                public void onClosed(@Nonnull Camera2CaptureSessionProxy session) {
                    super.onClosed(session);
                    LogHelper.i(TAG, "[onClosed],session = " + session);
                    if (mSession == session) {
                        mSession = null;
                    }
                }

                @Override
                public void onReady(@Nonnull Camera2CaptureSessionProxy session) {
                    super.onReady(session);
                    if (mSession == session) {
                        if (mIsSessionAbortCalled) {
                            LogHelper.d(TAG, "[onReady]");
                            mIsSessionAbortCalled = false;
                            repeatingPreview();
                        }
                    }
                }
            };

    /**
     * Capture callback.
     */
    private final CaptureCallback mCaptureCallback = new CaptureCallback() {

        @Override
        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long
                timestamp, long frameNumber) {
            super.onCaptureStarted(session, request, timestamp, frameNumber);
            int requestTemplate = request.get(CaptureRequest.CONTROL_CAPTURE_INTENT);
            boolean isNormalCapture = Camera2Proxy.TEMPLATE_STILL_CAPTURE == requestTemplate;
            if (!isNormalCapture) {
                return;
            }
            mICameraContext.getSoundPlayback().play(ISoundPlayback.SHUTTER_CLICK);
        }

        @Override
        public void onCaptureCompleted(@Nonnull CameraCaptureSession session,
                                       @Nonnull CaptureRequest request, @Nonnull
                                               TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            mSettingDevice2Configurator.getRepeatingCaptureCallback().onCaptureCompleted(
                    session, request, result);
            if (mModeDeviceCallback != null && !mFirstFrameArrived) {
                mFirstFrameArrived = true;
                mModeDeviceCallback.onPreviewCallback(null, 0);
            }
        }

        @Override
        public void onCaptureFailed(@Nonnull CameraCaptureSession session,
                                    @Nonnull CaptureRequest request, @Nonnull CaptureFailure
                                            failure) {
            super.onCaptureFailed(session, request, failure);
            mSettingDevice2Configurator.getRepeatingCaptureCallback()
                    .onCaptureFailed(session, request, failure);
            int captureIntent = request.get(CaptureRequest.CONTROL_CAPTURE_INTENT);
            if (CameraMetadata.CONTROL_CAPTURE_INTENT_STILL_CAPTURE == captureIntent) {
                LogHelper.d(TAG, "[onCaptureFailed] the capture has failed due to a result " +
                        failure.getReason());
            }
        }
    };

    /**
     * Handler with long exposure feature thread.
     */
    private final class LongExposureHandler extends Handler {

        public LongExposureHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.i(TAG, "[handleMessage] msg.what " + msg.what);
            switch (msg.what) {
                case MSG_IMAGE_RECEIVED:
                    mFirstFrameArrived = false;
                    if (mJpegCallback != null) {
                        mJpegCallback.onDataReceived((byte[]) msg.obj);
                    }
                    break;
                default:
                    break;
            }
        }
    }

}
