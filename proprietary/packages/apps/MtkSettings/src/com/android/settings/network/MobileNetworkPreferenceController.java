/*
 * Copyright (C) 2016 The Android Open Source Project
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
package com.android.settings.network;

import static android.os.UserHandle.myUserId;
import static android.os.UserManager.DISALLOW_CONFIG_MOBILE_NETWORKS;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.UserManager;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
/// M: Add for checking active sub. @{
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
/// @}
import android.telephony.TelephonyManager;
import android.util.FeatureFlagUtils;
import android.util.Log;

import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

/// M: Add for receiving other broadcasts also.
import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.core.FeatureFlags;
import com.android.settings.core.PreferenceControllerMixin;
import com.android.settings.network.telephony.MobileNetworkActivity;
import com.android.settingslib.RestrictedLockUtilsInternal;
import com.android.settingslib.RestrictedPreference;
import com.android.settingslib.Utils;
import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;

/// M: Add for checking call state.
import com.mediatek.settings.sim.TelephonyUtils;

/// M: Add for checking active sub.
import java.util.List;

public class MobileNetworkPreferenceController extends AbstractPreferenceController
        implements PreferenceControllerMixin, LifecycleObserver, OnStart, OnStop {

    private static final String TAG = "MobileNetworkPreferenceController";

    @VisibleForTesting
    static final String KEY_MOBILE_NETWORK_SETTINGS = "mobile_network_settings";
    @VisibleForTesting
    static final String MOBILE_NETWORK_PACKAGE = "com.android.phone";
    @VisibleForTesting
    static final String MOBILE_NETWORK_CLASS = "com.android.phone.MobileNetworkSettings";

    private final boolean mIsSecondaryUser;
    private final TelephonyManager mTelephonyManager;
    private final UserManager mUserManager;
    private Preference mPreference;
    @VisibleForTesting
    PhoneStateListener mPhoneStateListener;

    /// M: Revise for receiving other broadcasts also.
    private BroadcastReceiver mReceiver;

    public MobileNetworkPreferenceController(Context context) {
        super(context);
        mUserManager = (UserManager) context.getSystemService(Context.USER_SERVICE);
        mTelephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        mIsSecondaryUser = !mUserManager.isAdminUser();

        /// M: Revise for receiving other broadcasts also. @{
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.d(TAG, "onReceive, action=" + action);

                if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                    updateState(mPreference);
                } else if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                    updateState(mPreference);
                } else if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                    updateState(mPreference);
                }
            }
        };
        /// @}
    }

    @Override
    public boolean isAvailable() {
        return !isUserRestricted() && !Utils.isWifiOnly(mContext);
    }

    public boolean isUserRestricted() {
        return mIsSecondaryUser ||
                RestrictedLockUtilsInternal.hasBaseUserRestriction(
                        mContext,
                        DISALLOW_CONFIG_MOBILE_NETWORKS,
                        myUserId());
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreference = screen.findPreference(getPreferenceKey());
    }

    @Override
    public String getPreferenceKey() {
        return KEY_MOBILE_NETWORK_SETTINGS;
    }

    @Override
    public void onStart() {
        if (isAvailable()) {
            if (mPhoneStateListener == null) {
                mPhoneStateListener = new PhoneStateListener() {
                    @Override
                    public void onServiceStateChanged(ServiceState serviceState) {
                        updateState(mPreference);
                    }
                };
            }
            mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE);
        }
        /// M: Revise for receiving other broadcasts also. @{
        if (mReceiver != null) {
            final IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
            intentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
            intentFilter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);

            mContext.registerReceiver(mReceiver, intentFilter);
        }
        /// @}
    }

    @Override
    public void onStop() {
        if (mPhoneStateListener != null) {
            mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        }
        /// M: Revise for receiving other broadcasts also. @{
        if (mReceiver != null) {
            mContext.unregisterReceiver(mReceiver);
        }
        /// @}
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);

        if (preference instanceof RestrictedPreference &&
            ((RestrictedPreference) preference).isDisabledByAdmin()) {
                return;
        }

        boolean airplaneMode = (Settings.Global.getInt(
                mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0) != 0);
        boolean inCall = TelephonyUtils.isInCall();
        List<SubscriptionInfo> subList = SubscriptionManager.from(mContext)
                .getActiveSubscriptionInfoList(true);
        boolean enable = !airplaneMode && !inCall && subList != null && subList.size() > 0;
        Log.d(TAG, "updateState, enable=" + enable + ", airplaneMode=" + airplaneMode
                + ", inCall=" + inCall + ", subList=" + (subList == null ? "null" : subList));
        preference.setEnabled(enable);
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (KEY_MOBILE_NETWORK_SETTINGS.equals(preference.getKey())) {
            if (FeatureFlagUtils.isEnabled(mContext, FeatureFlags.MOBILE_NETWORK_V2)) {
                final Intent intent = new Intent(mContext, MobileNetworkActivity.class);
                mContext.startActivity(intent);
            } else {
                final Intent intent = new Intent(Intent.ACTION_MAIN);
                intent.setComponent(
                        new ComponentName(MOBILE_NETWORK_PACKAGE, MOBILE_NETWORK_CLASS));
                mContext.startActivity(intent);
            }
            return true;
        }
        return false;
    }

    @Override
    public CharSequence getSummary() {
        return mTelephonyManager.getNetworkOperatorName();
    }
}
