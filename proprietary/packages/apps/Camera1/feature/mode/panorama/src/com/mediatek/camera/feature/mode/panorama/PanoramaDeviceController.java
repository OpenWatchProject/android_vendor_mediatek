/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 *     the prior written permission of MediaTek inc. and/or its licensor, any
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
 *     NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.os.Build;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
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
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceConfigurator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;

import javax.annotation.Nonnull;

/**
 * An implementation of {@link IPanoramaDeviceController} with CameraProxy.
 */
@SuppressWarnings("deprecation")
class PanoramaDeviceController implements IPanoramaDeviceController,
        ISettingManager.SettingDeviceRequester {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(PanoramaDeviceController.class
            .getSimpleName());
    private static final int MTK_CAMERA_COMMAND_START_AUTORAMA = 0x10000009;
    private static final int MTK_CAMERA_COMMAND_STOP_AUTORAMA = 0x1000000A;
    private static final int MTK_CAMERA_MSG_EXT_DATA_AUTORAMA = 0x00000001;
    private static final int OPEN_CAMERA = 1;
    private static final int REQUEST_CHANGE_SETTING_VALUE = 2;
    private static final int REQUEST_CHANGE_COMMAND = 3;
    private static final int UPDATE_PREVIEW_SURFACE = 4;
    private static final int SET_PREVIEW_CALLBACK = 5;
    private static final int START_PREVIEW = 6;
    private static final int STOP_PREVIEW = 7;
    private static final int UPDATE_G_SENSOR_ORIENTATION = 8;
    private static final int GET_PREVIEW_SIZE = 9;
    private static final int SET_AUTORAMA_CALLBACK = 10;
    private static final int START_AUTORAMA = 11;
    private static final int STOP_AUTORAMA = 12;
    private static final int CLOSE_CAMERA = 13;
    private static final int CONFIG_PARAMETERS = 14;
    private static final int SET_CAMERA_STATE_CALLBACK = 15;
    private static final int DESTROY_DEVICE_CONTROLLER = 16;
    private static final int SET_PREVIEW_SIZE_READY_CALLBACK = 17;
    private static final String KEY_CAPTURE_MODE = "cap-mode";
    private static final String CAPTURE_MODE_PANORAMA_SHOT = "autorama";
    private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final String FALSE = "false";

    private final Activity mActivity;
    private final ICameraContext mCameraContext;
    private final StateCallback mCameraProxyStateCallback = new CameraDeviceProxyStateCallback();
    private volatile CameraProxy mCameraProxy;
    private String mCameraId;
    private CameraDeviceManager mCameraDeviceManager;
    private Object mSurfaceObject;
    private Object mSurfaceHolderSync = new Object();
    private ConditionVariable mWaitOpened = new ConditionVariable();
    private volatile CameraState mCameraState = CameraState.CAMERA_UNKNOWN;
    private volatile int mPreviewWidth;
    private volatile int mPreviewHeight;
    private int mJpegRotation = 0;
    private final Semaphore mCameraOpenCloseLock = new Semaphore(1, true);
    private PreviewCallback mPreviewCallback;
    private int mPreviewFormat;
    private ISettingManager mSettingManager;
    private SettingDeviceConfigurator mSettingDeviceConfigurator;
    private Handler mRequestHandler;
    private PreviewSizeCallback mCameraOpenedCallback;

    /**
     * this enum is used for tag native camera open state.
     */
    private enum CameraState {
        CAMERA_UNKNOWN, //initialize state.
        CAMERA_OPENING, //between open camera and open done callback.
        CAMERA_OPENED, //when camera open done.
    }

    private CameraStateCallback mCameraStateCallback;

    /**
     * PhotoDeviceController may use activity to get display rotation.
     *
     * @param activity the camera activity.
     */
    PanoramaDeviceController(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        Preconditions.checkNotNull(activity);
        Preconditions.checkNotNull(context);
        HandlerThread handlerThread = new HandlerThread("PanoramaDeviceController");
        handlerThread.start();
        mRequestHandler = new PanoramaDeviceHandler(handlerThread.getLooper());
        mActivity = activity;
        mCameraContext = context;
        mCameraDeviceManager = context.getDeviceManager(CameraApi.API1);
    }

    @Override
    public void requestChangeSettingValue(String key) {
        mRequestHandler.removeMessages(REQUEST_CHANGE_SETTING_VALUE);
        mRequestHandler.obtainMessage(REQUEST_CHANGE_SETTING_VALUE, key).sendToTarget();
    }

    @Override
    public void requestChangeCommand(String key) {
        mRequestHandler.obtainMessage(REQUEST_CHANGE_COMMAND, key).sendToTarget();
    }

    /**
     * Request to change Parameters and just configure it by setting.
     *
     * @param key The key of setting.
     */
    @Override
    public void requestChangeSettingValueJustSelf(String key) {

    }

    @Override
    public void requestChangeCommandImmediately(String key) {
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mCameraContext.getDeviceManager(CameraApi.API1);
    }

    @Override
    public void openCamera(PanoramaDeviceInfo panoramaDeviceInfo) {
        LogHelper.d(TAG, "[openCamera]");
        boolean sync = panoramaDeviceInfo.getNeedOpenCameraSync();
        mRequestHandler.obtainMessage(OPEN_CAMERA, panoramaDeviceInfo).sendToTarget();
        if (sync) {
            waitDone();
        }
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        mRequestHandler.obtainMessage(UPDATE_PREVIEW_SURFACE, surfaceObject).sendToTarget();
    }

    @Override
    public void setPreviewCallback(final PreviewCallback callback) {
        mRequestHandler.obtainMessage(SET_PREVIEW_CALLBACK, callback).sendToTarget();
    }

    @Override
    public void setCameraStateCallback(CameraStateCallback callback) {
        mRequestHandler.obtainMessage(SET_CAMERA_STATE_CALLBACK, callback).sendToTarget();
    }


    @Override
    public void setPreviewSizeReadyCallback(PreviewSizeCallback callback) {
        mRequestHandler.obtainMessage(SET_PREVIEW_SIZE_READY_CALLBACK,
                callback).sendToTarget();
    }

    @Override
    public void startPreview() {
        if (isCameraAvailable()) {
            mCameraProxy.startPreview();
            mSettingDeviceConfigurator.onPreviewStarted();
            mCameraStateCallback.onCameraPreviewStarted();
        }
    }

    @Override
    public void stopPreview() {
        mRequestHandler.obtainMessage(STOP_PREVIEW).sendToTarget();
        waitDone();
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mRequestHandler.obtainMessage(UPDATE_G_SENSOR_ORIENTATION, orientation).sendToTarget();
    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.d(TAG, "[closeCamera]");
        mRequestHandler.obtainMessage(CLOSE_CAMERA, sync ? 1 : 0).sendToTarget();
        waitDone();
    }

    @Override
    public Size getPreviewSize(double targetRatio) {
        double[] values = new double[3];
        values[0] = targetRatio;
        mRequestHandler.obtainMessage(GET_PREVIEW_SIZE, values).sendToTarget();
        waitDone();
        return new Size((int) values[1], (int) values[2]);
    }

    @Override
    public void setAutoRamaCallback(CameraProxy.VendorDataCallback panoramaCallback) {
        mRequestHandler.obtainMessage(SET_AUTORAMA_CALLBACK, panoramaCallback)
                .sendToTarget();
    }

    @Override
    public void startAutoRama(int num) {
        mRequestHandler.obtainMessage(START_AUTORAMA, num).sendToTarget();
    }

    @Override
    public void stopAutoRama(boolean merge) {
        mRequestHandler.obtainMessage(STOP_AUTORAMA, (merge ? 1 : 0)).sendToTarget();
    }

    @Override
    public void configParameters() {
        mRequestHandler.obtainMessage(CONFIG_PARAMETERS).sendToTarget();
    }

    @Override
    public void destroyDeviceController() {
        mRequestHandler.sendEmptyMessage(DESTROY_DEVICE_CONTROLLER);
    }

    /**
     * The panorama device handler.
     */
    private class PanoramaDeviceHandler extends Handler {
        public PanoramaDeviceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage] msg = " + stringify(msg.what));
            super.handleMessage(msg);

            if (isNeedRemoveNoUsedMessage(msg.what)) {
                LogHelper.d(TAG, "[handleMessage] - msg = " + stringify(msg.what) + "[dismiss]");
                return;
            }
            switch (msg.what) {
                case OPEN_CAMERA:
                    doOpenCamera((PanoramaDeviceInfo) msg.obj);
                    break;

                case REQUEST_CHANGE_SETTING_VALUE:
                    doRequestChangeSettingValue((String) msg.obj);
                    break;

                case REQUEST_CHANGE_COMMAND:
                    doRequestChangeCommand((String) msg.obj);
                    break;

                case UPDATE_PREVIEW_SURFACE:
                    doUpdatePreviewSurface(msg.obj);
                    break;

                case SET_PREVIEW_CALLBACK:
                    doSetPreviewCallback((PreviewCallback) msg.obj);
                    break;

                case START_PREVIEW:
                    break;

                case STOP_PREVIEW:
                    doStopPreview();
                    break;

                case UPDATE_G_SENSOR_ORIENTATION:
                    mJpegRotation = (Integer) msg.obj;
                    break;

                case GET_PREVIEW_SIZE:
                    doGetPreviewSize(msg);
                    break;

                case SET_AUTORAMA_CALLBACK:
                    doSetAutoRamaCallback((CameraProxy.VendorDataCallback) msg.obj);
                    break;

                case START_AUTORAMA:
                    doStartAutoRama((Integer) msg.obj);
                    break;

                case STOP_AUTORAMA:
                    doStopAutoRama((Integer) msg.obj == 1 ? true : false);
                    break;

                case CLOSE_CAMERA:
                    doCloseCamera((Integer) msg.obj == 1 ? true : false);
                    break;

                case CONFIG_PARAMETERS:
                    doConfigParameters();
                    break;

                case SET_CAMERA_STATE_CALLBACK:
                    doSetCameraStateCallback((CameraStateCallback) msg.obj);
                    break;

                case DESTROY_DEVICE_CONTROLLER:
                    doDestroyHandler();
                    break;
                case SET_PREVIEW_SIZE_READY_CALLBACK:
                    mCameraOpenedCallback = (PreviewSizeCallback) msg.obj;
                    break;
                default:
                    LogHelper.e(TAG, "[handleMessage] the message is not defined");
                    break;
            }
        }
    }

    private boolean isNeedRemoveNoUsedMessage(int message) {
        //if close message in the request handler, the follow message should be cancel execute.
        boolean value = false;
        if (mRequestHandler.hasMessages(CLOSE_CAMERA)) {
            switch (message) {
                case REQUEST_CHANGE_SETTING_VALUE:
                case REQUEST_CHANGE_COMMAND:
                case UPDATE_PREVIEW_SURFACE:
                case SET_PREVIEW_CALLBACK:
                case START_PREVIEW:
                case STOP_PREVIEW:
                case UPDATE_G_SENSOR_ORIENTATION:
                case GET_PREVIEW_SIZE:
                case SET_AUTORAMA_CALLBACK:
                case START_AUTORAMA:
                case STOP_AUTORAMA:
                case CONFIG_PARAMETERS:
                    value = true;
                    break;
                default:
                    break;
            }
        }
        return value;
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

    private static String stringify(int operation) {
        switch (operation) {
            case OPEN_CAMERA:
                return "OPEN_CAMERA";
            case REQUEST_CHANGE_SETTING_VALUE:
                return "REQUEST_CHANGE_SETTING_VALUE";
            case REQUEST_CHANGE_COMMAND:
                return "REQUEST_CHANGE_COMMAND";
            case UPDATE_PREVIEW_SURFACE:
                return "UPDATE_PREVIEW_SURFACE";
            case SET_PREVIEW_CALLBACK:
                return "SET_PREVIEW_CALLBACK";
            case START_PREVIEW:
                return "START_PREVIEW";
            case STOP_PREVIEW:
                return "STOP_PREVIEW";
            case UPDATE_G_SENSOR_ORIENTATION:
                return "UPDATE_G_SENSOR_ORIENTATION";
            case GET_PREVIEW_SIZE:
                return "GET_PREVIEW_SIZE";
            case SET_AUTORAMA_CALLBACK:
                return "SET_AUTORAMA_CALLBACK";
            case START_AUTORAMA:
                return "START_AUTORAMA";
            case STOP_AUTORAMA:
                return "STOP_AUTORAMA";
            case CLOSE_CAMERA:
                return "CLOSE_CAMERA";
            case CONFIG_PARAMETERS:
                return "CONFIG_PARAMETERS";
            case SET_CAMERA_STATE_CALLBACK:
                return "SET_CAMERA_STATE_CALLBACK";
            case DESTROY_DEVICE_CONTROLLER:
                return "DESTROY_DEVICE_CONTROLLER";

            default:
                return "UNKNOWN(" + operation + ")";
        }
    }

    private void doRequestChangeSettingValue(String key) {
        LogHelper.i(TAG, "[doRequestChangeSettingValue] key = " + key + ",mPreviewWidth = " +
                mPreviewWidth + ",mPreviewHeight = " + mPreviewHeight);
        if (mPreviewWidth == 0 || mPreviewHeight == 0) {
            LogHelper.e(TAG, "[doRequestChangeSettingValue] there maybe some error request this" +
                    ".Please check");
            return;
        }
        if (mCameraState == CameraState.CAMERA_OPENED && mCameraProxy != null) {
            Parameters parameters = mCameraProxy.getOriginalParameters(true);
            parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);
            boolean needRestartPreview = mSettingDeviceConfigurator.
                    configParameters(parameters);
            if (needRestartPreview) {
                doStopPreview();
                mCameraProxy.setParameters(parameters);
                startPreview();
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

    private void doUpdatePreviewSurface(Object surfaceObject) {
        LogHelper.d(TAG, "doUpdatePreviewSurface,surfaceHolder = " + surfaceObject + ",state : " +
                mCameraState + ",camera proxy = " + mCameraProxy);
        synchronized (mSurfaceHolderSync) {
            mSurfaceObject = surfaceObject;
            boolean isStateReady = CameraState.CAMERA_OPENED == mCameraState;
            if (isStateReady && mCameraProxy != null) {
                if (surfaceObject != null) {
                    doStartPreview(surfaceObject, mCameraProxy.getParameters());
                } else {
                    doStopPreview();
                }
            }
        }
    }

    private void doSetPreviewCallback(final PreviewCallback callback) {
        LogHelper.i(TAG, "doSetPreviewCallback,callback = " + callback + ", mPreviewCallback = " +
                mPreviewCallback);
        mPreviewCallback = callback;
        if (mPreviewCallback == null && mCameraProxy != null) {
            //clear the callback
            mCameraProxy.setOneShotPreviewCallback(null);
        }
    }

    private void doStopPreview() {
        LogHelper.i(TAG, "[doStopPreview]");
        if (isCameraAvailable()) {
            mCameraProxy.stopPreview();
            // here set preview display to force camera service to disconnect bq
            try {
                mCameraProxy.setPreviewDisplay(null);
            } catch (IOException e) {
                e.printStackTrace();
            }
            mSettingDeviceConfigurator.onPreviewStopped();
            mCameraStateCallback.onCameraPreviewStopped();
        }
    }

    private void doOpenCamera(PanoramaDeviceInfo info) {
        String cameraId = info.getCameraId();
        boolean sync = info.getNeedOpenCameraSync();
        LogHelper.i(TAG, "[doOpenCamera] + id: " + cameraId + ", sync = " + sync
                + ",camera state : " + mCameraState);
        Preconditions.checkNotNull(cameraId);
        if (!canDoOpenCamera(cameraId)) {
            LogHelper.i(TAG, "[doOpenCamera], condition is not ready, return");
            return;
        }

        mSettingManager = info.getSettingManager();
        mSettingManager.updateModeDeviceRequester(this);
        mSettingDeviceConfigurator = mSettingManager.getSettingDeviceConfigurator();
        // Do open camera action.
        mCameraId = cameraId;
        mCameraState = CameraState.CAMERA_OPENING;
        try {
            //when open the camera need reset the mCameraProxy to null
            mWaitOpened.close();
            mCameraOpenCloseLock.acquireUninterruptibly();
            if (sync) {
                mCameraDeviceManager.openCameraSync(mCameraId, mCameraProxyStateCallback, null);
            } else {
                mCameraDeviceManager.openCamera(mCameraId, mCameraProxyStateCallback, null);
            }
        } catch (CameraOpenException e) {
            //need show error and finish the activity.
            if (CameraOpenException.ExceptionType.SECURITY_EXCEPTION == e.getExceptionType()) {
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_HARDWARE_EXCEPTION);
            }
            mCameraOpenCloseLock.release();
        }
        LogHelper.i(TAG, "[doOpenCamera] -");
    }

    private void doCloseCamera(boolean sync) {
        LogHelper.i(TAG, "[doCloseCamera] + sync = " + sync + ",state = " + mCameraState + "," +
                "camera proxy = " + mCameraProxy);
        if (CameraState.CAMERA_UNKNOWN == mCameraState) {
            LogHelper.d(TAG, "[doCloseCamera]+, camera have closed or open failed,return");
            return;
        }
        mWaitOpened.block();
        mCameraState = CameraState.CAMERA_UNKNOWN;
        mCameraOpenCloseLock.acquireUninterruptibly();
        mCameraStateCallback.beforeCloseCamera();
        if (mCameraProxy != null) {
            if (sync) {
                mCameraProxy.close();
            } else {
                mCameraProxy.closeAsync();
            }
        }
        mCameraOpenCloseLock.release();
        mCameraId = null;
        mCameraProxy = null;
        mSurfaceObject = null;
        LogHelper.i(TAG, "[doCloseCamera] -");
    }

    private void doStartPreview(Object surfaceHolder, Parameters parameters) {
        LogHelper.d(TAG, "[doStartPreview] state : " + mCameraState);
        //set preview callback before start preview.
        mCameraProxy.setOneShotPreviewCallback(mFrameworkPreviewCallback);
        // Set preview display.
        try {
            if (mSurfaceObject instanceof SurfaceHolder) {
                mCameraProxy.setPreviewDisplay((SurfaceHolder) surfaceHolder);
            } else if (mSurfaceObject instanceof SurfaceTexture) {
                mCameraProxy.setPreviewTexture((SurfaceTexture) surfaceHolder);
            } else if (surfaceHolder == null) {
                mCameraProxy.setPreviewDisplay(null);
            }
        } catch (IOException e) {
            //if set preview exception, Can't do anything.
            throw new RuntimeException("set preview display exception");
        }
        setPreviewParameters(parameters);
        // Start preview.
        startPreview();
    }

    private void doSetAutoRamaCallback(CameraProxy.VendorDataCallback panoramaCallback) {
        LogHelper.d(TAG, "[doSetAutoRamaCallback]");
        if (panoramaCallback != null) {
            setCaptureParameters(mJpegRotation);
        }
        if (mCameraProxy != null) {
            mCameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_DATA_AUTORAMA, panoramaCallback);
        }
    }

    private void doStartAutoRama(int num) {
        LogHelper.d(TAG, "[doStartAutoRama]");
        if (mCameraProxy != null) {
            mCameraProxy.getParameters().set(KEY_CAPTURE_MODE, CAPTURE_MODE_PANORAMA_SHOT);
            mCameraProxy.sendCommand(MTK_CAMERA_COMMAND_START_AUTORAMA, num, 0);
        }
    }

    private void doStopAutoRama(boolean merge) {
        LogHelper.d(TAG, "[doStopAutoRama]");
        if (mCameraProxy != null) {
            mCameraProxy.sendCommand(MTK_CAMERA_COMMAND_STOP_AUTORAMA, (merge ? 1 : 0), 0);
        }
    }

    private void doConfigParameters() {
        Parameters parameters = mCameraProxy.getParameters();
        mSettingDeviceConfigurator.configParameters(parameters);
        mCameraProxy.setParameters(parameters);
    }

    private void doSetCameraStateCallback(CameraStateCallback callback) {
        mCameraStateCallback = callback;
    }

    private void doDestroyHandler() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
            mRequestHandler.getLooper().quitSafely();
        } else {
            mRequestHandler.getLooper().quit();
        }
    }

    private void setDisplayOrientation() {
        int displayRotation = CameraUtil.getDisplayRotation(mActivity);
        int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation, Integer
                .parseInt(mCameraId));
        mCameraProxy.setDisplayOrientation(displayOrientation);
        LogHelper.d(TAG, "[setDisplayOrientation],Rotation  = " + displayRotation + ",Orientation" +
                " = " +
                displayOrientation);
    }

    private void setCaptureParameters(int sensorOrientation) {
        int rotation = CameraUtil.getJpegRotation(Integer.parseInt(mCameraId), sensorOrientation);
        if (mCameraProxy != null) {
            Parameters parameters = mCameraProxy.getParameters();
            parameters.setRotation(rotation);
            mCameraProxy.setParameters(parameters);
        }
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

    private Size getTargetPreviewSize(double ratio) {
        Parameters parameters = mCameraProxy.getOriginalParameters(false);
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
        LogHelper.d(TAG, "[getTargetPreviewSize] " + mPreviewWidth + " X " + mPreviewHeight);
        return values;
    }

    private void setPreviewParameters(Parameters parameters) {
        LogHelper.d(TAG, "[setPreviewParameters] mPreviewWidth = " + mPreviewWidth + "," +
                "mPreviewHeight = " + mPreviewHeight);
        //set camera preview orientation.
        setDisplayOrientation();
        parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);
        mCameraProxy.setParameters(parameters);
    }

    private final Camera.PreviewCallback mFrameworkPreviewCallback = new Camera.PreviewCallback() {
        @Override
        public void onPreviewFrame(byte[] bytes, Camera camera) {
            if (mPreviewCallback != null) {
                mPreviewCallback.onPreviewCallback(bytes, mPreviewFormat);
            }
        }
    };

    private boolean isDisplayRotateSupported(Parameters parameters) {
        String supported = parameters.get(KEY_DISP_ROT_SUPPORTED);
        if (supported == null || FALSE.equals(supported)) {
            return false;
        }
        return true;
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

    private boolean isCameraAvailable() {
        return mCameraProxy != null;
    }

    /**
     * Open camera device state callback, this callback is send to camera device manager
     * by open camera interface.
     */
    private class CameraDeviceProxyStateCallback extends StateCallback {

        @Override
        public void onOpened(@Nonnull CameraProxy cameraProxy) {
            LogHelper.i(TAG, "[onOpened],proxy = " + cameraProxy +
                    ",mSurfaceObject = " + mSurfaceObject + ",state = " + mCameraState);
            mCameraProxy = cameraProxy;
            if (mCameraState == CameraState.CAMERA_UNKNOWN) {
                LogHelper.i(TAG, "[onOpened],current state is unknown,maybe close is coming");
                mCameraOpenCloseLock.release();
                return;
            }
            mCameraState = CameraState.CAMERA_OPENED;
            mCameraContext.getFeatureProvider().updateCameraParameters(mCameraId,
                    cameraProxy.getOriginalParameters(false));
            mSettingManager.createAllSettings();
            mSettingDeviceConfigurator.setOriginalParameters(
                    cameraProxy.getOriginalParameters(false));

            Parameters parameters = cameraProxy.getOriginalParameters(true);
            mPreviewFormat = parameters.getPreviewFormat();

            synchronized (mSurfaceHolderSync) {
                mCameraStateCallback.onCameraOpened();
                /**
                 * Apply parameters.
                 */
                mSettingDeviceConfigurator.configParameters(parameters);
                //update preview size
                updatePreviewSize();
                if (mCameraOpenedCallback != null) {
                    mCameraOpenedCallback.onPreviewSizeReady(
                                      new Size(mPreviewWidth, mPreviewHeight));
                }
                if (mPreviewHeight != 0 && mPreviewHeight != 0) {
                    //also need set the preview size into the parameters.
                    LogHelper.d(TAG, "[onOpened],mPreviewWidth = " + mPreviewWidth +
                            ", mPreviewHeight = " + mPreviewHeight);
                    parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);
                }
                mCameraProxy.setParameters(parameters);

                if (mSurfaceObject != null) {
                    doStartPreview(mSurfaceObject, parameters);
                }
            }
            mCameraOpenCloseLock.release();
            mWaitOpened.open();
        }

        @Override
        public void onClosed(@Nonnull CameraProxy cameraProxy) {
            LogHelper.i(TAG, "[onClosed], proxy = " + cameraProxy);
            if (mCameraProxy != null && mCameraProxy == cameraProxy) {
                //clear the Semaphore
                mCameraOpenCloseLock.release();
            }
            //reset the surface holder to null
            synchronized (mSurfaceHolderSync) {
                mSurfaceObject = null;
            }
            mWaitOpened.open();
        }

        @Override
        public void onDisconnected(@Nonnull CameraProxy cameraProxy) {
            LogHelper.i(TAG, "[onDisconnected]+, proxy = " + cameraProxy);
            mCameraState = CameraState.CAMERA_UNKNOWN;
            //clear the Semaphore
            mCameraOpenCloseLock.release();
            //reset the surface holder to null
            synchronized (mSurfaceHolderSync) {
                mSurfaceObject = null;
            }
            mWaitOpened.open();
            CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);
        }

        @Override
        public void onError(@Nonnull CameraProxy cameraProxy, int error) {
            LogHelper.i(TAG, "[onError]+, proxy = " + cameraProxy + ",error = " + error);
            mCameraState = CameraState.CAMERA_UNKNOWN;
            mCameraOpenCloseLock.release();
            //reset the surface holder to null
            synchronized (mSurfaceHolderSync) {
                mSurfaceObject = null;
            }
            mWaitOpened.open();
            CameraUtil.showErrorInfoAndFinish(mActivity, error);
        }
    }
}
