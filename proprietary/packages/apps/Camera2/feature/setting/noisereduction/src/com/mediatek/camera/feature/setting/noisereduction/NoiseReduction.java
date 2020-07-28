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

package com.mediatek.camera.feature.setting.noisereduction;


import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class is for Noise Reduction feature interacted with others.
 */

public class NoiseReduction extends SettingBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(NoiseReduction.class.getSimpleName());
    private static final String KEY_NOISE_REDUCTION = "key_noise_reduction";
    private static final String NOISE_REDUCTION_OFF = "off";
    private static final String NOISE_REDUCTION_ON = "on";
    private boolean mIsSupported = false;
    private ISettingChangeRequester mSettingChangeRequester;
    private NoiseReductionSettingView mSettingView;

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        LogHelper.d(TAG, "[init] " + this);
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "[uninit] " + this);
    }

    @Override
    public void addViewEntry() {
        if (mIsSupported) {
            mActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                  /*  mSettingView = new NoiseReductionSettingView();
                    mSettingView.setNoiseReductionViewListener(mNoiseReductionViewListener);
                    mAppUi.addSettingView(mSettingView);*/
                }
            });
        }
    }

    @Override
    public void removeViewEntry() {
        // mAppUi.removeSettingView(mSettingView);
    }

    @Override
    public void refreshViewEntry() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mSettingView != null) {
                    mSettingView.setChecked(NOISE_REDUCTION_ON.equals(getValue()));
                    mSettingView.setEnabled(getEntryValues().size() > 1);
                }
            }
        });
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue, List<String>
            supportValues) {
        super.overrideValues(headerKey, currentValue, supportValues);
        LogHelper.d(TAG, "[overrideValues] + headerKey = " + headerKey + ",currentValue = " +
                currentValue + ",supportValues " + supportValues + " ,getValue " + getValue());
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
        return KEY_NOISE_REDUCTION;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        NoiseReductionParametersConfig parametersConfig;
        if (mSettingChangeRequester == null) {
            parametersConfig = new NoiseReductionParametersConfig(this, mSettingDeviceRequester);
            mSettingChangeRequester = parametersConfig;
        }
        return (NoiseReductionParametersConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        NoiseReductionCaptureRequestConfig captureRequestConfig;
        if (mSettingChangeRequester == null) {
            captureRequestConfig =
                    new NoiseReductionCaptureRequestConfig(this, mSettingDevice2Requester);
            mSettingChangeRequester = captureRequestConfig;
        }
        return (NoiseReductionCaptureRequestConfig) mSettingChangeRequester;
    }

    private NoiseReductionSettingView.OnNoiseReductionViewListener mNoiseReductionViewListener
            = new NoiseReductionSettingView.OnNoiseReductionViewListener() {
        @Override
        public void onItemViewClick(boolean isOn) {
            LogHelper.i(TAG, "[onItemViewClick], isOn:" + isOn);
            String value = isOn ? NOISE_REDUCTION_ON : NOISE_REDUCTION_OFF;
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    setValue(value);
                    if (mSettingChangeRequester != null) {
                        mSettingChangeRequester.sendSettingChangeRequest();
                    }
                }
            });
            mDataStore.setValue(getKey(), value, getStoreScope(), true);
        }

        @Override
        public boolean onCachedValue() {
            return NOISE_REDUCTION_ON.equals(mDataStore.getValue(getKey(), NOISE_REDUCTION_ON,
                    getStoreScope()));
        }
    };

    /**
     * update set value.
     *
     * @param value the default value
     */
    public void updateValue(String value) {
        setValue(mDataStore.getValue(getKey(), value, getStoreScope()));
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // mSettingView.setChecked(NOISE_REDUCTION_ON.equals(getValue()));
            }
        });
    }

    /**
     * update whether the settings is support.
     *
     * @param isSupported the result
     */
    public void updateIsSupported(boolean isSupported) {
        mIsSupported = isSupported;
        LogHelper.d(TAG, "[updateIsSupported] mIsSupported = " + mIsSupported);
    }
}
