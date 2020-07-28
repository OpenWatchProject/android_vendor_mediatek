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

import android.os.PersistableBundle;
/// M: Add for updating the state.
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreference;

import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.settings.R;

/// @}

import java.util.ArrayList;
import java.util.List;


import mediatek.telephony.MtkCarrierConfigManager;
import com.mediatek.ims.internal.MtkImsManager;

/**
 * Preference controller for "Enhanced 4G LTE"
 */
public class CustomizeEnable4point5GPreferenceController extends TelephonyTogglePreferenceController {


    private static final String TAG = "CustomizeEnable4point5GPreferenceController";

    private Preference mPreference;
    private PreferenceScreen mPreferenceScreen;
    private TelephonyManager mTelephonyManager;
    private CarrierConfigManager mCarrierConfigManager;
    private PersistableBundle mCarrierConfig;
    private ImsManager mImsManager;
    private MtkImsManager mMtkImsManager;
    private int mPhoneId;
    private SwitchPreference mSwitchPreference;
    /// @}

    public CustomizeEnable4point5GPreferenceController(Context context, String key) {
        super(context, key);
    }

    public void init(int subId) {
        mSubId = subId;
        mCarrierConfigManager = mContext.getSystemService(CarrierConfigManager.class);
        mPhoneId = SubscriptionManager.getPhoneId(mSubId);
        mMtkImsManager = (MtkImsManager)ImsManager.getInstance(mContext, mPhoneId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
    }

    @Override
    public int getAvailabilityStatus(int subId) {

        PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(subId);
        boolean isVisible = (carrierConfig == null
                ? false : carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_SHOW_45G_OPTIONS));
        Log.d(TAG, "getAvailabilityStatusd, subId=" + subId
                + ", 4point5OptionFeatureSupport=" + isVisible);
        /// @}
        return isVisible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreferenceScreen = screen;
        mPreference = screen.findPreference(getPreferenceKey());
    }

    @Override
    public void updateState(Preference preference) {
        Log.i(TAG, "updateState, preference=" + preference);
        super.updateState(preference);
        mSwitchPreference = (SwitchPreference) preference;
        mSwitchPreference.setChecked(is4point5GSettingOn());
    }
    /**
     * Check 4.5 Setting.
     * @return Check if Enable 4.5 Setting is ON or not
     */
    private boolean is4point5GSettingOn() {
        boolean enh4glteMode = mMtkImsManager.isEnhanced4gLteModeSettingEnabledByUser()
            && mMtkImsManager.isNonTtyOrTtyOnVolteEnabled(mContext, mPhoneId);
        int settingsNetworkMode = android.provider.Settings.Global.getInt(
                        mContext.getContentResolver(),
                        android.provider.Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                        Phone.PREFERRED_NT_MODE);
        boolean isLteMode = false;
        if (settingsNetworkMode >= TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO) {
            isLteMode = true;
        }
        return enh4glteMode && isLteMode;
    }

    @Override
    public boolean isChecked() {
        return is4point5GSettingOn();
    }

    @Override
    public boolean setChecked(boolean isChecked) {
        /// M: enable4_5_settings enable volte and
        int networkMode = android.provider.Settings.Global.getInt(
                mContext.getContentResolver(),
                android.provider.Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                Phone.PREFERRED_NT_MODE);
        if (isChecked) {
            networkMode = TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA;
            Log.i(TAG, "Updated mode on unchecked:" + networkMode);
        } else {
            networkMode = TelephonyManager.NETWORK_MODE_WCDMA_PREF;
            Log.i(TAG, "Updated mode on checked:" + networkMode);
        }
        android.provider.Settings.Global.putInt(
                mContext.getContentResolver(),
                android.provider.Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                networkMode);
        //Set the modem network mode
        mTelephonyManager.setPreferredNetworkType(mSubId, networkMode);
        return true;
    }
}