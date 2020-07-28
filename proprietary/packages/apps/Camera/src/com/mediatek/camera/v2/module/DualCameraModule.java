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

package com.mediatek.camera.v2.module;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.CaptureResult;

import android.hardware.camera2.TotalCaptureResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.view.OrientationEventListener;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.control.ControlImpl;
import com.mediatek.camera.v2.control.IControl.IAaaController;
import com.mediatek.camera.v2.detection.DetectionManager;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.platform.device.CameraDeviceManager;
import com.mediatek.camera.v2.platform.device.CameraDeviceManager.CameraStateCallback;
import com.mediatek.camera.v2.platform.device.CameraDeviceProxy;
import com.mediatek.camera.v2.platform.device.CameraDeviceProxy.CameraSessionCallback;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DualCameraModule extends AbstractCameraModule {
    private static final Tag TAG = new Tag(DualCameraModule.class.getSimpleName());
    private Handler mCameraHandler;
    private HandlerThread mCameraHandlerThread;
    private CameraDeviceManager mCameraManager;
    private volatile CameraDeviceProxy mMainCameraDevice;
    private volatile CameraDeviceProxy mSubCameraDevice;

    private boolean mIsDeviceNeedSwitch = false;
    private ControlImpl mSubAaaControl;
    private DetectionManager mSubCamDetectionManager;

    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    private RequestType mCurrentRepeatingRequest = RequestType.PREVIEW;
    private volatile boolean mIsCameraModulePaused = false;

    public DualCameraModule(AppController app) {
        super(app);
        mDetectionManager = new DetectionManager(app, this, SettingCtrl.BACK_CAMERA);
        mAaaControl = new ControlImpl(app, this, true, SettingCtrl.BACK_CAMERA);

        mSubCamDetectionManager = new DetectionManager(app, this, SettingCtrl.FRONT_CAMERA);
        mSubAaaControl = new ControlImpl(app, this, true, SettingCtrl.FRONT_CAMERA);
    }

    @Override
    public void open(Activity activity, boolean isSecureCamera,
            boolean isCaptureIntent) {
        mSubAaaControl.open(activity, mAppUi.getModuleLayoutRoot(), isCaptureIntent);
        mSubCamDetectionManager.open(activity, mAppUi.getModuleLayoutRoot(), isCaptureIntent);
        super.open(activity, isSecureCamera, isCaptureIntent);
        initializeCameraStaticInfo(activity);

        // initialize module ui
        mAbstractModuleUI = new DualCameraModuleUi(activity, this,
                mAppUi.getModuleLayoutRoot(), mStreamManager.getPreviewCallback());
        mAppController.setModuleUiListener(mAbstractModuleUI);
        mAppController.addPreviewAreaSizeChangedListener(this);
    }

    @Override
    public void close() {
        LogHelper.i(TAG, "[close]+");
        super.close();
        mSubAaaControl.close();
        mSubCamDetectionManager.close();

        mAppController.removePreviewAreaSizeChangedListener(this);
        mIsDeviceNeedSwitch = false;

        LogHelper.i(TAG, "[close]-");
    }

    @Override
    public void resume() {
        LogHelper.i(TAG, "[resume]+");
        super.resume();
        mSubAaaControl.resume();
        mSubCamDetectionManager.resume();
        mIsCameraModulePaused = false;

        mCameraHandlerThread = new HandlerThread("DualCameraModule.CameraHandler");
        mCameraHandlerThread.start();
        mCameraHandler = new Handler(mCameraHandlerThread.getLooper());

        HandlerThread tmpThread_openCam = new HandlerThread("tmpThread_openCam");
        tmpThread_openCam.start();
        (new Handler(tmpThread_openCam.getLooper())).post(new Runnable() {
            @Override
            public void run() {
                openCamera();
            }
        });
        LogHelper.i(TAG, "[resume]-");
    }

    @Override
    public void pause() {
        LogHelper.i(TAG, "[pause]+");
        mIsCameraModulePaused = true;
        mCurrentMode.onActivityPause();
        closeCamera();
        super.pause();
        mSubAaaControl.pause();
        mSubCamDetectionManager.pause();
        LogHelper.i(TAG, "[pause]-");
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        super.onPreviewAreaChanged(previewArea);
        mSubAaaControl.onPreviewAreaChanged(previewArea);
        mSubCamDetectionManager.onPreviewAreaChanged(previewArea);
    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        super.onSettingChanged(result);
        boolean includeCameraId = result != null && result.containsKey(SettingKeys.KEY_CAMERA_ID);
        if (includeCameraId) {
            String cameraId = result.get(SettingKeys.KEY_CAMERA_ID);
            if (cameraId != mCameraId) {
                mIsDeviceNeedSwitch = !mIsDeviceNeedSwitch;
                mCameraId = cameraId;
                requestChangeCaptureRequets(false/*sync*/,
                        getDefaultRepeatingRequest(), CaptureType.REPEATING_REQUEST);
                LogHelper.i(TAG, "onSettingChanged cameraId:" + mCameraId);
            }
        }
    }

    @Override
    public void onCameraPicked(String newCameraId) {
        LogHelper.i(TAG, "onCameraPicked newCameraId: " + newCameraId);
        super.onCameraPicked(newCameraId);
        mCurrentMode.switchCamera(newCameraId);
    }

    @Override
    public void onLayoutOrientationChanged(boolean isLandscape) {

    }

    @Override
    public void onOrientationChanged(int orientation) {
        // We keep the last known orientation. So if the user first orient
        // the camera then point the camera to floor or sky, we still have
        // the correct orientation.
        if (orientation == OrientationEventListener.ORIENTATION_UNKNOWN) {
            return;
        }
        int newOrientation = Utils.roundOrientation(orientation, mOrientation);
        if (mOrientation != newOrientation) {
            mOrientation = newOrientation;
        }

        mAbstractModuleUI.onOrientationChanged(mOrientation);

        mAaaControl.onOrientationChanged(newOrientation);
        mSubAaaControl.onOrientationChanged(newOrientation);
        mDetectionManager.onOrientationChanged(newOrientation);
        mSubCamDetectionManager.onOrientationChanged(newOrientation);

        mCurrentMode.onOrientationChanged(newOrientation);
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        if (mPaused || mMainCameraDevice == null || mSubCameraDevice == null) {
            LogHelper.i(TAG, "[onSingleTapUp]- mPaused : " + mPaused);
            return false;
        }
        if (super.onSingleTapUp(x, y)) {
            return true;
        }
        if (mIsDeviceNeedSwitch) {
            mSubAaaControl.onSingleTapUp(x, y);
            mSubCamDetectionManager.onSingleTapUp(x, y);
        } else {
            mDetectionManager.onSingleTapUp(x, y);
            mAaaControl.onSingleTapUp(x, y);
        }
        LogHelper.i(TAG, "[onSingleTapUp]-");
        return false;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        LogHelper.i(TAG, "onLongPress x = " + x + " y = " + y);
        if (super.onLongPress(x, y)) {
            return true;
        }
        if (mIsDeviceNeedSwitch) {
            mSubCamDetectionManager.onLongPressed(x, y);
        } else {
            mDetectionManager.onLongPressed(x, y);
        }
        return false;
    }

    @Override
    public void requestChangeCaptureRequets(boolean sync, RequestType requestType,
            CaptureType captureType) {
        super.requestChangeCaptureRequets(sync, requestType, captureType);
        requestChangeCaptureRequets(true, sync, requestType, captureType);
        requestChangeCaptureRequets(false, sync, requestType, captureType);
    }

    @Override
    public void requestChangeCaptureRequets(boolean isMainCamera,
            boolean sync, RequestType requestType,
            CaptureType captureType) {
        if (RequestType.RECORDING == requestType || RequestType.PREVIEW == requestType) {
            mCurrentRepeatingRequest = requestType;
        }
        if (isMainCamera) {
            if (mMainCameraDevice == null) {
                LogHelper.i(TAG, "requestChangeCaptureRequets but main camera is null!");
                return;
            }
            mMainCameraDevice.requestChangeCaptureRequets(sync, requestType, captureType);
        } else {
            if (mSubCameraDevice == null) {
                LogHelper.i(TAG, "requestChangeCaptureRequets but sub camera is null!");
                return;
            }
            mSubCameraDevice.requestChangeCaptureRequets(sync, requestType, captureType);
        }
    }

    @Override
    public void requestChangeSessionOutputs(boolean sync) {
        requestChangeSessionOutputs(sync, true);
        requestChangeSessionOutputs(sync, false);
    }

    @Override
    public void requestChangeSessionOutputs(boolean sync, boolean isMainCamera) {
        if (isMainCamera) {
            if (mMainCameraDevice == null) {
                LogHelper.i(TAG, "requestChangeSessionOutputs but main camera is null!");
                return;
            }
            mMainCameraDevice.requestChangeSessionOutputs(sync);
        } else {
            if (mSubCameraDevice == null) {
                LogHelper.i(TAG, "requestChangeSessionOutputs but sub camera is null!");
                return;
            }
            mSubCameraDevice.requestChangeSessionOutputs(sync);
        }
    }

    @Override
    public IAaaController get3AController(String cameraId) {
        if (SettingCtrl.BACK_CAMERA.equals(cameraId)) {
            return mAaaControl;
        } else {
            return mSubAaaControl;
        }
    }

    @Override
    public RequestType getRepeatingRequestType() {
        return getDefaultRepeatingRequest();
    }

    private CameraStateCallback mMainCamStateCallback = new CameraStateCallback() {
        @Override
        public void onOpened(CameraDeviceProxy camera) {
            LogHelper.d(TAG, "[mMainCamStateCallback.onOpened]");
            mPendingSwitchCameraId = UNKNOWN;
            mMainCameraDevice = camera;
            requestChangeSessionOutputs(false, true);
        }

        @Override
        public void onError(int error) {
            showErrorAndFinish(error);
        }
    };

    private CameraSessionCallback mMainCamSessionCallback = new CameraSessionCallback() {
        @Override
        public void onSessionConfigured() {
            /**
             * only send main camera repeating request, because sub camera's session may
             * not re-configured.
             */
            requestChangeCaptureRequets(true, true, getDefaultRepeatingRequest(),
                    CaptureType.REPEATING_REQUEST);
        }

        @Override
        public void onSessionActive() {

        }

        @Override
        public CaptureCallback configuringSessionRequests(Builder requestBuilder,
                RequestType requestType, CaptureType captureType) {
            CaptureCallback captureCallback = null;
            switch (requestType) {
            case RECORDING:
            case PREVIEW:
                captureCallback = mMainSessionCapProgressCallback;
                break;
            case STILL_CAPTURE:
                if (!mIsDeviceNeedSwitch) {
                    captureCallback = mCurrentMode.getCaptureCallback();
                }
                break;
            default:
                break;
            }

            // apply crop region
            requestBuilder.set(CaptureRequest.SCALER_CROP_REGION, Utils.cropRegionForZoom(
                    mAppController.getActivity(),
                    SettingCtrl.BACK_CAMERA,
                    1f));
            Map<RequestType, CaptureRequest.Builder> requestBuilders =
                    new HashMap<RequestType, CaptureRequest.Builder>();
            requestBuilders.put(requestType, requestBuilder);
            // 1. apply 3A control
            mAaaControl.configuringSessionRequests(requestBuilders, captureType, true);
            // 2. apply addition parameter
            mDetectionManager.configuringSessionRequests(requestBuilders, captureType);
            // 3. apply mode parameter
            mCurrentMode.configuringSessionRequests(requestBuilders, true);

            return captureCallback;
        }

        @Override
        public void configuringSessionOutputs(List<Surface> sessionOutputSurfaces) {
            mCurrentMode.configuringSessionOutputs(sessionOutputSurfaces, true);
        }
    };

    private CaptureCallback mMainSessionCapProgressCallback = new CaptureCallback() {
        @Override
        public void onCaptureStarted(CameraCaptureSession session,
                CaptureRequest request, long timestamp, long frameNumber) {
            mAaaControl.onPreviewCaptureStarted(request, timestamp, frameNumber);
            mDetectionManager.onCaptureStarted(request, timestamp, frameNumber);
            mCurrentMode.onPreviewCaptureStarted(request, timestamp, frameNumber);
        }

        public void onCaptureProgressed(CameraCaptureSession session,
                CaptureRequest request, CaptureResult partialResult) {
            mAaaControl.onPreviewCaptureProgressed(request, partialResult);
        };

        @Override
        public void onCaptureCompleted(CameraCaptureSession session,
                CaptureRequest request, TotalCaptureResult result) {
            mAaaControl.onPreviewCaptureCompleted(request, result);
            mDetectionManager.onCaptureCompleted(request, result);
            mCurrentMode.onPreviewCaptureCompleted(request, result);
        }
    };

    private CameraStateCallback mSubCamStateCallback = new CameraStateCallback() {
        @Override
        public void onOpened(CameraDeviceProxy camera) {
            LogHelper.d(TAG, "[mSubCamStateCallback.onOpened]");
            mPendingSwitchCameraId = UNKNOWN;
            mSubCameraDevice = camera;
            requestChangeSessionOutputs(false, false);
        }

        @Override
        public void onError(int error) {
            showErrorAndFinish(error);
        }
    };

    private CameraSessionCallback mSubCamSessionCallback = new CameraSessionCallback() {

        @Override
        public void onSessionConfigured() {
           /**
             * only send sub camera repeating request, because main camera's session may
             * not re-configured.
             */
            requestChangeCaptureRequets(false, true,
                    getDefaultRepeatingRequest(), CaptureType.REPEATING_REQUEST);
        }

        @Override
        public void onSessionActive() {

        }

        @Override
        public CaptureCallback configuringSessionRequests(Builder requestBuilder,
                RequestType requestType, CaptureType captureType) {
            CaptureCallback captureCallback = null;
            switch (requestType) {
            case RECORDING:
            case PREVIEW:
                captureCallback = mSubSessionCapProgressCallback;
                break;
            case STILL_CAPTURE:
                if (mIsDeviceNeedSwitch) {
                    captureCallback = mCurrentMode.getCaptureCallback();
                }
                break;
            default:
                break;
            }
             // apply crop region
            requestBuilder.set(CaptureRequest.SCALER_CROP_REGION, Utils.cropRegionForZoom(
                    mAppController.getActivity(),
                    SettingCtrl.FRONT_CAMERA,
                    1f));
            Map<RequestType, CaptureRequest.Builder> requestBuilders
                = new HashMap<RequestType, CaptureRequest.Builder>();
            requestBuilders.put(requestType, requestBuilder);
            // 1. apply 3A control
            mSubAaaControl.configuringSessionRequests(requestBuilders, captureType, true);
            // 2. apply addition parameter
            mSubCamDetectionManager.configuringSessionRequests(requestBuilders, captureType);
            // 3. apply mode parameter
            mCurrentMode.configuringSessionRequests(requestBuilders, false);

            return captureCallback;
        }

        @Override
        public void configuringSessionOutputs(List<Surface> sessionOutputSurfaces) {
            mCurrentMode.configuringSessionOutputs(sessionOutputSurfaces, false);
        }
    };

    private CaptureCallback mSubSessionCapProgressCallback = new CaptureCallback() {
        @Override
        public void onCaptureStarted(CameraCaptureSession session,
                CaptureRequest request, long timestamp, long frameNumber) {
            mSubAaaControl.onPreviewCaptureStarted(request, timestamp, frameNumber);
            mSubCamDetectionManager.onCaptureStarted(request, timestamp, frameNumber);
            mCurrentMode.onPreviewCaptureStarted(request, timestamp, frameNumber);
        }

        public void onCaptureProgressed(CameraCaptureSession session,
                CaptureRequest request, CaptureResult partialResult) {
            mSubAaaControl.onPreviewCaptureProgressed(request, partialResult);
        };

        @Override
        public void onCaptureCompleted(CameraCaptureSession session,
                CaptureRequest request, TotalCaptureResult result) {
            mSubAaaControl.onPreviewCaptureCompleted(request, result);
            mSubCamDetectionManager.onCaptureCompleted(request, result);
            mCurrentMode.onPreviewCaptureCompleted(request, result);
        }
    };

    private void initializeCameraStaticInfo(Activity activity) {
        mCameraManager  = mAppController.getCameraManager();
        mCameraId = mSettingController.getCurrentCameraId();
        mIsDeviceNeedSwitch = (mCameraId == SettingCtrl.FRONT_CAMERA);
        LogHelper.i(TAG, "initializeCameraStaticInfo mCameraId = " + mCameraId);
    }

    private void openCamera() {
        LogHelper.i(TAG, "[openCamera]+, mIsCameraModulePaused:" + mIsCameraModulePaused);
        if (mCameraManager == null) {
            throw new IllegalStateException("openCamera, but CameraManager is null!");
        }
        acquireOpenCloseLock();
        // if module paused, don't need prepare surface and open camera.
        // multi-thread, so repeat check pause status
        if (!mIsCameraModulePaused) {
            mCurrentMode.prepareSurfaceBeforeOpenCamera();
        }
        if (!mIsCameraModulePaused) {
            mCameraManager.openSync(SettingCtrl.BACK_CAMERA,
                    mMainCamStateCallback, mMainCamSessionCallback, mCameraHandler);
        }
        if (!mIsCameraModulePaused) {
            mCameraManager.openSync(SettingCtrl.FRONT_CAMERA,
                    mSubCamStateCallback, mSubCamSessionCallback, mCameraHandler);
        }
        releaseOpenCloseLock();
        LogHelper.i(TAG, "[openCamera]-");
    }

    private void closeCamera() {
        LogHelper.i(TAG, "[closeCamera]+, mMainCameraDevice:" + mMainCameraDevice +
                ",mSubCameraDevice:" + mSubCameraDevice);
        acquireOpenCloseLock();
        if (mMainCameraDevice != null) {
            mMainCameraDevice.close();
            mMainCameraDevice = null;
        }
        if (mSubCameraDevice != null) {
            mSubCameraDevice.close();
            mSubCameraDevice = null;
        }
        releaseOpenCloseLock();
        LogHelper.i(TAG, "[closeCamera]-");
    }

    private RequestType getDefaultRepeatingRequest() {
        RequestType requiredRequestType = RequestType.PREVIEW;
        if (mCurrentRepeatingRequest == RequestType.RECORDING) {
            requiredRequestType = RequestType.RECORDING;
        }
        return requiredRequestType;
    }
}