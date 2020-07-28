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

package com.android.settings.network.telephony.gsm;

import java.util.ArrayList;

import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import androidx.preference.Preference;
import androidx.preference.ListPreference;
import androidx.preference.PreferenceScreen;


import com.android.settings.R;
import com.android.settings.network.telephony.TelephonyBasePreferenceController;

import com.mediatek.settings.UtilsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Preference controller for "Open network select"
 */
public class CustomizeVersionFemtoCellSelectionPreferenceController
        extends TelephonyBasePreferenceController
        implements ListPreference.OnPreferenceChangeListener {

    private static final String TAG = "CustomizeVersionFemtoCellSelectionPreferenceController";

    private TelephonyManager mTelephonyManager;
    private ListPreference mPreference;
    private static final String LOG_TAG = "CustomizeVersionFemtoCellSelectionPreferenceController";

    private int mPhoneId;
    private MtkTelephonyManagerEx mTelephonyManagerEx;
    int mMode;

    public CustomizeVersionFemtoCellSelectionPreferenceController(Context context, String key) {
        super(context, key);
    }

    public void init(int subId) {
        mSubId = subId;
        mPhoneId = SubscriptionManager.getSlotIndex(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        int visible = getAvailabilityStatus(subId);
        Log.i(TAG, "init visible = "+ visible);
        if (visible != AVAILABLE) {
            return;
        }
        new AsyncTask<Void, Void, Integer>() {
            @Override
            protected Integer doInBackground(Void... voids) {
                Log.d(TAG, "doInBackground queryFemtoCellSystemSelectionMode phoneId = " + mPhoneId);
                int mode = mTelephonyManagerEx.queryFemtoCellSystemSelectionMode(mPhoneId);
                return mode;
            }

            @Override
            protected void onPostExecute(Integer mode) {
                Log.d(TAG, "onPostExecute queryFemtoCellSystemSelectionMode onPostExecute = " + mode);
                mMode = mode;
                updateState(mPreference);
            }
        }.execute();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object object) {
        final int mode = Integer.parseInt((String) object);
        Log.d(TAG, "Update the Femto Cell Preference");
        mTelephonyManagerEx.setFemtoCellSystemSelectionMode(mPhoneId, mode);
        new AsyncTask<Void, Void, Boolean>() {
            @Override
            protected Boolean doInBackground(Void... voids) {
                Log.d(TAG, "doInBackground setFemtoCellSystemSelectionMode phoneId = " + mPhoneId);
                boolean isSucess = mTelephonyManagerEx.setFemtoCellSystemSelectionMode(mPhoneId, mode);
                return isSucess;
            }

            @Override
            protected void onPostExecute(Boolean result) {
                Log.d(TAG, "onPostExecute setFemtoCellSystemSelectionMode" +
                        " onPostExecute = " + result);
            }
        }.execute();
        return true;
    }

    @Override
    public int getAvailabilityStatus(int subId) {

        boolean visible = false;
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Log.d(TAG, "getAvailabilityStatus, subId is invalid.");
            visible = false;
        } else {
            visible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                    .customizeVersionFemtoCellSelectionPreference(subId);
        }
        return visible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreference = screen.findPreference(getPreferenceKey());

    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        String prefValue = Integer.toString(mMode);
        mPreference.setValue(prefValue);
    }

}