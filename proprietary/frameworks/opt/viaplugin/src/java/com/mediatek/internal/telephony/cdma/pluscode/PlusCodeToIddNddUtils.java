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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.internal.telephony.cdma.pluscode;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.SparseIntArray;

import com.android.internal.telephony.PhoneConstants;

//import com.mediatek.internal.telephony.cdma.pluscode.IPlusCodeUtils;
//import com.mediatek.internal.telephony.ITelephonyEx;

import java.util.List;

/**
 * The utilities of converting plus code to IddNdd.
 * @hide
 */
public class PlusCodeToIddNddUtils {
    static final String LOG_TAG = "PlusCodeToIddNddUtils";
    private static final boolean DBG = "eng".equals(Build.TYPE);

    public static final String INTERNATIONAL_PREFIX_SYMBOL = "+";

    private static PlusCodeHpcdTable sHpcd = PlusCodeHpcdTable.getInstance();
    private static MccIddNddSid sMccIddNddSid = null;

    private static int getCdmaSubId() {
        TelephonyManager tm = TelephonyManager.getDefault();
        int simCount = tm.getSimCount();
        for (int i = 0; i < simCount; i++) {
            if (tm.getCurrentPhoneTypeForSlot(i) == PhoneConstants.PHONE_TYPE_CDMA) {
                int subId[] = SubscriptionManager.getSubId(i);
                if (subId != null) {
                    return subId[0];
                }
            }
        }
        return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }

    /**
     * @return if can convert plus code to IddNdd.
     */
    public static boolean canFormatPlusToIddNdd() {
        Rlog.d(LOG_TAG, "-------------canFormatPlusToIddNdd-------------");
        int cdmaSubId = getCdmaSubId();
        TelephonyManager tm = TelephonyManager.getDefault();
        String value = tm.getNetworkOperator(cdmaSubId);
        Rlog.d(LOG_TAG, "cdmaSubId:" + cdmaSubId + ", network operator numeric:" + value);
        String mccStr = "";
        if (value.length() == 7) {
            mccStr = value.substring(0, 4);
        } else if (value.length() >= 3) {
            mccStr = value.substring(0, 3);
        }
        String sidStr = "";
        try {
            ServiceState ss = tm.getServiceStateForSubscriber(cdmaSubId);
            if (ss != null) {
                sidStr = Integer.toString(ss.getCdmaSystemId());
            }
        } catch (NullPointerException ex) {
            Rlog.d(LOG_TAG, "canFormatPlusToIddNdd, NullPointerException:" + ex);
        }
        String ltmoffStr = TelephonyManager.getTelephonyProperty(
                SubscriptionManager.getPhoneId(cdmaSubId),
                IPlusCodeUtils.PROPERTY_TIME_LTMOFFSET, "");
        Rlog.d(LOG_TAG, "mcc = " + mccStr + ", sid = " + Rlog.pii(LOG_TAG, sidStr)
                + ", ltm_off = " + ltmoffStr);

        boolean find = false;
        sMccIddNddSid = null;
        if (sHpcd != null) {
            boolean isValid = !mccStr.startsWith("2134");
            Rlog.d(LOG_TAG, "[canFormatPlusToIddNdd] Mcc = " + mccStr
                    + ", !Mcc.startsWith(2134) = " + isValid);

            if (!TextUtils.isEmpty(mccStr) && Character.isDigit(mccStr.charAt(0))
                    && !mccStr.startsWith("000") && isValid) {
                sMccIddNddSid = PlusCodeHpcdTable.getCcFromTableByMcc(mccStr);
                Rlog.d(LOG_TAG,
                        "[canFormatPlusToIddNdd] getCcFromTableByMcc mccIddNddSid = "
                        + Rlog.pii(LOG_TAG, sMccIddNddSid));
                find = (sMccIddNddSid != null) ? true : false;
            } else {
                List<String> mccArray = PlusCodeHpcdTable.getMccFromConflictTableBySid(sidStr);
                if (mccArray == null || mccArray.size() == 0) {
                    Rlog.d(LOG_TAG, "[canFormatPlusToIddNdd] Do not find cc by SID from confilcts"
                            + " table, so from lookup table");
                    sMccIddNddSid = PlusCodeHpcdTable.getCcFromMINSTableBySid(sidStr);
                    Rlog.d(LOG_TAG,
                            "[canFormatPlusToIddNdd] getCcFromMINSTableBySid mccIddNddSid = "
                            + Rlog.pii(LOG_TAG, sMccIddNddSid));
                } else if (mccArray.size() >= 2) {
                    String findMcc = sHpcd.getCcFromMINSTableByLTM(mccArray, ltmoffStr);
                    if (findMcc != null && findMcc.length() != 0) {
                        sMccIddNddSid = PlusCodeHpcdTable.getCcFromTableByMcc(findMcc);
                    }
                    Rlog.d(LOG_TAG,
                            "[canFormatPlusToIddNdd] conflicts, getCcFromTableByMcc mccIddNddSid = "
                            + Rlog.pii(LOG_TAG, sMccIddNddSid));
                } else if (mccArray.size() == 1) {
                    String findMcc = mccArray.get(0);
                    sMccIddNddSid = PlusCodeHpcdTable.getCcFromTableByMcc(findMcc);
                    Rlog.d(LOG_TAG,
                            "[canFormatPlusToIddNdd] do not conflicts, getCcFromTableByMcc" +
                            " mccIddNddSid = " + Rlog.pii(LOG_TAG, sMccIddNddSid));
                }
                find = (sMccIddNddSid != null) ? true : false;
            }
        }
        Rlog.d(LOG_TAG, "[canFormatPlusToIddNdd] find = " + find
                + ", mccIddNddSid = " + Rlog.pii(LOG_TAG, sMccIddNddSid));
        return find;
    }

