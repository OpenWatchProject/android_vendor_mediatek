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
/// M: Add for recording the phone Id.
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.euicc.EuiccManager;
import android.text.TextUtils;
import android.util.Log;

import androidx.preference.Preference;

/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * Preference controller for "Euicc preference"
 */
public class EuiccPreferenceController extends TelephonyBasePreferenceController {

    private static final String TAG = "EuiccPreferenceController";

    private TelephonyManager mTelephonyManager;

    public EuiccPreferenceController(Context context, String key) {
        super(context, key);
        mTelephonyManager = context.getSystemService(TelephonyManager.class);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        /// M: Add for log.
        final boolean showEuicc = MobileNetworkUtils.showEuiccSettings(mContext);

        Log.d(TAG, "getAvailabilityStatus, subId=" + subId + ", showEuicc=" + showEuicc);

        return showEuicc ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
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
    public CharSequence getSummary() {
        return mTelephonyManager.getSimOperatorName();
    }

    public void init(int subId) {
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (TextUtils.equals(preference.getKey(), getPreferenceKey())) {
            Intent intent = new Intent(EuiccManager.ACTION_MANAGE_EMBEDDED_SUBSCRIPTIONS);
            mContext.startActivity(intent);
            return true;
        }

        return false;
    }
}
