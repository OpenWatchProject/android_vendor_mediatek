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

package com.mediatek.settings.sim;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;

public class TelephonyUtils {
    private static boolean DBG =
            SystemProperties.get("ro.build.type").equals("eng") ? true : false;
    private static final String TAG = "TelephonyUtils";

    /// M: Define the SIM lock policy/valid/capability. @{
    public static final int SIM_LOCK_POLICY_UNKNOWN =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_UNKNOWN;
    public static final int SIM_LOCK_POLICY_NONE =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_NONE;
    public static final int SIM_LOCK_POLICY_SLOT1_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT1;
    public static final int SIM_LOCK_POLICY_SLOT2_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT2;
    public static final int SIM_LOCK_POLICY_SLOT_ALL =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL;
    public static final int SIM_LOCK_POLICY_SLOT1_LINKED =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT1;
    public static final int SIM_LOCK_POLICY_SLOT2_LINKED =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT2;
    public static final int SIM_LOCK_POLICY_SLOT_ALL_LINKED =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA;
    public static final int SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS;
    public static final int SIM_LOCK_POLICY_SLOT_ALL_LINKED_REVERSE =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE;
    public static final int SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_ECC = MtkIccCardConstants
            .SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE;
    public static final int SIM_LOCK_POLICY_LEGACY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LEGACY;

    public static final int SIM_LOCK_SIM_VALID_UNKNOWN =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_UNKNOWN;
    public static final int SIM_LOCK_SIM_VALID_YES =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_YES;
    public static final int SIM_LOCK_SIM_VALID_NO =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_NO;
    public static final int SIM_LOCK_SIM_VALID_ABSENT =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_VALID_CARD_ABSENT;

    public static final int SIM_LOCK_SIM_CAPABILITY_UNKNOWN =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_UNKNOWN;
    public static final int SIM_LOCK_SIM_CAPABILITY_FULL =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL;
    public static final int SIM_LOCK_SIM_CAPABILITY_CS_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_CS_ONLY;
    public static final int SIM_LOCK_SIM_CAPABILITY_PS_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY;
    public static final int SIM_LOCK_SIM_CAPABILITY_ECC_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_ECC_ONLY;
    public static final int SIM_LOCK_SIM_CAPABILITY_NO_SERVICE =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_NO_SERVICE;

    public static final int SIM_LOCK_CASE_UNKNOWN = -1;
    public static final int SIM_LOCK_CASE_NONE = 0;
    public static final int SIM_LOCK_CASE_ALL_EMPTY = 1; // all empty
    public static final int SIM_LOCK_CASE_ALL_UNKNOWN = 2; // all unknown
    public static final int SIM_LOCK_CASE_ALL_VALID = 3; // all valid
    public static final int SIM_LOCK_CASE_ALL_INVALID = 4; // all invalid
    public static final int SIM_LOCK_CASE_INVALID_1_VALID = 5; // invalid + single valid
    public static final int SIM_LOCK_CASE_INVALID_N_VALID = 6; // invalid + multiple valid
    public static final int SIM_LOCK_CASE_UNKNOWN_INVALID = 7; // unknown + invalid
    public static final int SIM_LOCK_CASE_UNKNOWN_1_VALID = 8; // unknown + single valid
    public static final int SIM_LOCK_CASE_UNKNOWN_N_VALID = 9; // unknown + multiple valid
    // unknown + invalid + single valid
    public static final int SIM_LOCK_CASE_UNKNOWN_INVALID_1_VALID = 10;
    // unknown + invalid + multiple valid
    public static final int SIM_LOCK_CASE_UNKNOWN_INVALID_N_VALID = 11;
    /// @}

    /// M: Add for SIM On/Off feature. @{
    public static final int SIM_ONOFF_STATE_ON = MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON;
    public static final int SIM_ONOFF_STATE_OFF = MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_OFF;
    public static final int SIM_ONOFF_SET_SUCCESS = MtkTelephonyManagerEx.SET_SIM_POWER_SUCCESS;
    public static final int SIM_ONOFF_SWITCHING_ON =
            MtkTelephonyManagerEx.SIM_POWER_STATE_EXECUTING_SIM_ON;
    public static final int SIM_ONOFF_SWITCHING_OFF =
            MtkTelephonyManagerEx.SIM_POWER_STATE_EXECUTING_SIM_OFF;
    /// @}

