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
package com.mediatek.internal.telephony.worldphone;

import android.content.Context;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import mediatek.telephony.MtkServiceState;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.ProxyController;
import com.mediatek.internal.telephony.MtkProxyController;
import com.mediatek.internal.telephony.ModemSwitchHandler;
import com.mediatek.internal.telephony.worldphone.WorldMode;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;

/**
 *@hide
 */
public class WorldPhoneUtil implements IWorldPhone {

    public static final int UTRAN_DIVISION_DUPLEX_MODE_UNKNOWN = 0;
    public static final int UTRAN_DIVISION_DUPLEX_MODE_FDD = 1;
    public static final int UTRAN_DIVISION_DUPLEX_MODE_TDD = 2;

    private static final int ACTIVE_MD_TYPE_UNKNOWN = 0;
    private static final int ACTIVE_MD_TYPE_WG   = 1;//3G(WCDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_TG   = 2;//3G(TDS-CDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_LWG  = 3;//4G(TDD-LTE+FDD-LTE)+3G(WCDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_LTG  = 4;//4G(TDD-LTE+FDD-LTE)+3G(TDS-CDMA)+2G(GSM)
    //4G(TDD-LTE+FDD-LTE)+3G(WCDMA+EVDO)+2G(GSM+CDMA2000)
    private static final int ACTIVE_MD_TYPE_LWCG = 5;
    private static final int ACTIVE_MD_TYPE_LtTG = 6;//4G(TDD-LTE)+3G(TDS-CDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_LfWG = 7;//4G(FDD-LTE)+3G(WCDMA)+2G(GSM)

    private static final int PROJECT_SIM_NUM = TelephonyManager.getDefault().getSimCount();
    private static final boolean IS_WORLD_MODE_SUPPORT =
            (SystemProperties.getInt("ro.vendor.mtk_md_world_mode_support", 0) == 1);
    private static Context sContext = null;
    private static Phone sDefultPhone = null;
    private static Phone[] sProxyPhones = null;
    private static Phone[] sActivePhones = new Phone[PROJECT_SIM_NUM];
//C2K world phone start

    public static int sToModem = 0;
    public static boolean sSimSwitching = false;
    // SVLTE Slot
    public static final int CSFB_ON_SLOT = -1;
    public static final int SVLTE_ON_SLOT_0 = 0;
    public static final int SVLTE_ON_SLOT_1 = 1;

    // Radio technology mode definition.
    public static final int RADIO_TECH_MODE_UNKNOWN = 1;
    public static final int RADIO_TECH_MODE_CSFB    = 2;
    public static final int RADIO_TECH_MODE_SVLTE   = 3;

    // Used for maintain the Card Modes after calculation.
    private static int[] sCardModes = initCardModes();

    /**
    * The property is used to get supported card type of each SIM card in the slot.
    * @hide
    */
    private static final String[]  PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    public static final int CARD_TYPE_NONE = 0;
    public static final int CARD_TYPE_SIM  = 1;
    public static final int CARD_TYPE_USIM = 2;
    public static final int CARD_TYPE_RUIM = 4;
    public static final int CARD_TYPE_CSIM = 8;
    private static int[] mC2KWPCardtype = new int[TelephonyManager.getDefault().getPhoneCount()];

    // Used to save the correct RADIO TECH.
    public static final String SVLTE_PROP = "persist.vendor.radio.svlte_slot";

    static private IWorldPhone sWorldPhone = null;