    private static String formatPlusCode(String number) {
        String formatNumber = null;

        // after called canFormatPlusCodeForSms() function. we have known the
        // value of variable "Find" and "mccIddNddSid".
        if (sMccIddNddSid != null) {
            String sCC = sMccIddNddSid.mCc;
            Rlog.d(LOG_TAG, "number auto format correctly, mccIddNddSid = " +
                    Rlog.pii(LOG_TAG, sMccIddNddSid.toString()));
            if (!number.startsWith(sCC)) {
                // CC dismatch, remove +(already erased before), add IDD
                formatNumber = sMccIddNddSid.mIdd + number;
                if (DBG) {
                    Rlog.d(LOG_TAG,
                            "CC dismatch, remove +(already erased before), add IDD formatNumber = "
                            + Rlog.pii(LOG_TAG, formatNumber));
                }
            } else {
                // CC matched.
                String nddStr = sMccIddNddSid.mNdd;
                if (sMccIddNddSid.mCc.equals("86") || sMccIddNddSid.mCc.equals("853")) {
                    // just add "00" before of number, if cc is chinese.
                    Rlog.d(LOG_TAG, "CC matched, cc is chinese");
                    nddStr = "00";
                } else {
                    // remove +(already erased before) and CC, add NDD.
                    number = number.substring(sCC.length(), number.length());
                    if (DBG) {
                        Rlog.d(LOG_TAG, "[isMobileNumber] number = " + Rlog.pii(LOG_TAG, number));
                    }
                    if (isMobileNumber(sCC, number)) {
                        Rlog.d(LOG_TAG, "CC matched, isMobile = true Ndd = ");
                        nddStr = "";
                    }
                }
                formatNumber = nddStr + number;
                if (DBG) {
                    Rlog.d(LOG_TAG,
                            "CC matched, remove +(already erased before) and CC," +
                            " add NDD formatNumber = " + Rlog.pii(LOG_TAG, formatNumber));
                }
                // CC matched and the number is mobile phone number, do not add NDD
            }
        }

        return formatNumber;
    }

