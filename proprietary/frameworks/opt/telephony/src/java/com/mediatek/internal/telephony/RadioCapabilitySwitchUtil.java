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

package com.mediatek.internal.telephony;

import android.annotation.ProductApi;
import android.content.Context;
import android.os.SystemProperties;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;

import android.telephony.MtkRadioAccessFamily;
import android.telephony.RadioAccessFamily;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.IPhoneSubInfo;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * Utility for capability switch.
 *
 */
public class RadioCapabilitySwitchUtil {
    private static final String LOG_TAG = "RadioCapabilitySwitchUtil";

    public static final int SIM_OP_INFO_UNKNOWN = 0;
    public static final int SIM_OP_INFO_OVERSEA = 1;
    public static final int SIM_OP_INFO_OP01 = 2;
    public static final int SIM_OP_INFO_OP02 = 3;
    public static final int SIM_OP_INFO_OP09 = 4;
    public static final int SIM_OP_INFO_OP18 = 4;

    public static final int SIM_TYPE_SIM = 0;
    public static final int SIM_TYPE_USIM = 1;
    public static final int SIM_TYPE_OTHER = 2;

    public static final int OP01_6M_PRIORITY_OP01_USIM = 0;
    public static final int OP01_6M_PRIORITY_OP01_SIM = 1;
    public static final int OP01_6M_PRIORITY_OTHER = 2;

    // sync to ril_oem.h for dsda
    public static final int SIM_SWITCH_MODE_SINGLE_TALK_MDSYS       = 1;
    public static final int SIM_SWITCH_MODE_SINGLE_TALK_MDSYS_LITE  = 2;
    public static final int SIM_SWITCH_MODE_DUAL_TALK               = 3;
    public static final int SIM_SWITCH_MODE_DUAL_TALK_SWAP          = 4;

    private static final String PROPERTY_ICCID = "vendor.ril.iccid.sim";
    private static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";

    // OP01 SIMs
    private static final String[] PLMN_TABLE_OP01= {
        "46000", "46002", "46007", "46008", "45412", "45413",
        // Lab test IMSI
        "00101", "00211", "00321", "00431", "00541", "00651",
        "00761", "00871", "00902", "01012", "01122", "01232",
        "46004", "46602", "50270"
    };

    // OP02 SIMs
    private static final String[] PLMN_TABLE_OP02= {
        "46001", "46006", "46009", "45407"
    };

    // OP09 SIMs
    private static final String[] PLMN_TABLE_OP09= {
        "46005", "45502", "46003", "46011"
    };

    // OP09 3G SIMs
    private static final String[] PLMN_TABLE_OP09_3G= {
        "20404"
    };

    // OP18 SIMs
    private static final String[] PLMN_TABLE_OP18 = {
        "405840", "405854", "405855", "405856",
        "405857", "405858", "405855", "405856",
        "405857", "405858", "405859", "405860",
        "405861", "405862", "405863", "405864",
        "405865", "405866", "405867", "405868",
        "405869", "405870", "405871", "405872",
        "405873", "405874"
    };

