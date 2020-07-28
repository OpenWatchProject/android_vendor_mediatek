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

package com.mediatek.camera.common.mode.photo.device;

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
import com.mediatek.camera.common.mode.photo.DeviceInfo;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import javax.annotation.Nonnull;

/**
 * Photo device controller.
 */
public class PhotoDeviceController implements IDeviceController, SettingDeviceRequester {
    private static final Tag TAG = new Tag(PhotoDeviceController.class.getSimpleName());
    // notify for Image before compress when taking capture
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_IMAGE_UNCOMPRESSED = 0x00000017;
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

    private Handler mRequestHandler;

    /**
     * Controller camera device
     *
     * @param activity current activity.
     * @param context  current camera context.
     */
    PhotoDeviceController(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        HandlerThread handlerThread = new HandlerThread("DeviceController");
        handlerThread.start();
        mRequestHandler = new PhotoDeviceHandler(handlerThread.getLooper(), this);
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
        boolean sync = info.getNeedOpenCameraSync();
        mRequestHandler.obtainMessage(PhotoDeviceAction.OPEN_CAMERA, info).sendToTarget();
        if (sync) {
            waitDone();
        }
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.UPDATE_PREVIEW_SURFACE, surfaceObject)
                .sendToTarget();
    }

    @Override
    public void setDeviceCallback(DeviceCallback callback) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.SET_PREVIEW_CALLBACK, callback)
                .sendToTarget();
    }

    @Override
    public void startPreview() {
        mRequestHandler.sendEmptyMessage(PhotoDeviceAction.START_PREVIEW);
    }

    @Override
    public void stopPreview() {
        mRequestHandler.sendEmptyMessage(PhotoDeviceAction.STOP_PREVIEW);
        waitDone();
    }

    @Override
    public void takePicture(@Nonnull IDeviceController.CaptureDataCallback callback) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.TAKE_PICTURE, callback).sendToTarget();
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.UPDATE_G_SENSOR_ORIENTATION, orientation)
                .sendToTarget();
    }

    @Override
    public void closeSession() {

    }

    @Override
    public void closeCamera(boolean sync) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.CLOSE_CAMERA, sync == true ? 1 : 0)
                .sendToTarget();
        waitDone();
    }

    @Override
    public Size getPreviewSize(double targetRatio) {
        double[] values = new double[3];
        values[0] = targetRatio;
        mRequestHandler.obtainMessage(PhotoDeviceAction.GET_PREVIEW_SIZE, values).sendToTarget();
        waitDone();
        return new Size((int) values[1], (int) values[2]);
    }

    @Override
    public void setPreviewSizeReadyCallback(PreviewSizeCallback callback) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.SET_PREVIEW_SIZE_READY_CALLBACK,
                callback).sendToTarget();
    }

    @Override
    public void setPictureSize(Size size) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.SET_PICTURE_SIZE, size).sendToTarget();
    }

    @Override
    public boolean isReadyForCapture() {
        boolean[] isReady = new boolean[1];
        mRequestHandler.obtainMessage(PhotoDeviceAction.IS_READY_FOR_CAPTURE, isReady)
                .sendToTarget();
        waitDone();
        return isReady[0];
    }

    @Override
    public void requestChangeSettingValue(String key) {
        mRequestHandler.removeMessages(PhotoDeviceAction.REQUEST_CHANGE_SETTING_VALUE);
        mRequestHandler.obtainMessage(PhotoDeviceAction.REQUEST_CHANGE_SETTING_VALUE, key)
                .sendToTarget();
    }

    @Override
    public void requestChangeSettingValueJustSelf(String key) {
        //if the handler has the key which don't execute, need remove this.
        mRequestHandler.removeMessages(PhotoDeviceAction
                .REQUEST_CHANGE_SETTING_VALUE_JUST_SELF, key);
        mRequestHandler.obtainMessage(PhotoDeviceAction.REQUEST_CHANGE_SETTING_VALUE_JUST_SELF,
                key).sendToTarget();
    }

    @Override
    public void requestChangeCommand(String key) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.REQUEST_CHANGE_COMMAND, key).sendToTarget();
    }

    @Override
    public void requestChangeCommandImmediately(String key) {
        mRequestHandler.obtainMessage(PhotoDeviceAction.REQUEST_CHANGE_COMMAND_IMMEDIATELY, key)
                .sendToTarget();
    }

    @Override
    public void destroyDeviceController() {
        mRequestHandler.sendEmptyMessage(PhotoDeviceAction.DESTROY_DEVICE_CONTROLLER);
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

    @Override
    public void setZSDStatus(String value) {

    }

    @Override
    public void setFormat(String value) {

    }

    @Override
    public void setSavedDataCallback(CaptureImageSavedCallback callback) {
    }
    /**
     * Use for handler device control command.
     */
    private class PhotoDeviceHandler extends Handler {
        private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
        private static final String KEY_PICTURE_SIZE = "key_picture_size";
        private static final String KEY_CONTINUOUS_SHOT = "key_continuous_shot";
        private static final String KEY_ZSD = "key_zsd";
        private static final String FALSE = "false";
        private static final String VALUE_ON = "on";
        private String mCameraId;
        private ISettingManager mSettingManager;
        private ISettingManager.SettingDeviceConfigurator mSettingDeviceConfigurator;
        private Object mWaitCameraOpenDone = new Object();

        private final CameraProxy.StateCallback mCameraProxyStateCallback =
                new CameraDeviceProxyStateCallback();
        private AtomicInteger mPostViewCallbackNumber = new AtomicInteger(0);
        private AtomicInteger mP2DoneCallBackCount = new AtomicInteger(0);
        private AtomicInteger  mCaptureCount = new AtomicInteger(0);
        private CameraProxy mCameraProxy;
        private Object mCaptureSync = new Object();
        private boolean mIsPreviewStarted = false;
        private DeviceCallback mModeDeviceCallback;
        private int mPreviewFormat;
        private long mCaptureStartTime = 0;
        private CaptureDataCallback mJpegReceivedCallback;
        private int mJpegRotation = 0;

        private volatile int mPreviewWidth;
        private volatile int mPreviewHeight;
        private PreviewSizeCallback mCameraOpenedCallback;
        private Object mSurfaceObject;
        private SettingDeviceRequester mSettingDeviceRequester;
        private boolean mNeedSubSectionInitSetting = false;
        private boolean mNeedQuitHandler = false;
        private boolean mIsNeedStartPreviewAfterCapture = false;


        /**
         * Photo device handler.
         *
         * @param looper current looper.
         */
        public PhotoDeviceHandler(Looper looper, SettingDeviceRequester requester) {
            super(looper);
            mSettingDeviceRequester = requester;
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (cancelDealMessage(msg.what)) {
                LogHelper.d(TAG, "[handleMessage] - msg = " +
                        PhotoDeviceAction.stringify(msg.what) + "[dismiss]");
                return;
            }
            switch (msg.what) {
                case PhotoDeviceAction.OPEN_CAMERA:
                    doOpenCamera((DeviceInfo) msg.obj);
                    break;
                case PhotoDeviceAction.CLOSE_CAMERA:
                    doCloseCamera((Integer) msg.obj == 1 ? true : false);
                    break;
                case PhotoDeviceAction.START_PREVIEW:
                    doStartPreview();
                    break;
                case PhotoDeviceAction.STOP_PREVIEW:
                    doStopPreview();
                    break;
                case PhotoDeviceAction.TAKE_PICTURE:
                    doTakePicture((CaptureDataCallback) msg.obj);
                    break;
                case PhotoDeviceAction.REQUEST_CHANGE_SETTING_VALUE:
                    String key = (String) msg.obj;
                    restoreStateForCShot(key);
                    if (mCameraProxy == null || mCameraState == CameraState.CAMERA_UNKNOWN) {
                        LogHelper.e(TAG, "camera is closed or in opening state, can't request " +
                                "change setting value,key = " + key);
                        return;
                    }
                    doRequestChangeSettingValue(key);
                    break;
                case PhotoDeviceAction.REQUEST_CHANGE_SETTING_VALUE_JUST_SELF:
                    String selfKey = (String) msg.obj;
                    if (mCameraProxy == null || mCameraState == CameraState.CAMERA_UNKNOWN) {
                        LogHelper.e(TAG, "camera is closed or in opening state, can't request " +
                                "change self setting value,key = " + selfKey);
                        return;
                    }
                    doRequestChangeSettingSelf(selfKey);
                    break;
                case PhotoDeviceAction.REQUEST_CHANGE_COMMAND:
                    doRequestChangeCommand((String) msg.obj);
                    break;
                case PhotoDeviceAction.REQUEST_CHANGE_COMMAND_IMMEDIATELY:
                    doRequestChangeCommandImmediately((String) msg.obj);
                    break;
                case PhotoDeviceAction.SET_PICTURE_SIZE:
                    doSetPictureSize((Size) msg.obj);
                    break;
                case PhotoDeviceAction.SET_PREVIEW_CALLBACK:
                    mModeDeviceCallback = (DeviceCallback) msg.obj;
                    break;
                case PhotoDeviceAction.SET_PREVIEW_SIZE_READY_CALLBACK:
                    mCameraOpenedCallback = (PreviewSizeCallback) msg.obj;
                    break;
                case PhotoDeviceAction.GET_PREVIEW_SIZE:
                    doGetPreviewSize(msg);
                    break;
                case PhotoDeviceAction.UPDATE_PREVIEW_SURFACE:
                    doUpdatePreviewSurface(msg.obj);
                    break;
                case PhotoDeviceAction.UPDATE_G_SENSOR_ORIENTATION:
                    mJpegRotation = (Integer) msg.obj;
                    break;
                case PhotoDeviceAction.IS_READY_FOR_CAPTURE:
                    boolean[] isReady = (boolean[]) msg.obj;
                    isReady[0] = isReadyForCapture();
                    break;
                case PhotoDeviceAction.DESTROY_DEVICE_CONTROLLER:
                    doDestroyHandler();
                    break;

                case PhotoDeviceAction.ON_CAMERA_OPENED:
                    doOnOpened((CameraProxy) msg.obj);
                    break;
                case PhotoDeviceAction.ON_CAMERA_DISCONNECTED:
                    doOnDisconnected();
                    break;
                case PhotoDeviceAction.ON_CAMERA_ERROR:
                    doOnError(msg.arg1);
                    break;
                default:
                    LogHelper.e(TAG, "[handleMessage] the message don't defined in " +
                            "photodeviceaction, need check");
                    break;
            }
        }

        private void doOpenCamera(DeviceInfo info) {
            String cameraId = info.getCameraId();
            boolean sync = info.getNeedOpenCameraSync();
            LogHelper.i(TAG, "[doOpenCamera] id: " + cameraId + ", sync = " + sync + ",camera " +
                    "state" + " : " + mCameraState);
            Preconditions.checkNotNull(cameraId);
            if (!canDoOpenCamera(cameraId)) {
                LogHelper.i(TAG, "[doOpenCamera], condition is not ready, return");
                return;
            }
            // Do open camera action.
            mCameraId = cameraId;
            mNeedSubSectionInitSetting = info.getNeedFastStartPreview();
            mCameraState = CameraState.CAMERA_OPENING;
            mSettingManager = info.getSettingManager();
            mSettingManager.updateModeDeviceRequester(mSettingDeviceRequester);
            mSettingDeviceConfigurator = mSettingManager.getSettingDeviceConfigurator();
            resetCountNumber();
            try {
                //when open the camera need reset the mCameraProxy to null
                if (sync) {
                    mCameraDeviceManager.openCameraSync(mCameraId, mCameraProxyStateCallback, null);
                } else {
                    mCameraDeviceManager.openCamera(mCameraId, mCameraProxyStateCallback, null);
                }
            }  catch (CameraOpenException e) {
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
                    mRequestHandler.sendEmptyMessage(PhotoDeviceAction.DESTROY_DEVICE_CONTROLLER);
                }
                resetCountNumber();
            }
        }

        private void doStartPreview() {
            if (isCameraAvailable()) {
                //set preview callback before start preview.
                mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
                // Start preview.
                mCameraProxy.startPreview();
                mCameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_IMAGE_UNCOMPRESSED,
                        mUncompressedImageCallback);
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
                mRequestHandler.sendEmptyMessage(PhotoDeviceAction.DESTROY_DEVICE_CONTROLLER);
            }
            resetCountNumber();
        }

        private void doTakePicture(CaptureDataCallback callback) {
            LogHelper.d(TAG, "[doTakePicture] mCameraProxy = " + mCameraProxy);
            if (mCameraProxy == null) {
                return;
            }
            mCaptureStartTime = System.currentTimeMillis();
            mJpegReceivedCallback = callback;
            setCaptureParameters(mJpegRotation);
            mSettingDeviceConfigurator.onPreviewStopped();
            mIsPreviewStarted = false;
            mIsNeedStartPreviewAfterCapture = true;
            mCaptureCount.incrementAndGet();
            mCameraProxy.takePicture(mShutterCallback, mRawCallback, mPostViewCallback,
                    mJpegCallback);
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

        private void doSetPictureSize(Size size) {
            //do nothing.
        }

        private Size getTargetPreviewSize(double ratio) {
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

            return values;
        }

        private void doGetPreviewSize(Message msg) {
            int oldPreviewWidth = mPreviewWidth;
            int oldPreviewHeight = mPreviewHeight;
            double[] values = (double[]) msg.obj;
            getTargetPreviewSize(values[0]);
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

        /**
         * During capturing (mIsInCapturing is true) if start CShot will lead photo device
         * can't receive jpeg call back and cause ANR.
         * @param key
         */
        private void restoreStateForCShot(String key) {
            if (KEY_CONTINUOUS_SHOT.equals(key)) {
                synchronized (mCaptureSync) {
                    if (mCaptureCount.get() > 0) {
                        mCaptureCount.set(0);
                        mCaptureSync.notify();
                    }
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
            LogHelper.d(TAG, "[checkIsCapturing] mCaptureCount = " + mCaptureCount.get());
            synchronized (mCaptureSync) {
                if (mCaptureCount.get() > 0) {
                    try {
                        LogHelper.d(TAG, "[checkIsCapturing] wait +");
                        mCaptureSync.wait();
                        LogHelper.d(TAG, "[checkIsCapturing] wait -");
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

        /**
         * Uncompressed image data callback.
         */
        private final CameraProxy.VendorDataCallback mUncompressedImageCallback
                = new CameraProxy.VendorDataCallback() {

            @Override
            public void onDataTaken(Message message) {
                LogHelper.d(TAG, "[onDataTaken] message = " + message.what);
            }

            @Override
            public void onDataCallback(int msgId, byte[] data, int arg1, int arg2) {
                LogHelper.d(TAG, "[UncompressedImageCallback] onDataCallback " + data);
                //if current is in capturing, also need notify the capture sync.
                //because jpeg will be callback next time.
                if (mJpegReceivedCallback != null) {
                    DataCallbackInfo info = new DataCallbackInfo();
                    info.data = data;
                    info.needUpdateThumbnail = false;
                    info.needRestartPreview = false;
                    mJpegReceivedCallback.onDataReceived(info);
                }
                mCameraProxy.startPreview();
                if (mFrameworkPreviewCallback != null) {
                    mFrameworkPreviewCallback.onPreviewFrame(null, null);
                }
                mIsNeedStartPreviewAfterCapture = false;
                mP2DoneCallBackCount.incrementAndGet();
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

        private final Camera.PictureCallback mPostViewCallback = new Camera.PictureCallback() {
            @Override
            public void onPictureTaken(byte[] bytes, Camera camera) {
                long postViewTime = System.currentTimeMillis() - mCaptureStartTime;
                LogHelper.d(TAG, "[mPostViewCallback],spend time : " + postViewTime + "ms," +
                        "data : " + bytes + ",mPostViewCallbackNumber = " +
                        mPostViewCallbackNumber.get());
                if (bytes != null) {
                    mPostViewCallbackNumber.incrementAndGet();
                    if (mJpegReceivedCallback != null) {
                        mJpegReceivedCallback.onPostViewCallback(bytes);
                    }
                }
            }
        };

        private final Camera.PictureCallback mJpegCallback = new Camera.PictureCallback() {
            @Override
            public void onPictureTaken(byte[] bytes, Camera camera) {
                long jpegTime = System.currentTimeMillis() - mCaptureStartTime;
                LogHelper.d(TAG, "[mJpegCallback],spend time :" + jpegTime + "ms" + "," +
                        "mPostViewCallbackNumber = " + mPostViewCallbackNumber.get()
                        + " mP2DoneCallBackCount = " + mP2DoneCallBackCount.get()
                        + " mIsNeedStartPreviewAfterCapture = " + mIsNeedStartPreviewAfterCapture
                        + " mCaptureCount = " + mCaptureCount.get());
                mCaptureCount.decrementAndGet();
                boolean startPreview
                        = mIsNeedStartPreviewAfterCapture && mP2DoneCallBackCount.get() == 0;
                if (mP2DoneCallBackCount.get() > 0) {
                    mP2DoneCallBackCount.decrementAndGet();
                }
                notifyCaptureDone(bytes, mPostViewCallbackNumber.get() == 0, startPreview);
                if (mPostViewCallbackNumber.get() > 0) {
                    mPostViewCallbackNumber.decrementAndGet();
                }
            }
        };

        private void setCaptureParameters(int sensorOrientation) {
            int rotation = CameraUtil.getJpegRotationFromDeviceSpec(Integer.parseInt(mCameraId),
                    sensorOrientation, mActivity);
            if (mCameraProxy != null) {
                Camera.Parameters parameters = mCameraProxy.getParameters();
                mSettingDeviceConfigurator.configParameters(parameters);
                Size thumbnailSize = CameraUtil.getSizeByTargetSize(
                        parameters.getSupportedJpegThumbnailSizes(),
                        parameters.getPictureSize(), true);
                if (thumbnailSize != null
                        && thumbnailSize.getWidth() != 0
                        && thumbnailSize.getHeight() != 0) {
                    parameters.setJpegThumbnailSize(thumbnailSize.getWidth(),
                            thumbnailSize.getHeight());
                }
                parameters.setRotation(rotation);
                mCameraProxy.setParameters(parameters);
            }
        }

        private void captureDone() {
            LogHelper.d(TAG, "[captureDone], mCaptureCount = " + mCaptureCount.get());
            if (mCaptureCount.get() == 0) {
                synchronized (mCaptureSync) {
                    mCaptureSync.notify();
                }
            }
        }

        private boolean isDisplayRotateSupported(Camera.Parameters parameters) {
            String supported = parameters.get(KEY_DISP_ROT_SUPPORTED);
            if (supported == null || FALSE.equals(supported)) {
                return false;
            }
            return true;
        }

        private void prePareAndStartPreview(Camera.Parameters parameters, boolean isFromOnOpened) {
            LogHelper.d(TAG, "[prePareAndStartPreview] state : " + mCameraState + "," +
                    "mSurfaceObject = " + mSurfaceObject);
            setSurfaceHolderParameters();
            setPreviewParameters(parameters);
            // Start preview.
            mCameraProxy.startPreview();
            mCameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_IMAGE_UNCOMPRESSED,
                    mUncompressedImageCallback);
            if (isFromOnOpened) {
                createSettingSecond(parameters);
            }
        }

        private void setPreviewParameters(Camera.Parameters parameters) {
            LogHelper.d(TAG, "[setPreviewParameters] mPreviewWidth = " + mPreviewWidth + "," +
                    "mPreviewHeight = " + mPreviewHeight);
            //set camera preview orientation.
            setDisplayOrientation();
            parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);
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
                getTargetPreviewSize(ratio);
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

        private void createSettingSecond(Camera.Parameters parameters) {
            mSettingManager.createSettingsByStage(2);
            mSettingDeviceConfigurator.setOriginalParameters(parameters);
            boolean needRestartPreview = mSettingDeviceConfigurator.configParameters(parameters);
            if (needRestartPreview) {
                mCameraProxy.stopPreview();
                mCameraProxy.setParameters(parameters);
                mCameraProxy.startPreview();
            } else {
                mCameraProxy.setParameters(parameters);
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
            if (mNeedSubSectionInitSetting) {
                mSettingManager.createSettingsByStage(1);
            } else {
                mSettingManager.createAllSettings();
            }
            mSettingDeviceConfigurator.setOriginalParameters(
                    cameraProxy.getOriginalParameters(false));

            Camera.Parameters parameters = cameraProxy.getOriginalParameters(true);
            mPreviewFormat = parameters.getPreviewFormat();
            mSettingDeviceConfigurator.configParameters(parameters);
            updatePreviewSize();
            if (mCameraOpenedCallback != null) {
                mCameraOpenedCallback.onPreviewSizeReady(new Size(mPreviewWidth, mPreviewHeight));
            }
            prePareAndStartPreview(parameters, mNeedSubSectionInitSetting);
            mSettingManager.getSettingController().addViewEntry();
            mSettingManager.getSettingController().refreshViewEntry();
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
            if (mRequestHandler.hasMessages(PhotoDeviceAction.CLOSE_CAMERA)) {
                switch (message) {
                    case PhotoDeviceAction.START_PREVIEW:
                    case PhotoDeviceAction.STOP_PREVIEW:
                    case PhotoDeviceAction.TAKE_PICTURE:
                    case PhotoDeviceAction.REQUEST_CHANGE_SETTING_VALUE:
                    case PhotoDeviceAction.SET_PICTURE_SIZE:
                    case PhotoDeviceAction.SET_PREVIEW_CALLBACK:
                    case PhotoDeviceAction.SET_PREVIEW_SIZE_READY_CALLBACK:
                    case PhotoDeviceAction.GET_PREVIEW_SIZE:
                    case PhotoDeviceAction.UPDATE_PREVIEW_SURFACE:
                    case PhotoDeviceAction.UPDATE_G_SENSOR_ORIENTATION:
                    case PhotoDeviceAction.REQUEST_CHANGE_COMMAND:
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
            boolean value = mRequestHandler.hasMessages(PhotoDeviceAction.ON_CAMERA_ERROR)
                    || mRequestHandler.hasMessages(PhotoDeviceAction.ON_CAMERA_CLOSED)
                    || mRequestHandler.hasMessages(PhotoDeviceAction.ON_CAMERA_DISCONNECTED)
                    || mRequestHandler.hasMessages(PhotoDeviceAction.ON_CAMERA_OPENED);
            LogHelper.d(TAG, "[hasDeviceStateCallback] value = " + value);
            return value;
        }


        private void notifyCaptureDone(byte[] data, boolean needUpdateThumbnail, boolean
                needRestartPreview) {
            captureDone();
            if (mJpegReceivedCallback != null) {
                DataCallbackInfo info = new DataCallbackInfo();
                info.data = data;
                info.needUpdateThumbnail = needUpdateThumbnail;
                info.needRestartPreview = needRestartPreview;
                mJpegReceivedCallback.onDataReceived(info);
            }
        }

        private void resetCountNumber() {
            mP2DoneCallBackCount.set(0);
            mCaptureCount.set(0);
            mPostViewCallbackNumber.set(0);
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
                    mRequestHandler.obtainMessage(PhotoDeviceAction.ON_CAMERA_OPENED, cameraProxy)
                            .sendToTarget();
                }
            }

            @Override
            public void onClosed(@Nonnull CameraProxy cameraProxy) {
                LogHelper.i(TAG, "[onClosed] current proxy : " + mCameraProxy + " closed proxy " +
                        "= " + cameraProxy);
                if (mCameraProxy != null && mCameraProxy == cameraProxy) {
                    synchronized (mWaitCameraOpenDone) {
                        mWaitCameraOpenDone.notifyAll();
                    }
                }
            }

            @Override
            public void onDisconnected(@Nonnull CameraProxy cameraProxy) {
                LogHelper.i(TAG, "[onDisconnected] current proxy : " + mCameraProxy + " closed " +
                        " proxy " + cameraProxy);
                mCaptureCount.set(0);
                captureDone();
                if (mCameraProxy != null && mCameraProxy == cameraProxy) {
                    synchronized (mWaitCameraOpenDone) {
                        mCameraState = CameraState.CAMERA_UNKNOWN;
                        mWaitCameraOpenDone.notifyAll();
                        mRequestHandler.obtainMessage(PhotoDeviceAction.ON_CAMERA_DISCONNECTED,
                                cameraProxy).sendToTarget();
                    }
                }
            }

            @Override
            public void onError(@Nonnull CameraProxy cameraProxy, int error) {
                LogHelper.i(TAG, "[onError] current proxy : " + mCameraProxy + " error " + error +
                        " proxy " + cameraProxy);
                //if current is in capturing, but close is wait capture done,
                //so this case need notify the capture done. otherwise will be ANR to pause.
                mCaptureCount.set(0);
                captureDone();
                if ((mCameraProxy != null && mCameraProxy == cameraProxy)
                        || error == CameraUtil.CAMERA_OPEN_FAIL) {
                    synchronized (mWaitCameraOpenDone) {
                        mCameraState = CameraState.CAMERA_UNKNOWN;
                        mWaitCameraOpenDone.notifyAll();
                        mRequestHandler.obtainMessage(PhotoDeviceAction.ON_CAMERA_ERROR, error, 0,
                                cameraProxy).sendToTarget();
                    }
                }
            }
        }
    }
}