    public WorldPhoneUtil() {
        logd("Constructor invoked");
        sDefultPhone = PhoneFactory.getDefaultPhone();
        sProxyPhones = PhoneFactory.getPhones();
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sActivePhones[i] = sProxyPhones[i];
        }
        if (sDefultPhone != null) {
            sContext = sDefultPhone.getContext();
        } else {
            logd("DefaultPhone = null");
        }
    }

    public static void makeWorldPhoneManager() {
        if (isWorldModeSupport() && isWorldPhoneSupport()) {
            logd("Factory World mode support");
            WorldMode.init();
        } else if (isWorldPhoneSupport()) {
            logd("Factory World phone support");
            sWorldPhone = WorldPhoneWrapper.getWorldPhoneInstance();
        } else {
            logd("Factory World phone not support");
        }
    }

    public static IWorldPhone getWorldPhone() {
        if (sWorldPhone == null) {
            logd("sWorldPhone is null");
        }
        return sWorldPhone;
    }

    public static int getProjectSimNum() {
        return PROJECT_SIM_NUM;
    }

    public static int getMajorSim() {
        if (!(((MtkProxyController)ProxyController.getInstance()).isCapabilitySwitching())) {
            String currMajorSim = SystemProperties.get(PROPERTY_MAJOR_SIM, "");
            if (currMajorSim != null && !currMajorSim.equals("")) {
                logd("[getMajorSim]: " + ((Integer.parseInt(currMajorSim)) - 1));
                return (Integer.parseInt(currMajorSim)) - 1;
            } else {
                logd("[getMajorSim]: fail to get major SIM");
                return MAJOR_SIM_UNKNOWN;
            }
        } else {
            logd("[getMajorSim]: radio capability is switching");
            return MAJOR_SIM_UNKNOWN;
        }
    }

    public static int getModemSelectionMode() {
        if (sContext == null) {
            logd("sContext = null");
            return SELECTION_MODE_AUTO;
        }
        return SystemProperties.getInt(WORLD_PHONE_AUTO_SELECT_MODE, SELECTION_MODE_AUTO);
    }

    public static boolean isWorldPhoneSupport() {
        return (RatConfiguration.isWcdmaSupported() && RatConfiguration.isTdscdmaSupported());
    }

    public static boolean isLteSupport() {
        return (RatConfiguration.isLteFddSupported() || RatConfiguration.isLteTddSupported());
    }

    public static String regionToString(int region) {
        String regionString;
        switch (region) {
            case REGION_UNKNOWN:
                regionString = "REGION_UNKNOWN";
                break;
            case REGION_DOMESTIC:
                regionString = "REGION_DOMESTIC";
                break;
            case REGION_FOREIGN:
                regionString = "REGION_FOREIGN";
                break;
            default:
                regionString = "Invalid Region";
                break;
        }
        return regionString;
    }

    public void setModemSelectionMode(int mode, int modemType) {
    }

    public void notifyRadioCapabilityChange(int capailitySimId) {
    }

    public static boolean isWorldModeSupport() {
        return IS_WORLD_MODE_SUPPORT;
    }

    /**
     * In world phone support project, modem support both FDD(Frequency-division duplexing) mode and
     * TDD(Time-division duplexin) mode for 3G(UTRAN). It runs only one mode, FDD or TDD, at a time.
     * This API will return acully divison duplexing mode.
     *
     * @return 0 :unknown
     *         1 : FDD
     *         2 : TDD
     */
    public static int get3GDivisionDuplexMode(){
        int duplexMode = UTRAN_DIVISION_DUPLEX_MODE_UNKNOWN;
        int activeMdType = getActiveModemType();

        switch (activeMdType) {
            case ACTIVE_MD_TYPE_WG:
            case ACTIVE_MD_TYPE_LWG:
            case ACTIVE_MD_TYPE_LWCG:
            case ACTIVE_MD_TYPE_LfWG:
                duplexMode = UTRAN_DIVISION_DUPLEX_MODE_FDD;
                break;
            case ACTIVE_MD_TYPE_TG:
            case ACTIVE_MD_TYPE_LTG:
            case ACTIVE_MD_TYPE_LtTG:
                duplexMode = UTRAN_DIVISION_DUPLEX_MODE_TDD;
                break;
            default:
                duplexMode = UTRAN_DIVISION_DUPLEX_MODE_UNKNOWN;
            break;
        }
        logd("get3GDivisionDuplexMode=" + duplexMode);
        return duplexMode;
    }

    private static int getActiveModemType(){
        int modemType = 0;
        int activeMdType = ACTIVE_MD_TYPE_UNKNOWN;
        int activeMode = -1;
        if (!isWorldModeSupport()) {
            modemType = ModemSwitchHandler.getActiveModemType();
            switch (modemType) {
                case ModemSwitchHandler.MD_TYPE_WG:
                    activeMdType = ACTIVE_MD_TYPE_WG;
                    break;
                case ModemSwitchHandler.MD_TYPE_TG:
                    activeMdType = ACTIVE_MD_TYPE_TG;
                    break;
                case ModemSwitchHandler.MD_TYPE_LWG:
                    activeMdType = ACTIVE_MD_TYPE_LWG;
                    break;
                case ModemSwitchHandler.MD_TYPE_LTG:
                    activeMdType = ACTIVE_MD_TYPE_LTG;
                    break;
                default:
                    activeMdType = ACTIVE_MD_TYPE_UNKNOWN;
                break;
            }
        } else {
            modemType = WorldMode.getWorldMode();
            activeMode = Integer.valueOf(
                    SystemProperties.get("vendor.ril.nw.worldmode.activemode",
                    Integer.toString(ACTIVE_MD_TYPE_UNKNOWN)));
            switch (modemType) {
                case WorldMode.MD_WORLD_MODE_LTG:
                case WorldMode.MD_WORLD_MODE_LCTG:
                case WorldMode.MD_WORLD_MODE_LFTG:
                case WorldMode.MD_WORLD_MODE_LFCTG:
                    activeMdType = ACTIVE_MD_TYPE_LTG;
                    break;
                case WorldMode.MD_WORLD_MODE_LWG:
                case WorldMode.MD_WORLD_MODE_LTWG:
                    activeMdType = ACTIVE_MD_TYPE_LWG;
                    break;
                case WorldMode.MD_WORLD_MODE_LWTG:
                case WorldMode.MD_WORLD_MODE_LWCTG:
                    if (activeMode > 0){
                        if (activeMode == 1){
                            //FDD mode
                            activeMdType = ACTIVE_MD_TYPE_LWG;
                        } else if (activeMode == 2){
                            //TDD mode
                            activeMdType = ACTIVE_MD_TYPE_LTG;
                        }
                    }
                    break;
                case WorldMode.MD_WORLD_MODE_LWCG:
                case WorldMode.MD_WORLD_MODE_LFWCG:
                case WorldMode.MD_WORLD_MODE_LTWCG:
                    activeMdType = ACTIVE_MD_TYPE_LWCG;
                    break;
                case WorldMode.MD_WORLD_MODE_LTTG:
                case WorldMode.MD_WORLD_MODE_LTCTG:
                    activeMdType = ACTIVE_MD_TYPE_LtTG;
                    break;
                case WorldMode.MD_WORLD_MODE_LFWG:
                    activeMdType = ACTIVE_MD_TYPE_LfWG;
                    break;
                default:
                    activeMdType = ACTIVE_MD_TYPE_UNKNOWN;
                break;
            }
        }
        logd("getActiveModemType=" + activeMdType + " activeMode=" + activeMode);
        return activeMdType;
    }

    public static boolean isWorldPhoneSwitching(){
        if (isWorldModeSupport()){
            return WorldMode.isWorldModeSwitching();
        } else {
            return false;
        }
    }

    private static int[] initCardModes() {
        int[] cardModes = new int[TelephonyManager.getDefault().getPhoneCount()];
        String svlteType[] = SystemProperties.get(SVLTE_PROP, "3,2,2,2").split(",");
        for (int i = 0; i < cardModes.length; i++) {
            if (i < svlteType.length) {
                cardModes[i] = Integer.parseInt(svlteType[i]);
            } else {
                cardModes[i] = RADIO_TECH_MODE_UNKNOWN;
            }
        }
        return cardModes;
    }

    private static int getFullCardType(int slotId) {
        if (slotId < 0 || slotId >= TelephonyManager.getDefault().getPhoneCount()) {
            logd("getFullCardType invalid slotId:" + slotId);
            return CARD_TYPE_NONE;
        }
        String cardType = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[slotId]);
        String appType[] = cardType.split(",");
        int fullType = CARD_TYPE_NONE;
        for (int i = 0; i < appType.length; i++) {
            if ("USIM".equals(appType[i])) {
                fullType = fullType | CARD_TYPE_USIM;
            } else if ("SIM".equals(appType[i])) {
                fullType = fullType | CARD_TYPE_SIM;
            } else if ("CSIM".equals(appType[i])) {
                fullType = fullType | CARD_TYPE_CSIM;
            } else if ("RUIM".equals(appType[i])) {
                fullType = fullType | CARD_TYPE_RUIM;
            }
        }
        logd("getFullCardType fullType=" + fullType + " cardType =" + cardType);
        return fullType;
    }

    public static int[] getC2KWPCardType() {
        for (int i = 0; i < mC2KWPCardtype.length; i++) {
            mC2KWPCardtype[i] = getFullCardType(i);
            logd("getC2KWPCardType mC2KWPCardtype[" + i + "]=" + mC2KWPCardtype[i]);
        }
        return mC2KWPCardtype;
    }

    /**
    * Get the SVLTE slot id.
    * @return CSFB_ON_SLOT or SVLTE_ON_SLOT_0
    * or SVLTE_ON_SLOT_1
    */
    public static int getActiveSvlteModeSlotId() {
        int svlteSlotId = CSFB_ON_SLOT;
        if (!isCdmaLteDcSupport()) {
            logd("[getActiveSvlteModeSlotId] SVLTE not support, return -1.");
            return svlteSlotId;
        }
        for (int i = 0; i < sCardModes.length; i++) {
            if (sCardModes[i] == RADIO_TECH_MODE_SVLTE) {
                svlteSlotId = i;
            }
        }
        logd("[getActiveSvlteModeSlotId] slotId: " + svlteSlotId);
        return svlteSlotId;
    }

    public static boolean isCdmaLteDcSupport() {
        if (SystemProperties.get("ro.vendor.mtk_c2k_lte_mode").equals("1") ||
                SystemProperties.get("ro.vendor.mtk_c2k_lte_mode").equals("2")) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isC2kSupport() {
        return RatConfiguration.isC2kSupported();
    }

    /**
    * Get the state for sim locked.
    * @param simApplicateionState for sim applicate state which need to check.
    * @return true is locked, false is unlocked.
    */
    public static boolean getSimLockedState(int simApplicateionState) {
        if ((simApplicateionState == TelephonyManager.SIM_STATE_PIN_REQUIRED) ||
                (simApplicateionState == TelephonyManager.SIM_STATE_PUK_REQUIRED) ||
                (simApplicateionState == TelephonyManager.SIM_STATE_NETWORK_LOCKED) ||
                (simApplicateionState == TelephonyManager.SIM_STATE_PERM_DISABLED) ||
                (simApplicateionState == TelephonyManager.SIM_STATE_UNKNOWN)) {
            return true;
        }
        return false;
    }

    public static void saveToModemType(int modemType) {
        sToModem = modemType;
    }

    public static int getToModemType() {
        return sToModem;
    }

    public static boolean isSimSwitching() {
        return sSimSwitching;
    }

    public static void setSimSwitchingFlag(boolean flag) {
        sSimSwitching = flag;
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[WPP_UTIL]" + msg);
    }
}

