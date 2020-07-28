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

import static android.provider.Telephony.Carriers.ENFORCE_MANAGED_URI;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
/// M: Add for checking service state.
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.euicc.EuiccManager;
import android.telephony.ims.feature.ImsFeature;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;

import androidx.annotation.VisibleForTesting;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.util.ArrayUtils;
import com.android.settings.R;
import com.android.settings.Utils;
import com.android.settings.core.BasePreferenceController;
import com.android.settingslib.graph.SignalDrawable;

/// M: Add for printing the CTA logs. @{
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
/// @}
/// M: Add for recording the phone/SIM states.
import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
/// M: Add for supporting CTVoLTE.
import com.mediatek.settings.sim.TelephonyUtils;
/// M: Add for recording the phone/SIM states.
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.Arrays;
import java.util.List;

public class MobileNetworkUtils {

    private static final String TAG = "MobileNetworkUtils";

    // CID of the device.
    private static final String KEY_CID = "ro.boot.cid";
    // CIDs of devices which should not show anything related to eSIM.
    private static final String KEY_ESIM_CID_IGNORE = "ro.setupwizard.esim_cid_ignore";
    // System Property which is used to decide whether the default eSIM UI will be shown,
    // the default value is false.
    private static final String KEY_ENABLE_ESIM_UI_BY_DEFAULT =
            "esim.enable_esim_system_ui_by_default";
    private static final String LEGACY_ACTION_CONFIGURE_PHONE_ACCOUNT =
            "android.telecom.action.CONNECTION_SERVICE_CONFIGURE";

    // The following constants are used to draw signal icon.
    public static final int NO_CELL_DATA_TYPE_ICON = 0;
    public static final Drawable EMPTY_DRAWABLE = new ColorDrawable(Color.TRANSPARENT);

    /**
     * Returns if DPC APNs are enforced.
     */
    public static boolean isDpcApnEnforced(Context context) {
        try (Cursor enforceCursor = context.getContentResolver().query(ENFORCE_MANAGED_URI,
                null, null, null, null)) {
            if (enforceCursor == null || enforceCursor.getCount() != 1) {
                return false;
            }
            enforceCursor.moveToFirst();
            return enforceCursor.getInt(0) > 0;
        }
    }

