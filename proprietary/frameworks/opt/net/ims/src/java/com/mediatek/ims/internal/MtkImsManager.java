/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.ims.internal;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ImsMmTelManager.RegistrationCallback;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.text.TextUtils;
import android.util.Log;
import android.net.Uri;

import com.android.ims.ImsCall;
import com.android.ims.ImsConfig;
import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsUtInterface;
import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsEcbm;
import com.android.ims.internal.IImsEcbmListener;
import com.android.ims.internal.IImsMultiEndpoint;
import com.android.ims.internal.IImsRegistrationListener;
import com.android.ims.internal.IImsService;
import com.android.ims.internal.IImsUt;
import com.android.ims.internal.IImsConfig;
import com.android.internal.telephony.IPhoneSubInfo;

import com.mediatek.common.carrierexpress.CarrierExpressManager;
import com.mediatek.ims.internal.ext.IImsManagerExt;
import com.mediatek.ims.internal.ext.OpImsCustomizationUtils;

import com.mediatek.ims.MtkImsCall;
import com.mediatek.ims.MtkImsConnectionStateListener;
import com.mediatek.ims.MtkImsUt;
import com.mediatek.ims.MtkImsConstants;
import com.mediatek.ims.internal.IMtkImsConfig;
import com.mediatek.ims.internal.IMtkImsService;
import com.mediatek.ims.internal.IMtkImsUt;
import com.mediatek.internal.telephony.IMtkPhoneSubInfoEx;
import com.mediatek.ims.internal.MtkImsConfig;
import com.mediatek.ims.internal.IMtkImsRegistrationListener;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import com.android.ims.ImsEcbm;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import mediatek.telephony.MtkCarrierConfigManager;
import mediatek.telephony.MtkServiceState;

import android.annotation.ProductApi;

/**
 * Provides APIs for MTK IMS services or modified AOSP APIs.
 */
public class MtkImsManager extends ImsManager {

    /// M: IMS VoLTE refactoring. @{
    /**
     * Key to retrieve the sequence number from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_SEQ_NUM = "android:imsSeqNum";

    /*
     * Key to retrieve the sequence number from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_DIAL_STRING = "android:imsDialString";
    /// @}

    /*
     * Key to retrieve the sequence number from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_MT_TO_NUMBER = "mediatek:mtToNumber";

    /// M: IMS VoLTE refactoring. @{
    /**
     * Key to retrieve the call mode from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_CALL_MODE = "android:imsCallMode";

    /// M: For IMS deregister done notification. @{
    public static final String ACTION_IMS_SERVICE_DEREGISTERED =
            MtkImsConstants.ACTION_IMS_SERVICE_DEREGISTERED;
    /// @}

    /// M: For IMS RTP report notification. @{
    public static final String ACTION_IMS_RTP_INFO = "com.android.ims.IMS_RTP_INFO";
    /// @}

    /// M: For IMS radio change notification. @{
    public static final String ACTION_IMS_RADIO_STATE_CHANGED =
            "com.android.ims.IMS_RADIO_STATE_CHANGED";
    /// @}

    /// M: For IMS radio change notification. @{
    public static final String EXTRA_IMS_RADIO_STATE = "android:imsRadioState";
    /// @}

    /**
     * Action for the incoming call indication intent for the Phone app.
     * Internal use only.
     * @hide
     */
    public static final String ACTION_IMS_INCOMING_CALL_INDICATION =
            "com.android.ims.IMS_INCOMING_CALL_INDICATION";

    /**
     * Action for the not ringing incoming call indication intent for the Phone
     * app. Internal use only.
     * @hide
     */
    public static final String ACTION_IMS_NOT_RINGING_INCOMING_CALL =
            "com.mediatek.ims.NOT_RINGING_INCOMING_CALL";

    /* M WFC */
    public static final String EXTRA_IMS_REG_STATE_KEY = "android:regState";
    public static final String EXTRA_IMS_ENABLE_CAP_KEY = "android:enablecap";
    public static final String EXTRA_IMS_DISABLE_CAP_KEY = "android:disablecap";
    public static final String EXTRA_IMS_REG_ERROR_KEY = "android:regError";
    /* M WFC */

    /// M: For IMS RTP report notification. @{
    /**
     * Part of the ACTION_IMS_RTP_INFO intents.
     * @hide
     */
    public static final String EXTRA_RTP_PDN_ID = "android:rtpPdnId";
    public static final String EXTRA_RTP_NETWORK_ID = "android:rtpNetworkId";
    public static final String EXTRA_RTP_TIMER = "android:rtpTimer";
    public static final String EXTRA_RTP_SEND_PKT_LOST = "android:rtpSendPktLost";
    public static final String EXTRA_RTP_RECV_PKT_LOST = "android:rtpRecvPktLost";
    public static final String EXTRA_PHONE_ID = "android:phoneId";

