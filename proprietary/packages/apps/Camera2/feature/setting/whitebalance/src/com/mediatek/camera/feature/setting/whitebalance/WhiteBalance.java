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

import com.mediatek.camera.R;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * White balance setting item.
 */

public class WhiteBalance extends SettingBase implements
        WhiteBalanceSettingView.OnValueChangeListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(WhiteBalance.class.getSimpleName());

    private static final String WHITE_BALANCE_KEY = "key_white_balance";
    private static final String KEY_WHITE_BALANCE_LOCK = "white-balance-lock";
    private static final String DEBUG_MODE_KEY
            = "com.mediatek.camera.feature.mode.debug.DebugMode";

    private WhiteBalanceSettingView mSettingView;
    private WhiteBalanceIndicatorView mIndicatorView;
    private ISettingChangeRequester mSettingChangeRequester;
    private String mCurrentValue;
    private List<String> mSupportValues;

    @Override
    public void init(IApp app, ICameraContext cameraContext,
                     SettingController settingController) {
        super.init(app, cameraContext, settingController);
        LogHelper.d(TAG, "[init]");
    }

    @Override
    public void unInit() {

    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        super.onModeOpened(modeKey, modeType);
    }

    @Override
    public void onModeClosed(String modeKey) {
        super.onModeClosed(modeKey);
    }

    @Override
    public void addViewEntry() {
        LogHelper.d(TAG, "[addViewEntry] entryValue:" + getEntryValues());
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                    if (mSettingView == null) {
                        mSettingView = new WhiteBalanceSettingView(mActivity, getKey());
                        mSettingView.setOnValueChangeListener(WhiteBalance.this);
                    }
                    if (mIndicatorView == null) {
                        mIndicatorView = new WhiteBalanceIndicatorView(mActivity);
                    }

                    mAppUi.addSettingView(mSettingView);

                    mAppUi.addToIndicatorView(mIndicatorView.getView(),
                            mIndicatorView.getViewPriority());
                }
            });
    }

    @Override
    public void removeViewEntry() {
        mAppUi.removeSettingView(mSettingView);
        if (mIndicatorView != null) {
            mAppUi.removeFromIndicatorView(mIndicatorView.getView());
        }
    }

    @Override
    public void refreshViewEntry() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                    if (mSettingView != null) {
                        mSettingView.setEntryValues(getEntryValues());
                        mSettingView.setValue(getValue());
                        mSettingView.setEnabled(getEntryValues().size() > 1);
                    }

                    if (mIndicatorView != null) {
                        mIndicatorView.updateIndicator(getValue());
                    }
                }
            });
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return WHITE_BALANCE_KEY;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        if (KEY_WHITE_BALANCE_LOCK.equals(currentValue)) {
            mCurrentValue = currentValue;
            mSupportValues = supportValues;
            return;
        }
        super.overrideValues(headerKey, currentValue, supportValues);
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mSettingChangeRequester == null) {
            WhiteBalanceParametersConfig parametersConfig =
                    new WhiteBalanceParametersConfig(this, mSettingDeviceRequester);
            mSettingChangeRequester = parametersConfig;
        }
        return (WhiteBalanceParametersConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mSettingChangeRequester == null) {
            WhiteBalanceCaptureRequestConfig captureRequestConfig
                    = new WhiteBalanceCaptureRequestConfig(this, mSettingDevice2Requester);
            mSettingChangeRequester = captureRequestConfig;
        }
        return (WhiteBalanceCaptureRequestConfig) mSettingChangeRequester;
    }

    /**
     * Initialize setting all values after platform supported values ready.
     *
     * @param platformSupportedValues The values current platform is supported.
     * @param defaultValue The white balance default value.
     */
    public void initializeValue(List<String> platformSupportedValues,
                                String defaultValue) {
        LogHelper.d(TAG, "[initializeValue], platformSupportedValues:" + platformSupportedValues);
        if (platformSupportedValues == null
                || platformSupportedValues.size() <= 0) {
            return;
        }
        setSupportedPlatformValues(platformSupportedValues);
        setSupportedEntryValues(platformSupportedValues);

        List<String> entryValues = new ArrayList<>(platformSupportedValues);
        setEntryValues(entryValues);

        String value = mDataStore.getValue(getKey(), defaultValue, getStoreScope());
        if (!entryValues.contains(value)) {
            value = entryValues.get(0);
        }
        setValue(value);
    }

    /**
     * Judge white balance should lock or unlock.
     * @return True if need lock white balance,
     *         False if need unlock white balance.
     */
    public boolean isNeedLock() {
        if (mCurrentValue != null && mCurrentValue.equals(KEY_WHITE_BALANCE_LOCK)
                && mSupportValues != null && mSupportValues.contains("true")) {
                return true;
        }
        return false;
    }

    @Override
    public void onValueChanged(String value) {
        LogHelper.d(TAG, "[onValueChanged], value:" + value);
        if (!getValue().equals(value)) {
            setValue(value);
            mDataStore.setValue(getKey(), value, getStoreScope(), true);
            mSettingChangeRequester.sendSettingChangeRequest();
            mIndicatorView.updateIndicator(value);
        }
    }
}
