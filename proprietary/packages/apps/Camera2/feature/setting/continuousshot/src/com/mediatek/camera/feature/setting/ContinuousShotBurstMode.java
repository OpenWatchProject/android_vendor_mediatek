/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.setting;

import android.annotation.TargetApi;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.IAppUiListener.OnShutterButtonListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.bgservice.CaptureSurface;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ICameraSetting.ICaptureRequestConfigure;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.feature.setting.CsState.State;
import com.mediatek.camera.portability.SystemProperties;

import junit.framework.Assert;
import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;


/**
 * This is used for API2 continuous shot.
 */

@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class ContinuousShotBurstMode extends ContinuousShotBase implements ICaptureRequestConfigure,
        OnShutterButtonListener {

    private static final Tag TAG = new Tag(ContinuousShotBurstMode.class.getSimpleName());
    private static final int[] mCaptureMode = new int[]{1};
    private static final int MAX_BURST_NUM = 10;
    private int ROUND_NUMBER = 2;
    private CsState mState;
    private CaptureRequest.Key<int[]> mKeyCsCaptureRequest;
    private final Object mNumberLock = new Object();
    private volatile int mImageCallbackNumber = 0;
    private boolean mIsCshotSupported = false;

    @Override
    public void init(IApp app,
            ICameraContext cameraContext,
            ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mState = new CsState();
        mState.updateState(State.STATE_INIT);
        MAX_CAPTURE_NUMBER = ROUND_NUMBER * MAX_BURST_NUM;
    }

    @Override
    public void unInit() {
        super.unInit();
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        return this;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        super.overrideValues(headerKey, currentValue, supportValues);
        LogHelper.d(TAG, "[overrideValues] getValue() = " + getValue() + ", headerKey = "
                + headerKey + ", currentValue = " + currentValue + ", supportValues  = "
                + supportValues);
        mIsCshotSupported = CONTINUOUSSHOT_ON.equals(getValue());
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        //front camera, not support
        if (characteristics.get(CameraCharacteristics.LENS_FACING)
                == CameraCharacteristics.LENS_FACING_FRONT) {
            mIsCshotSupported = false;
            return;
        }

        DeviceDescription deviceDescription = CameraApiHelper.
                getDeviceSpec(mActivity.getApplicationContext()).getDeviceDescriptionMap()
                .get(String.valueOf(Integer.parseInt(mSettingController.getCameraId())));
        if (deviceDescription != null) {
            mIsCshotSupported = deviceDescription.isCshotSupport()
                    && ICameraMode.ModeType.PHOTO == getModeType();
        }

        initializeValue(mIsCshotSupported);
        if (deviceDescription != null) {
            mKeyCsCaptureRequest = deviceDescription.getKeyCshotRequestMode();
        }
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
    }

    protected boolean startContinuousShot() {
        //whether it support Cshot
        if (!mIsCshotSupported) {
            return false;
        }
        if (mState.getCShotState() == State.STATE_INIT) {
            if (mHandler == null) {
                return false;
            }
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    try {
                        LogHelper.i(TAG, "[startContinuousShot]");
                        synchronized (mNumberLock) {
                            mImageCallbackNumber = 0;
                        }
                        mState.updateState(State.STATE_CAPTURE_STARTED);
                        onContinuousShotStarted();
                        for (int i = 0; i < ROUND_NUMBER; i++) {
                            createCaptureRequest();
                        }
                        playSound();
                    } catch (CameraAccessException e) {
                        mState.updateState(State.STATE_ERROR);
                        e.printStackTrace();
                    } catch (IllegalStateException e) {
                        mState.updateState(State.STATE_ERROR);
                        e.printStackTrace();
                    }
                }
            });
            return true;
        }
        return false;
    }

    protected boolean stopContinuousShot() {
        if (mState.getCShotState() == State.STATE_ERROR) {
            onContinuousShotStopped();
            onContinuousShotDone(0);
            mState.updateState(State.STATE_INIT);
        } else if (mState.getCShotState() == State.STATE_CAPTURE_STARTED) {
            if (mHandler == null) {
                return false;
            }
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    mState.updateState(State.STATE_STOPPED);
                    LogHelper.i(TAG, "[stopContinuousShot]");
                    Camera2CaptureSessionProxy session =
                            mSettingDevice2Requester.getCurrentCaptureSession();
                    try {
                        if (session != null) {
                            session.abortCaptures();
                        }
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                    onContinuousShotStopped();
                    onContinuousShotDone(mImageCallbackNumber);
                    stopSound();
                    mState.updateState(State.STATE_INIT);
                }
            });
            return true;
        }
        stopSound();
        return false;
    }

    protected void requestChangeOverrideValues() {
        mSettingDevice2Requester.createAndChangeRepeatingRequest();
    }

    private void createCaptureRequest()
            throws CameraAccessException, IllegalStateException {
        LogHelper.d(TAG, "[createCaptureRequest]");
        List<CaptureRequest> requests = new ArrayList<>();
        CaptureRequest.Builder captureBuilder = mSettingDevice2Requester
                .createAndConfigRequest(Camera2Proxy.TEMPLATE_STILL_CAPTURE);
        if (mKeyCsCaptureRequest != null) {
            captureBuilder.set(mKeyCsCaptureRequest, mCaptureMode);
        }
        captureBuilder.set(CaptureRequest.JPEG_QUALITY, JPEG_QUALITY_VALUE);
        CaptureSurface sharedCaptureSurface = mSettingDevice2Requester
                .getModeSharedCaptureSurface();
        Surface captureSurface = sharedCaptureSurface.getSurface();
        Assert.assertNotNull(captureSurface);
        captureBuilder.addTarget(captureSurface);
        sharedCaptureSurface.setCaptureCallback(mImageCallback);
        Surface previewSurface = mSettingDevice2Requester.getModeSharedPreviewSurface();
        captureBuilder.addTarget(previewSurface);
        Surface thumbnailSurface = mSettingDevice2Requester.getModeSharedThumbnailSurface();
        captureBuilder.removeTarget(thumbnailSurface);
        prepareCaptureInfo(captureBuilder);
        for (int i = 0; i < MAX_BURST_NUM; i++) {
            requests.add(captureBuilder.build());
        }
        Camera2CaptureSessionProxy session = mSettingDevice2Requester
                .getCurrentCaptureSession();
        session.captureBurst(requests, mCaptureCallback, mHandler);
    }

    private void prepareCaptureInfo(CaptureRequest.Builder captureBuilder) {
        LogHelper.d(TAG, "[prepareCaptureInfo] current builder : " + captureBuilder);
        //don't care preview surface, because the preview is added in device controller.
        //set the jpeg orientation
        int mJpegRotation = mApp.getGSensorOrientation();
        //TODO how to get the camera id from characteristics.
        //Current CS just support back camera.
        int mCurrentCameraId = 0;
        int rotation = CameraUtil.getJpegRotationFromDeviceSpec(mCurrentCameraId,
                mJpegRotation, mActivity);
        captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, rotation);
        if (mCameraContext.getLocation() != null) {
            captureBuilder.set(CaptureRequest.JPEG_GPS_LOCATION, mCameraContext.getLocation());
        }
    }

    private CaptureSurface.ImageCallback mImageCallback = new CaptureSurface.ImageCallback() {

        @Override
        public void onPictureCallback(
                byte[] data, int format, String formatTag, int width, int height) {
            synchronized (mNumberLock) {
                if (data != null) {
                    mImageCallbackNumber ++;
                    LogHelper.d(TAG, "[mImageCallback] Number = " + mImageCallbackNumber);
                    saveJpeg(data);
                    if (mImageCallbackNumber >= MAX_CAPTURE_NUMBER) {
                        CaptureSurface sharedCaptureSurface = mSettingDevice2Requester
                                .getModeSharedCaptureSurface();
                        sharedCaptureSurface.discardFreeBuffers();
                    }
                }
            }
        }
    };

    private final CameraCaptureSession.CaptureCallback mCaptureCallback = new
            CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long
                timestamp, long frameNumber) {
            super.onCaptureStarted(session, request, timestamp, frameNumber);
            LogHelper.d(TAG, "[onCaptureStarted] mState = " + mState.getCShotState()
                + ", frame number: " + frameNumber);
        }

        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request,
                CaptureResult partialResult) {
            super.onCaptureProgressed(session, request, partialResult);
            LogHelper.d(TAG, "[onCaptureProgressed] mState = " + mState.getCShotState());
        }

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            LogHelper.d(TAG, "[onCaptureCompleted] frame number: " + result.getFrameNumber());
        }

        @Override
        public void onCaptureFailed(CameraCaptureSession session, CaptureRequest request,
                CaptureFailure failure) {
            super.onCaptureFailed(session, request, failure);
            LogHelper.i(TAG, "[onCaptureFailed] fail: " + failure.getReason()
                    + ", frame number: " + failure.getFrameNumber());
            stopContinuousShot();
        }

        @Override
        public void onCaptureSequenceCompleted(CameraCaptureSession session, int sequenceId, long
                frameNumber) {
            super.onCaptureSequenceCompleted(session, sequenceId, frameNumber);
            LogHelper.i(TAG, "[onCaptureSequenceCompleted] last frame number: " + frameNumber);
        }

        @Override
        public void onCaptureSequenceAborted(CameraCaptureSession session, int sequenceId) {
            super.onCaptureSequenceAborted(session, sequenceId);
            LogHelper.i(TAG, "[onCaptureSequenceAborted]");

        }

        @Override
        public void onCaptureBufferLost(CameraCaptureSession session, CaptureRequest request,
                Surface target, long frameNumber) {
            super.onCaptureBufferLost(session, request, target, frameNumber);
            LogHelper.e(TAG, "[onCaptureBufferLost] frameNumber: " + frameNumber);

        }
    };

}
