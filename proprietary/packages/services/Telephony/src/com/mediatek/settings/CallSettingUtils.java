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

package com.mediatek.settings;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.preference.Preference;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.phone.GsmUmtsAdditionalCallOptions;
import com.android.phone.GsmUmtsCallBarringOptions;
import com.android.phone.GsmUmtsCallForwardOptions;
import com.android.phone.PhoneGlobals;
import com.android.phone.PhoneUtils;
import com.android.phone.R;
import com.android.phone.SubscriptionInfoHelper;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.settings.cdma.CdmaCallWaitingUtOptions;
import com.mediatek.settings.cdma.TelephonyUtilsEx;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import mediatek.telephony.MtkCarrierConfigManager;

/**
 * Call Setting Utils.
 * Only used for call setting related features.
 */
public class CallSettingUtils {
    private static final String TAG = "CallSettingUtils";

    ///Add for [Dual_Mic]
    private static final String DUALMIC_MODE = "Enable_Dual_Mic_Setting";
    private static final String GET_DUALMIC_MODE = "Get_Dual_Mic_Setting";
    private static final String DUALMIC_ENABLED = "Get_Dual_Mic_Setting=1";
    private static final String MTK_DUAL_MIC_SUPPORT = "MTK_DUAL_MIC_SUPPORT";
    private static final String MTK_DUAL_MIC_SUPPORT_on = "MTK_DUAL_MIC_SUPPORT=true";

    /// Add for [HAC]
    private static final String GET_HAC_SUPPORT = "GET_HAC_SUPPORT";
    private static final String GET_HAC_SUPPORT_ON = "GET_HAC_SUPPORT=1";
    private static final String GET_HAC_ENABLE = "GET_HAC_ENABLE";
    private static final String GET_HAC_ENABLE_ON = "GET_HAC_ENABLE=1";

    /// Add for dual volte feature
    private static final String ADDITIONAL_GSM_SETTINGS_KEY = "additional_gsm_call_settings_key";
    private static final String CALL_BARRING_KEY = "call_barring_key";
    private static final String CALL_FORWARDING_KEY = "call_forwarding_key";
    private static final String KEY_CALL_FORWARD = "button_cf_expand_key";

    // Sensitive log task: the default value is disable
    public static final boolean SENSITIVE_LOG = false;

    /**
     * Dialog type.
     */
    public enum DialogType {
        NONE,
        DATA_OPEN,
        DATA_TRAFFIC,
        DATA_ROAMING
    }

    /**
     * Operator id.
     */
    public enum OPID {
        OP01,   // CMCC
        OP02,   // CU
        OP09,   // CT
        OP12,   // Verizon
        OP117,  // Smartfren
        OP236   // USCC
    }

    private static final Map<OPID, List> mOPMap = new HashMap<OPID, List>() {
        {
            put(OPID.OP01, Arrays.asList("46000", "46002", "46004", "46007", "46008"));
            put(OPID.OP02, Arrays.asList("46001", "46006", "46009", "45407"));
            put(OPID.OP09, Arrays.asList("45502", "46003", "46011", "46012", "45507")); //20404
            put(OPID.OP12, Arrays.asList("311480"));
            put(OPID.OP117, Arrays.asList("51009", "51028"));
            put(OPID.OP236, Arrays.asList("31100", "311220", "311221", "311222", "311223", "311224",
                    "311225", "311226", "311227", "311228", "311229", "311580", "311581", "311582",
                    "311583", "311584", "311585", "311586", "311587", "311588", "311589"));
        }
    };

    /// Add for FDN feature
    private static final int GET_PIN_PUK_RETRY_EMPTY = -1;
    public static final int GET_PIN_PUK_RETRY_INVALID_COUNT = 255;
    private static final String[] PROPERTY_SIM_PIN2_RETRY = {
        "vendor.gsm.sim.retry.pin2",
        "vendor.gsm.sim.retry.pin2.2",
        "vendor.gsm.sim.retry.pin2.3",
        "vendor.gsm.sim.retry.pin2.4",
    };
    private static final String PROPERTY_SIM_PUK2_RETRY[] = {
        "vendor.gsm.sim.retry.puk2",
        "vendor.gsm.sim.retry.puk2.2",
        "vendor.gsm.sim.retry.puk2.3",
        "vendor.gsm.sim.retry.puk2.4",
    };

