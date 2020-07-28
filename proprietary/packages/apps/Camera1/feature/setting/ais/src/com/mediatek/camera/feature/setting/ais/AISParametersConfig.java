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

package com.mediatek.camera.feature.setting.ais;

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
 * This is for AIS capture flow in camera API1.
 */
public class AISParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
                                       AISParametersConfig.class.getSimpleName());
    private static final String KEY_MFB_AIS_VALUES = "mfb-values";
    private static final String KEY_MFB_AIS = "mfb";
    private static final String VALUE_AIS = "ais";
    private static final String VALUE_OFF = "off";
    private static final String VALUE_ON = "on";
    private static final String VALUE_AUTO = "auto";
    private boolean mIsSupported = false;

    private List<String> mSupportedValues = new ArrayList<>();
    private SettingDeviceRequester mDeviceRequester;
    private AIS mAIS;

    /**
     * AIS parameters configure constructor.
     * @param ais The instance of {@link AIS}.
     * @param deviceRequester The implementer of {@link SettingDeviceRequester}.
     */
    public AISParametersConfig(AIS ais, SettingDeviceRequester deviceRequester) {
        mAIS = ais;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        String str = originalParameters.get(KEY_MFB_AIS_VALUES);
        mSupportedValues = split(str);
        List<String> supportedMode = new ArrayList<>();
        if (mSupportedValues.contains(VALUE_AIS)) {
            supportedMode.add(VALUE_OFF);
            supportedMode.add(VALUE_ON);
            mIsSupported = true;
        }
        mAIS.initializeValue(supportedMode, originalParameters.get(KEY_MFB_AIS));
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        LogHelper.d(TAG, "[configParameters] ais = " + mAIS.getValue()
                + ", ais override value:" + mAIS.getOverrideValue());

        if (mIsSupported) {
            if (VALUE_ON.equals(mAIS.getValue())) {
                parameters.set(KEY_MFB_AIS, VALUE_AIS);
            } else {
                if (mSupportedValues.contains(VALUE_AUTO)
                        && !VALUE_OFF.equals(mAIS.getOverrideValue())) {
                    parameters.set(KEY_MFB_AIS, VALUE_AUTO);
                } else {
                    parameters.set(KEY_MFB_AIS, VALUE_OFF);
                }
            }
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mAIS.getKey());
    }

    private static List<String> split(String str) {
        List<String> subStrings = new ArrayList<>();
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            for (String s : splitter) {
                subStrings.add(s);
            }
        }
        return subStrings;
    }
}
