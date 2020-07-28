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

package com.mediatek.camera.feature.mode.longexposure;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.SurfaceHolder;

import com.google.common.base.Preconditions;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * Long exposure device controller.
 */
public class LongExposureDeviceController implements ILongExposureDeviceController,
        SettingDeviceRequester {
    private static final Tag TAG = new Tag(LongExposureDeviceController.class.getSimpleName());
    // notify for Image before compress when taking capture
    private volatile CameraState mCameraState = CameraState.CAMERA_UNKNOWN;


    private final Activity mActivity;
    private CameraDeviceManager mCameraDeviceManager;
    private ICameraContext mICameraContext;

    /**
     * this enum is used for tag native camera open state.
     */
    private enum CameraState {
        CAMERA_UNKNOWN, //initialize state.
        CAMERA_OPENING, //between open camera and open done callback.
        CAMERA_OPENED, //when camera open done.
    }

    private LongExposureHandler mRequestHandler;

    /**
     * Controller camera device
     *
     * @param activity current activity.
     * @param context  current camera context.
     */
    LongExposureDeviceController(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        // TODO: will use feature thread later
        HandlerThread handlerThread = new HandlerThread("LongExposureDeviceController");
        handlerThread.start();
        mRequestHandler = new LongExposureHandler(handlerThread.getLooper()/*Looper.myLooper()*/,
                this);
        mICameraContext = context;
        mActivity = activity;
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API1);
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API1);
    }

    @Override
    public void openCamera(DeviceInfo info) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.OPEN_CAMERA, info).sendToTarget();
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.UPDATE_PREVIEW_SURFACE,
                surfaceObject)
                .sendToTarget();
    }

    @Override
    public void setDeviceCallback(DeviceCallback callback) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.SET_PREVIEW_CALLBACK, callback)
                .sendToTarget();
    }

    @Override
    public void startPreview() {
        mRequestHandler.sendEmptyMessage(LongExposureDeviceAction.START_PREVIEW);
    }

    @Override
    public void stopPreview() {
        mRequestHandler.sendEmptyMessage(LongExposureDeviceAction.STOP_PREVIEW);
        waitDone();
    }

    @Override
    public void takePicture(@Nonnull JpegCallback callback) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.TAKE_PICTURE, callback)
                .sendToTarget();
    }

    @Override
    public void stopCapture() {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.ABORT_CAPTURE)
                .sendToTarget();
    }

    @Override
    public void setNeedWaitPictureDone(boolean needWaitPictureDone) {
        LogHelper.d(TAG, "[setNeedWaitPictureDone] mNeedWaitCaptureDone " + needWaitPictureDone);
        mRequestHandler.mNeedWaitCaptureDone = needWaitPictureDone;
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.UPDATE_G_SENSOR_ORIENTATION,
                orientation)
                .sendToTarget();
    }

    @Override
    public void closeCamera(boolean sync) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.CLOSE_CAMERA, sync)
                .sendToTarget();
        waitDone();
    }

    @Override
    public Size getPreviewSize(double targetRatio) {
        double[] values = new double[3];
        values[0] = targetRatio;
        mRequestHandler.obtainMessage(LongExposureDeviceAction.GET_PREVIEW_SIZE, values)
                .sendToTarget();
        waitDone();
        return new Size((int) values[1], (int) values[2]);
    }

    @Override
    public void setPreviewSizeReadyCallback(PreviewSizeCallback callback) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.SET_PREVIEW_SIZE_READY_CALLBACK,
                callback).sendToTarget();
    }

    @Override
    public void setPictureSize(Size size) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.SET_PICTURE_SIZE, size)
                .sendToTarget();
    }

    @Override
    public boolean isReadyForCapture() {
        boolean[] isReady = new boolean[1];
        mRequestHandler.obtainMessage(LongExposureDeviceAction.IS_READY_FOR_CAPTURE, isReady)
                .sendToTarget();
        waitDone();
        return isReady[0];
    }

    @Override
    public void requestChangeSettingValue(String key) {
        mRequestHandler.removeMessages(LongExposureDeviceAction.REQUEST_CHANGE_SETTING_VALUE);
        mRequestHandler.obtainMessage(LongExposureDeviceAction.REQUEST_CHANGE_SETTING_VALUE, key)
                .sendToTarget();
    }

    @Override
    public void requestChangeSettingValueJustSelf(String key) {
        //if the handler has the key which don't execute, need remove this.
        mRequestHandler.removeMessages(LongExposureDeviceAction
                .REQUEST_CHANGE_SETTING_VALUE_JUST_SELF, key);
        mRequestHandler.obtainMessage(LongExposureDeviceAction
                        .REQUEST_CHANGE_SETTING_VALUE_JUST_SELF,
                key).sendToTarget();
    }

    @Override
    public void requestChangeCommand(String key) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction.REQUEST_CHANGE_COMMAND, key)
                .sendToTarget();
    }

    @Override
    public void requestChangeCommandImmediately(String key) {
        mRequestHandler.obtainMessage(LongExposureDeviceAction
                .REQUEST_CHANGE_COMMAND_IMMEDIATELY, key)
                .sendToTarget();
    }

    @Override
    public void destroyDeviceController() {
        mRequestHandler.sendEmptyMessage(LongExposureDeviceAction.DESTROY_DEVICE_CONTROLLER);
    }

    private void waitDone() {
        final Object waitDoneLock = new Object();
        final Runnable unlockRunnable = new Runnable() {
            @Override
            public void run() {
                synchronized (waitDoneLock) {
                    waitDoneLock.notifyAll();
                }
            }
        };

        synchronized (waitDoneLock) {
            mRequestHandler.post(unlockRunnable);
            try {
                waitDoneLock.wait();
            } catch (InterruptedException ex) {
                LogHelper.e(TAG, "waitDone interrupted");
            }
        }
    }

    /**
     * Use for handler device control command.
     */
    private class LongExposureHandler extends Handler {
        private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
        private static final String KEY_PICTURE_SIZE = "key_picture_size";

        private static final String KEY_MANUAL_CAP_VALUES = "manual-cap-values";
        private static final String KEY_MANUAL_CAP = "manual-cap";

        private String mCameraId;
        private ISettingManager mSettingManager;
        private ISettingManager.SettingDeviceConfigurator mSettingDeviceConfigurator;
        private Object mWaitCameraOpenDone = new Object();

        private final CameraProxy.StateCallback mCameraProxyStateCallback =
                new CameraDeviceProxyStateCallback();
        private CameraProxy mCameraProxy;
        private Object mCaptureSync = new Object();
        private boolean mIsInCapturing = false;
        private boolean mNeedWaitCaptureDone = false;
        private boolean mIsPreviewStarted = false;
        private DeviceCallback mModeDeviceCallback;
        private int mPreviewFormat;
        private long mCaptureStartTime = 0;
        private JpegCallback mJpegReceivedCallback;
        private int mJpegRotation = 0;
        private volatile int mPreviewWidth;
        private volatile int mPreviewHeight;
        private PreviewSizeCallback mCameraOpenedCallback;
        private Object mSurfaceObject;
        private SettingDeviceRequester mSettingDeviceRequester;
        private boolean mNeedQuitHandler = false;

        /**
         * Long exposure device handler.
         *
         * @param looper feature looper.
         */
        public LongExposureHandler(Looper looper, SettingDeviceRequester requester) {
            super(looper);
            mSettingDeviceRequester = requester;
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (cancelDealMessage(msg.what)) {
                LogHelper.d(TAG, "[handleMessage] - msg = " +
                        LongExposureDeviceAction.stringify(msg.what) + "[dismiss]");
                return;
            }
            switch (msg.what) {
                case LongExposureDeviceAction.OPEN_CAMERA:
                    doOpenCamera((DeviceInfo) msg.obj);
                    break;
                case LongExposureDeviceAction.CLOSE_CAMERA:
                    doCloseCamera((boolean) msg.obj);
                    break;
                case LongExposureDeviceAction.START_PREVIEW:
                    doStartPreview();
                    break;
                case LongExposureDeviceAction.STOP_PREVIEW:
                    doStopPreview();
                    break;
                case LongExposureDeviceAction.TAKE_PICTURE:
                    doTakePicture((JpegCallback) msg.obj);
                    break;
                case LongExposureDeviceAction.ABORT_CAPTURE:
                    doStopPreview();
                    doStartPreview();
                    mIsInCapturing = false;
                    mNeedWaitCaptureDone = false;
                    break;
                case LongExposureDeviceAction.REQUEST_CHANGE_SETTING_VALUE:
                    String key = (String) msg.obj;
                    if (mCameraProxy == null || mCameraState == CameraState.CAMERA_UNKNOWN) {
                        LogHelper.e(TAG, "camera is closed or in opening state, can't request " +
                                "change setting value,key = " + key);
                        return;
                    }
                    doRequestChangeSettingValue(key);
                    break;
                case LongExposureDeviceAction.REQUEST_CHANGE_SETTING_VALUE_JUST_SELF:
                    String selfKey = (String) msg.obj;
                    if (mCameraProxy == null || mCameraState == CameraState.CAMERA_UNKNOWN) {
                        LogHelper.e(TAG, "camera is closed or in opening state, can't request " +
                                "change self setting value,key = " + selfKey);
                        return;
                    }
                    doRequestChangeSettingSelf(selfKey);
                    break;
                case LongExposureDeviceAction.REQUEST_CHANGE_COMMAND:
                    doRequestChangeCommand((String) msg.obj);
                    break;
                case LongExposureDeviceAction.REQUEST_CHANGE_COMMAND_IMMEDIATELY:
                    doRequestChangeCommandImmediately((String) msg.obj);
                    break;
                case LongExposureDeviceAction.SET_PICTURE_SIZE:
                    break;
                case LongExposureDeviceAction.SET_PREVIEW_CALLBACK:
                    mModeDeviceCallback = (DeviceCallback) msg.obj;
                    break;
                case LongExposureDeviceAction.SET_PREVIEW_SIZE_READY_CALLBACK:
                    mCameraOpenedCallback = (PreviewSizeCallback) msg.obj;
                    break;
                case LongExposureDeviceAction.GET_PREVIEW_SIZE:
                    doGetPreviewSize(msg);
                    break;
                case LongExposureDeviceAction.UPDATE_PREVIEW_SURFACE:
                    doUpdatePreviewSurface(msg.obj);
                    break;
                case LongExposureDeviceAction.UPDATE_G_SENSOR_ORIENTATION:
                    mJpegRotation = (Integer) msg.obj;
                    break;
                case LongExposureDeviceAction.IS_READY_FOR_CAPTURE:
                    boolean[] isReady = (boolean[]) msg.obj;
                    isReady[0] = isReadyForCapture();
                    break;
                case LongExposureDeviceAction.DESTROY_DEVICE_CONTROLLER:
                    doDestroyHandler();
                    break;

                case LongExposureDeviceAction.ON_CAMERA_OPENED:
                    doOnOpened((CameraProxy) msg.obj);
                    break;
                case LongExposureDeviceAction.ON_CAMERA_DISCONNECTED:
                    doOnDisconnected();
                    break;
                case LongExposureDeviceAction.ON_CAMERA_ERROR:
                    doOnError(msg.arg1);
                    break;
                default:
                    LogHelper.e(TAG, "[handleMessage] the message don't defined in " +
                            "LongExposureDeviceAction, need check");
                    break;
            }
        }

        private void doOpenCamera(DeviceInfo info) {
            String cameraId = info.getCameraId();
            LogHelper.i(TAG, "[doOpenCamera] id: " + cameraId + ",camera state" + " : " +
                    mCameraState);
            Preconditions.checkNotNull(cameraId);
            if (!canDoOpenCamera(cameraId)) {
                LogHelper.i(TAG, "[doOpenCamera], condition is not ready, return");
                return;
            }
            // Do open camera action.
            mCameraId = cameraId;
            mCameraState = CameraState.CAMERA_OPENING;
            mSettingManager = info.getSettingManager();
            mSettingManager.updateModeDeviceRequester(mSettingDeviceRequester);
            mSettingDeviceConfigurator = mSettingManager.getSettingDeviceConfigurator();
            try {
                mCameraDeviceManager.openCamera(mCameraId, mCameraProxyStateCallback, null);
            } catch (CameraOpenException e) {
                //need show error and finish the activity.
                if (CameraOpenException.ExceptionType.SECURITY_EXCEPTION == e.getExceptionType()) {
                    CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil
                            .CAMERA_HARDWARE_EXCEPTION);
                }
            }
        }

        private void doCloseCamera(boolean isSwitchCamera) {
            LogHelper.i(TAG, "[doCloseCamera] isSwitchCamera = " + isSwitchCamera + ",state = " +
                    mCameraState + ",camera proxy = " + mCameraProxy);
            if (CameraState.CAMERA_UNKNOWN == mCameraState) {
                mCameraId = null;
                return;
            }
            try {
                if (CameraState.CAMERA_OPENING == mCameraState) {
                    synchronized (mWaitCameraOpenDone) {
                        if (!hasDeviceStateCallback()) {
                            mWaitCameraOpenDone.wait();
                        }
                    }
                }
                mCameraState = CameraState.CAMERA_UNKNOWN;
                checkIsCapturing();
                //Must recycle the camera, otherwise when next time open the camera,
                //will not do open action because camera device proxy use the old one.
                if (mModeDeviceCallback != null) {
                    mModeDeviceCallback.beforeCloseCamera();
                }
                if (mCameraProxy != null) {
                    if (isSwitchCamera) {
                        mCameraProxy.close();
                    } else {
                        mCameraProxy.closeAsync();
                    }
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                mCameraId = null;
                mCameraProxy = null;
                mIsPreviewStarted = false;
                mSurfaceObject = null;
                if (mNeedQuitHandler) {
                    mRequestHandler.sendEmptyMessage(LongExposureDeviceAction
                            .DESTROY_DEVICE_CONTROLLER);
                }
                mIsInCapturing = false;
                mNeedWaitCaptureDone = false;
            }
        }

        private void doStartPreview() {
            if (isCameraAvailable()) {
                //set preview callback before start preview.
                mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
                // Start preview.
                mCameraProxy.startPreview();
            }
        }

        private void doStopPreview() {
            checkIsCapturing();
            if (isCameraAvailable()) {
                mSettingDeviceConfigurator.onPreviewStopped();
                if (mModeDeviceCallback != null) {
                    mModeDeviceCallback.afterStopPreview();
                }
                mIsPreviewStarted = false;
                mCameraProxy.stopPreviewAsync();
            }
            if (mNeedQuitHandler) {
                mRequestHandler.sendEmptyMessage(LongExposureDeviceAction
                        .DESTROY_DEVICE_CONTROLLER);
            }
        }

        private void doTakePicture(JpegCallback callback) {
            LogHelper.d(TAG, "[doTakePicture] mCameraProxy = " + mCameraProxy);
            if (mCameraProxy == null) {
                return;
            }
            synchronized (mCaptureSync) {
                mIsInCapturing = true;
            }
            mJpegReceivedCallback = callback;
            setCaptureParameters(mJpegRotation);
            mSettingDeviceConfigurator.onPreviewStopped();
            mIsPreviewStarted = false;
            ISettingManager.SettingController controller = mSettingManager
                    .getSettingController();
            String speed = controller.queryValue("key_shutter_speed");
            if (!LongExposureModeHelper.EXPOSURE_TIME_AUTO.equals(speed)) {
                mICameraContext.getSoundPlayback().play(ISoundPlayback.SHUTTER_CLICK);
            }
            mCaptureStartTime = System.currentTimeMillis();
            mCameraProxy.takePicture(mShutterCallback, mRawCallback, null, mJpegCallback);
        }

        private void doRequestChangeSettingValue(String key) {
            LogHelper.i(TAG, "[doRequestChangeSettingValue] key = " + key + ",mPreviewWidth = " +
                    mPreviewWidth + ",mPreviewHeight = " + mPreviewHeight);
            if (mPreviewWidth == 0 || mPreviewHeight == 0) {
                return;
            }
            if (mCameraState == CameraState.CAMERA_OPENED && mCameraProxy != null) {
                Camera.Parameters parameters = mCameraProxy.getOriginalParameters(true);
                parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);
                parameters.set(KEY_MANUAL_CAP, "on");
                boolean needRestartPreview =
                        mSettingDeviceConfigurator.configParameters(parameters);
                if (needRestartPreview) {
                    doStopPreview();
                    mCameraProxy.setParameters(parameters);
                    doStartPreview();
                } else {
                    mCameraProxy.setParameters(parameters);
                }
            }
        }

        private void doRequestChangeSettingSelf(String key) {
            LogHelper.i(TAG, "[doRequestChangeSettingSelf] key = " + key + ",mPreviewWidth = " +
                    mPreviewWidth + ",mPreviewHeight = " + mPreviewHeight);
            if (mPreviewWidth == 0 || mPreviewHeight == 0) {
                return;
            }
            if (mCameraState == CameraState.CAMERA_OPENED && mCameraProxy != null) {
                Camera.Parameters parameters = mCameraProxy.getParameters();
                boolean restart = mSettingDeviceConfigurator.configParametersByKey(parameters, key);
                if (restart) {
                    doStopPreview();
                    mCameraProxy.setParameters(parameters);
                    doStartPreview();
                } else {
                    mCameraProxy.setParameters(parameters);
                }
            }
        }

        private void doRequestChangeCommand(String key) {
            if (mCameraState == CameraState.CAMERA_OPENED && mCameraProxy != null) {
                mSettingDeviceConfigurator.configCommand(key, mCameraProxy);
            }
        }

        private void doRequestChangeCommandImmediately(String key) {
            if (mCameraState == CameraState.CAMERA_OPENED && mCameraProxy != null) {
                mSettingDeviceConfigurator.configCommand(key, mCameraProxy);
            }
        }

        private void updateTargetPreviewSize(double ratio) {
            Camera.Parameters parameters = mCameraProxy.getOriginalParameters(false);
            List<Camera.Size> previewSize = parameters.getSupportedPreviewSizes();
            int length = previewSize.size();
            List<Size> sizes = new ArrayList<>(length);
            for (int i = 0; i < length; i++) {
                sizes.add(i, new Size(previewSize.get(i).width, previewSize.get(i).height));
            }
            boolean isRotate = isDisplayRotateSupported(parameters);
            Size values = CameraUtil.getOptimalPreviewSize(mActivity, sizes, ratio, isRotate);
            mPreviewWidth = values.getWidth();
            mPreviewHeight = values.getHeight();
        }

        private void doGetPreviewSize(Message msg) {
            int oldPreviewWidth = mPreviewWidth;
            int oldPreviewHeight = mPreviewHeight;
            double[] values = (double[]) msg.obj;
            updateTargetPreviewSize(values[0]);
            values[1] = mPreviewWidth;
            values[2] = mPreviewHeight;
            boolean isSizeChanged = oldPreviewHeight != mPreviewHeight || oldPreviewWidth !=
                    mPreviewWidth;
            LogHelper.d(TAG, "[getPreviewSize], old size : " + oldPreviewWidth + " X " +
                    oldPreviewHeight + ", new  size :" + mPreviewWidth + " X " +
                    mPreviewHeight + ",is size changed: " + isSizeChanged);
            //if preview size change need do stop preview.
            if (isSizeChanged) {
                doStopPreview();
            }
        }

        private void doUpdatePreviewSurface(Object surfaceObject) {
            LogHelper.d(TAG, "[doUpdatePreviewSurface],surfaceHolder = " + surfaceObject + "," +
                    "state " + mCameraState + ",camera proxy = " + mCameraProxy);
            boolean isStateReady = CameraState.CAMERA_OPENED == mCameraState;
            if (isStateReady && mCameraProxy != null) {
                boolean onlySetSurfaceHolder = mSurfaceObject == null && surfaceObject != null;
                mSurfaceObject = surfaceObject;
                if (onlySetSurfaceHolder) {
                    setSurfaceHolderParameters();
                } else {
                    Camera.Parameters parameters = mCameraProxy.getOriginalParameters(true);
                    mSettingDeviceConfigurator.configParameters(parameters);
                    prePareAndStartPreview(parameters, false);
                }
            }
        }

        private boolean isReadyForCapture() {
            boolean value = true;
            if (mCameraProxy == null || !mIsPreviewStarted || mSurfaceObject == null) {
                value = false;
            }
            LogHelper.d(TAG, "[isReadyForCapture] proxy is null : " + (mCameraProxy == null) +
                    ",isPreview Started = " + mIsPreviewStarted);
            return value;
        }

        // Need check ,if camera is opened or camera is in opening ,don't need open it.
        // if the camera id is same as before, don't need open it again.
        private boolean canDoOpenCamera(String newCameraId) {
            boolean value = true;
            boolean isStateError = CameraState.CAMERA_UNKNOWN != mCameraState;
            boolean isSameCamera = (mCameraId != null && newCameraId.equalsIgnoreCase(mCameraId));
            if (isStateError || isSameCamera) {
                value = false;
            }
            LogHelper.d(TAG, "[canDoOpenCamera], mCameraState = " + mCameraState + ",new Camera: " +
                    newCameraId + ",current camera : " + mCameraId + ",value = " + value);
            return value;
        }

        private void checkIsCapturing() {
            LogHelper.d(TAG, "[checkIsCapturing] mIsInCapturing = " + mIsInCapturing + "," +
                    "mNeedWaitCaptureDone " + mNeedWaitCaptureDone);
            synchronized (mCaptureSync) {
                if (mIsInCapturing && mNeedWaitCaptureDone) {
                    try {
                        mCaptureSync.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        private boolean isCameraAvailable() {
            return CameraState.CAMERA_OPENED == mCameraState && mCameraProxy != null;
        }

        private final Camera.PreviewCallback mFrameworkPreviewCallback = new Camera
                .PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] bytes, Camera camera) {
                LogHelper.d(TAG, "[onPreviewFrame] mModeDeviceCallback = " + mModeDeviceCallback);
                mSettingDeviceConfigurator.onPreviewStarted();
                mIsPreviewStarted = true;
                if (mModeDeviceCallback != null) {
                    mModeDeviceCallback.onPreviewCallback(bytes, mPreviewFormat);
                }
            }
        };

        private final Camera.ShutterCallback mShutterCallback = new Camera.ShutterCallback() {
            @Override
            public void onShutter() {
                long spendTime = System.currentTimeMillis() - mCaptureStartTime;
                LogHelper.d(TAG, "[mShutterCallback], spend time : " + spendTime + "ms");
            }
        };

        private final Camera.PictureCallback mRawCallback = new Camera.PictureCallback() {
            @Override
            public void onPictureTaken(byte[] bytes, Camera camera) {
            }
        };

        private final Camera.PictureCallback mJpegCallback = new Camera.PictureCallback() {
            @Override
            public void onPictureTaken(byte[] bytes, Camera camera) {
                long jpegTime = System.currentTimeMillis() - mCaptureStartTime;
                LogHelper.d(TAG, "[mJpegCallback],spend time :" + jpegTime + "ms");
                notifyCaptureDone(bytes);
            }
        };

        private void setCaptureParameters(int sensorOrientation) {
            int rotation = CameraUtil.getJpegRotationFromDeviceSpec(Integer.parseInt(mCameraId),
                    sensorOrientation, mActivity);
            if (mCameraProxy != null) {
                Camera.Parameters parameters = mCameraProxy.getParameters();
                mSettingDeviceConfigurator.configParameters(parameters);
                parameters.setRotation(rotation);
                parameters.set(KEY_MANUAL_CAP, "on");
                LogHelper.d(TAG, "[setCaptureParameters] exposure-time " +
                        parameters.get("exposure-time"));
                mCameraProxy.setParameters(parameters);
            }
        }

        private void captureDone() {
            LogHelper.d(TAG, "[captureDone], mIsInCapturing = " + mIsInCapturing);
            synchronized (mCaptureSync) {
                if (mIsInCapturing) {
                    mIsInCapturing = false;
                    mNeedWaitCaptureDone = false;
                    LogHelper.d(TAG, "mNeedWaitCaptureDone false");
                    mCaptureSync.notify();
                }
            }
        }

        private boolean isDisplayRotateSupported(Camera.Parameters parameters) {
            String supported = parameters.get(KEY_DISP_ROT_SUPPORTED);
            if (supported == null) {
                return false;
            }
            return (new Boolean(true).toString()).equals(supported);
        }

        private void prePareAndStartPreview(Camera.Parameters parameters, boolean isFromOnOpened) {
            LogHelper.d(TAG, "[prePareAndStartPreview] state : " + mCameraState + "," +
                    "mSurfaceObject = " + mSurfaceObject);
            setSurfaceHolderParameters();
            setPreviewParameters(parameters);
            // Start preview.
            mCameraProxy.startPreview();
        }

        private void setPreviewParameters(Camera.Parameters parameters) {
            LogHelper.d(TAG, "[setPreviewParameters] mPreviewWidth = " + mPreviewWidth + "," +
                    "mPreviewHeight = " + mPreviewHeight);
            //set camera preview orientation.
            setDisplayOrientation();
            parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);
            parameters.set(KEY_MANUAL_CAP, "on");
            mCameraProxy.setParameters(parameters);
        }

        private void setDisplayOrientation() {
            int displayRotation = CameraUtil.getDisplayRotation(mActivity);
            int displayOrientation = CameraUtil.getDisplayOrientationFromDeviceSpec(displayRotation,
                    Integer.parseInt(mCameraId), mActivity);
            mCameraProxy.setDisplayOrientation(displayOrientation);
        }

        private void updatePreviewSize() {
            ISettingManager.SettingController controller = mSettingManager.getSettingController();
            String pictureSize = controller.queryValue(KEY_PICTURE_SIZE);
            if (pictureSize != null) {
                String[] pictureSizes = pictureSize.split("x");
                int width = Integer.parseInt(pictureSizes[0]);
                int height = Integer.parseInt(pictureSizes[1]);
                double ratio = (double) width / height;
                updateTargetPreviewSize(ratio);
            }
        }

        private void setSurfaceHolderParameters() {
            //set preview callback before start preview.
            if (mSurfaceObject != null) {
                mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
            }
            // Set preview display.
            try {
                if (mSurfaceObject instanceof SurfaceHolder) {
                    mCameraProxy.setPreviewDisplay((SurfaceHolder) mSurfaceObject);
                } else if (mSurfaceObject instanceof SurfaceTexture) {
                    mCameraProxy.setPreviewTexture((SurfaceTexture) mSurfaceObject);
                } else if (mSurfaceObject == null) {
                    mCameraProxy.setPreviewDisplay(null);
                }
            } catch (IOException e) {
                //if set preview exception, Can't do anything.
                throw new RuntimeException("set preview display exception");
            }
        }

        private void doOnOpened(CameraProxy cameraProxy) {
            LogHelper.i(TAG, "[doOnOpened] cameraProxy = " + cameraProxy + mCameraState);
            if (CameraState.CAMERA_OPENING != mCameraState) {
                LogHelper.d(TAG, "[doOnOpened] state is error, don't need do on camera opened");
                return;
            }
            mCameraState = CameraState.CAMERA_OPENED;
            if (mModeDeviceCallback != null) {
                mModeDeviceCallback.onCameraOpened(mCameraId);
            }
            mICameraContext.getFeatureProvider().updateCameraParameters(mCameraId,
                    cameraProxy.getOriginalParameters(false));
            mSettingManager.createAllSettings();
            mSettingDeviceConfigurator.setOriginalParameters(
                    cameraProxy.getOriginalParameters(false));
            Camera.Parameters parameters = cameraProxy.getOriginalParameters(true);
            mPreviewFormat = parameters.getPreviewFormat();
            Relation relation = LongExposureRestriction.getRestriction().getRelation("on",
                    false);
            mSettingManager.getSettingController().postRestriction(relation);
            mSettingManager.getSettingController().addViewEntry();
            mSettingManager.getSettingController().refreshViewEntry();
            mSettingDeviceConfigurator.configParameters(parameters);
            updatePreviewSize();
            if (mCameraOpenedCallback != null) {
                mCameraOpenedCallback.onPreviewSizeReady(new Size(mPreviewWidth, mPreviewHeight));
            }
            prePareAndStartPreview(parameters, false);
        }

        private void doOnDisconnected() {
            //reset the surface holder to null
            mSurfaceObject = null;
            CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);
        }

        private void doOnError(int error) {
            //reset the surface holder to null
            mSurfaceObject = null;
            mCameraState = CameraState.CAMERA_UNKNOWN;
            CameraUtil.showErrorInfoAndFinish(mActivity, error);
        }

        private void doDestroyHandler() {
            LogHelper.d(TAG, "[doDestroyHandler] mCameraState : " + mCameraState
                    + ",mIsPreviewStarted = " + mIsPreviewStarted);
            //first reset the mNeedQuitHandler to false;
            mNeedQuitHandler = false;
            if (CameraState.CAMERA_UNKNOWN == mCameraState || !mIsPreviewStarted) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                    mRequestHandler.getLooper().quitSafely();
                } else {
                    mRequestHandler.getLooper().quit();
                }
            } else {
                mNeedQuitHandler = true;
            }
        }

        private boolean cancelDealMessage(int message) {
            //if have close message in the request handler,so follow message can be cancel execute.
            boolean value = false;
            if (mRequestHandler.hasMessages(LongExposureDeviceAction.CLOSE_CAMERA)) {
                switch (message) {
                    case LongExposureDeviceAction.START_PREVIEW:
                    case LongExposureDeviceAction.STOP_PREVIEW:
                    case LongExposureDeviceAction.TAKE_PICTURE:
                    case LongExposureDeviceAction.REQUEST_CHANGE_SETTING_VALUE:
                    case LongExposureDeviceAction.SET_PICTURE_SIZE:
                    case LongExposureDeviceAction.SET_PREVIEW_CALLBACK:
                    case LongExposureDeviceAction.SET_PREVIEW_SIZE_READY_CALLBACK:
                    case LongExposureDeviceAction.GET_PREVIEW_SIZE:
                    case LongExposureDeviceAction.UPDATE_PREVIEW_SURFACE:
                    case LongExposureDeviceAction.UPDATE_G_SENSOR_ORIENTATION:
                    case LongExposureDeviceAction.REQUEST_CHANGE_COMMAND:
                        value = true;
                        break;
                    default:
                        value = false;
                        break;
                }
            }
            return value;
        }

        private boolean hasDeviceStateCallback() {
            boolean value = mRequestHandler.hasMessages(LongExposureDeviceAction.ON_CAMERA_ERROR)
                    || mRequestHandler.hasMessages(LongExposureDeviceAction.ON_CAMERA_CLOSED)
                    || mRequestHandler.hasMessages(LongExposureDeviceAction.ON_CAMERA_DISCONNECTED)
                    || mRequestHandler.hasMessages(LongExposureDeviceAction.ON_CAMERA_OPENED);
            LogHelper.d(TAG, "[hasDeviceStateCallback] value = " + value);
            return value;
        }

        private void notifyCaptureDone(byte[] data) {
            captureDone();
            if (mJpegReceivedCallback != null) {
                mJpegReceivedCallback.onDataReceived(data);
            }
        }

        /**
         * Open camera device state callback, this callback is send to camera device manager
         * by open camera interface.
         */
        private class CameraDeviceProxyStateCallback extends CameraProxy.StateCallback {

            @Override
            public void onOpened(@Nonnull CameraProxy cameraProxy) {
                LogHelper.i(TAG, "[onOpened]proxy = " + cameraProxy + " state = " + mCameraState);
                synchronized (mWaitCameraOpenDone) {
                    mCameraProxy = cameraProxy;
                    mWaitCameraOpenDone.notifyAll();
                    mRequestHandler.obtainMessage(LongExposureDeviceAction.ON_CAMERA_OPENED,
                            cameraProxy)
                            .sendToTarget();
                }
            }

            @Override
            public void onClosed(@Nonnull CameraProxy cameraProxy) {
            }

            @Override
            public void onDisconnected(@Nonnull CameraProxy cameraProxy) {
            }

            @Override
            public void onError(@Nonnull CameraProxy cameraProxy, int error) {
                LogHelper.i(TAG, "[onError] current proxy : " + mCameraProxy + " error " + error +
                        " proxy " + cameraProxy);
                //if current is in capturing, but close is wait capture done,
                //so this case need notify the capture done. otherwise will be ANR to pause.
                captureDone();
                if ((mCameraProxy != null && mCameraProxy == cameraProxy)
                        || error == CameraUtil.CAMERA_OPEN_FAIL) {
                    synchronized (mWaitCameraOpenDone) {
                        mCameraState = CameraState.CAMERA_UNKNOWN;
                        mWaitCameraOpenDone.notifyAll();
                        mRequestHandler.obtainMessage(LongExposureDeviceAction.ON_CAMERA_ERROR,
                                error, 0,
                                cameraProxy).sendToTarget();
                    }
                }
            }
        }
    }
}
