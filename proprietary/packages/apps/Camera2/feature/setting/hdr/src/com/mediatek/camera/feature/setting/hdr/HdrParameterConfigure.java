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
package com.mediatek.camera.feature.setting.hdr;

import android.hardware.Camera.Parameters;
import android.os.Message;
import android.text.TextUtils;
import android.view.View;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.feature.setting.hdr.IHdr.HdrModeType;
import com.mediatek.camera.portability.SystemProperties;

import java.util.ArrayList;
import java.util.List;

/**
 * This class used to configure hdr value to parameters and.
 * those settings which have restriction with hdr but don't have
 * setting item in setting will configure them value in this class.
 * and just used for api1.
 */
public class HdrParameterConfigure implements ICameraSetting.IParametersConfigure, IHdr.Listener {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(HdrParameterConfigure.class.getSimpleName());
    private static final boolean sIsDualCameraSupport =
            SystemProperties.getInt("ro.vendor.mtk_cam_dualzoom_support", 0) == 1 ? true : false;
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_ASD = 0x00000002;
    private static final String KEY_HDR_AUTO_MODE = "hdr-auto-mode";
    private static final String KEY_HDR_VIDEO_VALUES = "video-hdr-values";
    private static final String KEY_HDR_VIDEO = "video-hdr";
    private static final String HDR_AUTO_VALUE = "auto";
    private static final String HDR_OFF_VALUE = "off";
    private static final String HDR_ON_VALUE = "on";
    private static final int SCENE_HDR_DETECTION_OFF = 9;
    private static final int SCENE_HDR_DETECTION_ON = 10;

    private ISettingManager.SettingDeviceRequester mSettingDeviceRequester;
    private final Hdr mHdr;
    private int mLastScene = SCENE_HDR_DETECTION_OFF;
    private HdrModeType mHdrModeType;
    private HdrParameterValues mHdrParameterValues;
    private boolean mIsPreviewStarted;
    private boolean mIsHdrDetectionSupported;

    /**
     * The construction function.
     *
     * @param hdr the Hdr class object.
     * @param requester setting device request to send value changed request.
     */
    public HdrParameterConfigure(Hdr hdr,
                                 ISettingManager.SettingDeviceRequester requester) {
        mHdr = hdr;
        mSettingDeviceRequester = requester;
        mHdrParameterValues = new HdrParameterValues();
    }

    @Override
    public void setOriginalParameters(Parameters originalParameters) {
        LogHelper.d(TAG, "[setOriginalParameters], mode type:" + mHdr.getCurrentModeType());
        if (mHdr.getCurrentModeType() == ICameraMode.ModeType.VIDEO) {
            initHdrVideoSettingValues(originalParameters);
        } else {
            initHdrPhotoSettingValues(originalParameters);
        }
        mHdrParameterValues.setOriginalHdrValue(originalParameters);
        /*hdr need keep the state before switch mode. if the state is not supported in new mode,
          set hdr to off. if hdr is not supported in the new mode, keep the state for restore state
          when mode switch.
        */
        if (mHdr.getEntryValues().size() > 1 && !mHdr.getEntryValues().contains(mHdr.getValue())) {
            mHdr.setValue(HDR_OFF_VALUE);
        } else if (mHdr.getEntryValues().size() <= 1) {
            mHdr.resetRestriction();
        }
    }

