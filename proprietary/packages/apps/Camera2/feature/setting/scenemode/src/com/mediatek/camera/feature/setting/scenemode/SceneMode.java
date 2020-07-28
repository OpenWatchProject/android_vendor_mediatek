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
package com.mediatek.camera.feature.setting.scenemode;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * Picture size setting item.
 */

public class SceneMode extends SettingBase implements
        SceneModeSettingView.OnValueChangeListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SceneMode.class.getSimpleName());

    private static final String SCENE_HDR_DETECTION = "hdr-detection";
    public static final String VALUE_OF_AUTO_SCENE_DETECTION = "auto-scene-detection";

    private static final String SCENE_MODE_KEY = "key_scene_mode";
    private static final int INDICATOR_DELAY_TIME = 3000;
    private SceneModeSettingView mSettingView;
    private ISettingChangeRequester mSettingChangeRequester;
    private SceneModeIndicatorView mIndicatorView;
    private String mDetectedScene = null;
    private IAppUi.HintInfo mSceneIndicator;

    @Override
    public void init(IApp app, ICameraContext cameraContext,
                     SettingController settingController) {
        super.init(app, cameraContext, settingController);

        mSceneIndicator = new IAppUi.HintInfo();
        int id = mApp.getActivity().getResources().getIdentifier("hint_text_background",
                "drawable", mApp.getActivity().getPackageName());
        mSceneIndicator.mBackground = mActivity.getDrawable(id);
        mSceneIndicator.mType = IAppUi.HintType.TYPE_AUTO_HIDE;
        mSceneIndicator.mDelayTime = INDICATOR_DELAY_TIME;
        mSceneIndicator.mHintText = mActivity.getResources().getString(R.string.asd_hdr_guide);
        LogHelper.d(TAG, "[init]");
    }

    @Override
    public void unInit() {

    }

    @Override
    public void addViewEntry() {
        if (mSettingView == null) {
            mSettingView = new SceneModeSettingView(mActivity, getKey());
            mSettingView.setOnValueChangeListener(SceneMode.this);
        }
        if (mIndicatorView == null) {
            mIndicatorView = new SceneModeIndicatorView(mActivity);
        }
        LogHelper.d(TAG, "[addViewEntry], mSettingView:" + mSettingView);
        mAppUi.addSettingView(mSettingView);
        mAppUi.addToIndicatorView(mIndicatorView.getView(),
                mIndicatorView.getViewPriority());
    }

    @Override
    public void removeViewEntry() {
        LogHelper.d(TAG, "[removeViewEntry], mSettingView:" + mSettingView);
        mAppUi.removeSettingView(mSettingView);
        if (mIndicatorView != null) {
            mAppUi.removeFromIndicatorView(mIndicatorView.getView());
        }
    }

    @Override
    public void refreshViewEntry() {
        LogHelper.d(TAG, "[refreshViewEntry], entry values:" + getEntryValues()
                + ", value:" + getValue());
        if (mSettingView != null) {
            mSettingView.setEntryValues(getEntryValues());
            mSettingView.setValue(getValue());
            mSettingView.setEnabled(getEntryValues().size() > 1);
        }
        if (mIndicatorView != null) {
            mIndicatorView.updateIndicator(getValue());
        }
    }

    @Override
    public void postRestrictionAfterInitialized() {
        Relation relation = SceneModeRestriction.getRestrictionGroup()
                .getRelation(getValue(), false);
        if (relation != null) {
            mSettingController.postRestriction(relation);
        }
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return SCENE_MODE_KEY;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        String valueBeforeOverride = getValue();
        super.overrideValues(headerKey, currentValue, supportValues);
        LogHelper.d(TAG, "[overrideValues], headerKey:" + headerKey
                + ", currentValue:" + currentValue + ", supportValues:" + supportValues);
        if (getValue() != null && !getValue().equals(valueBeforeOverride)) {
            Relation relation = SceneModeRestriction.getRestrictionGroup()
                        .getRelation(getValue(), true);
            mSettingController.postRestriction(relation);
        }
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mSettingChangeRequester == null) {
            SceneModeParametersConfig parametersConfig =
                    new SceneModeParametersConfig(this, mSettingDeviceRequester);
            mSettingChangeRequester = parametersConfig;
        }
        return (SceneModeParametersConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mSettingChangeRequester == null) {
            SceneModeCaptureRequestConfig captureRequestConfig
                    = new SceneModeCaptureRequestConfig(mActivity, this, mSettingDevice2Requester,
                                mActivity.getApplicationContext());
            mSettingChangeRequester = captureRequestConfig;
        }
        return (SceneModeCaptureRequestConfig) mSettingChangeRequester;
    }

    /**
     * Initialize setting all values after platform supported values ready.
     *
     * @param platformSupportedValues The values current platform is supported.
     * @param defaultValue  The scene mode default value.
     */
    public void initializeValue(List<String> platformSupportedValues, String defaultValue) {
        LogHelper.d(TAG, "[initializeValue], platformSupportedValues:" + platformSupportedValues
                + "default value:" + defaultValue);
        if (platformSupportedValues == null
                || platformSupportedValues.size() <= 0) {
            return;
        }
        setSupportedPlatformValues(platformSupportedValues);

        List<String> entryValues = new ArrayList<>(platformSupportedValues);
        entryValues.remove("hdr");
        setSupportedEntryValues(entryValues);
        setEntryValues(entryValues);

        String value = mDataStore.getValue(getKey(), defaultValue, getStoreScope());
        if (!entryValues.contains(value)) {
            value = entryValues.get(0);
        }
        setValue(value);
    }

    /**
     * Scene is detected.
     *
     * @param scene The detected scene.
     */
    public void onSceneDetected(String scene) {
        LogHelper.d(TAG, "[onSceneDetected], detect scene:" + scene
                + ", last detected scene:" + mDetectedScene);
        boolean isFromHdrToOthers = SCENE_HDR_DETECTION.equals(mDetectedScene)
                && !SCENE_HDR_DETECTION.equals(scene);
        boolean isFromOthersToHdr = !SCENE_HDR_DETECTION.equals(mDetectedScene)
                && SCENE_HDR_DETECTION.equals(scene);

        // It may receive scene detected callback after scene mode changed
        // from auto to other values. So synchronized this.
        synchronized (this) {
            if (VALUE_OF_AUTO_SCENE_DETECTION.equals(getValue())) {
                mIndicatorView.updateIndicator(scene);
                if (isFromOthersToHdr) {
                    mAppUi.showScreenHint(mSceneIndicator);
                } else if (isFromHdrToOthers) {
                    mAppUi.hideScreenHint(mSceneIndicator);
                }
            }
        }
        if (VALUE_OF_AUTO_SCENE_DETECTION.equals(getValue())) {
            if (scene != null && !scene.equals(mDetectedScene)) {
                mHandler.post(new Runnable() {
                @Override
                public void run() {
                        mSettingChangeRequester.sendSettingChangeRequest();
                    }
                });
            }
        }
        mDetectedScene = scene;
    }

    @Override
    public void onValueChanged(String value) {
        LogHelper.d(TAG, "[onValueChanged], value:" + value);
        if (!getValue().equals(value)) {
            synchronized (this) {
                setValue(value);
            }
            mDataStore.setValue(getKey(), value, getStoreScope(), true);
            Relation relation = SceneModeRestriction.getRestrictionGroup()
                    .getRelation(value, true);
            mSettingController.postRestriction(relation);
            mSettingController.refreshViewEntry();
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSettingChangeRequester.sendSettingChangeRequest();
                }
            });
            mIndicatorView.updateIndicator(value);
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
