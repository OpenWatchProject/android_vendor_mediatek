/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.telephony;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.AccessNetworkConstants;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.DctConstants;
import com.android.internal.telephony.SettingsObserver;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;

/**
 * Sync data settings(mobile data switch, roaming data switch, default data slot)
 * to modem.
 */
public class DataStateController extends BaseController {
    private static final String TAG = "DataStateController";

    private static final int BASE = 10;
    private static final int EVENT_DATA_SETTING_CHANGE = BASE * 1;
    private static final int EVENT_ROAMING_SETTING_CHANGE = BASE * 2;
    private static final int EVENT_DOMESTIC_ROAMING_SETTING_CHANGE = BASE * 3;
    private static final int EVENT_INTERNATIONAL_ROAMING_SETTING_CHANGE = BASE * 4;

    private static final String DOMESTIC_DATA_ROAMING = "domestic_data_roaming";
    private static final String INTERNATIONAL_DATA_ROAMING = "international_data_roaming";
    private static final int SKIP_VALUE = -2;

    private SettingsObserver[] mSettingsObserver;
    private int[] mPhoneSubscriptions;
    private boolean[] mRadioState;

    // Listen sub change event and sync all data setting to modem.
    protected final OnSubscriptionsChangedListener mSubscriptionsChangedListener =
            new OnSubscriptionsChangedListener() {
        /**
         * Callback invoked when there is any change to any SubscriptionInfo. Typically
         * this method invokes {@link SubscriptionManager#getActiveSubscriptionInfoList}
         */
        @Override
        public void onSubscriptionsChanged() {
            // Set the network type, in case the radio does not restore it.
            for (int i = 0; i < mPhones.length; i++) {
                int sub = mPhones[i].getSubId();
                if (sub != mPhoneSubscriptions[i]) {
                    MtkTelephonyAssistGlobal.logd(TAG, "onSubscriptionsChanged"
                            + " phoneId = " + i
                            + " old subId = " + mPhoneSubscriptions[i]
                            + " new subId = " + sub);
                    mPhoneSubscriptions[i] = sub;
                    if (SubscriptionManager.isValidSubscriptionId(sub)) {
                        registerSettingsObserver(i);
                        syncDataSettingToMd(i, true);
                        syncDataSlotToMd();
                    }
                }
            }
        }
    };

