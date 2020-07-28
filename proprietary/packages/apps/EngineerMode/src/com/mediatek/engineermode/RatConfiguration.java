/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.engineermode;

import android.os.SystemProperties;

public class RatConfiguration {
    private static final String LOG_TAG = "RatConfig";

    /* the project config rat which is defined at build time */
    static final String PROPERTY_BUILD_RAT_CONFIG = "ro.vendor.mtk_protocol1_rat_config";
    /* the system property of active rat */
    static final String PROPERTY_RAT_CONFIG = "ro.vendor.mtk_ps1_rat";
    static final String PROPERTY_IS_USING_DEFAULT_CONFIG="ro.boot.opt_using_default";
    /* the valid characters of the human-readable rat config */
    /* the defination must be sync with ratconfig.c */
    static final String CDMA = "C";
    static final String LteFdd = "Lf";
    static final String LteTdd = "Lt";
    static final String WCDMA = "W";
    static final String TDSCDMA = "T";
    static final String GSM = "G";
    static final String NR = "N";
    static final String DELIMITER = "/";

    /* bitmask */
    /* the defination must be sync with ratconfig.c */
    public static final int MASK_NR = (1 << 6);
    public static final int MASK_CDMA = (1 << 5);
    public static final int MASK_LteFdd = (1 << 4);
    public static final int MASK_LteTdd = (1 << 3);
    public static final int MASK_WCDMA = (1 << 2);
    public static final int MASK_TDSCDMA = (1 << 1);
    public static final int MASK_GSM = (1);

    protected static final int MD_MODE_UNKNOWN = 0;
    protected static final int MD_MODE_LTG     = 8;   //uLTG
    protected static final int MD_MODE_LWG     = 9;   //uLWG
    protected static final int MD_MODE_LWTG    = 10;  //uLWTG
    protected static final int MD_MODE_LWCG    = 11;  //uLWCG
    protected static final int MD_MODE_LWCTG   = 12;  //uLWTCG(Auto mode)
    protected static final int MD_MODE_LTTG    = 13;  //LtTG
    protected static final int MD_MODE_LFWG    = 14;  //LfWG
    protected static final int MD_MODE_LFWCG   = 15;  //uLfWCG
    protected static final int MD_MODE_LCTG    = 16;  //uLCTG
    protected static final int MD_MODE_LTCTG   = 17;  //uLtCTG

    private static int max_rat = 0;
    private static boolean max_rat_initialized = false;
    private static int actived_rat = 0;
    private static boolean is_default_config = true;

    /*
     * transfer rat format from "L/T/G" to bitmask
     * @params String rat, the rat in format like "L/T/G"
     * @return int, the rat in bitmask.
     */
    protected static int ratToBitmask(String rat) {
        int iRat = 0;
        if (rat.contains(CDMA)) {
            iRat = iRat | MASK_CDMA;
        }
        if (rat.contains(LteFdd)) {
            iRat = iRat | MASK_LteFdd;
        }
        if (rat.contains(LteTdd)) {
            iRat = iRat | MASK_LteTdd;
        }
        if (rat.contains(WCDMA)) {
            iRat = iRat | MASK_WCDMA;
        }
        if (rat.contains(TDSCDMA)) {
            iRat = iRat | MASK_TDSCDMA;
        }
        if (rat.contains(GSM)) {
            iRat = iRat | MASK_GSM;
        }
        if (rat.contains(NR)) {
            iRat = iRat | MASK_NR;
        }
        return iRat;
    }

    /*
     * get the rat of project config
     * @return int, the rat in bitmask.
     */
    synchronized protected static int getMaxRat() {
        if( !max_rat_initialized) {
            String sMaxRat = SystemProperties.get(PROPERTY_BUILD_RAT_CONFIG, "");
            max_rat = ratToBitmask(sMaxRat);
            is_default_config =
                    (SystemProperties.getInt(PROPERTY_IS_USING_DEFAULT_CONFIG, 1) != 0) ?
                    true : false;
            max_rat_initialized = true;
            logd("getMaxRat: initial " + sMaxRat + " "+max_rat);
        }
        //logd("getMaxRat: " + max_rat);
        return max_rat;
    }

    /*
     * check rat config is supported by project config
     * @params int iRat, the rat to be checked.
     * @return boolean,
     *              true, the rat is supported.
     *              false, the rat is not supported.
     */
    protected static boolean checkRatConfig(int iRat) {
        int maxrat = getMaxRat();
        if ( (iRat | maxrat) == maxrat) {
            return true;
        } else {
            logd("checkRatConfig: FAIL with " + String.valueOf(iRat));
            return false;
        }
    }

