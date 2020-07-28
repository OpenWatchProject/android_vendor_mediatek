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

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.ArrayList;
import java.util.List;

/**
 * Shutter speed setting item.
 */
public class ShutterSpeed extends SettingBase implements ShutterSpeedSettingView
        .OnValueChangeListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ShutterSpeed.class.getSimpleName());
    private static final String KEY_SHUTTER_SPEED = "key_shutter_speed";
    private static final String LONG_EXPOSURE_MODE_KEY =
            "com.mediatek.camera.feature.mode.longexposure.LongExposureMode";
    private String mModeKey = LONG_EXPOSURE_MODE_KEY;
    private ISettingChangeRequester mSettingChangeRequester;
    private volatile boolean mIsSupported = false;

    private ShutterSpeedSettingView mSettingView;
    private ShutterSpeedIndicatorView mIndicatorView;

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     SettingController settingController) {
        super.init(app, cameraContext, settingController);
    }

    @Override
    public void unInit() {
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        super.onModeOpened(modeKey, modeType);
        LogHelper.d(TAG, "[onModeOpened] modeKey " + modeKey);
        mModeKey = modeKey;
    }

    @Override
    public synchronized void onModeClosed(String modeKey) {
        LogHelper.d(TAG, "[onModeClosed] modeKey " + modeKey);
        super.onModeClosed(modeKey);
    }

    @Override
    public void addViewEntry() {
        if (!mIsSupported) {
            return;
        }
        LogHelper.d(TAG, "[addViewEntry]");
        if (mSettingView == null) {
            mSettingView = new ShutterSpeedSettingView(getKey(), mActivity);
            mSettingView.setOnValueChangeListener(ShutterSpeed.this);
        }
        if (mIndicatorView == null) {
            mIndicatorView = new ShutterSpeedIndicatorView(mActivity);
        }
        if (getEntryValues().size() > 1) {
            mAppUi.addSettingView(mSettingView);
            mAppUi.addToIndicatorView(mIndicatorView.getView(),
                    mIndicatorView.getViewPriority());
        }
    }

    @Override
    public void refreshViewEntry() {
        if (!mIsSupported) {
            return;
        }
        LogHelper.d(TAG, "[refreshViewEntry]");
        if (mSettingView != null) {
            mSettingView.setEntryValues(getEntryValues());
            mSettingView.setValue(getValue());
            mSettingView.setEnabled(getEntryValues().size() > 1);
        }
        if (mIndicatorView != null) {
            if (getEntryValues().size() > 1) {
                mIndicatorView.updateIndicator(getValue());
            } else {
                mAppUi.removeFromIndicatorView(mIndicatorView.getView());
            }
        }
    }

    @Override
    public void removeViewEntry() {
        if (!mIsSupported) {
            return;
        }
        LogHelper.d(TAG, "[removeViewEntry]");
        if (mSettingView != null) {
            mAppUi.removeSettingView(mSettingView);
        }
        if (mIndicatorView != null) {
            mAppUi.removeFromIndicatorView(mIndicatorView.getView());
        }
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO;
    }

    @Override
    public String getKey() {
        return KEY_SHUTTER_SPEED;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester
                    = new ShutterSpeedParametersConfig(this, mSettingDeviceRequester);
        }
        return (ShutterSpeedParametersConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester
                    = new ShutterSpeedCaptureRequestConfig(this, mSettingDevice2Requester);
        }
        return (ShutterSpeedCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public void onValueChanged(String value) {
        LogHelper.i(TAG, "[onValueChanged], value:" + value);
        if (getValue().equals(value)) {
            return;
        }
        if (mIndicatorView != null) {
            mIndicatorView.updateIndicator(value);
        }
        setValue(value);
        mDataStore.setValue(getKey(), value, getStoreScope(), false);
    }

    /**
     * Initialize values when platform supported values is ready.
     *
     * @param platformSupportedValues The platform supported values,will never be null,will
     *                                always has "auto" value.
     * @param defaultValue            The platform default values
     */
    public void onValueInitialized(List<String> platformSupportedValues,
                                   String defaultValue) {
        if (platformSupportedValues.size() <= 1) {
            LogHelper.w(TAG, "[onValueInitialized] shutter speed is not supportted");
            return;
        }
        mIsSupported = true;
        setSupportedPlatformValues(platformSupportedValues);
        setSupportedEntryValues(platformSupportedValues);
        if (LONG_EXPOSURE_MODE_KEY.equals(mModeKey)) {
            setEntryValues(platformSupportedValues);
            String value = mDataStore.getValue(getKey(), defaultValue, getStoreScope());
            setValue(value);
        } else {
            List<String> appSupportedList = new ArrayList<>();
            appSupportedList.add(ShutterSpeedHelper.AUTO);
            setEntryValues(appSupportedList);
            setValue(ShutterSpeedHelper.AUTO);
        }
    }
}