    /**
     * Replace plus code with IDD or NDD input: the number input by the user.
     * @param number the number value
     * @return the number after deal with plus code
     */
    public static String replacePlusCodeWithIddNdd(String number) {
        if (DBG) {
            Rlog.d(LOG_TAG, "replacePlusCodeWithIddNdd number = " + Rlog.pii(LOG_TAG, number));
        }
        if (number == null || number.length() == 0
                || !number.startsWith(INTERNATIONAL_PREFIX_SYMBOL)) {
            if (DBG) {
                Rlog.d(LOG_TAG, "number can't format correctly, number = "
                        + Rlog.pii(LOG_TAG, number));
            }
            return null;
        }

        boolean bFind = canFormatPlusToIddNdd();

        if (!bFind) {
            return null;
        }

        // remove "+" from the phone number;
        if (number.startsWith(INTERNATIONAL_PREFIX_SYMBOL)) {
            if (DBG) {
                Rlog.d(LOG_TAG, "number before remove plus char , number = "
                        + Rlog.pii(LOG_TAG, number));
            }
            number = number.substring(1, number.length());
            if (DBG) {
                Rlog.d(LOG_TAG, "number after   remove plus char , number = "
                        + Rlog.pii(LOG_TAG, number));
            }
        }

        String formatNumber = null;

        // after called canFormatPlusCodeForSms() function. we have known the
        // value of variable "Find" and "mccIddNddSid".
        if (bFind) {
            formatNumber = formatPlusCode(number);
        }

        return formatNumber;
    }

    private static final SparseIntArray MOBILE_NUMBER_SPEC_MAP =
            TelephonyPlusCode.MOBILE_NUMBER_SPEC_MAP;

    private static boolean isMobileNumber(String sCC, String number) {
        if (DBG) {
            Rlog.d(LOG_TAG, "[isMobileNumber] number = " + Rlog.pii(LOG_TAG, number)
                    + ", sCC = " + sCC);
        }
        if (number == null || number.length() == 0) {
            Rlog.d(LOG_TAG, "[isMobileNumber] please check the param ");
            return false;
        }
        boolean isMobile = false;

        if (MOBILE_NUMBER_SPEC_MAP == null) {
            Rlog.d(LOG_TAG, "[isMobileNumber] MOBILE_NUMBER_SPEC_MAP == null ");
            return isMobile;
        }

        int size = MOBILE_NUMBER_SPEC_MAP.size();
        int iCC;
        try {
            iCC = Integer.parseInt(sCC);
        } catch (NumberFormatException e) {
            Rlog.e(LOG_TAG, android.util.Log.getStackTraceString(e));
            return isMobile;
        }

        Rlog.d(LOG_TAG, "[isMobileNumber] iCC = " + iCC);
        for (int i = 0; i < size; i++) {
            Rlog.d(LOG_TAG, "[isMobileNumber] value = "
                    + MOBILE_NUMBER_SPEC_MAP.valueAt(i) + ", key =  "
                    + MOBILE_NUMBER_SPEC_MAP.keyAt(i));
            if (MOBILE_NUMBER_SPEC_MAP.valueAt(i) == iCC) {
                Rlog.d(LOG_TAG, "[isMobileNumber]  value = icc");
                String prfix = Integer.toString(MOBILE_NUMBER_SPEC_MAP.keyAt(i));
                Rlog.d(LOG_TAG, "[isMobileNumber]  prfix = " + prfix);
                if (number.startsWith(prfix)) {
                    Rlog.d(LOG_TAG, "[isMobileNumber]  number.startsWith(prfix) = true");
                    isMobile = true;
                    break;
                }
            }
        }

        return isMobile;
    }

