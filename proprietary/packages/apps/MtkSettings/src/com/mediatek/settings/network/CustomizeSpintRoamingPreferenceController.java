/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.mediatek.settings.network;

import com.android.settings.network.telephony.TelephonyBasePreferenceController;

import android.content.Context;
import android.content.Intent;
import android.os.PersistableBundle;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.util.Log;

import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import mediatek.telephony.MtkCarrierConfigManager;

/**
 * Preference controller for "Apn settings"
 */
public class CustomizeSpintRoamingPreferenceController extends TelephonyBasePreferenceController {

    private static final String TAG = "CustomizeSpintRoamingPreferenceController";

    CarrierConfigManager mCarrierConfigManager;
    private Preference mPreference;
    public static String SUB_ID_EXTRA = "extra_subid";
    public CustomizeSpintRoamingPreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = new CarrierConfigManager(context);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(subId);
        boolean isAvailable = (carrierConfig == null
                ? false :carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_ROAMING_BAR_GUARD_BOOL));

        Log.i(TAG, "getAvailabilityStatus, subId=" + subId
                + ", isAvailable=" + isAvailable);
        return isAvailable ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }
    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (getPreferenceKey().equals(preference.getKey())) {
            Intent intent = new Intent(mContext, RoamingSettings.class);
            intent.putExtra(SUB_ID_EXTRA, mSubId);
            mContext.startActivity(intent);
            return true;
        }
        return false;
    }

    public void init(int subId) {
        mSubId = subId;
    }

}
