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
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.MeteringRectangle;
import android.os.Build;
import android.util.Range;
import android.util.Rational;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
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
    private int mCurrentEv = 0;
    protected int mMinExposureCompensation = 0;
    protected int mMaxExposureCompensation = 0;
    protected float mExposureCompensationStep = 1.0f;

    private boolean mAeLock;
    private Boolean mIsAeLockAvailable;
    private boolean mAePreTriggerAndCaptureEnabled = true;
    private boolean mAePreTriggerRequestProcessed = false;
    private Exposure mExposure;
    private ISettingManager.SettingDevice2Requester mDevice2Requester;

    /**
     * The construction function.
     *
     * @param exposure         the Exposure class object.
     * @param device2Requester Requester used to request to config capture request.
     */
    public ExposureCaptureRequestConfigure(Exposure exposure, ISettingManager
            .SettingDevice2Requester
            device2Requester) {
        mExposure = exposure;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        updateCapabilities(characteristics);
        buildExposureCompensation();
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
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
    public void doAePreTrigger() {
        if (mAePreTriggerAndCaptureEnabled) {
            sendAePreTriggerCaptureRequest();
        } else {
            LogHelper.w(TAG, "[doAePreTrigger] is ignore because the last ae PreTrigger is not " +
                    "complete");
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

    private CameraCaptureSession.CaptureCallback mPreviewCallback
            = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                                       TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            if (!mAePreTriggerAndCaptureEnabled) {
                checkAeState(request, result);
            }
        }
    };

    @Override
    public boolean getAeLock() {
        return mAeLock;
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

    private void checkAeState(CaptureRequest request, TotalCaptureResult result) {
        if (request == null || result == null) {
            LogHelper.w(TAG, "[checkAeState] request " + request + ",result " + result);
            return;
        }
        Integer aePrecaptureTrigger = request.get(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER);
        Integer aeState = result.get(TotalCaptureResult.CONTROL_AE_STATE);
        if (aeState == null || aePrecaptureTrigger == null) {
            LogHelper.w(TAG, "[checkAeState] aeState = " + aeState + " ,aePrecaptureTrigger " +
                    aePrecaptureTrigger);
            return;
        }
        if (!mAePreTriggerRequestProcessed) {
            mAePreTriggerRequestProcessed = (aePrecaptureTrigger
                    == CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
        }
        if (mAePreTriggerRequestProcessed &&
                (aeState == CaptureResult.CONTROL_AE_STATE_CONVERGED
                        || aeState == CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED)) {
            LogHelper.d(TAG, "[checkAeState] go to capture with aeState : " + aeState);
            mExposure.capture();
            mAePreTriggerAndCaptureEnabled = true;
            mAePreTriggerRequestProcessed = false;
        }
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

    /**
     * Adds current regions to CaptureRequest and base AF mode + AF_TRIGGER_IDLE.
     *
     * @param builder Build for the CaptureRequest
     */
    private void addBaselineCaptureKeysToRequest(CaptureRequest.Builder builder) {
        if (mIsAeLockAvailable != null && mIsAeLockAvailable) {
            LogHelper.d(TAG, "[addBaselineCaptureKeysToRequest] mAeLock " + mAeLock);
            builder.set(CaptureRequest.CONTROL_AE_LOCK, mAeLock);
        }
        if (mExposureCompensationStep != 0) {
            int exposureCompensationIndex = (int) (mCurrentEv / mExposureCompensationStep);
            LogHelper.d(TAG, "[addBaselineCaptureKeysToRequest] exposureCompensationIndex " +
                    exposureCompensationIndex);
            builder.set(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION, exposureCompensationIndex);
        }
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
}
