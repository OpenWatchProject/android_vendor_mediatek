/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.RatConfiguration;


/* support about world phone api */
public class WorldModeUtil {

    private static final String TAG = "WorldModeActivity";
    private static final int ACTIVE_MD_TYPE_UNKNOWN = 0;
    private static final int ACTIVE_MD_TYPE_WG   = 1;//3G(WCDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_TG   = 2;//3G(TDS-CDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_LWG  = 3;//4G(TDD-LTE+FDD-LTE)+3G(WCDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_LTG  = 4;//4G(TDD-LTE+FDD-LTE)+3G(TDS-CDMA)+2G(GSM)
    //4G(TDD-LTE+FDD-LTE)+3G(WCDMA+EVDO)+2G(GSM+CDMA2000)
    private static final int ACTIVE_MD_TYPE_LWCG = 5;
    private static final int ACTIVE_MD_TYPE_LtTG = 6;//4G(TDD-LTE)+3G(TDS-CDMA)+2G(GSM)
    private static final int ACTIVE_MD_TYPE_LfWG = 7;//4G(FDD-LTE)+3G(WCDMA)+2G(GSM)


    private static final int MD_TYPE_UNKNOWN = 0;
    private static final int MD_TYPE_WG      = 3;
    private static final int MD_TYPE_TG      = 4;
    private static final int MD_TYPE_LWG     = 5;
    private static final int MD_TYPE_LTG     = 6;
    private static final int MD_TYPE_FDD     = 100;
    private static final int MD_TYPE_TDD     = 101;

    private static final int MD_WORLD_MODE_UNKNOWN = 0;
    private static final int MD_WORLD_MODE_LTG     = 8;   //uLTG
    private static final int MD_WORLD_MODE_LWG     = 9;   //uLWG
    private static final int MD_WORLD_MODE_LWTG    = 10;  //uLWTG
    private static final int MD_WORLD_MODE_LWCG    = 11;  //uLWCG
    private static final int MD_WORLD_MODE_LWCTG   = 12;  //uLWTCG(Auto mode)
    private static final int MD_WORLD_MODE_LTTG    = 13;  //LtTG
    private static final int MD_WORLD_MODE_LFWG    = 14;  //LfWG
    private static final int MD_WORLD_MODE_LFWCG   = 15;  //uLfWCG
    private static final int MD_WORLD_MODE_LCTG    = 16;  //uLCTG
    private static final int MD_WORLD_MODE_LTCTG   = 17;  //uLtCTG
    private static final int MD_WORLD_MODE_LTWG    = 18;  //uLtWG
    private static final int MD_WORLD_MODE_LTWCG   = 19;  //uLTWCG
    private static final int MD_WORLD_MODE_LFTG    = 20;  //uLfTG
    private static final int MD_WORLD_MODE_LFCTG   = 21;  //uLfCTG
    private static final int MD_WORLD_MODE_NLWG    = 22;  //uNLWG
    private static final int MD_WORLD_MODE_NLWTG   = 23;  //NLWTG
    private static final int MD_WORLD_MODE_NLWCTG  = 24;  //NLWCTG

    private static final String PROPERTY_RAT_CONFIG = "ro.vendor.mtk_ps1_rat";
    private static final String PROPERTY_ACTIVE_MD = "vendor.ril.active.md";
    private static final String WCDMA = "W";
    private static final String TDSCDMA = "T";
    private static final String CDMA = "C";
    private static final int UTRAN_DIVISION_DUPLEX_MODE_UNKNOWN = 0;
    private static final int UTRAN_DIVISION_DUPLEX_MODE_FDD = 1;
    private static final int UTRAN_DIVISION_DUPLEX_MODE_TDD = 2;

    // World mode result cause for EN.
    private static final int WORLD_MODE_RESULT_SUCCESS            = 100;
    private static final int WORLD_MODE_RESULT_ERROR              = 101;
    private static final int WORLD_MODE_RESULT_WM_ID_NOT_SUPPORT  = 102;

