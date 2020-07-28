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

package com.mediatek.camera.feature.setting.eis;

import android.app.Activity;
import android.content.Intent;
import android.hardware.Camera;
import android.provider.MediaStore;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;

import java.util.ArrayList;
import java.util.List;

/**
 * This is for EIS capture flow in camera API1.
 */
public class EISParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
                                       EISParametersConfig.class.getSimpleName());
    private static final String EIS_OFF = "off";
    private static final String EIS_ON = "on";
    private static final int CAMERA_MODE_MTK_VDO = 2;
    private SettingDeviceRequester mDeviceRequester;
    private boolean mIsSupported = false;
    private Activity mActivity;
    private EIS mEIS;
    private boolean mOldValue = false;

    /**
     * EIS parameters configure constructor.
     * @param eis The instance of {@link EIS}.
     * @param deviceRequester The implementer of {@link SettingDeviceRequester}.
     * @param activity the activity.
     */
    public EISParametersConfig(EIS eis, SettingDeviceRequester deviceRequester, Activity activity) {
        mEIS = eis;
        mActivity = activity;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        updateSupportedValues(originalParameters);
        if (mIsSupported) {
            mEIS.updateValue(EIS_OFF);
            mOldValue = false;
        }
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (mIsSupported) {
            LogHelper.d(TAG, "[configParameters] eis = " + mEIS.getValue());
            boolean toggle = EIS_ON.equals(mEIS.getValue()) ? true : false;
            parameters.setVideoStabilization(toggle);
            if (mOldValue != toggle) {
                mOldValue = toggle;
                return true;
            }
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mEIS.getKey());
    }

    private void updateSupportedValues(Camera.Parameters originalParameters) {
        if (originalParameters.isVideoStabilizationSupported()) {
            List<String> supported = new ArrayList<>();
            supported.add(EIS_ON);
            supported.add(EIS_OFF);
            mEIS.setSupportedPlatformValues(supported);
            mEIS.setEntryValues(supported);
            mEIS.setSupportedEntryValues(supported);
            mIsSupported = true;
        }
        LogHelper.d(TAG, "[updateSupportedValues] mIsSupported : " + mIsSupported);
    }
}
