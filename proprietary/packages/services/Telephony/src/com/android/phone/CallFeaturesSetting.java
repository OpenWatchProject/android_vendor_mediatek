/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.phone;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.UserManager;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.util.Log;
import android.util.Pair;
import android.view.MenuItem;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.dataconnection.DataEnabledSettings;
import com.android.phone.PhoneGlobals.SubInfoUpdateListener;
import com.android.phone.settings.PhoneAccountSettingsFragment;
import com.android.phone.settings.SuppServicesUiUtil;
import com.android.phone.settings.VoicemailSettingsActivity;
import com.android.phone.settings.fdn.FdnSetting;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.phone.ext.DefaultCallFeaturesSettingExt;
import com.mediatek.phone.ext.ExtensionManager;
import com.mediatek.settings.CallSettingUtils;
import com.mediatek.settings.CallSettingUtils.DialogType;
/// M: Add for SIM On/Off feature. @{
import com.mediatek.settings.SimOnOffSwitchHandler;
import com.mediatek.settings.SimOnOffSwitchHandler.OnSimOnOffSwitchListener;
/// @}
import com.mediatek.settings.TelephonyUtils;
import com.mediatek.settings.cdma.CdmaCallForwardOptions;
import com.mediatek.settings.cdma.CdmaCallWaitOptions;
import com.mediatek.settings.cdma.CdmaCallWaitingUtOptions;
import com.mediatek.settings.cdma.TelephonyUtilsEx;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;

import mediatek.telephony.MtkCarrierConfigManager;

/**
 * Top level "Call settings" UI; see res/xml/call_feature_setting.xml
 *
 * This preference screen is the root of the "Call settings" hierarchy available from the Phone
 * app; the settings here let you control various features related to phone calls (including
 * voicemail settings, the "Respond via SMS" feature, and others.)  It's used only on
 * voice-capable phone devices.
 *
 * Note that this activity is part of the package com.android.phone, even
 * though you reach it from the "Phone" app (i.e. DialtactsActivity) which
 * is from the package com.android.contacts.
 *
 * For the "Mobile network settings" screen under the main Settings app,
 * See {@link com.android.settings.network.telephony.MobileNetworkActivity}.
 */
