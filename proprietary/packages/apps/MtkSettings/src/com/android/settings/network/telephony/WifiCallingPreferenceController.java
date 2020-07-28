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
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.wifi.WifiManager;
import android.os.Looper;
import android.os.PersistableBundle;
import android.provider.Settings;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.android.ims.ImsConfig;
import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsServiceClass;

import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.R;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;

import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.IWfcSettingsExt;
import com.mediatek.ims.MtkImsConnectionStateListener;
import com.mediatek.ims.internal.MtkImsManager;

/// M: Add more condition for enabled state.
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.List;


/**
 * Preference controller for "Wifi Calling"
 */
public class WifiCallingPreferenceController extends TelephonyBasePreferenceController implements
        LifecycleObserver, OnStart, OnStop {

    private static final String TAG = "WifiCallingPreferenceController";

    @VisibleForTesting
    static final String KEY_PREFERENCE_CATEGORY = "calling_category";

    private TelephonyManager mTelephonyManager;
    @VisibleForTesting
    CarrierConfigManager mCarrierConfigManager;
    @VisibleForTesting
    ImsManager mImsManager;
    @VisibleForTesting
    PhoneAccountHandle mSimCallManager;
    /// M: Phone state has been listened in MobileNetworkActivity.
    // private PhoneCallStateListener mPhoneStateListener;
    private Preference mPreference;
    private boolean mEditableWfcRoamingMode;
    private boolean mUseWfcHomeModeForRoaming;
    /// M: Add for WFC plugin. wfc summary update feature@{
    IWfcSettingsExt mWfcExt;
    BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (mWfcExt.customizeBroadcastReceiveIntent()) {
                if (null != mPreference) {
                    Log.i(TAG, "receive broadcast and update OP UI");
                    mWfcExt.customizedWfcSummary(mPreference);
                }
            }
        }
    };
    MtkImsConnectionStateListener mImsConnectionStateListener =
        new MtkImsConnectionStateListener() {
        @Override
        public void onCapabilitiesStatusChanged(ImsFeature.Capabilities config) {
            boolean isWfc = config.isCapable(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
            Log.d(TAG, "onCapabilitiesStatusChanged isWfc=" + isWfc);
            if (isWfc && mWfcExt.customizeBroadcastReceiveIntent()) {
                    if (null != mPreference) {
                        Log.i(TAG, "onFeatureCapabilityChanged and update OP UI");
                        mWfcExt.customizedWfcSummary(mPreference);
                    }
            }
        }
    };
    /// @}
    public WifiCallingPreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = context.getSystemService(CarrierConfigManager.class);
        mTelephonyManager = context.getSystemService(TelephonyManager.class);
        /// M: Phone state has been listened in MobileNetworkActivity.
        // mPhoneStateListener = new PhoneCallStateListener(Looper.getMainLooper());
        mEditableWfcRoamingMode = true;
        mUseWfcHomeModeForRoaming = false;
        /// M: Add for WFC plugin. @{
        mWfcExt = UtilsExt.getWfcSettingsExt(context);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        /// M: for AT & T feature
        ///user can set "persist.vendor.entitlement_enabled" open vowifi item @{
        boolean customerVisible = mWfcExt.customizedATTWfcVisable();
        boolean customerVWZVisible = mWfcExt.customizedVWZWfcVisable();
        Log.i(TAG, "getAvailabilityStatus"
                + " customerVisible = " + customerVisible
        		+ " customerVWZVisible = " + customerVWZVisible);
        if (customerVisible || customerVWZVisible) {
            return CONDITIONALLY_UNAVAILABLE;
        }
        /// @}
        /// M: Add for log. @{
        boolean enabled = false;
        if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            enabled = MobileNetworkUtils.isWifiCallingEnabled(mContext, subId);
        }
        /// @}

        Log.i(TAG, "getAvailabilityStatus, subId=" + subId + ", enabled=" + enabled);

        return enabled ? AVAILABLE : UNSUPPORTED_ON_DEVICE;
    }

    @Override
    public void onStart() {
        /// M: Phone state has been listened in MobileNetworkActivity.
        // mPhoneStateListener.register(mSubId);
        IntentFilter filter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        filter.addAction(WifiManager.RSSI_CHANGED_ACTION);
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
        unregisterImsListener();
        try {
            ((MtkImsManager) mImsManager).addImsConnectionStateListener(
                    mImsConnectionStateListener);
            Log.i(TAG, "register ims succeed, " + mImsConnectionStateListener);
        } catch (ImsException e) {
            // Could not get the ImsService.
            Log.w(TAG,"register ims fail!");
        }
    }
    public void unregisterImsListener() {
        if (mImsManager != null) {
            try {
                ((MtkImsManager) mImsManager).removeImsConnectionStateListener(
                        mImsConnectionStateListener);
                Log.i(TAG, "unregister ims succeed, " + mImsConnectionStateListener);
            } catch (ImsException e) {
                // Could not get the ImsService.
                Log.w(TAG,"unregister ims fail!");
            }
        }
    }

    @Override
    public void onStop() {
        /// M: Phone state has been listened in MobileNetworkActivity.
        // mPhoneStateListener.unregister();
        mContext.unregisterReceiver(mReceiver);
        unregisterImsListener();
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreference = screen.findPreference(getPreferenceKey());
        Intent intent = mPreference.getIntent();
        if (intent != null) {
            intent.putExtra(Settings.EXTRA_SUB_ID, mSubId);
        }
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        if (mSimCallManager != null) {
            Intent intent = MobileNetworkUtils.buildPhoneAccountConfigureIntent(mContext,
                    mSimCallManager);
            if (intent == null) {
                // Do nothing in this case since preference is invisible
                return;
            }
            final PackageManager pm = mContext.getPackageManager();
            List<ResolveInfo> resolutions = pm.queryIntentActivities(intent, 0);
            preference.setTitle(resolutions.get(0).loadLabel(pm));
            preference.setSummary(null);
            preference.setIntent(intent);
        } else {
            final String title = SubscriptionManager.getResourcesForSubId(mContext, mSubId)
                    .getString(R.string.wifi_calling_settings_title);
            preference.setTitle(title);
            int resId = com.android.internal.R.string.wifi_calling_off_summary;
            if (mImsManager.isWfcEnabledByUser()) {
                boolean wfcRoamingEnabled = mEditableWfcRoamingMode && !mUseWfcHomeModeForRoaming;
                final boolean isRoaming = mTelephonyManager.isNetworkRoaming();
                int wfcMode = mImsManager.getWfcMode(isRoaming && wfcRoamingEnabled);
                switch (wfcMode) {
                    case ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY:
                        resId = com.android.internal.R.string.wfc_mode_wifi_only_summary;
                        break;
                    case ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED:
                        resId = com.android.internal.R.string
                                .wfc_mode_cellular_preferred_summary;
                        break;
                    case ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED:
                        resId = com.android.internal.R.string.wfc_mode_wifi_preferred_summary;
                        break;
                    default:
                        break;
                }
            }
            preference.setSummary(resId);
            ///MTK TMO feature, customize wfc summary
            mWfcExt.customizedWfcSummary(preference);
        }
        /// M: Add more condition for enabled state.
        preference.setEnabled(
                MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId));
    }

    public WifiCallingPreferenceController init(int subId) {
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        mImsManager = ImsManager.getInstance(mContext, SubscriptionManager.getPhoneId(mSubId));
        mSimCallManager = mContext.getSystemService(TelecomManager.class)
                .getSimCallManagerForSubscription(mSubId);
        if (mCarrierConfigManager != null) {
            final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(mSubId);
            if (carrierConfig != null) {
                mEditableWfcRoamingMode = carrierConfig.getBoolean(
                        CarrierConfigManager.KEY_EDITABLE_WFC_ROAMING_MODE_BOOL);
                mUseWfcHomeModeForRoaming = carrierConfig.getBoolean(
                        CarrierConfigManager
                                .KEY_USE_WFC_HOME_NETWORK_MODE_IN_ROAMING_NETWORK_BOOL);
            }
        }
        return this;
    }

    private class PhoneCallStateListener extends PhoneStateListener {

        public PhoneCallStateListener(Looper looper) {
            super(looper);
        }

        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            updateState(mPreference);
        }

        public void register(int subId) {
            mSubId = subId;
            mTelephonyManager.listen(this, PhoneStateListener.LISTEN_CALL_STATE);
        }

        public void unregister() {
            mTelephonyManager.listen(this, PhoneStateListener.LISTEN_NONE);
        }
    }
}