    /**
     * Returns true if Wifi calling is enabled for at least one subscription.
     */
    public static boolean isWifiCallingEnabled(Context context) {
        SubscriptionManager subManager = context.getSystemService(SubscriptionManager.class);
        if (subManager == null) {
            Log.e(TAG, "isWifiCallingEnabled: couldn't get system service.");
            return false;
        }
        for (int subId : subManager.getActiveSubscriptionIdList()) {
            if (isWifiCallingEnabled(context, subId)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns true if Wifi calling is enabled for the specific subscription with id {@code subId}.
     */
    public static boolean isWifiCallingEnabled(Context context, int subId) {
        final PhoneAccountHandle simCallManager =
                TelecomManager.from(context).getSimCallManagerForSubscription(subId);
        final int phoneId = SubscriptionManager.getSlotIndex(subId);

        boolean isWifiCallingEnabled;
        if (simCallManager != null) {
            Intent intent = buildPhoneAccountConfigureIntent(
                    context, simCallManager);

            isWifiCallingEnabled = intent != null;
        } else {
            ImsManager imsMgr = ImsManager.getInstance(context, phoneId);
            isWifiCallingEnabled = imsMgr != null
                    && imsMgr.isWfcEnabledByPlatform()
                    && imsMgr.isWfcProvisionedOnDevice()
                    && isImsServiceStateReady(imsMgr);
        }

        return isWifiCallingEnabled;
    }

    @VisibleForTesting
    static Intent buildPhoneAccountConfigureIntent(
            Context context, PhoneAccountHandle accountHandle) {
        Intent intent = buildConfigureIntent(
                context, accountHandle, TelecomManager.ACTION_CONFIGURE_PHONE_ACCOUNT);

        if (intent == null) {
            // If the new configuration didn't work, try the old configuration intent.
            intent = buildConfigureIntent(context, accountHandle,
                    LEGACY_ACTION_CONFIGURE_PHONE_ACCOUNT);
        }
        return intent;
    }

    private static Intent buildConfigureIntent(
            Context context, PhoneAccountHandle accountHandle, String actionStr) {
        if (accountHandle == null || accountHandle.getComponentName() == null
                || TextUtils.isEmpty(accountHandle.getComponentName().getPackageName())) {
            return null;
        }

        // Build the settings intent.
        Intent intent = new Intent(actionStr);
        intent.setPackage(accountHandle.getComponentName().getPackageName());
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.putExtra(TelecomManager.EXTRA_PHONE_ACCOUNT_HANDLE, accountHandle);

        // Check to see that the phone account package can handle the setting intent.
        PackageManager pm = context.getPackageManager();
        List<ResolveInfo> resolutions = pm.queryIntentActivities(intent, 0);
        if (resolutions.size() == 0) {
            intent = null;  // set no intent if the package cannot handle it.
        }

        return intent;
    }

    public static boolean isImsServiceStateReady(ImsManager imsMgr) {
        boolean isImsServiceStateReady = false;

        try {
            if (imsMgr != null && imsMgr.getImsServiceState() == ImsFeature.STATE_READY) {
                isImsServiceStateReady = true;
            }
        } catch (ImsException ex) {
            Log.e(TAG, "Exception when trying to get ImsServiceStatus: " + ex);
        }

        Log.d(TAG, "isImsServiceStateReady=" + isImsServiceStateReady);
        return isImsServiceStateReady;
    }

    /**
     * Whether to show the entry point to eUICC settings.
     *
     * <p>We show the entry point on any device which supports eUICC as long as either the eUICC
     * was ever provisioned (that is, at least one profile was ever downloaded onto it), or if
     * the user has enabled development mode.
     */
    public static boolean showEuiccSettings(Context context) {
        EuiccManager euiccManager =
                (EuiccManager) context.getSystemService(EuiccManager.class);
        if (!euiccManager.isEnabled()) {
            return false;
        }

        final ContentResolver cr = context.getContentResolver();

        TelephonyManager tm =
                (TelephonyManager) context.getSystemService(TelephonyManager.class);
        String currentCountry = tm.getNetworkCountryIso().toLowerCase();
        String supportedCountries =
                Settings.Global.getString(cr, Settings.Global.EUICC_SUPPORTED_COUNTRIES);
        boolean inEsimSupportedCountries = false;
        if (TextUtils.isEmpty(currentCountry)) {
            inEsimSupportedCountries = true;
        } else if (!TextUtils.isEmpty(supportedCountries)) {
            List<String> supportedCountryList =
                    Arrays.asList(TextUtils.split(supportedCountries.toLowerCase(), ","));
            if (supportedCountryList.contains(currentCountry)) {
                inEsimSupportedCountries = true;
            }
        }
        final boolean esimIgnoredDevice =
                Arrays.asList(TextUtils.split(SystemProperties.get(KEY_ESIM_CID_IGNORE, ""), ","))
                        .contains(SystemProperties.get(KEY_CID, null));
        final boolean enabledEsimUiByDefault =
                SystemProperties.getBoolean(KEY_ENABLE_ESIM_UI_BY_DEFAULT, true);
        final boolean euiccProvisioned =
                Settings.Global.getInt(cr, Settings.Global.EUICC_PROVISIONED, 0) != 0;
        final boolean inDeveloperMode =
                Settings.Global.getInt(cr, Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0) != 0;

        return (inDeveloperMode || euiccProvisioned
                || (!esimIgnoredDevice && enabledEsimUiByDefault && inEsimSupportedCountries));
    }

    /**
     * Set whether to enable data for {@code subId}, also whether to disable data for other
     * subscription
     */
    public static void setMobileDataEnabled(Context context, int subId, boolean enabled,
            boolean disableOtherSubscriptions) {
        final TelephonyManager telephonyManager = context.getSystemService(TelephonyManager.class)
                .createForSubscriptionId(subId);
        final SubscriptionManager subscriptionManager = context.getSystemService(
                SubscriptionManager.class);
        telephonyManager.setDataEnabled(enabled);

        /// M: Add for printing the CTA logs. @{
        if (enabled) {
            CtaManager ctaManager = CtaManagerFactory.getInstance().makeCtaManager();
            ctaManager.printCtaInfor(context,
                    CtaManager.KeywordType.NETWORKCONNECT,
                    "MobileNetworkUtils.setMobileDataEnabled",
                    CtaManager.ActionType.ENABLE_MOBILE_NETWORKCONNECT,
                    "mobiledata");
        }
        /// @}

        if (disableOtherSubscriptions) {
            List<SubscriptionInfo> subInfoList =
                    subscriptionManager.getActiveSubscriptionInfoList(true);
            if (subInfoList != null) {
                for (SubscriptionInfo subInfo : subInfoList) {
                    // We never disable mobile data for opportunistic subscriptions.
                    if (subInfo.getSubscriptionId() != subId && !subInfo.isOpportunistic()) {
                        context.getSystemService(TelephonyManager.class).createForSubscriptionId(
                                subInfo.getSubscriptionId()).setDataEnabled(false);
                    }
                }
            }
        }
    }

    /**
     * Return {@code true} if show CDMA category
     */
    public static boolean isCdmaOptions(Context context, int subId) {
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            return false;
        }
        final TelephonyManager telephonyManager = context.getSystemService(TelephonyManager.class)
                .createForSubscriptionId(subId);
        final PersistableBundle carrierConfig = context.getSystemService(
                CarrierConfigManager.class).getConfigForSubId(subId);


        if (telephonyManager.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            return true;
        } else if (carrierConfig != null
                && !carrierConfig.getBoolean(
                CarrierConfigManager.KEY_HIDE_CARRIER_NETWORK_SETTINGS_BOOL)
                && carrierConfig.getBoolean(CarrierConfigManager.KEY_WORLD_PHONE_BOOL)) {
            return true;
        }

        if (isWorldMode(context, subId)) {
            final int settingsNetworkMode = android.provider.Settings.Global.getInt(
                    context.getContentResolver(),
                    android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId,
                    Phone.PREFERRED_NT_MODE);
            if (settingsNetworkMode == TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA
                    || settingsNetworkMode == TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO) {
                return true;
            }

            if (shouldSpeciallyUpdateGsmCdma(context, subId)) {
                return true;
            }
        }

        /// M: Add for supporting CTVoLTE. @{
        boolean showCdmaOption = shouldShowCdmaOption(context, subId);
        if (showCdmaOption) {
            return true;
        }
        /// @}

        return false;
    }

    /**
     * return {@code true} if we need show Gsm related settings
     */
    public static boolean isGsmOptions(Context context, int subId) {
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            return false;
        }
        if (isGsmBasicOptions(context, subId)) {
            return true;
        }
        final int networkMode = android.provider.Settings.Global.getInt(
                context.getContentResolver(),
                android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId,
                Phone.PREFERRED_NT_MODE);
        if (isWorldMode(context, subId)) {
            if (networkMode == TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO
                    || networkMode == TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA) {
                return true;
            } else if (shouldSpeciallyUpdateGsmCdma(context, subId)) {
                return true;
            }
        }

        return false;
    }

    private static boolean isGsmBasicOptions(Context context, int subId) {
        final TelephonyManager telephonyManager = context.getSystemService(TelephonyManager.class)
                .createForSubscriptionId(subId);
        final PersistableBundle carrierConfig = context.getSystemService(
                CarrierConfigManager.class).getConfigForSubId(subId);

        if (telephonyManager.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
            return true;
        } else if (carrierConfig != null
                && !carrierConfig.getBoolean(
                CarrierConfigManager.KEY_HIDE_CARRIER_NETWORK_SETTINGS_BOOL)
                && carrierConfig.getBoolean(CarrierConfigManager.KEY_WORLD_PHONE_BOOL)) {
            return true;
        }

        return false;
    }

    /**
     * Return {@code true} if it is world mode, and we may show advanced options in telephony
     * settings
     */
    public static boolean isWorldMode(Context context, int subId) {
        final PersistableBundle carrierConfig = context.getSystemService(
                CarrierConfigManager.class).getConfigForSubId(subId);
        return carrierConfig == null
                ? false
                : carrierConfig.getBoolean(CarrierConfigManager.KEY_WORLD_MODE_ENABLED_BOOL);
    }

    /**
     * Return {@code true} if we need show settings for network selection(i.e. Verizon)
     */
    public static boolean shouldDisplayNetworkSelectOptions(Context context, int subId) {
        final TelephonyManager telephonyManager = TelephonyManager.from(context)
                .createForSubscriptionId(subId);
        final PersistableBundle carrierConfig = context.getSystemService(
                CarrierConfigManager.class).getConfigForSubId(subId);
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID
                || carrierConfig == null
                || !carrierConfig.getBoolean(
                CarrierConfigManager.KEY_OPERATOR_SELECTION_EXPAND_BOOL)
                || carrierConfig.getBoolean(
                CarrierConfigManager.KEY_HIDE_CARRIER_NETWORK_SETTINGS_BOOL)
                || (carrierConfig.getBoolean(CarrierConfigManager.KEY_CSP_ENABLED_BOOL)
                && !telephonyManager.isManualNetworkSelectionAllowed())) {
            return false;
        }

        /// M: Add for supporting CTVoLTE. @{
        boolean showCdmaOption = shouldShowCdmaOption(context, subId);
        if (showCdmaOption) {
            return false;
        }
        /// @}

        final int networkMode = android.provider.Settings.Global.getInt(
                context.getContentResolver(),
                android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId,
                Phone.PREFERRED_NT_MODE);
        if (networkMode == TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO
                && isWorldMode(context, subId)) {
            return false;
        }
        if (shouldSpeciallyUpdateGsmCdma(context, subId)) {
            return false;
        }

        if (isGsmBasicOptions(context, subId)) {
            return true;
        }

        if (isWorldMode(context, subId)) {
            if (networkMode == TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA) {
                return true;
            }
        }

        return false;
    }

