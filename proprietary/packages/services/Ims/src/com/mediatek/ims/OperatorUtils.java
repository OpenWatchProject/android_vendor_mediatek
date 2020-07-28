/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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

package com.mediatek.ims;

import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.ims.common.SubscriptionManagerHelper;
import com.mediatek.ims.ImsConstants;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


/**
 * Operator Utility to get operator information.
 */
public class OperatorUtils {

    public enum OPID {
        OP01, // CMCC
        OP02, // CU
        OP03, // Orange
        OP05, // TMO EU
        OP06, // VDF
        OP07, // AT&T
        OP08, // TMO US
        OP09, // CT
        OP11, // H3G
        OP12, // VzW
        OP15, // Telefonica
        OP16, // EE
        OP18, // RJIL
        OP50, // Softbank
        OP129,  // KDDI
        OP156,  // Telenor
        OP130, // Brazil-TIM
        OP120, // Brazil-Claro
        OP132, // Brazil-v1vo
        OPOi,  // Brazil-Oi
        OP165, // Sunrise
        OP152, // Optus
        OP117, // Smartfen
        OP131, // TrueMove
        OP125, // DTAC
        OP132_Peru, // Movistar
        OP147, //Airtel-India
        OP151, // M1
        OP236,  // USCC
    }

    private static final String LOG_TAG = "OperatorUtils";

    private static final String PROPERTY_MTK_DYNAMIC_IMS_SWITCH =
                                    "persist.vendor.mtk_dynamic_ims_switch";
    private static final String PROPERTY_MTK_CT_VOLTE_SUPPORT =
                                    "persist.vendor.mtk_ct_volte_support";
    private static final String PROPERTY_MTK_UIM_SUBSCRIBERID = "vendor.ril.uim.subscriberid";

    private static final Map<OPID, List> mOPMap = new HashMap<OPID, List>() {
        {
            put(OPID.OP01, Arrays.asList("46000", "46002", "46004", "46007", "46008"));
            put(OPID.OP02, Arrays.asList("46001", "46006", "46009", "45407"));
            put(OPID.OP03, Arrays.asList("20801", "20802"));
            put(OPID.OP05, Arrays.asList("23203", "23204", "21901", "23001", "21630",
                    "29702", "20416", "20420", "26002", "22004", "23430", "26201", "26206", "26278"));
            put(OPID.OP06, Arrays.asList("21401", "21406", "20404", "28602", "23415", "27602",
                    "23003", "23099", "60202", "28802", "54201", "26202", "26204", "26209",
                    "62002", "20205", "21670", "27402", "27403", "27201", "22210", "27801",
                    "53001", "26801", "22601", "42702"));
            put(OPID.OP07, Arrays.asList("310030", "310070", "310090", "310150",
                    "310170", "310280", "310380", "310410", "310560", "310680", "311180"));
            put(OPID.OP08, Arrays.asList("310160", "310260", "310490", "310580", "310660",
                    "310200", "310210", "310220", "310230", "310240", "310250", "310270",
                    "310310", "310800"));
            put(OPID.OP09, Arrays.asList("46003", "46011", "46012", "45502", "45507"));
            put(OPID.OP11, Arrays.asList("23420"));
            put(OPID.OP12, Arrays.asList("310590", "310890", "311270", "311480"));
            put(OPID.OP15, Arrays.asList("26203", "26207", "26208", "26211", "26277"));
            put(OPID.OP16, Arrays.asList("23430", "23431", "23432", "23433", "23434"));
            put(OPID.OP18, Arrays.asList("405854", "405855", "405856","405872", "405857", "405858",
                    "405859", "405860", "405861", "405862", "405873", "405863", "405864", "405874",
                    "405865", "405866", "405867", "405868", "405869", "405871", "405870", "405840"));
            put(OPID.OP50, Arrays.asList("44020"));
            put(OPID.OP129, Arrays.asList("44007", "44008",
                    "44050", "44051", "44052", "44053", "44054", "44055", "44056",
                    "44070", "44071", "44072", "44073", "44074", "44075", "44076", "44077", "44078", "44079",
                    "44088", "44089", "44110", "44170"));

            put(OPID.OP156, Arrays.asList("23802"));
            put(OPID.OP130, Arrays.asList("72402", "72403", "72404"));
            put(OPID.OP120, Arrays.asList("72405"));
            put(OPID.OP132, Arrays.asList("72406", "72410", "72411", "72423"));
            put(OPID.OPOi, Arrays.asList("72416", "72424", "72431"));
            put(OPID.OP165, Arrays.asList("22802"));
            put(OPID.OP152, Arrays.asList("50502"));
            put(OPID.OP117, Arrays.asList("51009", "51028"));
            put(OPID.OP131, Arrays.asList("52004"));
            put(OPID.OP125, Arrays.asList("52005"));
            put(OPID.OP132_Peru, Arrays.asList("71606"));
            put(OPID.OP147, Arrays.asList("40410", "40431",
                "40440", "40445", "40449", "40551", "40552", "40553", "40554", "40555",
                "40556", "40490", "40492", "40493", "40494", "40495", "40496", "40497",
                "40498", "40402", "40403", "40416", "40470", "405030","405035","405036",
                "405037","405038","405039","405044"));
            put(OPID.OP151, Arrays.asList("52503"));
            put(OPID.OP236, Arrays.asList("31100", "311220", "311221", "311222", "311223", "311224",
                    "311225", "311226", "311227", "311228", "311229", "311580", "311581", "311582",
                    "311583", "311584", "311585", "311586", "311587", "311588", "311589"));
        }
    };

