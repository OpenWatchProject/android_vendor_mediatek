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

import android.app.ActionBar;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.provider.Settings;
/// M: Add for handling call/radio state change.
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
/// M: Add for listening phone/SIM state change.
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.Menu;
/// M: Add for handling the subscription change..
import android.view.MenuItem;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

/// M: Add for SIM On/Off feature.
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.util.CollectionUtils;
import com.android.settings.R;
import com.android.settings.core.FeatureFlags;
import com.android.settings.core.SettingsBaseActivity;
import com.android.settings.development.featureflags.FeatureFlagPersistent;
import com.android.settings.network.SubscriptionUtil;

import com.google.android.material.bottomnavigation.BottomNavigationView;

/// M: Add for operator config changed.
import com.mediatek.common.carrierexpress.CarrierExpressManager;
/// M: Add for handling main phone changed.
import com.mediatek.ims.internal.MtkImsManager;
/// M: Add for supporting SIM hot swap.
import com.mediatek.settings.sim.SimHotSwapHandler;
/// M: Add for checking radio/SIM state.
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class MobileNetworkActivity extends SettingsBaseActivity {

    private static final String TAG = "MobileNetworkActivity";
    @VisibleForTesting
    static final String MOBILE_SETTINGS_TAG = "mobile_settings:";
    @VisibleForTesting
    static final int SUB_ID_NULL = Integer.MIN_VALUE;

    @VisibleForTesting
    SubscriptionManager mSubscriptionManager;
    @VisibleForTesting
    int mCurSubscriptionId;
    @VisibleForTesting
    List<SubscriptionInfo> mSubscriptionInfos = new ArrayList<>();
    private PhoneChangeReceiver mPhoneChangeReceiver;

    /// M: Add for recording phone state. @{
    private int mPhoneId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
    private boolean mIsInCall = false;
    private boolean mIsRadioOn = false;
    private int mPhoneType = TelephonyManager.PHONE_TYPE_NONE;;
    /// @}

    /// M: Add for handling main phone changed.
    private int mMainPhoneId;

    /// M: Add for handling radio state change. @{
    private int mPhoneCount = TelephonyManager.getDefault().getPhoneCount();
    private PhoneStateListener[] mPhoneStateListener = new PhoneStateListener[mPhoneCount];
    /// @}

    /// M: Add for SIM On/Off feature. @{
    private boolean mSimOnOffEnabled = false;
    private boolean mIsSimOn = false;
    /// @}

    private final SubscriptionManager.OnSubscriptionsChangedListener
            mOnSubscriptionsChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            List<SubscriptionInfo> curList = mSubscriptionManager.getActiveSubscriptionInfoList(true);

            /// M: Add for supporting SIM hot swap. @{
            if (SimHotSwapHandler.hasHotSwapHappened(mSubscriptionInfos, curList)) {
                finish();
                return;
            }
            /// @}

            if (!Objects.equals(mSubscriptionInfos, curList)) {
                /// M: Add for handling the subscription change.
                handleSubscriptionChange(curList);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (FeatureFlagPersistent.isEnabled(this, FeatureFlags.NETWORK_INTERNET_V2)) {
            setContentView(R.layout.mobile_network_settings_container_v2);
        } else {
            setContentView(R.layout.mobile_network_settings_container);
        }
        setActionBar(findViewById(R.id.mobile_action_bar));
        mPhoneChangeReceiver = new PhoneChangeReceiver();
        mSubscriptionManager = getSystemService(SubscriptionManager.class);
        mSubscriptionInfos = mSubscriptionManager.getActiveSubscriptionInfoList(true);
        mCurSubscriptionId = savedInstanceState != null
                ? savedInstanceState.getInt(Settings.EXTRA_SUB_ID, SUB_ID_NULL)
                : SUB_ID_NULL;

        final ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        /// M: Add for recording phone state. @{
        mIsInCall = TelephonyUtils.isInCall();
        if (SubscriptionManager.isValidSubscriptionId(mCurSubscriptionId)) {
            mPhoneId = SubscriptionManager.getPhoneId(mCurSubscriptionId);
            mIsRadioOn = TelephonyUtils.isRadioOn(mCurSubscriptionId, this);
            mPhoneType = TelephonyManager.getDefault().getCurrentPhoneTypeForSlot(mPhoneId);
        } else {
            mPhoneId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
            mIsRadioOn = false;
            mPhoneType = TelephonyManager.PHONE_TYPE_NONE;
        }
        /// @}

        /// M: Add for SIM On/Off feature. @{
        mSimOnOffEnabled = TelephonyUtils.isSimOnOffEnabled();
        if (SubscriptionManager.isValidSubscriptionId(mCurSubscriptionId)) {
            if (mSimOnOffEnabled) {
                mIsSimOn = (TelephonyUtils.getSimOnOffState(mPhoneId)
                        == TelephonyUtils.SIM_ONOFF_STATE_ON);
            } else {
                mIsSimOn = true;
            }
        } else {
            mIsSimOn = false;
        }
        Log.d(TAG, "onCreate, simOnOffEnabled=" + mSimOnOffEnabled
                + ", phoneId=" + mPhoneId + ", subId=" + mCurSubscriptionId
                + ", isSimOn=" + mIsSimOn);
        /// @}

        /// M: Add for registering receiver and listener. @{
        final IntentFilter intentFilter = new IntentFilter(
                TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);

        // Finish the activity when airplane mode changed.
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        // Update screen status when phone state changed.
        intentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        /// M: Add for handling main phone changed. @{
        mMainPhoneId = TelephonyUtils.getMainCapabilityPhoneId();
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
        /// @}
        /// M: Add for SIM ME Lock feature. @{
        // Update screen status when SIM lock state changed.
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION);
        /// @}
        /// M: Add for SIM On/Off feature. @{
        // Update screen status when SIM on/off state changed.
        intentFilter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        /// @}
        /// M: Add for operator config changed. @{
        // Finish the activity when operator config changed.
        intentFilter.addAction(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED);
        /// @}
        registerReceiver(mPhoneChangeReceiver, intentFilter);
        mSubscriptionManager.addOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
        registerPhoneStateListener();
        /// @}

        /// M: Add for recording phone state.
        updatePhoneStatus();

        updateSubscriptions(savedInstanceState);
    }

    @Override
    protected void onStart() {
        super.onStart();

        /// M: Add for checking airplane mode. @{
        if (TelephonyUtils.isAirplaneModeOn(this)) {
            Log.d(TAG, "onStart, airplane mode is on, finish activity.");
            finish();
        }
        /// @}
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    /// M: Add for unregistering receiver and listener. @{
    @Override
    protected void onDestroy() {
        unregisterReceiver(mPhoneChangeReceiver);
        mSubscriptionManager.removeOnSubscriptionsChangedListener(mOnSubscriptionsChangeListener);
        unregisterPhoneStateListener();

        super.onDestroy();
    }
    /// @}

    @Override
    protected void onSaveInstanceState(@NonNull Bundle outState) {
        super.onSaveInstanceState(outState);
        saveInstanceState(outState);
    }

    @VisibleForTesting
    void saveInstanceState(@NonNull Bundle outState) {
        outState.putInt(Settings.EXTRA_SUB_ID, mCurSubscriptionId);
    }

    @VisibleForTesting
    void updateSubscriptions(Bundle savedInstanceState) {
        // Set the title to the name of the subscription. If we don't have subscription info, the
        // title will just default to the label for this activity that's already specified in
        // AndroidManifest.xml.
        /// M: Revise for checking whether the current subId exists or not first. @{
        SubscriptionInfo subscription;
        if (mCurSubscriptionId != SUB_ID_NULL) {
            subscription = mSubscriptionManager.getActiveSubscriptionInfo(mCurSubscriptionId);
        } else {
            subscription = getSubscription();
        }
        /// @}
        if (subscription != null) {
            setTitle(subscription.getDisplayName());
        }

        mSubscriptionInfos = mSubscriptionManager.getActiveSubscriptionInfoList(true);

        if (!FeatureFlagPersistent.isEnabled(this, FeatureFlags.NETWORK_INTERNET_V2)) {
            updateBottomNavigationView();
        }

        if (savedInstanceState == null) {
            switchFragment(new MobileNetworkSettings(), getSubscriptionId());
        }
    }

    /**
     * Get the current subscription to display. First check whether intent has {@link
     * Settings#EXTRA_SUB_ID} and if so find the subscription with that id. If not, just return the
     * first one in the mSubscriptionInfos list since it is already sorted by sim slot.
     */
    @VisibleForTesting
    SubscriptionInfo getSubscription() {
        final Intent intent = getIntent();
        if (intent != null) {
            final int subId = intent.getIntExtra(Settings.EXTRA_SUB_ID, SUB_ID_NULL);
            if (subId != SUB_ID_NULL) {
                for (SubscriptionInfo subscription :
                        SubscriptionUtil.getAvailableSubscriptions(this)) {
                    if (subscription.getSubscriptionId() == subId) {
                        return subscription;
                    }
                }
            }
        }

        if (CollectionUtils.isEmpty(mSubscriptionInfos)) {
            return null;
        }
        return mSubscriptionInfos.get(0);
    }

    /**
     * Get the current subId to display.
     */
    @VisibleForTesting
    int getSubscriptionId() {
        final SubscriptionInfo subscription = getSubscription();
        if (subscription != null) {
            return subscription.getSubscriptionId();
        }
        return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }

    @VisibleForTesting
    void updateBottomNavigationView() {
        final BottomNavigationView navigation = findViewById(R.id.bottom_nav);

        if (CollectionUtils.size(mSubscriptionInfos) <= 1) {
            navigation.setVisibility(View.GONE);
        } else {
            final Menu menu = navigation.getMenu();
            menu.clear();
            for (int i = 0, size = mSubscriptionInfos.size(); i < size; i++) {
                final SubscriptionInfo subscriptionInfo = mSubscriptionInfos.get(i);
                menu.add(0, subscriptionInfo.getSubscriptionId(), i,
                        subscriptionInfo.getDisplayName())
                        .setIcon(R.drawable.ic_settings_sim);
            }
            navigation.setOnNavigationItemSelectedListener(item -> {
                switchFragment(new MobileNetworkSettings(), item.getItemId());
                return true;
            });
        }
    }

    @VisibleForTesting
    void switchFragment(Fragment fragment, int subscriptionId) {
        switchFragment(fragment, subscriptionId, false /* forceUpdate */);
    }

    @VisibleForTesting
    void switchFragment(Fragment fragment, int subscriptionId, boolean forceUpdate) {
        if (mCurSubscriptionId != SUB_ID_NULL && subscriptionId == mCurSubscriptionId
                && !forceUpdate) {
            return;
        }
        final FragmentManager fragmentManager = getSupportFragmentManager();

        /// M: Disable the previous fragment before switching fragment. @{
        final Fragment prevFragment = fragmentManager.findFragmentByTag(
                buildFragmentTag(mCurSubscriptionId));
        if (prevFragment != null && (prevFragment instanceof MobileNetworkSettings)) {
            MobileNetworkSettings mobileFragment = (MobileNetworkSettings) prevFragment;
            mobileFragment.setValidity(false);
        }
        /// @}

        final FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
        final Bundle bundle = new Bundle();
        bundle.putInt(Settings.EXTRA_SUB_ID, subscriptionId);

        fragment.setArguments(bundle);
        fragmentTransaction.replace(R.id.main_content, fragment,
                buildFragmentTag(subscriptionId));
        /// M: Add for allowing that this commit can be lost.
        fragmentTransaction.commitAllowingStateLoss();
        mCurSubscriptionId = subscriptionId;

        /// M: Add for recording phone state. @{
        mIsInCall = TelephonyUtils.isInCall();
        mPhoneId = SubscriptionManager.getPhoneId(mCurSubscriptionId);
        mIsRadioOn = TelephonyUtils.isRadioOn(mCurSubscriptionId, this);
        mPhoneType = TelephonyManager.getDefault().getCurrentPhoneTypeForSlot(mPhoneId);
        /// @}

        /// M: Add for SIM On/Off feature. @{
        if (mSimOnOffEnabled) {
            mIsSimOn = (TelephonyUtils.getSimOnOffState(mPhoneId)
                    == TelephonyUtils.SIM_ONOFF_STATE_ON);
        } else {
            mIsSimOn = true;
        }
        Log.d(TAG, "switchFragment, simOnOffEnabled=" + mSimOnOffEnabled
                + ", phoneId=" + mPhoneId + ", subId=" + mCurSubscriptionId
                + ", isSimOn=" + mIsSimOn);
        /// @}

        /// M: Add for recording phone state.
        updatePhoneStatus();

        /// M: Update screen status.
        updateScreenStatus();

        /// M: Update title and bottom navigation. @{
        if (mSubscriptionInfos != null) {
            final BottomNavigationView navigation = findViewById(R.id.bottom_nav);
            Menu menu = null;
            if (navigation != null) {
                menu = navigation.getMenu();
                if (menu != null) {
                    menu.setGroupEnabled(0, false);
                }
            }

            for (SubscriptionInfo subInfo : mSubscriptionInfos) {
                MenuItem item = null;
                if (menu != null) {
                    item = menu.findItem(subInfo.getSubscriptionId());
                }

                if (subInfo.getSubscriptionId() == mCurSubscriptionId) {
                    setTitle(subInfo.getDisplayName());
                    if (item != null) {
                        item.setChecked(true);
                    }
                    break;
                }
            }
        }
        /// @}
    }

    private String buildFragmentTag(int subscriptionId) {
        return MOBILE_SETTINGS_TAG + subscriptionId;
    }

    private class PhoneChangeReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive, action=" + action);

            if (isFinishing() || isDestroyed()) {
                Log.d(TAG, "onReceive, activity is finished.");
                return;
            }

            /// M: Add for receiving more broadcast intents. @{
            if (action.equals(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED)) {
                // When the radio changes (ex: CDMA->GSM), refresh the fragment.
                // This is very rare to happen.
                /// M: Add for recording phone state. @{
                // When the current SIM's phone type is not changed, there is
                // no need to refresh the fragment.
                if (mPhoneId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
                    int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                            SubscriptionManager.INVALID_SIM_SLOT_INDEX);
                    if (phoneId != mPhoneId) {
                        Log.d(TAG, "onReceive, phoneId is not matched, curPhoneId="
                                + mPhoneId + ", changedPhoneId=" + phoneId);
                        return;
                    }
                    int phoneType = TelephonyManager.getDefault()
                            .getCurrentPhoneTypeForSlot(mPhoneId);
                    if (phoneType == mPhoneType) {
                        Log.d(TAG, "onReceive, phoneType=" + mPhoneType
                                + " is not changed.");
                        return;
                    }
                } else {
                    Log.d(TAG, "onReceive, phoneId=" + mPhoneId + " is invalid.");
                    return;
                }
                /// @}

                if (mCurSubscriptionId != SUB_ID_NULL) {
                    /// M: Disable the preferred network type before switching fragment. @{
                    final FragmentManager fragmentManager = getSupportFragmentManager();
                    final Fragment fragment = fragmentManager.findFragmentByTag(
                            buildFragmentTag(mCurSubscriptionId));
                    if (fragment != null && (fragment instanceof MobileNetworkSettings)) {
                        MobileNetworkSettings mobileFragment = (MobileNetworkSettings) fragment;
                        EnabledNetworkModePreferenceController enabledNetworkModeController =
                                mobileFragment.getController(
                                        EnabledNetworkModePreferenceController.class);
                        if (enabledNetworkModeController != null) {
                            enabledNetworkModeController.setEnableState(false);
                        }
                    }
                    /// @}
                    switchFragment(new MobileNetworkSettings(), mCurSubscriptionId,
                            true /* forceUpdate */);
                }
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            } else if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                boolean isInCall = TelephonyUtils.isInCall();
                if (mIsInCall != isInCall) {
                    mIsInCall = isInCall;
                    MobileNetworkUtils.setInCallState(mIsInCall);
                    updateScreenStatus();
                }
            /// M: Add for handling main phone changed. @{
            } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)
                    || action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED)) {
                int currMainPhoneId = TelephonyUtils.getMainCapabilityPhoneId();
                if (currMainPhoneId != mMainPhoneId) {
                    int currPhoneId = SubscriptionManager.getPhoneId(mCurSubscriptionId);
                    if (currPhoneId == mMainPhoneId || currPhoneId == currMainPhoneId) {
                        Log.d(TAG, "Main phone ID is changed from " + mMainPhoneId
                                + " to " + currMainPhoneId + ".");
                        switchFragment(new MobileNetworkSettings(), mCurSubscriptionId,
                                true /* forceUpdate */);
                    }
                    mMainPhoneId = currMainPhoneId;
                }
            /// @}
            /// M: Add for SIM On/Off feature. @{
            } else if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                handleSimApplicationStateChange(intent);
            /// @}
            /// M: Add for operator config changed. @{
            } else if (action.equals(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED)) {
                finish();
            }
            /// @}
            /// @}
        }
    }

    /// M: Add for handling radio state change. @{
    private PhoneStateListener getPhoneStateListener(int phoneId, int subId) {
        final int phoneIdListened = phoneId;
        final int subIdListened = subId;
        mPhoneStateListener[phoneId]  = new PhoneStateListener() {
            @Override
            public void onRadioPowerStateChanged(int state) {
                Log.d(TAG, "PhoneStateListener.onRadioPowerStateChanged, state=" + state
                        + ", phoneId=" + phoneIdListened);
                if (isFinishing() || isDestroyed()) {
                    Log.d(TAG, "PhoneStateListener.onRadioPowerStateChanged"
                            + ", activity is finished.");
                    return;
                }

                if (phoneIdListened == mPhoneId) {
                    boolean isRadioOn = (state == TelephonyManager.RADIO_POWER_ON);
                    if (mIsRadioOn != isRadioOn) { 
                        mIsRadioOn = isRadioOn;
                        MobileNetworkUtils.setRadioOnState(mIsRadioOn, mPhoneId);
                        updateScreenStatus();
                    }
                }
            }
        };
        return mPhoneStateListener[phoneId];
    }

    private void registerPhoneStateListener() {
        if (mSubscriptionInfos != null) {
            final TelephonyManager tm = (TelephonyManager)
                    getSystemService(Context.TELEPHONY_SERVICE);
            Log.d(TAG, "Register for phone state change");
            for (int i = 0; i < mSubscriptionInfos.size(); i++) {
                SubscriptionInfo subInfo = mSubscriptionInfos.get(i);
                int subId = subInfo.getSubscriptionId();
                int slotId = subInfo.getSimSlotIndex();
                tm.createForSubscriptionId(subId).listen(getPhoneStateListener(slotId, subId),
                        PhoneStateListener.LISTEN_RADIO_POWER_STATE_CHANGED);
            }
        }
    }

    private void unregisterPhoneStateListener() {
        final TelephonyManager tm = (TelephonyManager)
                getSystemService(Context.TELEPHONY_SERVICE);
        for (int i = 0; i < mPhoneCount; i++) {
            if (mPhoneStateListener[i] != null) {
                tm.listen(mPhoneStateListener[i], PhoneStateListener.LISTEN_NONE);
                mPhoneStateListener[i] = null;
            }
        }
    }
    /// @}

    /// M: Add for recording phone state. @{
    private void updatePhoneStatus() {
        MobileNetworkUtils.setInCallState(mIsInCall);
        if (SubscriptionManager.isValidPhoneId(mPhoneId)) {
            MobileNetworkUtils.setRadioOnState(mIsRadioOn, mPhoneId);
            MobileNetworkUtils.setSimOnState(mIsSimOn, mPhoneId);
        }

        for (SubscriptionInfo subInfo : mSubscriptionInfos) {
            MobileNetworkUtils.updateIccCardType(
                    subInfo.getSubscriptionId(),
                    subInfo.getSimSlotIndex());
            MobileNetworkUtils.updateCdmaCardType(
                    subInfo.getSimSlotIndex());
        }
    }
    /// @}

    /// M: Add for receiving more broadcast intents. @{
    private void updateScreenStatus() {
        Log.d(TAG, "updateScreenStatus, subId=" + mCurSubscriptionId);

        final FragmentManager fragmentManager = getSupportFragmentManager();
        Fragment fragment = fragmentManager.findFragmentByTag(
                buildFragmentTag(mCurSubscriptionId));
        if (fragment != null && (fragment instanceof MobileNetworkSettings)) {
            MobileNetworkSettings mobileFragment = (MobileNetworkSettings) fragment;
            mobileFragment.updateAllPreferenceStates();
        }
    }
    /// @}

    /// M: Add for SIM On/Off feature. @{
    private void handleSimApplicationStateChange(Intent intent) {
        if (!mSimOnOffEnabled) {
            Log.d(TAG, "handleSimApplicationStateChange, SIM On/Off is not enabled.");
            return;
        }

        Bundle extra = intent.getExtras();
        if (extra == null) {
            Log.d(TAG, "handleSimApplicationStateChange, extra=null");
            return;
        }

        int slotId = extra.getInt(PhoneConstants.SLOT_KEY,
                SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        int subId = extra.getInt(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        boolean isSimOn = (TelephonyUtils.getSimOnOffState(slotId)
                == TelephonyUtils.SIM_ONOFF_STATE_ON);
        if (SubscriptionManager.isValidSubscriptionId(subId)
                && (mCurSubscriptionId == subId)
                && (mIsSimOn != isSimOn)) {
            Log.d(TAG, "handleSimApplicationStateChange, subId=" + subId
                    + ", preSimOn=" + mIsSimOn + ", curSimOn =" + isSimOn);
            mIsSimOn = isSimOn;
            MobileNetworkUtils.setSimOnState(mIsSimOn, mPhoneId);
            updateScreenStatus();
        }
    }
    /// @}

    /// M: Add for handling the subscription change. @{
    private void handleSubscriptionChange(List<SubscriptionInfo> subList) {
        mSubscriptionInfos = subList;
        final int simNum = CollectionUtils.size(mSubscriptionInfos);

        // No need to update when there is no SIM.
        if (simNum == 0) {
            return;
        }

        SubscriptionInfo subInfo;

        // Update title when there is only 1 SIM.
        if (simNum == 1) {
            subInfo = mSubscriptionInfos.get(0);
            setTitle(subInfo.getDisplayName());
            return;
        }

        // Update title and bottom when there are more than 1 SIM.
        final BottomNavigationView navigation = findViewById(R.id.bottom_nav);
        Menu menu = null;
        if (navigation != null) {
            menu = navigation.getMenu();
        }

        for (int i = 0; i < simNum; i++) {
            subInfo = mSubscriptionInfos.get(i);
            // Update title for current SIM.
            if (subInfo.getSubscriptionId() == mCurSubscriptionId) {
                setTitle(subInfo.getDisplayName());
            }
            // Update bottom for all SIM.
            if (menu != null) {
                MenuItem item = menu.findItem(subInfo.getSubscriptionId());
                if (item != null) {
                    item.setTitle(subInfo.getDisplayName());
                }
            }
        }
    }
    /// @}

    @Override
    public void reportFullyDrawn() {
        super.reportFullyDrawn();

        /// M: Update bottom navigation. @{
        if (mSubscriptionInfos != null) {
            final BottomNavigationView navigation = findViewById(R.id.bottom_nav);
            if (navigation != null) {
                Menu menu = navigation.getMenu();
                if (menu != null) {
                    menu.setGroupEnabled(0, true);
                }
            }
        }
        /// @}
    }
}
