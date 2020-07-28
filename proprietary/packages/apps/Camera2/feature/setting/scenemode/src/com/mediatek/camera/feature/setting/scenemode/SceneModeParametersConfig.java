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
package com.mediatek.camera.feature.setting.scenemode;

import android.hardware.Camera;
import android.os.Message;
import android.text.TextUtils;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Scene mode parameters configure.
 */

public class SceneModeParametersConfig implements
        ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            SceneModeParametersConfig.class.getSimpleName());

    private static final String VALUE_AUTO = "auto";
    private static final String VALUE_OFF = "off";
    private static final String VALUE_NIGHT = "night";
    private static final String VALUE_HDR_DETECTION = "hdr-detection";
    private static final String VALUE_PORTRAIT = "portrait";
    private static final String VALUE_LANDSCAPE = "landscape";
    private static final String VALUE_NIGHT_PORTRAIT = "night-portrait";
    private static final String VALUE_BACKLIGHT_PORTRAIT = "backlight-portrait";

    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_ASD = 0x00000002;
    private static final String SUPPORTED_VALUES_SUFFIX = "-values";
    private static final String KEY_CAPTURE_MODE = "cap-mode";
    private static final String CAPTURE_MODE_ASD = "asd";

    private SceneMode mSceneMode;
    private SettingDeviceRequester mDeviceRequester;
    private List<String> mSupportedSceneMode = new ArrayList<>();
    private String mValue;
    private String mDetectedValue;
    private boolean mIsAsdChanged;
    private CameraProxy mCameraProxy;

    private static Map<Integer, String> sSceneMapping = new HashMap<>();
    static {
        sSceneMapping.put(0, VALUE_OFF);
        sSceneMapping.put(1, VALUE_NIGHT);
        sSceneMapping.put(2, VALUE_HDR_DETECTION);
        sSceneMapping.put(3, VALUE_PORTRAIT);
        sSceneMapping.put(4, VALUE_LANDSCAPE);
        sSceneMapping.put(5, VALUE_OFF);
        sSceneMapping.put(6, VALUE_NIGHT_PORTRAIT);
        sSceneMapping.put(7, VALUE_OFF);
        sSceneMapping.put(8, VALUE_BACKLIGHT_PORTRAIT);
    }

    /**
     * Scene mode parameters config constructor.
     *
     * @param sceneMode The instance of {@link SceneMode}.
     * @param deviceRequester The implementer of {@link SettingDeviceRequester}.
     */
    public SceneModeParametersConfig(SceneMode sceneMode,
                                     SettingDeviceRequester deviceRequester) {
        mSceneMode = sceneMode;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        List<String> supportedSceneModes = originalParameters.getSupportedSceneModes();
        List<String> supportedCaptureModes = split(
                originalParameters.get(KEY_CAPTURE_MODE + SUPPORTED_VALUES_SUFFIX));

        if (supportedSceneModes != null) {
            int index = supportedSceneModes.indexOf(VALUE_AUTO);
            if (index != -1) {
                supportedSceneModes.set(index, VALUE_OFF);
            }
        }

        if (supportedCaptureModes != null
                && supportedCaptureModes.indexOf(CAPTURE_MODE_ASD) > 0) {
            supportedSceneModes.add(SceneMode.VALUE_OF_AUTO_SCENE_DETECTION);
        }
        mSupportedSceneMode = new ArrayList<>(supportedSceneModes);
        mSceneMode.initializeValue(supportedSceneModes, originalParameters.getSceneMode());
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        String value = mSceneMode.getValue();
        if (value == null) {
            return false;
        }
        if (VALUE_OFF.equals(value)) {
            value = VALUE_AUTO;
        }
        LogHelper.d(TAG, "[configParameters], value:" + value
                + ", lastValue:" + mValue + ", mDetectedValue:" + mDetectedValue);
        if (SceneMode.VALUE_OF_AUTO_SCENE_DETECTION.equals(value)) {
            parameters.set(KEY_CAPTURE_MODE, CAPTURE_MODE_ASD);
            if (mSupportedSceneMode.contains(mDetectedValue)) {
                parameters.setSceneMode(mDetectedValue);
            } else {
                parameters.setSceneMode(Camera.Parameters.SCENE_MODE_AUTO);
            }
            parameters.setExposureCompensation(0);
            parameters.setColorEffect(Camera.Parameters.EFFECT_NONE);
        } else {
            parameters.setSceneMode(value);
        }

        boolean isValueChanged = !value.equals(mValue);
        if (isValueChanged) {
            boolean isNullToOff = (mValue == null
                    && Camera.Parameters.SCENE_MODE_AUTO.equals(value));
            mIsAsdChanged = SceneMode.VALUE_OF_AUTO_SCENE_DETECTION.equals(value)
                    || SceneMode.VALUE_OF_AUTO_SCENE_DETECTION.equals(mValue);
            mValue = value;
            if (!isNullToOff) {
                // If scene mode value is changed, it needs to set scene mode parameters
                // to native twice. Because native layer will override the parameters which
                // is affected by scene mode, like:flash, exposure, white balance and so on,
                // however, the override value maybe not AP layer wants to set. So, scene mode
                // setting firstly set parameter to let native layer finish override work,
                // then mode will set the parameters which AP layer wants to set again later.
                if (mCameraProxy == null) {
                    // if camera proxy is null, temporary send setting change request
                    // again to set parameters once more, and send command request to
                    // get camera proxy instance. So it can directly set parameters by
                    // camera proxy later.
                    sendSettingChangeRequest();
                    mDeviceRequester.requestChangeCommand(mSceneMode.getKey());
                } else {
                    mCameraProxy.setParameters(parameters);
                }
            }
            if (mIsAsdChanged) {
                mDeviceRequester.requestChangeCommand(mSceneMode.getKey());
            }
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        LogHelper.d(TAG, "[configCommand], mIsAsdChanged:" + mIsAsdChanged);
        mCameraProxy = cameraProxy;
        // Config command method will be invoke when scene mode parameter is configured at
        // first time, because the mCameraProxy is null, however in this time the auto scene
        // detection maybe not changed, in this case, it don't need to set asd callback.
        if (mIsAsdChanged) {
            setAsdCallback(cameraProxy);
        }
    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mSceneMode.getKey());
    }

    private List<String> split(String str) {
        if (str == null) {
            return null;
        }

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        List<String> subStrings = new ArrayList<>();
        for (String s : splitter) {
            subStrings.add(s);
        }
        return subStrings;
    }

    private void setAsdCallback(CameraProxy cameraProxy) {
        String value = mSceneMode.getValue();
        if (SceneMode.VALUE_OF_AUTO_SCENE_DETECTION.equals(value)) {
            cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_ASD,
                    new CameraProxy.VendorDataCallback() {
                        @Override
                        public void onDataTaken(Message message) {
                            LogHelper.d(TAG, "[onDataTaken], message:" + message);
                            mSceneMode.onSceneDetected(sSceneMapping.get(message.arg2));
                        }

                        @Override
                        public void onDataCallback(int msgId, byte[] data, int arg1, int arg2) {
                            LogHelper.d(TAG, "[onDataCallback], arg1:" + arg1);
                            mDetectedValue = sSceneMapping.get(arg1);
                            mSceneMode.onSceneDetected(mDetectedValue);
                        }
                    });
        } else {
            cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_ASD, null);
            mDetectedValue = null;
        }
    }
}
