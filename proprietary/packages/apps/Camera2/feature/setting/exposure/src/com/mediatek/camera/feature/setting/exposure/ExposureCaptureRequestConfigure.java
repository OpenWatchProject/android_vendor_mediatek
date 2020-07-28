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
package com.mediatek.camera.feature.setting.exposure;

import android.annotation.TargetApi;
import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Range;
import android.util.Rational;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.feature.setting.exposure.IExposure.FlashFlow;
import com.mediatek.camera.portability.SystemProperties;

import java.util.ArrayList;
import java.util.List;

/**
 * This class used to configure exposure value to capture request.
 * Just used for api2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class ExposureCaptureRequestConfigure implements ICameraSetting.ICaptureRequestConfigure,
        IExposure.Listener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            ExposureCaptureRequestConfigure.class.getSimpleName());
    private CameraCharacteristics mCameraCharacteristics;
    private static final int[] CAM_FLASH_CUSTOMIZED_RESULT_NON_PANEL = new int[]{0};
    private static final int[] CAM_FLASH_CUSTOMIZED_RESULT_PRE_FLASH = new int[]{1};
    private static final int[] CAM_FLASH_CUSTOMIZED_RESULT_MAIN_FLASH = new int[]{2};
    private static final String FLASH_AUTO_VALUE = "auto";
    private static final String FLASH_OFF_VALUE = "off";
    private static final String FLASH_ON_VALUE = "on";
    private static final String EXTERNLA_FLASH_CUSTOMIZED =
            "vendor.mtk.camera.external.flash.customized";
    private static final String FLASH_KEY_CUSTOMIZED_RESULT =
            "com.mediatek.flashfeature.customizedResult";
    private static final String FLASH_KEY_CUSTOMIZATION_AVAILABLE =
            "com.mediatek.flashfeature.customization.available";
    private static final String SHUTTER_AUTO_VALUE = "Auto";
    private static final int TEMPLATE_STILL_CAPTURE = 2;
    private static final int TEMPLATE_ZERO_SHUTTER_LAG = 5;
    private static boolean sIsCustomizedFlash =
            SystemProperties.getInt(EXTERNLA_FLASH_CUSTOMIZED, 0) == 1;
    private static final int PRE_FLASH_BRIGHTNESS = 255;
    private static final int MAIN_FLASH_BRIGHTNESS = 255;

    private CaptureResult.Key<byte[]> mKeyFlashCustomizedResult;
    private Boolean mIsFlashCustomizedSupported = Boolean.FALSE;
    private IExposure.FlashFlow mFlow = FlashFlow.FLASH_FLOW_NO_FLASH;
    private int mCurrentEv = 0;
    protected int mMinExposureCompensation = 0;
    protected int mMaxExposureCompensation = 0;
    protected float mExposureCompensationStep = 1.0f;

    private boolean mAeLock;
    private Boolean mIsAeLockAvailable;
    private boolean mAePreTriggerAndCaptureEnabled = true;
    private boolean mAePreTriggerRequestProcessed = false;
    private boolean mExternelCaptureTriggered = false;
    private Exposure mExposure;
    private ISettingManager.SettingDevice2Requester mDevice2Requester;
    private int mAEMode = CameraMetadata.CONTROL_AE_MODE_OFF;
    private Integer mLastConvergedState = CaptureResult.CONTROL_AE_STATE_INACTIVE;
    private Integer mAeState = CaptureResult.CONTROL_AE_STATE_INACTIVE;
    private int mLastCustomizedValue = -1;
    private Boolean mIsFlashAvailable = Boolean.FALSE;
    private Context mContext;
    private boolean mNeedChangeFlashModeToTorch;
    private Handler mHandler;

    /**
     * The construction function.
     *
     * @param exposure the Exposure class object.
     * @param device2Requester Requester used to request to config capture request.
     * @param context The camera context.
     */
    public ExposureCaptureRequestConfigure(Exposure exposure, ISettingManager
            .SettingDevice2Requester device2Requester, Context context) {
        mHandler = new Handler(Looper.myLooper());
        mContext = context;
        mExposure = exposure;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mCameraCharacteristics = characteristics;
        mNeedChangeFlashModeToTorch = false;
        mIsFlashAvailable = characteristics.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
        LogHelper.d(TAG, "[setCameraCharacteristics] mIsFlashAvailable " + mIsFlashAvailable);
        initFlashVendorKey();
        initFlow(characteristics);
        updateCapabilities(characteristics);
        buildExposureCompensation();
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        String shutterValue = mExposure.getCurrentShutterValue();
        LogHelper.d(TAG, "[configCaptureRequest, shutterValue " + shutterValue);
        if (CameraUtil.isStillCaptureTemplate(captureBuilder)
                && shutterValue != null && !SHUTTER_AUTO_VALUE.equals(shutterValue)) {
            mAEMode = CameraMetadata.CONTROL_AE_MODE_OFF;
        } else {
            updateAeMode();
        }

        addBaselineCaptureKeysToRequest(captureBuilder);
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return mPreviewCallback;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.createAndChangeRepeatingRequest();
    }

    @Override
    public void updateEv(int value) {
        LogHelper.v(TAG, "[updateEv] + value = " + value);
        if (value >= mMinExposureCompensation && value <= mMaxExposureCompensation) {
            mCurrentEv = value;
            mExposure.setValue(String.valueOf(mCurrentEv));
        } else {
            LogHelper.w(TAG, "[updateEv] invalid exposure range: " + value);
        }
        LogHelper.v(TAG, "[updateEv] -");
    }

    @Override
    public boolean needConsiderAePretrigger() {
        return true;
    }

    @Override
    public boolean checkTodoCapturAfterAeConverted() {
        switch (mFlow) {
            case FLASH_FLOW_NO_FLASH:
                return false;
            case FLASH_FLOW_NORMAL:
                if (CameraUtil.hasFocuser(mCameraCharacteristics)) {
                    return false;
                }
                doNormalCapture();
                return true;
            case FLASH_FLOW_PANEL_STANDARD:
                doStandardCapture();
                return true;
            case FLASH_FLOW_PANEL_CUSTOMIZATION:
                mExternelCaptureTriggered = false;
                doCustomizedCapture();
                return true;
            default:
                return false;
        }
    }

    @Override
    public void setAeLock(boolean lock) {
        if (mIsAeLockAvailable == null || !mIsAeLockAvailable) {
            LogHelper.w(TAG, "[setAeLock] Ae lock not supported");
            return;
        }
        mAeLock = lock;
    }

    @Override
    public boolean getAeLock() {
        return mAeLock;
    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public void overrideExposureValue(String currentValue, List<String> supportValues) {
        int value = Integer.valueOf(currentValue);
        if (value >= mMinExposureCompensation && value <= mMaxExposureCompensation) {
            mCurrentEv = value;
        } else {
            LogHelper.w(TAG, "[overrideExposureValue] invalid exposure range: " + value);
        }
    }

    private void initFlashVendorKey() {
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mExposure.getCameraId()));
        if (deviceDescription != null) {
            mIsFlashCustomizedSupported = deviceDescription.isFlashCustomizedAvailable();
            mKeyFlashCustomizedResult = deviceDescription.getKeyFlashCustomizedResult();
            LogHelper.i(TAG, "[initFlashVendorKey] mIsFlashCustomizedSupported "
                    + mIsFlashCustomizedSupported + ",sIsCustomizedFlash " + sIsCustomizedFlash);
        }

    }

    private void initFlow(CameraCharacteristics cs) {
        if (mIsFlashAvailable) {
            mFlow = FlashFlow.FLASH_FLOW_NORMAL;
            LogHelper.d(TAG, "[initFlow] normal flow");
            return;
        }
        if (mExposure.isThirdPartyIntent()) {
            LogHelper.d(TAG, "[initFlow] isThirdPartyIntent return");
            return;
        }
        if (mIsFlashCustomizedSupported && sIsCustomizedFlash) {
            mFlow = FlashFlow.FLASH_FLOW_PANEL_CUSTOMIZATION;
            LogHelper.d(TAG, "[initFlow] customized flow");
            return;
        }

        if (isExternalFlashSupported(cs)) {
            mFlow = IExposure.FlashFlow.FLASH_FLOW_PANEL_STANDARD;
            LogHelper.d(TAG, "[initFlow] standard flow");
        }
    }


    private CameraCaptureSession.CaptureCallback mPreviewCallback
            = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                                       TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            mAeState = result.get(TotalCaptureResult.CONTROL_AE_STATE);
            if (mAeState != null && (mAeState == CaptureResult.CONTROL_AE_STATE_CONVERGED
                    || mAeState == CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED)) {
                mLastConvergedState = mAeState;
            }
            //Do set panel off when capture with external flash done
            if (CameraUtil.isStillCaptureTemplate(result) && mExposure.isPanelOn()) {
                mExposure.setPanel(false, -1);
                setOriginalAeMode();
                sendSettingChangeRequest();
                return;
            }
            if (request == null || result == null) {
                LogHelper.w(TAG, "[onCaptureCompleted] request " + request
                        + ",result " + result);
                return;
            }
            dispatchResult(request, result);
        }
    };

    private void doNormalCapture() {
        if (!needAePreTriggerAndCapture()) {
            mExposure.capture();
            return;
        }
        if (mAePreTriggerAndCaptureEnabled) {
            sendAePreTriggerCaptureRequest();
        } else {
            LogHelper.w(TAG, "[doNormalCapture] sendAePreTriggerCaptureRequest is ignore because" +
                    "the last ae PreTrigger is not complete");
        }
    }
    /**
     * When the front camera supported CameraMetadata#CONTROL_AE_MODE_ON_EXTERNAL_FLASH,better
     * font-facing flash support via an app drawing a bring white screen by reducing tatal capture
     * time.
     */
    private void doStandardCapture() {
        String flashValue = mExposure.getCurrentFlashValue();
        LogHelper.d(TAG, "[doStandardCapture] with flash = " + flashValue);
        switch (flashValue) {
            case FLASH_ON_VALUE:
                captureStandardPanel();
                break;
            case FLASH_AUTO_VALUE:
                captureStandardWithFlashAuto();
                break;
            case FLASH_OFF_VALUE:
                mExposure.capture();
                break;
            default:
                LogHelper.w(TAG, "[doStandardCapture] error flash value" + flashValue);
        }
    }

    private void doCustomizedCapture() {
        String flashValue = mExposure.getCurrentFlashValue();
        LogHelper.d(TAG, "[doCustomizedCapture] with flash = " + flashValue);
        switch (flashValue) {
            case FLASH_ON_VALUE:
                sendAePreTriggerCaptureRequest();
                break;
            case FLASH_AUTO_VALUE:
                captureCustomizedWithFlashAuto();
                break;
            case FLASH_OFF_VALUE:
                mExposure.capture();
                break;
            default:
                LogHelper.w(TAG, "[doCustomizedCapture] error flash value" + flashValue);
        }
    }

    /**
     * Request preview capture stream with auto focus trigger cycle.
     */
    private void sendAePreTriggerCaptureRequest() {
        // Step 1: Request single frame CONTROL_AF_TRIGGER_START.
        CaptureRequest.Builder builder = mDevice2Requester
                .createAndConfigRequest(mDevice2Requester.getRepeatingTemplateType());
        if (builder == null) {
            LogHelper.w(TAG, "[sendAePreTriggerCaptureRequest] builder is null");
            return;
        }
        builder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
        // Step 2: Call repeatingPreview to update mControlAFMode.
        Camera2CaptureSessionProxy sessionProxy = mDevice2Requester.getCurrentCaptureSession();
        LogHelper.d(TAG, "[sendAePreTriggerCaptureRequest] sessionProxy " + sessionProxy);
        try {
            if (sessionProxy != null) {
                LogHelper.d(TAG, "[sendAePreTriggerCaptureRequest] " +
                        "CONTROL_AE_PRECAPTURE_TRIGGER_START");
                mAePreTriggerAndCaptureEnabled = false;
                sessionProxy.capture(builder.build(), mPreviewCallback, null);
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void dispatchResult(CaptureRequest request, TotalCaptureResult result) {
        switch (mFlow) {
            case FLASH_FLOW_NORMAL:
                if (!mAePreTriggerAndCaptureEnabled) {
                    checkAeStateTodoNormalCapture(request, result);
                }
                break;
            case FLASH_FLOW_PANEL_STANDARD:
                checkAeStateTodoStandardCapture(request, result);
                break;
            case FLASH_FLOW_PANEL_CUSTOMIZATION:
                checkAeStateTodoCustomizedCapture(request, result);
                break;
            default:
                break;
        }
    }

    private void checkAeStateTodoNormalCapture(CaptureRequest request, TotalCaptureResult result) {
        Integer aePrecaptureTrigger = request.get(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER);
        if (mAeState == null || aePrecaptureTrigger == null) {
            LogHelper.w(TAG, "[checkAeStateTodoNormalCapture] mAeState = " +
                    mAeState + " ,aePrecaptureTrigger " + aePrecaptureTrigger);
            return;
        }
        if (!mAePreTriggerRequestProcessed) {
            mAePreTriggerRequestProcessed = (aePrecaptureTrigger
                    == CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
        }
        if (mAePreTriggerRequestProcessed &&
                (mAeState == CaptureResult.CONTROL_AE_STATE_CONVERGED
                        || mAeState == CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED)) {
            LogHelper.d(TAG, "[checkAeStateTodoNormalCapture] go to capture with mAeState : " +
                    mAeState);
            mExposure.capture();
            mAePreTriggerAndCaptureEnabled = true;
            mAePreTriggerRequestProcessed = false;
        }
    }

    private void checkAeStateTodoStandardCapture(CaptureRequest request,
                                                 TotalCaptureResult result) {
        Integer aeMode = request.get(CaptureRequest.CONTROL_AE_MODE);
        if (aeMode == null || mAeState == null) {
            LogHelper.w(TAG, "[checkAeStateTodoStandardCapture] aeMode = "
                    + aeMode + " ,mAeState " + mAeState);
            return;
        }
        if (aeMode != CameraMetadata.CONTROL_AE_MODE_ON_EXTERNAL_FLASH) {
            mExternelCaptureTriggered = false;
        }
        if (mExternelCaptureTriggered) {
            return;
        }
        if ((aeMode == CameraMetadata.CONTROL_AE_MODE_ON_EXTERNAL_FLASH) && (
                mAeState == CaptureResult.CONTROL_AE_STATE_CONVERGED
                        || mAeState == CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED)) {
            LogHelper.d(TAG, "[checkAeStateTodoStandardCapture] go to capture with mAeState : "
                    + mAeState);
            mExposure.capture();
            mExternelCaptureTriggered = true;
        }
    }

    private void checkAeStateTodoCustomizedCapture(CaptureRequest request,
                                                   TotalCaptureResult result) {
        List<CaptureResult.Key<?>> keyList = result.getKeys();
        for (CaptureResult.Key<?> key : keyList) {
            if (key.getName().equals(FLASH_KEY_CUSTOMIZED_RESULT)) {
                byte[] value = result.get(mKeyFlashCustomizedResult);
                if (value == null) {
                    return;
                }
                LogHelper.d(TAG, "[checkAeStateTodoCustomizedCapture], value[0]: " + value[0]);
                if (value[0] == mLastCustomizedValue) {
                    // do not update result every times
                    return;
                }
                if (value[0] == CAM_FLASH_CUSTOMIZED_RESULT_NON_PANEL[0]) {
                    if (!mExternelCaptureTriggered && (mAeState != null &&
                            mAeState == CaptureResult.CONTROL_AE_STATE_CONVERGED)) {
                        LogHelper.d(TAG, "[checkAeStateTodoStandardCapture] go to capture with " +
                                "mAeState : " + mAeState);
                        mExposure.capture();
                        mExternelCaptureTriggered = true;
                    }
                } else if (value[0] == CAM_FLASH_CUSTOMIZED_RESULT_PRE_FLASH[0]) {
                    mExposure.setPanel(true, PRE_FLASH_BRIGHTNESS);
                } else if (value[0] == CAM_FLASH_CUSTOMIZED_RESULT_MAIN_FLASH[0]) {
                    if (!mExternelCaptureTriggered) {
                        mExposure.setPanel(true, MAIN_FLASH_BRIGHTNESS);
                        mExposure.capture();
                        mExternelCaptureTriggered = true;
                    }
                }
                mLastCustomizedValue = value[0];
                break;
            }
        }
    }

    /**
     * Adds current regions to CaptureRequest and base AF mode + AF_TRIGGER_IDLE.
     *
     * @param builder Build for the CaptureRequest
     */
    private void addBaselineCaptureKeysToRequest(CaptureRequest.Builder builder) {
        builder.set(CaptureRequest.CONTROL_AE_MODE, mAEMode);
        int exposureCompensationIndex = -1;
        if (mIsAeLockAvailable != null && mIsAeLockAvailable) {
            builder.set(CaptureRequest.CONTROL_AE_LOCK, mAeLock);
        }
        if (mExposureCompensationStep != 0) {
            exposureCompensationIndex = (int) (mCurrentEv / mExposureCompensationStep);
            builder.set(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION, exposureCompensationIndex);
        }
        LogHelper.d(TAG, "[addBaselineCaptureKeysToRequest]" + " mAEMode = " + mAEMode + ",mAeLock "
                + mAeLock + ",exposureCompensationIndex " + exposureCompensationIndex);
        builder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_IDLE);
    }

    private void updateCapabilities(CameraCharacteristics characteristics) {
        if (characteristics == null) {
            LogHelper.w(TAG, "[updateCapabilities] characteristics is null");
            return;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            mIsAeLockAvailable =
                    characteristics.get(CameraCharacteristics.CONTROL_AE_LOCK_AVAILABLE);
        }
        mMaxExposureCompensation = getMaxExposureCompensation(characteristics);
        mMinExposureCompensation = getMinExposureCompensation(characteristics);
        mExposureCompensationStep = getExposureCompensationStep(characteristics);
    }

    private void buildExposureCompensation() {
        if (mMaxExposureCompensation == 0 && mMinExposureCompensation == 0) {
            return;
        }
        LogHelper.d(TAG, "[buildExposureCompensation]+ exposure compensation range (" +
                mMinExposureCompensation + ", "
                + mMaxExposureCompensation + "),with step " + mExposureCompensationStep);
        int maxValue = (int) Math.floor(mMaxExposureCompensation * mExposureCompensationStep);
        int minValue = (int) Math.ceil(mMinExposureCompensation * mExposureCompensationStep);
        ArrayList<String> values = new ArrayList<String>();
        for (int i = minValue; i <= maxValue; ++i) {
            values.add(String.valueOf(i));
        }
        initPlatformSupportedValues(values);
        int finalSize = values.size();
        int[] entryValues = new int[finalSize];
        for (int i = 0; i < finalSize; i++) {
            entryValues[i] = Integer.parseInt(values.get(finalSize - i - 1));
        }
        mExposure.initExposureCompensation(entryValues);
        LogHelper.d(TAG, "[buildExposureCompensation] - values  = " + values);
    }

    private void initPlatformSupportedValues(ArrayList<String> values) {
        int defaultEv = 0;
        mCurrentEv = defaultEv;
        mExposure.setValue(String.valueOf(defaultEv));
        mExposure.setSupportedPlatformValues(values);
        mExposure.setSupportedEntryValues(values);
        mExposure.setEntryValues(values);
    }

    private boolean isExposureCompensationSupported(CameraCharacteristics characteristics) {
        Range<Integer> compensationRange =
                characteristics.get(CameraCharacteristics.CONTROL_AE_COMPENSATION_RANGE);
        return compensationRange.getLower() != 0 || compensationRange.getUpper() != 0;
    }

    private int getMinExposureCompensation(CameraCharacteristics characteristics) {
        if (!isExposureCompensationSupported(characteristics)) {
            return -1;
        }
        Range<Integer> compensationRange =
                characteristics.get(CameraCharacteristics.CONTROL_AE_COMPENSATION_RANGE);
        return compensationRange.getLower();
    }

    private int getMaxExposureCompensation(CameraCharacteristics characteristics) {
        if (!isExposureCompensationSupported(characteristics)) {
            return -1;
        }
        Range<Integer> compensationRange =
                characteristics.get(CameraCharacteristics.CONTROL_AE_COMPENSATION_RANGE);
        return compensationRange.getUpper();
    }

    private float getExposureCompensationStep(CameraCharacteristics characteristics) {
        if (!isExposureCompensationSupported(characteristics)) {
            return -1.0f;
        }
        Rational compensationStep = characteristics.get(
                CameraCharacteristics.CONTROL_AE_COMPENSATION_STEP);
        return (float) compensationStep.getNumerator() / compensationStep.getDenominator();
    }

    private void updateAeMode() {
        if (mAEMode == CameraMetadata.CONTROL_AE_MODE_ON_EXTERNAL_FLASH) {
            return;
        }
        setOriginalAeMode();
    }

    private void setOriginalAeMode() {
        String flashValue = mExposure.getCurrentFlashValue();
        if (FLASH_ON_VALUE.equalsIgnoreCase(flashValue)) {
            if (mNeedChangeFlashModeToTorch ||
                    mExposure.getCurrentModeType() == ICameraMode.ModeType.VIDEO) {
                mAEMode = CameraMetadata.CONTROL_AE_MODE_ON;
                return;
            }
            mAEMode = CameraMetadata.CONTROL_AE_MODE_ON_ALWAYS_FLASH;
            // AE mode -> ON_ALWAYS_FLASH, flash mode value doesn't affect
            return;
        }
        if (FLASH_AUTO_VALUE.equalsIgnoreCase(flashValue)) {
            if (mNeedChangeFlashModeToTorch) {
                mAEMode = CameraMetadata.CONTROL_AE_MODE_ON;
                return;
            }
            // AE mode -> ON_AUTO_FLASH, flash mode value doesn't affect
            mAEMode = CameraMetadata.CONTROL_AE_MODE_ON_AUTO_FLASH;
            return;
        }
        // flash -> off, must need AE mode -> on
        mAEMode = CameraMetadata.CONTROL_AE_MODE_ON;
    }

    /**
     * Mark to change flash mode to torch by state later.
     * When AE state is {@link CaptureResult#CONTROL_AE_STATE_FLASH_REQUIRED},need change flash mode
     * to torch.
     *
     * @param mark Whether need to mark or not.
     */
    protected void changeFlashToTorchByAeState(boolean needChange) {
        LogHelper.d(TAG, "[changeFlashToTorchByAeState] + needChange = " + needChange +
                ",mAeState = " + mAeState + ",mLastConvergedState = " + mLastConvergedState);
        if (!needChange) {
            mNeedChangeFlashModeToTorch = false;
            LogHelper.d(TAG, "[changeFlashToTorchByAeState] - mNeedChangeFlashModeToTorch = false");
            return;
        }
        String flashValue = mExposure.getCurrentFlashValue();
        if (FLASH_ON_VALUE.equalsIgnoreCase(flashValue)) {
            mNeedChangeFlashModeToTorch = true;
        }
        if (FLASH_AUTO_VALUE.equalsIgnoreCase(flashValue)) {
            if (mAeState != null && mAeState == CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED) {
                mNeedChangeFlashModeToTorch = true;
            } else if (mAeState != null && mAeState == CaptureResult.CONTROL_AE_STATE_SEARCHING &&
                    mLastConvergedState == CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED) {
                mNeedChangeFlashModeToTorch = true;
            } else {
                mNeedChangeFlashModeToTorch = false;
            }
        }
        LogHelper.d(TAG, "[changeFlashToTorchByAeState] - mNeedChangeFlashModeToTorch = " +
                mNeedChangeFlashModeToTorch);
    }

    private boolean isExternalFlashSupported(CameraCharacteristics characteristics) {
        boolean isSupported = false;
        int[] availableAeModes = characteristics.get(
                CameraCharacteristics.CONTROL_AE_AVAILABLE_MODES);
        if (availableAeModes == null) {
            return false;
        }
        loop:
        for (int mode : availableAeModes) {
            switch (mode) {
                case CameraMetadata.CONTROL_AE_MODE_ON_EXTERNAL_FLASH:
                    isSupported = true;
                    break loop;
                default:
                    break;
            }
        }
        LogHelper.d(TAG, "[isExternalFlashSupported] isSupported = " + isSupported);
        return isSupported;
    }

    private void captureStandardPanel() {
        LogHelper.d(TAG, "[captureStandardPanel]");
        //do nothing when it still do last panel capture
        if (mExternelCaptureTriggered) {
            return;
        }
        //step:panel on
        mExposure.setPanel(true, PRE_FLASH_BRIGHTNESS);
        //step2:set ae mode to external flash.
        mAEMode = CameraMetadata.CONTROL_AE_MODE_ON_EXTERNAL_FLASH;
        //step3:cpature after ae convert
        mHandler.post(new Runnable() {
            public void run() {
                sendSettingChangeRequest();
            }
        });
    }

    private void captureStandardWithFlashAuto() {
        LogHelper.d(TAG, "[capturePanelWithFlashAuto] with ae state = " + mAeState);
        if (mAeState == null) {
            return;
        }
        switch (mAeState) {
            case CaptureResult.CONTROL_AE_STATE_CONVERGED:
                mExposure.capture();
                break;
            case CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED:
                captureStandardPanel();
                break;
            case CaptureResult.CONTROL_AE_STATE_SEARCHING:
                sendAePreTriggerCaptureRequest();
                break;
            default:
                mExposure.capture();
                break;
        }
    }

    private void captureCustomizedWithFlashAuto() {
        LogHelper.d(TAG, "[captureCustomizedWithFlashAuto] with ae state = " + mAeState);
        if (mAeState == null) {
            return;
        }
        switch (mAeState) {
            case CaptureResult.CONTROL_AE_STATE_CONVERGED:
                mExposure.capture();
                break;
            case CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED:
            case CaptureResult.CONTROL_AE_STATE_SEARCHING:
                sendAePreTriggerCaptureRequest();
                break;
            default:
                mExposure.capture();
                break;
        }
    }

    private boolean needAePreTriggerAndCapture() {
        boolean needAePretrigger = false;
        String flashValue = mExposure.getCurrentFlashValue();
        if (FLASH_ON_VALUE.equals(flashValue) || FLASH_AUTO_VALUE.equals(flashValue)) {
            needAePretrigger = true;
        } else {
            needAePretrigger = false;
        }
        return needAePretrigger;
    }

}