    /*
     * get the active rat in bitmask.
     * @return int, the rat in bitmask.
     */
    protected static int getRatConfig() {
        int default_rat_config = getMaxRat();
        if (default_rat_config == 0) {
            actived_rat = 0;
            return actived_rat;
        }
        if (is_default_config) {
            actived_rat = default_rat_config;
            return default_rat_config;
        }
        String rat = SystemProperties.get(PROPERTY_RAT_CONFIG, "");
        if (rat.length() > 0) {
            actived_rat = ratToBitmask(rat);
            if (checkRatConfig(actived_rat) == false) {
                logd("getRatConfig: invalid PROPERTY_RAT_CONFIG, set to max_rat");
                actived_rat = getMaxRat();
            }
        } else {
            logd("getRatConfig: ger property PROPERTY_RAT_CONFIG fail, initialize");
            actived_rat = getMaxRat();
        }
        return actived_rat;
    }

    /*
     * transfer the format from bitmask to "L/T/G".
     * @params int iRat, rat in bitmask
     * @return String, rat in format like "L/T/G".
     */
    protected static String ratToString(int iRat) {
        String rat = "";
        if ((iRat & MASK_CDMA) == MASK_CDMA){
            rat += (DELIMITER + CDMA);
        }
        if ((iRat & MASK_LteFdd) == MASK_LteFdd){
            rat += (DELIMITER + LteFdd);
        }
        if ((iRat & MASK_LteTdd) == MASK_LteTdd){
            rat += (DELIMITER + LteTdd);
        }
        if ((iRat & MASK_WCDMA) == MASK_WCDMA){
            rat += (DELIMITER + WCDMA);
        }
        if ((iRat & MASK_TDSCDMA) == MASK_TDSCDMA){
            rat += (DELIMITER + TDSCDMA);
        }
        if ((iRat & MASK_GSM) == MASK_GSM){
            rat += (DELIMITER + GSM);
        }
        if ((iRat & MASK_NR) == MASK_NR){
            rat += (DELIMITER + NR);
        }
        if (rat.length() > 0) {
            // for remove the delimiter at rat[0]
            rat = rat.substring(1);
        }
        return rat;
    }

    /*
     * check C2k suppport
     * @return boolean, cases as following
     *       true, rat is active and project config supports it.
     *       false, rat is inactive no matter project config supports.
     */
    public static boolean isC2kSupported() {
        return (getMaxRat() & getRatConfig() & MASK_CDMA) == MASK_CDMA ? true : false;
    }

    /*
     * check LteFdd suppport
     * @return boolean, cases as following
     *       true, rat is active and project config supports it.
     *       false, rat is inactive no matter project config supports.
     */
    public static boolean isLteFddSupported() {
        return (getMaxRat() & getRatConfig() & MASK_LteFdd) == MASK_LteFdd? true : false;
    }

    /*
     * check LteTdd suppport
     * @return boolean, cases as following
     *       true, rat is active and project config supports it.
     *       false, rat is inactive no matter project config supports.
     */
    public static boolean isLteTddSupported() {
        return (getMaxRat() & getRatConfig() & MASK_LteTdd) == MASK_LteTdd? true : false;
    }

    /*
     * check Wcdma suppport
     * @return boolean, cases as following
     *       true, rat is active and project config supports it.
     *       false, rat is inactive no matter project config supports.
     */
    public static boolean isWcdmaSupported() {
        return (getMaxRat() & getRatConfig() & MASK_WCDMA) == MASK_WCDMA? true : false;
    }

    /*
     * check Tdscdma suppport
     * @return boolean, cases as following
     *       true, rat is active and project config supports it.
     *       false, rat is inactive no matter project config supports.
     */
    public static boolean isTdscdmaSupported() {
        return (getMaxRat() & getRatConfig() & MASK_TDSCDMA) == MASK_TDSCDMA? true : false;
    }

    /*
     * check GSM suppport
     * @return boolean, cases as following
     *       true, rat is active and project config supports it.
     *       false, rat is inactive no matter project config supports.
     */
    public static boolean isGsmSupported() {
        return (getMaxRat() & getRatConfig() & MASK_GSM) == MASK_GSM? true : false;
    }

    /*
     * check NR suppport
     * @return boolean, cases as following
     *       true, rat is active and project config supports it.
     *       false, rat is inactive no matter project config supports.
     */
    public static boolean isNrSupported() {
        return (getMaxRat() & getRatConfig() & MASK_NR) == MASK_NR? true : false;
    }

    /*
     * get the active rat
     * @return String, the rat in format like C/Lf/Lt/T/W/G
     */
    public static String getActiveRatConfig() {
        String rat = ratToString(getRatConfig());
        logd("getActiveRatConfig: " + rat);
        return rat;
    }

    private static void logd(String msg) {
        Elog.d(LOG_TAG, msg);
    }

}
