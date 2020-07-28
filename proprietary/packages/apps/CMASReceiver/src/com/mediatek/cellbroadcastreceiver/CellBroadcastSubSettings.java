/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.mediatek.cellbroadcastreceiver;

import com.mediatek.cellbroadcastreceiver.CheckBoxAndSettingsPreference.OnSettingChangedListener;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.util.Log;
/**
 * Settings activity for the cell broadcast receiver.
 */
public class CellBroadcastSubSettings extends PreferenceActivity implements
        OnSettingChangedListener {
    private static final String TAG = "[CMAS]CellBroadcastSubSettings";
    private CheckBoxAndSettingsPreference mEnableSevereCheckBox;
    private CheckBoxAndSettingsPreference mEnableExtremeCheckBox;
    private CheckBoxAndSettingsPreference mEnableAllCheckBox;
    private int mSlotId = 0;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSlotId = getIntent().getIntExtra("slotid", 0);
        addPreferencesFromResource(R.xml.preferences_sub);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        String slotKey = CMASUtils.getSlotKey(mSlotId);
        mEnableAllCheckBox = (CheckBoxAndSettingsPreference) findPreference(
            CheckBoxAndSettingsPreference.KEY_ENABLE_ALL_IMMINENT_ALERT);
        mEnableAllCheckBox.setOnSettingChangedListener(this);
        mEnableAllCheckBox.setSlotId(mSlotId);
        mEnableAllCheckBox.setChecked(prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_ALL_IMMINENT_ALERT, true));

        mEnableExtremeCheckBox = (CheckBoxAndSettingsPreference) findPreference(
            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_EXTREME_ALERTS);
        mEnableExtremeCheckBox.setChecked(prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_EXTREME_ALERTS, true));

        mEnableSevereCheckBox = (CheckBoxAndSettingsPreference) findPreference(
            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_SEVERE_ALERTS);
        mEnableSevereCheckBox.setSlotId(mSlotId);
        mEnableSevereCheckBox.setChecked(prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_SEVERE_ALERTS, true));
        setAlertPreferenceEnable(!mEnableAllCheckBox.isChecked());
        boolean imminentAlertChecked = prefs.getBoolean(slotKey +
            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_IMMINENT_ALERTS, true);
        if (!imminentAlertChecked) {
            mEnableAllCheckBox.setEnabled(false);
            mEnableSevereCheckBox.setEnabled(false);
        }
    }

    @Override
    public void onEnableCBChanged() {
    }

    @Override
    public void onEnableAllChanged() {
        Log.d(TAG, "onEnableAllImminentChanged ");
        if (mEnableAllCheckBox.isChecked()) {
            Log.d(TAG, "onEnableAllImminentChanged true ");
            setAlertPreferenceEnable(true);
        } else {
            Log.d(TAG, "onEnableAllImminentChanged false");
            setAlertPreferenceChecked(true);
            setAlertPreferenceEnable(false);
        }
    }

    private void setAlertPreferenceChecked(boolean checked) {
        mEnableSevereCheckBox.setChecked(checked);
    }

    private void setAlertPreferenceEnable(boolean enabled) {
        mEnableSevereCheckBox.setEnabled(enabled);
    }
}
