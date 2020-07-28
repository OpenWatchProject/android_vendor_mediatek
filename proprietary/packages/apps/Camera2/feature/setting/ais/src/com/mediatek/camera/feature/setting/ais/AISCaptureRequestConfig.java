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

import android.annotation.TargetApi;

import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;


/**
 * This is for AIS capture flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class AISCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(AISCaptureRequestConfig.class.getSimpleName());

    private static final String AIS_AVAILABLE_MODES_KEY_NAME
            = "com.mediatek.mfnrfeature.availablemfbmodes";
    private static final String AIS_REQUEST_MODE_KEY_NAME
            = "com.mediatek.mfnrfeature.mfbmode";
    private static final String AIS_RESULT_MODE_KEY_NAME
            = "com.mediatek.mfnrfeature.mfbresult";

    private static final String VALUE_OFF = "off";
    private static final String VALUE_ON = "on";
    private static final String VALUE_MFLL = "mfll";
    private static final String VALUE_AIS = "ais";
    private static final String VALUE_AUTO = "auto";

    private CameraCharacteristics.Key<int[]> mAisAvailableModesKey = null;
    private CaptureRequest.Key<int[]> mAisRequestModeKey = null;
    private CaptureResult.Key<int[]> mAisResultModeKey = null;
    private AIS mAis;
    private SettingDevice2Requester mDeviceRequester;
    private List<String> mSupportedValues = new ArrayList<>();
    private Context mContext;

    /**
     * AIS mode enum value.
     */
    enum ModeEnum {
        OFF(0),
        MFLL(1),
        AIS(2),
        AUTO(255);

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
     * AIS capture request configure constructor.
     * @param ais The instance of {@link AIS}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     * @param context The camera context.
     */
    public AISCaptureRequestConfig(AIS ais, SettingDevice2Requester device2Requester,
                                   Context context) {
        mContext = context;
        mAis = ais;
        mDeviceRequester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        boolean isAisSupported = true;
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mAis.getCameraId()));
        if (deviceDescription != null) {
            mAisAvailableModesKey =  deviceDescription.getKeyAisAvailableModes();
        }


        if (mAisAvailableModesKey == null) {
            LogHelper.d(TAG, "ais available modes key isn't existed");
            isAisSupported = false;
        }

        if (deviceDescription != null) {
            mAisRequestModeKey = deviceDescription.getKeyAisRequestMode();
        }


        if (mAisRequestModeKey == null) {
            LogHelper.d(TAG, "ais request key isn't existed");
            isAisSupported = false;
        }

        if (deviceDescription != null) {
            mAisResultModeKey = deviceDescription.getKeyAisResult();
        }


        if (mAisResultModeKey == null) {
            LogHelper.d(TAG, "asd result key isn't existed");
            isAisSupported = false;
        }

        List<String> supportedModes = new ArrayList<>();
        if (isAisSupported) {
            int[] modes = characteristics.get(mAisAvailableModesKey);
            mSupportedValues.addAll(convertEnumToString(modes));
            if (!mSupportedValues.contains(VALUE_AIS)) {
                LogHelper.d(TAG, "do not support ais value");
            } else {
                supportedModes.add(VALUE_OFF);
                supportedModes.add(VALUE_ON);
            }
        }
        mAis.initializeValue(supportedModes, VALUE_OFF);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        String value = mAis.getValue();
        String overrideValue = mAis.getOverrideValue();
        LogHelper.d(TAG, "[configCaptureRequest], value:" + value
                + ", ais override value:" + overrideValue);
        if (value == null) {
            return;
        }
        if (VALUE_OFF.equals(value)) {
            if (!VALUE_OFF.equals(overrideValue)
                    && mSupportedValues.contains(VALUE_AUTO)) {
                value = VALUE_AUTO;
            }
        }
        if (VALUE_ON.equals(value)) {
            value = VALUE_AIS;
        }
        int[] mode = new int[1];
        mode[0] = convertStringToEnum(value);
        LogHelper.d(TAG, "[configCaptureRequest], mode[0]:" + mode[0]);
        captureBuilder.set(mAisRequestModeKey, mode);
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.createAndChangeRepeatingRequest();
    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    private List<String> convertEnumToString(int[] enumIndexs) {
        if (enumIndexs == null) {
            LogHelper.d(TAG, "[convertEnumToString], convert enum indexs is null");
            return new ArrayList<>();
        }
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

    private String convertEnumToString(int enumIndex) {
        String name = null;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            if (mode.getValue() == enumIndex) {
                name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                break;
            }
        }
        return name;
    }

    private int convertStringToEnum(String value) {
        int enumIndex = 0;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            String modeName = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
            if (modeName.equalsIgnoreCase(value)) {
                enumIndex = mode.getValue();
                break;
            }
        }
        return enumIndex;
    }
}