    /// M: Telephony global configuration
    private static final String PROPERTY_IMS_SUPPORT = "persist.vendor.ims_support";
    private static final String MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    private static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";
    private static final String PROPERTY_MTK_VOLTE_SUPPORT = "persist.vendor.volte_support";
    private static final String PROPERTY_MTK_VILTE_SUPPORT = "persist.vendor.vilte_support";
    private static final String PROPERTY_MTK_WFC_SUPPORT = "persist.vendor.mtk_wfc_support";
    private static final String PROPERTY_DYNAMIC_IMS_SWITCH =
            "persist.vendor.mtk_dynamic_ims_switch";
    /// M: Add for CT VoLTE @{
    private static final String PROPERTY_CT_VOLTE_SUPPORT = "persist.vendor.mtk_ct_volte_support";
    private static final String PROPERTY_TEST_SIM1 = "vendor.gsm.sim.ril.testsim";
    private static final String PROPERTY_TEST_SIM2 = "vendor.gsm.sim.ril.testsim.2";
    private static final String PROPERTY_TEST_SIM3 = "vendor.gsm.sim.ril.testsim.3";
    private static final String PROPERTY_TEST_SIM4 = "vendor.gsm.sim.ril.testsim.4";
    private static final String PROPERTY_ICCID_SIM[] = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4"
    };
    /// Sync volte setting value.
    private static final String PROPERTY_IMSCONFIG_FORCE_NOTIFY =
            "vendor.ril.imsconfig.force.notify";
    /// @}

	// VzW request: Combo setting for VoLTE and ViLTE
    private static final String PROPERTY_VOLTE_ENALBE = "persist.vendor.mtk.volte.enable";
    private static final String PROPERTY_WFC_ENALBE = "persist.vendor.mtk.wfc.enable";
    private static final String PROPERTY_VILTE_ENALBE = "persist.vendor.mtk.vilte.enable";
    private static final String PROPERTY_VIWIFI_ENALBE = "persist.vendor.mtk.viwifi.enable";

    /// M: MIMS related
    // Ims user setting keys
    private static final String VOLTE_SETTING = "volte_setting";
    private static final String TTY_MODE = "tty_mode";
    private static final String VILTE_SETTING = "vilte_setting";
    private static final String WFC_SETTING = "wfc_setting";
    private static final String WFC_MODE_SETTING = "wfc_mode_setting";
    private static final String WFC_ROAMING_MODE_SETTING = "wfc_roaming_mode_setting";
    private static final String WFC_ROAMING_SETTING = "wfc_roaming_setting";

    // SIM ID mapping
    protected static final int SIM_ID_1 = 0;
    protected static final int SIM_ID_2 = 1;
    protected static final int SIM_ID_3 = 2;
    protected static final int SIM_ID_4 = 3;

    // MIMS Enhanced 4G Settings Key
    public static final String ENHANCED_4G_MODE_ENABLED_SIM2 = "volte_vt_enabled_sim2";
    public static final String ENHANCED_4G_MODE_ENABLED_SIM3 = "volte_vt_enabled_sim3";
    public static final String ENHANCED_4G_MODE_ENABLED_SIM4 = "volte_vt_enabled_sim4";
    // MIMS VT Settings Key
    public static final String VT_IMS_ENABLED_SIM2 = "vt_ims_enabled_sim2";
    public static final String VT_IMS_ENABLED_SIM3 = "vt_ims_enabled_sim3";
    public static final String VT_IMS_ENABLED_SIM4 = "vt_ims_enabled_sim4";
    // MIMS data enable property
    public static final String DATA_ENABLED_PROP = "net.lte.ims.data.enabled";
    // MIMS data roaming property
    public static final String DATA_ROAMING_PROP = "net.lte.data.roaming";
    public static final String DATA_ROAMING_SETTING_PROP = "net.lte.data.roaming.setting";
    // MIMS WFC Settings Key
    public static final String WFC_IMS_ENABLED_SIM2 = "wfc_ims_enabled_sim2";
    public static final String WFC_IMS_ENABLED_SIM3 = "wfc_ims_enabled_sim3";
    public static final String WFC_IMS_ENABLED_SIM4 = "wfc_ims_enabled_sim4";
    public static final String WFC_IMS_MODE_SIM2 = "wfc_ims_mode_sim2";
    public static final String WFC_IMS_MODE_SIM3 = "wfc_ims_mode_sim3";
    public static final String WFC_IMS_MODE_SIM4 = "wfc_ims_mode_sim4";
    public static final String WFC_IMS_ROAMING_ENABLED_SIM2 = "wfc_ims_roaming_enabled_sim2";
    public static final String WFC_IMS_ROAMING_ENABLED_SIM3 = "wfc_ims_roaming_enabled_sim3";
    public static final String WFC_IMS_ROAMING_ENABLED_SIM4 = "wfc_ims_roaming_enabled_sim4";
    public static final String WFC_IMS_ROAMING_MODE_SIM2 = "wfc_ims_roaming_mode_sim2";
    public static final String WFC_IMS_ROAMING_MODE_SIM3 = "wfc_ims_roaming_mode_sim3";
    public static final String WFC_IMS_ROAMING_MODE_SIM4 = "wfc_ims_roaming_mode_sim4";
    // MIMS TTY Settings Key
    public static final String PREFERRED_TTY_MODE = "preferred_tty_mode";
    public static final String PREFERRED_TTY_MODE_SIM2 = "preferred_tty_mode_sim2";
    public static final String PREFERRED_TTY_MODE_SIM3 = "preferred_tty_mode_sim3";
    public static final String PREFERRED_TTY_MODE_SIM4 = "preferred_tty_mode_sim4";

    /**
     * M: Define IMS service support emergency event, extend by AOSP
     * registrationServiceCapabilityChanged event parameter comment
     */
    public static final int SERVICE_REG_CAPABILITY_EVENT_REMOVED = 0;
    public static final int SERVICE_REG_CAPABILITY_EVENT_ADDED = 1;
    public static final int SERVICE_REG_CAPABILITY_EVENT_ECC_SUPPORT = 2;
    public static final int SERVICE_REG_CAPABILITY_EVENT_ECC_NOT_SUPPORT = 4;

    /** @}*/
    /**
     * For notify UI wifi PDN Out Of Service State
     */
    public static final int SERVICE_REG_EVENT_WIFI_PDN_OOS_START            = 5;
    public static final int SERVICE_REG_EVENT_WIFI_PDN_OOS_END_WITH_DISCONN = 6;
    public static final int SERVICE_REG_EVENT_WIFI_PDN_OOS_END_WITH_RESUME  = 7;
    public static final int OOS_END_WITH_DISCONN = 0;
    public static final int OOS_START = 1;
    public static final int OOS_END_WITH_RESUME = 2; // default

    public static final int IMS_REGISTERING = MtkImsConstants.IMS_REGISTERING;
    public static final int IMS_REGISTERED = MtkImsConstants.IMS_REGISTERED;
    public static final int IMS_REGISTER_FAIL = MtkImsConstants.IMS_REGISTER_FAIL;

    /**
     * For accessing the Mediatek IMS related service. Internal use only.
     * @hide
     */
    public static final String MTK_IMS_SERVICE = "mtkIms";

    private static final String TAG = "MtkImsManager";
    private static final boolean DBG = true;
    private IMtkImsService mMtkImsService = null;
    private MtkImsServiceDeathRecipient mMtkDeathRecipient = new MtkImsServiceDeathRecipient();

    private MtkImsUt mMtkUt = null;

    private ArrayList<ImsMmTelManager.RegistrationCallback> mCallbacks
            = new ArrayList<ImsMmTelManager.RegistrationCallback>();

    ///M: RTT broadcast incoming RTT call indication
    public static final String EXTRA_RTT_INCOMING_CALL = "rtt_feature:rtt_incoming_call";

    ///M: IMSI switch feature for OP18
    private static final boolean mSupportImsiSwitch =
                        SystemProperties.get("ro.vendor.mtk_imsi_switch_support", "0").equals("1");

    @ProductApi
    public static boolean isSupportMims() {
        return (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) > 1);
    }

    public static int getMainPhoneIdForSingleIms(Context context) {
        // duplication from RadioCapabilitySwitchUtil
        int phoneId = 0;
        phoneId = SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        log("[getMainPhoneIdForSingleIms] : " + phoneId);
        return phoneId;
    }

    @ProductApi
    @Override
    public boolean isEnhanced4gLteModeSettingEnabledByUser() {
        int sub = getSubId();
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(
                sub, SubscriptionManager.ENHANCED_4G_MODE_ENABLED,
                SUB_PROPERTY_NOT_INITIALIZED, mContext);
        boolean onByDefault = getBooleanCarrierConfig(
                CarrierConfigManager.KEY_ENHANCED_4G_LTE_ON_BY_DEFAULT_BOOL);
        boolean isPSsupport = isPhoneIdSupportIms(mPhoneId);

        // If sim not ready yet, default off
        if (sub == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            TelephonyManager manager = (TelephonyManager) mContext.getSystemService(
                    Context.TELEPHONY_SERVICE);
            int cardState = manager.getSimState(mPhoneId);
            if (cardState != TelephonyManager.SIM_STATE_ABSENT) {
                onByDefault = false;
            } else {
                log("isEnhanced4gLteModeSettingEnabledByUser, sim absent");
            }
        }
        // If Enhanced 4G LTE Mode is uneditable or not initialized, we use the default value
        if (!getBooleanCarrierConfig(CarrierConfigManager.KEY_EDITABLE_ENHANCED_4G_LTE_BOOL)
                || getBooleanCarrierConfig(CarrierConfigManager.KEY_HIDE_ENHANCED_4G_LTE_BOOL)
                || setting == SUB_PROPERTY_NOT_INITIALIZED) {
            if (onByDefault) {
                // For CT 3G SIM, shall be off,
                // carrier config volte value is not consist to CT request.
                String iccid = SystemProperties.get(PROPERTY_ICCID_SIM[mPhoneId],
                        "N/A");
                if (!TextUtils.isEmpty(iccid) && isOp09SimCard(iccid)) {
                    MtkIccCardConstants.CardType cardType =
                        MtkTelephonyManagerEx.getDefault().getCdmaCardType(mPhoneId);
                    if (cardType != null && !cardType.is4GCard()) {
                        log("isEnhanced4gLteModeSettingEnabledByUser, CT 3G card case");
                        onByDefault = false;
                    }
                }
            }
            return onByDefault;
        } else {
            return (setting == ImsConfig.FeatureValueConstants.ON) && isPSsupport;
        }
    }

    protected boolean shouldForceUpdated() {
        return (SystemProperties.getInt(PROPERTY_IMSCONFIG_FORCE_NOTIFY, 0) != 0);
    }

    /**
     * @param context
     * @param phoneId
     * @return
     */
    @ProductApi
    public static boolean isNonTtyOrTtyOnVolteEnabled(Context context,
            int phoneId) {
        if (!isSupportMims()) {
            phoneId = getMainPhoneIdForSingleIms(context);
        }
        return getAppropriateManagerForPlugin(context, phoneId)
                .isNonTtyOrTtyOnVolteEnabled();
    }

    /**
     * Returns a platform configuration for VoLTE which may override the user
     * setting on a per Slot basis.
     */
    @Override
    public boolean isVolteEnabledByPlatform() {
        // We first read the per slot value. If doesn't exist, we read the
        // general value. If still
        // doesn't exist, we use the hardcoded default value.
        if (SystemProperties.getInt(
                PROPERTY_DBG_VOLTE_AVAIL_OVERRIDE + Integer.toString(mPhoneId),
                SYSTEM_PROPERTY_NOT_SET) == 1
                || SystemProperties.getInt(PROPERTY_DBG_VOLTE_AVAIL_OVERRIDE,
                        SYSTEM_PROPERTY_NOT_SET) == 1) {
            return true;
        }

        // Dynamic IMS Switch
        boolean isResourceSupport = true;
        boolean isCarrierConfigSupport = true;
        boolean isGbaValidSupport = true;
        boolean isFeatureEnableByPlatformExt = true;
        boolean isPSsupport = true;

        isResourceSupport = isImsResourceSupport(
                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE);
        isCarrierConfigSupport = getBooleanCarrierConfig(
                CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
        isGbaValidSupport = isGbaValid();
        isFeatureEnableByPlatformExt = isFeatureEnabledByPlatformExt(
                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE);
        isPSsupport = isPhoneIdSupportIms(mPhoneId);

        log("Volte, isResourceSupport:" + isResourceSupport
                + ", isCarrierConfigSupport:" + isCarrierConfigSupport
                + ", isGbaValidSupport:" + isGbaValidSupport
                + ", isFeatureEnableByPlatformExt:"
                + isFeatureEnableByPlatformExt + ", isPSsupport:" + isPSsupport);

        return (SystemProperties.getInt(PROPERTY_MTK_VOLTE_SUPPORT, 0) == 1)
                && (isLteSupported())
                && isResourceSupport && isCarrierConfigSupport
                && isGbaValidSupport && isFeatureEnableByPlatformExt
                && isPSsupport;
    }

    /**
     * Change VoLTE Preference setting (AT+CEVDP)
     */
    public void setVoltePreferSetting(int mode) {
        try {
            MtkImsConfig config = getConfigInterfaceEx();
            if (config != null) {
                config.setVoltePreference(mode);
            }
        } catch (ImsException e) {
            loge("setVoltePreferSetting(): " + e);
        }
    }


    /**
     * Change persistent WFC enabled setting for slot.
     */
    public void setWfcSetting(boolean enabled) {
        if (enabled && !isWfcProvisionedOnDevice()) {
            log("setWfcSetting: Not possible to enable WFC due to provisioning.");
            return;
        }

        int subId = getSubId();
        if (isSubIdValid(subId)) {
            SubscriptionManager.setSubscriptionProperty(subId, SubscriptionManager.WFC_IMS_ENABLED,
                    booleanToPropertyString(enabled));
        } else {
            loge("setWfcSetting: invalid sub id, can not set WFC setting in siminfo db; subId="
                    + subId);
        }

        TelephonyManager tm = (TelephonyManager)
                mContext.getSystemService(Context.TELEPHONY_SERVICE);
        boolean isRoaming = tm.isNetworkRoaming(subId);
        if (isRoaming && isConvertRoamingStateForSpecificOP()) {
            isRoaming = false;
        }
        setWfcNonPersistent(enabled, getWfcMode(isRoaming));
    }

    /**
     * Returns data roaming enabled/disabled flag.
     * @return Data roaming enabled/disabled flag.
     */
    protected boolean isDataRoaming() {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        if (tm == null) {
            loge("isDataRoaming(): TelephonyManager null");
            return false;
        }

        ServiceState ss = tm.getServiceStateForSubscriber(getSubId());
        if (ss == null) {
            loge("isDataRoaming(): ServiceState null");
            return false;
        }

        log("[" + mPhoneId + "][SubId=" + getSubId() + "] isDataRoaming(): " + ss.getDataRoaming());

        return ss.getDataRoaming();
    }

    /**
     * Returns data roaming enabled/disabled flag.
     * @return Data roaming enabled/disabled flag.
     */
    protected boolean isDataRoamingSettingsEnabled() {
        boolean isDataRoamingSettingsEnabled = SystemProperties.getBoolean(DATA_ROAMING_SETTING_PROP + String.valueOf(getSubId()), false);
        log("[" + mPhoneId + "][SubId=" + getSubId() + "] isDataRoamingSettingsEnabled(): " + isDataRoamingSettingsEnabled);
        return isDataRoamingSettingsEnabled;
    }

    /**
     * Set data roaming enabled/disabled flag.
     * @param enabled True if data roaming is enabled, otherwise disabled.
     */
    public void setDataRoamingSettingsEnabled(boolean enabled) {
        log("[" + mPhoneId + "][SubId=" + getSubId() + "] setDataRoamingSettingsEnabled(): " + enabled);
        SystemProperties.set(DATA_ROAMING_SETTING_PROP + String.valueOf(getSubId()), enabled ? TRUE : FALSE);
    }

    /**
     * Returns a platform configuration for VT which may override the user
     * setting.
     * <p>
     * Note: VT presumes that VoLTE is enabled (these are configuration settings
     * which must be done correctly).
     */
    @Override
    public boolean isVtEnabledByPlatform() {
        // We first read the per slot value. If doesn't exist, we read the
        // general value. If still
        // doesn't exist, we use the hardcoded default value.
        if (SystemProperties.getInt(
                PROPERTY_DBG_VT_AVAIL_OVERRIDE + Integer.toString(mPhoneId),
                SYSTEM_PROPERTY_NOT_SET) == 1
                || SystemProperties.getInt(PROPERTY_DBG_VT_AVAIL_OVERRIDE,
                        SYSTEM_PROPERTY_NOT_SET) == 1) {
            return true;
        }

        // Dynamic IMS Switch
        boolean isResourceSupport = true;
        boolean isCarrierConfigSupport = true;
        boolean isGbaValidSupport = true;
        boolean isFeatureEnableByPlatformExt = true;

        //sometiime, when handling EVENT_RADIO_ON and update vilte config to MD,
        //the vt resouce is false because of IMS_CFG_CONFIG_LOADED has not completed yet.
        //it maybe make some case fail because SIM loaded too late to update again when IMS regiter
        //So checking test SIM, we make sure vt resource is ture here becasue imsconfigcontroller
        //always set vt resouece ture if isTestSim as well.
        if (!isTestSim()) {
            isResourceSupport = isImsResourceSupport(
                    ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE);
        }
        isCarrierConfigSupport = getBooleanCarrierConfig(
                CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);
        isGbaValidSupport = isGbaValid();
        isFeatureEnableByPlatformExt = isFeatureEnabledByPlatformExt(
                ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE);

        log("Vt, isResourceSupport:" + isResourceSupport
                + ", isCarrierConfigSupport:" + isCarrierConfigSupport
                + ", isGbaValidSupport:" + isGbaValidSupport
                + ", isFeatureEnableByPlatformExt:"
                + isFeatureEnableByPlatformExt);

        return (SystemProperties.getInt(PROPERTY_MTK_VILTE_SUPPORT, 0) == 1)
                && (isLteSupported())
                && isResourceSupport && isCarrierConfigSupport
                && isGbaValidSupport && isFeatureEnableByPlatformExt;
    }

    /**
     * Change persistent VT enabled setting for slot.
     */
    @Override
    public void setVtSetting(boolean enabled) {
        if (enabled && !isVtProvisionedOnDevice()) {
            log("setVtSetting: Not possible to enable Vt due to provisioning.");
            return;
        }

        int subId = getSubId();
        if (isSubIdValid(subId)) {
            SubscriptionManager.setSubscriptionProperty(subId, SubscriptionManager.VT_IMS_ENABLED,
                    booleanToPropertyString(enabled));
        } else {
            loge("setVtSetting: sub id invalid, skip modifying vt state in subinfo db; subId="
                    + subId);
        }

        try {
            changeMmTelCapability(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                    ImsRegistrationImplBase.REGISTRATION_TECH_LTE, enabled);
            changeMmTelCapability(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                    ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN, enabled);

            if (enabled) {
                log("setVtSetting(b) : turnOnIms");
                turnOnIms();
            } else if (isTurnOffImsAllowedByPlatform()
                    && (!isVolteEnabledByPlatform()
                    || !isEnhanced4gLteModeSettingEnabledByUser())) {
                log("setVtSetting(b) : imsServiceAllowTurnOff -> turnOffIms");
                turnOffIms();
            }
        } catch (ImsException e) {
            // The ImsService is down. Since the SubscriptionManager already recorded the user's
            // preference, it will be resent in updateImsServiceConfig when the ImsPhoneCallTracker
            // reconnects.
            loge("setVtSetting(b): ", e);
        }
    }

    /**
     * Change persistent VT enabled setting for slot only, but not update config.
     */
    public void setVtSettingOnly(boolean enabled) {
        SubscriptionManager.setSubscriptionProperty(getSubId(), SubscriptionManager.VT_IMS_ENABLED,
                booleanToPropertyString(enabled));
    }

    /**
     * @deprecated Does not take into account roaming state of the network, use
     * {@link #setWfcNonPersistent(boolean, int, boolean)}.
     */
    @Override
    public void setWfcNonPersistent(boolean enabled, int wfcMode) {
        super.setWfcNonPersistent(enabled, wfcMode);

        // Update viwifi feature value for customized
        CapabilityChangeRequest request = new CapabilityChangeRequest();
        updateVideoCallFeatureValue(request);
        try {
            changeMmTelCapability(request);
        } catch (ImsException e) {
            loge("setWfcNonPersistent(): ", e);
        }
    }

    // Mark for temporary.
    /**
     * Change persistent WFC preference setting
     * @param roaming {@code false} for home network setting, {@code true} for
     *            roaming setting
     */
    // @Override
    // public void setWfcMode(int wfcMode, boolean roaming) {
    //     super.setWfcMode(wfcMode, roaming);
        /*
         * Currently UI only displays Calling preference and doesn't display
         * roaming preference UI always call setWfcMode(roaming == false). We
         * chooses to make Calling preference work both in home and roaming
         * environment, so always set Calling preference to Modem
         */
    //     TelephonyManager tm = (TelephonyManager) mContext
    //             .getSystemService(Context.TELEPHONY_SERVICE);
    //     if (roaming != (tm.isNetworkRoaming(getSubId()) || isCellularDataRoaming())) {
    //         setWfcModeInternal(wfcMode);
    //     }
    // }

    protected void setWfcModeInternal(int wfcMode) {
        if (!supportMdAutoSetupIms()) {
            log("Set wfc mode to MD.");
            setWfcModeConfigEx(wfcMode);
        }
        final int value = wfcMode;
        mExecutorFactory.executeRunnable(() -> {
            try {
                getConfigInterface().setConfig(
                        ProvisioningManager.KEY_VOICE_OVER_WIFI_MODE_OVERRIDE, value);
            } catch (ImsException e) {
                // do nothing
            }
        });
    }

    private void setWfcModeConfigEx(int wfcMode) {
        log("setWfcModeConfigEx wfcMode:" + wfcMode + ", phoneId:" + mPhoneId);

        // Sync wifi preferred mode to modem instead of via provisioning.
        try {
            getConfigInterfaceEx().setWfcMode(wfcMode);
        } catch (ImsException e) {
            // do nothing
        }
    }

    private boolean supportMdAutoSetupIms() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }

    private boolean isConvertRoamingStateForSpecificOP() {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        boolean isWfcModeHomeForDomRoaming = getBooleanCarrierConfig(
                    MtkCarrierConfigManager.MTK_KEY_WFC_MODE_DOMESTIC_ROMAING_TO_HOME);
        log("isWfcModeHomeForDomRoaming:" + isWfcModeHomeForDomRoaming);

        boolean ret = false;;

        if (isWfcModeHomeForDomRoaming) {
            if (tm != null) {
                ServiceState ss = tm.getServiceStateForSubscriber(getSubId());
                if (ss != null) {
                    int voiceRoamingType = ss.getVoiceRoamingType();
                    int dataRoamingType = ss.getDataRoamingType();

                    if (voiceRoamingType == ServiceState.ROAMING_TYPE_DOMESTIC ||
                            dataRoamingType == ServiceState.ROAMING_TYPE_DOMESTIC) {
                        log("Convert roaming to HOME if it's domestic roaming, " +
                                " voiceRoamingType: " + voiceRoamingType +
                                " dataRoamingType: " + dataRoamingType);
                        ret = true;
                    }
                } else {
                    loge("isCellularDataRoaming(): ServiceState null");
                }
            } else {
                loge("isCellularDataRoaming(): TelephonyManager null");
            }
        }

        return ret;
    }

    @Override
    public boolean isWfcEnabledByPlatform() {
        // We first read the per slot value. If doesn't exist, we read the
        // general value. If still
        // doesn't exist, we use the hardcoded default value.
        if (SystemProperties.getInt(
                PROPERTY_DBG_WFC_AVAIL_OVERRIDE + Integer.toString(mPhoneId),
                SYSTEM_PROPERTY_NOT_SET) == 1
                || SystemProperties.getInt(PROPERTY_DBG_WFC_AVAIL_OVERRIDE,
                        SYSTEM_PROPERTY_NOT_SET) == 1) {
            return true;
        }

        // Dynamic IMS Switch
        boolean isResourceSupport = true;
        boolean isCarrierConfigSupport = true;
        boolean isGbaValidSupport = true;
        boolean isFeatureEnableByPlatformExt = true;

        isResourceSupport = isImsResourceSupport(
                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI);
        isCarrierConfigSupport = getBooleanCarrierConfig(
                CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);
        isGbaValidSupport = isGbaValid();
        isFeatureEnableByPlatformExt = isFeatureEnabledByPlatformExt(
                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI);

        log("Wfc, isResourceSupport:" + isResourceSupport
                + ", isCarrierConfigSupport:" + isCarrierConfigSupport
                + ", isGbaValidSupport:" + isGbaValidSupport
                + ", isFeatureEnableByPlatformExt:"
                + isFeatureEnableByPlatformExt);

        return (SystemProperties.getInt(PROPERTY_MTK_WFC_SUPPORT, 0) == 1)
                && (isLteSupported())
                && isResourceSupport && isCarrierConfigSupport
                && isGbaValidSupport && isFeatureEnableByPlatformExt;
    }

    private String getTtyModeSettingKeyForSlot() {
        if (mPhoneId == SIM_ID_2) {
            return PREFERRED_TTY_MODE_SIM2;
        } else if (mPhoneId == SIM_ID_3) {
            return PREFERRED_TTY_MODE_SIM3;
        } else if (mPhoneId == SIM_ID_4) {
            return PREFERRED_TTY_MODE_SIM4;
        }
        return Settings.Secure.PREFERRED_TTY_MODE;
    }

    /**
     * Do <b>NOT</b> use this directly, instead use {@link ImsManager#getInstance(Context, int)}.
     */
    private MtkImsManager(Context context, int phoneId) {
        super(context, phoneId);
        createMtkImsService(true);
    }

    /*
     * Returns a flag indicating whether the IMS service is available.
     */
    public boolean isServiceAvailable() {
        boolean available = true;
        if (super.isServiceAvailable() == false) {
            logw("ImsService binder is not available and rebind again");
            createImsService();
        }
        if (mMtkImsService == null) {
            createMtkImsService(true);
        }
        // checking MtkImsService should also cover ImsService
        available = (mMtkImsService != null);
        log("isServiceAvailable=" + available);
        return available;
    }

    public void close() {

        /// M: add for debug @{
        if (DBG)
            log("close");
        /// @}

        super.close();
        mMtkUt = null;
    }

    /**
     * Gets the configuration interface to provision / withdraw the
     * supplementary service settings.
     * @param serviceId a service id which is obtained from
     *            {@link ImsManager#open}
     * @return the Ut interface instance
     * @throws ImsException if getting the Ut interface results in an error
     */
    public ImsUtInterface getSupplementaryServiceConfiguration()
            throws ImsException {
        if (mMtkUt == null || !mMtkUt.isBinderAlive()) {
            try {
                checkAndThrowExceptionIfServiceUnavailable();
            } catch (ImsException e) {
                loge("getSupplementaryServiceConfiguration(): ", e);
                return null;
            }

            try {
                IImsUt iUt = mMmTelFeatureConnection.getUtInterface();
                IMtkImsUt iMtkUt = mMtkImsService.getMtkUtInterface(mPhoneId);

                if (iUt == null) {
                    throw new ImsException(
                            "getSupplementaryServiceConfiguration()",
                            ImsReasonInfo.CODE_UT_NOT_SUPPORTED);
                }

                mMtkUt = new MtkImsUt(iUt, iMtkUt);
            } catch (RemoteException e) {
                throw new ImsException(
                        "getSupplementaryServiceConfiguration()", e,
                        ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
            }
        }
        return mMtkUt;
    }

    /**
     * Creates a {@link ImsCall} to make a call.
     * @param profile a call profile to make the call (it contains service type,
     *            call type, media information, etc.)
     * @param participants participants to invite the conference call
     * @param listener listen to the call events from {@link ImsCall}
     * @return a {@link ImsCall} object
     * @throws ImsException if calling the IMS service results in an error
     */
    @Override
    public ImsCall makeCall(ImsCallProfile profile, String[] callees,
            ImsCall.Listener listener) throws ImsException {
        if (DBG) {
            log("makeCall :: profile=" + profile + ", callees="
                    + sensitiveEncode(Arrays.toString(callees)));
        }

        checkAndThrowExceptionIfServiceUnavailable();

        ImsCall call = new MtkImsCall(mContext, profile);

        call.setListener(listener);
        ImsCallSession session = createCallSession(profile);

        if ((callees != null) && (callees.length == 1) &&
        /// M: For VoLTE enhanced conference call. @{
                !profile.getCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE)) {
            /// @}
            call.start(session, callees[0]);
        } else {
            call.start(session, callees);
        }

        return call;
    }

    /**
     * Update video call over LTE config
     * @return whether feature is On
     * @throws ImsException
     */
    @Override
    protected void updateVideoCallFeatureValue(CapabilityChangeRequest request) {
        boolean available = isVtEnabledByPlatform();
        boolean vilteEnabled = isEnhanced4gLteModeSettingEnabledByUser() && isVtEnabledByUser();
        boolean isNonTty = isNonTtyOrTtyOnVolteEnabled();
        boolean isDataEnabled = isDataEnabled();
        boolean ignoreDataEnabledChanged = isTestSim()?
                isVTIgnoreDataChangedByOpid(SystemProperties.get("persist.vendor.operator.optr", "OM")):
                getBooleanCarrierConfig(CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS);
        boolean isDataRoamingEnable = !(isDataRoaming() && !isDataRoamingSettingsEnabled());
        boolean ignoreDataRoaming = getBooleanCarrierConfig(
                MtkCarrierConfigManager.MTK_KEY_IGNORE_DATA_ROAMING_FOR_VIDEO_CALLS);
        boolean isCameraAvailable = true;

        try {
            if (mMtkImsService != null) {
                isCameraAvailable = mMtkImsService.isCameraAvailable();
            } else {
              log("mMtkImsService is not ready yet");
            }
        } catch (RemoteException e) {
            log("mMtkImsService exception");
        }

        /// M: There is no need to consider the data enabled condition when
        // calculating
        /// ViLTE feature value with test SIM inserted @{
        /// boolean isFeatureOn = available && enabled && isNonTty
        /// && (ignoreDataEnabledChanged || isDataEnabled);
        boolean isVilteFeatureOn = available && vilteEnabled && isNonTty && isCameraAvailable
                && (ignoreDataEnabledChanged || (isDataEnabled && (ignoreDataRoaming || isDataRoamingEnable)));
        /// @}

        log("[" + mPhoneId + "] updateVideoCallFeatureValue ViLTE: available = " + available
                + ", vilteEnabled = " + vilteEnabled + ", nonTTY = " + isNonTty
                + ", ignoreDataEnabledChanged = " + ignoreDataEnabledChanged
                + ", data enabled = " + isDataEnabled
                + ", ignoreDataRoaming = " + ignoreDataRoaming
                + ", data roaming enabled = " + isDataRoamingEnable
                + ", camera support = " + isCameraAvailable
                + ", is test sim = " + isTestSim()
                + " (ignore data = "
                    + (!isTestSim()? "No need to check":
                    isVTIgnoreDataChangedByOpid(SystemProperties.get("persist.vendor.operator.optr", "OM")))
                + ")");

        if (isVilteFeatureOn) {
            request.addCapabilitiesToEnableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                    ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        } else {
            request.addCapabilitiesToDisableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                    ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        }

        // ViWiFi
        boolean isViWifiNeedCheckWfcEnabled = getBooleanCarrierConfig(
                MtkCarrierConfigManager.MTK_KEY_VT_OVER_WIFI_CHECK_WFC_ENABLE_BOOL);
        boolean isViWifiNeedCheckVolteEnabled = getBooleanCarrierConfig(
                MtkCarrierConfigManager.MTK_KEY_VT_OVER_WIFI_CHECK_VOLTE_ENABLE_BOOL);
        boolean viwifiEnabled = isVtEnabledByUser()
                && (!isViWifiNeedCheckVolteEnabled || isEnhanced4gLteModeSettingEnabledByUser())
                && (!isViWifiNeedCheckWfcEnabled || isWfcEnabledByUser());
        boolean isViWifiNeedCheckDataEnabled = getBooleanCarrierConfig(
                MtkCarrierConfigManager.MTK_KEY_VT_OVER_WIFI_CHECK_DATA_ENABLE_BOOL);

        boolean isViwifiFeatureOn = available && viwifiEnabled && isNonTty && isCameraAvailable
                && (!isViWifiNeedCheckDataEnabled || isDataEnabled);

        log("[" + mPhoneId + "] updateVideoCallFeatureValue ViWiFi: available = " + available
                + ", viwifiEnabled = " + viwifiEnabled
                + ", isViWifiNeedCheckWfcEnabled = " + isViWifiNeedCheckWfcEnabled
                + ", isWfcEnabledByUser = " + isWfcEnabledByUser()
                + ", isViWifiNeedCheckDataEnabled = " + isViWifiNeedCheckDataEnabled
                + ", isViWifiNeedCheckVolteEnabled = " + isViWifiNeedCheckVolteEnabled);

        if (isViwifiFeatureOn) {
            request.addCapabilitiesToEnableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                    ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN);

        } else {
            request.addCapabilitiesToDisableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                    ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN);
        }
    }

    /**
     * Update WFC config
     */
    protected void updateWfcFeatureAndProvisionedValues(CapabilityChangeRequest request) {
        TelephonyManager tm = new TelephonyManager(mContext, getSubId());
        boolean isNetworkRoaming = tm.isNetworkRoaming();
        boolean available = isWfcEnabledByPlatform();
        boolean enabled = isWfcEnabledByUser();

        if (isNetworkRoaming && isConvertRoamingStateForSpecificOP()) {
            isNetworkRoaming = false;
        }

        int mode = getWfcMode(isNetworkRoaming);
        boolean roaming = isWfcRoamingEnabledByUser();
        boolean isFeatureOn = available && enabled;

        log("updateWfcFeatureAndProvisionedValues: available = " + available
                + ", enabled = " + enabled
                + ", mode = " + mode
                + ", roaming = " + roaming);

        if (isFeatureOn) {
            request.addCapabilitiesToEnableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                    ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN);
        } else {
            request.addCapabilitiesToDisableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                    ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN);
        }

        if (!isFeatureOn) {
            mode = ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED;
            roaming = false;
        }
        setWfcModeInternal(mode);
        setWfcRoamingSettingInternal(roaming);
    }

    /**
     * Binds the IMS service only if the service is not created.
     */
    private void checkAndThrowExceptionIfServiceUnavailable()
            throws ImsException {
        if (mMmTelFeatureConnection == null
                || !mMmTelFeatureConnection.isBinderAlive()) {
            createImsService();

            if (mMmTelFeatureConnection == null) {
                throw new ImsException("Service is unavailable",
                        ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
            }
        }

        if (mMtkImsService == null) {
            createMtkImsService(true);

            if (mMtkImsService == null) {
                throw new ImsException("MtkImsService is unavailable",
                        ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
            }
        }
    }

    private static String getMtkImsServiceName(int phoneId) {
        return MTK_IMS_SERVICE;
    }

    /**
     * Binds the IMS service to make/receive the call.
     */
    private void createMtkImsService(boolean checkService) {
        if (checkService) {
            IBinder binder = ServiceManager
                    .checkService(getMtkImsServiceName(mPhoneId));

            if (binder == null) {
                /// M: add for debug @{
                if (DBG)
                    log("createMtkImsService binder is null");
                /// @}
                return;
            }
        }

        IBinder b = ServiceManager.getService(getMtkImsServiceName(mPhoneId));

        if (b != null) {
            try {
                b.linkToDeath(mMtkDeathRecipient, 0);
            } catch (RemoteException e) {
            }
        }

        mMtkImsService = IMtkImsService.Stub.asInterface(b);
        /// M: add for debug @{
        if (DBG)
            log("mMtkImsService = " + mMtkImsService);
        /// @}
    }

    private static void log(String s) {
        Rlog.d(TAG, s);
    }

    private static void logw(String s) {
        Rlog.w(TAG, s);
    }

    private static void loge(String s) {
        Rlog.e(TAG, s);
    }

    private static void loge(String s, Throwable t) {
        Rlog.e(TAG, s, t);
    }

    private static void logi(String s) {
        Rlog.i(TAG, s);
    }

    private static String sensitiveEncode(String s) {
        return Rlog.pii(TAG, s);
    }

    /**
     * Death recipient class for monitoring IMS service.
     */
    private class MtkImsServiceDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            mMtkImsService = null;
            mMtkUt = null;
            mNotifyOnly = false;
            loge("[" + mPhoneId + "]" + "MtkImsService binder died!");
        }
    }

    /**
     * Gets the call Num from the specified incoming call broadcast intent.
     * @param incomingCallIntent the incoming call broadcast intent
     * @return the call Num or null if the intent does not contain it
     */
    private String getCallNum(Intent incomingCallIntent) {
        if (incomingCallIntent == null) {
            return null;
        }

        return incomingCallIntent.getStringExtra(EXTRA_DIAL_STRING);
    }

    /**
     * Gets the sequence number from the specified incoming call broadcast
     * intent.
     * @param incomingCallIntent the incoming call broadcast intent
     * @return the sequence number or null if the intent does not contain it
     * @hide
     */
    private int getSeqNum(Intent incomingCallIntent) {
        if (incomingCallIntent == null) {
            return (-1);
        }

        return incomingCallIntent.getIntExtra(EXTRA_SEQ_NUM, -1);
    }

    private String getMtToNumber(Intent incomingCallIntent) {
        if (incomingCallIntent == null) {
            return null;
        }

        return incomingCallIntent.getStringExtra(EXTRA_MT_TO_NUMBER);
    }

    /**
     * To Allow or refuse incoming call indication to send to App.
     * @param phoneId a service id which is obtained from
     *            {@link ImsManager#open}
     * @param incomingCallIndication the incoming call broadcast intent.
     * @param isAllow to indication to allow or refuse the incoming call
     *            indication.
     * @throws ImsException if set call indication results in an error.
     * @hide
     */
    public void setCallIndication(int phoneId, Intent incomingCallIndication,
            boolean isAllow, int cause) throws ImsException {
        if (DBG) {
            log("setCallIndication :: phoneId=" + phoneId
                    + ", incomingCallIndication=" + incomingCallIndication
                    + ", isAllow=" + isAllow
                    + ", cause=" + cause);
        }

        checkAndThrowExceptionIfServiceUnavailable();

        if (incomingCallIndication == null) {
            throw new ImsException("Can't retrieve session with null intent",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        }

        int incomingPhoneId = getPhoneId(incomingCallIndication);

        if (phoneId != incomingPhoneId) {
            throw new ImsException(
                    "Service id is mismatched in the incoming call intent",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        }

        String callId = getCallId(incomingCallIndication.getExtras());

        if (callId == null) {
            throw new ImsException(
                    "Call ID missing in the incoming call intent",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        }

        String callNum = getCallNum(incomingCallIndication);

        if (callNum == null) {
            throw new ImsException(
                    "Call Num missing in the incoming call intent",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        }

        int seqNum = getSeqNum(incomingCallIndication);

        if (seqNum == -1) {
            throw new ImsException(
                    "seqNum missing in the incoming call intent",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        }

        String toNumber = getMtToNumber(incomingCallIndication);

        try {
            mMtkImsService.setCallIndication(phoneId, callId, callNum,
                    seqNum, toNumber, isAllow, cause);
        } catch (RemoteException e) {
            throw new ImsException("setCallIndication()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    /**
     * To hangup all calls.
     * @throws ImsException if getting the ims status result in an error.
     * @hide
     */
    public void hangupAllCall(int phoneId) throws ImsException {

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            mMtkImsService.hangupAllCall(phoneId);
        } catch (RemoteException e) {
            throw new ImsException("hangupAll()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    /**
     * Gets the phone identifier from the specified incoming call broadcast
     * intent.
     * @param incomingCallIntent the incoming call broadcast intent
     * @return the phone identifier or -1 if the intent does not contain it
     */
    private static int getPhoneId(Intent incomingCallIntent) {
        if (incomingCallIntent == null) {
            return (-1);
        }

        return incomingCallIntent.getIntExtra(EXTRA_PHONE_ID, -1);
    }

    /// M: Dynamic IMS Switch @{

    /**
     * Returns a feature value by currently phone id's resource.
     */
    protected boolean isImsResourceSupport(int feature) {
        // Don't care if Dynamic IMS Switch not enabled.
        boolean support = true;
        log("isImsResourceSupport, feature:" + feature);
        if ("1".equals(SystemProperties.get(PROPERTY_DYNAMIC_IMS_SWITCH))) {

            if (!SubscriptionManager.isValidPhoneId(mPhoneId)) {
                loge("Invalid main phone " + mPhoneId
                        + ", return true as don't care");
                return support;
            }
            try {
                MtkImsConfig config = getConfigInterfaceEx();
                if (config != null) {
                    support = (config.getImsResCapability(feature)
                            != ImsConfig.FeatureValueConstants.ON) ? false : true;
                }
            } catch (ImsException e) {
                loge("isImsResourceSupport() failed!" + e);
            }
            if (DBG) {
                log("isImsResourceSupport(" + feature + ") return " + support
                        + " on phone: " + mPhoneId);
            }
        }

        return support;
    }

    @Override
    public void factoryReset() {
        // Set VoLTE to default
        boolean value = getBooleanCarrierConfig(
                        CarrierConfigManager.KEY_ENHANCED_4G_LTE_ON_BY_DEFAULT_BOOL);
        if (SystemProperties.getInt(PROPERTY_CT_VOLTE_SUPPORT, 0) != 0) {
            // For CT 3G SIM, VoLTE setting shall be off.
            // carrier config volte value is not consist to CT request.
            int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId);
            SubscriptionInfo subInfo =
                SubscriptionManager.from(mContext).getActiveSubscriptionInfo(subId);
            String iccid = subInfo != null ? subInfo.getIccId() : null;
            if (!TextUtils.isEmpty(iccid) && isOp09SimCard(iccid)) {
                MtkIccCardConstants.CardType cardType =
                    MtkTelephonyManagerEx.getDefault().getCdmaCardType(mPhoneId);
                if (cardType != null && !cardType.is4GCard()) {
                    log("factoryReset, CT 3G card case");
                    value = false;
                }
            }
        }
        // Set VoLTE to default
        SubscriptionManager
                .setSubscriptionProperty(
                        getSubId(),
                        SubscriptionManager.ENHANCED_4G_MODE_ENABLED,
                        booleanToPropertyString(value));

        // Set VoWiFi to default
        SubscriptionManager
                .setSubscriptionProperty(
                        getSubId(),
                        SubscriptionManager.WFC_IMS_ENABLED,
                        booleanToPropertyString(getBooleanCarrierConfig(
                                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ENABLED_BOOL)));

        // Set VoWiFi mode to default
        SubscriptionManager
                .setSubscriptionProperty(
                        getSubId(),
                        SubscriptionManager.WFC_IMS_MODE,
                        Integer.toString(getIntCarrierConfig(
                                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_MODE_INT)));

        // Set VoWiFi roaming to default
        SubscriptionManager
                .setSubscriptionProperty(
                        getSubId(),
                        SubscriptionManager.WFC_IMS_ROAMING_ENABLED,
                        booleanToPropertyString(getBooleanCarrierConfig(
                                CarrierConfigManager
                                .KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_ENABLED_BOOL)));

        // Set VT to default
        SubscriptionManager.setSubscriptionProperty(getSubId(),
                SubscriptionManager.VT_IMS_ENABLED,
                booleanToPropertyString(true));

        // Push settings to ImsConfig
        updateImsServiceConfig(true);
    }

    /**
     * Static version API to get the config interface.
     */
    public MtkImsConfig getConfigInterfaceEx() throws ImsException {
        MtkImsConfig config = null;

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            IMtkImsConfig binder = mMtkImsService
                    .getConfigInterfaceEx(mPhoneId);
            if (binder == null) {
                throw new ImsException("getConfigInterfaceEx()",
                        ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
            }
            config = new MtkImsConfig(binder, mContext);
        } catch (RemoteException e) {
            throw new ImsException("getConfigInterfaceEx()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
        return config;
    }

    /// @}

    /**
     * Creates a {@link ImsCall} to take an incoming call.
     * @param sessionId a session id which is obtained from
     *            {@link ImsManager#open}
     * @param incomingCallExtras the incoming call broadcast intent
     * @param listener to listen to the call events from {@link ImsCall}
     * @return a {@link ImsCall} object
     * @throws ImsException if calling the IMS service results in an error
     */
    @Override
    public ImsCall takeCall(IImsCallSession session, Bundle incomingCallExtras,
            ImsCall.Listener listener) throws ImsException {
        if (DBG) {
            log("takeCall :: incomingCall=" + sensitiveEncode("" + incomingCallExtras));
        }

        checkAndThrowExceptionIfServiceUnavailable();

        if (incomingCallExtras == null) {
            throw new ImsException("Can't retrieve session with null intent",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        }

        // MTK SS {
        boolean isUssd = incomingCallExtras.getBoolean(ImsManager.EXTRA_USSD, false);
        if (isUssd) {
            log("takeCall :: isUssd = true, invoke original AOPS's takeCall()");
            return super.takeCall(session, incomingCallExtras, listener);
        }
        // MTK SS }

        String callId = getCallId(incomingCallExtras);

        if (callId == null) {
            throw new ImsException(
                    "Call ID missing in the incoming call intent",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        }

        try {
            // It's better to get session from parameter in the future.
            IMtkImsCallSession mtkSession = mMtkImsService.getPendingMtkCallSession(
                    mPhoneId, callId);
            session = mtkSession.getIImsCallSession();

            if (mtkSession == null) {
                throw new ImsException(
                        "No pending IMtkImsCallSession for the call",
                        ImsReasonInfo.CODE_LOCAL_NO_PENDING_CALL);
            }

            ImsCallProfile callProfile = mtkSession.getCallProfile();
            if (callProfile == null) {
                throw new ImsException("takeCall(): profile is null",
                        ImsReasonInfo.CODE_UNSPECIFIED);
            }
            ImsCall call = new MtkImsCall(mContext, callProfile);

            call.attachSession(new MtkImsCallSession(session, mtkSession));
            call.setListener(listener);

            return call;
        } catch (Throwable t) {
            throw new ImsException("takeCall()", t,
                    ImsReasonInfo.CODE_UNSPECIFIED);
        }
    }

    /**
     * Creates a {@link ImsCallSession} with the specified call profile. Use
     * other methods, if applicable, instead of interacting with
     * {@link ImsCallSession} directly.
     * @param serviceId a service id which is obtained from
     *            {@link ImsManager#open}
     * @param profile a call profile to make the call
     */
    @Override
    protected ImsCallSession createCallSession(ImsCallProfile profile)
            throws ImsException {
        try {
            log("createCallSession: profile = " + profile);
            ImsCallSession imsCallSession = super.createCallSession(profile);
            log("createCallSession: imsCallSession = " + imsCallSession);
            log("createCallSession: imsCallSession.getSession() = " + imsCallSession.getSession());
            IMtkImsCallSession mtkImsCallSessionImpl =  mMtkImsService.createMtkCallSession(mPhoneId, profile, null, imsCallSession.getSession());

            return new MtkImsCallSession(imsCallSession.getSession(), mtkImsCallSessionImpl);

        } catch (RemoteException e) {
            Rlog.w(TAG,
                    "CreateCallSession: Error, remote exception: "
                            + e.getMessage());
            throw new ImsException("createCallSession()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    @Override
    protected void setLteFeatureValues(boolean turnOn) {
        log("setLteFeatureValues: " + turnOn);
        CapabilityChangeRequest request = new CapabilityChangeRequest();
        if (turnOn) {
            request.addCapabilitiesToEnableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                    ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        } else {
            request.addCapabilitiesToDisableForTech(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                    ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        }

        if (isVolteEnabledByPlatform() && isVtEnabledByPlatform()) {
            boolean ignoreDataEnabledChanged = isTestSim()?
                    isVTIgnoreDataChangedByOpid(SystemProperties.get("persist.vendor.operator.optr", "OM")):
                    getBooleanCarrierConfig(CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS);
            boolean isDataRoamingEnable = !(isDataRoaming() && !isDataRoamingSettingsEnabled());
            boolean ignoreDataRoaming = getBooleanCarrierConfig(
                     MtkCarrierConfigManager.MTK_KEY_IGNORE_DATA_ROAMING_FOR_VIDEO_CALLS);
            boolean isCameraAvailable = true;

            try {
                if (mMtkImsService != null) {
                    isCameraAvailable = mMtkImsService.isCameraAvailable();
                } else {
                    log("mMtkImsService is not ready yet");
                }
            } catch (RemoteException e) {
                Log.e(TAG, "setLteFeatureValues: isCameraAvailable() Exception: " + e.getMessage());
            }
            /// M: There is no need to consider the data enabled condition when
            // calculating
            /// ViLTE feature value with test SIM inserted @{
            boolean enableViLte = turnOn
                    && isVtEnabledByUser() && isCameraAvailable
                    && (ignoreDataEnabledChanged || (isDataEnabled() && (ignoreDataRoaming || isDataRoamingEnable)));

            /// ViWiFi feature value with test SIM inserted @{
            boolean isViWifiNeedCheckWfcEnabled = getBooleanCarrierConfig(
                    MtkCarrierConfigManager.MTK_KEY_VT_OVER_WIFI_CHECK_WFC_ENABLE_BOOL);
            boolean isViWifiNeedCheckVolteEnabled = getBooleanCarrierConfig(
                    MtkCarrierConfigManager.MTK_KEY_VT_OVER_WIFI_CHECK_VOLTE_ENABLE_BOOL);
            boolean viwifiEnabled = isVtEnabledByUser()
                    && (!isViWifiNeedCheckVolteEnabled || turnOn)
                    && (!isViWifiNeedCheckWfcEnabled || isWfcEnabledByUser());
            boolean isViWifiNeedCheckDataEnabled = getBooleanCarrierConfig(
                    MtkCarrierConfigManager.MTK_KEY_VT_OVER_WIFI_CHECK_DATA_ENABLE_BOOL);

            boolean enableViWifi = viwifiEnabled && isCameraAvailable
                    && (!isViWifiNeedCheckDataEnabled || isDataEnabled());

            /// @}
            if (enableViLte) {
                request.addCapabilitiesToEnableForTech(
                        MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                        ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
            } else {
                request.addCapabilitiesToDisableForTech(
                        MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                        ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
            }

            if (enableViWifi) {
                request.addCapabilitiesToEnableForTech(
                        MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                        ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN);
            } else {
                request.addCapabilitiesToDisableForTech(
                        MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                        ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN);
            }
        }
        try {
            mMmTelFeatureConnection.changeEnabledCapabilities(request, null);
        } catch (RemoteException e) {
            Log.e(TAG, "setLteFeatureValues: Exception: " + e.getMessage());
            changeMmTelCapabilityInternally(request);
        }
    }

    /**
     * An API to customization platform enabled status.
     * @param context The context for retrive plug-in.
     * @param feature The IMS feature defined in ImsConfig.FeatureConstants.
     * @return return enabled status.
     */
    private boolean isFeatureEnabledByPlatformExt(int feature) {
        boolean isEnabled = true;
        if (mContext == null) {
            logw("Invalid: context=" + mContext + ", return " + isEnabled);
            return isEnabled;
        }
        IImsManagerExt imsMgrExt = getImsManagerPluginInstance(mContext);
        if (imsMgrExt == null) {
            logw("plugin null=" + imsMgrExt + ", return " + isEnabled);
            return isEnabled;
        }
        isEnabled = imsMgrExt.isFeatureEnabledByPlatform(mContext,
                feature, mPhoneId);
        return isEnabled;
    }

    /**
     * Get the phone id with main capability.
     */
    protected int getMainCapabilityPhoneId(Context context) {
        return getMainPhoneIdForSingleIms(context);
    }

    private static IImsManagerExt getImsManagerPluginInstance(Context context) {
        log("getImsManagerPluginInstance");
        // Do not cache sImsManagerExt because CarrierExpress dynamic switch
        IImsManagerExt imsMgrExt = OpImsCustomizationUtils.getOpFactory(context)
                .makeImsManagerExt(context);
        if (imsMgrExt == null) {
            log("Unable to create ImsManagerPluginInstane");
        }
        return imsMgrExt;
    }

    protected boolean isVTIgnoreDataChangedByOpid(String optr) {
        //For OP01, OP02, OP09, OP17, OP50, OP149, OP207
        //VT should ignore mobile data enabled changed
        if (("OP01".equals(optr) || "OP02".equals(optr) || "OP09".equals(optr)
            || "OP17".equals(optr) || "OP50".equals(optr) || "OP149".equals(optr)
            || "OP149".equals(optr))) {
            return true;

        } else {
            return false;
        }
    }

    /// M: VILTE enable not dependent on data enable, only used for China test sim @{
    protected boolean isTestSim() {
        int phoneId = mPhoneId;
        if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) == 1) {
            phoneId = getMainCapabilityPhoneId(mContext);
        }
        boolean isTestSim = false;
        switch (phoneId) {
            case SIM_ID_1:
                isTestSim = "1".equals(SystemProperties.get(PROPERTY_TEST_SIM1,
                        "0"));
                break;
            case SIM_ID_2:
                isTestSim = "1".equals(SystemProperties.get(PROPERTY_TEST_SIM2,
                        "0"));
                break;
            case SIM_ID_3:
                isTestSim = "1".equals(SystemProperties.get(PROPERTY_TEST_SIM3,
                        "0"));
                break;
            case SIM_ID_4:
                isTestSim = "1".equals(SystemProperties.get(PROPERTY_TEST_SIM4,
                        "0"));
                break;
        }
        // only return ture for mccmnc=00101 or 11111 when mccmnc is valid
        if (isTestSim) {
            TelephonyManager tm = (TelephonyManager) mContext
                    .getSystemService(Context.TELEPHONY_SERVICE);
            String currentMccMnc = tm.getSimOperatorNumericForPhone(phoneId);
            log("isTestSim, currentMccMnc:" + currentMccMnc);
            if (currentMccMnc != null
                    && !currentMccMnc.equals("")
                    && !("00101".equals(currentMccMnc))
                    && !("11111".equals(currentMccMnc))
                    && !("46011".equals(currentMccMnc))) {
                isTestSim = false;
            }
        }
        return isTestSim;
    }

    /// @}

    private boolean isOp09SimCard(String iccId) {
        boolean isOp09Card = false;
        if (iccId.startsWith("898603") || iccId.startsWith("898611")
                || iccId.startsWith("8985302") || iccId.startsWith("8985307")
                || iccId.startsWith("8985231")) {
            isOp09Card = true;
        } else if (iccId.startsWith("894900")) {
            String currentMccMnc = "";
            if (mPhoneId == 0) {
                currentMccMnc = SystemProperties.get("vendor.gsm.ril.uicc.mccmnc", "");
            } else {
                currentMccMnc =
                      SystemProperties.get("vendor.gsm.ril.uicc.mccmnc." + mPhoneId, "");
            }
            log("isOp09SimCard, currentMccMnc:" + currentMccMnc);
            if (currentMccMnc != null
                    && ("46011".equals(currentMccMnc) || "46003".equals(currentMccMnc))) {
                isOp09Card = true;
            }
        }
        return isOp09Card;
    }

    /// @}

    /**
     * For some op, phone id may need to be replaced, this method help to find
     * an appropriate manager instance by plugin impl
     * @return
     */
    private static MtkImsManager getAppropriateManagerForPlugin(Context context,
            int phoneId) {
        IImsManagerExt imsMgrExt = MtkImsManager.getImsManagerPluginInstance(context);

        if (imsMgrExt != null) {
            phoneId = imsMgrExt.getImsPhoneId(context, phoneId);
        }
        return ((MtkImsManager) ImsManager.getInstance(context,
                phoneId));
    }

    public void notifyRegServiceCapabilityChangedEvent(int event) {
        switch (event) {
            case SERVICE_REG_CAPABILITY_EVENT_ECC_SUPPORT:
                synchronized (mCallbacks) {
                    for (ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                        if (callback instanceof MtkImsConnectionStateListener) {
                            ((MtkImsConnectionStateListener) callback)
                                    .onImsEmergencyCapabilityChanged(true);
                        }
                    }
                }
                break;
            case SERVICE_REG_CAPABILITY_EVENT_ECC_NOT_SUPPORT:
                synchronized (mCallbacks) {
                    for (ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                        if (callback instanceof MtkImsConnectionStateListener) {
                            ((MtkImsConnectionStateListener) callback)
                                    .onImsEmergencyCapabilityChanged(false);
                        }
                    }
                }
                break;
            case SERVICE_REG_EVENT_WIFI_PDN_OOS_START:
                synchronized (mCallbacks) {
                    for(ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                        if (callback instanceof MtkImsConnectionStateListener) {
                            ((MtkImsConnectionStateListener) callback)
                                .onWifiPdnOOSStateChanged(OOS_START);
                        }
                    }
                }
                break;
            case SERVICE_REG_EVENT_WIFI_PDN_OOS_END_WITH_DISCONN:
                synchronized (mCallbacks) {
                    for(ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                        if (callback instanceof MtkImsConnectionStateListener) {
                            ((MtkImsConnectionStateListener) callback)
                                .onWifiPdnOOSStateChanged(OOS_END_WITH_DISCONN);
                        }
                    }
                }
                break;
            case SERVICE_REG_EVENT_WIFI_PDN_OOS_END_WITH_RESUME:
                synchronized (mCallbacks) {
                    for(ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                        if (callback instanceof MtkImsConnectionStateListener) {
                            ((MtkImsConnectionStateListener) callback)
                                .onWifiPdnOOSStateChanged(OOS_END_WITH_RESUME);
                        }
                    }
                }
                break;
            default:
                break;
        }
    }

    private boolean isLteSupported() {
        String ps1Rat = SystemProperties.get("ro.vendor.mtk_ps1_rat", "");
        return ps1Rat.contains("L");
    }

    private static int getFeaturePropValue(String propName, int phoneId) {
      int defaultPropValue = 0;
      int featureValue, propResult;
      featureValue = SystemProperties.getInt(propName, defaultPropValue);

      if (isSupportMims()) {
        propResult = ((featureValue & (1 << phoneId)) > 0)?1:0;
      } else {
        // Backward compatibility, only use phone Id 0 to get.
        propResult = ((featureValue & (1 << 0)) > 0)?1:0;
      }
      return propResult;
    }

    // VzW requirement: Combo setting for VoLTE and ViLTE
    private void setComboFeatureValue(int volte_en, int vt_en, int wfc_en) {
        int[] features = {ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE,
                ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE,
                ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI,
                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI};
        int[] networks = {TelephonyManager.NETWORK_TYPE_LTE, TelephonyManager.NETWORK_TYPE_LTE,
                          TelephonyManager.NETWORK_TYPE_IWLAN, TelephonyManager.NETWORK_TYPE_IWLAN};
        int[] setvalues = {0, 0, 0, 0};

        int oldvolteValue = getFeaturePropValue(PROPERTY_VOLTE_ENALBE, mPhoneId);
        int oldvilteValue = getFeaturePropValue(PROPERTY_VILTE_ENALBE, mPhoneId);
        int oldviwifiValue = getFeaturePropValue(PROPERTY_VIWIFI_ENALBE, mPhoneId);
        int oldWfcValue = getFeaturePropValue(PROPERTY_WFC_ENALBE, mPhoneId);

        setvalues[0] = (volte_en != -1)? volte_en : oldvolteValue;
        setvalues[1] = (vt_en != -1)? vt_en : oldvilteValue;
        setvalues[2] = (vt_en != -1)? vt_en : oldviwifiValue;
        setvalues[3] = (wfc_en != -1)? wfc_en : oldWfcValue;

        ImsManager imsManager = ImsManager.getInstance(mContext, mPhoneId);
        try {
            MtkImsConfig config = ((MtkImsManager)imsManager).getConfigInterfaceEx();
            if (config != null) {
                config.setMultiFeatureValues(features, networks, setvalues,
                        mImsConfigListener);
            }
        } catch (ImsException e) {
            loge("setComboFeatureValue(): " + e);
        }
    }

    @ProductApi
    public void setEnhanced4gLteModeVtSetting(
            Context context, boolean e4genabled, boolean vtenabled){
        int volte_value = e4genabled ? 1 : 0;
        int vt_value = vtenabled ? 1 : 0;
        boolean ignoreDataEnabledChanged;
        boolean enableViLte = false;
        int prevSetting;

        ImsManager imsManager = ImsManager.getInstance(context, mPhoneId);
        if (imsManager != null) {
            try{
                ImsConfig config = imsManager.getConfigInterface();
                if (isSupportMims() == false) {
                    prevSetting = isEnhanced4gLteModeSettingEnabledByUser()? 1 : 0;
                    SubscriptionManager.setSubscriptionProperty(getSubId(),
                                SubscriptionManager.ENHANCED_4G_MODE_ENABLED,
                                Integer.toString(volte_value));
                    setVtSettingOnly(vt_value == 1);
                } else {
                    int value = getBooleanCarrierConfig(
                    CarrierConfigManager.KEY_EDITABLE_ENHANCED_4G_LTE_BOOL) ? volte_value : 1;
                    prevSetting = SubscriptionManager.getIntegerSubscriptionProperty(
                            getSubId(), SubscriptionManager.ENHANCED_4G_MODE_ENABLED,
                            SUB_PROPERTY_NOT_INITIALIZED, mContext);

                    if (prevSetting == value
                            && (SystemProperties.getInt(PROPERTY_IMSCONFIG_FORCE_NOTIFY, 0) == 0)) {
                        volte_value = prevSetting;
                    }
                    else {
                        SubscriptionManager.setSubscriptionProperty(getSubId(),
                                SubscriptionManager.ENHANCED_4G_MODE_ENABLED,
                                Integer.toString(volte_value));
                    }
                    SubscriptionManager.setSubscriptionProperty(getSubId(),
                            SubscriptionManager.ENHANCED_4G_MODE_ENABLED,
                            Integer.toString(volte_value));

                    setVtSettingOnly(vt_value == 1);
                }

                if (isNonTtyOrTtyOnVolteEnabled(context, mPhoneId)) {
                    if (isVolteEnabledByPlatform() && isVtEnabledByPlatform()) {
                        ignoreDataEnabledChanged = isTestSim()?
                                isVTIgnoreDataChangedByOpid(SystemProperties.get("persist.vendor.operator.optr", "OM")):
                                getBooleanCarrierConfig(
                                    CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS);
                        enableViLte = e4genabled && isVtEnabledByUser() &&
                          (ignoreDataEnabledChanged || isDataEnabled());
                    }
                }
                else {
                    //No change
                    volte_value = prevSetting;
                }

                setComboFeatureValue(volte_value, (vtenabled && enableViLte) ? 1 : 0, -1);

                if (e4genabled || vtenabled) {
                    log("setEnhanced4gLteModeVtSetting() : turnOnIms");
                    turnOnIms();
                } else if (ImsManager.isTurnOffImsAllowedByPlatform(context)
                                && (!imsManager.isVolteEnabledByPlatform()
                                    || !isEnhanced4gLteModeSettingEnabledByUser())
                                && (!imsManager.isWfcEnabledByPlatform()
                                    || !isWfcEnabledByUser())) {
                    log("setEnhanced4gLteModeVtSetting() : imsServiceAllowTurnOff -> turnOffIms");
                    turnOffIms();
                }
            } catch (ImsException e) {
                loge("setEnhanced4gLteModeVtSetting error");
            }
        } else {
            loge("setEnhanced4gLteModeVtSetting error");
            loge("getInstance null for phoneId=" + mPhoneId);
        }
    }

    public MmTelFeature.MmTelCapabilities queryCapabilityStatus() {
            MmTelFeature.MmTelCapabilities capabilities = null;
        try {
            capabilities = mMmTelFeatureConnection.queryCapabilityStatus();
        } catch (RemoteException e) {
            loge("Fail to queryCapabilityStatus " + e.getMessage());
            capabilities = new MmTelFeature.MmTelCapabilities();
        } finally {
            if (DBG) {
                log("queryCapabilityStatus = " + capabilities);
            }
            return capabilities;
        }
    }

    ///M: For JIO IR feature starts
    protected boolean shouldEnableImsForIR() {
        if (!mSupportImsiSwitch) {
            log("[IR] IMSI switch feature not supported");
            return true;
        }
        boolean enableIms = true;
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId);
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
           log("[IR] shouldEnableImsForIR: Invalid subId so return");
           return true;
        }
        String permanentMccMnc = getOperatorNumericFromImpi("0");
        String currentMccMnc = getMccMncForSubId(subId, SubscriptionManager.from(mContext));
        if (!(permanentMccMnc.equals(currentMccMnc)) && !("0".equals(permanentMccMnc))) {
             enableIms = false;;
        }
        log("[IR] updateVolteFeatureValue: subId = " + subId
                + ", phoneId = " + mPhoneId
                + ", Current currentMccMnc = " + currentMccMnc
                + ", permanentMccMnc = " + permanentMccMnc
                + ", enableIms = " + enableIms);
        return enableIms;
    }

    private String getOperatorNumericFromImpi(String defaultValue) {
        String[] mImsMccMncList = {"405840", "405854", "405855", "405856", "405857", "405858",
                                       "405859", "405860", "405861", "405862", "405863", "405864",
                                       "405865", "405866", "405867", "405868", "405869", "405870",
                                       "405871", "405872", "405873", "405874"};
        final String mccTag = "mcc";
        final String mncTag = "mnc";
        final int mccLength = 3;
        final int mncLength = 3;

        if (mImsMccMncList == null  || mImsMccMncList.length == 0) {
            log("[IR] mImsMccMncList is null, returning default mccmnc");
            return defaultValue;
        }
        String impi = null;
        log("[IR] IMPI requested by phoneId: " + mPhoneId);
        impi = getIsimImpi(MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId));
        log("[IR] IMPI : " + impi);

        if (impi == null  || impi.equals("")) {
            log("[IR] impi is null/empty, returning default mccmnc");
            return defaultValue;
        }
        int mccPosition = impi.indexOf(mccTag);
        int mncPosition = impi.indexOf(mncTag);
        if (mccPosition == -1 || mncPosition == -1) {
            log("[IR] mcc/mnc position -1, returning default mccmnc");
            return defaultValue;
        }
        String masterMccMnc = impi.substring(mccPosition + mccTag.length(), mccPosition
                + mccTag.length() + mccLength) + impi.substring(mncPosition + mncTag.length(),
                mncPosition + mncTag.length() + mncLength);
        log("[IR] MccMnc fetched from IMPI: " + masterMccMnc);
        if (masterMccMnc == null || masterMccMnc.equals("")) {
            log("[IR] IMPI MccMnc is null/empty, Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        for (String mccMnc : mImsMccMncList) {
            if (masterMccMnc.equals(mccMnc)) {
                log("[IR] mccMnc matched, Returning mccmnc from IMPI: " + masterMccMnc);
                return masterMccMnc;
            }
        }
        log("[IR] IMPI mcc/mnc not matched, returning default mccmnc");
        return defaultValue;
    }

    private IMtkPhoneSubInfoEx getMtkSubscriberInfoEx() {
       // get it each time because that process crashes a lot
       return IMtkPhoneSubInfoEx.Stub.asInterface(ServiceManager.getService("iphonesubinfoEx"));
    }

    private String getIsimImpi(int subId) {
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
           log("[IR] getIsimImpi: Invalid subId so return");
           return null;
        }
        try {
            return getMtkSubscriberInfoEx().getIsimImpiForSubscriber(subId);
        } catch (RemoteException ex) {
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            return null;
        }
    }

    private static String getMccMncForSubId(int subId, SubscriptionManager subsMgr) {
        String mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
        if (mccMnc != null && mccMnc.length() > 0) {
            log("[IR] Getting mcc mnc from TelephonyManager.getSimOperator");
            return mccMnc;
        } else {
            List<SubscriptionInfo> sil = subsMgr.getActiveSubscriptionInfoList();
            log("[IR] Getting mcc mnc from from subinfo for subId = " + subId);
            if (sil != null && sil.size() > 0) {
                for (SubscriptionInfo subInfo : sil) {
                    if (subInfo.getSubscriptionId() == subId) {
                        int mcc = subInfo.getMcc();
                        int mnc = subInfo.getMnc();
                        mccMnc = String.valueOf(mcc) + String.valueOf(mnc);
                        log("[IR] getMccMncForSubId from subInfo = " + mccMnc);
                        return mccMnc;
                    }
               }
            }
            return mccMnc;
        }
    }
    ///M: For JIO IR feature ends

    protected IImsRegistrationListener mListener = null;
    protected IMtkImsRegistrationListener mMtkImsListener = null;
    private boolean mNotifyOnly = true;

    private void hookProprietaryImsListener() throws ImsException {
        if (mMtkImsService == null) {
            log("hookProprietaryImsListener get NULL mMtkImsService so create it");
            createMtkImsService(true);
        }

        if (mListener == null) {
            log("[" + mPhoneId + "] hook proprietary IMS listener");
            mNotifyOnly = false;
            mListener = new IImsRegistrationListener.Stub() {
                @Override
                public void registrationConnected() throws RemoteException {
                    // Implemented in the Registration Adapter
                }

                @Override
                public void registrationProgressing() throws RemoteException {
                    // Implemented in the Registration Adapter
                }

                @Override
                public void registrationConnectedWithRadioTech(int imsRadioTech)
                        throws RemoteException {
                    // Implemented in the Registration Adapter
                    if (DBG) {
                        log("registrationConnectedWithRadioTech :: imsRadioTech=" + imsRadioTech);
                    }

                    synchronized (mCallbacks) {
                        for (ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                            if (callback instanceof MtkImsConnectionStateListener) {
                                ((MtkImsConnectionStateListener) callback)
                                        .onImsConnected(imsRadioTech);
                            }
                        }
                    }
                }

                @Override
                public void registrationProgressingWithRadioTech(int imsRadioTech)
                        throws RemoteException {
                    // Implemented in the Registration Adapter
                }

                @Override
                public void registrationDisconnected(ImsReasonInfo imsReasonInfo)
                        throws RemoteException {
                    // Implemented in the Registration Adapter
                    synchronized (mCallbacks) {
                        for (ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                            if (callback instanceof MtkImsConnectionStateListener) {
                                ((MtkImsConnectionStateListener) callback)
                                        .onImsDisconnected(imsReasonInfo);
                            }
                        }
                    }
                }

                @Override
                public void registrationResumed() throws RemoteException {
                    // Don't care
                }

                @Override
                public void registrationSuspended() throws RemoteException {
                    // Don't care
                }

                @Override
                public void registrationServiceCapabilityChanged(int serviceClass, int event)
                        throws RemoteException {
                    notifyRegServiceCapabilityChangedEvent(event);
                }

                @Override
                public void registrationFeatureCapabilityChanged(int serviceClass,
                        int[] enabledFeatures, int[] disabledFeatures) throws RemoteException {
                    MmTelFeature.MmTelCapabilities capabilities =
                            convertCapabilities(enabledFeatures);
                    log("registrationFeatureCapabilityChanged :: enabledFeatures=" + capabilities);

                    synchronized (mCallbacks) {
                        for (ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                            if (callback instanceof MtkImsConnectionStateListener) {
                                ((MtkImsConnectionStateListener) callback)
                                        .onCapabilitiesStatusChanged(capabilities);
                            }
                        }
                    }
                }

                @Override
                public void voiceMessageCountUpdate(int count) throws RemoteException {
                }

                @Override
                public void registrationAssociatedUriChanged(Uri[] uris) throws RemoteException {
                    // Implemented in the Registration Adapter
                }

                @Override
                public void registrationChangeFailed(int targetAccessTech,
                        ImsReasonInfo imsReasonInfo) throws RemoteException {
                    // Implemented in the Registration Adapter
                }
            };
        } else {
            log("mListener was created");
        }

        if (mMtkImsListener == null) {
            mMtkImsListener = new IMtkImsRegistrationListener.Stub() {
                @Override
                public void onRegistrationImsStateChanged(int state, Uri[] uris,
                        int expireTime, ImsReasonInfo imsReasonInfo) throws RemoteException {
                    log("onRegistrationImsStateChanged, state: " + state + ", uri: " + uris
                            + ", expireTime: " + expireTime + ", imsReasonInfo: " + imsReasonInfo);
                    synchronized (mCallbacks) {
                        for (ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                            if (callback instanceof MtkImsConnectionStateListener) {
                                ((MtkImsConnectionStateListener) callback)
                                        .onRegistrationImsStateInd(state, uris, expireTime,
                                        imsReasonInfo.getCode(), imsReasonInfo.getExtraMessage());
                            }
                        }
                    }
                }

                @Override
                public void onRedirectIncomingCallIndication(int phoneId, String[] info) {
                    log("redirectIncomingCallIndication, phoneId: " + phoneId +
                            ", info: " + info);
                    synchronized (mCallbacks) {
                        for (ImsMmTelManager.RegistrationCallback callback : mCallbacks) {
                            if (callback instanceof MtkImsConnectionStateListener) {
                                ((MtkImsConnectionStateListener) callback)
                                        .onRedirectIncomingCallInd(phoneId, info);
                            }
                        }
                    }
                }
            };
        } else {
            log ("mMtkListener was created");
        }

        try {
            if (mMtkImsService != null) {
                mMtkImsService.registerProprietaryImsListener(mPhoneId, mListener, mMtkImsListener,
                        mNotifyOnly);
                mNotifyOnly = true;
            } else {
                log("mMtkImsService is not ready yet");
            }
        } catch (RemoteException e) {
            throw new ImsException("registerProprietaryImsListener(listener)", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    private MmTelFeature.MmTelCapabilities convertCapabilities(int[] enabledFeatures) {
        boolean[] featuresEnabled = new boolean[enabledFeatures.length];
        for (int i = ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE;
                i <= ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI
                && i < enabledFeatures.length; i++) {
            if (enabledFeatures[i] == i) {
                featuresEnabled[i] = true;
            } else if (enabledFeatures[i] == ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN) {
                // FEATURE_TYPE_UNKNOWN indicates that a feature is disabled.
                featuresEnabled[i] = false;
            }
        }
        MmTelFeature.MmTelCapabilities capabilities = new MmTelFeature.MmTelCapabilities();
        if (featuresEnabled[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE]
                || featuresEnabled[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI]) {
            // voice is enabled
            capabilities.addCapabilities(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
        }
        if (featuresEnabled[ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE]
                || featuresEnabled[ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI]) {
            // video is enabled
            capabilities.addCapabilities(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
        }
        if (featuresEnabled[ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE]
                || featuresEnabled[ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI]) {
            // ut is enabled
            capabilities.addCapabilities(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT);
        }
        return capabilities;
    }

    @ProductApi
    public void addImsConnectionStateListener(
            ImsMmTelManager.RegistrationCallback callback) throws ImsException {

        synchronized (mCallbacks) {
            log("ImsConnectionStateListener added: " + callback);
            mCallbacks.add(callback);
            hookProprietaryImsListener();
        }
    }

    @ProductApi
    public void removeImsConnectionStateListener(
            ImsMmTelManager.RegistrationCallback callback) throws ImsException {

        synchronized (mCallbacks) {
            mCallbacks.remove(callback);
            log("ImsConnectionStateListener removed: " + callback
                    + ", size: " + mCallbacks.size());
        }
    }

    // For 3-SIM START
    // Modem 3-SIM rule: Only support IMS on the protocl stack id(1-base)  <=
    // persist.vendor.mims_support(0-base).
    //
    // The mapping table of phone id and protocol stack id in the case of
    // persist.vendor.mims_support[2]
    //                               |     SIM1         |     SIM2                 |       SIM3
    // ------------------------------------------------------------------------
    // Main protocl on SIM1 | Protocol 1(M) | Protocol 2               |   Protocol 3(w/o IMS)
    // Main protocl on SIM2 | Protocol 2      |  Protocol 1(M)         |   Protocol 3(w/o IMS)
    // Main protocl on SIM3 | Protocol 2      | Protocol 3(w/o IMS) |   Protocol 1(M)
    // Get the protocol stack id by slot id, the slot is 0-based, protocol stack id is 1-based
    // If the protocol stack id of phone id <= persist.vendor.mims_support,
    // return true. Otherwise, return false
    private boolean isPhoneIdSupportIms(int phoneId) {
        int isImsSupport = SystemProperties.getInt(PROPERTY_IMS_SUPPORT, 0);
        int mimsCount = SystemProperties.getInt(MULTI_IMS_SUPPORT, 1);
        TelephonyManager.MultiSimVariants config =
                TelephonyManager.getDefault().getMultiSimConfiguration();
        boolean result = false;

        if (config != TelephonyManager.MultiSimVariants.TSTS) {
            return true;
        }

        if (isImsSupport == 0 || !SubscriptionManager.isValidPhoneId(phoneId)) {
            log("[" + phoneId + "] isPhoneIdSupportIms, not support IMS");
            return result;
        }

        if (mimsCount == 1) {
            if (getMainCapabilityPhoneId(mContext) == phoneId) {
                result = true;
            }
        } else {
            int protocalStackId = MtkTelephonyManagerEx.getDefault().getProtocolStackId(phoneId);
            log("isPhoneIdSupportIms(), mimsCount:" + mimsCount + ", phoneId:" + phoneId  +
                    ", protocalStackId:" + protocalStackId + ", MainCapabilityPhoneId:"
                    + getMainCapabilityPhoneId(mContext));
            if (protocalStackId <= mimsCount) {
                result = true;
            }
        }
        return result;
    }
    // For 3-SIM END

    private boolean isCellularDataRoaming() {
        TelephonyManager tm = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        if (tm == null) {
            loge("isCellularDataRoaming(): TelephonyManager null");
            return false;
        }

        ServiceState ss = tm.getServiceStateForSubscriber(getSubId());
        if (ss == null) {
            loge("isCellularDataRoaming(): ServiceState null");
            return false;
        }

        if (ss instanceof MtkServiceState) {
            MtkServiceState mtkSs = (MtkServiceState) ss;
            int regState = mtkSs.getCellularDataRegState();
            boolean isDataroaming = mtkSs.getCellularDataRoaming();

            log("isCellularDataRoaming(): regState = " + regState
                    + ", isDataroaming = " + isDataroaming);
            if (regState == ServiceState.STATE_IN_SERVICE && isDataroaming) {
                return true;
            }
        } else {
            loge("isCellularDataRoaming(): not MtkServiceState");
        }

        return false;
    }

    // For VzW Client API
    public void setMTRedirect(boolean enable) throws ImsException {
        log("setMTRedirect: " + enable);

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            mMtkImsService.setMTRedirect(mPhoneId, enable);
        } catch (RemoteException e) {
            throw new ImsException("setMTRedirect()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    public void fallBackAospMTFlow() throws ImsException {
        log("fallBackAospMTFlow");

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            mMtkImsService.fallBackAospMTFlow(mPhoneId);
        } catch (RemoteException e) {
            throw new ImsException("fallBackAospMTFlow()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    public void setSipHeader(HashMap<String, String> extraHeaders, String fromUri)
            throws ImsException {
        log("setSipHeader fromUri: " + sensitiveEncode(fromUri) + ", extraHeaders: " + extraHeaders);

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            mMtkImsService.setSipHeader(mPhoneId, extraHeaders, fromUri);
        } catch (RemoteException e) {
            throw new ImsException("setSipHeader()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    public void setCallIndication(int phoneId, String callId, String callNum, int seqNum,
            String toNumber, boolean isAllow) throws ImsException {
        log("setCallIndication phoneId:" + phoneId + ", callId:" + callId + ",callNum:"
            + sensitiveEncode(callNum) + ",seqNum:" + seqNum + ",toNumber:"
            + sensitiveEncode(toNumber) + ",isAllow:" + isAllow);

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            mMtkImsService.setCallIndication(
                phoneId, callId, callNum, seqNum, toNumber, isAllow, -1);
        } catch (RemoteException e) {
            throw new ImsException("setCallIndication()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    public IMtkImsCallSession getPendingMtkCallSession(String callId)
            throws ImsException {
        log("getPendingMtkCallSession callId: " + callId);

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            return mMtkImsService.getPendingMtkCallSession(mPhoneId, callId);
        } catch (RemoteException e) {
            throw new ImsException("getPendingMtkCallSession()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    public ImsCallSession createCallSession(int serviceType, int callType)
            throws ImsException {
        try {
            ImsCallProfile profile = new ImsCallProfile(serviceType, callType);
            log("createCallSession: profile = " + profile);
            ImsCallSession imsCallSession = super.createCallSession(profile);
            log("createCallSession: imsCallSession = " + imsCallSession);
            log("createCallSession: imsCallSession.getSession() = " + imsCallSession.getSession());
            IMtkImsCallSession mtkImsCallSessionImpl =  mMtkImsService.createMtkCallSession(
                    mPhoneId, profile, null, imsCallSession.getSession());

            return new MtkImsCallSession(imsCallSession.getSession(), mtkImsCallSessionImpl);

        } catch (RemoteException e) {
            Rlog.w(TAG,
                    "CreateCallSession: Error, remote exception: "
                            + e.getMessage());
            throw new ImsException("createCallSession()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }

    protected void changeMmTelCapabilityInternally(CapabilityChangeRequest r) {
        try {
            if (mMtkImsService != null) {
                log("[" + mPhoneId + "] changeMmTelCapabilityInternally " + r);
                mMtkImsService.changeEnabledCapabilities(mPhoneId, r);
            }
        } catch (RemoteException e) {
            loge("Fail to changeMmTelCapabilityInternally " + e);
        }
    }

    public void setVdpProvisioned(int value) {
        try {
            Log.i(ImsManager.class.getSimpleName(), "Setting Vdp value to " + value);
            getConfigInterface().setProvisionedValue(
                    MtkImsConstants.ConfigConstants.VOICE_DOMAIN_PREFERENCE, value);
        } catch (ImsException e) {
            Log.e(ImsManager.class.getSimpleName(), "Unable to set Vdp value to "
                    + value + ": " + e);
        }
    }

    /**
     * Set pre-call info for dialing IMS call.
     * @param mode to indicate the purpose, 3 = RCS enriched calling
     * @param address prepare to dialing to
     * @param fromUri set the "from:" header in the SIP header
     * @param extraHeaders additional header for this call
     * @param location including latitude, longtitude and accuracy
     * @throws ImsException if calling the IMS service results in an error
     */
    public void setImsPreCallInfo(int mode, String address, String fromUri,
            HashMap<String, String> extraHeaders, String[] location) throws ImsException {
        log("setImsPreCallInfo mode : " + mode + ", address: " + sensitiveEncode(address) +
                ", fromUri: " + sensitiveEncode(fromUri) + ", extraHeaders: " + extraHeaders +
                ", location: " + location);

        checkAndThrowExceptionIfServiceUnavailable();

        try {
            mMtkImsService.setImsPreCallInfo(mPhoneId, mode, address, fromUri, extraHeaders,
                    location);
        } catch (RemoteException e) {
            throw new ImsException("setImsPreCallInfo()", e,
                    ImsReasonInfo.CODE_LOCAL_IMS_SERVICE_DOWN);
        }
    }
}
