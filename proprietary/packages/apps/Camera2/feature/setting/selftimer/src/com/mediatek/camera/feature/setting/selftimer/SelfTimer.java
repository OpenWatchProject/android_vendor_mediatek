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
package com.mediatek.camera.feature.setting.selftimer;

import android.content.Intent;
import android.view.View;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class is for self timer feature interacted with others.
 */

public class SelfTimer extends SettingBase implements IAppUiListener.OnShutterButtonListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SelfTimer.class.getSimpleName());
    private static final int HIGHEST_PRIORITY = 10;
    private static final int DEFAULT_EXTRA_TEMER = 3;
    private static final String EXTRA_TIMER_DURATION_SECONDS =
            "com.google.assistant.extra.TIMER_DURATION_SECONDS";
    private static final String EXTRA_OPEN_ONLY_GOOGLE =
            "com.google.assistant.extra.CAMERA_OPEN_ONLY";
    private static final String EXTRA_OPEN_ONLY_ANDROID = "android.intent.extra.CAMERA_OPEN_ONLY";
    private static final String MODE_DEVICE_STATE_UNKNOWN = "unknown";
    private static final String MODE_DEVICE_STATE_PREVIEWING = "previewing";
    private static final String SELF_TIMER_STATE_KEY = "self_timer_key";
    private static final String STATE_START = "start";
    private static final String STATE_STOP = "stop";
    private StatusMonitor.StatusResponder mSelfTimerStatusResponder;
    private String mModeDeviceState = MODE_DEVICE_STATE_UNKNOWN;
    private SelfTimerCtrl mSelfTimerCtrl = new SelfTimerCtrl();
    private SelfTimerSettingView mSelfTimerSettingView;
    private List<String> mSupportValues = new ArrayList<String>();
    private boolean mIsTimerStarted = false;
    private boolean mIsOpenOnly = false;
    private boolean mIsVoiceInteractionRoot = false;
    private boolean mIsExtraTimerCaptured = false;
    private int mExtraTimer = 0;
    private String mValueBeforeVoice;

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mSelfTimerCtrl.init(app);
        mSelfTimerCtrl.setSelfTimerListener(mSelfTimerListener);
        initSettingValue();
        mAppUi.registerOnShutterButtonListener(this, HIGHEST_PRIORITY);
        mApp.registerOnOrientationChangeListener(mOrientationListener);
        mApp.registerBackPressedListener(mBackPressedListener, HIGHEST_PRIORITY);
        mSelfTimerStatusResponder = mCameraContext
                .getStatusMonitor(mSettingController.getCameraId())
                .getStatusResponder(SELF_TIMER_STATE_KEY);
    }

    @Override
    public void unInit() {
        mSelfTimerCtrl.unInit();
        mAppUi.unregisterOnShutterButtonListener(this);
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mApp.unRegisterBackPressedListener(mBackPressedListener);
    }

    @Override
    public void addViewEntry() {
        mSelfTimerSettingView = mSelfTimerCtrl.getSelfTimerSettingView();
        mSelfTimerSettingView.setOnValueChangeListener(mValueChangeListener);
        mAppUi.addSettingView(mSelfTimerSettingView);
        LogHelper.d(TAG, "[addViewEntry] getValue() :" + getValue());
    }

    @Override
    public void removeViewEntry() {
        mAppUi.removeSettingView(mSelfTimerSettingView);
        initSelfTimerResBySwitch(false);
        mIsTimerStarted = false;
        LogHelper.d(TAG, "[removeViewEntry]");
    }

    @Override
    public void refreshViewEntry() {
        int size = getEntryValues().size();
        if (mSelfTimerSettingView != null) {
            mSelfTimerSettingView.setEntryValues(getEntryValues());
            mSelfTimerSettingView.setValue(getValue());
            mSelfTimerSettingView.setEnabled(size > 1);
        }

        if (size <= 1) {
            LogHelper.d(TAG, "[overrideValues], self timer remove from UI");
            initSelfTimerResBySwitch(false);
        } else {
            initSelfTimerResBySwitch(true);
        }
    }

    @Override
    public void updateModeDeviceState(String newState) {
        mModeDeviceState = newState;
        if (mIsVoiceInteractionRoot && !mIsOpenOnly) {
            LogHelper.d(TAG, "assistant trigger take picture: mIsExtraTimerCaptured = "
                    + mIsExtraTimerCaptured);
            if (!mIsExtraTimerCaptured) {
                onShutterButtonClick();
                mIsExtraTimerCaptured = true;
                setValue(mValueBeforeVoice);
            }
        }
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return false;
    }

    @Override
    public boolean onShutterButtonClick() {
        String valueInStore = getValue();
        if (mIsTimerStarted) {
            return true;
        } else if (needSelfTimerStart(valueInStore)) {
            if (MODE_DEVICE_STATE_PREVIEWING.equals(mModeDeviceState)) {
                startSelfTimer(valueInStore);
            }
            return true;
        } else {
            return false;
        }
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
        return ISelfTimerViewListener.KEY_SELF_TIMER;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        return null;
    }

    @Override
    public void postRestrictionAfterInitialized() {}

    @Override
    public IParametersConfigure getParametersConfigure() {
        return null;
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(TAG, "onModeOpened modeKey " + modeKey);
        if (ICameraMode.ModeType.VIDEO == modeType) {
            List<String> supportValues = new ArrayList<>();
            supportValues.add(ISelfTimerViewListener.OFF);
            overrideValues(modeKey, ISelfTimerViewListener.OFF, supportValues);
        }
    }

    @Override
    public void onModeClosed(String modeKey) {
        LogHelper.d(TAG, "onModeClosed modeKey :" + modeKey);
        super.onModeClosed(modeKey);
    }

    private void startSelfTimer(String value) {
        mSelfTimerCtrl.showSelfTimerView(value);
    }

    private ISelfTimerViewListener.OnValueChangeListener mValueChangeListener
            = new ISelfTimerViewListener.OnValueChangeListener() {
        @Override
        public void onValueChanged(String value) {
            setValue(value);
            mDataStore.setValue(getKey(), value, getStoreScope(), true);
            mSelfTimerCtrl.initResBySwitch(value);
        }
    };

    private ISelfTimerViewListener.OnSelfTimerListener mSelfTimerListener
            = new ISelfTimerViewListener.OnSelfTimerListener() {
        @Override
        public void onTimerDone() {
            LogHelper.d(TAG, "[onTimerDone]");
            mSelfTimerStatusResponder.statusChanged(SELF_TIMER_STATE_KEY, STATE_STOP);
            mIsTimerStarted = false;
            mAppUi.applyAllUIVisibility(View.VISIBLE);
            mAppUi.triggerShutterButtonClick(HIGHEST_PRIORITY);
        }

        @Override
        public void onTimerInterrupt() {
            mIsTimerStarted = false;
            LogHelper.d(TAG, "[onTimerInterrupt]");
            mSelfTimerStatusResponder.statusChanged(SELF_TIMER_STATE_KEY, STATE_STOP);
            mAppUi.applyAllUIVisibility(View.VISIBLE);
        }

        @Override
        public void onTimerStart() {
            LogHelper.d(TAG, "[onTimerStart]");
            mSelfTimerStatusResponder.statusChanged(SELF_TIMER_STATE_KEY, STATE_START);
            mIsTimerStarted = true;
            mAppUi.applyAllUIVisibility(View.INVISIBLE);
        }
    };

    private IApp.OnOrientationChangeListener mOrientationListener =
            new IApp.OnOrientationChangeListener() {
                @Override
                public void onOrientationChanged(int orientation) {
                    if (mSelfTimerCtrl != null) {
                        mSelfTimerCtrl.onOrientationChanged(orientation);
                    }
                }
            };

    private IApp.BackPressedListener mBackPressedListener =
            new IApp.BackPressedListener() {
                @Override
                public boolean onBackPressed() {
                    if (mSelfTimerCtrl != null && mSelfTimerCtrl.onInterrupt()) {
                        return true;
                    }
                    return false;
                }
            };

    private void initSettingValue() {
        mSupportValues.add(ISelfTimerViewListener.OFF);
        mSupportValues.add(ISelfTimerViewListener.TWO_SECONDS);
        mSupportValues.add(ISelfTimerViewListener.TEN_SECONDS);
        setSupportedPlatformValues(mSupportValues);
        setSupportedEntryValues(mSupportValues);
        setEntryValues(mSupportValues);
        parseIntent();
        String valueInStore = mDataStore.getValue(getKey(),
                ISelfTimerViewListener.OFF, getStoreScope());
        if (mIsVoiceInteractionRoot && !mIsOpenOnly) {
            mValueBeforeVoice = valueInStore;
            valueInStore = String.valueOf(mExtraTimer);
        }
        setValue(valueInStore);
    }

    private void initSelfTimerResBySwitch(boolean isOn) {
        if (mIsVoiceInteractionRoot && !mIsExtraTimerCaptured) {
            LogHelper.d(TAG, "from voice interaction to captue, not show indicator view");
            return;
        }
        if (!isOn) {
            mSelfTimerCtrl.initResBySwitch(ISelfTimerViewListener.OFF);
            LogHelper.d(TAG, "[initSelfTimerResBySwitch] is off");
            return;
        }
        String valueInStore = getValue();
        if (needSelfTimerStart(valueInStore)) {
            mSelfTimerCtrl.initResBySwitch(valueInStore);
        }
    }

    private boolean needSelfTimerStart(String referenceValue) {
        if (ISelfTimerViewListener.OFF.equals(referenceValue)) {
            return false;
        } else {
            return true;
        }
    }

    private void parseIntent() {
        Intent intent = mActivity.getIntent();
        mIsVoiceInteractionRoot = mActivity.isVoiceInteractionRoot();
        mIsOpenOnly = intent.getBooleanExtra(EXTRA_OPEN_ONLY_ANDROID, false)
                || intent.getBooleanExtra(EXTRA_OPEN_ONLY_GOOGLE, false);
        mExtraTimer = intent.getIntExtra(EXTRA_TIMER_DURATION_SECONDS, DEFAULT_EXTRA_TEMER);
        LogHelper.d(TAG, "parseIntent() mIsVoiceInteractionRoot = " + mIsVoiceInteractionRoot
                + ", mIsOpenOnly = " + mIsOpenOnly + ", mExtraTimer = " + mExtraTimer);
    }

}