    /* bitmask */
    /* the defination must be sync with ratconfig.c */
    private static final int MASK_NR      = (1 << 6);
    private static final int MASK_CDMA    = (1 << 5);
    private static final int MASK_LTEFDD  = (1 << 4);
    private static final int MASK_LTETDD  = (1 << 3);
    private static final int MASK_WCDMA   = (1 << 2);
    private static final int MASK_TDSCDMA = (1 << 1);
    private static final int MASK_GSM     = (1);

    private static final boolean IS_NR_SUPPORT =  RatConfiguration.isNrSupported();
    private static final String PROPERTY_MAJOR_SIM = "persist.vendor.radio.simswitch";
    private static final String STATUS_SYNC_PREFIX = "STATUS_SYNC";

    public static final int MD_WM_CHANGED_UNKNOWN = -1;
    public static final int MD_WM_CHANGED_START   = 0;
    public static final int MD_WM_CHANGED_END     = 1;

    /**
     * Broadcast world mode change state.
     */
    public static final String EXTRA_WORLD_MODE_CHANGE_STATE = "worldModeState";

    /**
     * Broadcast Action: The world mode changed.
     */
    public static final String ACTION_WORLD_MODE_CHANGED
            = "mediatek.intent.action.ACTION_WORLD_MODE_CHANGED";

    /* check World Phone support */
    public static boolean isWorldPhoneSupport() {
        String rat = SystemProperties.get(PROPERTY_RAT_CONFIG, "");
        if (rat.length() > 0) {
            if (rat.contains(WCDMA) && rat.contains(TDSCDMA)) {
                return true;
            }
        }
        return false;
    }

    /*check 91/92/93 and after chip World Phone support*/
    public static boolean isWorldModeSupport() {
        return (SystemProperties.getInt("ro.vendor.mtk_md_world_mode_support", 0) == 1);
    }

