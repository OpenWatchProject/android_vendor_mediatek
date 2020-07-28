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

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.AsyncTask;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.annotation.VisibleForTesting;
import androidx.preference.ListPreference;
import androidx.preference.Preference;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.settings.R;

import com.mediatek.telephony.MtkTelephonyManagerEx;

import com.mediatek.settings.UtilsExt;
/// M: Add for checking CT SIM.

/**
 * Preference controller for "Enabled network mode"
 */
public class CustomizeTMONetworkModePreferenceController extends
        TelephonyBasePreferenceController implements
        ListPreference.OnPreferenceChangeListener {

    private static final String TAG = "CustomizeTMONetworkModePreferenceController";

    private CarrierConfigManager mCarrierConfigManager;
    private TelephonyManager mTelephonyManager;
    private int mNetworkMode;
    private boolean mIsDisable2G = false;
    private AlertDialog mDialog;
    private MtkTelephonyManagerEx mTelephonyManagerEx;
    private int mPhoneId;
    private ListPreference mListPreference;
    public CustomizeTMONetworkModePreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = context.getSystemService(CarrierConfigManager.class);
    }
    @Override
    public int getAvailabilityStatus(int subId) {
        boolean visible;
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Log.d(TAG, "getAvailabilityStatus, subId is invalid.");
            visible = false;
        } else {
            visible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                    .customizeTMONetworkModePreference(subId);
        }
        return visible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        mListPreference = (ListPreference) preference;
        mNetworkMode = getPreferredNetworkMode();
        updatePreferenceValueAndSummary(mListPreference, mNetworkMode);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object object) {
        final int settingsMode = Integer.parseInt((String) object);
        if (TelephonyManager.NETWORK_MODE_LTE_WCDMA == settingsMode) {
            if (!isSettingOn()) {
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                Log.d(TAG, "isNetworkUpdateNeeded alert");
                builder.setTitle(mContext.getString(R.string.dialog_disable_2g_Alert_title));
                builder.setMessage(mContext.getString(R.string.dialog_disable_2g_summary));
                builder.setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        Log.d(TAG, "onCancel");
                        updatePreferenceValueAndSummary((ListPreference) preference, mNetworkMode);
                    }
                });
                builder.setPositiveButton(mContext.getString(
                        R.string.dialog_disable_2g_alert_ok),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Log.i(TAG, "setPositiveButton, click disable 2g start");
                                mIsDisable2G = true;
                                handleSwitchAction(true);
                                preference.setEnabled(false);
                                // Revise the network mode to NETWORK_MODE_LTE_GSM_WCDMA.
                                int networkMode = TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA;
                                updatePreferenceValueAndSummary((ListPreference) preference, networkMode);
                                if (mTelephonyManager.setPreferredNetworkType(mSubId, networkMode)) {
                                    Settings.Global.putInt(mContext.getContentResolver(),
                                            Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                                            networkMode);
                                    updatePreferenceValueAndSummary((ListPreference) preference, networkMode);
                                }
                            }
                        });
                builder.setNegativeButton(mContext.getString(
                    R.string.dialog_disable_2g_alert_cancel),
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            Log.d(TAG, "For no don't change mode");
                            updatePreferenceValueAndSummary((ListPreference) preference, mNetworkMode);
                        }
                    });
                mDialog = builder.show();
            }
            return true;
        } else {
            if (isSettingOn()) {
                mListPreference.setEnabled(false);
                mIsDisable2G = false;
                handleSwitchAction(false);
            }
            if (mTelephonyManager.setPreferredNetworkType(mSubId, settingsMode)) {
                Settings.Global.putInt(mContext.getContentResolver(),
                        Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                        settingsMode);
                updatePreferenceValueAndSummary((ListPreference) preference, settingsMode);
                return true;
            }
        }
        return false;
    }

    public void init(int subId) {

        mSubId = subId;
        mPhoneId = SubscriptionManager.getSlotIndex(subId);
        int visible = getAvailabilityStatus(subId);
        Log.i(TAG, "init visible = "+ visible);
        if (visible != AVAILABLE) {
            return;
        }
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(mSubId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        ///need queryDisable2G or not
        mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        Log.i(TAG, "init getDisable2G feature start mSubId = " + mSubId + " mPhoneId = " + mPhoneId);
        new AsyncTask<Void, Void, Integer>() {
            @Override
            protected Integer doInBackground(Void... voids) {
                Log.d(TAG, "doInBackground  phoneId = " + mPhoneId);
                int mode = mTelephonyManagerEx.getDisable2G(mPhoneId);
                return mode;
            }

            @Override
            protected void onPostExecute(Integer result) {
                Log.d(TAG, "onPostExecute getDisable2G onPostExecute = " + result);
                if (result == 0) {
                    mIsDisable2G = false;
                } else if (result == 1) {
                    mIsDisable2G = true;
                }
                if (null != mListPreference) {
                    Log.i(TAG, "handleget2gMode done, set list can click and update summary");
                    mListPreference.setEnabled(true);
                    mNetworkMode = getPreferredNetworkMode();
                    updatePreferenceValueAndSummary(mListPreference, mNetworkMode);
                }
            }
        }.execute();
    }

    private int getPreferredNetworkMode() {
        int mode = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                Phone.PREFERRED_NT_MODE);
        Log.i(TAG, "getPreferredNetworkMode mode = " + mode);
        return mode;
    }

    private void updatePreferenceValueAndSummary(ListPreference preference, int networkMode) {
        Log.d(TAG, "updatePreferenceValueAndSummary, subId=" + mSubId
                + ", networkMode=" + networkMode);
        mNetworkMode = networkMode;
        switch (networkMode) {
            case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_ONLY:
            case TelephonyManager.NETWORK_MODE_LTE_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
                if (!isSettingOn()) {
                    preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA));
                    preference.setSummary(mContext.getText(R.string.lte_on));
                } else {
                    preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_WCDMA));
                    preference.setSummary(mContext.getText(R.string.lte_3g));
                }
                break;
            case TelephonyManager.NETWORK_MODE_WCDMA_PREF:
            case TelephonyManager.NETWORK_MODE_WCDMA_ONLY:
            case TelephonyManager.NETWORK_MODE_GSM_UMTS:
            case TelephonyManager.NETWORK_MODE_CDMA_NO_EVDO:
            case TelephonyManager.NETWORK_MODE_CDMA_EVDO:
            case TelephonyManager.NETWORK_MODE_EVDO_NO_CDMA:
            case TelephonyManager.NETWORK_MODE_GLOBAL:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_WCDMA_PREF));
                preference.setSummary(mContext.getText(R.string.lte_off));
                break;
            case TelephonyManager.NETWORK_MODE_GSM_ONLY:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_GSM_ONLY));
                preference.setSummary(mContext.getText(R.string.only_2g));
                break;
            default:
                preference.setSummary(
                    mContext.getString(R.string.mobile_network_mode_error, networkMode));
            }
    }

    private boolean isSettingOn() {
        return mIsDisable2G;
    }

    private void handleSwitchAction(final boolean switchValue) {
        new AsyncTask<Void, Void, Boolean>() {
            @Override
            protected Boolean doInBackground(Void... voids) {
                Log.d(TAG, "doInBackground setDisable2G phoneId = " + mPhoneId);
                boolean isSucess = mTelephonyManagerEx.setDisable2G(mPhoneId, switchValue);
                return isSucess;
            }

            @Override
            protected void onPostExecute(Boolean result) {
                Log.d(TAG, "onPostExecute setDisable2G onPostExecute = " + result);
                if (!result) {
                    mIsDisable2G = (mIsDisable2G ? false : true);
                } else {
                    Log.d(TAG, "handleSet2gMode no exception");
                }
                if (null != mListPreference) {
                    Log.i(TAG, "handleSet2gMode done, set list can click and update summary");
                    mListPreference.setEnabled(true);
                    mNetworkMode = getPreferredNetworkMode();
                    updatePreferenceValueAndSummary(mListPreference, mNetworkMode);
                }
            }
        }.execute();
    }
}
