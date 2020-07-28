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
package com.mediatek.camera.feature.setting.flash;

import android.app.Activity;
import android.content.Intent;
import android.provider.MediaStore;
import android.view.View;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * Class used to handle flash feature flow.
 */
public class Flash extends SettingBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(Flash.class.getSimpleName());

    public static final String FLASH_AUTO_VALUE = "auto";
    public static final String FLASH_OFF_VALUE = "off";
    public static final String FLASH_ON_VALUE = "on";
    private static final String FLASH_DEFAULT_VALUE = "off";
    private static final String FLASH_KEY = "key_flash";
    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String VALUE_CSHOT_START = "start";
    private static final String VALUE_CSHOT_STOP = "stop";
    private ICameraMode.ModeType mModeType;
    private String mCurrentMode = "com.mediatek.camera.common.mode.photo.PhotoMode";
    private String mSdofMode = "com.mediatek.camera.feature.mode.vsdof.photo.SdofPhotoMode";

    private String mLongExposureMode =
            "com.mediatek.camera.feature.mode.longexposure.LongExposureMode";
    private String mPanoramaMode =
            "com.mediatek.camera.feature.mode.panorama.PanoramaMode";
    private String mHdrMode =
            "com.mediatek.camera.feature.mode.hdr.HdrMode";
    private FlashParameterConfigure mFlashParameterConfigure;
    private ICaptureRequestConfigure mFlashRequestConfigure;
    private FlashViewController mFlashViewController;
    private ISettingChangeRequester mSettingChangeRequester;

    private static final String VIDEO_STATUS_KEY = "key_video_status";
    private static final String VIDEO_STATUS_RECORDING = "recording";
    private static final String VIDEO_STATUS_PREVIEW = "preview";
    // [Add for CCT tool] Receive keycode and set flash on/set flash off @{
    private IApp.KeyEventListener mKeyEventListener;
    // @}

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        String value = mDataStore.getValue(FLASH_KEY, FLASH_DEFAULT_VALUE, getStoreScope());
        setValue(value);
        if (mFlashViewController == null) {
            mFlashViewController = new FlashViewController(this, app);
        }
        mStatusMonitor.registerValueChangedListener(VIDEO_STATUS_KEY, mStatusChangeListener);
        mStatusMonitor.registerValueChangedListener(KEY_CSHOT, mStatusChangeListener);
        // [Add for CCT tool] Receive keycode and enable/disable flash @{
        mKeyEventListener = mFlashViewController.getKeyEventListener();
        mApp.registerKeyEventListener(mKeyEventListener, IApp.DEFAULT_PRIORITY);
        // @}
    }

    @Override
    public void unInit() {
        mStatusMonitor.unregisterValueChangedListener(VIDEO_STATUS_KEY,
                mStatusChangeListener);
        mStatusMonitor.unregisterValueChangedListener(KEY_CSHOT, mStatusChangeListener);
        // [Add for CCT tool] Receive keycode and enable/disable flash @{
        if (mKeyEventListener != null) {
            mApp.unRegisterKeyEventListener(mKeyEventListener);
        }
        // @}
    }

    @Override
    public void addViewEntry() {
        mFlashViewController.addQuickSwitchIcon();
        mFlashViewController.showQuickSwitchIcon(getEntryValues().size() > 1);
    }

    @Override
    public void removeViewEntry() {
        mFlashViewController.removeQuickSwitchIcon();
    }

    @Override
    public void refreshViewEntry() {
        int num = getEntryValues().size();
        if (num <= 1) {
            mFlashViewController.showQuickSwitchIcon(false);
        } else {
            mFlashViewController.showQuickSwitchIcon(true);
        }
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        mCurrentMode = modeKey;
        mModeType = modeType;
    }

    @Override
    public void onModeClosed(String modeKey) {
        mFlashViewController.hideFlashChoiceView();
        if (mFlashRequestConfigure != null) {
            ((FlashRequestConfigure) mFlashRequestConfigure).changeFlashToTorchByAeState(false);
        }
        super.onModeClosed(modeKey);
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return FLASH_KEY;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mFlashParameterConfigure == null) {
            mFlashParameterConfigure = new FlashParameterConfigure(this, mSettingDeviceRequester);
        }
        mSettingChangeRequester = mFlashParameterConfigure;
        return mFlashParameterConfigure;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mFlashRequestConfigure == null) {
            mFlashRequestConfigure = new FlashRequestConfigure(this, mSettingDevice2Requester);
        }
        mSettingChangeRequester = mFlashRequestConfigure;
        return mFlashRequestConfigure;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        LogHelper.d(TAG, "[overrideValues] headerKey = " + headerKey
                + " ,currentValue = " + currentValue + ",supportValues = " + supportValues);
        if (headerKey.equals("key_scene_mode") && mSettingController.queryValue("key_scene_mode")
                .equals("hdr")) {
            return;
        }
        String lastValue = getValue();
        if (headerKey.equals("key_hdr") && currentValue != null && (currentValue != lastValue)) {
            onFlashValueChanged(currentValue);
        }

        if (!headerKey.equals("key_hdr")) {
            super.overrideValues(headerKey, currentValue, supportValues);
            if (!lastValue.equals(getValue())) {
                Relation relation = FlashRestriction.getFlashRestriction()
                        .getRelation(getValue(), true);
                mSettingController.postRestriction(relation);
            }
            mActivity.runOnUiThread(new Runnable() {
                public void run() {
                    if (supportValues != null) {
                        mFlashViewController.showQuickSwitchIcon(supportValues.size() > 1);
                    } else {
                        if (isFlashSupportedInCurrentMode()) {
                            mFlashViewController.showQuickSwitchIcon(getEntryValues().size() > 1);
                        }
                    }
                }
            });
        }
    }

    @Override
    public void postRestrictionAfterInitialized() {
        Relation relation = FlashRestriction.getFlashRestriction().getRelation(getValue(), false);
        if (relation != null) {
            mSettingController.postRestriction(relation);
        }
    }

    /**
     * Get current mode type.
     *
     * @return mModeType current mode type.
     */
    public ICameraMode.ModeType getCurrentModeType() {
        return mModeType;
    }

    /**
     * Called when flash value changed.
     *
     * @param value The new value.
     */
    public void onFlashValueChanged(String value) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (!value.equals(getValue())) {
                    LogHelper.d(TAG, "[onFlashValueChanged] value = " + value);
                    setValue(value);
                    mSettingController.postRestriction(
                            FlashRestriction.getFlashRestriction().getRelation(value, true));
                    mSettingController.refreshViewEntry();
                    mSettingChangeRequester.sendSettingChangeRequest();
                    mDataStore.setValue(FLASH_KEY, value, getStoreScope(), false, true);
                }
            }
        });
    }

    protected boolean isThirdPartyIntent() {
        Activity activity = mApp.getActivity();
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        boolean value = MediaStore.ACTION_IMAGE_CAPTURE.equals(action) ||
                MediaStore.ACTION_VIDEO_CAPTURE.equals(action);
        return value;
    }

    private StatusMonitor.StatusChangeListener mStatusChangeListener = new StatusMonitor
            .StatusChangeListener() {

        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.d(TAG, "[onStatusChanged] + key " + key + "," +
                    "value " + value);
            switch (key) {
                case VIDEO_STATUS_KEY:
                    //only API2 need to check whether ae state flash required
                    if (mFlashRequestConfigure == null) {
                        return;
                    }
                    if (VIDEO_STATUS_RECORDING.equals(value)) {
                        ((FlashRequestConfigure) mFlashRequestConfigure)
                                .changeFlashToTorchByAeState(true);
                    } else if (VIDEO_STATUS_PREVIEW.equals(value)) {
                        ((FlashRequestConfigure) mFlashRequestConfigure)
                                .changeFlashToTorchByAeState(false);
                    }
                    break;
                case KEY_CSHOT:
                    //only API2 need to check whether ae state flash required
                    if (mFlashRequestConfigure == null) {
                        return;
                    }
                    if (VALUE_CSHOT_START.equals(value)) {
                        ((FlashRequestConfigure) mFlashRequestConfigure)
                                .changeFlashToTorchByAeState(true);
                    } else if (VALUE_CSHOT_STOP.equals(value)) {
                        ((FlashRequestConfigure) mFlashRequestConfigure)
                                .changeFlashToTorchByAeState(false);
                    }
                    break;
                default:
                    break;
            }
            LogHelper.d(TAG, "[onStatusChanged] -");
        }
    };

    private boolean isFlashSupportedInCurrentMode() {
        return !mCurrentMode.equals(mLongExposureMode) && !mCurrentMode.equals(mSdofMode)
                &&!mCurrentMode.equals(mPanoramaMode)&&!mCurrentMode.equals(mHdrMode);
    }

}