    /**
     * Replace puls code with IDD or NDD input: the phone number for MT or
     * sender of sms or mms.
     * @param number the number value
     * @return the number after deal with plus code
     */
    public static String removeIddNddAddPlusCode(String number) {
        if (DBG) {
            Rlog.d(LOG_TAG, "[removeIddNddAddPlusCode] befor format number = "
                    + Rlog.pii(LOG_TAG, number));
        }
        if (number == null || number.length() == 0) {
            Rlog.d(LOG_TAG, "[removeIddNddAddPlusCode] please check the param ");
            return number;
        }

        String formatNumber = number;
        boolean bFind = false;

        if (!number.startsWith("+")) {
            bFind = canFormatPlusToIddNdd();

            if (!bFind) {
                Rlog.d(LOG_TAG,
                        "[removeIddNddAddPlusCode] find no operator that match the MCC ");
                return number;
            }

            if (sMccIddNddSid != null) {
                String strIdd = sMccIddNddSid.mIdd;
                Rlog.d(LOG_TAG,
                        "[removeIddNddAddPlusCode] find match the cc, Idd = " + strIdd);
                if (number.startsWith(strIdd) && number.length() > strIdd.length()) {
                    number = number.substring(strIdd.length(), number.length());
                    formatNumber = INTERNATIONAL_PREFIX_SYMBOL + number;
                }
            }
        }

        if (DBG) {
            Rlog.d(LOG_TAG, "[removeIddNddAddPlusCode] number after format = "
                    + Rlog.pii(LOG_TAG, formatNumber));
        }
        return formatNumber;
    }

    /**
     * @return if can format plus code for sms.
     */
    public static boolean canFormatPlusCodeForSms() {
        boolean canFormat = false;
        String mcc = SystemProperties.get(
                TelephonyPlusCode.PROPERTY_ICC_CDMA_OPERATOR_MCC, "");
        if (DBG) {
            Rlog.d(LOG_TAG, "[canFormatPlusCodeForSms] Mcc = " + mcc);
        }
        sMccIddNddSid = null;
        if (sHpcd != null) {
            if (DBG) {
                Rlog.d(LOG_TAG, "[canFormatPlusCodeForSms] Mcc = " + mcc);
            }
            if (mcc != null && mcc.length() != 0) {
                sMccIddNddSid = PlusCodeHpcdTable.getCcFromTableByMcc(mcc);
                if (DBG) {
                    Rlog.d(LOG_TAG,
                            "[canFormatPlusCodeForSms] getCcFromTableByMcc mccIddNddSid = "
                            + Rlog.pii(LOG_TAG, sMccIddNddSid));
                }
                canFormat = (sMccIddNddSid != null) ? true : false;
            }
        }
        return canFormat;

    }

    /**
     * Replace puls code, the phone number for MT or sender of sms or mms.
     * @param number the number value
     * @return the number after deal with plus code
     */
    public static String replacePlusCodeForSms(String number) {
        if (DBG) {
            Rlog.d(LOG_TAG, "replacePlusCodeForSms number = " + Rlog.pii(LOG_TAG, number));
        }
        if (number == null || number.length() == 0
                || !number.startsWith(INTERNATIONAL_PREFIX_SYMBOL)) {
            if (DBG) {
                Rlog.d(LOG_TAG, "number can't format correctly, number = "
                        + Rlog.pii(LOG_TAG, number));
            }
            return null;
        }

        boolean camFormat = canFormatPlusCodeForSms();
        if (!camFormat) {
            return null;
        }

        // remove "+" from the phone number;
        if (number.startsWith(INTERNATIONAL_PREFIX_SYMBOL)) {
            if (DBG) {
                Rlog.d(LOG_TAG, "number before remove plus char , number = "
                        + Rlog.pii(LOG_TAG, number));
            }
            number = number.substring(1, number.length());
            if (DBG) {
                Rlog.d(LOG_TAG, "number after   remove plus char , number = "
                        + Rlog.pii(LOG_TAG, number));
            }
        }

        String formatNumber = null;

        // after called canFormatPlusCodeForSms() function. we have known the
        // value of variable "Find" and "mccIddNddSid".
        if (camFormat) {
            formatNumber = formatPlusCode(number);
        }

        return formatNumber;

    }

