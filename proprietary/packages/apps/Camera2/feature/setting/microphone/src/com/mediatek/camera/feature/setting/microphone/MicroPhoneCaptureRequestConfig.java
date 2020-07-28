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

package com.mediatek.camera.feature.setting.microphone;

import android.annotation.TargetApi;

import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.List;


/**
 * This is for MicroPhone capture flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class MicroPhoneCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            MicroPhoneCaptureRequestConfig.class.getSimpleName());

    private static final String MICROPHONE_OFF = "off";
    private static final String MICROPHONE_ON = "on";
    private ISettingManager.SettingDevice2Requester mDevice2Requester;
    private MicroPhone mMicroPhone;
    /**
     * MicroPhone capture request configure constructor.
     *
     * @param micro The instance of {@link MicroPhone}.
     * @param device2Requester The implementer of {@link ISettingManager.SettingDevice2Requester}.
     */
    public MicroPhoneCaptureRequestConfig(MicroPhone micro,
                                       ISettingManager.SettingDevice2Requester device2Requester) {
        mMicroPhone = micro;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        LogHelper.d(TAG, "setCameraCharacteristics");
        updateSupportedValues();
        mMicroPhone.updateValue(MICROPHONE_ON);
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
        mDevice2Requester.createAndChangeRepeatingRequest();
    }

    private void updateSupportedValues() {
        List<String> supported = new ArrayList<>();
        supported.add(MICROPHONE_OFF);
        supported.add(MICROPHONE_ON);
        mMicroPhone.setSupportedPlatformValues(supported);
        mMicroPhone.setEntryValues(supported);
        mMicroPhone.setSupportedEntryValues(supported);
    }
}
