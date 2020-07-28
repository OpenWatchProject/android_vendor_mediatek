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
/// M: Add for updating the state. @{
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
/// @}
import android.os.Looper;
import android.os.PersistableBundle;
/// M: Add for updating the state.
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
/// M: Add for showing toast.
import android.widget.Toast;

import androidx.annotation.VisibleForTesting;
/// M: Add for supporting CTVoLTE.
import androidx.fragment.app.FragmentManager;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreference;

/// M: Add for checking IMS switching state.
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.IccCardConstants;
/// M: Add for updating the state.
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;
import com.android.settings.R;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;

/// M: Add for checking multiple IMS supported state.
import com.mediatek.ims.internal.MtkImsManager;
/// M: Add for handling the click quickly case. @{
import com.mediatek.ims.internal.MtkImsManagerEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
/// M: Add for handling state changed.
import com.mediatek.internal.telephony.MtkSubscriptionManager;
/// @}
import com.mediatek.settings.UtilsExt;
/// M: Add for supporting CTVoLTE.
import com.mediatek.settings.network.Enhanced4gLteDialogFragment;
import com.mediatek.settings.sim.TelephonyUtils;
import mediatek.telephony.MtkCarrierConfigManager;

/// @}

import java.util.ArrayList;
import java.util.List;

/**
 * Preference controller for "Enhanced 4G LTE"
 */
