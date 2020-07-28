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

package com.mediatek.camera.feature.setting.zsd;

import android.hardware.Camera;
import android.text.TextUtils;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.List;

/**
 * Zsd parameters configure in camera API1.
 */
public class ZSDParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            ZSDParametersConfig.class.getSimpleName());

    private static final String KEY_ZSD_MODE = "zsd-mode";
    private static final String KEY_ZSD_SUPPORTED_MODE = "zsd-mode-values";
    private static final String VALUE_ON = "on";
    private static final int CAMERA_MODE_MTK_PRV = 1;
    private ISettingManager.SettingDeviceRequester mDeviceRequester;
    private ZSD mZsd;
    private String mValue;

    /**
     * Zsd parameters configure constructor.
     *
     * @param zsd The instance of {@link ZSD}.
     * @param deviceRequester The instance of {@link ISettingManager.SettingDeviceRequester}.
     */
    public ZSDParametersConfig(ZSD zsd,
                               ISettingManager.SettingDeviceRequester deviceRequester) {
        mZsd = zsd;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        List<String> supportedValues = split(originalParameters.get(KEY_ZSD_SUPPORTED_MODE));
        String defaultValue = originalParameters.get(KEY_ZSD_MODE);
        LogHelper.d(TAG, "[setOriginalParameters] defaultValue = " + defaultValue);
        mZsd.initializeValue(supportedValues, defaultValue);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (mZsd.getValue() == null) {
            return false;
        }
        boolean changed = !(mZsd.getValue().equals(mValue));
        parameters.set(KEY_ZSD_MODE, mZsd.getValue());
        mValue = mZsd.getValue();
        return changed;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mZsd.getKey());
    }

    private ArrayList<String> split(String str) {
        if (str == null) {
            return null;
        }

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<String> subStrings = new ArrayList<>();
        for (String s : splitter) {
            subStrings.add(s);
        }
        return subStrings;
    }
}
