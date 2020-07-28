/*
 * Copyright (C) 2011-2014 MediaTek Inc.
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

package com.mediatek.dialer.calllog.accountfilter;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import com.android.internal.telephony.TelephonyIntents;

/** M: [Call Log Account Filter] support CALL_LOG_ACCOUNT_FILTER @{ */
public class PhoneAccountInfoHelper {
    public static final String TAG = "PhoneAccountInfoHelper";
    /**
     * Key for get and save the perfer phone account.
     */
    public static final String PRE_KEY = "account_id";
    private static final String PREF_NAME = "account_select";
    private Context mContext;
    private static SharedPreferences mPref;
    private volatile static PhoneAccountInfoHelper mPhoneAccountInfoHelper;
    /**
     * For Call Log Filter, means display call log from all accounts
     */
    public static final String FILTER_ALL_ACCOUNT_ID = "all_account";

    private PhoneAccountInfoHelper(Context context) {
        Log.d(TAG, "PhoneAccountInfoHelper mPref:" + mPref);
        mContext = context.getApplicationContext();
        mPref = mContext.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
        updatePreferAccountInfo(mContext);
    }

    public static PhoneAccountInfoHelper getInstance(Context context) {
        if (mPhoneAccountInfoHelper == null) {
            synchronized(PhoneAccountInfoHelper.class) {
                if (mPhoneAccountInfoHelper == null) {
                    mPhoneAccountInfoHelper = new PhoneAccountInfoHelper(context);
                }
            }
        }
        return mPhoneAccountInfoHelper;
    }

    public void registerSimChangeReceiver() {
        Log.d(TAG, "registerAccountReceiverIfNeeded()");
        try {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter
                    .addAction(TelephonyIntents.ACTION_DEFAULT_VOICE_SUBSCRIPTION_CHANGED);
            intentFilter
                    .addAction(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE);
            intentFilter
                    .addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);

            mContext.registerReceiver(mReceiver, intentFilter);
        } catch (RuntimeException e) {
            Log.d(TAG, "registerAccountReceiverIfNeeded()-->e = " + e);
        }
    }

    public void unregisterSimChangeReceiver() {
        mContext.unregisterReceiver(mReceiver);
    }

    public void registerSharedPreferenceChangeListener(
            SharedPreferences.OnSharedPreferenceChangeListener listener) {
        mPref.registerOnSharedPreferenceChangeListener(listener);

    }

    public void unRegisterSharedPreferenceChangeListener(
            SharedPreferences.OnSharedPreferenceChangeListener listener) {
        mPref.unregisterOnSharedPreferenceChangeListener(listener);
    }

    /**
     * receiver to listen the account info changed.
     */
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TelephonyIntents.ACTION_DEFAULT_VOICE_SUBSCRIPTION_CHANGED
                    .equals(action)
                    || TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE
                            .equals(action)
                    || TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED
                            .equals(action)) {
                Log.d(TAG, "BroadcastReceiver()-->action = " + action);
                if (updatePreferAccountInfo(mContext)) {
                }
            }
        }
    };

    /**
     * Update prefer account info according phone according.
     * 
     * @param context context
     */
    public boolean updatePreferAccountInfo(Context context) {
        Log.d(TAG, "updatePreferAccountInfo");
        String preferAccountId = getPreferAccountId();
        if (FILTER_ALL_ACCOUNT_ID.equals(preferAccountId)) {
            Log.d(TAG,
                    "ifNeedNoifyAccountChange() user choose to show all call log, "
                            + "no need care phone account info.");
            return false;
        }

        boolean preferAccountRemoved = true;
        TelecomManager telecomManager = (TelecomManager) context
                .getSystemService(Context.TELECOM_SERVICE);
        List<PhoneAccountHandle> handles = telecomManager
                .getCallCapablePhoneAccounts();
        for (PhoneAccountHandle handle : handles) {
            if (handle.getId().equals(preferAccountId)) {
                preferAccountRemoved = false;
                break;
            }
        }
        // if the selected account is removed or only one account left, show all
        // accounts
        if (preferAccountRemoved
                || telecomManager.getCallCapablePhoneAccounts().size() <= 1) {
            setPreferAccountId(FILTER_ALL_ACCOUNT_ID);
            return true;
        }
        return false;
    }

    /**
     * Get the filter value for Call Logs filter.
     * 
     * @param context context
     * @return {@link FILTER_ALL_ACCOUNT_ID}, PhoneAccount Id
     */
    public static String getPreferAccountId() {
        if (mPref == null) {
            return FILTER_ALL_ACCOUNT_ID;
        }
        String accountPrefered = mPref.getString(PRE_KEY, FILTER_ALL_ACCOUNT_ID);
        Log.d(TAG, "getPreferAccountId(), " + accountPrefered);
        return accountPrefered;
    }

    /**
     * Save the user selected value for Call Logs filter.
     * 
     * @param context context
     * @param id {@link FILTER_ALL_ACCOUNT_ID}, PhoneAccount Id
     */
    public void setPreferAccountId(String id) {
        Log.d(TAG, "setPreferAccountId()- id = " + id);
        Log.d(TAG, "setPreferAccountId() NA mPref = " + mPref);
        SharedPreferences.Editor editor = mPref.edit();
        editor.putString(PRE_KEY, id);
        editor.apply();
        Log.d(TAG, "setPreferAccountId() NA, " + id);
        return;
    }
}
