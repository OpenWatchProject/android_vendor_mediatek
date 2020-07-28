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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.content.Context;
import android.content.res.Resources;
import android.os.SystemProperties;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.Rlog;

import mediatek.telephony.MtkServiceState;

public class ServiceStateTrackerExt implements IServiceStateTrackerExt {
    static final String TAG = "SSTExt";
    protected Context mContext;

    private static final String[] PROPERTY_RIL_FULL_UICC_TYPE  = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };
    private static final int CARD_TYPE_NONE = 0;
    private static final int CARD_TYPE_USIM = 1;
    private static final int CARD_TYPE_CSIM = 2;
    private static final int CARD_TYPE_RUIM = 4;

    public ServiceStateTrackerExt() {
    }

    public ServiceStateTrackerExt(Context context) {
        mContext = context;
    }

    public String onUpdateSpnDisplay(String plmn, MtkServiceState ss, int phoneId) {
        return plmn;
    }

    public boolean isImeiLocked() {
        return false;
    }

    public boolean isBroadcastEmmrrsPsResume(int value) {
        return false;
    }

    public boolean needEMMRRS() {
        return false;
    }

    public boolean needSpnRuleShowPlmnOnly() {
        //[ALPS01679495]-start: don't show SPN for CTA case
        if (SystemProperties.get("ro.vendor.mtk_cta_support").equals("1")) {
            return true;
        }
        //[ALPS01679495]-end
        return false;
    }

    public boolean needBrodcastAcmt(int errorType, int errorCause) {
        return false;
    }

    public boolean needRejectCauseNotification(int cause) {
        return false;
    }
    /**
     * Ignore for Femtocell for given cause and state.
     * @param state current state
     * @param cause current cause
     * @return true/false
     */
    public boolean needIgnoreFemtocellUpdate(int state, int cause) {
        return false;
    }
    /**
     * Show CSG ID or not.
     * @param hnbName HNB name of network
     * @return true/false
     */
    public boolean needToShowCsgId() {
        return true;
    }
    public boolean needBlankDisplay(int cause) {
        return false;
    }
    public boolean needIgnoredState(int state, int newState, int cause) {
        if ((state == ServiceState.STATE_IN_SERVICE) && (newState == 2)) {
            /* Don't update for searching state, there shall be final registered state
               update later */
            Rlog.i(TAG, "set dontUpdateNetworkStateFlag for searching state");
            return true;
        }

        /* -1 means modem didn't provide <cause> information. */
        if (cause != -1) {
            // [ALPS01384143] need to check if previous state is IN_SERVICE for invalid sim
            if ((state == ServiceState.STATE_IN_SERVICE) && (newState == 3) && (cause != 0)) {
            //if((newState == 3) && (cause != 0)){
                /* This is likely temporarily network failure, don't update for better UX */
                Rlog.i(TAG, "set dontUpdateNetworkStateFlag for REG_DENIED with cause");
                return true;
            //[ALPS01976914] - start
            } else if ((state == ServiceState.STATE_IN_SERVICE)
                    && (newState == 0)
                    && (cause != 0)) {
                Rlog.i(TAG, "set dontUpdateNetworkStateFlag for NOT_REG_AND_NOT_SEARCH with cause");
                return true;
            }
            //[ALPS01976914] - end
        }

        Rlog.i(TAG, "clear dontUpdateNetworkStateFlag");

        return false;
    }

    public boolean operatorDefinedInternationalRoaming(String operatorNumeric) {
        return false;
    }

    public void log(String text) {
        Rlog.d(TAG, text);
    }

    public boolean allowSpnDisplayed() {
        return true;
    }

    //[ALPS01862093]-Start: not supprot auto switch rat for SIM card type
    //du to UI spec. define chagend
    public int needAutoSwitchRatMode(int phoneId, String nwPlmn) {
        return -1;
    }

    public boolean isSupportRatBalancing() {
        return false;
    }

    //[ALPS02257292] To support disable IVSR
    /**
     * Return if need disable IVSR.
     * @return if need disable IVSR
     */
    public boolean isNeedDisableIVSR() {
        return false;
    }

    public String onUpdateSpnDisplayForIms(
                      String plmn, MtkServiceState ss, int lac, int phoneId, Object simRecords) {
        return plmn;
    }

    private boolean isCdmaLteDcSupport() {
        if (SystemProperties.get("ro.vendor.mtk_c2k_lte_mode").equals("1")
            || SystemProperties.get("ro.vendor.mtk_c2k_lte_mode").equals("2")) {
            return true;
        }
        return false;
    }


    private String[] getSupportCardType(int slotId) {
        String property = null;
        String prop = null;
        String values[] = null;

        if (slotId < 0 || slotId >= PROPERTY_RIL_FULL_UICC_TYPE.length) {
            log("getSupportCardType: invalid slotId " + slotId);
            return null;
        }
        prop = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[slotId], "");
        if ((!prop.equals("")) && (prop.length() > 0)) {
            values = prop.split(",");
        }
        log("getSupportCardType slotId " + slotId + ", prop value= " + prop +
                ", size= " + ((values != null) ? values.length : 0));
        return values;
    }

    private boolean isCdma4GCard(int slotId) {
        String values[] = null;
        int cardType = CARD_TYPE_NONE;

        // Get the support card type.
        values = getSupportCardType(slotId);
        if (values == null) {
            log("isCdma4GCard, get non support card type");
            return false;
        }

        // Parse the card type and judge whether Cdma4G card or not.
        for (int i = 0; i < values.length; i++) {
            if ("USIM".equals(values[i])) {
                cardType |= CARD_TYPE_USIM;
            } else if ("RUIM".equals(values[i])) {
                cardType |= CARD_TYPE_RUIM;
            } else if ("CSIM".equals(values[i])) {
                cardType |= CARD_TYPE_CSIM;
            }
        }
        log("isCdma4GCard, cardType=" + cardType);
        if (((cardType & CARD_TYPE_RUIM) > 0
                || (cardType & CARD_TYPE_CSIM) > 0)
             && ((cardType & CARD_TYPE_USIM) > 0)) {
            return true;
        }
        return false;
    }


    /**
     * update roaming status for some special SIM.
     *
     * @param strServingPlmn the plmn which the phone has registed currently.
     * @param strHomePlmn the previous five number of IMSI
     * @return whether show roaming or not
     */
    public boolean isRoamingForSpecialSIM(String strServingPlmn, String strHomePlmn) {
        boolean cdmaLteSupport = isCdmaLteDcSupport();
        log("isRoamingForSpecialSIM, strServingPlmn: " + strServingPlmn + ", strHomePlmn: " +
             strHomePlmn + ", cdmaLteSupport = " + cdmaLteSupport);
        if (cdmaLteSupport) {
            if (strServingPlmn != null && !strServingPlmn.startsWith("460")) {
                if ("45403".equals(strHomePlmn) || "45404".equals(strHomePlmn)) {
                    Rlog.d(TAG, "special SIM, force roaming. IMSI:" + strHomePlmn);
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * For TMO, show ECC when PS is in service but IMS's ECC is support.
     * @hide
     */
    public boolean showEccForIms() {
        return false;
    }

    public boolean getMtkRsrpOnly() {
        return true;
    }

    public int[] getMtkLteRsrpThreshold() {
        return null;
    }

    public int[] getMtkLteRssnrThreshold() {
        return null;
    }
}
