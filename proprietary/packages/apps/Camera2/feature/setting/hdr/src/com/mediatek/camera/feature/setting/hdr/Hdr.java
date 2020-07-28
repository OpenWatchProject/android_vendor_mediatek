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


import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class use to handle HDR feature flow.
 */
public class Hdr extends SettingBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(Hdr.class.getSimpleName());

    private static final String HDR_DEFAULT_VALUE = "off";
    private static final String HDR_ON_VALUE = "on";
    private static final String HDR_AUTO_VALUE = "auto";
    private static final String HDR_KEY = "key_hdr";

    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String KEY_DNG = "key_dng";
    private static final String KEY_FLASH = "key_flash";

    private ICameraMode.ModeType mModeType;
    private HdrParameterConfigure mHdrParameterConfigure;
    private HdrRequestConfigure mHdrRequestConfigure;
    private HdrViewController mHdrViewController;
    private IHdr.Listener mHdrDeviceListener = null;

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        LogHelper.d(TAG, "init");
        String value = mDataStore.getValue(HDR_KEY, HDR_DEFAULT_VALUE, getStoreScope());
        setValue(value);
        if (mHdrViewController == null) {
            mHdrViewController = new HdrViewController(app, this);
        }
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "unInit");
    }

    @Override
    public void addViewEntry() {
        LogHelper.d(TAG, "[addViewEntry]");
        mHdrViewController.addQuickSwitchIcon();
        mHdrViewController.showQuickSwitchIcon(getEntryValues().size() > 1);
    }

    @Override
    public void removeViewEntry() {
        LogHelper.d(TAG, "[removeViewEntry]");
        mHdrViewController.closeHdrChoiceView();
        mHdrViewController.removeQuickSwitchIcon();
    }

    @Override
    public void refreshViewEntry() {
        int num = getEntryValues().size();
        LogHelper.d(TAG, "refreshViewEntry, entry num = " + num);
        if (num <= 1) {
            mHdrViewController.showQuickSwitchIcon(false);
        } else {
            mHdrViewController.showQuickSwitchIcon(true);
        }
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        mModeType = modeType;
    }

    @Override
    public void onModeClosed(String modeKey) {
        super.onModeClosed(modeKey);
        mHdrViewController.closeHdrChoiceView();
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return HDR_KEY;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mHdrParameterConfigure == null) {
            mHdrParameterConfigure = new HdrParameterConfigure(this, mSettingDeviceRequester);
        }
        mHdrDeviceListener = (IHdr.Listener) mHdrParameterConfigure;
        return mHdrParameterConfigure;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mHdrRequestConfigure == null) {
            mHdrRequestConfigure = new HdrRequestConfigure(this, mSettingDevice2Requester,
                    mActivity.getApplicationContext());
        }
        mHdrDeviceListener = (IHdr.Listener) mHdrRequestConfigure;
        return mHdrRequestConfigure;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        String lastValue = getValue();
        LogHelper.i(TAG, "[overrideValues] headerKey = " + headerKey
                + ", currentValue = " + currentValue + ",supportValues = " + supportValues);
        if (headerKey.equals("key_flash") && currentValue != null && (currentValue != lastValue)) {
            onHdrValueChanged(currentValue);
        }
        if (!headerKey.equals("key_flash")) {
            super.overrideValues(headerKey, currentValue, supportValues);
            if (!lastValue.equals(getValue())) {
                // Continuous shot override hdr and dng off, while hdr and dng are
                // mutually exclusive, so when starting continuous shot, the hdr will
                // be off and the override that it is acted on dng will be clean. After
                // continuous shot stopped, the hdr will be overridden as off by dng.
                // So keep the override that hdr acts on dng during continuous shot. It
                // is same to dng.
                if (KEY_CSHOT.equals(headerKey)) {
                    handleHdrRestriction(true, true);
                } else {
                    handleHdrRestriction(true, false);
                }
            }
        }
    }

    @Override
    public void updateModeDeviceState(String newState) {
        //update mode state to controller
        if (IHdr.MODE_DEVICE_STATE_OPENED.equals(newState)) {
            int cameraId = Integer.parseInt(mSettingController.getCameraId());
            mHdrDeviceListener.setCameraId(cameraId);
        }
        mHdrDeviceListener.updateModeDeviceState(newState);
    }

    @Override
    public PreviewStateCallback getPreviewStateCallback() {
        return mPreviewStateCallback;
    }

    @Override
    public void postRestrictionAfterInitialized() {
        //called in setOriginalParameters
        if (getEntryValues().size() > 1) {
            handleHdrRestriction(false, false);
        }
    }

    @Override
    public String getStoreScope() {
        return mDataStore.getGlobalScope();
    }

    /**
     * Get current mode key.
     *
     * @return mModeType current mode key.
     */
    public ICameraMode.ModeType getCurrentModeType() {
        return mModeType;
    }

    /**
     * Used to update view for auto detection.
     *
     * @param isAutoVisiable true, means show.
     */
    public void onAutoDetectionResult(boolean isAutoVisiable) {
        mHdrViewController.showHdrIndicator(isAutoVisiable);
    }

    /**
     * Used to reset restriction since the new mode may be not support hdr.
     */
    public void resetRestriction() {
        Relation hdrRelation = HdrRestriction.getHdrRestriction().getRelation(
                HDR_DEFAULT_VALUE, true);
        LogHelper.d(TAG, "[resetRestriction] hdr");
        mSettingController.postRestriction(hdrRelation);
    }

    /**
     * Used to update hdr current value and post restriction.
     *
     * @param value current hdr value.
     */
    public void onHdrValueChanged(String value) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (!value.equals(getValue())) {
                    LogHelper.d(TAG, "[onHdrValueChanged] value = " + value);

                    setValue(value);
                    removeExclusionOverrides();
                    handleHdrRestriction(true, false);
                    mSettingController.refreshViewEntry();
                    mDataStore.setValue(HDR_KEY, value, getStoreScope(), false, true);
                    //request device to change setting value.
                    mHdrDeviceListener.onHdrValueChanged();
                }
            }
        });
    }

    private PreviewStateCallback mPreviewStateCallback =
            new PreviewStateCallback() {
                @Override
                public void onPreviewStopped() {
                    LogHelper.d(TAG, "[onPreviewStopped] +");
                    if (mHdrDeviceListener != null) {
                        mHdrDeviceListener.onPreviewStateChanged(false);
                    }
                }

                @Override
                public void onPreviewStarted() {
                    LogHelper.d(TAG, "[onPreviewStarted] +");
                    if (mHdrDeviceListener != null) {
                        mHdrDeviceListener.onPreviewStateChanged(true);
                    }
                }
            };

    private void handleHdrRestriction(boolean empty, boolean withoutDng) {
        String hdrCurrentValue = getValue();
        Relation hdrRelation = HdrRestriction.getHdrRestriction().getRelation(hdrCurrentValue,
                empty);
        if (hdrRelation == null) {
            return;
        }
        if (withoutDng) {
            hdrRelation.removeBody(KEY_DNG);
        }

        if (HDR_ON_VALUE.equals(hdrCurrentValue) && mHdrDeviceListener.isMStreamHDRSupported()) {
            hdrRelation.addBody("key_zsd", "on", "on");
        } else if (HDR_ON_VALUE.equals(hdrCurrentValue) || HDR_AUTO_VALUE.equals(hdrCurrentValue)) {
            hdrRelation.addBody("key_zsd", "off", "off");
        }

        LogHelper.d(TAG, "[postRestriction] hdr");
        mSettingController.postRestriction(hdrRelation);
    }

    private void removeExclusionOverrides() {
        removeOverride(KEY_DNG);
        removeOverride(KEY_FLASH);
    }
}