    public static boolean isMainCapabilitySimOperator(OPID id) {
        int phoneId = getMainCapabilityPhoneId();
        return isMatched(id, phoneId);
    }

    public static boolean isMatched(OPID id, int phoneId) {
        String mccMnc = getSimOperatorNumericForPhone(phoneId);
        // Rlog.d(LOG_TAG, "mccMnc: " + mccMnc);
        List mccMncList = mOPMap.get(id);
        if (mccMncList != null && mccMncList.contains(mccMnc)) {
            return true;
        }
        return false;
    }

    private static int getMainCapabilityPhoneId() {
       int phoneId = SystemProperties.getInt(ImsConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
       if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
           phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
       }
       Rlog.d(LOG_TAG, "getMainCapabilityPhoneId = " + phoneId);
       return phoneId;
   }

    /**
     * Returns the MCC+MNC (mobile country code + mobile network code) of the
     * provider of the SIM for a particular subscription. 5 or 6 decimal digits.
     *
     * @param phoneId for which SimOperator is returned
     */
    public static String getSimOperatorNumericForPhone(int phoneId) {
        String mccMncPropertyName = "";
        String mccMnc = "";
        int phoneType = 0;
        int subId = SubscriptionManagerHelper.getSubIdUsingPhoneId(phoneId);

        phoneType = TelephonyManager.getDefault().getCurrentPhoneType(subId);

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

        Rlog.w(LOG_TAG, "getMccMnc, mccMnc value:" + Rlog.pii(LOG_TAG, mccMnc));

        return mccMnc;
    }

      /**
     * to check if CT VoLTE disable.
     *
     * @return True if CT volte disabled and it is a CT card .
     */
    public static boolean isCTVolteDisabled(int phoneId) {
        if ("1".equals(SystemProperties.get(PROPERTY_MTK_DYNAMIC_IMS_SWITCH)) &&
            (SystemProperties.getInt(PROPERTY_MTK_CT_VOLTE_SUPPORT, 0) == 0)) {
            if (isMatched(OPID.OP09, phoneId)) {
                Rlog.d(LOG_TAG, "SIM loaded, but CT VoLTE shall not support");
                return true;
            }
        }
        return false;
    }

    public static boolean isOperator(String mccMnc, OPID id) {
        boolean r = false;
        if (mOPMap.get(id).contains(mccMnc)) {
            r = true;
        }


        Rlog.d(LOG_TAG, "isOperator: id = " + Rlog.pii(LOG_TAG, id)
                + ", matched = " + (r ? "true" : "false"));
        return r;
    }
}
