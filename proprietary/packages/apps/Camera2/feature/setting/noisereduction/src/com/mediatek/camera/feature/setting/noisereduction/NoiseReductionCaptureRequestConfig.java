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

package com.mediatek.camera.feature.setting.noisereduction;

import android.annotation.TargetApi;

import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraCharacteristics.Key;
import android.hardware.camera2.CaptureRequest;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;

/**
 * This is for NoiseReduction capture flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class NoiseReductionCaptureRequestConfig implements
                    ICameraSetting.ICaptureRequestConfigure {

    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            NoiseReductionCaptureRequestConfig.class.getSimpleName());

    private CaptureRequest.Key<int[]> mNoiseReductionKey;
    private Key<int[]> mNoiseReductionAvailableModes;
    private static final String NR3D_MODE_REQUEST = "com.mediatek.nrfeature.3dnrmode";
    private static final String NR3D_MODES_STRING = "com.mediatek.nrfeature.available3dnrmodes";

    private static final String NOISE_REDUCTION_OFF = "off";
    private static final String NOISE_REDUCTION_ON = "on";
    private static final int MTK_3DNR_MODE_OFF = 0;
    private static final int MTK_3DNR_MODE_ON = 1;
    private boolean mIsSupported = false;

    private CameraCharacteristics mCameraCharacteristics;
    private SettingDevice2Requester mDevice2Requester;
    private NoiseReduction mNoiseReduction;

    /**
     * NoiseReduction capture request configure constructor.
     *
     * @param noise The instance of {@link NoiseReduction}.
     *
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     */
    public NoiseReductionCaptureRequestConfig(
                       NoiseReduction noise, SettingDevice2Requester device2Requester) {
        mNoiseReduction = noise;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mCameraCharacteristics = characteristics;
        initModesKeys();
        initModeKey();
        if (mNoiseReductionKey == null || mNoiseReductionAvailableModes == null) {
            LogHelper.w(TAG, "[setCameraCharacteristics] mNoiseReductionKey or " +
                    "mNoiseReductionAvailableModes is null");
            return;
        }
        updateSupportedValues();
        if (mIsSupported) {
            mNoiseReduction.updateValue(NOISE_REDUCTION_ON);
        }
    }

    private void initModesKeys() {
        List<Key<?>> keys = mCameraCharacteristics.getKeys();
        for (Key<?> key : keys) {
            if (NR3D_MODES_STRING.equals(key.getName())) {
                mNoiseReductionAvailableModes = (Key<int[]>) key;
            }
        }
    }

    private void initModeKey() {
        List<CaptureRequest.Key<?>> keys = mCameraCharacteristics.getAvailableCaptureRequestKeys();
        for (CaptureRequest.Key<?> key : keys) {
            if (NR3D_MODE_REQUEST.equals(key.getName())) {
                mNoiseReductionKey = (CaptureRequest.Key<int[]>) key;
                break;
            }
        }
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        if (!mIsSupported) {
            return;
        }
        String value = mNoiseReduction.getValue();
        LogHelper.d(TAG, "[configCaptureRequest] current nose reduction value = " + value);
        if ("on".equals(value)) {
            int[] currentMode = {MTK_3DNR_MODE_ON};
            captureBuilder.set(mNoiseReductionKey, currentMode);
        } else {
            int[] currentMode = {MTK_3DNR_MODE_OFF};
            captureBuilder.set(mNoiseReductionKey, currentMode);
        }
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
        mDevice2Requester.createAndChangeRepeatingRequest();
    }

    private void updateSupportedValues() {
        List<String> supported = getSupported3DNRValues();
        mNoiseReduction.setSupportedPlatformValues(supported);
        mNoiseReduction.setEntryValues(supported);
        mNoiseReduction.setSupportedEntryValues(supported);
        if (supported != null && supported.size() > 1) {
            mIsSupported = true;
        }
        mNoiseReduction.updateIsSupported(mIsSupported);
    }

    private List<String> getSupported3DNRValues() {
        int[] availableMode = getValueFromKey(mNoiseReductionAvailableModes);
        List<String> supportedValues = null;
        if (availableMode != null) {
            supportedValues = new ArrayList<String>();
            for (int i = 0; i < availableMode.length; i++) {
                switch (availableMode[i]) {
                    case MTK_3DNR_MODE_OFF:
                        supportedValues.add(NOISE_REDUCTION_OFF);
                        break;
                    case MTK_3DNR_MODE_ON:
                        supportedValues.add(NOISE_REDUCTION_ON);
                        break;
                    default:
                        break;
                }
            }
        }
        return supportedValues;
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
