/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.settings.development;


import android.content.Context;

import androidx.annotation.Nullable;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;      // M: add import

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.development.AbstractEnableAdbPreferenceController;
// M: add import @{
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.IDevExt;
// @}

public class AdbPreferenceController extends AbstractEnableAdbPreferenceController implements
        PreferenceControllerMixin {

    private final DevelopmentSettingsDashboardFragment mFragment;

    // M: USB preference change @{
    private IDevExt mDevExt;
    // @}

    public AdbPreferenceController(Context context, DevelopmentSettingsDashboardFragment fragment) {
        super(context);
        mFragment = fragment;
        // M: init DevExt plugin @{
        mDevExt = UtilsExt.getDevExt(context);
        // @}
    }

    // M: customize the ADB preference @{
    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mDevExt.customUSBPreference(mPreference);
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        mDevExt.customUSBPreference(mPreference);
    }
    // @}

    public void onAdbDialogConfirmed() {
        writeAdbSetting(true);
    }

    public void onAdbDialogDismissed() {
        updateState(mPreference);
    }

    @Override
    public void showConfirmationDialog(@Nullable Preference preference) {
        EnableAdbWarningDialog.show(mFragment);
    }

    @Override
    public void dismissConfirmationDialog() {
        // intentional no-op
    }

    @Override
    public boolean isConfirmationDialogShowing() {
        // intentional no-op
        return false;
    }

    @Override
    protected void onDeveloperOptionsSwitchDisabled() {
        super.onDeveloperOptionsSwitchDisabled();
        writeAdbSetting(false);
        mPreference.setChecked(false);
        // M: customize the ADB preference @{
        mDevExt.customUSBPreference(mPreference);
        // @}
    }

    // M: customize the ADB preference @{
    @Override
    public void onDeveloperOptionsEnabled() {
        super.onDeveloperOptionsEnabled();
        mDevExt.customUSBPreference(mPreference);
    }
    // @}
}