    // OP02 case
    private static final String NO_SIM_VALUE = "N/A";
    public static final String CN_MCC = "460";
    private static final String[] PROPERTY_SIM_ICCID = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4"
    };
    public static final int SHOW_DIALOG = 0;
    public static final int NOT_SHOW_DIALOG = 1;
    public static final int IMSI_NOT_READY_OR_SIM_LOCKED = 2;
    public static final int ICCID_ERROR = 3;
    public static final int SIM_SWITCHING = 4;
    // sim icc status
    // 0: imsi not ready
    // 1: imsi ready
    public static final String IMSI_NOT_READY = "0";
    public static final String IMSI_READY = "1";
    private static final String[] PROPERTY_SIM_IMSI_STATUS = {
        "vendor.ril.imsi.status.sim1",
        "vendor.ril.imsi.status.sim2",
        "vendor.ril.imsi.status.sim3",
        "vendor.ril.imsi.status.sim4"
    };

    private static final String[]  PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    private static final String[] PROPERTY_RIL_CT3G = {
        "vendor.gsm.ril.ct3g",
        "vendor.gsm.ril.ct3g.2",
        "vendor.gsm.ril.ct3g.3",
        "vendor.gsm.ril.ct3g.4",
    };

    public static final int ENHANCEMENT_T_PLUS_T = 0;
    public static final int ENHANCEMENT_T_PLUS_W = 1;
    public static final int ENHANCEMENT_T_PLUS_C = 2;
    public static final int ENHANCEMENT_W_PLUS_C = 3;
    public static final int ENHANCEMENT_W_PLUS_W = 4;
    public static final int ENHANCEMENT_W_PLUS_NA = 5;

    //Readable Constant to define SubsidyLock feature based upon Sim Lock Policy
    public static final int SUBSIDY_LOCK_SUPPORT =
                          MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_VOICE;

    /**
     * Get all SIMs operator and type.
     *
     * @param simOpInfo SIM operator info
     * @param simType SIM type
     */
    public static boolean getSimInfo(int[] simOpInfo, int[] simType, int insertedStatus) {
        String[] strMnc = new String[simOpInfo.length];
        String[] strSimType = new String[simOpInfo.length];
        String propStr;

        for (int i = 0; i < simOpInfo.length; i++) {
            if (i == 0) {
                propStr = "vendor.gsm.ril.uicctype";
            } else {
                propStr = "vendor.gsm.ril.uicctype." + (i + 1);
            }
            strSimType[i] = SystemProperties.get(propStr, "");
            if (strSimType[i].equals("SIM")) {
                simType[i] = RadioCapabilitySwitchUtil.SIM_TYPE_SIM;
            } else if (strSimType[i].equals("USIM")) {
                simType[i] = RadioCapabilitySwitchUtil.SIM_TYPE_USIM;
            } else {
                simType[i] = RadioCapabilitySwitchUtil.SIM_TYPE_OTHER;
            }

            try {
                IPhoneSubInfo subInfo = IPhoneSubInfo.Stub.asInterface(
                        ServiceManager.getService("iphonesubinfo"));
                if (subInfo == null) {
                    logd("subInfo stub is null");
                    return false;
                }
                int[] subIdList = SubscriptionManager.getSubId(i);
                if (subIdList == null) {
                    // means no sim in current slot from Android Q
                    logd("subIdList is null");
                } else {
                    strMnc[i] = subInfo.getSubscriberIdForSubscriber(
                            subIdList[0], "com.mediatek.internal.telephony");
                    if (strMnc[i] == null) {
                        logd("strMnc[" + i + "] is null, get mnc by ril.uim.subscriberid");
                        propStr = "vendor.ril.uim.subscriberid." + (i + 1);
                        strMnc[i] = SystemProperties.get(propStr, "");
                    }
                    if (strMnc[i].equals("")) {
                        logd("strMnc[" + i + "] is null, get mnc by vendor.gsm.ril.uicc.mccmnc");
                        if (i == 0) {
                            propStr = "vendor.gsm.ril.uicc.mccmnc";
                        } else {
                            propStr = "vendor.gsm.ril.uicc.mccmnc." + i;
                        }
                        strMnc[i] = SystemProperties.get(propStr, "");
                    }
                }
            } catch (RemoteException ex) {
                logd("get subInfo stub fail");
                strMnc[i] = "error";
            }

            if (strMnc[i] == null) {
                logd("strMnc[" + i + "] is null");
                strMnc[i] = "";
            }

            if (strMnc[i].length() >= 6) {
                strMnc[i] = strMnc[i].substring(0, 6);
            } else if (strMnc[i].length() >= 5) {
                strMnc[i] = strMnc[i].substring(0, 5);
            }
            logd("SimType[" + i + "]= " + strSimType[i] + "insertedStatus:" + insertedStatus);

            if ((insertedStatus >= 0) && (((1 << i) & insertedStatus) > 0)) {
                if (strMnc[i].equals("") || strMnc[i].equals("error")) {
                    logd("SIM is inserted but no imsi");
                    return false;
                }
                if (strMnc[i].equals("sim_lock")) {
                    logd("SIM is lock, wait pin unlock");
                    return false;
                }
                if (strMnc[i].equals("N/A") || strMnc[i].equals("sim_absent")) {
                    logd("strMnc have invalid value, return false");
                    return false;
                }
                if (strMnc[i].matches("[0-9]+") == false) {
                    logd("strMnc have non-numeric value, return false");
                    return false;
                }
            }
            for (String mccmnc : PLMN_TABLE_OP01) {
                if (strMnc[i].startsWith(mccmnc)) {
                    simOpInfo[i] = SIM_OP_INFO_OP01;
                    break;
                }
            }
            if (simOpInfo[i] == SIM_OP_INFO_UNKNOWN) {
                for (String mccmnc : PLMN_TABLE_OP02) {
                    if (strMnc[i].startsWith(mccmnc)) {
                        simOpInfo[i] = SIM_OP_INFO_OP02;
                        break;
                    }
                }
            }
            if (simOpInfo[i] == SIM_OP_INFO_UNKNOWN) {
                for (String mccmnc : PLMN_TABLE_OP09) {
                    if (strMnc[i].startsWith(mccmnc)) {
                        simOpInfo[i] = SIM_OP_INFO_OP09;
                        break;
                    }
                }
            }
            if (simOpInfo[i] == SIM_OP_INFO_UNKNOWN) {
                for (String mccmnc : PLMN_TABLE_OP09_3G) {
                    if (strMnc[i].startsWith(mccmnc)) {
                        String uimDualMode = SystemProperties.get(PROPERTY_RIL_CT3G[i]);
                        if ("1".equals(uimDualMode)) {
                            simOpInfo[i] = SIM_OP_INFO_OP09;
                            break;
                        }
                    }
                }
            }
            if (SystemProperties.get("persist.vendor.operator.optr", "").equals("OP18")) {
                if (simOpInfo[i] == SIM_OP_INFO_UNKNOWN) {
                    for (String mccmnc : PLMN_TABLE_OP18) {
                        if (strMnc[i].startsWith(mccmnc)) {
                            simOpInfo[i] = SIM_OP_INFO_OP18;
                            break;
                        }
                    }
                }
            }
            if (simOpInfo[i] == SIM_OP_INFO_UNKNOWN) {
                if (!strMnc[i].equals("") && !strMnc[i].equals("N/A")) {
                    simOpInfo[i] = SIM_OP_INFO_OVERSEA;
                }
            }
            logd("strMnc[" + i + "]= " + strMnc[i] + ", simOpInfo[" + i + "]=" + simOpInfo[i]);
        }
        // logd("getSimInfo(simOpInfo): " + Arrays.toString(simOpInfo));
        // logd("getSimInfo(simType): " + Arrays.toString(simType));
        return true;
    }

    public static boolean isVolteEnabled(int phoneId, Context context) {
        ImsManager imsManager = ImsManager.getInstance(context, phoneId);
        boolean imsUseEnabled = (imsManager.isVolteEnabledByPlatform()
                && imsManager.isEnhanced4gLteModeSettingEnabledByUser());

        if (imsUseEnabled == true) {
            // check 4G is enabled or not
            int subId[] = SubscriptionManager.getSubId(phoneId);
            if (subId != null) {
                int nwMode = Settings.Global.getInt(
                        context.getContentResolver(),
                        Settings.Global.PREFERRED_NETWORK_MODE + subId[0],
                        MtkRILConstants.PREFERRED_NETWORK_MODE);
                int rafFromNwMode = MtkRadioAccessFamily.getRafFromNetworkType(nwMode);
                int rafLteGroup = MtkRadioAccessFamily.RAF_LTE | MtkRadioAccessFamily.RAF_LTE_CA;
                if ((rafFromNwMode & rafLteGroup) == 0) {
                    imsUseEnabled = false;
                }
                logd("isVolteEnabled, imsUseEnabled = " + imsUseEnabled
                        + ", nwMode = " + nwMode
                        + ", rafFromNwMode = " + rafFromNwMode
                        + ", rafLteGroup = " + rafLteGroup);
            } else {
                logd("isVolteEnabled, subId[] is null");
            }
        }
        logd("isVolteEnabled = " + imsUseEnabled);
        return imsUseEnabled;
    }

    public static boolean isHVolteEnabled() {
        if (SystemProperties.get("persist.vendor.mtk_ct_volte_support").equals("2")
                || SystemProperties.get("persist.vendor.mtk_ct_volte_support").equals("3")) {
            return true;
        }
        return false;
    }

    public static boolean isCdmaCard(int phoneId, int opInfo, Context context) {
        boolean isCdmaSim = false;
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            logd("isCdmaCard invalid phoneId:" + phoneId);
            return isCdmaSim;
        }

        String cardType = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[phoneId]);
        isCdmaSim = (cardType.indexOf("CSIM") >= 0 || cardType.indexOf("RUIM") >= 0);

        if (!isCdmaSim && "SIM".equals(cardType)) {
            String uimDualMode = SystemProperties.get(PROPERTY_RIL_CT3G[phoneId]);
            if ("1".equals(uimDualMode)) {
                isCdmaSim = true;
            }
        }

        if (opInfo == SIM_OP_INFO_OP09) {
            isCdmaSim = true;
        }

        if (isCdmaSim == true && isVolteEnabled(phoneId, context) == true
                && isHVolteEnabled() == false) {
            // if volte is enabled and h-volte is disbale, SRLTE is unused for CT card, treat as CU sim
            isCdmaSim = false;
            logd("isCdmaCard, volte is enabled, SRLTE is unused for CT card");
        }

        return isCdmaSim;
    }

    /**
     * Check if support SIM switch enhancement
     *
     * @return true : support SIM switch enhancement.
     * @       false  :  don't support SIM switch enhancement
     */
    public static boolean isSupportSimSwitchEnhancement(int simType) {
        boolean ret = false;
        switch (simType) {
            // CMCC + CMCC
            case ENHANCEMENT_T_PLUS_T:
                ret = true;
                break;

            // CMCC + CU
            case ENHANCEMENT_T_PLUS_W:
                ret = true;
                break;

            // CMCC + CT
            case ENHANCEMENT_T_PLUS_C:
                ret = false;
                break;

            // CT + CU
            case ENHANCEMENT_W_PLUS_C:
                ret = false;
                break;

            // CU + CU
            case ENHANCEMENT_W_PLUS_W:
                ret = true;
                break;

            // CU + Empty
            case ENHANCEMENT_W_PLUS_NA:
                ret = true;
                break;

            default:
                break;
        }
        return ret;
    }

    /**
     * Check if need to skip switch capability.
     *
     * @param majorPhoneId new major phone ID
     * @return true : don't switch and stay current capability setting
     * @       false  :  keep do setCapability
     */
    public static boolean isSkipCapabilitySwitch(int majorPhoneId, int phoneNum, Context context) {
        int[] simOpInfo = new int[phoneNum];
        int[] simType = new int[phoneNum];
        int insertedState = 0;
        int insertedSimCount = 0;
        int tSimCount = 0;
        int wSimCount = 0;
        int cSimCount = 0;
        String[] currIccId = new String[phoneNum];
        boolean hasOp09Sim = false;
        int op09VolteOffPhoneId = -1;
        if (isPS2SupportLTE()) {
            if (phoneNum > 2) {
                if (majorPhoneId < 2 && getMainCapabilityPhoneId() < 2
                        && !RatConfiguration.isC2kSupported()
                        && !RatConfiguration.isTdscdmaSupported()) {
                    return true;
                }
                return false;
            }
            // check sim cards number
            for (int i = 0; i < phoneNum; i++) {
                currIccId[i] = SystemProperties.get(PROPERTY_ICCID + (i + 1));
                if (currIccId[i] == null || "".equals(currIccId[i])) {
                    logd("iccid not found, do capability switch");
                    return false;
                }
                if (!NO_SIM_VALUE.equals(currIccId[i])) {
                    // if sim on/off feature is enable and sim is OFF state, treat as ABSENT state
                    if (RadioCapabilitySwitchUtil.isSimOn(i) == true
                            && isRadioOffBySimManagement(i) == false) {
                        ++insertedSimCount;
                        insertedState = insertedState | (1 << i);
                    } else {
                        logd("isSkipCapabilitySwitch, slot" + i + " is power off.");
                    }
                }
            }

            // no sim card
            if (insertedSimCount == 0) {
                logd("no sim card, skip capability switch");
                return true;
            }

            // check sim info
            if (getSimInfo(simOpInfo, simType, insertedState) == false) {
                logd("cannot get sim operator info, do capability switch");
                return false;
            }

            for (int i = 0; i < phoneNum; i++) {
                if (((1 << i) & insertedState) > 0) {
                    if (SIM_OP_INFO_OP01 == simOpInfo[i]) {
                        tSimCount++;
                    } else if (isCdmaCard(i, simOpInfo[i], context)) {
                        cSimCount++;
                        op09VolteOffPhoneId = i;
                    } else if (SIM_OP_INFO_UNKNOWN!= simOpInfo[i]){
                        wSimCount++;
                    }
                    if (simOpInfo[i] == SIM_OP_INFO_OP09) {
                        hasOp09Sim = true;
                    }
                }
            }

            logd("isSkipCapabilitySwitch : Inserted SIM count: " + insertedSimCount
                    + ", insertedStatus: " + insertedState + ", tSimCount: " + tSimCount
                    + ", wSimCount: " + wSimCount + ", cSimCount: " + cSimCount );
            // t + t --> don't need to capability switch
            if (isSupportSimSwitchEnhancement(ENHANCEMENT_T_PLUS_T)
                    && (insertedSimCount == 2) && (tSimCount == 2)) {
                return true;
            }

            // t + w --> if support real T+W, always on t card
            if (isSupportSimSwitchEnhancement(ENHANCEMENT_T_PLUS_W)
                    && (insertedSimCount == 2) && (tSimCount == 1) && (wSimCount == 1)) {
                if (isTPlusWSupport() && (simOpInfo[majorPhoneId] != SIM_OP_INFO_OP01)
                        && hasOp09Sim == false) {
                    return true;
                }
            }

            // t + c --> always on c card
            if (isSupportSimSwitchEnhancement(ENHANCEMENT_T_PLUS_C)
                    && (insertedSimCount == 2) && (tSimCount == 1) && (cSimCount == 1)) {
                if (!isCdmaCard(majorPhoneId, simOpInfo[majorPhoneId], context)) {
                    return true;
                }
            }

            // w + c--> always on c card
            if (isSupportSimSwitchEnhancement(ENHANCEMENT_W_PLUS_C)
                    && (insertedSimCount == 2) && (wSimCount == 1) && (cSimCount == 1)) {
                if (!isCdmaCard(majorPhoneId, simOpInfo[majorPhoneId], context)) {
                    return true;
                }
            }

            // w + w --> don't need to capability switch
            if (isSupportSimSwitchEnhancement(ENHANCEMENT_W_PLUS_W)
                    && (insertedSimCount == 2) && (wSimCount == 2)) {
                return true;
            }

            // w + empty --> don't need to capability switch
            if (isSupportSimSwitchEnhancement(ENHANCEMENT_W_PLUS_NA)
                    && (insertedSimCount == 1) && (wSimCount == 1)) {
                return true;
            }

            // c + c and only one card volte on --> always on volte off card
            if ((SIM_OP_INFO_OP09 == simOpInfo[0]) && (SIM_OP_INFO_OP09 == simOpInfo[1])
                    && (cSimCount == 1) && (wSimCount == 1)
                    && (op09VolteOffPhoneId != majorPhoneId)) {
                return true;
            }
        }

        return false;
    }

    /**
     * Check if any higher priority SIM exists.
     *
     * @param curId current phone ID uses main capability
     * @param op01Usim array to indicate if op01 USIM
     * @param op01Sim array to indicate if op01 SIM
     * @param overseaUsim array to indicate if oversea USIM
     * @param overseaSim array to indicate if oversea SIM
     * @return higher priority SIM ID
     */
    public static int getHigherPrioritySimForOp01(int curId, boolean[] op01Usim, boolean[] op01Sim
            , boolean[] overseaUsim, boolean[] overseaSim) {
        int targetSim = -1;
        int phoneNum = op01Usim.length;

        if (op01Usim[curId] == true) {
            return curId;
        }
        for (int i = 0; i < phoneNum; i++) {
            if (op01Usim[i] == true) {
                targetSim = i;
            }
        }
        if (targetSim != -1 || op01Sim[curId] == true) {
            return targetSim;
        }
        for (int i = 0; i < phoneNum; i++) {
            if (op01Sim[i] == true) {
                targetSim = i;
            }
        }
        if (targetSim != -1 || overseaUsim[curId] == true) {
            return targetSim;
        }
        for (int i = 0; i < phoneNum; i++) {
            if (overseaUsim[i] == true) {
                targetSim = i;
            }
        }
        if (targetSim != -1 || overseaSim[curId] == true) {
            return targetSim;
        }
        for (int i = 0; i < phoneNum; i++) {
            if (overseaSim[i] == true) {
                targetSim = i;
            }
        }
        return targetSim;
    }

    /**
     * Get the highest priority phoneId.
     *
     * @param capPhoneId phone ID that uses main capability
     * @param priority array to indicate SIM priority
     * @return highest priority phone ID
     */
    public static int getHighestPriorityPhone(int capPhoneId, int[] priority) {
        int targetPhone = 0;
        int phoneNum = priority.length;
        int highestPriorityCount = 0;
        int highestPriorityBitMap = 0;

        for (int i = 0; i < phoneNum; i++) {
            if (priority[i] < priority[targetPhone]) {
                targetPhone = i;
                highestPriorityCount = 1;
                highestPriorityBitMap = (1 << i);
            } else if (priority[i] == priority[targetPhone]) {
                highestPriorityCount++;
                highestPriorityBitMap |= (1 << i);
            }
        }

        if (highestPriorityCount == 1) {
            return targetPhone;
        } else if (capPhoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
            // more than two cards has highest priority
            // and capability sim is not found/set.
            return SubscriptionManager.INVALID_PHONE_INDEX;
        } else if ((highestPriorityBitMap & (1 << capPhoneId)) != 0) {
            return capPhoneId;
        }
        return -1;
    }

    /**
     * Get main capability phone ID.
     *
     * @return Phone ID with main capability
     */
    @ProductApi
    public static int getMainCapabilityPhoneId() {
        int phoneId = 0;
        phoneId = SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        // logd("[RadioCapSwitchUtil] getMainCapabilityPhoneId " + phoneId);
        return phoneId;
    }

    private static void logd(String s) {
        Rlog.d(LOG_TAG, "[RadioCapSwitchUtil] " + s);
    }

    /**
     * Check SIM type and operator type to decide displaying dialog or not.
     *
     * @return SHOW_DIALOG: show dialog
     *             NOT_SHOW_DIALOG: don't show dialog
     *             IMSI_NOT_READY_OR_SIM_LOCKED: IMSI not ready or sim locked. Wait
     *             ACTION_SIM_STATE_CHANGED to check isNeedShowSimDialog() again
     *             ICCID_ERROR: read ICCID error. Wait ACTION_SIM_STATE_CHANGED
     *              to check isNeedShowSimDialog() again
     *             SIM_SWITCHING: in sim switching, don't show dialog
     */
    public static int isNeedShowSimDialog() {
        if (SystemProperties.getBoolean("ro.vendor.mtk_disable_cap_switch", false) == true) {
            logd("mtk_disable_cap_switch is true");
            return SHOW_DIALOG;
        }

        logd("isNeedShowSimDialog start");
        int phoneCount = TelephonyManager.getDefault().getPhoneCount();
        int[] simOpInfo = new int[phoneCount];
        int[] simType = new int[phoneCount];
        String[] currIccId = new String[phoneCount];
        int insertedSimCount = 0;
        int insertedStatus = 0;
        int op02CardCount = 0;
        ArrayList<Integer> usimIndexList = new ArrayList<Integer>();
        ArrayList<Integer> simIndexList = new ArrayList<Integer>();
        ArrayList<Integer> op02IndexList = new ArrayList<Integer>();
        ArrayList<Integer> otherIndexList = new ArrayList<Integer>();

        for (int i = 0; i < phoneCount; i++) {
            currIccId[i] = SystemProperties.get(PROPERTY_SIM_ICCID[i]);
            logd("currIccid[" + i + "] : " + currIccId[i]);
            if (currIccId[i] == null || "".equals(currIccId[i])) {
                Log.e(LOG_TAG, "iccid not found, wait for next sim state change");
                return ICCID_ERROR;
            }
            if (!NO_SIM_VALUE.equals(currIccId[i])) {
                // if sim on/off feature is enable and sim is OFF state, treat as ABSENT state
                if (RadioCapabilitySwitchUtil.isSimOn(i) == true) {
                    insertedSimCount++;
                    insertedStatus = insertedStatus | (1 << i);
                } else {
                    logd("isNeedShowSimDialog, slot" + i + " is power off.");
                }
            }
        }

        if (insertedSimCount < 2) {
            logd("isNeedShowSimDialog: insert sim count < 2, do not show dialog");
            return NOT_SHOW_DIALOG;
        }
        if (isCapabilitySwitching()) {
            logd("SIM switch executing");
            return SIM_SWITCHING;
        }
        if (getSimInfo(simOpInfo, simType, insertedStatus) == false) {
            Log.e(LOG_TAG, "isNeedShowSimDialog: Can't get SIM information");
            return IMSI_NOT_READY_OR_SIM_LOCKED;
        }
        for (int i = 0; i < phoneCount; i++) {
            // check SIM type
            if (SIM_TYPE_USIM == simType[i]) {
                usimIndexList.add(i);
            } else if (SIM_TYPE_SIM == simType[i]) {
                simIndexList.add(i);
            }

            // check SIM operator
            if (SIM_OP_INFO_OP02 == simOpInfo[i]) {
                op02IndexList.add(i);
            } else {
                otherIndexList.add(i);
            }
        }
        logd("usimIndexList size = " + usimIndexList.size());
        logd("op02IndexList size = " + op02IndexList.size());

        if (usimIndexList.size() >= 2) {
            // check OP02USIM count
            for (int i = 0; i < usimIndexList.size(); i++) {
                if (op02IndexList.contains(usimIndexList.get(i))) {
                    op02CardCount++;
                }
            }

            if (op02CardCount == 1) {
                logd("isNeedShowSimDialog: One OP02Usim inserted, not show dialog");
                return NOT_SHOW_DIALOG;
            }
        } else if (usimIndexList.size() == 1) {
            logd("isNeedShowSimDialog: One Usim inserted, not show dialog");
            return NOT_SHOW_DIALOG;
        } else {
            // usimIndexList.size() = 0 (all SIM cards)
            // check OP02SIM count
            for (int i = 0; i < simIndexList.size(); i++) {
                if (op02IndexList.contains(simIndexList.get(i))) {
                    op02CardCount++;
                }
            }

            if (op02CardCount == 1) {
                logd("isNeedShowSimDialog: One non-OP02 Usim inserted, not show dialog");
                return NOT_SHOW_DIALOG;
            }
        }
        logd("isNeedShowSimDialog: Show dialog");
        return SHOW_DIALOG;
    }

    public static boolean isAnySimLocked(int phoneNum) {
        if (RatConfiguration.isC2kSupported()) {
            logd("isAnySimLocked always returns false in C2K");
            return false;
        }
        // iccid property is not equal to N/A and imsi property is empty => sim locked
        String[] mnc = new String[phoneNum];
        String[] iccid = new String[phoneNum];
        String propStr;
        for (int i=0; i<phoneNum; i++) {
            iccid[i] = SystemProperties.get(PROPERTY_SIM_ICCID[i]);
            // only get mcc,mnc when SIM inserted
            if (!iccid[i].equals(NO_SIM_VALUE)) {
                mnc[i] = TelephonyManager.getTelephonyProperty(
                        i, "vendor.gsm.sim.operator.numeric", "");
                if (mnc[i].length() >= 6) {
                    mnc[i] = mnc[i].substring(0, 6);
                } else if (mnc[i].length() >= 5) {
                    mnc[i] = mnc[i].substring(0, 5);
                }
                if (!mnc[i].equals("")) {
                    logd("i = " + i + " from gsm.sim.operator.numeric:" + mnc[i] + " ,iccid = "
                            + iccid[i]);
                }
            }

            if (!iccid[i].equals(NO_SIM_VALUE) && (mnc[i].equals("") ||
                    mnc[i].equals("sim_lock"))) {
                return true;
            }
        }
        return false;
    }

    @ProductApi
    public static boolean isPS2SupportLTE() {
        if (SystemProperties.get("persist.vendor.radio.mtk_ps2_rat").indexOf('L') != -1) {
            // logd("isPS2SupportLTE = true");
            return true;
        }
        // logd("isPS2SupportLTE = false");
        return false;
    }

    public static boolean isTPlusWSupport() {
        if (SystemProperties.get("vendor.ril.simswitch.tpluswsupport").equals("1")) {
            // logd("return true for T+W support");
            return true;
        }
        return false;
    }

    public static void updateSimImsiStatus(int slot, String value) {
        logd("updateSimImsiStatus slot = " + slot + ", value = " + value);
        String propStr = PROPERTY_SIM_IMSI_STATUS[slot];
        SystemProperties.set(propStr, value);
    }

    private static String getSimImsiStatus(int slot) {
        String propStr = PROPERTY_SIM_IMSI_STATUS[slot];
        return SystemProperties.get(propStr, IMSI_NOT_READY);
    }

    public static void clearAllSimImsiStatus() {
        logd("clearAllSimImsiStatus");
        for (int i = 0; i < PROPERTY_SIM_IMSI_STATUS.length; i++) {
            updateSimImsiStatus(i, IMSI_NOT_READY);
        }
    }

    public static boolean isDssNoResetSupport() {
        if (SystemProperties.get("vendor.ril.simswitch.no_reset_support").equals("1")) {
            // logd("return true for isDssNoResetSupport");
            return true;
        }
        // logd("return false for isDssNoResetSupport");
        return false;
    }

    // Get the protocol stack id by slot id, the slot is 0-based, protocol stack id is 1-based
    public static int getProtocolStackId(int slot) {
        // The major sim is mapped to PS1, from 93 modem which supports dynamic sim switch without
        // modem reset, the other sims are mapped to PS2~PS4 in ascending order; For the modem
        // before 93, the major sim is switched protocol stack with SIM1.
        int majorSlot = getMainCapabilityPhoneId();
        if (slot == majorSlot) {
            return 1;
        }
        if (isDssNoResetSupport()) {
            if (slot < majorSlot) {
                return slot + 2;
            }
        } else if (slot == 0){
            return majorSlot + 1;
        }
        return slot + 1;
    }

    public static String getHashCode(String iccid) {
        try {
            MessageDigest alga = MessageDigest.getInstance("SHA-256");
            alga.update(iccid.getBytes());
            byte[] hashCode = alga.digest();
            String strIccid = new String(hashCode);
            return strIccid;
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("RadioCapabilitySwitchUtil SHA-256 must exist");
        }
    }

    public static boolean isSimOn(int slotId) {
        if (MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled() == false) {
            return true;
        }

        int state = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
        if (state == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_OFF) {
            return false;
        } else {
            return true;
        }
    }

    public static boolean isRadioOffBySimManagement(int phoneId) {
        boolean result = false;
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
        try {
            IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub
                    .asInterface(ServiceManager.getService("phoneEx"));
            if (null == iTelEx) {
                logd("iTelEx is null!");
                return false;
            }
            result = iTelEx.isRadioOffBySimManagement(subId);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
        return result;
    }
    public static boolean isCapabilitySwitching() {
        boolean result = false;
        try {
            IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub
                    .asInterface(ServiceManager.getService("phoneEx"));
            if (null == iTelEx) {
                logd("iTelEx is null!");
                return false;
            }
            result = iTelEx.isCapabilitySwitching();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
        return result;
    }
    //Subsidy Lock will be supported if Sim Lock Policy returns 10
    public static boolean isSubsidyLockFeatureOn(){
        boolean supportSubsidYLock = false;
        int lockPolicy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        // logd("DS lockPolicy :" + lockPolicy);
        if(SUBSIDY_LOCK_SUPPORT == lockPolicy){
            return true;
        }
        return supportSubsidYLock;
    }

    /**
     * Api to get subsidylock status.
     * @hide
     */
    public static boolean isSubsidyLockForOmSupported() {
         boolean isSubsidyLockSupported =
                isSubsidyLockFeatureOn();
        //boolean subsidylockRilStatus =
          //      SystemProperties.get("persist.subsidylock.lockstatus_ril", "1").equals("1");
        boolean subsidylockStatus =
                !(SystemProperties.get("persist.vendor.subsidylock", "0").equals("2"));

        // logd("isSubsidyLockSupported : " + isSubsidyLockSupported +
                //"\n subsidylockRilStatus : " + subsidylockRilStatus +
                // "\n subsidylockStatus : " + subsidylockStatus);
        return (isSubsidyLockSupported && (subsidylockStatus /*|| subsidylockRilStatus*/));
    }
}
