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

package com.mediatek.camera.feature.setting.zsd;

import android.content.Intent;
import android.provider.MediaStore;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * ZSD setting.
 */

public class ZSD extends SettingBase implements ZSDSettingView.OnZsdClickListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ZSD.class.getSimpleName());

    private static final String VALUE_ON = "on";
    private static final String VALUE_OFF = "off";
    private static final String KEY_ZSD = "key_zsd";
    private ZSDSettingView mSettingView;
    private ISettingChangeRequester mSettingChangeRequester;
    private boolean mSessionValue = false;
    private boolean mIsZsdSupported = false;
    private boolean mIsThirdParty = false;
    private StatusMonitor.StatusResponder mZSDStateStatusResponder;

    // [Add for CCT tool] Receive keycode and enable/disable ZSD @{
    private IApp.KeyEventListener mKeyEventListener;
    // @}
    private List<String> mPlatformSupportedValues = null;

    @Override
    public void init(IApp app, ICameraContext cameraContext, SettingController settingController) {
        super.init(app, cameraContext, settingController);
        Intent intent = mActivity.getIntent();
        String action = intent.getAction();
        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
                || MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            mIsThirdParty = true;
        }
        String value = mDataStore.getValue(getKey(), VALUE_ON, getStoreScope());
        LogHelper.d(TAG, "[init], value:" + value);
        setValue(value);
        mZSDStateStatusResponder = mStatusMonitor.getStatusResponder(KEY_ZSD);
    }

    @Override
    public void unInit() {
        // [Add for CCT tool] Receive keycode and enable/disable ZSD @{
        if (mKeyEventListener != null) {
            mApp.unRegisterKeyEventListener(mKeyEventListener);
        }
        // @}
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        if (!mIsZsdSupported) {
            return;
        }
        super.overrideValues(headerKey, currentValue, supportValues);
    }


    @Override
    public void addViewEntry() {
        if (!mIsZsdSupported) {
            return;
        }
        if (mSettingView == null) {
            mSettingView = new ZSDSettingView(getKey());
            mSettingView.setZsdOnClickListener(this);
            // [Add for CCT tool] Receive keycode and enable/disable ZSD @{
            mKeyEventListener = mSettingView.getKeyEventListener();
            mApp.registerKeyEventListener(mKeyEventListener, IApp.DEFAULT_PRIORITY);
            // @}
        }
        mAppUi.addSettingView(mSettingView);
    }

    @Override
    public void removeViewEntry() {
        if (!mIsZsdSupported) {
            return;
        }
        mAppUi.removeSettingView(mSettingView);
    }

    @Override
    public void refreshViewEntry() {
        if (mSettingView != null) {
            mSettingView.setChecked(VALUE_ON.equals(getValue()));
            mSettingView.setEnabled(getEntryValues().size() > 1);
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
        return KEY_ZSD;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mIsThirdParty) {
            return null;
        }
        if (mSettingChangeRequester == null) {
            ZSDParametersConfig config = new ZSDParametersConfig(this, mSettingDeviceRequester);
            mSettingChangeRequester = config;
        }
        return (ZSDParametersConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mIsThirdParty) {
            return null;
        }
        if (mSettingChangeRequester == null) {
            ZSDCaptureRequestConfig config = new ZSDCaptureRequestConfig(
                    this, mSettingDevice2Requester, mActivity.getApplicationContext());
            mSettingChangeRequester = config;
        }
        return (ZSDCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public void onZsdClicked(boolean checked) {
        String value = checked ? VALUE_ON : VALUE_OFF;
        LogHelper.d(TAG, "[onZsdClicked], value:" + value);
        setValue(value);
        if (VALUE_ON.equalsIgnoreCase(value)) {
            mSessionValue = true;
        } else {
            mSessionValue = false;
        }
        mDataStore.setValue(getKey(), value, getStoreScope(), false);
        mZSDStateStatusResponder.statusChanged(getKey(), value);
        mHandler.post(new Runnable() {
                @Override
                public void run() {
        mSettingChangeRequester.sendSettingChangeRequest();
    }
        });
    }

    public boolean isZsdSupported() {
        return mIsZsdSupported;
    }

    public boolean isSessionOn() {
        return mSessionValue;
    }

    /**
     * Initialize zsd values when platform supported values is ready.
     *
     * @param platformSupportedValues The platform supported values.
     * @param defaultValue The platform default values
     */
    public void initializeValue(List<String> platformSupportedValues,
                                String defaultValue) {
        LogHelper.d(TAG, "[initializeValue], platformSupportedValues:" + platformSupportedValues
                + ", defaultValue:" + defaultValue);
        if (platformSupportedValues != null && platformSupportedValues.size() > 1) {
            mIsZsdSupported = true;
            if (mPlatformSupportedValues == null) {
                if (VALUE_ON.equals(defaultValue)) {
                    mSessionValue = true;
                } else {
                    mSessionValue = false;
                }
                mPlatformSupportedValues = platformSupportedValues;
            }
            setSupportedPlatformValues(platformSupportedValues);
            setSupportedEntryValues(platformSupportedValues);
            setEntryValues(platformSupportedValues);
            String value = mDataStore.getValue(getKey(), defaultValue, getStoreScope());
            setValue(value);
        }
    }

    /**
     * Get current camera id.
     * @return The current camera id.
     */
    protected int getCameraId() {
        int cameraId = Integer.parseInt(mSettingController.getCameraId());
        return cameraId;
    }

}
