/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.mediatek.op18.systemui;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.provider.Settings;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import com.mediatek.provider.MtkSettingsExt;
import java.util.Iterator;

/** Implementation of the SimSelectionView controller. **/
public class SimSelectionController {
    private static final String TAG = "OP18SimSelectionController";

    private final Context mContext;
    private final Handler mHandler;
    private final SimSettingObserver mSimSettingObserver;
    private static SubscriptionManager sSubscriptionManager;
    private static TelecomManager sTelecomManager;
    private static TelephonyManager sTelephonyManager;
    private boolean mListening;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "mReceiver action = " + action);
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                // Do not update, if simSelection View group is not ready
                if (SimSelectionView.getSimSelectionViewGroup() != null) {
                    // Since View group is present, SimSelectionView instance is already made
                    // So, params passed in getInstance will be ignored
                    SimSelectionView simSelectView = SimSelectionView.getInstance(null, null);
                    simSelectView.updateSimSelectionView();
                }
            }
        }
    };

    /** ContentObserver to watch brightness. **/
    private class SimSettingObserver extends ContentObserver {
        private final Uri mSimSettingModeUri =
                Settings.Global.getUriFor(MtkSettingsExt.Global.CURRENT_NETWORK_CALL);

        public SimSettingObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, mSimSettingModeUri);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            int enable = Settings.Global.getInt(mContext.getContentResolver(),
                    MtkSettingsExt.Global.CURRENT_NETWORK_CALL, 0);
            Log.d(TAG, "sim setting changed, enabled: " + enable);
            if (showSimSelectionDialog(mContext)) {
                // Set sim account as ims capable account
                setMainCapabilityAccount();
            }
            // Update SIM selection view
            if (SimSelectionView.getSimSelectionViewGroup() != null) {
                // Since View group is present, SimSelectionView instance is already made
                // So, params passed in getInstance will be ignored
                SimSelectionView simSelectView = SimSelectionView.getInstance(null, null);
                simSelectView.updateSimSelectionView();
            }
        }

        public void startObserving() {
            final ContentResolver cr = mContext.getContentResolver();
            cr.registerContentObserver(mSimSettingModeUri, false, this);
        }

        public void stopObserving() {
            final ContentResolver cr = mContext.getContentResolver();
            cr.unregisterContentObserver(this);
        }

    }

    /**
     * Constructor.
     * @param context context
     */
    public SimSelectionController(Context context) {
        Log.d(TAG, "in constructor");
        mContext = context;
        sSubscriptionManager = SubscriptionManager.from(context);
        sTelecomManager = TelecomManager.from(context);
        sTelephonyManager = TelephonyManager.from(context);
        mHandler = new Handler();
        mSimSettingObserver = new SimSettingObserver(mHandler);
    }

    /**
     * Registers SimSelectionView callbacks.
     * @return
     */
    public void registerCallbacks() {
        if (mListening) {
            return;
        }
        IntentFilter intentFilter = new IntentFilter(TelephonyIntents
                .ACTION_SUBINFO_RECORD_UPDATED);
        mContext.registerReceiver(mReceiver, intentFilter);
        mSimSettingObserver.startObserving();
        mListening = true;
    }

    /**
     * Unregisters SimSelectionView callbacks.
     * @return
     */
    public void unregisterCallbacks() {
        if (!mListening) {
            return;
        }
        mContext.unregisterReceiver(mReceiver);
        mSimSettingObserver.stopObserving();
        mListening = false;
    }

    /**
     * Decides whether to show SimSelectionView or not.
     * @param context context
     * @return true/false
     */
    public static boolean showSimSelectionDialog(Context context) {
        int enable = Settings.Global.getInt(context.getContentResolver(),
                    MtkSettingsExt.Global.CURRENT_NETWORK_CALL, 0);
        /* Should not use sim count to check number of SIMs, as SIM count gets present before
         * updation of sub info. While we have to show Sim selection Settings,
         * only when sub info of both sims are present. */
        SubscriptionInfo sir0 = sSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(0);
        SubscriptionInfo sir1 = sSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(1);
        if (enable == 1 && sir0 != null && sir1 != null) {
            return true;
       } else {
            return false;
       }
    }

    /**
         * Sets phone account of selected subId.
         * @param subId subId
         * @return
         */
    public void setPhoneAccount(int subId) {
        PhoneAccountHandle phoneAccountHandle = subscriptionIdToPhoneAccountHandle(subId);
        sTelecomManager.setUserSelectedOutgoingPhoneAccount(phoneAccountHandle);
        Log.d(TAG, "account set:" + phoneAccountHandle);
    }

    /**
         * Sets Main capability phone account.
         * @return subId
         */
    public int setMainCapabilityAccount() {
        int masterPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(masterPhoneId);
        Log.d(TAG, "main capability account phoneId:" + masterPhoneId);
        Log.d(TAG, "main capability account subId:" + subId);
        setPhoneAccount(subId);
        return subId;
    }

    private PhoneAccountHandle subscriptionIdToPhoneAccountHandle(int subId) {
        final Iterator<PhoneAccountHandle> phoneAccounts =
                sTelecomManager.getCallCapablePhoneAccounts().listIterator();

        while (phoneAccounts.hasNext()) {
            final PhoneAccountHandle phoneAccountHandle = phoneAccounts.next();
            final PhoneAccount phoneAccount = sTelecomManager.getPhoneAccount(phoneAccountHandle);
            if (subId == sTelephonyManager.getSubIdForPhoneAccount(phoneAccount)) {
                return phoneAccountHandle;
            }
        }
        return null;
    }
}