    /* check project support CDMA */
    public static boolean isC2kSupport() {
        String rat = SystemProperties.get(PROPERTY_RAT_CONFIG, "");
        if (rat.length() > 0) {
            if (rat.contains(CDMA)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns current modem type
     * @internal
     * @return 0 : modem type is unknown
     *         3 : switch to WG(MD_TYPE_WG)
     *         4 : switch to TG(MD_TYPE_TG)
     *         5 : switch to FDD CSFB(MD_TYPE_LWG)
     *         6 : switch to TDD CSFB(MD_TYPE_LTG)
     */
    public static int getWorldModeId() {
        int modemType = 0;
        modemType = Integer.valueOf(
                SystemProperties.get(PROPERTY_ACTIVE_MD,
                Integer.toString(MD_TYPE_UNKNOWN)));
        return modemType;
    }

    private static int getActiveModemType(){
        int modemType = 0;
        int activeMdType = ACTIVE_MD_TYPE_UNKNOWN;
        int activeMode = -1;
        if (!isWorldModeSupport()) {
            modemType = getWorldModeId();
            switch (modemType) {
                case MD_TYPE_WG:
                    activeMdType = ACTIVE_MD_TYPE_WG;
                    break;
                case MD_TYPE_TG:
                    activeMdType = ACTIVE_MD_TYPE_TG;
                    break;
                case MD_TYPE_LWG:
                    activeMdType = ACTIVE_MD_TYPE_LWG;
                    break;
                case MD_TYPE_LTG:
                    activeMdType = ACTIVE_MD_TYPE_LTG;
                    break;
                default:
                    activeMdType = ACTIVE_MD_TYPE_UNKNOWN;
                break;
            }
        } else {
            modemType = getWorldModeId();
            activeMode = Integer.valueOf(
                    SystemProperties.get("vendor.ril.nw.worldmode.activemode", "0"));
            switch (modemType) {
                case MD_WORLD_MODE_LTG:
                case MD_WORLD_MODE_LCTG:
                case MD_WORLD_MODE_LFTG:
                case MD_WORLD_MODE_LFCTG:
                    activeMdType = ACTIVE_MD_TYPE_LTG;
                    break;
                case MD_WORLD_MODE_LWG:
                case MD_WORLD_MODE_LTWG:
                case MD_WORLD_MODE_NLWG:
                    activeMdType = ACTIVE_MD_TYPE_LWG;
                    break;
                case MD_WORLD_MODE_LWTG:
                case MD_WORLD_MODE_LWCTG:
                case MD_WORLD_MODE_NLWTG:
                case MD_WORLD_MODE_NLWCTG:
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
                case MD_WORLD_MODE_LWCG:
                case MD_WORLD_MODE_LFWCG:
                case MD_WORLD_MODE_LTWCG:
                    activeMdType = ACTIVE_MD_TYPE_LWCG;
                    break;
                case MD_WORLD_MODE_LTTG:
                case MD_WORLD_MODE_LTCTG:
                    activeMdType = ACTIVE_MD_TYPE_LtTG;
                    break;
                case MD_WORLD_MODE_LFWG:
                    activeMdType = ACTIVE_MD_TYPE_LfWG;
                    break;
                default:
                    activeMdType = ACTIVE_MD_TYPE_UNKNOWN;
                break;
            }
        }
        Elog.d(TAG, "getActiveModemType=" + activeMdType + " activeMode=" + activeMode);
        return activeMdType;
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
        Elog.d(TAG, "get3GDivisionDuplexMode=" + duplexMode);
        return duplexMode;
    }

   private static boolean checkWmCapability(int worldMode, int bandMode) {
        int iRat = 0;
        if (worldMode == MD_WORLD_MODE_LTG) {
            iRat = (MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTTG) {
            iRat = (MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWTG) {
            iRat = (MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFWG) {
            iRat = (MASK_LTEFDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWG) {
            iRat = (MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFWCG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWCG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTWG) {
            iRat = (MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTWCG) {
            iRat = (MASK_CDMA |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFTG) {
            iRat = (MASK_LTEFDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_NLWG) {
            iRat = (MASK_NR |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_NLWTG) {
            iRat = (MASK_NR |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_NLWCTG) {
            iRat = (MASK_NR |
                    MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_TDSCDMA |
                    MASK_GSM);
        }
        if (true == IS_NR_SUPPORT) {
            bandMode = bandMode | MASK_NR;
        }
        if (true == isC2kSupport()) {
            bandMode = bandMode | MASK_CDMA;
        }
        if (true == isWorldPhoneSupport() &&
            (MASK_WCDMA == (iRat & MASK_WCDMA) || MASK_TDSCDMA == (iRat & MASK_TDSCDMA))) {
            bandMode = bandMode | MASK_WCDMA | MASK_TDSCDMA;
        }

        Elog.d(TAG, "checkWmCapab: modem=" + worldMode + " rat=" + iRat + " bandMode=" + bandMode);
        /*  1. iRat bit mask sholud be a subset of bandMode.
         *  2. C2K should be support in worldMode and bandMode at the same time.
         */
        if (iRat == (iRat & bandMode) &&
                (iRat & MASK_CDMA) == (bandMode & MASK_CDMA)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Trigger RILD to switch world mode.
     * @param worldMode 0 : world mode is unknown
     *                  8 : uTLG (MD_WORLD_MODE_LTG)
     *                  9 : uLWG (MD_WORLD_MODE_LWG)
     *                  10 : uLWTG (MD_WORLD_MODE_LWTG)
     *                  11 : uLWCG (MD_WORLD_MODE_LWCG)
     *                  12 : uLWTCG (MD_WORLD_MODE_LWCTG)
     *                  13 : LtTG (MD_WORLD_MODE_LTTG)
     *                  14 : LfWG (MD_WORLD_MODE_LFWG)
     *                  15 : uLfWCG (MD_WORLD_MODE_LFWCG)
     *                  16 : uLCTG (MD_WORLD_MODE_LCTG)
     *                  17 : uLtCTG (MD_WORLD_MODE_LTCTG)
     *                  18 : uLtWG (MD_WORLD_MODE_LTWG)
     *                  19 : uLtWCG (MD_WORLD_MODE_LTWCG)
     *                  20 : uLfTG (MD_WORLD_MODE_LFTG)
     *                  21 : uLfCTG (MD_WORLD_MODE_LFCTG)
     *                  22 : uNLWG (MD_WORLD_MODE_NLWG)
     *                  23 : NLWTG (MD_WORLD_MODE_NLWTG)
     *                  24 : NLWCTG (MD_WORLD_MODE_NLWCTG)
     *
     * @param bandMode  ( 1<< 6) : MASK_NR
     *                  (1 << 5) : MASK_CDMA
     *                  (1 << 4) : MASK_LTEFDD
     *                  (1 << 3) : MASK_LTETDD
     *                  (1 << 4) : MASK_WCDMA
     *                  (1 << 1) : MASK_TDSCDMA
     *                  (1) : MASK_GSM
     * @return WORLD_MODE_RESULT_SUCCESS if WM-ID is correct.
     */
    public static int setWorldModeWithBand(int worldMode, int bandMode) {
        if (checkWmCapability(worldMode, bandMode) == false) {
            Elog.d(TAG, "setWorldModeWithBand: not match, modem=" + worldMode + " bandMode=" + bandMode);
            return WORLD_MODE_RESULT_WM_ID_NOT_SUPPORT;
        }
        setWorldMode(worldMode);
        return WORLD_MODE_RESULT_SUCCESS;
    }

    public static int getMajorSim() {
        String currMajorSim = SystemProperties.get(PROPERTY_MAJOR_SIM, "");
        if (currMajorSim != null && !currMajorSim.equals("")) {
            Elog.d(TAG, "[getMajorSim]: " + ((Integer.parseInt(currMajorSim)) - 1));
            return (Integer.parseInt(currMajorSim)) - 1;
        } else {
            Elog.d(TAG, "[getMajorSim]: fail to get major SIM");
            return -99;
        }
    }

    private static void setWorldMode(int worldMode) {
        Elog.d(TAG, "[setWorldMode] worldMode=" + worldMode);
        int currentWorldMode = getWorldModeId();
        if (worldMode == currentWorldMode) {
            if (worldMode == MD_WORLD_MODE_LTG) {
                Elog.d(TAG, "Already in uTLG mode");
            } else if (worldMode == MD_WORLD_MODE_LWG) {
                Elog.d(TAG, "Already in uLWG mode");
            } else if (worldMode == MD_WORLD_MODE_LWTG) {
                Elog.d(TAG, "Already in uLWTG mode");
            } else if (worldMode == MD_WORLD_MODE_LWCG) {
                Elog.d(TAG, "Already in uLWCG mode");
            } else if (worldMode == MD_WORLD_MODE_LWCTG) {
                Elog.d(TAG, "Already in uLWTCG mode");
            } else if (worldMode == MD_WORLD_MODE_LTTG) {
                Elog.d(TAG, "Already in LtTG mode");
            } else if (worldMode == MD_WORLD_MODE_LFWG) {
                Elog.d(TAG, "Already in LfWG mode");
            } else if (worldMode == MD_WORLD_MODE_LFWCG) {
                Elog.d(TAG, "Already in uLfWCG mode");
            } else if (worldMode == MD_WORLD_MODE_LCTG) {
                Elog.d(TAG, "Already in uLCTG mode");
            } else if (worldMode == MD_WORLD_MODE_LTCTG) {
                Elog.d(TAG, "Already in uLtCTG mode");
            } else if (worldMode == MD_WORLD_MODE_LTWG) {
                Elog.d(TAG, "Already in uLtWG mode");
            } else if (worldMode == MD_WORLD_MODE_LTWCG) {
                Elog.d(TAG, "Already in uLtWCG mode");
            } else if (worldMode == MD_WORLD_MODE_LFTG) {
                Elog.d(TAG, "Already in uLfTG mode");
            } else if (worldMode == MD_WORLD_MODE_LFCTG) {
                Elog.d(TAG, "Already in uLfCTG mode");
            } else if (worldMode == MD_WORLD_MODE_NLWG) {
                Elog.d(TAG, "Already in uNLWG mode");
            } else if (worldMode == MD_WORLD_MODE_NLWTG) {
                Elog.d(TAG, "Already in uNLWTG mode");
            } else if (worldMode == MD_WORLD_MODE_NLWCTG) {
                Elog.d(TAG, "Already in uNLWCTG mode");
            }
            return;
        }
        int maxMode = MD_WORLD_MODE_LFCTG;
        if (true == IS_NR_SUPPORT) {
            maxMode = MD_WORLD_MODE_NLWCTG;
        }
        if ((worldMode >= MD_WORLD_MODE_LTG) &&
                (worldMode <= maxMode)) {
            String str = "MTK worldmodeid," + String.valueOf(worldMode);
            if (isC2kSupport() == true ||
                    FeatureSupport.is93Modem() == true) {
                EmUtils.invokeOemRilRequestStringsEm(new String[] {STATUS_SYNC_PREFIX, str}, null);
            } else {
                EmUtils.reloadModemType(worldMode);
                EmUtils.storeModemType(worldMode);
                EmUtils.rebootModem();
            }
        } else {
            Elog.d(TAG, "Invalid world mode:" + worldMode);
            return;
        }
    }

    /**
     * return world mode string.
     * @param worldMode : valid world mode id.
     * @return the rat string of world mode.
     */
    public static String worldModeIdToString(int worldMode) {
        String worldModeString = "unknown";
        String duplexMode = "unknown";
        int activeMode = Integer.valueOf(
                SystemProperties.get("vendor.ril.nw.worldmode.activemode", "0"));
        if (activeMode == 1) {
            duplexMode = "FDD";
        } else if (activeMode == 2) {
            duplexMode = "TDD";
        }
        switch (worldMode) {
            case MD_WORLD_MODE_LTG:
                worldModeString = "LTG";
                break;
            case MD_WORLD_MODE_LWG:
                worldModeString = "LWG";
                break;
            case MD_WORLD_MODE_LWTG:
                worldModeString = "LWTG(Auto mode:" + duplexMode + ")";
                break;
            case MD_WORLD_MODE_LWCG:
                worldModeString = "LWCG";
                break;
            case MD_WORLD_MODE_LWCTG:
                worldModeString = "LWCTG(Auto mode:" + duplexMode + ")";
                break;
            case MD_WORLD_MODE_LTTG:
                worldModeString = "LtTG";
                break;
            case MD_WORLD_MODE_LFWG:
                worldModeString = "LfWG";
                break;
            case MD_WORLD_MODE_LFWCG:
                worldModeString = "LfWCG";
                break;
            case MD_WORLD_MODE_LCTG:
                worldModeString = "LCTG";
                break;
            case MD_WORLD_MODE_LTCTG:
                worldModeString = "LtCTG";
                break;
            case MD_WORLD_MODE_LTWG:
                worldModeString = "LtWG";
                break;
            case MD_WORLD_MODE_LTWCG:
                worldModeString = "LtWCG";
                break;
            case MD_WORLD_MODE_LFTG:
                worldModeString = "LfTG";
                break;
            case MD_WORLD_MODE_LFCTG:
                worldModeString = "LfCTG";
                break;
            case MD_WORLD_MODE_NLWG:
                worldModeString = "NLWG";
                break;
            case MD_WORLD_MODE_NLWTG:
                worldModeString = "NLWTG(Auto mode:" + duplexMode + ")";
                break;
            case MD_WORLD_MODE_NLWCTG:
                worldModeString = "NLWCTG(Auto Mmode:" + duplexMode + ")";
                break;
            default:
                worldModeString = "unknown world mode id";
                break;
        }
        return worldModeString;
    }

}
