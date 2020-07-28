/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.android.settings.datausage;

import android.app.settings.SettingsEnums;
/// M: Add for SIM On/Off feature.
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
/// M: Add for SIM On/Off feature.
import android.content.IntentFilter;
import android.net.NetworkTemplate;
import android.os.Bundle;
import android.os.RemoteException;
/// M: Add for SIM On/Off feature. @{
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
/// @}
import android.util.AttributeSet;
import android.util.Log;

import androidx.preference.Preference;

/// M: Add for SIM On/Off feature.
import com.android.internal.telephony.PhoneConstants;
import com.android.settings.R;
import com.android.settings.core.SubSettingLauncher;
import com.android.settings.datausage.CellDataPreference.DataStateListener;

/// M: Add for SIM On/Off feature.
import com.mediatek.settings.sim.TelephonyUtils;

public class BillingCyclePreference extends Preference implements TemplatePreference {

    private static final String TAG = "BillingCyclePreference";

    private NetworkTemplate mTemplate;
    private NetworkServices mServices;
    private int mSubId;

    /// M: Add for SIM On/Off feature. @{
    private boolean mSimOnOffEnabled = false;
    private int mSimState = TelephonyUtils.SIM_ONOFF_STATE_ON;
    private int mSlotId;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            handleSimApplicationStateChanged(intent);
        }
    };
    /// @}

    public BillingCyclePreference(Context context, AttributeSet attrs) {
        super(context, attrs);

        /// M: Add for SIM On/Off feature. @{
        mSimOnOffEnabled = TelephonyUtils.isSimOnOffEnabled();
        Log.d(TAG, "BillingCyclePreference, simOnOffEnabled=" + mSimOnOffEnabled);
        /// @}
    }

    @Override
    public void onAttached() {
        super.onAttached();
        mListener.setListener(true, mSubId, getContext());

        /// M: Update enabled state.
        updateEnabled();

        /// M: Add for SIM On/Off feature. @{
        if (mSimOnOffEnabled) {
            getContext().registerReceiver(mReceiver, new IntentFilter(
                    TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED));
        }
        /// @}
    }

    @Override
    public void onDetached() {
        /// M: Add for SIM On/Off feature. @{
        if (mSimOnOffEnabled) {
            getContext().unregisterReceiver(mReceiver);
        }
        /// @}

        mListener.setListener(false, mSubId, getContext());
        super.onDetached();
    }

    @Override
    public void setTemplate(NetworkTemplate template, int subId,
            NetworkServices services) {
        mTemplate = template;
        mSubId = subId;
        mServices = services;
        setSummary(null);

        setIntent(getIntent());

        /// M: Add for SIM On/Off feature. @{
        if (mSimOnOffEnabled) {
            mSlotId = SubscriptionManager.getPhoneId(subId);
            mSimState = TelephonyUtils.getSimOnOffState(mSlotId);
            Log.d(TAG, "setTemplate, slotId=" + mSlotId + ", simState=" + mSimState);
        }
        /// @}
    }

    private void updateEnabled() {
        try {
            setEnabled(mServices.mNetworkService.isBandwidthControlEnabled()
                    && mServices.mTelephonyManager.getDataEnabled(mSubId)
                    /// M: Add for SIM On/Off feature. @{
                    && (mSimState == TelephonyUtils.SIM_ONOFF_STATE_ON)
                    /// @}
                    && mServices.mUserManager.isAdminUser());
        } catch (RemoteException e) {
            setEnabled(false);
        }
    }

    @Override
    public Intent getIntent() {
        Bundle args = new Bundle();
        args.putParcelable(DataUsageList.EXTRA_NETWORK_TEMPLATE, mTemplate);
        return new SubSettingLauncher(getContext())
                .setDestination(BillingCycleSettings.class.getName())
                .setArguments(args)
                .setTitleRes(R.string.billing_cycle)
                .setSourceMetricsCategory(SettingsEnums.PAGE_UNKNOWN)
                .toIntent();
    }

    private final DataStateListener mListener = new DataStateListener() {
        @Override
        public void onChange(boolean selfChange) {
            updateEnabled();
        }
    };

    /// M: Add for SIM On/Off feature. @{
    private void handleSimApplicationStateChanged(Intent intent) {
        Bundle extra = intent.getExtras();
        if (extra == null) {
            Log.d(TAG, "handleSimApplicationStateChange, extra=null");
            return;
        }

        int slotId = extra.getInt(PhoneConstants.SLOT_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        if (slotId == mSlotId) {
            int state = TelephonyUtils.getSimOnOffState(slotId);
            Log.d(TAG, "handleSimApplicationStateChange, slotId=" + slotId
                    + ", prevState=" + mSimState + ", currState=" + state);
            if (mSimState != state) {
                updateEnabled();
            }
            mSimState = state;
        }
    }
    /// @}
}
