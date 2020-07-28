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

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.PersistableBundle;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import androidx.preference.Preference;

import com.android.internal.telephony.PhoneConstants;

/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * Preference controller for "Data service setup"
 */
public class DataServiceSetupPreferenceController extends TelephonyBasePreferenceController {

    private static final String TAG = "DataServiceSetupPreferenceController";

    private CarrierConfigManager mCarrierConfigManager;
    private TelephonyManager mTelephonyManager;
    private String mSetupUrl;

    public DataServiceSetupPreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = context.getSystemService(CarrierConfigManager.class);
        mTelephonyManager = context.getSystemService(TelephonyManager.class);
        mSetupUrl = Settings.Global.getString(mContext.getContentResolver(),
                Settings.Global.SETUP_PREPAID_DATA_SERVICE_URL);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(subId);
        /// M: Add for log. @{
        final boolean hideCarrierNetwork = (carrierConfig == null
                ? true : carrierConfig.getBoolean(
                        CarrierConfigManager.KEY_HIDE_CARRIER_NETWORK_SETTINGS_BOOL));
        final int lteOnCdmaMode = mTelephonyManager.getLteOnCdmaMode();
        final boolean isLteOnCdma = (lteOnCdmaMode == PhoneConstants.LTE_ON_CDMA_TRUE);
        final boolean enabled = (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID
                && !hideCarrierNetwork && isLteOnCdma && !TextUtils.isEmpty(mSetupUrl));
        /// @}

        Log.i(TAG, "getAvailabilityStatus, subId=" + subId
                + ", enalbed=" + enabled + ", hideCarrierNetwork=" + hideCarrierNetwork
                + ", lteOnCdmaMode=" + lteOnCdmaMode + ", isLteOnCdma=" + isLteOnCdma
                + ", setupUrl=" + (TextUtils.isEmpty(mSetupUrl) ? "null" : mSetupUrl));

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

    public void init(int subId) {
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (getPreferenceKey().equals(preference.getKey())) {
            if (!TextUtils.isEmpty(mSetupUrl)) {
                String imsi = mTelephonyManager.getSubscriberId();
                if (imsi == null) {
                    imsi = "";
                }
                final String url = TextUtils.expandTemplate(mSetupUrl, imsi).toString();
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                mContext.startActivity(intent);
            }
            return true;
        }

        return false;
    }
}