    /**
     * Replace puls code with IDD or NDD input: the phone number for MT or
     * sender of sms or mms.
     * @param number the number value
     * @return the number after deal with plus code
     */
    public static String removeIddNddAddPlusCodeForSms(String number) {
        if (DBG) {
            Rlog.d(LOG_TAG, "[removeIddNddAddPlusCodeForSms] befor format number = "
                    + Rlog.pii(LOG_TAG, number));
        }
        if (number == null || number.length() == 0) {
            Rlog.d(LOG_TAG, "[removeIddNddAddPlusCodeForSms] please check the param ");
            return number;
        }

        String formatNumber = number;
        if (!number.startsWith("+")) {
            boolean camFormat = canFormatPlusCodeForSms();
            if (!camFormat) {
                Rlog.d(LOG_TAG,
                        "[removeIddNddAddPlusCodeForSms] find no operator that match the MCC ");
                return formatNumber;
            }

            if (sMccIddNddSid != null) {
                String strIdd = sMccIddNddSid.mIdd;
                if (DBG) {
                    Rlog.d(LOG_TAG,
                            "[removeIddNddAddPlusCodeForSms] find match the cc, Idd = " + strIdd);
                }
                if (number.startsWith(strIdd) && number.length() > strIdd.length()) {
                    number = number.substring(strIdd.length(), number.length());
                    if (DBG) {
                        Rlog.d(LOG_TAG, "[removeIddNddAddPlusCodeForSms] sub num = "
                                + Rlog.pii(LOG_TAG, number));
                    }
                    formatNumber = INTERNATIONAL_PREFIX_SYMBOL + number;
                }
            }
        }
        if (DBG) {
            Rlog.d(LOG_TAG, "[removeIddNddAddPlusCodeForSms] number after format = "
                    + Rlog.pii(LOG_TAG, formatNumber));
        }
        return formatNumber;
    }


    /**
     * Check mcc by sid ltm off.
     * @param mccMnc the MCCMNC
     * @return the MCCMNC
     */
    public static String checkMccBySidLtmOff(String mccMnc) {
        Rlog.d(LOG_TAG, "[checkMccBySidLtmOff] mccMnc = " + mccMnc);
        int cdmaSubId = getCdmaSubId();
        String strSid = "";
        try {
            ServiceState ss = TelephonyManager.getDefault()
                    .getServiceStateForSubscriber(cdmaSubId);
            if (ss != null) {
                strSid = Integer.toString(ss.getCdmaSystemId());
            }
        } catch (NullPointerException ex) {
            Rlog.d(LOG_TAG, "checkMccBySidLtmOff, NullPointerException:" + ex);
        }
        String strLtmOff = TelephonyManager.getTelephonyProperty(
                SubscriptionManager.getPhoneId(cdmaSubId),
                IPlusCodeUtils.PROPERTY_TIME_LTMOFFSET, "");
        Rlog.d(LOG_TAG, "[checkMccBySidLtmOff] Sid = " + Rlog.pii(LOG_TAG, strSid)
                + ", Ltm_off = " + strLtmOff);

        String strMcc = PlusCodeHpcdTable.getMccFromConflictTableBySidLtmOff(strSid, strLtmOff);
        String tempMcc;
        String strMccMnc;

        Rlog.d(LOG_TAG, "[checkMccBySidLtmOff] MccFromConflictTable = " + strMcc);

        if (strMcc != null) {
            tempMcc = strMcc;
        } else {
            strMcc = PlusCodeHpcdTable.getMccFromMINSTableBySid(strSid);
            Rlog.d(LOG_TAG, "[checkMccBySidLtmOff] MccFromMINSTable = " + strMcc);
            if (strMcc != null) {
                tempMcc = strMcc;
            } else {
                tempMcc = mccMnc;
            }
        }

        Rlog.d(LOG_TAG, "[checkMccBySidLtmOff] tempMcc = " + tempMcc);

        if (tempMcc.startsWith("310") || tempMcc.startsWith("311") || tempMcc.startsWith("312")) {
            strMccMnc = PlusCodeHpcdTable.getMccMncFromSidMccMncListBySid(strSid);
            Rlog.d(LOG_TAG, "[checkMccBySidLtmOff] MccMnc = " + strMccMnc);
            if (strMccMnc != null) {
                tempMcc = strMccMnc;
            }
        }

        return tempMcc;
    }
}
