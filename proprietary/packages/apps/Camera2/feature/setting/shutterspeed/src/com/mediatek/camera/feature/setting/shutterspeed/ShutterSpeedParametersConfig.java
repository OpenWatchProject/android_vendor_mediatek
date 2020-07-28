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
 *   MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.camera.feature.setting.shutterspeed;

import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

/**
 * Configure shutter speed parameters on API1.
 */

class ShutterSpeedParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(ShutterSpeedParametersConfig.class.getSimpleName());


    private ShutterSpeed mShutterSpeed;
    private ISettingManager.SettingDeviceRequester mDeviceRequester;
    private static final String KEY_EXPOSURE_TIME = "exposure-time";
    private static final int S_TO_MS = 1000;
    private boolean mIsSupported = false;

    /**
     * Shutter Speed parameters configure constructor.
     *
     * @param shutterSpeed    The instance of {@link ShutterSpeed}.
     * @param deviceRequester The instance of {@link ISettingManager.SettingDeviceRequester}.
     */
    public ShutterSpeedParametersConfig(ShutterSpeed shutterSpeed,
                                        ISettingManager.SettingDeviceRequester deviceRequester) {
        mShutterSpeed = shutterSpeed;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        mIsSupported = ShutterSpeedHelper.isShutterSpeedSupported(originalParameters);
        mShutterSpeed.onValueInitialized(ShutterSpeedHelper.getSupportedList(originalParameters)
                , ShutterSpeedHelper.ONE_SECONDS);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (!mIsSupported) {
            return false;
        }
        String value = mShutterSpeed.getValue();
        if (value != null) {
            if (ShutterSpeedHelper.AUTO.equals(value)) {
                parameters.set(KEY_EXPOSURE_TIME, ShutterSpeedHelper.AUTO);
            } else {
                parameters.set(KEY_EXPOSURE_TIME, Integer.valueOf(value) * S_TO_MS);
            }
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mShutterSpeed.getKey());
    }

}
