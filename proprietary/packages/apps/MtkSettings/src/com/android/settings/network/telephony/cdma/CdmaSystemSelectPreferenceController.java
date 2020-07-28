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

package com.android.settings.network.telephony.cdma;

import android.content.Context;
/// M: Getting/Setting CDMA roaming mode may be blocked by other network operation.
import android.os.AsyncTask;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.preference.ListPreference;
import androidx.preference.Preference;

import com.android.internal.telephony.Phone;
/// M: Add for checking the phone/SIM states.
import com.android.settings.network.telephony.MobileNetworkUtils;

/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * Preference controller for "System Select"
 */
public class CdmaSystemSelectPreferenceController extends CdmaBasePreferenceController
        implements ListPreference.OnPreferenceChangeListener {

    private static final String TAG = "CdmaSystemSelectPreferenceController";

    /// M: Getting/Setting CDMA roaming mode may be blocked by other network operation. @{
    private int mCdmaRoamingMode;
    private boolean mIsModeGotten = false;
    private boolean mIsGettingMode = false;
    private boolean mIsSettingMode = false;
    private int mNewMode;
    private AsyncTask mTask = null;
    /// @}

    public CdmaSystemSelectPreferenceController(Context context, String key) {
        super(context, key);
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        final ListPreference listPreference = (ListPreference) preference;
        /// M: No need to get CDMA roaming mode when preference is not available. @{
        int availableStatus = getAvailabilityStatus();
        listPreference.setVisible(availableStatus == AVAILABLE);
        if (availableStatus != AVAILABLE) {
            return;
        }
        /// @}

        /// M: Getting CDMA roaming mode may be blocked by other network operation. @{
        if (mIsGettingMode || mIsSettingMode) {
            Log.d(TAG, "updateState, phoneId=" + mPhoneId
                    + ", isGettingMode=" + mIsGettingMode
                    + ", isSettingMode=" + mIsSettingMode);
            return;
        }

        if (mIsModeGotten) {
            updateModeState(listPreference, mCdmaRoamingMode);
            return;
        }

        listPreference.setEnabled(false);
        mIsGettingMode = true;

        mTask = new AsyncTask<Void, Void, Integer>() {
            @Override
            protected Integer doInBackground(Void... voids) {
                int mode = TelephonyManager.CDMA_ROAMING_MODE_RADIO_DEFAULT;
                if (mTelephonyManager != null && mPreference != null) {
                    Log.d(TAG, "updateState, doInBackground, phoneId=" + mPhoneId);
                    mode = mTelephonyManager.getCdmaRoamingMode();
                } else {
                    Log.d(TAG, "updateState, doInBackground, phoneId=" + mPhoneId
                            + ", tm=" + (mTelephonyManager == null ? "null" : mTelephonyManager)
                            + ", preference=" + (mPreference == null ? "null" : mPreference));
                }
                return new Integer(mode);
            }

            @Override
            protected void onPostExecute(Integer cdmaRoamingMode) {
                if (mTelephonyManager == null || mPreference == null) {
                    Log.d(TAG, "updateState, onPostExecute, phoneId=" + mPhoneId
                            + ", tm=" + (mTelephonyManager == null ? "null" : mTelephonyManager)
                            + ", preference=" + (mPreference == null ? "null" : mPreference));
                } else {
                    Log.d(TAG, "updateState, onPostExecute, phoneId=" + mPhoneId
                            + ", cdmaRoamingMode=" + cdmaRoamingMode);

                    updateModeState((ListPreference) mPreference, cdmaRoamingMode.intValue());
                    mIsModeGotten = true;
                }
                mIsGettingMode = false;
                mTask = null;
            }
        }.execute();
        /// @}
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object object) {
        int newMode = Integer.parseInt((String) object);

        /// M: Setting CDMA roaming mode may be blocked by other network operation. @{
        if (mIsSettingMode) {
            Log.d(TAG, "onPreferenceChange, still setting mode.");
            return false;
        }

        final ListPreference listPreference = (ListPreference) preference;
        listPreference.setEnabled(false);
        mIsSettingMode = true;
        mNewMode = newMode;

        mTask = new AsyncTask<Void, Void, Boolean>() {
            @Override
            protected Boolean doInBackground(Void... voids) {
                boolean result = false;
                if (mTelephonyManager != null && mPreference != null) {
                    Log.d(TAG, "onPreferenceChange, doInBackground, phoneId=" + mPhoneId
                            + ", mode=" + mNewMode);
                    result = mTelephonyManager.setCdmaRoamingMode(mNewMode);
                } else {
                    Log.d(TAG, "onPreferenceChange, doInBackground, phoneId=" + mPhoneId
                            + ", tm=" + (mTelephonyManager == null ? "null" : mTelephonyManager)
                            + ", preference=" + (mPreference == null ? "null" : mPreference));
                }
                return new Boolean(result);
            }

            @Override
            protected void onPostExecute(Boolean result) {
                if (mTelephonyManager == null || mPreference == null) {
                    Log.d(TAG, "onPreferenceChange, onPostExecute, phoneId=" + mPhoneId
                            + ", tm=" + (mTelephonyManager == null ? "null" : mTelephonyManager)
                            + ", preference=" + (mPreference == null ? "null" : mPreference));
                } else {
                    Log.d(TAG, "onPreferenceChange, onPostExecute, phoneId=" + mPhoneId
                            + ", result=" + result);

                    if (result.booleanValue()) {
                        Settings.Global.putInt(mContext.getContentResolver(),
                                Settings.Global.CDMA_ROAMING_MODE, mNewMode);
                        mCdmaRoamingMode = mNewMode;
                    }

                    updateModeState((ListPreference) mPreference, mCdmaRoamingMode);
                }
                mIsSettingMode = false;
                mTask = null;
            }
        }.execute();
        /// @}

        return false;
    }

    private void resetCdmaRoamingModeToDefault() {
        final ListPreference listPreference = (ListPreference) mPreference;
        //set the mButtonCdmaRoam
        listPreference.setValue(Integer.toString(TelephonyManager.CDMA_ROAMING_MODE_ANY));
        //set the Settings.System
        Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.CDMA_ROAMING_MODE,
                TelephonyManager.CDMA_ROAMING_MODE_ANY);
        //Set the Status
        mTelephonyManager.setCdmaRoamingMode(TelephonyManager.CDMA_ROAMING_MODE_ANY);
    }

    /// M: Getting/Setting CDMA roaming mode may be blocked by other network operation. @{
    @Override
    public void init(int subId) {
        super.init(subId);
        if (mTask != null) {
            mTask.cancel(true);
            mTask = null;
        }
        mIsModeGotten = false;
        mIsGettingMode = false;
        mIsSettingMode = false;
    }

    private void updateModeState(ListPreference listPreference, int mode) {
        if (mode != TelephonyManager.CDMA_ROAMING_MODE_RADIO_DEFAULT) {
            if (mode == TelephonyManager.CDMA_ROAMING_MODE_HOME
                    || mode == TelephonyManager.CDMA_ROAMING_MODE_ANY) {
                listPreference.setValue(Integer.toString(mode));
            } else {
                resetCdmaRoamingModeToDefault();
                mode = TelephonyManager.CDMA_ROAMING_MODE_ANY;
            }
        }
        mCdmaRoamingMode = mode;

        final int settingsNetworkMode = Settings.Global.getInt(
                mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                Phone.PREFERRED_NT_MODE);
        listPreference.setEnabled(
                settingsNetworkMode != TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA
                && MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId));
    }
    /// @}
}