public class Enhanced4gLtePreferenceController extends TelephonyTogglePreferenceController
        implements LifecycleObserver, OnStart, OnStop {

    private static final String DIALOG_TAG = "Enhanced4gLteDialog";

    private static final String TAG = "Enhanced4gLtePreferenceController";

    private Preference mPreference;
    private PreferenceScreen mPreferenceScreen;
    private TelephonyManager mTelephonyManager;
    private CarrierConfigManager mCarrierConfigManager;
    private PersistableBundle mCarrierConfig;
    @VisibleForTesting
    ImsManager mImsManager;
    /// M: Phone state has been listened in MobileNetworkActivity.
    // private PhoneCallStateListener mPhoneStateListener;
    private final List<On4gLteUpdateListener> m4gLteListeners;
    private final CharSequence[] mVariantTitles;
    private final CharSequence[] mVariantSumaries;
    private IntentFilter mIntentFilter;
    private final int VARIANT_TITLE_VOLTE = 0;
    private final int VARIANT_TITLE_ADVANCED_CALL = 1;
    private final int VARIANT_TITLE_4G_CALLING = 2;

    /// M: Add for updating the state.
    private DataContentObserver mDataContentObserver;

    /// M: Add for supporting CTVoLTE. @{
    private FragmentManager mFragmentManager;
    private boolean mNeedToast;
    private boolean mNeedDialog;
    /// @}

    /// M: Add for handling state changed. @{
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(TAG, "onReceive, action=" + action);

            if (TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED.equals(action)) {
                String propKey = intent.getStringExtra(MtkSubscriptionManager.INTENT_KEY_PROP_KEY);
                if (SubscriptionManager.ENHANCED_4G_MODE_ENABLED.equals(propKey)) {
                    if (mPreference != null) {
                        updateState(mPreference);
                    }
                    for (final On4gLteUpdateListener lsn : m4gLteListeners) {
                        lsn.on4gLteUpdated();
                    }
                }
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
            /// M: update ImsManager when ims service restart. @{
            } else if (ImsManager.ACTION_IMS_SERVICE_UP.equals(intent.getAction())) {
                int phoneId = SubscriptionManager.getPhoneId(mSubId);
                if (SubscriptionManager.isValidPhoneId(phoneId)) {
                    mImsManager = ImsManager.getInstance(mContext, phoneId);
                } else {
                    mImsManager = null;
                }
                Log.d(TAG, "onReceive, update ImsManager=" + (mImsManager == null
                        ? "null" : mImsManager) + ", preference="
                        + (mPreference == null ? "null" : mPreference.getTitle()));
                if (mImsManager != null && mPreference != null) {
                    updateState(mPreference);
                }
            /// @}
            }
        }
    };
    /// @}
    public Enhanced4gLtePreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = context.getSystemService(CarrierConfigManager.class);
        m4gLteListeners = new ArrayList<>();
        /// M: Phone state has been listened in MobileNetworkActivity.
        // mPhoneStateListener = new PhoneCallStateListener(Looper.getMainLooper());
        mVariantTitles = context.getResources()
                .getTextArray(R.array.enhanced_4g_lte_mode_title_variant);
        mVariantSumaries = context.getResources()
                .getTextArray(R.array.enhanced_4g_lte_mode_sumary_variant);
        /// M: Add for updating the state.
        mDataContentObserver = new DataContentObserver(new Handler(Looper.getMainLooper()));
        /// M: Add for handling state changed. @{
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        /// M: for china mobile feature, only cmcc card can show volte item start.
        mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        /// M: update ImsManager when ims service restart.
        mIntentFilter.addAction(ImsManager.ACTION_IMS_SERVICE_UP);
        /// @}
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        /// M: China mobile feature: VOLTE item only show in CMCC card
        /// or version project remove, or TMO remove @{
        boolean customizeVolteState
                = UtilsExt.getMobileNetworkSettingsExt(mContext).customizeVolteState(subId);
        if (customizeVolteState) {
            return CONDITIONALLY_UNAVAILABLE;
        }
        /// @}
        /// M: Add for check vice CARD. @{
        boolean multiIms = MtkImsManager.isSupportMims();
        if (!multiIms) {
            int mainPhoneId = TelephonyUtils.getMainCapabilityPhoneId();
            if (SubscriptionManager.getPhoneId(subId) != mainPhoneId) {
                return CONDITIONALLY_UNAVAILABLE;
            }
        }
        /// @}

        /// M: Add for checking CT SIM card for non-C2K project. @{
        if (!TelephonyUtils.isMtkC2kSupported() && MobileNetworkUtils.isCtSim(mPhoneId)) {
            return CONDITIONALLY_UNAVAILABLE;
        }
        /// @}

        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(subId);
        /// M: Add for log. @{
        final boolean isVolteEnabled = (mImsManager == null
                ? false : mImsManager.isVolteEnabledByPlatform());
        final boolean isVolteProvisioned = (mImsManager == null
                ? false : mImsManager.isVolteProvisionedOnDevice());
        final boolean isImsServiceReady =
                MobileNetworkUtils.isImsServiceStateReady(mImsManager);
        final boolean hideEnhanced4gLte = (carrierConfig == null
                ? false : carrierConfig.getBoolean(
                        CarrierConfigManager.KEY_HIDE_ENHANCED_4G_LTE_BOOL));
        boolean is45GpointVisible = (carrierConfig == null
                ? false : carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_SHOW_45G_OPTIONS));

        final boolean isVisible = subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID
                && isVolteEnabled && isVolteProvisioned
                && isImsServiceReady && !hideEnhanced4gLte
                && !is45GpointVisible;
        if (!isVisible) {
            Log.d(TAG, "getAvailabilityStatus, subId=" + subId
                    + ", isVisible=" + isVisible
                    + ", isVolteEnabled=" + isVolteEnabled
                    + ", isVolteProvisioned=" + isVolteProvisioned
                    + ", isImsServiceReady=" + isImsServiceReady
                    + ", hideEnhanced4gLte=" + hideEnhanced4gLte
                    + ", is45GpointVisible=" + is45GpointVisible);
        }
        /// @}
        return isVisible
                ? (is4gLtePrefEnabled() ? AVAILABLE : AVAILABLE_UNSEARCHABLE)
                : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mPreferenceScreen = screen;
        mPreference = screen.findPreference(getPreferenceKey());
        UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizaEnhanced4gLTEPreference(mPreference, mSubId);
    }

    @Override
    public void onStart() {
        /// M: Phone state has been listened in MobileNetworkActivity.
        // mPhoneStateListener.register(mSubId);
        /// M: Add for updating the state.
        mDataContentObserver.register(mContext, mSubId);
        mContext.registerReceiver(mReceiver, mIntentFilter);
    }

    @Override
    public void onStop() {
        /// M: Phone state has been listened in MobileNetworkActivity.
        // mPhoneStateListener.unregister();
        /// M: Add for updating the state.
        mDataContentObserver.unRegister(mContext);
        mContext.unregisterReceiver(mReceiver);
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        final SwitchPreference switchPreference = (SwitchPreference) preference;
        /// M: Revise for supporting CTVoLTE. @{
        if (MobileNetworkUtils.isCtSim(mPhoneId)) {
            switchPreference.setTitle(R.string.enhanced_4g_lte_mode_ct_title);
            switchPreference.setSummary(R.string.enhanced_4g_lte_mode_ct_summary);
        } else {
            final boolean show4GForLTE = (mCarrierConfig == null
                    ? false : mCarrierConfig.getBoolean(
                            CarrierConfigManager.KEY_SHOW_4G_FOR_LTE_DATA_ICON_BOOL));
            int variant4glteTitleIndex = (mCarrierConfig == null
                    ? VARIANT_TITLE_VOLTE : mCarrierConfig.getInt(
                            CarrierConfigManager.KEY_ENHANCED_4G_LTE_TITLE_VARIANT_INT));

            if (variant4glteTitleIndex != VARIANT_TITLE_ADVANCED_CALL) {
                variant4glteTitleIndex = show4GForLTE ? VARIANT_TITLE_4G_CALLING : VARIANT_TITLE_VOLTE;
            }

            switchPreference.setTitle(mVariantTitles[variant4glteTitleIndex]);
            switchPreference.setSummary(mVariantSumaries[variant4glteTitleIndex]);
        }
        /// @}
        UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizaEnhanced4gLTEPreference(switchPreference, mSubId);
        /// M: Add more condition for enabled state.
        switchPreference.setEnabled(is4gLtePrefEnabled()
                && MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId));
        /// M: Add for check whether the ImsManager is null or not.
        switchPreference.setChecked(mImsManager != null
                && mImsManager.isEnhanced4gLteModeSettingEnabledByUser()
                && mImsManager.isNonTtyOrTtyOnVolteEnabled());
    }

    @Override
    public boolean setChecked(boolean isChecked) {
        /// M: Add for supporting CTVoLTE. @{
        mNeedToast = isImsSwitching();
        if (mNeedToast) {
            return false;
        }

        mNeedDialog = isDialogNeeded();
        if (mNeedDialog) {
            return false;
        }
        /// @}

        /// M: Add for check whether the ImsManager is null or not. @{
        if (mImsManager == null) {
            return false;
        }
        /// @}

        mImsManager.setEnhanced4gLteModeSetting(isChecked);
        for (final On4gLteUpdateListener lsn : m4gLteListeners) {
            lsn.on4gLteUpdated();
        }
        return true;
    }

    @Override
    public boolean isChecked() {
        /// M: Add for check whether the ImsManager is null or not.
        return mImsManager == null ? false : mImsManager.isEnhanced4gLteModeSettingEnabledByUser();
    }

    /// M: Add for handling the click quickly case. @{
    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (getPreferenceKey().equals(preference.getKey())) {
            if (mNeedToast) {
                Toast.makeText(mContext, R.string.cannot_switch_enhanced_4g_lte_mode,
                        Toast.LENGTH_SHORT).show();
                return true;
            }

            if (mNeedDialog) {
                showVolteUnavailableDialog();
                return true;
            }

            return true;
        }

        return false;
    }
    /// @}

    /// M: Add for supporting CTVoLTE. @{
    public Enhanced4gLtePreferenceController init(FragmentManager fragmentManager, int subId) {
        mFragmentManager = fragmentManager;
        mSubId = subId;
        /// M: Add for recording the phone Id.
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        mCarrierConfig = mCarrierConfigManager.getConfigForSubId(mSubId);
        if (mSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            mImsManager = ImsManager.getInstance(mContext, SubscriptionManager.getPhoneId(mSubId));
        /// M: Add for check whether the ImsManager is null or not.
        } else {
            mImsManager = null;
        /// @}
        }

        return this;
    }
    /// @}

    /// M: Revise for supporting CTVoLTE. @{
    public Enhanced4gLtePreferenceController init(int subId) {
        return init(null, subId);
    }
    /// @}

    public Enhanced4gLtePreferenceController addListener(On4gLteUpdateListener lsn) {
        m4gLteListeners.add(lsn);
        return this;
    }

    private boolean is4gLtePrefEnabled() {
        /// M: Call state has been checked in other method.
        // final int callState = mTelephonyManager.getCallState(mSubId);
        /// M: Add for log. @{
        final boolean isNonTty = (mImsManager == null
                ? false : mImsManager.isNonTtyOrTtyOnVolteEnabled());
        final boolean isEditable = (mCarrierConfig == null
                ? false : mCarrierConfig.getBoolean(
                        CarrierConfigManager.KEY_EDITABLE_ENHANCED_4G_LTE_BOOL));
        final boolean enabled = (mSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID
                && isNonTty && isEditable);
        /// @}
        /// M: Add for supporting CTVoLTE.
        boolean enabledForCtVolte = isCtVoltePrefEnabled();

        if (!enabled) {
            Log.d(TAG, "is4gLtePrefEnabled, subId=" + mSubId + ", enabled=" + enabled
                    + ", enabledForCtVolte=" + enabledForCtVolte
                    + ", isNonTty=" + isNonTty + ", isEditable=" + isEditable);
        }

        return enabled && enabledForCtVolte;
    }

    private class PhoneCallStateListener extends PhoneStateListener {

        public PhoneCallStateListener(Looper looper) {
            super(looper);
        }

        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            Log.d(TAG, "onCallStateChanged, subId=" + mSubId + ", state=" + state);
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

    /**
     * Update other preferences when 4gLte state is changed
     */
    public interface On4gLteUpdateListener {
        void on4gLteUpdated();
    }

    /// M: Add for updating the state. @{
    private class DataContentObserver extends ContentObserver {

        public DataContentObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            Log.d(TAG, "onChange, selfChange=" + selfChange);
            updateState(mPreference);
        }

        public void register(Context context, int subId) {
            final Uri uri = Settings.Global.getUriFor(
                    Settings.Global.PREFERRED_NETWORK_MODE + subId);
            context.getContentResolver().registerContentObserver(uri, false, this);
        }

        public void unRegister(Context context) {
            context.getContentResolver().unregisterContentObserver(this);
        }
    }
    /// @}

    /// M: Add for supporting CTVoLTE. @{
    private boolean isCtVoltePrefEnabled() {
        boolean enabled = false;
        if (TelephonyUtils.isCtVolteEnabled() && MobileNetworkUtils.isCtSim(mPhoneId)) {
            if (MobileNetworkUtils.isCt4gSim(mPhoneId)) {
                if (TelephonyUtils.isCtAutoVolteEnabled()) {
                    enabled = true;
                } else {
                    int networkType = Settings.Global.getInt(mContext.getContentResolver(),
                            Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                            Phone.PREFERRED_NT_MODE);
                    switch (networkType) {
                        case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO:
                        case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_LTE_ONLY:
                        case TelephonyManager.NETWORK_MODE_LTE_WCDMA:
                        case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA:
                        case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM:
                        case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_WCDMA:
                        case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_CDMA_EVDO:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_WCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_GSM:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_WCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_GSM_WCDMA:
                        case TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                            enabled = true;
                            break;

                        default:
                            break;
                    }
                }
            }
        } else {
            enabled = true;
        }

        return enabled;
    }

    private boolean isDialogNeeded() {
        if (TelephonyUtils.isCtVolteEnabled() && MobileNetworkUtils.isCtSim(mPhoneId)
                && !isChecked()) {
            int networkType = mTelephonyManager.getNetworkType(mSubId);
            if (networkType != TelephonyManager.NETWORK_TYPE_LTE
                    && !mTelephonyManager.isNetworkRoaming(mSubId)) {
                if (!TelephonyUtils.isCtAutoVolteEnabled()) {
                    if (mFragmentManager != null) {
                        Log.d(TAG, "isDialogNeeded, return true, networkType=" + networkType);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private void showVolteUnavailableDialog() {
        final Enhanced4gLteDialogFragment dialogFragment =
                Enhanced4gLteDialogFragment.newInstance(
                        Enhanced4gLteDialogFragment.TYPE_VOLTE_UNAVAILABLE_DIALOG, mSubId);
        dialogFragment.show(mFragmentManager, DIALOG_TAG);
    }
    /// @}

    /// M: Add for checking IMS switching state. @{
    private boolean isImsSwitching() {
        try {
            int state = MtkImsManagerEx.getInstance().getImsState(
                    SubscriptionManager.getPhoneId(mSubId));
            Log.d(TAG, "isImsSwitching, state=" + state);
            switch (state) {
                case MtkPhoneConstants.IMS_STATE_ENABLING:
                case MtkPhoneConstants.IMS_STATE_DISABLING:
                    return true;

                default:
                    return false;
            }
        } catch (ImsException e) {
            Log.e(TAG, "isImsSwitching, e=" + e);
            return false;
        }
    }
    /// @}
}