    private static final String OPERATOR_OP09 = "OP09";
    private static final String SEGC = "SEGC";

    /**
     * Return if the sim card is special operator.
     * @param subId sub id identify the sim card
     * @param id the id of special operator
     * @return true if the sim card is special operator
     */
    public static boolean isOperator(int subId, OPID id) {
        boolean r = false;
        String mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
        if (mOPMap.get(id).contains(mccMnc)) {
            r = true;
        }
        //Log.d(TAG, "subId: " + subId + ", " + mccMnc + (r ? " = " : " != ") + idToString(id));
        return r;
    }

    /**
     * Return if the sim card is special operator.
     * @param mccMnc the sim card of special operator
     * @param id the id of special operator
     * @return true if the sim card is special operator
     */
    public static boolean isOperator(String mccMnc, OPID id) {
        boolean r = false;
        if (mOPMap.get(id).contains(mccMnc)) {
            r = true;
        }
        //Log.d(TAG, "" + mccMnc + (r ? " = " : " != ") + idToString(id));
        return r;
    }

    private static String idToString(OPID id) {
        if (id == OPID.OP01) {
            return "OP01";
        } else if (id == OPID.OP02) {
            return "OP02";
        } else if (id == OPID.OP09) {
            return "OP09";
        } else if (id == OPID.OP12) {
            return "OP12";
        } else if (id == OPID.OP117) {
            return "OP117";
        }
        return "ERR";
    }

    /**
     * Return if the sim card is cmcc or cu.
     * @param subId sub id identify the sim card
     * @return true if the sim card is cmcc or cu
     */
    public static boolean isCmccOrCuCard(int subId) {
        String mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
        return isOperator(mccMnc, OPID.OP01) || isOperator(mccMnc, OPID.OP02);
    }

    /**
     * Returns the MCC+MNC (mobile country code + mobile network code) of the
     * provider of the SIM for a particular subscription. 5 or 6 decimal digits.
     *
     * @param phone for which SimOperator is returned
     * @return the MCC+MNC
     */
    public static String getSimOperatorForPhone(Phone phone) {
        String mccMncPropertyName = "";
        String mccMnc = "";
        int phoneType = phone.getPhoneType();
        int phoneId = phone.getPhoneId();
        log("getPhoneType: " + phoneType + ", getPhoneId: " + phoneId);

        if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            if (phoneId == 0) {
                mccMncPropertyName = "vendor.cdma.ril.uicc.mccmnc";
            } else {
                mccMncPropertyName = "vendor.cdma.ril.uicc.mccmnc." + phoneId;
            }
        } else {
            //  Regard as PhoneConstants.PHONE_TYPE_GSM
            if (phoneId == 0) {
                mccMncPropertyName = "vendor.gsm.ril.uicc.mccmnc";
            } else {
                mccMncPropertyName = "vendor.gsm.ril.uicc.mccmnc." + phoneId;
            }
        }
        mccMnc = SystemProperties.get(mccMncPropertyName, "");