public class CallFeaturesSetting extends PreferenceActivity
        implements Preference.OnPreferenceChangeListener, SubInfoUpdateListener {
    private static final String LOG_TAG = "CallFeaturesSetting";
    private static final boolean DBG = true; //(PhoneGlobals.DBG_LEVEL >= 2);

    // String keys for preference lookup
    // TODO: Naming these "BUTTON_*" is confusing since they're not actually buttons(!)
    // TODO: Consider moving these strings to strings.xml, so that they are not duplicated here and
    // in the layout files. These strings need to be treated carefully; if the setting is
    // persistent, they are used as the key to store shared preferences and the name should not be
    // changed unless the settings are also migrated.
    private static final String VOICEMAIL_SETTING_SCREEN_PREF_KEY = "button_voicemail_category_key";
    private static final String BUTTON_FDN_KEY   = "button_fdn_key";
    private static final String BUTTON_RETRY_KEY       = "button_auto_retry_key";
    private static final String BUTTON_GSM_UMTS_OPTIONS = "button_gsm_more_expand_key";
    private static final String BUTTON_CDMA_OPTIONS = "button_cdma_more_expand_key";

    private static final String PHONE_ACCOUNT_SETTINGS_KEY =
            "phone_account_settings_preference_screen";

    private static final String ENABLE_VIDEO_CALLING_KEY = "button_enable_video_calling";

    private Phone mPhone;
    private ImsManager mImsMgr;
    private SubscriptionInfoHelper mSubscriptionInfoHelper;
    private TelecomManager mTelecomManager;

    private SwitchPreference mButtonAutoRetry;
    private PreferenceScreen mVoicemailSettingsScreen;
    private SwitchPreference mEnableVideoCalling;
    private Preference mButtonWifiCalling;

    /*
     * Click Listeners, handle click based on objects attached to UI.
     */

    // Click listener for all toggle events
    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference == mButtonAutoRetry) {
            android.provider.Settings.Global.putInt(mPhone.getContext().getContentResolver(),
                    android.provider.Settings.Global.CALL_AUTO_RETRY,
                    mButtonAutoRetry.isChecked() ? 1 : 0);
            return true;
        } else if (preference == preferenceScreen.findPreference(
                GsmUmtsCallOptions.CALL_FORWARDING_KEY)) {
            return doSsOverUtPrecautions(preference);
        } else if (preference == preferenceScreen.findPreference(
                GsmUmtsCallOptions.CALL_BARRING_KEY)) {
            return doSsOverUtPrecautions(preference);
        }
        /// M: Add for our inner features @{
        if (onPreferenceTreeClickMTK(preferenceScreen, preference)) {
            return true;
        }
        /// @}
        return false;
    }

    private boolean doSsOverUtPrecautions(Preference preference) {
        PersistableBundle b = null;
        if (mSubscriptionInfoHelper.hasSubId()) {
            b = PhoneGlobals.getInstance().getCarrierConfigForSubId(
                    mSubscriptionInfoHelper.getSubId());
        } else {
            b = PhoneGlobals.getInstance().getCarrierConfig();
        }

        String configKey;
        if (preference.getKey().equals(GsmUmtsCallOptions.CALL_FORWARDING_KEY)) {
            configKey = CarrierConfigManager.KEY_CALL_FORWARDING_OVER_UT_WARNING_BOOL;
        } else {
            configKey = CarrierConfigManager.KEY_CALL_BARRING_OVER_UT_WARNING_BOOL;
        }
        /// M: Add for data traffic dialog feature of dual volte @{
        if (b != null && b.getBoolean(configKey) && mPhone != null) {
            if (SuppServicesUiUtil.isSsOverUtPrecautions(this, mPhone)) {
                SuppServicesUiUtil.showBlockingSuppServicesDialog(this, mPhone,
                        preference.getKey()).show();
                return true;
            } else if (SuppServicesUiUtil.isSsOverUtPrecautionsRoaming(this, mPhone)) {
                CallSettingUtils.showDialogTips(this, mPhone.getSubId(),
                        DialogType.DATA_ROAMING, preference);
                return true;
            } else if (SuppServicesUiUtil.isSsOverUtPrecautionsExt(this, mPhone)) {
                CallSettingUtils.showDialogTips(this, mPhone.getSubId(),
                        DialogType.DATA_TRAFFIC, preference);
                return true;
            }
        /// @}
        }
        /// M: Debug info
        log("No Ut Precautions for: " + configKey);
        return false;
    }

    /**
     * Implemented to support onPreferenceChangeListener to look for preference
     * changes.
     *
     * @param preference is the preference to be changed
     * @param objValue should be the value of the selection, NOT its localized
     * display value.
     */
    @Override
    public boolean onPreferenceChange(Preference preference, Object objValue) {
        if (DBG) log("onPreferenceChange: \"" + preference + "\" changed to \"" + objValue + "\"");

        if (preference == mEnableVideoCalling) {
            if (mImsMgr.isEnhanced4gLteModeSettingEnabledByUser()) {
                mImsMgr.setVtSetting((boolean) objValue);
                /// M: For Plugin to get updated video Preference
                ExtensionManager.getCallFeaturesSettingExt()
                        .videoPreferenceChange((boolean) objValue);
            } else {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                DialogInterface.OnClickListener networkSettingsClickListener =
                        new Dialog.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                /// M: Start network setting in com.android.settings. @{
                                //startActivity(new Intent(mPhone.getContext(),
                                //        com.android.phone.MobileNetworkSettings.class));
                                Intent intent = new Intent(
                                        Settings.ACTION_NETWORK_OPERATOR_SETTINGS);
                                int subId = mPhone.getSubId();
                                log("start mobile network setting, subId=" + subId);
                                intent.putExtra(Settings.EXTRA_SUB_ID, subId);
                                startActivity(intent);
                                /// @}
                            }
                        };
                builder.setMessage(getResources().getString(
                                R.string.enable_video_calling_dialog_msg))
                        .setNeutralButton(getResources().getString(
                                R.string.enable_video_calling_dialog_settings),
                                networkSettingsClickListener)
                        .setPositiveButton(android.R.string.ok, null)
                        .show();
                return false;
            }
        }

        // Always let the preference setting proceed.
        return true;
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        if (DBG) log("onCreate: Intent is " + getIntent());

        // Make sure we are running as an admin user.
        if (!UserManager.get(this).isAdminUser()) {
            Toast.makeText(this, R.string.call_settings_admin_user_only,
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        mSubscriptionInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        mSubscriptionInfoHelper.setActionBarTitle(
                getActionBar(), getResources(), R.string.call_settings_with_label);
        mPhone = mSubscriptionInfoHelper.getPhone();
        mTelecomManager = TelecomManager.from(this);
        /// M: Add for MTK hotswap. @{
        if (mPhone == null) {
            log("onCreate: mPhone is null, finish!!!");
            finish();
            return;
        }
        /// @}
        /// M: Register related listeners & events.
        registerEventCallbacks();
    }

    private void updateImsManager(Phone phone) {
        log("updateImsManager :: phone.getContext()=" + phone.getContext()
                + " phone.getPhoneId()=" + phone.getPhoneId());
        mImsMgr = ImsManager.getInstance(phone.getContext(), phone.getPhoneId());
        if (mImsMgr == null) {
            log("updateImsManager :: Could not get ImsManager instance!");
        } else {
            log("updateImsManager :: mImsMgr=" + mImsMgr);
        }
    }

    private void listenPhoneState(boolean listen) {
        TelephonyManager telephonyManager =
                (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        telephonyManager.listen(mPhoneStateListener, listen
                ? PhoneStateListener.LISTEN_CALL_STATE : PhoneStateListener.LISTEN_NONE);
    }

    private final PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            if (DBG) log("PhoneStateListener onCallStateChanged: state is " + state);
            // Use TelecomManager#getCallStete instead of 'state' parameter because it needs
            // to check the current state of all phone calls.
            boolean isCallStateIdle =
                    mTelecomManager.getCallState() == TelephonyManager.CALL_STATE_IDLE;
            if (mEnableVideoCalling != null) {
                mEnableVideoCalling.setEnabled(isCallStateIdle);
            }
            if (mButtonWifiCalling != null) {
                mButtonWifiCalling.setEnabled(isCallStateIdle);
            }
        }
    };

    private final ProvisioningManager.Callback mProvisioningCallback =
            new ProvisioningManager.Callback() {
        @Override
        public void onProvisioningIntChanged(int item, int value) {
            if (item == ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED
                    || item == ImsConfig.ConfigConstants.VLT_SETTING_ENABLED
                    || item == ImsConfig.ConfigConstants.LVC_SETTING_ENABLED) {
                updateVtWfc();
            }
        }
    };

    @Override
    protected void onPause() {
        super.onPause();
        listenPhoneState(false);

        // Remove callback for provisioning changes.
        try {
            if (mImsMgr != null) {
                mImsMgr.getConfigInterface().removeConfigCallback(
                        mProvisioningCallback.getBinder());
            }
        } catch (ImsException e) {
            Log.w(LOG_TAG, "onPause: Unable to remove callback for provisioning changes");
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        updateImsManager(mPhone);
        listenPhoneState(true);
        PreferenceScreen preferenceScreen = getPreferenceScreen();
        if (preferenceScreen != null) {
            preferenceScreen.removeAll();
        }

        addPreferencesFromResource(R.xml.call_feature_setting);

        TelephonyManager telephonyManager = getSystemService(TelephonyManager.class)
                .createForSubscriptionId(mPhone.getSubId());

        // Note: The PhoneAccountSettingsActivity accessible via the
        // android.telecom.action.CHANGE_PHONE_ACCOUNTS intent is accessible directly from
        // the AOSP Dialer settings page on multi-sim devices.
        // Where a device does NOT make the PhoneAccountSettingsActivity directly accessible from
        // its Dialer app, this check must be modified in the device's AOSP branch to ensure that
        // the PhoneAccountSettingsActivity is always accessible.
        if (telephonyManager.isMultiSimEnabled()) {
            Preference phoneAccountSettingsPreference = findPreference(PHONE_ACCOUNT_SETTINGS_KEY);
            getPreferenceScreen().removePreference(phoneAccountSettingsPreference);
        }

        PreferenceScreen prefSet = getPreferenceScreen();
        mVoicemailSettingsScreen =
                (PreferenceScreen) findPreference(VOICEMAIL_SETTING_SCREEN_PREF_KEY);
        mVoicemailSettingsScreen.setIntent(mSubscriptionInfoHelper.getIntent(
                VoicemailSettingsActivity.class));

        maybeHideVoicemailSettings();

        mButtonAutoRetry = (SwitchPreference) findPreference(BUTTON_RETRY_KEY);

        mEnableVideoCalling = (SwitchPreference) findPreference(ENABLE_VIDEO_CALLING_KEY);
        mButtonWifiCalling = findPreference(getResources().getString(
                R.string.wifi_calling_settings_key));

        PersistableBundle carrierConfig =
                PhoneGlobals.getInstance().getCarrierConfigForSubId(mPhone.getSubId());

        if (carrierConfig.getBoolean(CarrierConfigManager.KEY_AUTO_RETRY_ENABLED_BOOL)) {
            mButtonAutoRetry.setOnPreferenceChangeListener(this);
            int autoretry = Settings.Global.getInt(
                    getContentResolver(), Settings.Global.CALL_AUTO_RETRY, 0);
            mButtonAutoRetry.setChecked(autoretry != 0);
        } else {
            prefSet.removePreference(mButtonAutoRetry);
            mButtonAutoRetry = null;
        }

        Preference cdmaOptions = prefSet.findPreference(BUTTON_CDMA_OPTIONS);
        Preference gsmOptions = prefSet.findPreference(BUTTON_GSM_UMTS_OPTIONS);
        Preference fdnButton = prefSet.findPreference(BUTTON_FDN_KEY);
        fdnButton.setIntent(mSubscriptionInfoHelper.getIntent(FdnSetting.class));
        if (carrierConfig.getBoolean(CarrierConfigManager.KEY_WORLD_PHONE_BOOL)) {
            cdmaOptions.setIntent(mSubscriptionInfoHelper.getIntent(CdmaCallOptions.class));
            gsmOptions.setIntent(mSubscriptionInfoHelper.getIntent(GsmUmtsCallOptions.class));
        } else {
            prefSet.removePreference(cdmaOptions);
            prefSet.removePreference(gsmOptions);

            int phoneType = mPhone.getPhoneType();
            if (carrierConfig.getBoolean(CarrierConfigManager.KEY_HIDE_CARRIER_NETWORK_SETTINGS_BOOL)) {
                prefSet.removePreference(fdnButton);
            } else {
                /// M: [CT VOLTE] or [SmartFren: Indonesia VOLTE card] @{
                boolean isUtPreferOnlyByCdmaSim = CallSettingUtils.
                        isUtPreferOnlyByCdmaSim(mPhone.getSubId(), mPhone);
                if (phoneType == PhoneConstants.PHONE_TYPE_CDMA || isUtPreferOnlyByCdmaSim) {
                /// @}
                    prefSet.removePreference(fdnButton);

                    if (!carrierConfig.getBoolean(
                            CarrierConfigManager.KEY_VOICE_PRIVACY_DISABLE_UI_BOOL)) {
                        addPreferencesFromResource(R.xml.cdma_call_privacy);
                        /// M: ALPS02087723 Set the right cdma phone instance
                        configCdmaVoicePrivacy(prefSet, isUtPreferOnlyByCdmaSim);
                    }
                    /// M: CDMA type phone call settings item @{
                    addPreferencesFromResource(R.xml.mtk_cdma_call_options);
                    configCdmaCallWaiting(prefSet, carrierConfig);
                    /// @}
                } else if (phoneType == PhoneConstants.PHONE_TYPE_GSM) {
                    if (mPhone.getIccCard() == null || !mPhone.getIccCard().getIccFdnAvailable()) {
                        prefSet.removePreference(fdnButton);
                    }
                    if (carrierConfig.getBoolean(
                            CarrierConfigManager.KEY_ADDITIONAL_CALL_SETTING_BOOL)) {
                        addPreferencesFromResource(R.xml.gsm_umts_call_options);
                        GsmUmtsCallOptions.init(prefSet, mSubscriptionInfoHelper);
                    }
                } else {
                    throw new IllegalStateException("Unexpected phone type: " + phoneType);
                }
            }
        }
        updateVtWfc();

        // Register callback for provisioning changes.
        try {
            if (mImsMgr != null) {
                mImsMgr.getConfigInterface().addConfigCallback(mProvisioningCallback);
            }
        } catch (ImsException e) {
            Log.w(LOG_TAG, "onResume: Unable to register callback for provisioning changes.");
        }
    }

    private void updateVtWfc() {
        PreferenceScreen prefSet = getPreferenceScreen();
        TelephonyManager telephonyManager = getSystemService(TelephonyManager.class)
                .createForSubscriptionId(mPhone.getSubId());
        PersistableBundle carrierConfig =
                PhoneGlobals.getInstance().getCarrierConfigForSubId(mPhone.getSubId());
        boolean editableWfcRoamingMode = true;
        boolean useWfcHomeModeForRoaming = false;
        if (carrierConfig != null) {
            editableWfcRoamingMode = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_EDITABLE_WFC_ROAMING_MODE_BOOL);
            useWfcHomeModeForRoaming = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_USE_WFC_HOME_NETWORK_MODE_IN_ROAMING_NETWORK_BOOL,
                    false);
        }
        /// M: IMS video call @{
        mVtEnabledByPlatform = mImsMgr.isVtEnabledByPlatform();
        mVtProvisionedOnDevice = mImsMgr.isVtProvisionedOnDevice();
        mIgnoreDataChanged = carrierConfig.getBoolean(
                CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS);
        boolean isDataEnabled =  mPhone.getDataEnabledSettings().isDataEnabled();
        log("isVtEnabledByPlatform:" + mVtEnabledByPlatform +
                   "\nisVtProvisionedOnDevice:" + mVtProvisionedOnDevice +
                   "\nisIgnoreDataChanged:" + mIgnoreDataChanged +
                   "\nisDataEnabled:" + isDataEnabled);
        if (mVtEnabledByPlatform && mVtProvisionedOnDevice &&
                     (mIgnoreDataChanged || isDataEnabled)) {
            /// M: ALPS04015392 Remove Vt preference if no main capability phone for single ims.
            if (MtkImsManager.isSupportMims() || TelephonyUtilsEx.isCapabilityPhone(mPhone)) {
                boolean isEnhanced4gEnabledByUser =
                    mImsMgr.isEnhanced4gLteModeSettingEnabledByUser();
                boolean currentValue = isEnhanced4gEnabledByUser
                        ? mImsMgr.isVtEnabledByUser() : false;
                log("isEnhanced4gLteModeSettingEnabledByUser:" + isEnhanced4gEnabledByUser +
                       "\nisVtEnabledByUser:" + currentValue);
                mEnableVideoCalling.setChecked(currentValue);
                mEnableVideoCalling.setOnPreferenceChangeListener(this);
                prefSet.addPreference(mEnableVideoCalling);
            } else {
                prefSet.removePreference(mEnableVideoCalling);
            }
        /// @}
        } else {
            prefSet.removePreference(mEnableVideoCalling);
        }

        final PhoneAccountHandle simCallManager = mTelecomManager.getSimCallManagerForSubscription(
                mPhone.getSubId());
        if (simCallManager != null) {
            Intent intent = PhoneAccountSettingsFragment.buildPhoneAccountConfigureIntent(
                    this, simCallManager);
            /// M: Debug info @{
            log("simCallManager is not null");
            /// @}
            if (intent != null) {
                PackageManager pm = mPhone.getContext().getPackageManager();
                List<ResolveInfo> resolutions = pm.queryIntentActivities(intent, 0);
                if (!resolutions.isEmpty()) {
                    /// M: Debug info @{
                    log("set wfc by: " + resolutions.get(0).loadLabel(pm));
                    /// @}
                    mButtonWifiCalling.setTitle(resolutions.get(0).loadLabel(pm));
                    mButtonWifiCalling.setSummary(null);
                    mButtonWifiCalling.setIntent(intent);
                    prefSet.addPreference(mButtonWifiCalling);
                } else {
                    /// M: Debug info @{
                    log("Remove WFC Preference since resolutions is empty");
                    /// @}
                    prefSet.removePreference(mButtonWifiCalling);
                }
            } else {
                /// M: Debug info @{
                log("Remove WFC Preference since PhoneAccountConfigureIntent is null");
                /// @}
                prefSet.removePreference(mButtonWifiCalling);
            }
        } else if (!mImsMgr.isWfcEnabledByPlatform() || !mImsMgr.isWfcProvisionedOnDevice()) {
            /// M: Debug info @{
            log("Remove WFC Preference since wfc is not enabled on the device.");
            /// @}
            prefSet.removePreference(mButtonWifiCalling);
        } else {
            String title = SubscriptionManager.getResourcesForSubId(mPhone.getContext(),
                    mPhone.getSubId()).getString(R.string.wifi_calling);
            mButtonWifiCalling.setTitle(title);

            int resId = com.android.internal.R.string.wifi_calling_off_summary;
            if (mImsMgr.isWfcEnabledByUser()) {
                boolean isRoaming = telephonyManager.isNetworkRoaming();
                boolean wfcRoamingEnabled = editableWfcRoamingMode && !useWfcHomeModeForRoaming;
                int wfcMode = mImsMgr.getWfcMode(isRoaming && wfcRoamingEnabled);
                switch (wfcMode) {
                    case ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY:
                        resId = com.android.internal.R.string.wfc_mode_wifi_only_summary;
                        break;
                    case ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED:
                        resId = com.android.internal.R.string.wfc_mode_cellular_preferred_summary;
                        break;
                    case ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED:
                        resId = com.android.internal.R.string.wfc_mode_wifi_preferred_summary;
                        break;
                    default:
                        if (DBG) log("Unexpected WFC mode value: " + wfcMode);
                }
            }
            /// M: Remove WFC preference mode by operator @{
            boolean removeWfcPrefMode = carrierConfig.getBoolean(
                    MtkCarrierConfigManager.MTK_KEY_WFC_REMOVE_PREFERENCE_MODE_BOOL);
            log("removeWfcPrefMode:" + removeWfcPrefMode);
            if (removeWfcPrefMode) {
                mButtonWifiCalling.setSummary(null);
            } else {
                mButtonWifiCalling.setSummary(resId);
                Intent intent = mButtonWifiCalling.getIntent();
                if (intent != null) {
                    intent.putExtra(Settings.EXTRA_SUB_ID, mPhone.getSubId());
                }
            }
            /// @}
            prefSet.addPreference(mButtonWifiCalling);
        }

        try {
            if (mImsMgr.getImsServiceState() != ImsFeature.STATE_READY) {
                log("Feature state not ready so remove vt and wfc settings for "
                        + " phone =" + mPhone.getPhoneId());
                prefSet.removePreference(mButtonWifiCalling);
                prefSet.removePreference(mEnableVideoCalling);
            }
        } catch (ImsException ex) {
            log("Exception when trying to get ImsServiceStatus: " + ex);
            prefSet.removePreference(mButtonWifiCalling);
            prefSet.removePreference(mEnableVideoCalling);
        }

        /// M: update screen status
        updateScreenStatus();

        /// M: WFC @{
        ExtensionManager.getCallFeaturesSettingExt().initOtherCallFeaturesSetting(this, mPhone);
        ExtensionManager.getCallFeaturesSettingExt()
                .onCallFeatureSettingsEvent(DefaultCallFeaturesSettingExt.RESUME);
        /// @}
    }

    /**
     * Hides the top level voicemail settings entry point if the default dialer contains a
     * particular manifest metadata key. This is required when the default dialer wants to display
     * its own version of voicemail settings.
     */
    private void maybeHideVoicemailSettings() {
        String defaultDialer = getSystemService(TelecomManager.class).getDefaultDialerPackage();
        if (defaultDialer == null) {
            return;
        }
        try {
            Bundle metadata = getPackageManager()
                    .getApplicationInfo(defaultDialer, PackageManager.GET_META_DATA).metaData;
            if (metadata == null) {
                return;
            }
            if (!metadata
                    .getBoolean(TelephonyManager.METADATA_HIDE_VOICEMAIL_SETTINGS_MENU, false)) {
                if (DBG) {
                    log("maybeHideVoicemailSettings(): not disabled by default dialer");
                }
                return;
            }
            getPreferenceScreen().removePreference(mVoicemailSettingsScreen);
            if (DBG) {
                log("maybeHideVoicemailSettings(): disabled by default dialer");
            }
        } catch (NameNotFoundException e) {
            // do nothing
            if (DBG) {
                log("maybeHideVoicemailSettings(): not controlled by default dialer");
            }
        }
    }

    @Override
    protected void onNewIntent(Intent newIntent) {
        setIntent(newIntent);

        mSubscriptionInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        mSubscriptionInfoHelper.setActionBarTitle(
                getActionBar(), getResources(), R.string.call_settings_with_label);
        mPhone = mSubscriptionInfoHelper.getPhone();
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        if (itemId == android.R.id.home) {  // See ActionBar#setDisplayHomeAsUpEnabled()
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /**
     * Finish current Activity and go up to the top level Settings ({@link CallFeaturesSetting}).
     * This is useful for implementing "HomeAsUp" capability for second-level Settings.
     */
    public static void goUpToTopLevelSetting(
            Activity activity, SubscriptionInfoHelper subscriptionInfoHelper) {
        Intent intent = subscriptionInfoHelper.getIntent(CallFeaturesSetting.class);
        intent.setAction(Intent.ACTION_MAIN);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        activity.startActivity(intent);
        activity.finish();
    }

    // -------------------------------MTK-----------------------------
    /// M: GSM type phone call settings item --> call forward & call wait & caller id & call barring
    private static final String CALL_FORWARDING_KEY = "call_forwarding_key";
    private static final String CALL_BARRING_KEY = "call_barring_key";
    private static final String ADDITIONAL_GSM_SETTINGS_KEY = "additional_gsm_call_settings_key";

    /// M: CDMA type phone call settings item --> call forward & call wait & caller id
    private static final String KEY_CALL_FORWARD = "button_cf_expand_key";
    private static final String KEY_CALL_WAIT = "button_cw_key";
    private static final String KEY_CALLER_ID = "button_caller_id"; //SmartFren card type support

    /// M: Add for call private voice feature
    private static final String BUTTON_CP_KEY = "button_voice_privacy_key";

    /// M: Add for ims state
    private int mImsRegState = ServiceState.STATE_POWER_OFF;
    private MyHandler mHandler = new MyHandler();

    /// M: Add for SIM On/Off feature. @{
    private boolean mSimOnOffEnabled;
    private SimOnOffSwitchHandler mSimOnOffSwitchHandler = null;
    /// @}

    /// M: Add for vilte SS feature. @{
    private boolean mVtEnabledByPlatform;
    private boolean mVtProvisionedOnDevice;
    private boolean mIgnoreDataChanged;
    /// @}

    /**
     * Listen to the IMS service state change.
     *
     */
    private final ImsMmTelManager.RegistrationCallback mImsRegistrationCallback =
            new ImsMmTelManager.RegistrationCallback() {

                @Override
                public void onRegistered(
                        @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
                    log("onImsConnected imsRadioTech=" + imsRadioTech +
                            " mImsRegState=" + mImsRegState);
                    if (mImsRegState != ServiceState.STATE_IN_SERVICE) {
                        mImsRegState = ServiceState.STATE_IN_SERVICE;
                        Message msg = mHandler.obtainMessage(
                                MyHandler.MESSAGE_IMS_STATE_CHANGED);
                        mHandler.sendMessage(msg);
                    }
                }

                @Override
                public void onUnregistered(ImsReasonInfo imsReasonInfo) {
                    log("onImsDisconnected imsReasonInfo=" + imsReasonInfo +
                            " mImsRegState=" + mImsRegState);
                    if (mImsRegState != ServiceState.STATE_OUT_OF_SERVICE) {
                        mImsRegState = ServiceState.STATE_OUT_OF_SERVICE;
                        Message msg = mHandler.obtainMessage(
                                MyHandler.MESSAGE_IMS_STATE_CHANGED);
                        mHandler.sendMessage(msg);
                    }
                }
            };

    @Override
    protected void onDestroy() {
        if (UserManager.get(this).isAdminUser()) {
            /// M: WFC @{
            ExtensionManager.getCallFeaturesSettingExt()
                    .onCallFeatureSettingsEvent(DefaultCallFeaturesSettingExt.DESTROY);
            /// @}
            unregisterEventCallbacks();
        }
        super.onDestroy();
    }

    @Override
    public void handleSubInfoUpdate() {
        finish();
    }

    /**
     * For internal features.
     * @param preferenceScreen
     * @param preference
     * @return
     */
    private boolean onPreferenceTreeClickMTK(
            PreferenceScreen preferenceScreen, Preference preference) {
        log("onPreferenceTreeClickMTK: " + preference.getKey());
        int subId = mPhone.getSubId();
        Intent intent;

        /// M: Add for data traffic dialog feature of dual volte @{
        PersistableBundle carrierConfig =
                PhoneGlobals.getInstance().getCarrierConfigForSubId(mPhone.getSubId());
        if (preference == preferenceScreen.findPreference(ADDITIONAL_GSM_SETTINGS_KEY)) {
            boolean isCLIROverUtPrecautions = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_CALLER_ID_OVER_UT_WARNING_BOOL);
            boolean isCWOverUtPrecautions = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_CALL_WAITING_OVER_UT_WARNING_BOOL);
            if ((isCLIROverUtPrecautions || isCWOverUtPrecautions)) {
                if (SuppServicesUiUtil.isSsOverUtPrecautionsRoaming(this, mPhone)) {
                    CallSettingUtils.showDialogTips(this, subId,
                            DialogType.DATA_ROAMING, preference);
                    return true;
                } else if (SuppServicesUiUtil.isSsOverUtPrecautionsExt(this, mPhone)) {
                    CallSettingUtils.showDialogTips(this, mPhone.getSubId(),
                            DialogType.DATA_TRAFFIC, preference);
                    return true;
                }
            }
        /// @}
        /// M: CDMA type phone call setting item click handling @{
        } else if (preference == preferenceScreen.findPreference(KEY_CALL_FORWARD)) {
            /// M: [CT VOLTE] @{
            boolean isCFOverUtPrecautions = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_CALL_FORWARDING_OVER_UT_WARNING_BOOL);
            if (isCFOverUtPrecautions && CallSettingUtils.isCtVolte4gSim(subId)) {
                if (SuppServicesUiUtil.isSsOverUtPrecautions(this, mPhone)) {
                    SuppServicesUiUtil.showBlockingSuppServicesDialog(this, mPhone,
                            preference.getKey()).show();
                } else if (SuppServicesUiUtil.isSsOverUtPrecautionsRoaming(this, mPhone)) {
                    CallSettingUtils.showDialogTips(this, mPhone.getSubId(),
                            DialogType.DATA_ROAMING, preference);
                } else if (SuppServicesUiUtil.isSsOverUtPrecautionsExt(this, mPhone)) {
                    CallSettingUtils.showDialogTips(this, mPhone.getSubId(),
                            DialogType.DATA_TRAFFIC, preference);
                } else {
                    intent = mSubscriptionInfoHelper.getIntent(GsmUmtsCallForwardOptions.class);
                    startActivity(intent);
                }
            } else {
            /// @}
                intent = mSubscriptionInfoHelper.getIntent(CdmaCallForwardOptions.class);
                startActivity(intent);
            }
            return true;
        } else if (preference == preferenceScreen.findPreference(KEY_CALL_WAIT)) {
            boolean isImsOn = TelephonyUtils.isImsServiceAvailable(this, subId);
            boolean isCWOverUtPrecautions = carrierConfig.getBoolean(
                    CarrierConfigManager.KEY_CALL_WAITING_OVER_UT_WARNING_BOOL);
            if (!isImsOn) {
                // If not register Ims, it is cs dial domain.
                // show cdma call waiting dialog directly.
                showDialog(CdmaCallWaitOptions.CW_MODIFY_DIALOG);
            } else if (isCWOverUtPrecautions && CallSettingUtils.isCtVolte4gSim(subId)) {
                if (SuppServicesUiUtil.isSsOverUtPrecautions(this, mPhone)) {
                    SuppServicesUiUtil.showBlockingSuppServicesDialog(this, mPhone,
                            preference.getKey()).show();
                } else if (SuppServicesUiUtil.isSsOverUtPrecautionsRoaming(this, mPhone)) {
                    CallSettingUtils.showDialogTips(this, mPhone.getSubId(),
                            DialogType.DATA_ROAMING, preference);
                } else if (SuppServicesUiUtil.isSsOverUtPrecautionsExt(this, mPhone)) {
                    CallSettingUtils.showDialogTips(this, mPhone.getSubId(),
                            DialogType.DATA_TRAFFIC, preference);
                } else {
                    intent = mSubscriptionInfoHelper.getIntent(CdmaCallWaitingUtOptions.class);
                    startActivity(intent);
                }
            } else {
                showDialog(CdmaCallWaitOptions.CW_MODIFY_DIALOG);
            }
            return true;
        }
        /// @}

        return false;
    }

    private void updateScreenStatus() {
        PreferenceScreen pres = getPreferenceScreen();
        if (pres == null) {
            log("updateScreenStatus, screen is null");
            return;
        }

        int subId = mPhone.getSubId();
        boolean hasSubId = SubscriptionManager.isValidSubscriptionId(subId);
        boolean isAirplaneModeEnabled = TelephonyUtils.isAirplaneModeOn(
                PhoneGlobals.getInstance());
        log("updateScreenStatus, isAirplaneMode=" + isAirplaneModeEnabled +
                ", hasSubId=" + hasSubId);

        /// M: Add for SIM On/Off feature. @{
        boolean isSimOn = true;
        if (mSimOnOffEnabled) {
            int slotId = SubscriptionManager.getSlotIndex(subId);
            isSimOn = (MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId)
                    == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON);
            log("updateScreenStatus, slotId=" + slotId + ", isSimOn=" + isSimOn);
        }
        /// @}

        for (int i = 0; i < pres.getPreferenceCount(); i++) {
            Preference pref = pres.getPreference(i);
            pref.setEnabled(!isAirplaneModeEnabled && hasSubId && isSimOn);
        }

        /// M: The CF UI will be disabled when air plane mode is on.
        /// but SS should be still workable when IMS is registered,
        /// So Enable the CF UI when IMS is registered. @{
        if (hasSubId) {
            boolean isImsOn = false;
            if (mImsRegState != ServiceState.STATE_POWER_OFF) {
                isImsOn = (mImsRegState == ServiceState.STATE_IN_SERVICE);
            } else {
                isImsOn = TelephonyUtils.isImsServiceAvailable(this, subId);
            }
            if (isImsOn && (MtkImsManager.isSupportMims()
                    || TelephonyUtilsEx.isCapabilityPhone(mPhone))) {
                Preference prefCf = getPreferenceScreen().findPreference(CALL_FORWARDING_KEY);
                Preference prefCb = getPreferenceScreen().findPreference(CALL_BARRING_KEY);
                Preference prefCw = getPreferenceScreen().findPreference(ADDITIONAL_GSM_SETTINGS_KEY);
                if (prefCf != null) {
                    log(" --- set SS item enabled when IMS is registered ---");
                    prefCf.setEnabled(true);
                }
                if (prefCb != null) {
                    prefCb.setEnabled(true);
                }
                if (prefCw != null) {
                    prefCw.setEnabled(true);
                }
                /// M: ALPS03457899 [SmartFren] ut prefer cdma sim by ims on. @{
                Preference prefCdmaCf = getPreferenceScreen().findPreference(KEY_CALL_FORWARD);
                Preference prefCdmaCw = getPreferenceScreen().findPreference(KEY_CALL_WAIT);
                Preference prefCdmaCi = getPreferenceScreen().findPreference(KEY_CALLER_ID);
                if (prefCdmaCf != null) {
                    log(" --- set CDMA SS item enabled when IMS is registered ---");
                    prefCdmaCf.setEnabled(true);
                }
                if (prefCdmaCw != null) {
                    prefCdmaCw.setEnabled(true);
                }
                if (prefCdmaCi != null) {
                    prefCdmaCi.setEnabled(true);
                }
                /// @}
            }
        }
        /// @}
        updateOptionsByCallState(hasSubId);
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("onReceive, action = " + action);
            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                updateScreenStatus();
            }
        }
    };

    // dialog creation method, called by showDialog()
    @Override
    protected Dialog onCreateDialog(int dialogId) {
        /// M: CDMA call waiting option.
        if (dialogId == CdmaCallWaitOptions.CW_MODIFY_DIALOG) {
            return new CdmaCallWaitOptions(this, mPhone).createDialog();
        }
        return null;
    }

    /**
     * 1. Listen sim hot swap related change.
     * 2. ACTION_AIRPLANE_MODE_CHANGED
     * 3. Call Status for VT item
     */
    private void registerEventCallbacks() {
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
        /// M: register airplane mode
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mReceiver, intentFilter);
        try {
            ImsManager imsManager = ImsManager.getInstance(mPhone.getContext(),
                    mPhone.getPhoneId());
            if (imsManager != null) {
                imsManager.addRegistrationCallback(mImsRegistrationCallback);
                log("addRegistrationCallback:" + mImsRegistrationCallback);
            }
        } catch (ImsException e) {
            log("ImsException:" + e);
        }
        mPhone.getDefaultPhone().getDataEnabledSettings().registerForDataEnabledChanged(
                mHandler, MyHandler.MESSAGE_DATA_ENABLED_CHANGED, null);

        /// M: Add for SIM On/Off feature. @{
        mSimOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        if (mSimOnOffEnabled) {
            mSimOnOffSwitchHandler = new SimOnOffSwitchHandler(this, mPhone.getPhoneId());
            mSimOnOffSwitchHandler.registerOnSimOnOffSwitch(new OnSimOnOffSwitchListener() {
                @Override
                public void onSimOnOffStateChanged() {
                    log("onSimOnOffStateChanged, finish activity.");
                    finish();
                }
            });
        }
        /// @}
    }

    private void unregisterEventCallbacks() {
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
        unregisterReceiver(mReceiver);
        ImsManager imsManager = ImsManager.getInstance(mPhone.getContext(),
                mPhone.getPhoneId());
        if (imsManager != null) {
            imsManager.removeRegistrationListener(mImsRegistrationCallback);
            log("removeRegistrationListener:" + mImsRegistrationCallback);
        }
        mPhone.getDefaultPhone().getDataEnabledSettings().unregisterForDataEnabledChanged(
                mHandler);

        /// M: Add for SIM On/Off feature. @{
        if (mSimOnOffSwitchHandler != null) {
            mSimOnOffSwitchHandler.unregisterOnSimOnOffSwitch();
            mSimOnOffSwitchHandler = null;
        }
        /// @}
    }

    /**
     * Handler Class to call features setting.
     */
    private class MyHandler extends Handler {
        static final int MESSAGE_IMS_STATE_CHANGED = 0;
        static final int MESSAGE_DATA_ENABLED_CHANGED = 1;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_IMS_STATE_CHANGED:
                    updateScreenStatus();
                    break;
                case MESSAGE_DATA_ENABLED_CHANGED:
                    AsyncResult ar = (AsyncResult) msg.obj;
                    if (ar.result instanceof Pair) {
                        Pair<Boolean, Integer> p = (Pair<Boolean, Integer>) ar.result;
                        handleDataEnabledChanged(p.first, p.second);
                    }
                    break;
                default:
                    break;
            }
        }

        private void handleDataEnabledChanged(boolean enabled, int reason) {
            log("handleDataEnabledChanged: enabled=" + enabled + ", reason=" + reason);
            if (mEnableVideoCalling != null && reason != DataEnabledSettings.REASON_REGISTERED) {
                PreferenceScreen prefSet = getPreferenceScreen();
                if (mVtEnabledByPlatform && mVtProvisionedOnDevice &&
                     (mIgnoreDataChanged || enabled)) {
                    log("handleDataEnabledChanged: addPreference");
                    prefSet.addPreference(mEnableVideoCalling);
                } else {
                    log("handleDataEnabledChanged: removePreference");
                    prefSet.removePreference(mEnableVideoCalling);
                }
            }
        }
    }

    /**
     * This is for VT and WFC option, when during call, disable it.
     */
    private void updateOptionsByCallState(boolean hasSubId) {
        if (mEnableVideoCalling != null) {
            boolean isInCall = TelephonyUtils.isInCall(this);
            mEnableVideoCalling.setEnabled(hasSubId && !isInCall);
            mButtonWifiCalling.setEnabled(hasSubId && !isInCall);
        }
    }

    private void configCdmaVoicePrivacy(PreferenceScreen prefSet, boolean isRemoved) {
        CdmaVoicePrivacySwitchPreference ccp =
                (CdmaVoicePrivacySwitchPreference) findPreference(BUTTON_CP_KEY);
        if (ccp != null) {
            /// M: No voice privacy item for special operator
            if (isRemoved) {
                log("Voice privacy option removed");
                prefSet.removePreference(ccp);
            } else {
               ccp.setPhone(mPhone);
            }
        }
    }

    private void configCdmaCallerId(PreferenceScreen prefSet, boolean isUtPreferByCdmaSimAndImsOn) {
        if (!isUtPreferByCdmaSimAndImsOn) {
            Preference callerIDPreference = prefSet.findPreference(KEY_CALLER_ID);
            prefSet.removePreference(callerIDPreference);
            log("No support by operator, so remove Caller ID pref for CDMA");
        }
    }

    private void configCdmaCallWaiting(PreferenceScreen prefSet, PersistableBundle carrierConfig) {
        String carrierName = carrierConfig.getString(CarrierConfigManager.KEY_CARRIER_NAME_STRING);
        if (carrierName != null && carrierName.equalsIgnoreCase("Sprint")) {
            Preference callWaiting = prefSet.findPreference(KEY_CALL_WAIT);
            prefSet.removePreference(callWaiting);
            log("No support by operator, so remove call waiting pref for CDMA");
        }
    }
}
