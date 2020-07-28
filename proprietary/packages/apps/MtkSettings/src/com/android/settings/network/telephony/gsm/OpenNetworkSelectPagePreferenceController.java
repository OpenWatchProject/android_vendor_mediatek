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

import android.app.settings.SettingsEnums;
import android.content.Context;
import android.os.Bundle;
/// M: Add for getting selection mode async.
import android.os.Handler;
/// M: Add for handling the service state change.
import android.os.Looper;
import android.provider.Settings;
/// M: Add for handling the service state change.
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.android.settings.R;
import com.android.settings.core.BasePreferenceController;
import com.android.settings.core.SubSettingLauncher;
import com.android.settings.network.telephony.MobileNetworkUtils;
import com.android.settings.network.telephony.NetworkSelectSettings;
import com.android.settings.network.telephony.TelephonyBasePreferenceController;
/// M: Add for checking the service state.
import com.android.settingslib.Utils;
/// M: Add for handling the service state change. @{
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;
/// @}
/// M: Add for getting selection mode async.
import com.android.settingslib.utils.ThreadUtils;

/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * Preference controller for "Open network select"
 */
public class OpenNetworkSelectPagePreferenceController extends
        TelephonyBasePreferenceController implements
        /// M: Add for handling the service state change.
        LifecycleObserver, OnStart, OnStop,
        AutoSelectPreferenceController.OnNetworkSelectModeListener {

    private static final String TAG = "OpenNetworkSelectPagePreferenceController";

    private TelephonyManager mTelephonyManager;
    private Preference mPreference;

    /// M: Add for getting selection mode async. @{
    private final Handler mUiHandler;
    private int mSelectMode = TelephonyManager.NETWORK_SELECTION_MODE_UNKNOWN;
    private boolean mIsGettingMode = false;
    /// @}

    public OpenNetworkSelectPagePreferenceController(Context context, String key) {
        super(context, key);
        mTelephonyManager = context.getSystemService(TelephonyManager.class);
        mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        /// M: Add for getting selection mode async.
        mUiHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        /// M: Add for log.
        final boolean enabled =
                MobileNetworkUtils.shouldDisplayNetworkSelectOptions(mContext, subId);

        Log.i(TAG, "getAvailabilityStatus, subId=" + subId + ", enabled=" + enabled);

        return enabled ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreference = screen.findPreference(getPreferenceKey());
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);

        /// M: Add for getting selection mode async. @{
        Log.d(TAG, "updateState, phoneId=" + mPhoneId
                + ", isGettingMode=" + mIsGettingMode);
        if (!mIsGettingMode) {
            mIsGettingMode = true;
            mPreference.setEnabled(false);
            ThreadUtils.postOnBackgroundThread(() -> {
                Log.d(TAG, "postOnBackgroundThread, phoneId=" + mPhoneId
                        + ", isGettingMode=" + mIsGettingMode);
                if (!mIsGettingMode || mPreference == null) {
                    return;
                }

                // Get network selection mode in background
                mSelectMode = mTelephonyManager.getNetworkSelectionMode();

                // Update UI in UI thread
                mUiHandler.postDelayed(() -> {
                            Log.d(TAG, "postDelayed, phoneId=" + mPhoneId
                                    + ", isGettingMode=" + mIsGettingMode);
                            if (!mIsGettingMode || mPreference == null) {
                                return;
                            }

                            mIsGettingMode = false;
                            /// M: Add more condition for enabled state.
                            mPreference.setEnabled(mSelectMode
                                    != TelephonyManager.NETWORK_SELECTION_MODE_AUTO
                                    && MobileNetworkUtils.shouldEnableMobileNetworkOption(
                                            mPhoneId));
                        }, 0);
            });
        }
        /// @}
    }

    @Override
    public CharSequence getSummary() {
        final ServiceState ss = mTelephonyManager.getServiceState();
        /// M: Add for checking the service state.
        if (Utils.isInService(ss)) {
            return mTelephonyManager.getNetworkOperatorName();
        } else {
            return mContext.getString(R.string.network_disconnected);
        }
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (TextUtils.equals(preference.getKey(), getPreferenceKey())) {
            final Bundle bundle = new Bundle();
            bundle.putInt(Settings.EXTRA_SUB_ID, mSubId);
            new SubSettingLauncher(mContext)
                    .setDestination(NetworkSelectSettings.class.getName())
                    .setSourceMetricsCategory(SettingsEnums.MOBILE_NETWORK_SELECT)
                    .setTitleRes(R.string.choose_network_title)
                    .setArguments(bundle)
                    .launch();
            return true;
        }

        return false;
    }

    public OpenNetworkSelectPagePreferenceController init(int subId) {
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);

        /// M: Add for getting selection mode async. @{
        mUiHandler.removeCallbacksAndMessages(null);
        mSelectMode = TelephonyManager.NETWORK_SELECTION_MODE_UNKNOWN;
        mIsGettingMode = false;
        /// @}

        return this;
    }

    @Override
    public void onNetworkSelectModeChanged() {
        updateState(mPreference);
    }

    /// M: Add for handling the service state change. @{
    private MyPhoneStateListener mPhoneStateListener =
            new MyPhoneStateListener(Looper.getMainLooper());

    @Override
    public void onStart() {
        mPhoneStateListener.register(mSubId);
    }

    @Override
    public void onStop() {
        mPhoneStateListener.unregister();
    }

    private class MyPhoneStateListener extends PhoneStateListener {

        public MyPhoneStateListener(Looper looper) {
            super(looper);
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            Log.d(TAG, "PhoneStateListener.onServiceStateChanged"
                    + ", serviceState=" + serviceState + ", subId=" + mSubId);
            if (mPreference != null) {
                String summary = mContext.getString(R.string.network_disconnected);
                if (Utils.isInService(serviceState)) {
                    summary = mTelephonyManager.getNetworkOperatorName();
                }
                mPreference.setSummary(summary);
            }
        }

        public void register(int subId) {
            mSubId = subId;
            mTelephonyManager.listen(this, PhoneStateListener.LISTEN_SERVICE_STATE);
        }

        public void unregister() {
            mTelephonyManager.listen(this, PhoneStateListener.LISTEN_NONE);
        }
    }
    /// @}
}