        log("getSimOperatorForPhone: " + mccMnc);
        return mccMnc;
    }

    /**
     * Return if the sim card is ct.
     * @param subId sub id identify the sim card
     * @return true if the sim card is ct
     */
    public static boolean isCtSim(int subId) {
        return isOperator(subId, OPID.OP09);
    }

    /**
     * Check whether ct volte normal or mix is enable.
     * @return true if all the conditions are satisfied
     */
    public static boolean isCtVolte() {
        boolean result = false;
        String feature = SystemProperties.get("persist.vendor.mtk_ct_volte_support");
        if (feature.equals("1") || feature.equals("2") || feature.equals("3")) {
            result = true;
        }
        log("isCtVolte: " + result + " feature: " + feature);
        return result;
    }

    /**
     * Check whether ct volte mix is enable.
     * @return true if all the conditions are satisfied
     */
    public static boolean isCtVolteMix() {
        // 1 is ct volte normal case, volte and cdma can not coexist.
        // 2 is ct volte mix case, volte and cdma can coexist.
        // 3 is customer special requirement. If device registered on volte, the phone type is gsm.
        // If device does not registered on volte, the phone type is cdma.
        boolean result = false;
        String feature = SystemProperties.get("persist.vendor.mtk_ct_volte_support");
        if (feature.equals("2") || feature.equals("3")) {
            result = true;
        }
        log("isCtVolteMixEnabled: " + result);
        return result;
    }

    /**
     * Check whether UT prefer for cdma sim operators.
     * @param subId the given subId
     * @return true if all the conditions are satisfied
     */
    public static boolean isUtPreferCdmaSim(int subId) {
        String mccMnc = TelephonyManager.getDefault().getSimOperator(subId);
        return (isCtVolte() && isOperator(mccMnc, OPID.OP09) &&
                TelephonyUtilsEx.isCdma4gCard(subId)) || isOperator(mccMnc, OPID.OP117);
    }

    /**
     * Check whether ct volte mix is enable and is ct 4g sim.
     * @param subId the given subId
     * @return true if all the conditions are satisfied
     */
    public static boolean isCtVolteMix4gSim(int subId) {
        return isCtVolteMix() && isOperator(subId, OPID.OP09) &&
                TelephonyUtilsEx.isCdma4gCard(subId);
    }

    /**
     * Check whether ct volte is enable and it is ct 4g sim.
     * @param subId the given subId
     * @return true if all the conditions are satisfied
     */
    public static boolean isCtVolte4gSim(int subId) {
        boolean result = false;
        if (!isCtVolte()) {
            return result;
        }
        CardType cardType = MtkTelephonyManagerEx.getDefault().getCdmaCardType(
                SubscriptionManager.getSlotIndex(subId));
        if (cardType != null) {
            if ((MtkIccCardConstants.CardType.CT_4G_UICC_CARD).equals(cardType)) {
                result = true;
            }
        }
        log("isCtVolte4gSim: " + result);
        return result;
    }

    /**
     * Return if the sim card supports UT.
     * @param subId sub id identify the sim card
     * @return true if the sim card supports UT
     */
    public static boolean isUtSupport(int subId) {
        boolean result = false;
        if (isCmccOrCuCard(subId) && TelephonyUtils.isUSIMCard(
                PhoneGlobals.getInstance().getApplicationContext(), subId)) {
            result = true;
        } else if (isUtPreferCdmaSim(subId)) {
            // [CT VOLTE]
            result = true;
        }
        return result;
    }

    /**
     * Return if the sim card is UT prefer only by cdma sim.
     * @param subId sub id identify the sim card
     * @return true if all the conditions are satisfied
     */
    public static boolean isUtPreferOnlyByCdmaSim(int subId) {
        return (isCtVolte() && isOperator(subId, OPID.OP09) &&
                TelephonyUtilsEx.isCdma4gCard(subId));
    }

    /**
     * Return if the sim card is UT prefer only by cdma sim.
     * @param subId sub id identify the sim card
     * @param phone for which SimOperator is returned
     * @return true if all the conditions are satisfied
     */
    public static boolean isUtPreferOnlyByCdmaSim(int subId, Phone phone) {
        boolean result = false;
        if (isCtVolte()) {
            String mccMnc = getSimOperatorForPhone(phone);
            result = (isOperator(mccMnc, OPID.OP09) && TelephonyUtilsEx.isCdma4gCard(subId));
        }
        return result;
    }

    /**
     * Return if the sim card is UT prefer by cdma sim and IMS available.
     * @param context Context
     * @param subId sub id identify the sim card
     * @param hasImsState has ims state or not
     * @param isImsOn is ims state on or not
     * @return true if all the conditions are satisfied
     */
    public static boolean isUtPreferByCdmaSimAndImsOn(Context context, int subId,
            boolean hasImsState, boolean isImsOn) {
        boolean result = false;
        if (hasImsState) {
            result = isOperator(subId, OPID.OP117) && isImsOn;
        } else {
            result = isOperator(subId, OPID.OP117) &&
                    TelephonyUtils.isImsServiceAvailable(context, subId);
        }
        return result;
    }

    /**
     * Get DualMic feature is supported or not.
     * @return true if dual mic mode is supported
     */
    public static boolean isMtkDualMicSupport() {
        String state = null;
        AudioManager audioManager = (AudioManager)
                PhoneGlobals.getInstance().getSystemService(Context.AUDIO_SERVICE);
        if (audioManager != null) {
            state = audioManager.getParameters(MTK_DUAL_MIC_SUPPORT);
            log("isMtkDualMicSupport: " + state);
            if (state.equalsIgnoreCase(MTK_DUAL_MIC_SUPPORT_on)) {
                return true;
            }
        }
        return false;
    }

    /**
     * set DualMic noise reduction mode.
     * @param dualMic the value to show the user set
     */
    public static void setDualMicMode(String dualMic) {
        Context context = PhoneGlobals.getInstance().getApplicationContext();
        if (context == null) {
            return;
        }
        AudioManager audioManager = (AudioManager)
                context.getSystemService(Context.AUDIO_SERVICE);
        audioManager.setParameters(DUALMIC_MODE + "=" + dualMic);
    }

    /**
     * Get DualMic noise reduction mode.
     * @return true if dual mic mode is enabled
     */
    public static boolean isDualMicModeEnabled() {
        Context context = PhoneGlobals.getInstance().getApplicationContext();
        if (context == null) {
            return false;
        }
        String state = null;
        AudioManager audioManager = (AudioManager)
                context.getSystemService(Context.AUDIO_SERVICE);
        if (audioManager != null) {
            state = audioManager.getParameters(GET_DUALMIC_MODE);
            log("getDualMicMode(): state: " + state);
            if (state.equalsIgnoreCase(DUALMIC_ENABLED)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Get HAC's state. For upgrade issue.
     * In KK we don't use DB, so we still need use query Audio State to sync with DB.
     * @return 1, HAC enable; 0, HAC disable.
     */
    public static int isHacEnable() {
        Context context = PhoneGlobals.getInstance().getApplicationContext();
        if (context == null) {
            log("isHacEnable : context is null");
            return 0;
        }
        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (audioManager != null) {
            String hac = audioManager.getParameters(GET_HAC_ENABLE);
            log("hac enable: " + hac);
            return GET_HAC_ENABLE_ON.equals(hac) ? 1 : 0;
        }
        log("isHacEnable : audioManager is null");
        return 0;
    }

    /**
     * Add for HAC(hearing aid compatible).
     * if return true, support HAC ,show UI. otherwise, disappear.
     * @return true, support. false, not support.
     */
    public static boolean isHacSupport() {
        Context context = PhoneGlobals.getInstance().getApplicationContext();
        if (context == null) {
            return false;
        }
        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (audioManager != null) {
            String hac = audioManager.getParameters(GET_HAC_SUPPORT);
            log("hac support: " + hac);
            return GET_HAC_SUPPORT_ON.equals(hac);
        }
        return false;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    /**
     * Check phonebook is ready or not.
     * @param context Context
     * @param subId sub id identify the sim card
     * @return true if phonebook is ready.
     */
    public static boolean isPhoneBookReady(Context context, int subId) {
        final IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx")); //TODO: Use Context.TELEPHONY_SERVICEEX
        boolean isPhoneBookReady = false;
        try {
            isPhoneBookReady = telephonyEx.isPhbReady(subId);
            Log.d(TAG, "isPhoneBookReady:" + isPhoneBookReady + ", subId:" + subId);
        } catch (RemoteException e) {
            Log.e(TAG, "isPhoneBookReady catch exception:");
            e.printStackTrace();
        }
        if (!isPhoneBookReady) {
            Toast.makeText(context,
                    context.getString(R.string.fdn_phone_book_busy), Toast.LENGTH_SHORT).show();
        }
        return isPhoneBookReady;
    }

    /**
     * Get the SIM card's mobile data connection status, which is inserted in the given sub.
     * @param subId the given subId
     * @param context
     * @return true, if enabled, else false.
     */
    public static boolean isMobileDataEnabled(int subId) {
        if (PhoneUtils.isValidSubId(subId)) {
            boolean isDataEnable = PhoneUtils.getPhoneUsingSubId(subId).isUserDataEnabled();
            log("isMobileDataEnabled: " + isDataEnable);
            return isDataEnable;
        }
        log("isMobileDataEnabled invalid subId: " + subId);
        return false;
    }

    /**
     * This function is get the point, whether we should show a tip to user. Conditions:
     * 1. VoLTE condition / Support UT, no mater IMS enable/not
     * 2. Mobile Data Connection is not enable for singe volte
     * 3. Data traffic dialog for dual volte
     * 4. Data roaming should be open or not
     * @param context Context
     * @param subId the given subId
     * @return Dialog Type if should show tip.
     */
    public static DialogType getDialogTipsType(Context context, int subId) {
        DialogType result = DialogType.NONE;
        if (!PhoneUtils.isValidSubId(subId)) {
            log("getDialogTipsType invalid subId: " + subId);
            return result;
        }

        PersistableBundle carrierConfig =
                   PhoneGlobals.getInstance().getCarrierConfigForSubId(subId);
        /// M: Some operator no need show open mobile data dialog @{
        if (!carrierConfig.
                getBoolean(MtkCarrierConfigManager.MTK_KEY_SHOW_OPEN_MOBILE_DATA_DIALOG_BOOL)) {
            return result;
        }
        /// @}

        // 1. IMS is registered, VoLTE condition
        // 2. Support UT and PS prefer (CNOP VoLTE)
        // 3. CT VoLTE enable and is CT 4G sim, the condition 2 can support
        // 4. Smart Fren sim, the condition 1 can support
        if (TelephonyUtils.isImsServiceAvailable(context, subId) || isUtSupport(subId)) {
            log("getDialogTipsType: ss query need mobile data connection!");
            ///M: When wfc registered, no need check mobile data because SS can go over wifi. @{
            boolean isWfcEnabled = ((TelephonyManager) context
                    .getSystemService(Context.TELEPHONY_SERVICE)).isWifiCallingAvailable();
            if (isWfcEnabled && !isCmccOrCuCard(subId)) {
                return result;
            }
            /// @}

            boolean isMobileDataAvailable = isMobileDataEnabled(subId);
            boolean isDataTraffic = ((subId != SubscriptionManager.getDefaultDataSubscriptionId())
                    || !isMobileDataAvailable);
            boolean isMims = MtkImsManager.isSupportMims();

            // Data should open or not for singe volte.
            if (!isMobileDataAvailable && !isMims) {
                result = DialogType.DATA_OPEN;
            /// M: Add for dual volte feature, data traffic dialog @{
            } else if (isDataTraffic && isMims) {
                result = DialogType.DATA_TRAFFIC;
            /// @}
            } else if (!MtkTelephonyManagerEx.getDefault().isInHomeNetwork(subId)
                // is Network Roaming and special operator card
                && isCmccOrCuCard(subId)
                && !PhoneUtils.getPhoneUsingSubId(subId).getDataRoamingEnabled()) {
                //M: Add for data roaming tips
                //When CMCC Network is Roaming and data roaming not enabled,
                //we should also give user tips to turn it on.
                log("getDialogTipsType: network is roaming!");
                result = DialogType.DATA_ROAMING;
            }
        }
        log("getDialogTipsType subId: " + subId + ",result: " + result);
        return result;
    }

    /**
     * Show a tip dialog according to the dialog type.
     * @param context Context
     * @param subId the given subId
     * @param type Dialog Type
     * @param preference Preference
     */
    public static void showDialogTips(final Context context, int subId, DialogType type,
            Preference preference) {
        Intent intent = null;

        if (type == DialogType.DATA_TRAFFIC) {
            log("showDialogTips preference: " + preference);
            if (preference != null) {
                if (preference.getKey().equals(CALL_FORWARDING_KEY)
                        || (preference.getKey().equals(KEY_CALL_FORWARD))) {
                    intent = new Intent(context, GsmUmtsCallForwardOptions.class);
                } else if (preference.getKey().equals(CALL_BARRING_KEY)) {
                    intent = new Intent(context, GsmUmtsCallBarringOptions.class);
                } else if (preference.getKey().equals(ADDITIONAL_GSM_SETTINGS_KEY)) {
                    intent = new Intent(context, GsmUmtsAdditionalCallOptions.class);
                } else {
                    intent = new Intent(context, CdmaCallWaitingUtOptions.class);
                }
                SubscriptionInfoHelper.addExtrasToIntent(intent, MtkSubscriptionManager
                        .getSubInfo(null, subId));
            }
        }
        MobileDataDialogFragment.show(intent, type, subId,
                ((Activity) context).getFragmentManager());
    }

    private static boolean isOP09CSupport() {
        boolean result = OPERATOR_OP09.equals(
                SystemProperties.get("persist.vendor.operator.optr", "")) &&
                SEGC.equals(SystemProperties.get("persist.vendor.operator.seg", ""));
        Log.d(TAG, "isOP09CSupport: " + result);
        return result;
    }

    /**
     * Check whether we should show data traffic dialog.
     * @param subId the given subId
     * @return true if the conditions are satisfied
     */
    public static boolean shouldShowDataTrafficDialog(int subId) {
        return MtkImsManager.isSupportMims() && isOP09CSupport() && !isMobileDataEnabled(subId);
    }

    /**
     * Get pin2 left retry times.
     * @param subId the sub which one user want to get
     * @return the left times
     */
    public static int getPin2RetryNumber(int subId) {
        if (!PhoneUtils.isValidSubId(subId)) {
            log("getPin2RetryNumber invalid subId: " + subId);
            return GET_PIN_PUK_RETRY_EMPTY;
        }
        int slot = SubscriptionManager.getSlotIndex(subId);
        log("getPin2RetryNumber subId: " + subId + ",Slot: " + slot);
        String pin2RetryStr = null;
        try {
        if (TelephonyUtils.isGeminiProject()) {
            if (slot < PROPERTY_SIM_PIN2_RETRY.length) {
                pin2RetryStr = PROPERTY_SIM_PIN2_RETRY[slot];
            } else {
                Log.w(TAG, "getPin2RetryNumber: Error happened!");
                pin2RetryStr = PROPERTY_SIM_PIN2_RETRY[0];
            }
        } else {
            pin2RetryStr = PROPERTY_SIM_PIN2_RETRY[0];
        }
        } catch (ArrayIndexOutOfBoundsException e) {
            log("getPin2RetryNumber exception: " + e.getMessage());
            pin2RetryStr = PROPERTY_SIM_PIN2_RETRY[0];
        }
        return SystemProperties.getInt(pin2RetryStr, GET_PIN_PUK_RETRY_EMPTY);
    }

    /**
     * Get the pin2 retry tips messages.
     * @param context Context
     * @param subId the given subId
     * @param isPin pin2 or puk
     * @return String of tips
     */
    public static String getPinPuk2RetryLeftNumTips(Context context, int subId, boolean isPin) {
        if (!PhoneUtils.isValidSubId(subId)) {
            log("getPinPuk2RetryLeftNumTips inValid SubId: " + subId);
            return " ";
        }
        int retryCount = GET_PIN_PUK_RETRY_EMPTY;
        if (isPin) {
            retryCount = getPin2RetryNumber(subId);
        } else {
            retryCount = getPuk2RetryNumber(subId);
        }
        log("getPinPuk2RetryLeftNumTips retry count: " + retryCount + ",isPin: " + isPin);
        switch (retryCount) {
            case GET_PIN_PUK_RETRY_EMPTY:
                return " ";
            default:
                return context.getString(R.string.retries_left, retryCount);
        }
    }

    /**
     * Get puk2 left retry times.
     * @param subId the sub which one user want to get
     * @return the left times
     */
    public static int getPuk2RetryNumber(int subId) {
        if (!PhoneUtils.isValidSubId(subId)) {
            log("getPuk2RetryNumber inValid SubId: " + subId);
            return -1;
        }
        int slot = SubscriptionManager.getSlotIndex(subId);
        log("getPuk2RetryNumber subId: " + subId + ",Slot: " + slot);
        String puk2RetryStr;
        if (TelephonyUtils.isGeminiProject()) {
            if (slot < PROPERTY_SIM_PIN2_RETRY.length) {
                puk2RetryStr = PROPERTY_SIM_PUK2_RETRY[slot];
            } else {
                Log.w(TAG, "getPuk2RetryNumber: Error happened!");
                puk2RetryStr = PROPERTY_SIM_PUK2_RETRY[0];
            }
        } else {
            puk2RetryStr = PROPERTY_SIM_PUK2_RETRY[0];
        }
        return SystemProperties.getInt(puk2RetryStr, GET_PIN_PUK_RETRY_EMPTY);
    }

    /**
     * To safe string for sensitive log if DBGLOG is not open.
     * @param tag the log tag
     * @param message the log message
     * @param sensitiveLog the sensitive log
     */
    public static void sensitiveLog(String tag, String message, String sensitiveLog) {
        if (SENSITIVE_LOG) {
            Log.d(tag, message + sensitiveLog);
        } else {
            Log.d(tag, message + toSafeString(sensitiveLog));
        }
    }

    /**
     * To safe string for sensitive log.
     * @param sensitiveLog the sensitive log
     * @return the safe string
     */
    public static String toSafeString(String sensitiveLog) {
        // log empty string.
        if (sensitiveLog == null) {
            return "";
        }

        StringBuilder builder = new StringBuilder();
        int len = sensitiveLog.length();
        for (int i = 0; i < len; i++) {
            builder.append('x');
        }
        return builder.toString();
    }
}
