/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.android.camera.manager;

import android.content.res.TypedArray;
import android.hardware.Camera.Parameters;
import android.view.View;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.ParametersHelper;
import com.android.camera.R;
import com.android.camera.SettingUtils;
import com.android.camera.ui.RotateImageView;

import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.preference.IconListPreference;
import com.mediatek.camera.setting.preference.ListPreference;

import java.util.List;

public class IndicatorManager extends ViewManager implements
        CameraActivity.OnParametersReadyListener, CameraActivity.OnPreferenceReadyListener {
    private static final String TAG = "IndicatorManager";

    private static final int[] VIEW_IDS = new int[] {
        R.id.onscreen_dng_indicator,
        R.id.onscreen_white_balance_indicator,
        R.id.onscreen_scene_indicator,
        R.id.onscreen_exposure_indicator,
        R.id.onscreen_selftimer_indicator,
        R.id.onscreen_voice_indicator,
    };

    private static final int INDICATOR_COUNT = VIEW_IDS.length;

    private static final String[] SETTING_KEYS = new String[] {
        SettingConstants.KEY_DNG,
        SettingConstants.KEY_WHITE_BALANCE,
        SettingConstants.KEY_SCENE_MODE,
        SettingConstants.KEY_EXPOSURE,
        SettingConstants.KEY_SELF_TIMER,
        SettingConstants.KEY_VOICE,
    };

    private static final boolean[] FROM_PARAMETERS = new boolean[] {
        false,
        true,
        true,
        true,
        false,
        false,
        false,
    };
    private static final int ROW_DNG = 0;
    private static final int ROW_WHITE_BALANCE = 1;
    private static final int ROW_SCENE_MODE = 2;
    private static final int ROW_EXPOSURE = 3;
    private static final int ROW_SELF_TIME = 4;
    private static final int ROW_VOICE = 5;

    private RotateImageView[] mViews = new RotateImageView[INDICATOR_COUNT];
    private ListPreference[] mPrefs = new ListPreference[INDICATOR_COUNT];
    private String[] mDefaults = new String[INDICATOR_COUNT];
    private String[] mOverrides = new String[INDICATOR_COUNT];
    private boolean mPreferenceReady;
    private View mIndicatorGroup;
    private boolean[] mVisibles;

    public IndicatorManager(CameraActivity context) {
        super(context);
        context.addOnParametersReadyListener(this);
        context.addOnPreferenceReadyListener(this);
        // disable animation for cross with remaining.
        setAnimationEnabled(true, false);
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.onscreen_indicators);
        for (int i = 0; i < INDICATOR_COUNT; i++) {
            mViews[i] = (RotateImageView) view.findViewById(VIEW_IDS[i]);
        }
        mIndicatorGroup = view.findViewById(R.id.on_screen_group);
        return view;
    }

    public void onPreferenceReady() {
        for (int i = 0; i < INDICATOR_COUNT; i++) {
            String key = SETTING_KEYS[i];
            mPrefs[i] = getContext().getListPreference(key);
            mDefaults[i] = getContext().getISettingCtrl().getDefaultValue(key);
        }
        mPreferenceReady = true;
    }

    public void onCameraParameterReady() {
        refreshModeIndicator(true);
        refresh();
    }

    @Override
    public void onRefresh() {
        if (!mPreferenceReady || getContext().isSwitchingCamera()) {
            Log.w(TAG, "onRefresh() why refresh before preference ready? ", new Throwable());
            return;
        }
        refreshModeIndicator(false);
        int showcount = 0;
        for (int i = 0; i < INDICATOR_COUNT; i++) {
            String key = SETTING_KEYS[i];
            String value = null;
            if (mOverrides[i] != null) {
                value = mOverrides[i]; // override value
            } else {
                value = getContext().getISettingCtrl().getSettingValue(key);
                if (SettingConstants.KEY_SCENE_MODE.equals(key)) {
                    if (ParametersHelper.KEY_SCENE_MODE_HDR.equals(value)) {
                        value = Parameters.SCENE_MODE_AUTO;
                    }
                }
            }

            if (mPrefs[i] instanceof IconListPreference) {
                if (!mVisibles[i] || value == null
                        || (mDefaults[i] != null && mDefaults[i].equals(value))) {
                    mViews[i].setVisibility(View.GONE);
                } else {
                    mViews[i].setVisibility(View.VISIBLE);
                    IconListPreference iconPref = ((IconListPreference) mPrefs[i]);
                    if (iconPref.getOriginalIconIds() != null) {
                        // we may disable some entry values for unsupported
                        // cases.
                        // so here search original value for dynamic cases.
                        int index = SettingUtils.index(iconPref.getOriginalEntryValues(), value);
                        mViews[i].setImageResource(iconPref.getOriginalIconIds()[index]);
                    }
                    showcount++;
                }
            } else {
                // If one feature is only supported by main camera, its indicator should be
                // cleared when switch to front camera.
                mViews[i].setVisibility(View.GONE);
//                Log.d(TAG, "key:" + key + ", pref:" + mPrefs[i]);
            }
        }
        if (showcount > 0) {
            mIndicatorGroup.setBackgroundResource(R.drawable.bg_indicator_background);
        } else {
            mIndicatorGroup.setBackgroundDrawable(null);
        }
    }

    public synchronized void refreshModeIndicator(boolean force) {
//        Log.d(TAG, "refreshModeIndicator(" + force + ") mVisibles=" + mVisibles);
        if (mVisibles == null || force) {
            mVisibles = new boolean[INDICATOR_COUNT];
            for (int i = 0; i < INDICATOR_COUNT; i++) {
                boolean visible = true;
                String key = SETTING_KEYS[i];
                int row = SettingConstants.getSettingId(key);
                if (getContext().isImageCaptureIntent()) {

                    visible = SettingUtils
                            .contains(SettingConstants.SETTING_GROUP_CAMERA_FOR_UI, row);
                } else if (getContext().isVideoMode()) {
                    visible = SettingUtils.contains(SettingConstants.SETTING_GROUP_VIDEO_FOR_UI,
                            row);
                }
                mVisibles[i] = visible;
            }
        }
    }

    // AsdActor should save original scene and restore it when release it.
    public void saveSceneMode() {
//        Log.d(TAG, "saveSceneMode() mPreferenceReady=" + mPreferenceReady);
        // Clear all overrider values and set it to auto as default.
        getContext().getISettingCtrl().onSettingChanged(SettingConstants.KEY_SCENE_MODE,
                Parameters.SCENE_MODE_AUTO);
        getContext().notifyPreferenceChanged(null);
    }

    public void restoreSceneMode() {
        for (int i = 0, len = mOverrides.length; i < len; i++) {
            mOverrides[i] = null;
        }
//        Log.d(TAG, "restoreSceneMode() mPreferenceReady=" + mPreferenceReady);
    }

    public void onDetectedSceneMode(int scene) {
        // Application shouldn't keep scene[int]-scene[String] mapping.
        // Here we maintain this logic until native FPM correct this bad logic.
        TypedArray asdModeMapping = getContext().getResources().obtainTypedArray(
                R.array.scenemode_native_mapping_entryvalues);
        String sceneMode = asdModeMapping.getString(scene);
        asdModeMapping.recycle();
        // here notify preference changed!
        String localOverride = mOverrides[ROW_SCENE_MODE];
        String preferenceValue = Parameters.SCENE_MODE_AUTO;
        if (!sceneMode.equals(localOverride)) {
            // Set local override value to native detected value.
            mOverrides[ROW_SCENE_MODE] = sceneMode;

            Parameters parameters = getContext().getParameters();
            List<String> supportedSceneModes = parameters.getSupportedSceneModes();
            if (supportedSceneModes != null && supportedSceneModes.contains(sceneMode)) {
                preferenceValue = sceneMode;
            }

            getContext().getISettingCtrl().onSettingChanged(SettingConstants.KEY_SCENE_MODE,
                    preferenceValue);
            getContext().notifyPreferenceChanged(null);
            refresh();
        }
    }
}
