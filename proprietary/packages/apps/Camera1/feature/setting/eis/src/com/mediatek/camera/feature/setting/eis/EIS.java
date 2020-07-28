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

package com.mediatek.camera.feature.setting.eis;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class is for EIS feature interacted with others.
 */

public class EIS extends SettingBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(EIS.class.getSimpleName());
    private static final String KEY_NOISE_EIS = "key_eis";
    private static final String EIS_OFF = "off";
    private static final String EIS_ON = "on";
    private boolean mIsSupported = false;
    private ISettingChangeRequester mSettingChangeRequester;
    private EISSettingView mSettingView;

    @Override
    public void init(IApp app, ICameraContext cameraContext, SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mSettingView = new EISSettingView();
        mSettingView.setEISViewListener(mEISViewListener);
    }

    @Override
    public void unInit() {

    }

    @Override
    public void addViewEntry() {
        mAppUi.addSettingView(mSettingView);
    }

    @Override
    public void removeViewEntry() {
        mAppUi.removeSettingView(mSettingView);
    }

    @Override
    public void refreshViewEntry() {
        if (mSettingView != null) {
            mSettingView.setChecked(EIS_ON.equals(getValue()));
            mSettingView.setEnabled(getEntryValues().size() > 1);
        }
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
        return KEY_NOISE_EIS;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        EISParametersConfig parametersConfig;
        if (mSettingChangeRequester == null) {
            parametersConfig = new EISParametersConfig(this, mSettingDeviceRequester, mActivity);
            mSettingChangeRequester = parametersConfig;
        }
        return (EISParametersConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        EISCaptureRequestConfig captureRequestConfig;
        if (mSettingChangeRequester == null) {
            captureRequestConfig = new EISCaptureRequestConfig(this, mSettingDevice2Requester);
            mSettingChangeRequester = captureRequestConfig;
        }
        return (EISCaptureRequestConfig) mSettingChangeRequester;
    }

    private EISSettingView.OnEISViewListener mEISViewListener
            = new EISSettingView.OnEISViewListener() {
        @Override
        public void onItemViewClick(boolean isOn) {
            LogHelper.d(TAG, "[onItemViewClick], isOn:" + isOn);
            String value = isOn ? EIS_ON : EIS_OFF;
            setValue(value);
            mDataStore.setValue(getKey(), value, getStoreScope(), false);
            mSettingChangeRequester.sendSettingChangeRequest();
        }

        @Override
        public boolean onCachedValue() {
            return EIS_ON.equals(
                    mDataStore.getValue(getKey(), EIS_OFF, getStoreScope()));
        }
    };

    /**
     * update set value.
     * @param value the default value
     */
    public void updateValue(String value) {
        setValue(mDataStore.getValue(getKey(), value, getStoreScope()));
    }

}
