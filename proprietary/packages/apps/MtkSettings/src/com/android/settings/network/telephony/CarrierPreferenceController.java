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

package com.android.settings.network.telephony;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;

/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * Preference controller for "Carrier Settings"
 */
public class CarrierPreferenceController extends TelephonyBasePreferenceController {

    private static final String TAG = "CarrierPreferenceController";

    @VisibleForTesting
    CarrierConfigManager mCarrierConfigManager;

    public CarrierPreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = new CarrierConfigManager(context);
    }

    public void init(int subId) {
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(subId);

        /// M: Add for log. @{
        final boolean enableCarrier = (carrierConfig == null
                ? false : carrierConfig.getBoolean(
                        CarrierConfigManager.KEY_CARRIER_SETTINGS_ENABLE_BOOL));
        final boolean isCdmaOpt = MobileNetworkUtils.isCdmaOptions(mContext, subId);
        final boolean isGsmOpt = MobileNetworkUtils.isGsmOptions(mContext, subId);
        final boolean enabled = enableCarrier && (isCdmaOpt || isGsmOpt);
        /// @}

        Log.i(TAG, "getAvailabilityStatus, subId=" + subId
                + ", enalbed=" + enabled + ", enableCarrier=" + enableCarrier
                + ", isCdmaOpt=" + isCdmaOpt + ", isGsmOpt=" + isGsmOpt);

        // Return available if it is in CDMA or GSM mode, and the flag is on
        return enabled ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    /// M: Add more condition for enabled state. @{
    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        preference.setEnabled(
                MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId));
    }
    /// @}

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (getPreferenceKey().equals(preference.getKey())) {
            final Intent carrierSettingsIntent = getCarrierSettingsActivityIntent(mSubId);
            if (carrierSettingsIntent != null) {
                mContext.startActivity(carrierSettingsIntent);
            }
            return true;
        }

        return false;
    }

    private Intent getCarrierSettingsActivityIntent(int subId) {
        final PersistableBundle config = mCarrierConfigManager.getConfigForSubId(subId);
        final ComponentName cn = ComponentName.unflattenFromString(
                config == null ? "" : config.getString(
                        CarrierConfigManager.KEY_CARRIER_SETTINGS_ACTIVITY_COMPONENT_NAME_STRING,
                        "" /* default value */));

        if (cn == null) return null;

        final Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.setComponent(cn);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, subId);

        final PackageManager pm = mContext.getPackageManager();
        final ResolveInfo resolveInfo = pm.resolveActivity(intent, 0 /* flags */);
        return resolveInfo != null ? intent : null;
    }
}
