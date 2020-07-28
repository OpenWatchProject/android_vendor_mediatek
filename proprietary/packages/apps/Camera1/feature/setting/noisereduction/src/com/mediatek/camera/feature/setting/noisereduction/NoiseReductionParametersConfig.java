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

import android.hardware.Camera;
import android.text.TextUtils;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;

import java.util.ArrayList;
import java.util.List;

/**
 * This is for Noise Reduction capture flow in camera API1.
 */
public class NoiseReductionParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            NoiseReductionParametersConfig.class.getSimpleName());

    private static final String KEY_3DNR_MODE_KEY = "3dnr-mode-values";
    private static final String KEY_3DNR_MODE = "3dnr-mode";

    private NoiseReduction mNoiseReduction;
    private boolean mIsSupported = false;
    private SettingDeviceRequester mDeviceRequester;
    /**
     * Noise Reduction parameters configure constructor.
     * @param noise The instance of {@link NoiseReduction}.
     * @param deviceRequester The implementer of {@link SettingDeviceRequester}.
     */
    public NoiseReductionParametersConfig(NoiseReduction noise,
                                          SettingDeviceRequester deviceRequester) {
        mNoiseReduction = noise;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        updateSupportedValues(originalParameters);
        if (mIsSupported) {
            mNoiseReduction.updateValue(originalParameters.get(KEY_3DNR_MODE));
        }
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (mIsSupported) {
            String value = mNoiseReduction.getValue();
            LogHelper.d(TAG, "[configParameters] value = " + value);
            if (value != null) {
                parameters.set(KEY_3DNR_MODE, value);
            }
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mNoiseReduction.getKey());
    }

    private void updateSupportedValues(Camera.Parameters originalParameters) {
        List<String> supported = getSupported3DNRValues(originalParameters);
        mNoiseReduction.setSupportedPlatformValues(supported);
        mNoiseReduction.setEntryValues(supported);
        mNoiseReduction.setSupportedEntryValues(supported);
        if (supported != null && supported.size() > 1) {
            mIsSupported = true;
        }
        mNoiseReduction.updateIsSupported(mIsSupported);
    }

    // Copied from android.hardware.Camera
    // Splits a comma delimited string to an ArrayList of String.
    // Return null if the passing string is null or the size is 0.
    private static ArrayList<String> split(String str) {
        ArrayList<String> substrings = null;
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            substrings = new ArrayList<String>();
            for (String s : splitter) {
                substrings.add(s);
            }
        }
        return substrings;
    }

    private static List<String> getSupported3DNRValues(Camera.Parameters parameters) {
        List<String> supportedList = null;
        if (parameters != null) {
            String str = parameters.get(KEY_3DNR_MODE_KEY);
            supportedList = split(str);
        }
        return supportedList;
    }

}
