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
package com.mediatek.camera.feature.setting.format;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;

import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class is for self timer feature interacted with others.
 */

public class Format extends SettingBase implements IAppUiListener.OnShutterButtonListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(Format.class.getSimpleName());
    private static final int HIGHEST_PRIORITY = 10;
    private FormatCtrl mFormatCtrl = new FormatCtrl();
    private FormatSettingView mFormatSettingView;
    private List<String> mSupportValues = new ArrayList<String>();
    public static String FORMAT_JPEG = "jpeg";
    public static String FORMAT_HEIF = "heif";
    private ISettingChangeRequester mSettingChangeRequester;

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mFormatCtrl.init(app);
        initSettingValue();
        mAppUi.registerOnShutterButtonListener(this, HIGHEST_PRIORITY);
    }

    @Override
    public void unInit() {
        mFormatCtrl.unInit();
        mAppUi.unregisterOnShutterButtonListener(this);
    }

    @Override
    public void addViewEntry() {
        mFormatSettingView = mFormatCtrl.getFormatSettingView();
        mFormatSettingView.setOnValueChangeListener(mValueChangeListener);
        mAppUi.addSettingView(mFormatSettingView);
        LogHelper.d(TAG, "[addViewEntry] getValue() :" + getValue());
    }

    @Override
    public void removeViewEntry() {
        mAppUi.removeSettingView(mFormatSettingView);
        LogHelper.d(TAG, "[removeViewEntry]");
    }

    @Override
    public void refreshViewEntry() {
        int size = getEntryValues().size();
        if (mFormatSettingView != null) {
            mFormatSettingView.setEntryValues(getEntryValues());
            mFormatSettingView.setValue(getValue());
            mFormatSettingView.setEnabled(size > 1);
        }
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        String lastValue = getValue();
        super.overrideValues(headerKey, currentValue, supportValues);
        String curValue = getValue();
        if (!lastValue.equals(curValue)) {
            mSettingController.refreshViewEntry();
            mFormatCtrl.setFormatStatus(curValue, false);
            mAppUi.refreshSettingView();
            mSettingController.postRestriction(
                    FormatRestriction.getRestriction().getRelation(curValue, true));
            if (mSettingChangeRequester != null) {
                mSettingChangeRequester.sendSettingChangeRequest();
            }
        }
    }

    @Override
    public void updateModeDeviceState(String newState) {
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return false;
    }

    @Override
    public boolean onShutterButtonClick() {
        return false;
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        return false;
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO;
    }

    @Override
    public String getKey() {
        return IFormatViewListener.KEY_FORMAT;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester
                    = mFormatCtrl.getCaptureRequestConfigure(mSettingDevice2Requester);
        }
        return (FormatCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public void postRestrictionAfterInitialized() {
        mSettingController.postRestriction(
                FormatRestriction.getRestriction().getRelation(getValue(), true));
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        return null;
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(TAG, "onModeOpened modeKey " + modeKey);
        super.onModeOpened(modeKey, modeType);
    }

    @Override
    public void onModeClosed(String modeKey) {
        LogHelper.d(TAG, "onModeClosed modeKey :" + modeKey);
        super.onModeClosed(modeKey);
    }

    private IFormatViewListener.OnValueChangeListener mValueChangeListener
            = new IFormatViewListener.OnValueChangeListener() {
        @Override
        public void onValueChanged(String value) {
            setValue(value);
            mDataStore.setValue(getKey(), value, getStoreScope(), true);
            mSettingController.refreshViewEntry();
            mFormatCtrl.setFormatStatus(value, false);
            mAppUi.refreshSettingView();
            mSettingController.postRestriction(
                    FormatRestriction.getRestriction().getRelation(value, true));
            if (mSettingChangeRequester != null) {
                mSettingChangeRequester.sendSettingChangeRequest();
            }
        }
    };

    private void initSettingValue() {
        mSupportValues.add(FORMAT_JPEG);
        mSupportValues.add(FORMAT_HEIF);
        setSupportedPlatformValues(mSupportValues);
        setSupportedEntryValues(mSupportValues);
        setEntryValues(mSupportValues);
        String valueInStore = mDataStore.getValue(getKey(), FORMAT_JPEG, getStoreScope());
        setValue(valueInStore);
    }
}