    /**
     * Return {@code true} if Tdscdma is supported in current subscription
     */
    public static boolean isTdscdmaSupported(Context context, int subId) {
        return isTdscdmaSupported(context,
                context.getSystemService(TelephonyManager.class).createForSubscriptionId(subId),
                subId);
    }

    //TODO(b/117651939): move it to telephony
    private static boolean isTdscdmaSupported(Context context,
            TelephonyManager telephonyManager, int subId) {
        final PersistableBundle carrierConfig = context.getSystemService(
                CarrierConfigManager.class).getConfigForSubId(subId);

        if (carrierConfig == null) {
            return false;
        }

        if (carrierConfig.getBoolean(CarrierConfigManager.KEY_SUPPORT_TDSCDMA_BOOL)) {
            return true;
        }

        /// M: The service state may be null. @{
        ServiceState serviceState = telephonyManager.getServiceState();
        if (serviceState == null) {
            return false;
        }
        String operatorNumeric = serviceState.getOperatorNumeric();
        /// @}

        String[] numericArray = carrierConfig.getStringArray(
                CarrierConfigManager.KEY_SUPPORT_TDSCDMA_ROAMING_NETWORKS_STRING_ARRAY);
        if (numericArray == null || operatorNumeric == null) {
            return false;
        }
        for (String numeric : numericArray) {
            if (operatorNumeric.equals(numeric)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Return subId that supported by search. If there are more than one, return first one,
     * otherwise return {@link SubscriptionManager#INVALID_SUBSCRIPTION_ID}
     */
    public static int getSearchableSubscriptionId(Context context) {
        final SubscriptionManager subscriptionManager = context.getSystemService(
                SubscriptionManager.class);
        final int subIds[] = subscriptionManager.getActiveSubscriptionIdList();

        return subIds.length >= 1 ? subIds[0] : SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }

    /**
     * Return availability for a default subscription id. If subId already been set, use it to
     * check, otherwise traverse all active subIds on device to check.
     * @param context context
     * @param defSubId Default subId get from telephony preference controller
     * @param callback Callback to check availability for a specific subId
     * @return Availability
     *
     * @see BasePreferenceController#getAvailabilityStatus()
     */
    public static int getAvailability(Context context, int defSubId,
            TelephonyAvailabilityCallback callback) {
        final SubscriptionManager subscriptionManager = context.getSystemService(
                SubscriptionManager.class);
        if (defSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            // If subId has been set, return the corresponding status
            return callback.getAvailabilityStatus(defSubId);
        } else {
            // Otherwise, search whether there is one subId in device that support this preference
            final int[] subIds = subscriptionManager.getActiveSubscriptionIdList();
            if (ArrayUtils.isEmpty(subIds)) {
                return callback.getAvailabilityStatus(
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            } else {
                for (final int subId : subIds) {
                    final int status = callback.getAvailabilityStatus(subId);
                    if (status == BasePreferenceController.AVAILABLE) {
                        return status;
                    }
                }
                return callback.getAvailabilityStatus(subIds[0]);
            }
        }
    }

    /**
     * This method is migrated from {@link com.android.phone.MobileNetworkSettings} and we should
     * use it carefully. This code snippet doesn't have very clear meaning however we should
     * update GSM or CDMA differently based on what it returns.
     *
     * 1. For all CDMA settings, make them visible if it return {@code true}
     * 2. For GSM settings, make them visible if it return {@code true} unless 3
     * 3. For network select settings, make it invisible if it return {@code true}
     */
    @VisibleForTesting
    static boolean shouldSpeciallyUpdateGsmCdma(Context context, int subId) {
        final int networkMode = android.provider.Settings.Global.getInt(
                context.getContentResolver(),
                android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId,
                Phone.PREFERRED_NT_MODE);
        if (networkMode == TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM
                || networkMode == TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM_WCDMA
                || networkMode == TelephonyManager.NETWORK_MODE_LTE_TDSCDMA
                || networkMode == TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_WCDMA
                || networkMode == TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA
                || networkMode == TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA) {
            if (!isTdscdmaSupported(context, subId) && isWorldMode(context, subId)) {
                return true;
            }
        }

        return false;
    }

    public static Drawable getSignalStrengthIcon(Context context, int level, int numLevels,
            int iconType, boolean cutOut) {
        SignalDrawable signalDrawable = new SignalDrawable(context);
        signalDrawable.setLevel(
                SignalDrawable.getState(level, numLevels, cutOut));

        // Make the network type drawable
        Drawable networkDrawable =
                iconType == NO_CELL_DATA_TYPE_ICON
                        ? EMPTY_DRAWABLE
                        : context
                                .getResources().getDrawable(iconType, context.getTheme());

        // Overlay the two drawables
        final Drawable[] layers = {networkDrawable, signalDrawable};
        final int iconSize =
                context.getResources().getDimensionPixelSize(R.dimen.signal_strength_icon_size);

        LayerDrawable icons = new LayerDrawable(layers);
        // Set the network type icon at the top left
        icons.setLayerGravity(0 /* index of networkDrawable */, Gravity.TOP | Gravity.LEFT);
        // Set the signal strength icon at the bottom right
        icons.setLayerGravity(1 /* index of SignalDrawable */, Gravity.BOTTOM | Gravity.RIGHT);
        icons.setLayerSize(1 /* index of SignalDrawable */, iconSize, iconSize);
        icons.setTintList(Utils.getColorAttr(context, android.R.attr.colorControlNormal));
        return icons;
    }

    /// M: Add for recording the phone/SIM states. @{
    private static final int sPhoneCount = TelephonyManager.getDefault().getPhoneCount();
    private static boolean sIsInCall = false;
    private static boolean[] sIsRadioOn = new boolean[sPhoneCount];
    private static boolean[] sIsSimOn = new boolean[sPhoneCount];
    private static CardType[] sCdmaCardType = new CardType[sPhoneCount];
    private static String[] sIccCardType = new String[sPhoneCount];

    public static void setInCallState(boolean isInCall) {
        sIsInCall = isInCall;
    }

    public static void setRadioOnState(boolean isRadioOn, int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            sIsRadioOn[phoneId] = isRadioOn;
        }
    }

    public static void setSimOnState(boolean isSimOn, int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            sIsSimOn[phoneId] = isSimOn;
        }
    }

    public static boolean shouldEnableMobileNetworkOption(int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            boolean inCall = sIsInCall;
            boolean radioOn = sIsRadioOn[phoneId];
            boolean simOn = sIsSimOn[phoneId];
            boolean enabled = !inCall && radioOn && simOn;
            if (!enabled) {
                Log.d(TAG, "shouldEnableMobileNetworkOption, phoneId=" + phoneId
                        + ", enabled=" + enabled + ", inCall=" + inCall
                        + ", radioOn=" + radioOn + ", simOn=" + simOn);
            }
            return enabled;
        } else {
            return false;
        }
    }

    public static void updateIccCardType(int subId, int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            sIccCardType[phoneId] =
                    MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
            Log.d(TAG, "updateIccCardType, phoneId=" + phoneId + ", iccCardType="
                    + (sIccCardType[phoneId] == null ? "null" : sIccCardType[phoneId]));
        }
    }

    public static boolean isUsim(int subId, int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            if (TextUtils.isEmpty(sIccCardType[phoneId])) {
                updateIccCardType(subId, phoneId);
            }

            boolean result = "USIM".equals(sIccCardType[phoneId]);
            return result;
        } else {
            return false;
        }
    }

