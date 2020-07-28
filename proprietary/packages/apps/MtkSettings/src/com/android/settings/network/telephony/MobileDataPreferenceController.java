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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.FragmentManager;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreference;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.R;
import com.android.settings.network.MobileDataContentObserver;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;

import com.mediatek.settings.UtilsExt;
/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

/// M: Update the preference when receiving the broadcasts.
import java.util.List;

/**
 * Preference controller for "Mobile data"
 */
public class MobileDataPreferenceController extends TelephonyTogglePreferenceController
        implements LifecycleObserver, OnStart, OnStop {

    private static final String DIALOG_TAG = "MobileDataDialog";

    private static final String TAG = "MobileDataPreferenceController";

    private SwitchPreference mPreference;
    private PreferenceScreen mPreferenceScreen;

    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private MobileDataContentObserver mDataContentObserver;
    private FragmentManager mFragmentManager;

    @VisibleForTesting
    int mDialogType;
    @VisibleForTesting
    boolean mNeedDialog;

    /// M: Update the preference when receiving the broadcasts. @{
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);

            if (action.equals(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION)) {
                handleSimLockStateChange();
            /// M: for china mobile feature, only cmcc card can show volte item start. @{
            } else if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(
                        intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE))) {
                    boolean customizeVolteStateReceiveIntent
                            = UtilsExt.getMobileNetworkSettingsExt(mContext)
                                    .customizeVolteStateReceiveIntent();
                    if (null != mPreferenceScreen && customizeVolteStateReceiveIntent) {
                        Log.i(TAG, "onReceive , update ui");
                        displayPreference(mPreferenceScreen);
                    }
                }
            /// @}
            }
        }
    };
    /// @}

    public MobileDataPreferenceController(Context context, String key) {
        super(context, key);
        mSubscriptionManager = context.getSystemService(SubscriptionManager.class);
        mDataContentObserver = new MobileDataContentObserver(new Handler(Looper.getMainLooper()));
        mDataContentObserver.setOnMobileDataChangedListener(() -> updateState(mPreference));
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        /// M: China mobile feature: VOLTE item only show in CMCC card @{
        boolean customizeMobileDataState
                = UtilsExt.getMobileNetworkSettingsExt(mContext).customizaMobileDataState(subId);
        Log.i(TAG, "getAvailabilityStatus subid = " + subId
                + " customizeMobileDataState = " + customizeMobileDataState);
        if (customizeMobileDataState) {
            return CONDITIONALLY_UNAVAILABLE;
        }
        /// @}
        return subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID
                ? AVAILABLE
                : DISABLED_DEPENDENT_SETTING;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreferenceScreen = screen;
        mPreference = screen.findPreference(getPreferenceKey());
        UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizaMobileDataSummary(mPreference, mSubId);
    }

    @Override
    public void onStart() {
        if (mSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            mDataContentObserver.register(mContext, mSubId);

            /// M: Update the preference when receiving the broadcasts. @{
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION);
            /// M: for china mobile feature, only cmcc card can show volte item start
            intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);

            mContext.registerReceiver(mReceiver, intentFilter);
            /// @}
        }
    }

    @Override
    public void onStop() {
        if (mSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            mDataContentObserver.unRegister(mContext);

            /// M: Update the preference when receiving the broadcasts.
            mContext.unregisterReceiver(mReceiver);
        }
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (TextUtils.equals(preference.getKey(), getPreferenceKey())) {
            if (mNeedDialog) {
                showDialog(mDialogType);
            }
            return true;
        }

        return false;
    }

    @Override
    public boolean setChecked(boolean isChecked) {
        mNeedDialog = isDialogNeeded();

        if (!mNeedDialog) {
            // Update data directly if we don't need dialog
            MobileNetworkUtils.setMobileDataEnabled(mContext, mSubId, isChecked, false);
            return true;
        }

        return false;
    }

    @Override
    public boolean isChecked() {
        return mTelephonyManager.isDataEnabled();
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        if (isOpportunistic()) {
            preference.setEnabled(false);
            preference.setSummary(R.string.mobile_data_settings_summary_auto_switch);
        } else {
            /// M: Add more condition for enabled state. @{
            preference.setEnabled(
                    MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId)
                    && shouldEnableCellDataPrefForSimLock());
            /// @}
            preference.setSummary(R.string.mobile_data_settings_summary);
        }
        Log.i(TAG, "updatestated .");
        UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizaMobileDataSummary(mPreference, mSubId);
    }

    private boolean isOpportunistic() {
        SubscriptionInfo info = mSubscriptionManager.getActiveSubscriptionInfo(mSubId);
        return info != null && info.isOpportunistic();
    }

    public void init(FragmentManager fragmentManager, int subId) {
        mFragmentManager = fragmentManager;
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
    }

    @VisibleForTesting
    boolean isDialogNeeded() {
        final boolean enableData = !isChecked();
        final boolean isMultiSim = (mTelephonyManager.getSimCount() > 1);
        /// M: Don't change the data SIM when turning on non data SIM's data. @{
        //final int defaultSubId = mSubscriptionManager.getDefaultDataSubscriptionId();
        final boolean needToDisableOthers = false; //mSubscriptionManager
        //        .isActiveSubscriptionId(defaultSubId) && defaultSubId != mSubId;
        /// @}
        if (enableData && isMultiSim && needToDisableOthers) {
            mDialogType = MobileDataDialogFragment.TYPE_MULTI_SIM_DIALOG;
            return true;
        }
        return false;
    }

    private void showDialog(int type) {
        final MobileDataDialogFragment dialogFragment = MobileDataDialogFragment.newInstance(type,
                mSubId);
        dialogFragment.show(mFragmentManager, DIALOG_TAG);
    }

    /// M: Update the preference when receiving the broadcasts. @{
    private void handleSimLockStateChange() {
        if (mPreference != null) {
            updateState(mPreference);
        }
    }

    private boolean shouldEnableCellDataPrefForSimLock() {
        int policy = TelephonyUtils.getSimLockPolicy();
        int slot = SubscriptionManager.getSlotIndex(mSubId);

        boolean enabledForSimLock = true;
        switch (policy) {
            case TelephonyUtils.SIM_LOCK_POLICY_UNKNOWN:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_REVERSE:
                int simCapa = TelephonyUtils.getSimLockSimCapability(slot);
                if (simCapa != TelephonyUtils.SIM_LOCK_SIM_CAPABILITY_FULL
                        && simCapa != TelephonyUtils.SIM_LOCK_SIM_CAPABILITY_PS_ONLY) {
                    enabledForSimLock = false;
                }
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_ECC:
                enabledForSimLock = isValidSimExistForSimLock();
                break;

            default:
                break;
        }

        if (!enabledForSimLock) {
            Log.d(TAG, "shouldEnableCellDataPrefForSimLock, policy=" + policy
                    + ", enabled=" + enabledForSimLock);
        }
        return enabledForSimLock;
    }

    private boolean isValidSimExistForSimLock() {
        List<SubscriptionInfo> subInfoList =
                SubscriptionManager.from(mContext).getActiveSubscriptionInfoList(true);

        if (subInfoList != null) {
            for (SubscriptionInfo info : subInfoList) {
                int slotId = info.getSimSlotIndex();
                int simValid = TelephonyUtils.getSimLockSimValid(slotId);
                if (simValid == TelephonyUtils.SIM_LOCK_SIM_VALID_YES) {
                    Log.d(TAG, "isValidSimExistForSimLock, SIM" + slotId
                            + "=" + info.getSubscriptionId() + " is valid.");
                    return true;
                }
            }
        }

        Log.d(TAG, "isValidSimExistForSimLock, no valid SIM.");
        return false;
    }
    /// @}
}
