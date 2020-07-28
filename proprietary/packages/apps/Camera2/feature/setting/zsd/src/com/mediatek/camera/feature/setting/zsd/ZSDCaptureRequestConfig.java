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

import android.annotation.TargetApi;
import android.app.ActivityManager;
import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.ActivityManagerExt;

import java.util.ArrayList;
import java.util.List;


/**
 * This is for EIS capture flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class ZSDCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            ZSDCaptureRequestConfig.class.getSimpleName());
    private static final String VALUE_ON = "on";
    private static final String VALUE_OFF = "off";
    private static final byte[] NON_ZSL_MODE = new byte[]{0};
    private static final byte[] ZSL_MODE = new byte[]{1};
    private SettingDevice2Requester mDeviceRequester;
    private ZSD mZsd;
    private Context mContext;

    /**
     * Zsd capture request configure constructor.
     * @param zsd The instance of {@link ZSD}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     * @param context The camera context.
     */
    public ZSDCaptureRequestConfig(ZSD zsd, SettingDevice2Requester device2Requester,
                                   Context context) {
        mZsd = zsd;
        mDeviceRequester = device2Requester;
        mContext = context;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        List<String> platformSupportedValues = new ArrayList<String>();
        platformSupportedValues.clear();
        platformSupportedValues.add(VALUE_OFF);
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mZsd.getCameraId()));
        if (deviceDescription != null && deviceDescription.isZslSupport()) {
            platformSupportedValues.add(VALUE_ON);
        }
        boolean isLowRamDevice = ActivityManagerExt.isLowRamDeviceStatic();
        String zsdOldValue = mZsd.getValue();
        LogHelper.d(TAG, "[setCameraCharacteristics] isLowRamDevice = " + isLowRamDevice
                    + " zsdOldValue = " + zsdOldValue);
        if (isLowRamDevice || VALUE_OFF.equalsIgnoreCase(zsdOldValue)) {
            mZsd.initializeValue(platformSupportedValues, VALUE_OFF);
        } else {
            mZsd.initializeValue(platformSupportedValues, VALUE_ON);
        }
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (!mZsd.isZsdSupported() || captureBuilder == null) {
            return;
        }

        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mZsd.getCameraId()));

        if (VALUE_ON.equalsIgnoreCase(mZsd.getValue())) {
            LogHelper.d(TAG, "[configCaptureRequest] zsd on");
            if (CameraUtil.isStillCaptureTemplate(captureBuilder)) {
                captureBuilder.set(CaptureRequest.CONTROL_ENABLE_ZSL, true);
            }
            captureBuilder.set(deviceDescription.getKeyZslRequestKey(), ZSL_MODE);
        } else {
            LogHelper.d(TAG, "[configCaptureRequest] zsd off");
            if (CameraUtil.isStillCaptureTemplate(captureBuilder)) {
                captureBuilder.set(CaptureRequest.CONTROL_ENABLE_ZSL, false);
            }
            if (mZsd.isSessionOn()) {
                captureBuilder.set(deviceDescription.getKeyZslRequestKey(), ZSL_MODE);
            } else {
                captureBuilder.set(deviceDescription.getKeyZslRequestKey(), NON_ZSL_MODE);
            }
        }
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestRestartSession();
    }

}