    // Default data change listener.
    protected final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            MtkTelephonyAssistGlobal.logd(TAG, "onReceive action = " + action);
            if (action != null
                    && action.equals(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                syncDataSlotToMd();
            }
        }
    };

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int phoneId = msg.what % BASE;
            int evendId = msg.what - phoneId;
            MtkTelephonyAssistGlobal.logd(TAG, "handleMessage phoneId = " + phoneId
                    + " evendId = " + evendId);

            switch (evendId) {
                case EVENT_DATA_SETTING_CHANGE:
                case EVENT_ROAMING_SETTING_CHANGE:
                    syncDataSettingToMd(phoneId, false);
                    if (evendId == EVENT_ROAMING_SETTING_CHANGE) {
                        // Send message to DcTracker.java to handle roaming settings change event.
                        // we comment DcTracker.java's listen code to make sure always first sync
                        // data roaming settings, then trigger activate/deactivate pdn, required
                        // by our modem.
                        mPhones[phoneId].getDcTracker(
                                AccessNetworkConstants.TRANSPORT_TYPE_WWAN).sendMessage(
                                        obtainMessage(DctConstants.EVENT_ROAMING_SETTING_CHANGE));
                        // send to wlan too for safty, aosp trigger this too.
                        if (mPhones[phoneId].getDcTracker(
                            AccessNetworkConstants.TRANSPORT_TYPE_WLAN) != null) {
                                mPhones[phoneId].getDcTracker(
                                    AccessNetworkConstants.TRANSPORT_TYPE_WLAN).sendMessage(
                                        obtainMessage(DctConstants.EVENT_ROAMING_SETTING_CHANGE));
                        }
                    }
                    break;
                case EVENT_DOMESTIC_ROAMING_SETTING_CHANGE:
                case EVENT_INTERNATIONAL_ROAMING_SETTING_CHANGE:
                    syncDataSettingToMd(phoneId, true);
                    // Send message to Dctracker.java to handle domestic and
                    // internatiomal roaming settings change event.
                    mPhones[phoneId].getDcTracker(
                            AccessNetworkConstants.TRANSPORT_TYPE_WWAN).sendMessage(
                                    obtainMessage(DctConstants.EVENT_ROAMING_SETTING_CHANGE));
                    break;
                default:
                    break;
            }
        }
    };

    public DataStateController(Context context) {
        super(context);
    }

    @Override
    public void initialize() {
        MtkTelephonyAssistGlobal.logd(TAG, "onPhoneCreated");

        mPhoneSubscriptions = new int[mPhones.length];
        mSettingsObserver = new SettingsObserver[mPhones.length];
        mRadioState = new boolean[mPhones.length];

        // Listen to data roaming on/off change.
        for (int i = 0; i < mPhones.length; i++) {
            mPhoneSubscriptions[i] = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
            mSettingsObserver[i] = new SettingsObserver(mContext, mHandler);
            registerSettingsObserver(i);
        }

        // Default data slot change.
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        mContext.registerReceiver(mBroadcastReceiver, filter);

        // Subscription change.
        SubscriptionManager.from(mContext).addOnSubscriptionsChangedListener(
                mSubscriptionsChangedListener);
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        MtkTelephonyAssistGlobal.logd(TAG, "onDestroy");

        mContext.unregisterReceiver(mBroadcastReceiver);
        SubscriptionManager.from(mContext).removeOnSubscriptionsChangedListener(
                mSubscriptionsChangedListener);

        if (mPhones != null) {
            for (int i = 0; i < mPhones.length; i++) {
                unregisterSettingsObserver(i);
            }
        }
    }

    @Override
    public void onRadioChanged(int phoneId) {
        boolean state = mPhones[phoneId].isRadioOn();
        MtkTelephonyAssistGlobal.logd(TAG, "phoneId = " + phoneId
                + " old state = " + mRadioState[phoneId] + " new state = " + state);
        if (!mRadioState[phoneId] && state) {
            syncAllDataStateToMd();
        }
        mRadioState[phoneId] = state;
    }

    private void unregisterSettingsObserver(int phoneId) {
        mPhones[phoneId].getDataEnabledSettings().unregisterForDataEnabledChanged(mHandler);
        mSettingsObserver[phoneId].unobserve();
    }

    private void registerSettingsObserver(int phoneId) {
        if (phoneId >= mPhones.length || phoneId < 0) {
            MtkTelephonyAssistGlobal.logd(TAG, "registerSettingsObserver invalid phoneId");
            return;
        }

        final int phoneSubId = mPhones[phoneId].getSubId();
        // Unregister before register.
        unregisterSettingsObserver(phoneId);

        // Register data setting from DataEnabledSettings.java
        // Modem has requirement that data setting values should be sent to modem before
        // PDN activation/deactivation requests to modem, here the way of register data
        // setting keeps consist with DcTracker.java
        // DataStateController.java registers first and then DcTracker.java registers
        // will make DataStateController.java receives data setting change event first.
        mPhones[phoneId].getDataEnabledSettings().registerForDataEnabledChanged(
                mHandler, EVENT_DATA_SETTING_CHANGE + phoneId, null);
        // No need to unregister, DcTracker.java will unregister itsself.
        for (int transport : mPhones[phoneId].getTransportManager().getAvailableTransports()) {
            mPhones[phoneId].getDataEnabledSettings().registerForDataEnabledChanged(
                    mPhones[phoneId].getDcTracker(transport),
                    DctConstants.EVENT_DATA_ENABLED_CHANGED, null);
        }

        if (TelephonyManager.getDefault().getSimCount() > 1) {
            mSettingsObserver[phoneId].observe(
                    Settings.Global.getUriFor(Settings.Global.DATA_ROAMING + phoneSubId),
                    EVENT_ROAMING_SETTING_CHANGE + phoneId);
        } else {
            mSettingsObserver[phoneId].observe(
                    Settings.Global.getUriFor(Settings.Global.DATA_ROAMING),
                    EVENT_ROAMING_SETTING_CHANGE + phoneId);
        }

        mSettingsObserver[phoneId].observe(
                Settings.Global.getUriFor(DOMESTIC_DATA_ROAMING + phoneSubId),
                EVENT_DOMESTIC_ROAMING_SETTING_CHANGE + phoneId);
        mSettingsObserver[phoneId].observe(
                Settings.Global.getUriFor(INTERNATIONAL_DATA_ROAMING + phoneSubId),
                EVENT_INTERNATIONAL_ROAMING_SETTING_CHANGE + phoneId);
    }

    // Sync all data state to modem including Data on/off, Roaming On/off, Data slot.
    private void syncAllDataStateToMd() {
        for (int i = 0; i < mPhones.length; i++) {
            syncDataSettingToMd(i, true);
        }
        syncDataSlotToMd();
    }

    // Sync Data on/off, Roaming On/off setting by request.
    private void syncDataSettingToMd(final int phoneId, boolean isSyncExtra) {
        int subId = mPhones[phoneId].getSubId();
        if (SubscriptionManager.isValidSubscriptionId(subId)) {
            int dataOn = (isDataOn(phoneId) ? 1 : 0);
            int roamingOn = (isRoamingDataOn(phoneId) ? 1 : 0);
            int domesticOn = SKIP_VALUE;
            int internationalOn = SKIP_VALUE;

            if (isSyncExtra) {
                domesticOn = (isDomesticRoamingDataOn(phoneId) ? 1 : 0);
                internationalOn = (isInternationalRoamingDataOn(phoneId) ? 1 : 0);
            }

            // Always sync data switch and data roaming switch,
            // sync domestic roaming switch and international roaming switch both or not.
            if (mAssistRIL[phoneId] != null) {
                mAssistRIL[phoneId].syncDataSettings(new int[] {dataOn, roamingOn, SKIP_VALUE,
                        domesticOn, internationalOn}, null);
            } else {
                MtkTelephonyAssistGlobal.loge(TAG, "mAssistRIL is null");
            }
        }
    }

    // Sync default data slot to modem, always use slot 0.
    private void syncDataSlotToMd() {
        if (mAssistRIL[0] != null) {
            mAssistRIL[0].syncDataSettings(
                    new int[] {SKIP_VALUE, SKIP_VALUE, getDataSim(), SKIP_VALUE, SKIP_VALUE}, null);
        } else {
            MtkTelephonyAssistGlobal.loge(TAG, "mAssistRIL is null");
        }
    }

    private int getDataSim() {
        int defaultSubId = SubscriptionController.getInstance().getDefaultDataSubId();
        return SubscriptionController.getInstance().getSlotIndex(defaultSubId);
    }

    private boolean isDataOn(int phoneId) {
        return mPhones[phoneId].getDataEnabledSettings().isUserDataEnabled();
    }

    private boolean isRoamingDataOn(int phoneId) {
        return mPhones[phoneId].getDataEnabledSettings().getDataRoamingEnabled();
    }

    private boolean isDomesticRoamingDataOn(int phoneId) {
        try {
            return (Settings.Global.getInt(mContext.getContentResolver(),
                    DOMESTIC_DATA_ROAMING + mPhones[phoneId].getSubId()) != 0);
        } catch (SettingNotFoundException e) {
        }

        return false;
    }

    private boolean isInternationalRoamingDataOn(int phoneId) {
        try {
            return (Settings.Global.getInt(mContext.getContentResolver(),
                    INTERNATIONAL_DATA_ROAMING + mPhones[phoneId].getSubId()) != 0);
        } catch (SettingNotFoundException e) {
        }

        return false;
    }
}