    public static void updateCdmaCardType(int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            sCdmaCardType[phoneId] =
                    MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);
            Log.d(TAG, "updateSimCardType, phoneId=" + phoneId + ", cdmaCardType="
                    + (sCdmaCardType[phoneId] == null ? "null" : sCdmaCardType[phoneId]));
        }
    }

    public static boolean isCtSim(int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            if (sCdmaCardType[phoneId] == null
                    || sCdmaCardType[phoneId] == CardType.UNKNOW_CARD) {
                updateCdmaCardType(phoneId);
            }

            boolean result = false;
            if (sCdmaCardType[phoneId] != null) {
                switch (sCdmaCardType[phoneId]) {
                    case CT_3G_UIM_CARD:
                    case CT_UIM_SIM_CARD:
                    case CT_4G_UICC_CARD:
                    case CT_EXCEL_GG_CARD:
                        result = true;
                        break;

                    default:
                        break;
                }
            }
            return result;
        } else {
            return false;
        }
    }

    public static boolean isCt4gSim(int phoneId) {
        if (phoneId >= 0 && phoneId < sPhoneCount) {
            if (sCdmaCardType[phoneId] == null
                    || sCdmaCardType[phoneId] == CardType.UNKNOW_CARD) {
                updateCdmaCardType(phoneId);
            }

            boolean result = false;
            if (sCdmaCardType[phoneId] != null) {
                switch (sCdmaCardType[phoneId]) {
                    case CT_4G_UICC_CARD:
                        result = true;
                        break;

                    default:
                        break;
                }
            }
            return result;
        } else {
            return false;
        }
    }

    public static boolean isMultiCtSim(SubscriptionManager subscriptionManager) {
        int count = 0;
        List<SubscriptionInfo> subList =
                subscriptionManager.getActiveSubscriptionInfoList(true);

        if (subList == null || subList.size() < 2) {
            return false;
        }

        for (SubscriptionInfo info : subList) {
            int phoneId = info.getSimSlotIndex();
            if (isCtSim(phoneId)) {
                count++;
                if (count > 1) {
                    return true;
                }
            }
        }

        return false;
    }

    public static boolean shouldShowCdmaOption(Context context, int subId) {
        if (!TelephonyUtils.isC2kSupported()) {
            return false;
        }

        TelephonyManager telephonyManager =
                TelephonyManager.from(context).createForSubscriptionId(subId);
        SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
        int phoneId = SubscriptionManager.getPhoneId(subId);

        boolean ctVolteEnabled = TelephonyUtils.isCtVolteEnabled();
        if (ctVolteEnabled) {
            boolean ct4gSim = isCt4gSim(phoneId);
            if (ct4gSim) {
                boolean roaming = telephonyManager.isNetworkRoaming();
                if (!roaming) {
                    int mainPhoneId = TelephonyUtils.getMainCapabilityPhoneId();
                    if (phoneId == mainPhoneId) {
                        return true;
                    } else {
                        boolean multiCtSim = isMultiCtSim(subscriptionManager);
                        if (!multiCtSim) {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
    /// @}
}
