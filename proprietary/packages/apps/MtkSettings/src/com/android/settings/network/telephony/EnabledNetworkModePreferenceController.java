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
/// Add for setting the preferred network type async. @{
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
/// @}
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
/// M: Add for revising the entry values.
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.VisibleForTesting;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
/// M: Add for handling phone type change.
import androidx.preference.PreferenceScreen;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.settings.R;

/// M: Add for revising the entry values.
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.settings.UtilsExt;
/// M: Add for checking CT SIM.
import com.mediatek.settings.sim.TelephonyUtils;

import mediatek.telephony.MtkCarrierConfigManager;

/**
 * Preference controller for "Enabled network mode"
 */
public class EnabledNetworkModePreferenceController extends
        TelephonyBasePreferenceController implements
        ListPreference.OnPreferenceChangeListener {

    private static final String TAG = "EnabledNetworkModePreferenceController";

    private CarrierConfigManager mCarrierConfigManager;
    private TelephonyManager mTelephonyManager;
    private boolean mIsGlobalCdma;
    @VisibleForTesting
    boolean mShow4GForLTE;

    /// M: Add for supporting CTVoLTE. @{
    private SubscriptionManager mSubscriptionManager;
    private int mPhoneType;
    private boolean mShowCdmaOption = false;
    /// @}

    /// M: Add for supporting 5G.
    private String m5gDisplayName = "5G";

    public EnabledNetworkModePreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = context.getSystemService(CarrierConfigManager.class);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        /// M: for China mobile feature,user can not select network type @{
        boolean customerVisible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizeAutoSelectNetworkTypePreference();
        boolean cmccOpenMarketvisible
                = SystemProperties.get("ro.vendor.cmcc_light_cust_support").equals("1");
        boolean customerTMOVisible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizeTMONetworkModePreference(subId);
        boolean customerATTVisible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizeATTNetworkModePreference(subId);
        boolean customerVWZVisible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizeVWZNetworkModePreference(subId);
        boolean customerRjiovisible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizeRjioNetworkModePreference(subId);
        customerVisible = customerVisible
                || cmccOpenMarketvisible
                || customerTMOVisible
                || customerATTVisible
                || customerVWZVisible
                || customerRjiovisible;
        if (customerVisible) {
            return customerVisible ? CONDITIONALLY_UNAVAILABLE : AVAILABLE;
        }
        /// @ }
        /// M: Add for checking modem version for China Telecom SIM card. @{
        if (!TelephonyUtils.is93OrLaterModem() && MobileNetworkUtils.isCtSim(mPhoneId)) {
            Log.d(TAG, "getAvailabilityStatus, CT SIM for old modem.");
            return CONDITIONALLY_UNAVAILABLE;
        }
        /// @}

        boolean visible;
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(subId);
        final TelephonyManager telephonyManager = TelephonyManager
                .from(mContext).createForSubscriptionId(subId);
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Log.d(TAG, "getAvailabilityStatus, subId is invalid.");
            visible = false;
        } else if (carrierConfig == null) {
            Log.d(TAG, "getAvailabilityStatus, carrierConfig is null.");
            visible = false;
        } else if (carrierConfig.getBoolean(
                CarrierConfigManager.KEY_HIDE_CARRIER_NETWORK_SETTINGS_BOOL)) {
            Log.d(TAG, "getAvailabilityStatus, KEY_HIDE_CARRIER_NETWORK_SETTINGS_BOOL is true.");
            visible = false;
        } else if (carrierConfig.getBoolean(
                CarrierConfigManager.KEY_HIDE_PREFERRED_NETWORK_TYPE_BOOL)
                && !telephonyManager.getServiceState().getRoaming()
                && telephonyManager.getServiceState().getDataRegState()
                == ServiceState.STATE_IN_SERVICE) {
            Log.d(TAG, "getAvailabilityStatus, KEY_HIDE_PREFERRED_NETWORK_TYPE_BOOL is true.");
            visible = false;
        } else if (carrierConfig.getBoolean(CarrierConfigManager.KEY_WORLD_PHONE_BOOL)) {
            Log.d(TAG, "getAvailabilityStatus, KEY_WORLD_PHONE_BOOL is true.");
            visible = false;
        } else if (carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_SHOW_45G_OPTIONS)){
            Log.d(TAG, "getAvailabilityStatus, MTK_KEY_SHOW_45G_OPTIONS is true.");
            visible = false;
        } else {
            visible = true;
        }

        return visible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        final ListPreference listPreference = (ListPreference) preference;
        final int networkMode = getPreferredNetworkMode();
        /// M: Add for supporting CTVoLTE. @{
        mPhoneType = mTelephonyManager.getPhoneType();
        if (mPhoneType != PhoneConstants.PHONE_TYPE_CDMA) {
            mShowCdmaOption = MobileNetworkUtils.shouldShowCdmaOption(mContext, mSubId);
        } else {
            mShowCdmaOption = true;
        }
        /// @}
        updatePreferenceEntries(listPreference);
        updatePreferenceValueAndSummary(listPreference, networkMode);
        /// M: Add more condition for enabled state.
        listPreference.setEnabled(mEnableState
                && MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId));
        /// M: Add for checking capability.
        updatePreferenceForCapability(listPreference);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object object) {
        final int settingsMode = Integer.parseInt((String) object);

        /// M: Add for setting the preferred network type async. @{
        preference.setEnabled(false);
        AsyncTask.execute(
                new PreferredNetworkTypeSyncTask(mTelephonyManager,
                        mSubId, settingsMode));
        /// @}

        return false;
    }

    public void init(int subId) {
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);

        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(mSubId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);

        /// M: Add for support CTVoLTE.
        mSubscriptionManager = SubscriptionManager.from(mContext);

        final boolean isLteOnCdma =
                mTelephonyManager.getLteOnCdmaMode() == PhoneConstants.LTE_ON_CDMA_TRUE;
        mIsGlobalCdma = isLteOnCdma
                && carrierConfig.getBoolean(CarrierConfigManager.KEY_SHOW_CDMA_CHOICES_BOOL);
        mShow4GForLTE = carrierConfig != null
                ? carrierConfig.getBoolean(
                CarrierConfigManager.KEY_SHOW_4G_FOR_LTE_DATA_ICON_BOOL)
                : false;
    }

    private int getPreferredNetworkMode() {
        return Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                Phone.PREFERRED_NT_MODE);
    }

    private void updatePreferenceEntries(ListPreference preference) {
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(mSubId);
        /// M: Add for supporting CTVoLTE.
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA || mShowCdmaOption) {
            final int lteForced = android.provider.Settings.Global.getInt(
                    mContext.getContentResolver(),
                    android.provider.Settings.Global.LTE_SERVICE_FORCED + mSubId,
                    0);
            /// M: Add for log.
            final int lteOnCdmaMode = mTelephonyManager.getLteOnCdmaMode();
            final boolean isLteOnCdma = lteOnCdmaMode == PhoneConstants.LTE_ON_CDMA_TRUE;
            final int settingsNetworkMode = android.provider.Settings.Global.getInt(
                    mContext.getContentResolver(),
                    android.provider.Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                    Phone.PREFERRED_NT_MODE);
            Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                    + ", phoneType=" + mPhoneType + ", lteForced=" + lteForced
                    + ", lteOnCdmaMode=" + lteOnCdmaMode
                    + ", settingsNetworkMode=" + settingsNetworkMode);
            if (isLteOnCdma) {
                if (lteForced == 0) {
                    preference.setEntries(
                            R.array.enabled_networks_cdma_choices);
                    preference.setEntryValues(
                            R.array.enabled_networks_cdma_values);
                } else {
                    switch (settingsNetworkMode) {
                        case TelephonyManager.NETWORK_MODE_CDMA_EVDO:
                        case TelephonyManager.NETWORK_MODE_CDMA_NO_EVDO:
                        case TelephonyManager.NETWORK_MODE_EVDO_NO_CDMA:
                            preference.setEntries(
                                    R.array.enabled_networks_cdma_no_lte_choices);
                            preference.setEntryValues(
                                    R.array.enabled_networks_cdma_no_lte_values);
                            break;
                        /// M: Add for supporting 5G. @{
                        case TelephonyManager.NETWORK_MODE_NR_LTE_CDMA_EVDO:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                        /// @}
                        case TelephonyManager.NETWORK_MODE_GLOBAL:
                        case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO:
                        case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_LTE_ONLY:
                            preference.setEntries(
                                    R.array.enabled_networks_cdma_only_lte_choices);
                            preference.setEntryValues(
                                    R.array.enabled_networks_cdma_only_lte_values);
                            break;
                        default:
                            preference.setEntries(
                                    R.array.enabled_networks_cdma_choices);
                            preference.setEntryValues(
                                    R.array.enabled_networks_cdma_values);
                            break;
                    }
                }
            /// M: Add for supporting CT SIM without LTE. @{
            } else if (MobileNetworkUtils.isCtSim(mPhoneId)) {
                preference.setEntries(
                        R.array.enabled_networks_cdma_no_lte_choices);
                preference.setEntryValues(
                        R.array.enabled_networks_cdma_no_lte_values);
            /// @}
            }
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            if (MobileNetworkUtils.isTdscdmaSupported(mContext, mSubId)) {
                Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                        + ", phoneType=" + mPhoneType + ", tdscdma is supported.");
                preference.setEntries(
                        R.array.enabled_networks_tdscdma_choices);
                preference.setEntryValues(
                        R.array.enabled_networks_tdscdma_values);
            } else if (carrierConfig != null
                    && !carrierConfig.getBoolean(CarrierConfigManager.KEY_PREFER_2G_BOOL)
                    && !carrierConfig.getBoolean(CarrierConfigManager.KEY_LTE_ENABLED_BOOL)) {
                Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                        + ", phoneType=" + mPhoneType
                        + ", KEY_PREFER_2G_BOOL and KEY_LTE_ENABLED_BOOL are false.");
                preference.setEntries(R.array.enabled_networks_except_gsm_lte_choices);
                preference.setEntryValues(R.array.enabled_networks_except_gsm_lte_values);
            } else if (carrierConfig != null
                    && !carrierConfig.getBoolean(CarrierConfigManager.KEY_PREFER_2G_BOOL)) {
                int select = mShow4GForLTE
                        ? R.array.enabled_networks_except_gsm_4g_choices
                        : R.array.enabled_networks_except_gsm_choices;
                Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                        + ", phoneType=" + mPhoneType + ", show4GforLTE=" + mShow4GForLTE
                        + ", KEY_PREFER_2G_BOOL is false");
                preference.setEntries(select);
                preference.setEntryValues(
                        R.array.enabled_networks_except_gsm_values);
            } else if (carrierConfig != null
                    && !carrierConfig.getBoolean(CarrierConfigManager.KEY_LTE_ENABLED_BOOL)) {
                Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                        + ", phoneType=" + mPhoneType + ", KEY_LTE_ENABLED_BOOL is false.");
                preference.setEntries(
                        R.array.enabled_networks_except_lte_choices);
                preference.setEntryValues(
                        R.array.enabled_networks_except_lte_values);
            } else if (mIsGlobalCdma) {
                Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                        + ", phoneType=" + mPhoneType + ", isGlobalCdma=" + mIsGlobalCdma);
                preference.setEntries(R.array.enabled_networks_cdma_choices);
                preference.setEntryValues(R.array.enabled_networks_cdma_values);
            } else {
                Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                        + ", phoneType=" + mPhoneType + ", show4GforLTE=" + mShow4GForLTE);
                int select = mShow4GForLTE ? R.array.enabled_networks_4g_choices
                        : R.array.enabled_networks_choices;
                preference.setEntries(select);
                preference.setEntryValues(R.array.enabled_networks_values);
            }
        }
        //TODO(b/117881708): figure out what world mode is, then we can optimize code. Otherwise
        // I prefer to keep this old code
        if (MobileNetworkUtils.isWorldMode(mContext, mSubId)) {
            Log.d(TAG, "updatePreferenceEntries, subId=" + mSubId
                    + ", phoneType=" + mPhoneType + ", worldMode is true.");
            preference.setEntries(
                    R.array.preferred_network_mode_choices_world_mode);
            preference.setEntryValues(
                    R.array.preferred_network_mode_values_world_mode);
        /// M: Add for revising the entry values. @{
        } else {
            revisePreferenceEntryValues(preference);
        /// @}
        }
    }

    private void updatePreferenceValueAndSummary(ListPreference preference, int networkMode) {
        Log.d(TAG, "updatePreferenceValueAndSummary, subId=" + mSubId
                + ", networkMode=" + networkMode);
        preference.setValue(Integer.toString(networkMode));
        switch (networkMode) {
            case TelephonyManager.NETWORK_MODE_TDSCDMA_WCDMA:
            case TelephonyManager.NETWORK_MODE_TDSCDMA_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_TDSCDMA_GSM:
                /*
                 *  M: Add for revising the entry values.
                 *  for NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA for need add C case,
                 *  for NETWORK_MODE_GLOBAL for china telecom case
                 */
                if (preference.findIndexOfValue(Integer.toString(
                        TelephonyManager.NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA)) >= 0) {
                    preference.setValue(Integer.toString(
                            TelephonyManager.NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA));
                } else {
                    preference.setValue(Integer.toString(
                            TelephonyManager.NETWORK_MODE_GLOBAL));
                }
                preference.setSummary(R.string.network_3G);
                /// @}
                break;
            case TelephonyManager.NETWORK_MODE_WCDMA_ONLY:
            case TelephonyManager.NETWORK_MODE_GSM_UMTS:
            case TelephonyManager.NETWORK_MODE_WCDMA_PREF:
                if (!mIsGlobalCdma) {
                    /// M: Add for revising the entry values.
                    preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_GLOBAL));
                    preference.setSummary(R.string.network_3G);
                } else {
                    preference.setValue(Integer.toString(TelephonyManager
                            .NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA));
                    preference.setSummary(R.string.network_global);
                }
                break;
            case TelephonyManager.NETWORK_MODE_GSM_ONLY:
                if (!mIsGlobalCdma) {
                    /// M: Add for revising the entry values.
                    preference.setValue(
                            Integer.toString(MtkRILConstants.NETWORK_MODE_CDMA_GSM));
                    preference.setSummary(R.string.network_2G);
                } else {
                    preference.setValue(
                            Integer.toString(TelephonyManager
                                    .NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA));
                    preference.setSummary(R.string.network_global);
                }
                break;
            case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA:
                if (MobileNetworkUtils.isWorldMode(mContext, mSubId)) {
                    preference.setSummary(
                            R.string.preferred_network_mode_lte_gsm_umts_summary);
                    break;
                }
                /// M: Add for revising the entry values. @{
                if (MobileNetworkUtils.isTdscdmaSupported(mContext, mSubId)
                        && preference.findIndexOfValue(Integer.toString(TelephonyManager
                                .NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA)) >= 0) {
                    preference.setValue(
                            Integer.toString(TelephonyManager
                                    .NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA));
                    preference.setSummary(R.string.network_lte);
                    break;
                }
                /// @}
            case TelephonyManager.NETWORK_MODE_LTE_ONLY:
            case TelephonyManager.NETWORK_MODE_LTE_WCDMA:
                if (!mIsGlobalCdma) {
                    /// M: Add for revising the entry values.
                    preference.setValue(Integer.toString(
                            TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA));
                    preference.setSummary(
                            mShow4GForLTE ? R.string.network_4G : R.string.network_lte);
                } else {
                    preference.setValue(
                            Integer.toString(TelephonyManager
                                    .NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA));
                    preference.setSummary(R.string.network_global);
                }
                break;
            case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO:
                if (MobileNetworkUtils.isWorldMode(mContext, mSubId)) {
                    preference.setSummary(
                            R.string.preferred_network_mode_lte_cdma_summary);
                } else {
                    /// M: Add for revising the entry values. @{
                    if (preference.findIndexOfValue(Integer.toString(
                            TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO)) >= 0) {
                        preference.setValue(Integer.toString(
                                TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO));
                    } else {
                        preference.setValue(Integer.toString(
                                TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA));
                    }
                    CharSequence currEntry = preference.getEntry();
                    if (!TextUtils.isEmpty(currEntry)) {
                        preference.setSummary(currEntry);
                    } else {
                        preference.setSummary(R.string.network_lte);
                    }
                    /// @}
                }
                break;
            case TelephonyManager.NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                /// M: Add for revising the entry values. @{
                if (preference.findIndexOfValue(Integer.toString(TelephonyManager
                        .NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA)) >= 0) {
                    preference.setValue(Integer.toString(TelephonyManager
                            .NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA));
                } else {
                    preference.setValue(Integer.toString(
                            TelephonyManager.NETWORK_MODE_GLOBAL));
                }
                /// @}
                preference.setSummary(R.string.network_3G);
                break;
            case TelephonyManager.NETWORK_MODE_CDMA_EVDO:
            case TelephonyManager.NETWORK_MODE_EVDO_NO_CDMA:
            case TelephonyManager.NETWORK_MODE_GLOBAL:
                /// M: Add for revising the entry values. @{
                if (networkMode == TelephonyManager.NETWORK_MODE_GLOBAL
                        && MobileNetworkUtils.isTdscdmaSupported(mContext, mSubId)
                        && preference.findIndexOfValue(Integer.toString(TelephonyManager
                                .NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA)) >= 0) {
                    preference.setValue(
                            Integer.toString(TelephonyManager
                                    .NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA));
                    preference.setSummary(R.string.network_lte);
                } else {
                    preference.setValue(
                            Integer.toString(TelephonyManager.NETWORK_MODE_GLOBAL));
                }
                /// @}
                preference.setSummary(R.string.network_3G);
                break;
            case TelephonyManager.NETWORK_MODE_CDMA_NO_EVDO:
                preference.setValue(
                        Integer.toString(TelephonyManager.NETWORK_MODE_CDMA_NO_EVDO));
                preference.setSummary(R.string.network_1x);
                break;
            case TelephonyManager.NETWORK_MODE_TDSCDMA_ONLY:
                preference.setValue(
                        Integer.toString(TelephonyManager.NETWORK_MODE_TDSCDMA_ONLY));
                preference.setSummary(R.string.network_3G);
                break;
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
                /// M: Add for checking whether the value exists in entry or not.
                if (MobileNetworkUtils.isTdscdmaSupported(mContext, mSubId)
                        && preference.findIndexOfValue(Integer.toString(TelephonyManager
                                .NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA)) >= 0) {
                    preference.setValue(
                            Integer.toString(TelephonyManager
                                    .NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA));
                    preference.setSummary(R.string.network_lte);
                } else {
                    preference.setValue(
                            Integer.toString(TelephonyManager
                                    .NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA));
                    /// M: Add for log.
                    final boolean isWorldMode =
                            MobileNetworkUtils.isWorldMode(mContext, mSubId);
                    /// M: Add for supporting CTVoLTE.
                    if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA
                            || mIsGlobalCdma || isWorldMode || mShowCdmaOption) {
                        preference.setSummary(R.string.network_global);
                    } else {
                        preference.setSummary(mShow4GForLTE
                                ? R.string.network_4G : R.string.network_lte);
                    }
                }
                break;
            /// M: Add for supporting 5G. @{
            case TelephonyManager.NETWORK_MODE_NR_LTE_CDMA_EVDO:
            case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                preference.setValue(Integer.toString(
                        TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA));
                preference.setSummary(m5gDisplayName);
                break;
            /// @}
            /// M: Add for revising the entry values. @{
            case MtkRILConstants.NETWORK_MODE_CDMA_GSM:
                preference.setValue(Integer.toString(
                        MtkRILConstants.NETWORK_MODE_CDMA_GSM));
                preference.setSummary(R.string.network_2G);
                break;
            /// @}
            default:
                preference.setSummary(
                        mContext.getString(R.string.mobile_network_mode_error, networkMode));
        }
    }

    /// M: Add for revising the entry values. @{
    private void revisePreferenceEntryValues(ListPreference preference) {
        CharSequence[] values = preference.getEntryValues();
        int valueNum = values.length;
        String[] newValues = new String[valueNum];
        for (int i = 0; i < valueNum; i++) {
            Integer valueInt = Integer.valueOf(values[i].toString());
            switch (valueInt.intValue()) {
                case TelephonyManager.NETWORK_MODE_WCDMA_PREF:
                case TelephonyManager.NETWORK_MODE_CDMA_EVDO:
                    newValues[i] = String.valueOf(TelephonyManager.NETWORK_MODE_GLOBAL);
                    break;

                case TelephonyManager.NETWORK_MODE_GSM_ONLY:
                    newValues[i] = String.valueOf(
                            MtkRILConstants.NETWORK_MODE_CDMA_GSM);
                    break;

                case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA:
                    newValues[i] = String.valueOf(
                            TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA);
                    break;

                case TelephonyManager.NETWORK_MODE_TDSCDMA_GSM_WCDMA:
                    newValues[i] = String.valueOf(
                            TelephonyManager.NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA);
                    break;

                default:
                    newValues[i] = values[i].toString();
                    break;
            }
        }
        preference.setEntryValues(newValues);
    }
    /// @}

    /// M: Add for checking the capability. @{
    private void updatePreferenceForCapability(ListPreference preference) {
        String rat = TelephonyUtils.getRadioRatPropertyValue(mPhoneId);
        boolean is4gSupported = TelephonyUtils.is4gSupportedInRat(rat);
        boolean is3gSupported = TelephonyUtils.is3gSupportedInRat(rat);
        boolean is5gDevice = TelephonyUtils.is5gSupportedByDevice();
        boolean is5gSupported = false;

        if (is4gSupported) {
            is5gSupported = TelephonyUtils.is5gSupportedInRat(mPhoneId, rat);
        }

        if (!is5gDevice && is4gSupported && is3gSupported) {
            return;
        }

        if (!is4gSupported && !is3gSupported) {
            preference.setSummary(R.string.network_2G);
            preference.setEnabled(false);
            return;
        }

        String strGlobal = mContext.getString(R.string.network_global);
        String strLte = mContext.getString(R.string.network_lte);
        String str4g = mContext.getString(R.string.network_4G);
        String str3g = mContext.getString(R.string.network_3G);

        CharSequence[] entries = preference.getEntries();
        CharSequence[] values = preference.getEntryValues();

        int[] validValuesInt = new int[values.length];
        int validCount = 0;

        for (int i = 0; i < values.length; i++) {
            String entryString = entries[i].toString();
            if ((!is4gSupported && (strLte.equals(entryString)
                    || str4g.equals(entryString)))
                    || (!is3gSupported && str3g.equals(entryString))) {
                validValuesInt[i] = -1;
            } else {
                validValuesInt[i] = Integer.parseInt(values[i].toString());
                validCount++;
            }
        }

        if (validCount == 0) {
            preference.setEnabled(false);
            Log.d(TAG, "updatePreferenceForCapability, no valid type for subId=" + mSubId);
            return;
        }

        if (is5gSupported) {
            validCount++;
        }

        CharSequence[] newEntries = new CharSequence[validCount];
        CharSequence[] newValues = new CharSequence[validCount];
        int count = 0;

        if (is5gSupported) {
            newEntries[0] = m5gDisplayName;
            newValues[0] = String.valueOf(TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA);
            count++;
        }

        for (int i = 0; i < validValuesInt.length; i++) {
            if (validValuesInt[i] != -1) {
                newEntries[count] = entries[i];
                newValues[count] = values[i];
                count++;
            }
        }

        String valueString = preference.getValue();
        boolean selectedValueExist = false;

        for (int i = 0; i < count; i++) {
            if (valueString.equals(newValues[i].toString())) {
                selectedValueExist = true;
                break;
            }
        }

        preference.setEntries(newEntries);
        preference.setEntryValues(newValues);

        if (!selectedValueExist) {
            preference.setValue(newValues[0].toString());
            preference.setSummary(newEntries[0]);
        }
    }
    /// @}

    /// M: Add for handling phone type change. @{
    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreference = screen.findPreference(getPreferenceKey());
    }

    private Preference mPreference = null;
    private boolean mEnableState = true;

    public void setEnableState(boolean enable) {
        Log.d(TAG, "setEnableState, enable=" + enable);
        mEnableState = enable;
        if (mPreference != null) {
            updateState(mPreference);
        }
    }
    /// @}

    /// M: Add for setting the preferred network type async. @{
    private static final int EVENT_SET_PREFERRED_NETWORK_TYPE_DONE = 1;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SET_PREFERRED_NETWORK_TYPE_DONE:
                    Log.e(TAG, "handleMessage, set preferred network type done.");
                    if (mPreference != null) {
                        updateState(mPreference);
                    }
                    break;

                default:
                    break;
            }
        }
    };

    private final class PreferredNetworkTypeSyncTask implements Runnable {
        private TelephonyManager mTelephonyManager;
        private int mSubId;
        private int mNetworkType;

        PreferredNetworkTypeSyncTask(TelephonyManager telephonyManager,
            int subId, int networkType) {
            mTelephonyManager = telephonyManager;
            mSubId = subId;
            mNetworkType = networkType;
        }

        @Override
        public void run() {
            if (mTelephonyManager.setPreferredNetworkType(mSubId, mNetworkType)) {
                Settings.Global.putInt(mContext.getContentResolver(),
                        Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                        mNetworkType);
                Log.e(TAG, "PreferredNetworkTypeSyncTask.run, set preferred network type success"
                        + ", subId=" + mSubId + ", networkType=" + mNetworkType);
            } else {
                Log.e(TAG, "PreferredNetworkTypeSyncTask.run, set preferred network type failed"
                        + ", subId=" + mSubId + ", networkType=" + mNetworkType);
            }
            Message msg = mHandler.obtainMessage(EVENT_SET_PREFERRED_NETWORK_TYPE_DONE);
            msg.sendToTarget();
        }
    }
    /// @}
}
