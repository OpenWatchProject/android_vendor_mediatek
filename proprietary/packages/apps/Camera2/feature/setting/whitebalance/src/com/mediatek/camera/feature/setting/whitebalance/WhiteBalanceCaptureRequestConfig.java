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
package com.mediatek.camera.feature.setting.whitebalance;

import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * White balance capture request configure.
 */

public class WhiteBalanceCaptureRequestConfig implements
        ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            WhiteBalanceCaptureRequestConfig.class.getSimpleName());

    private WhiteBalance mWhiteBalance;
    private SettingDevice2Requester mDevice2Requester;

    /**
     * Scene mode enum value.
     */
    enum ModeEnum {
        OFF(0),
        AUTO(1),
        INCANDESCENT(2),
        FLUORESCENT(3),
        WARM_FLUORESCENT(4),
        DAYLIGHT(5),
        CLOUDY_DAYLIGHT(6),
        TWILIGHT(7),
        SHADE(8);

        private int mValue = 0;
        ModeEnum(int value) {
            this.mValue = value;
        }

        /**
         * Get enum value which is in integer.
         *
         * @return The enum value.
         */
        public int getValue() {
            return this.mValue;
        }

        /**
         * Get enum name which is in string.
         *
         * @return The enum name.
         */
        public String getName() {
            return this.toString();
        }
    }

    /**
     * White balance capture quest configure constructor.
     *
     * @param whiteBalance The instance of {@link WhiteBalance}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     */
    public WhiteBalanceCaptureRequestConfig(WhiteBalance whiteBalance,
                                         SettingDevice2Requester device2Requester) {
        mWhiteBalance = whiteBalance;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        int[] modeEnums = characteristics.get(CameraCharacteristics.CONTROL_AWB_AVAILABLE_MODES);

        List<String> modeNames = convertEnumToString(modeEnums);
        mWhiteBalance.initializeValue(modeNames, "auto");
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        String value = mWhiteBalance.getValue();
        LogHelper.d(TAG, "[configCaptureRequest], value:" + value);
        if (value != null) {
            captureBuilder.set(CaptureRequest.CONTROL_AWB_MODE, convertStringToEnum(value));
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

    private List<String> convertEnumToString(int[] enumIndexs) {
        ModeEnum[] modes = ModeEnum.values();
        List<String> names = new ArrayList<>(enumIndexs.length);
        for (int i = 0; i < enumIndexs.length; i++) {
            int enumIndex = enumIndexs[i];
            for (ModeEnum mode : modes) {
                if (mode.getValue() == enumIndex) {
                    String name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                    names.add(name);
                    break;
                }
            }
        }
        return names;
    }

    private int convertStringToEnum(String value) {
        int enumIndex = 0;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            String modeName = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
            if (modeName.equalsIgnoreCase(value)) {
                enumIndex = mode.getValue();
            }
        }
        return enumIndex;
    }
}