    /**
     * Get whether airplane mode is in on.
     * @param context Context.
     * @return True for on.
     */
    public static boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    /**
     * Calling API to get subId is in on.
     * @param subId Subscribers ID.
     * @return {@code true} if radio on
     */
    public static boolean isRadioOn(int subId, Context context) {
        ITelephony phone = ITelephony.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICE));
        boolean isOn = false;
        if (phone != null) {
            try {
                isOn = subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID ? false
                        : phone.isRadioOnForSubscriber(subId, context.getPackageName());
            } catch (RemoteException e) {
                Log.e(TAG, "isRadioOn, RemoteException=" + e);
            }
        } else {
            Log.e(TAG, "isRadioOn, ITelephony is null.");
        }
        log("isRadioOn=" + isOn + ", subId=" + subId);
        return isOn;
    }

    /**
     * capability switch.
     * @return true : switching.
     */

    public static boolean isCapabilitySwitching() {
        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        boolean isSwitching = false;
        if (iTelEx != null) {
            try {
                isSwitching = iTelEx.isCapabilitySwitching();
            } catch (RemoteException e) {
                Log.e(TAG, "isCapabilitySwitching, RemoteException=" + e);
            }
        } else {
            log("isCapabilitySwitching, IMtkTelephonyEx service not ready.");
        }
        log("isSwitching=" + isSwitching);
        return isSwitching;
    }

    private static void log(String msg){
        if (DBG) {
            Log.d(TAG, msg);
        }
    }

    public static boolean isInCall() {
        TelephonyManager tm = TelephonyManager.getDefault();
        if (tm == null) {
            return false;
        }

        int phoneCount = tm.getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            int callState = tm.getCallStateForSlot(i);
            if (callState != TelephonyManager.CALL_STATE_IDLE) {
                Log.d(TAG, "isInCall, SIM" + (i + 1) + " callState=" + callState);
                return true;
            }
        }
        return false;
    }

    /**
     * Get the phone id with main capability.
     */
    public static int getMainCapabilityPhoneId() {
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (iTelEx != null) {
            try {
                phoneId = iTelEx.getMainCapabilityPhoneId();
            } catch (RemoteException e) {
                log("getMainCapabilityPhoneId, RemoteException=" + e);
            }
        } else {
            log("getMainCapabilityPhoneId, IMtkTelephonyEx service not ready.");
            phoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        }
        return phoneId;
    }

    /**
     * Check whether the MTK LTE feature is supported or not.
     */
    private static boolean sLteSupported = RatConfiguration.isLteFddSupported()
                || RatConfiguration.isLteTddSupported();
    public static boolean isMtkLteSupported() {
        boolean isSupport = sLteSupported;
        log("isMtkLteSupported, isSupport=" + isSupport);
        return isSupport;
    }

    /**
     * Check whether the MTK C2K feature is supported or not.
     */
    private static boolean sC2kSupported = RatConfiguration.isC2kSupported();
    public static boolean isMtkC2kSupported() {
        boolean isSupport = sC2kSupported;
        log("isMtkC2kSupported, isSupport=" + isSupport);
        return isSupport;
    }

    /**
     * Check whether the CTVoLTE feature is enabled or not.
     */
    private static String sCtVolteSupport = SystemProperties.get("persist.vendor.mtk_ct_volte_support");
    public static boolean isCtVolteEnabled() {
        String volteValue = sCtVolteSupport;
        boolean result = (volteValue.equals("1") || volteValue.equals("2")
                || volteValue.equals("3"));
        log("isCtVolteEnabled, volteValue=" + volteValue + ", enabled=" + result);
        return result;
    }

    /**
     * Check whether the CT Auto VoLTE feature is enabled or not.
     */
    public static boolean isCtAutoVolteEnabled() {
        String volteValue = sCtVolteSupport;
        boolean result = (volteValue.equals("2") || volteValue.equals("3"));
        log("isCtAutoVolteEnabled, volteValue=" + volteValue + ", enalbed=" + result);
        return result;
    }

    /**
     * Check whether the SIM is ready.
     */
    public static boolean isSimStateReady(int subId) {
        int simState = TelephonyManager.getDefault().getSimState(
                SubscriptionManager.getPhoneId(subId));
        boolean isReady = (simState == TelephonyManager.SIM_STATE_READY);
        log("isSimStateReady, isReady=" + isReady + ", simState=" + simState);
        return isReady;
    }

    /// M: Add for revising the SIM operator numeric. @{
    public static String getSimOperatorNumeric(int subId, TelephonyManager tm) {
        String mccmnc = "";
        int phoneId = SubscriptionManager.getPhoneId(subId);
        boolean cdma4gDualModeCard = false;
        CardType cdmaCardType = MtkTelephonyManagerEx.getDefault().getCdmaCardType(phoneId);

        if (cdmaCardType != null) {
            switch (cdmaCardType) {
                case CT_4G_UICC_CARD:
                case NOT_CT_UICC_CARD:
                    cdma4gDualModeCard = true;
                    break;

                default:
                    break;
            }
        }

        if (cdma4gDualModeCard) {
            String[] numeric = MtkTelephonyManagerEx.getDefault()
                    .getSimOperatorNumericForPhoneEx(phoneId);
            if (numeric != null && numeric[0] != null) {
                mccmnc = numeric[0];
            }
        } else {
            mccmnc = tm.getSimOperator(subId);
        }

        log("getSimOperatorNumeric, subId=" + subId + ", numeric=" + mccmnc
                + ", cdmaCardType=" + (cdmaCardType == null ? "null" : cdmaCardType)
                + ", cdma4gDualModeCard=" + cdma4gDualModeCard);

        return mccmnc;
    }
    /// @}

    public static boolean shouldEnableMobileNetworkOption(Context context, int subId) {
        boolean inCall = isInCall();
        boolean radioOn = isRadioOn(subId, context);
        int simState = getSimOnOffState(SubscriptionManager.getPhoneId(subId));
        boolean enabled = !inCall && radioOn
                && (simState == TelephonyUtils.SIM_ONOFF_STATE_ON);
        if (!enabled) {
            Log.d(TAG, "shouldEnableMobileNetworkOption, subId=" + subId
                    + ", enabled=" + enabled + ", inCall=" + inCall
                    + ", radioOn=" + radioOn + ", simState=" + simState);
        }
        return enabled;
    }

    /// M: Get the SIM lock policy/valid/capability. @{
    /**
     * Get the SIM lock policy.
     * @return the policy value.
     */
    public static int getSimLockPolicy() {
        int policy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        return policy;
    }

    /**
     * Get the SIM validity in SIM lock feature.
     * @param slotId the SIM slot ID.
     * @return the SIM validity.
     */
    public static int getSimLockSimValid(int slotId) {
        int valid = MtkTelephonyManagerEx.getDefault().checkValidCard(slotId);
        return valid;
    }

    /**
     * Get the SIM capability in SIM lock feature.
     * @param slotId the SIM slot ID.
     * @return the SIM capability.
     */
    public static int getSimLockSimCapability(int slotId) {
        int capa = MtkTelephonyManagerEx.getDefault().getShouldServiceCapability(slotId);
        return capa;
    }

    /**
     * Get the SIM case in SIM lock feature.
     * @param simSlotNum the SIM slot number.
     * @param simInserted the SIM inserted state array.
     * @param simValid the SIM validity array.
     * @return the SIM case ID.
     */
    public static int getSimLockCase(int simSlotNum, boolean[] simInserted,
            int[] simValid) {
        int countInserted = 0;
        int countUnknown = 0;
        int countValid = 0;
        int countInvalid = 0;
        int caseId = SIM_LOCK_CASE_NONE;

        for (int i = 0; i < simSlotNum; i++) {
            if (simInserted[i]) {
                countInserted++;
                switch (simValid[i]) {
                    case TelephonyUtils.SIM_LOCK_SIM_VALID_UNKNOWN:
                        countUnknown++;
                        break;

                    case TelephonyUtils.SIM_LOCK_SIM_VALID_NO:
                        countInvalid++;
                        break;

                    case TelephonyUtils.SIM_LOCK_SIM_VALID_YES:
                        countValid++;
                        break;

                    default:
                        countInserted--;
                        break;
                }
            }
        }

        if (countInserted == 0) {
            caseId = SIM_LOCK_CASE_ALL_EMPTY;
        } else if (countUnknown == countInserted) {
            caseId = SIM_LOCK_CASE_ALL_UNKNOWN;
        } else if (countValid == countInserted) {
            caseId = SIM_LOCK_CASE_ALL_VALID;
        } else if (countInvalid == countInserted) {
            caseId = SIM_LOCK_CASE_ALL_INVALID;
        } else if (countUnknown == 0) {
            if (countValid == 1) {
                caseId = SIM_LOCK_CASE_INVALID_1_VALID;
            } else {
                caseId = SIM_LOCK_CASE_INVALID_N_VALID;
            }
        } else if (countValid == 0) {
            caseId = SIM_LOCK_CASE_UNKNOWN_INVALID;
        } else if (countInvalid == 0) {
            if (countValid == 1) {
                caseId = SIM_LOCK_CASE_UNKNOWN_1_VALID;
            } else {
                caseId = SIM_LOCK_CASE_UNKNOWN_N_VALID;
            }
        } else if (countValid == 1) {
            caseId = SIM_LOCK_CASE_UNKNOWN_INVALID_1_VALID;
        } else {
            caseId = SIM_LOCK_CASE_UNKNOWN_INVALID_N_VALID;
        }

        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < simSlotNum; i++) {
            builder.append(", insert[" + i + "]=" + simInserted[i]
                    + ", valid[" + i + "]="
                    + TelephonyUtils.getSimLockSimValidString(simValid[i]));
        }
        log("getSimLockCase, case=" + getSimLockCaseString(caseId)
                + builder.toString());

        return caseId;
    }

    /**
     * Get the SIM lock policy string.
     * @param policy the policy ID.
     * @return the policy string.
     */
    public static String getSimLockPolicyString(int policy) {
        String str;
        switch (policy) {
            case SIM_LOCK_POLICY_UNKNOWN:
                str = "unknown";
                break;

            case SIM_LOCK_POLICY_NONE:
                str = "none";
                break;

            case SIM_LOCK_POLICY_SLOT1_ONLY:
                str = "1only";
                break;

            case SIM_LOCK_POLICY_SLOT2_ONLY:
                str = "2only";
                break;

            case SIM_LOCK_POLICY_SLOT_ALL:
                str = "all";
                break;

            case SIM_LOCK_POLICY_SLOT1_LINKED:
                str = "1lk";
                break;

            case SIM_LOCK_POLICY_SLOT2_LINKED:
                str = "2lk";
                break;

            case SIM_LOCK_POLICY_SLOT_ALL_LINKED:
                str = "alk";
                break;

            case SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS:
                str = "alk_cs";
                break;

            case SIM_LOCK_POLICY_SLOT_ALL_LINKED_REVERSE:
                str = "alk_rev";
                break;

            case SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_ECC:
                str = "alk_ecc";
                break;

            case SIM_LOCK_POLICY_LEGACY:
                str = "legacy";
                break;

            default:
                str = "wrong";
                break;
        }

        return str + "(" + policy + ")";
    }

    /**
     * Get the SIM valid string in SIM lock feature.
     * @param valid the SIM valid ID.
     * @return the SIM valid string.
     */
    public static String getSimLockSimValidString(int valid) {
        String str;
        switch (valid) {
            case SIM_LOCK_SIM_VALID_UNKNOWN:
                str = "unknown";
                break;

            case SIM_LOCK_SIM_VALID_YES:
                str = "yes";
                break;

            case SIM_LOCK_SIM_VALID_NO:
                str = "no";
                break;

            case SIM_LOCK_SIM_VALID_ABSENT:
                str = "absent";
                break;

            default:
                str = "wrong";
                break;
        }

        return str + "(" + valid + ")";
    }

    /**
     * Get the SIM capability string in SIM lock feature.
     * @param capability the SIM capability ID.
     * @return the SIM capability string.
     */
    public static String getSimLockSimCapabilityString(int capability) {
        String str;
        switch (capability) {
            case SIM_LOCK_SIM_CAPABILITY_UNKNOWN:
                str = "unknown";
                break;

            case SIM_LOCK_SIM_CAPABILITY_FULL:
                str = "full";
                break;

            case SIM_LOCK_SIM_CAPABILITY_CS_ONLY:
                str = "cs_only";
                break;

            case SIM_LOCK_SIM_CAPABILITY_PS_ONLY:
                str = "ps_only";
                break;

            case SIM_LOCK_SIM_CAPABILITY_ECC_ONLY:
                str = "ecc_only";
                break;

            case SIM_LOCK_SIM_CAPABILITY_NO_SERVICE:
                str = "no_service";
                break;

            default:
                str = "wrong";
                break;
        }

        return str + "(" + capability + ")";
    }

    /**
     * Get the SIM Lock case string.
     * @param caseId the case ID.
     * @return the case string.
     */
    public static String getSimLockCaseString(int caseId) {
        String str;
        switch (caseId) {
            case SIM_LOCK_CASE_NONE:
                str = "none";
                break;

            case SIM_LOCK_CASE_ALL_EMPTY:
                str = "empty";
                break;

            case SIM_LOCK_CASE_ALL_UNKNOWN:
                str = "unknown";
                break;

            case SIM_LOCK_CASE_ALL_VALID:
                str = "valid";
                break;

            case SIM_LOCK_CASE_ALL_INVALID:
                str = "invalid";
                break;

            case SIM_LOCK_CASE_INVALID_1_VALID:
                str = "inv_1vld";
                break;

            case SIM_LOCK_CASE_INVALID_N_VALID:
                str = "inv_Nvld";
                break;

            case SIM_LOCK_CASE_UNKNOWN_INVALID:
                str = "unkwn_inv";
                break;

            case SIM_LOCK_CASE_UNKNOWN_1_VALID:
                str = "unkwn_1vld";
                break;

            case SIM_LOCK_CASE_UNKNOWN_N_VALID:
                str = "unkwn_Nvld";
                break;

            case SIM_LOCK_CASE_UNKNOWN_INVALID_1_VALID:
                str = "unkwn_inv_1vld";
                break;

            case SIM_LOCK_CASE_UNKNOWN_INVALID_N_VALID:
                str = "unkwn_inv_Nvld";
                break;

            default:
                str = "wrong";
                break;
        }

        return str + "(" + caseId + ")";
    }

    /// M: Add for SIM On/Off feature. @{
    public static boolean isSimOnOffEnabled() {
        return MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
    }

    public static int setSimOnOffState(int slotId, int state) {
        return MtkTelephonyManagerEx.getDefault().setSimPower(slotId, state);
    }

    public static int getSimOnOffState(int slotId) {
        return MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
    }

    public static int getSimOnOffSwitchingState(int slotId) {
        return MtkTelephonyManagerEx.getDefault().getSimOnOffExecutingState(slotId);
    }
    /// @}

    public static boolean isLteNetworkTypeEnabled(Context context, int subId) {
        boolean enabled = false;
        int networkType = Settings.Global.getInt(
                context.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + subId,
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
        Log.d(TAG, "isLteNetworkTypeEnabled, enabled=" + enabled
                + ", networkType=" + networkType);
        return enabled;
    }

    /// M: Add for checking whether the C2K is supported or not. @{
    private static final boolean sIsC2kSupported = RatConfiguration.isC2kSupported();
    public static boolean isC2kSupported() {
        return sIsC2kSupported;
    }
    /// @}

    /// M: Add for checking whether the modem is 93 or later. @{
    private static final String PROPERTY_MTK_RIL_MODE = "ro.vendor.mtk_ril_mode";

    private static boolean sIs93Modem = "c6m_1rild".equals(SystemProperties.get(PROPERTY_MTK_RIL_MODE));
    public static boolean is93OrLaterModem() {
        return sIs93Modem;
    }
    /// @}

    /// M: Add for checking capability. @{
    private static final String PROPERTY_RADIO_PS1_RAT = "ro.vendor.mtk_ps1_rat";
    private static final String PROPERTY_RADIO_PS2_RAT = "persist.vendor.radio.mtk_ps2_rat";
    private static final String PROPERTY_RADIO_NR_SLOT = "persist.vendor.radio.nrslot";
    private static final boolean sIs5gSupported = RatConfiguration.isNrSupported();

    public static String getRadioRatPropertyValue(int phoneId) {
        int mainPhoneId = getMainCapabilityPhoneId();
        if (phoneId == mainPhoneId) {
            return SystemProperties.get(PROPERTY_RADIO_PS1_RAT, "");
        } else {
            return SystemProperties.get(PROPERTY_RADIO_PS2_RAT, "");
        }
    }

    public static boolean is5gSupportedByDevice() {
        return sIs5gSupported;
    }

    public static boolean is5gSupportedInRat(int phoneId, String rat) {
        if (!sIs5gSupported) {
            return false;
        }
        String nrSlotStr = SystemProperties.get(PROPERTY_RADIO_NR_SLOT, "");
        if (TextUtils.isEmpty(nrSlotStr)) {
            return true;
        }

        int nrSlotInt = Integer.parseInt(nrSlotStr) - 1;

        if (phoneId == nrSlotInt) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean is4gSupportedInRat(String rat) {
        return rat.indexOf('L') != -1;
    }

    public static boolean is3gSupportedInRat(String rat) {
        return (rat.indexOf('T') != -1) || (rat.indexOf('W') != -1);
    }
    /// @}
 }

