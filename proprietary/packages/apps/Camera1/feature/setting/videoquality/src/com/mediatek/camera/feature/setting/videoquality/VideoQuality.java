/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.videoquality;

import android.app.Activity;
import android.content.Intent;
import android.media.CamcorderProfile;
import android.provider.MediaStore;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.SettingBase;


import java.util.List;

import javax.annotation.Nonnull;

/**
 * VideoQuality setting item.
 */
public class VideoQuality extends SettingBase implements
VideoQualitySettingView.OnValueChangeListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(VideoQuality.class.getSimpleName());
    private static final String KEY_VIDEO_QUALITY = "key_video_quality";
    private ISettingChangeRequester mSettingChangeRequester;
    private VideoQualitySettingView mSettingView;

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mSettingView = new VideoQualitySettingView(getKey(), this);
        mSettingView.setOnValueChangeListener(this);
    }

    @Override
    public void unInit() {

    }

    @Override
    public void addViewEntry() {
        if (!isCaptureByIntent()) {
            mAppUi.addSettingView(mSettingView);
        }
    }

    @Override
    public void removeViewEntry() {
        mAppUi.removeSettingView(mSettingView);
    }

    @Override
    public void refreshViewEntry() {
        mSettingView.setValue(getValue());
        mSettingView.setEntryValues(getEntryValues());
        mSettingView.setEnabled(getEntryValues().size() > 1);
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public SettingType getSettingType() {
        return SettingType.VIDEO;
    }

    @Override
    public String getKey() {
        return KEY_VIDEO_QUALITY;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        VideoQualityParametersConfig parametersConfig;
        if (mSettingChangeRequester == null) {
            parametersConfig = new VideoQualityParametersConfig(this, mSettingDeviceRequester);
            mSettingChangeRequester = parametersConfig;
        }
        return (VideoQualityParametersConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        VideoQualityCaptureRequestConfig captureRequestConfig;
        if (mSettingChangeRequester == null) {
            captureRequestConfig =
                    new VideoQualityCaptureRequestConfig(this, mSettingDevice2Requester);
            mSettingChangeRequester = captureRequestConfig;
        }
        return (VideoQualityCaptureRequestConfig) mSettingChangeRequester;
    }

    public String getCameraId() {
        return mSettingController.getCameraId();
    }

    public IApp getApp() {
        return mApp;
    }
    /**
     * Invoked after setting's all values are initialized.
     */
    public void onValueInitialized() {
        mSettingView.setValue(getValue());
        mSettingView.setEntryValues(getEntryValues());
    }
    /**
     * Callback when video quality value changed.
     * @param value The changed video quality, such as "1920x1080".
     */
    @Override
    public void onValueChanged(String value) {
        LogHelper.d(TAG, "[onValueChanged], value:" + value);
        if (!getValue().equals(value)) {
            setValue(value);
            mDataStore.setValue(getKey(), value, getStoreScope(), false);
            mSettingChangeRequester.sendSettingChangeRequest();
        }
    }

    /**
     * update set value.
     * @param defaultValue the default value
     */
    public void updateValue(String defaultValue) {
        String value = parseIntent();
        if (value == null) {
            value = mDataStore.getValue(getKey(), defaultValue, getStoreScope());
        }
        setValue(value);
    }

    private boolean isCaptureByIntent() {
        boolean isCaptureIntent = false;
        Activity activity = mApp.getActivity();
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            isCaptureIntent = true;
        }
        return isCaptureIntent;
    }

    private String parseIntent() {
        String quality = null;
        Activity activity = mApp.getActivity();
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            boolean userLimitQuality = intent.hasExtra(MediaStore.EXTRA_VIDEO_QUALITY);
            if (userLimitQuality) {
                int extraVideoQuality = intent.getIntExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0);
                if (extraVideoQuality > 0 &&
                        CamcorderProfile.hasProfile(
                                Integer.parseInt(getCameraId()), extraVideoQuality))  {
                    quality = Integer.toString(extraVideoQuality);
                } else {
                    quality = Integer.toString(CamcorderProfile.QUALITY_LOW);
                }
            } else {
                quality = Integer.toString(CamcorderProfile.QUALITY_LOW);
            }
        }
        return quality;
    }
}