    @Override
    public boolean configParameters(Parameters parameters) {
        if (mHdr.getEntryValues().isEmpty()) {
            return false;
        }
        boolean restartPreview;
        if (mHdr.getCurrentModeType() == ICameraMode.ModeType.VIDEO) {
            restartPreview = configVideoHdrValues(parameters, mHdr.getValue());
        } else {
            restartPreview = configPhotoHdrValues(parameters, mHdr.getValue());
        }
        mHdrParameterValues.setOriginalHdrValue(parameters);
        LogHelper.d(TAG, "[configParameters], value: " + mHdr.getValue()
            + ", reconfig hdr parameters: " + restartPreview);
        return restartPreview;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        LogHelper.d(TAG, "[configCommand], value : " + mHdr.getValue());
        if (HDR_AUTO_VALUE.equals(mHdr.getValue())) {
            cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_ASD, mVendorDataCallback);
        } else {
            cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_ASD, null);
        }
    }

    @Override
    public boolean isZsdHdrSupported() {
        boolean enable = SystemProperties.getInt("ro.vendor.mtk_zsdhdr_support", 0) == 1;
        LogHelper.d(TAG, "[isZsdHdrSupported], enabled : " + enable);
        return enable;
    }

    @Override
    public boolean isMStreamHDRSupported() {
        return false;
    }

    @Override
    public void sendSettingChangeRequest() {
    }

    @Override
    public void onPreviewStateChanged(boolean isPreviewStarted) {
        mIsPreviewStarted = isPreviewStarted;
        mLastScene = SCENE_HDR_DETECTION_OFF;
        if (!isPreviewStarted || !mIsHdrDetectionSupported) {
            return;
        }
        // it always restart preview after hdr changed.
        if (HDR_AUTO_VALUE.equals(mHdr.getValue())) {
            mSettingDeviceRequester.requestChangeCommand(mHdr.getKey());
        }
    }

    @Override
    public void onHdrValueChanged() {
        mSettingDeviceRequester.requestChangeSettingValue(mHdr.getKey());
        if (HDR_AUTO_VALUE.equals(mHdr.getValue()) && mIsHdrDetectionSupported) {
            mSettingDeviceRequester.requestChangeCommand(mHdr.getKey());
        }
    }

    @Override
    public void updateModeDeviceState(String newState) {

    }

    @Override
    public void setCameraId(int id) {

    }

    private CameraProxy.VendorDataCallback mVendorDataCallback = new CameraProxy
            .VendorDataCallback() {
        @Override
        public void onDataTaken(Message message) {

        }

        @Override
        public void onDataCallback(int msgId, byte[] data, int arg1, int arg2) {
            int currentScene = arg1;
            if (!HDR_AUTO_VALUE.equals(mHdr.getValue())) {
                mLastScene = SCENE_HDR_DETECTION_OFF;
                return;
            }
            if (SCENE_HDR_DETECTION_ON != currentScene && SCENE_HDR_DETECTION_OFF != currentScene) {
                return;
            }
            if (mLastScene != currentScene) {
                LogHelper.d(TAG, "[onDataCallback], mLastScene: " + mLastScene
                        + ", currentScene: " + currentScene);
                if (SCENE_HDR_DETECTION_ON == currentScene) {
                    mHdr.onAutoDetectionResult(true);
                } else {
                    mHdr.onAutoDetectionResult(false);
                }
                mLastScene = currentScene;
            }
        }
    };

    private boolean configVideoHdrValues(Parameters parameters, String hdrValue) {
        boolean result;
        switch (mHdrModeType) {
            case ZVHDR_VIDEO:
                setVideoHdrParametersTypeZvhdr(parameters, hdrValue);
                break;
            case MVHDR_VIDEO:
                setVideoHdrParametersTypeMvhdr(parameters, hdrValue);
                break;
            default:
                break;
        }
        result = mHdrParameterValues.isParametersValueChanged(parameters);
        return result;
    }

    private boolean configPhotoHdrValues(Parameters parameters, String hdrValue) {
        boolean result;
        switch (mHdrModeType) {
            case ZVHDR_PHOTO:
                setPhotoHdrParametersTypeZvhdr(parameters, hdrValue);
                break;
            case MVHDR_PHOTP:
                setPhotoHdrParametersTypeMvhdr(parameters, hdrValue);
                //Photo mode - vhdr no need set
                break;
            case NONVHDR_PHOTO:
                setPhotoHdrParametersTypeNonhdr(parameters, hdrValue);
                //Normal sensor no need restart preview
                break;
            default:
                break;
        }
        result = mHdrParameterValues.isParametersValueChanged(parameters);
        return result;
    }

    /**
     * Used to get the hdr value with the parameter.
     */
    private class HdrParameterValues {
        private String mVideoHdrValue;

        public void setOriginalHdrValue(Parameters parameters) {
            mVideoHdrValue = parameters.get(KEY_HDR_VIDEO);
        }

        public boolean isParametersValueChanged(Parameters parameters) {
            boolean isDetectionModeChanged = false;

            if (mVideoHdrValue != null
                    && parameters.get(KEY_HDR_VIDEO) != null
                    && !mVideoHdrValue.equals(
                    parameters.get(KEY_HDR_VIDEO))) {
                isDetectionModeChanged = true;
            } else if (mVideoHdrValue == null
                    && parameters.get(KEY_HDR_VIDEO) != null) {
                if (parameters.get(KEY_HDR_VIDEO).equals("on")) {
                    isDetectionModeChanged = true;
                }
            }
            LogHelper.d(TAG, "[isParametersChanged], changed = " + isDetectionModeChanged);
            return isDetectionModeChanged;
        }
    }

    private boolean setPhotoHdrParametersTypeZvhdr(Parameters parameters, String hdrValue) {
        if (HDR_ON_VALUE.equals(hdrValue)) {
            //on
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "on");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
            //setExtendHdrRestriction(parameters);
        } else if (HDR_AUTO_VALUE.equals(hdrValue)) {
            //auto
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "on");
            parameters.set(KEY_HDR_AUTO_MODE, "on");
            //setExtendHdrRestriction(parameters);
        } else {
            //off
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
        }
        return true;
    }

    private boolean setPhotoHdrParametersTypeMvhdr(Parameters parameters, String hdrValue) {
        if (HDR_ON_VALUE.equals(hdrValue)) {
            //on
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
            //setExtendHdrRestriction(parameters);
        } else if (HDR_AUTO_VALUE.equals(hdrValue)) {
            //auto
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "on");
            //setExtendHdrRestriction(parameters);
        } else {
            //off
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
        }
        return false;
    }

    private boolean setPhotoHdrParametersTypeNonhdr(Parameters parameters, String hdrValue) {
        if (HDR_ON_VALUE.equals(hdrValue)) {
            //on
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
            //setExtendHdrRestriction(parameters);
        } else if (HDR_AUTO_VALUE.equals(hdrValue)) {
            //auto
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "on");
            //setExtendHdrRestriction(parameters);
        } else {
            //off
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
        }
        return false;
    }

    private boolean setVideoHdrParametersTypeZvhdr(Parameters parameters, String hdrValue) {
        if (HDR_ON_VALUE.equals(hdrValue)) {
            //on
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "on");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
            //setExtendHdrRestriction(parameters);
        } else if (HDR_AUTO_VALUE.equals(hdrValue)) {
            //auto
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "on");
            parameters.set(KEY_HDR_AUTO_MODE, "on");
            //setExtendHdrRestriction(parameters);
        } else {
            //off
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
        }
        return true;
    }

    private boolean setVideoHdrParametersTypeMvhdr(Parameters parameters, String hdrValue) {
        if (HDR_ON_VALUE.equals(hdrValue)) {
            //on
            parameters.setSceneMode("hdr");
            parameters.set(KEY_HDR_VIDEO, "on");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
            //setExtendHdrRestriction(parameters);
        } else {
            //off
            parameters.set(KEY_HDR_VIDEO, "off");
            parameters.set(KEY_HDR_AUTO_MODE, "off");
        }
        return true;
    }

    /**
     * Set restriction to the settings which without ui.
     */
    private void setExtendHdrRestriction(Parameters parameters) {
        for (HdrRestriction.HdrRelation relation : HdrRestriction.getHdrRelation()) {
            parameters.set(relation.getKey(), relation.getValue());
        }
    }

    private boolean isHdrDetectionSupported(Parameters parameters) {
        boolean enable = "true".equals(parameters.get("hdr-detection-supported"));
        LogHelper.d(TAG, "[isHdrDetectionSupported], enabled:" + enable);
        mIsHdrDetectionSupported = enable;
        return enable;
    }

    private boolean isVideoHdrSupported(Parameters originalParameters) {
        boolean enable = getParametersSupportedValues(
                originalParameters, KEY_HDR_VIDEO_VALUES).size() > 1;
        LogHelper.d(TAG, "[isVideoHdrSupported], enabled:" + enable);
        return enable;
    }

    private boolean isHdrSceneModeSupport(Parameters originalParameters) {
        boolean enable = false;
        List<String> supportedList = originalParameters.getSupportedSceneModes();
        if (supportedList != null && supportedList.indexOf(Parameters.SCENE_MODE_HDR) > 0) {
            enable = true;
        }
        LogHelper.d(TAG, "[isHdrSceneModeSupport], enabled:" + enable);
        return enable;
    }

    private boolean isSingleFrameHDRSupported(Parameters originalParameters) {
        boolean enable = "true".equals(originalParameters.get("single-frame-cap-hdr-supported"));
        LogHelper.d(TAG, "[isSingleFrameCapHdrSupported], enabled : " + enable);
        return enable;
    }

    private List<String> getParametersSupportedValues(Parameters parameters, String key) {
        List<String> supportedList = new ArrayList<>();
        if (parameters != null) {
            String str = parameters.get(key);
            supportedList = split(str);
        }
        return supportedList;
    }

    private ArrayList<String> split(String str) {
        ArrayList<String> substrings = new ArrayList<>();
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            for (String s : splitter) {
                substrings.add(s);
            }
        }
        LogHelper.d(TAG, "[split] (" + str + ") return " + substrings);
        return substrings;
    }

    private void initHdrPhotoSettingValues(Parameters originalParameters) {
        List<String> supportedList = new ArrayList<>();
        supportedList.add(HDR_OFF_VALUE);
        if (isHdrSceneModeSupport(originalParameters)) {
            supportedList.add(HDR_ON_VALUE);
            mHdrModeType = HdrModeType.NONVHDR_PHOTO;
        }
        if (isHdrDetectionSupported(originalParameters)) {
            supportedList.add(HDR_AUTO_VALUE);
        }
        if (isVideoHdrSupported(originalParameters)) {
            mHdrModeType = HdrModeType.MVHDR_PHOTP;
        }
        if (isVideoHdrSupported(originalParameters)
                && isSingleFrameHDRSupported(originalParameters)) {
            mHdrModeType = HdrModeType.ZVHDR_PHOTO;
        }
        if (!isHdrSceneModeSupport(originalParameters)) {
            supportedList.clear();
            mHdrModeType = HdrModeType.SCENE_MODE_DEFAULT;
        }
        mHdr.setSupportedPlatformValues(supportedList);
        mHdr.setSupportedEntryValues(supportedList);
        mHdr.setEntryValues(supportedList);
    }

    private void initHdrVideoSettingValues(Parameters originalParameters) {
        List<String> supportedList = new ArrayList<>();
        mHdrModeType = HdrModeType.NONVHDR_VIDEO;
        supportedList.add(HDR_OFF_VALUE);
        if (isVideoHdrSupported(originalParameters)) {
            supportedList.add(HDR_ON_VALUE);
            mHdrModeType = HdrModeType.MVHDR_VIDEO;
        }
        if (isVideoHdrSupported(originalParameters)
                && isSingleFrameHDRSupported(originalParameters)) {
            mHdrModeType = HdrModeType.ZVHDR_VIDEO;
        }
        if (isHdrDetectionSupported(originalParameters)
                && isVideoHdrSupported(originalParameters)
                && isSingleFrameHDRSupported(originalParameters)) {
            supportedList.add(HDR_AUTO_VALUE);
        }
        if (!isHdrSceneModeSupport(originalParameters) || sIsDualCameraSupport) {
            supportedList.clear();
            mHdrModeType = HdrModeType.SCENE_MODE_DEFAULT;
        }
        mHdr.setSupportedPlatformValues(supportedList);
        mHdr.setSupportedEntryValues(supportedList);
        mHdr.setEntryValues(supportedList);
    }

}
