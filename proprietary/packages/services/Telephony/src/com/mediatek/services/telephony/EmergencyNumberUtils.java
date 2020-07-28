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
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

package com.mediatek.services.telephony;

import android.os.Build;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.text.TextUtils;

import com.android.internal.telephony.MccTable;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.emergency.EmergencyNumberTracker;
import com.android.services.telephony.Log;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;

public class EmergencyNumberUtils {
    private static final String TAG = "ECCNumUtils";

    private String mNumber;
    private boolean mIsGsmOnlyNumber;
    private boolean mIsGsmPreferredNumber;
    private boolean mIsCdmaPreferredNumber;

    private static final String ECC_LIST_PREFERENCE_PATH
            = "/system/vendor/etc/ecc_list_preference.xml";
    private static final String OPERATOR_ATTR = "Operator";
    private static final String ECC_LIST_ATTR  = "EccList";
    private static final String ECC_GSM_ONLY_TAG       = "GsmOnly";
    private static final String ECC_GSM_PREFERRED_TAG  = "GsmPref";
    private static final String ECC_CDMA_PREFERRED_TAG = "CdmaPref";
    private static final boolean MTK_C2K_SUPPORT
            = "1".equals(SystemProperties.get("ro.vendor.mtk_c2k_support"));

    private static HashMap<String, String> sGsmOnlyEccMap       = new HashMap<String, String>();
    private static HashMap<String, String> sGsmPreferredEccMap  = new HashMap<String, String>();
    private static HashMap<String, String> sCdmaPreferredEccMap = new HashMap<String, String>();
    private static String sOp;

    static {
        parseEccListPreference();
        sOp = SystemProperties.get("persist.vendor.operator.optr", "OM");
    }

    public EmergencyNumberUtils(String number) {
        mNumber = number;
        Log.d(TAG, "Number:" + Log.pii(number) + ", operator:" + sOp);
        mIsGsmOnlyNumber = isGsmOnlyNumber(mNumber);
        mIsGsmPreferredNumber = isGsmPreferredNumber(mNumber);
        mIsCdmaPreferredNumber = isCdmaPreferredNumber(mNumber);
    }

/*
<?xml version="1.0" encoding="utf-8"?>
<EccPrefTable>
    <!--
        The attribute definition for tag GsmOnly, GsmPref, CdmaPref:
        - Operator: OM or OPXX
        - EccList: the preferred ECC list
    -->
    <GsmOnly Operator="460" EccList="112,000,08,118" />
    <GsmOnly Operator="OP01" EccList="112" />
    <GsmPref Operator="460" EccList="911,999" />
    <GsmPref Operator="OP01" EccList="000,08,118,911,999" />
    <CdmaPref Operator="460" EccList="110,119,120,122" />
    <CdmaPref Operator="OP20" EccList="911" />
</EccPrefTable>
*/
    /**
     * Parse Ecc List Preference From XML File
     */
    private static void parseEccListPreference() {
        sGsmOnlyEccMap.clear();
        sGsmPreferredEccMap.clear();
        sCdmaPreferredEccMap.clear();

        try {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            XmlPullParser parser = factory.newPullParser();
            if (parser == null) {
                return;
            }
            FileReader fileReader = new FileReader(ECC_LIST_PREFERENCE_PATH);
            parser.setInput(fileReader);
            int eventType = parser.getEventType();

            while (eventType != XmlPullParser.END_DOCUMENT) {
                switch (eventType) {
                    case XmlPullParser.START_TAG:
                        String eccTag = parser.getName();
                        String op = null;
                        String eccList = null;
                        int attrNum = parser.getAttributeCount();
                        for (int i = 0; i < attrNum; ++i) {
                            String name = parser.getAttributeName(i);
                            String value = parser.getAttributeValue(i);
                            if (name.equals(OPERATOR_ATTR)) {
                                op = value;
                            } else if (name.equals(ECC_LIST_ATTR)) {
                                eccList = value;
                            }
                        }
                        if (op != null && eccList != null) {
                            if (eccTag.equals(ECC_GSM_ONLY_TAG)) {
                                sGsmOnlyEccMap.put(op, eccList);
                            } else if (eccTag.equals(ECC_GSM_PREFERRED_TAG)) {
                                sGsmPreferredEccMap.put(op, eccList);
                            } else if (eccTag.equals(ECC_CDMA_PREFERRED_TAG)) {
                                sCdmaPreferredEccMap.put(op, eccList);
                            }
                        }
                        break;
                    default:
                        break;
                }
                eventType = parser.next();
            }
            fileReader.close();
        } catch (FileNotFoundException e) {
            Log.d(TAG, "Ecc List Preference file not found");
            sGsmOnlyEccMap.put("460", "112,000,08,118");
            sGsmOnlyEccMap.put("OP01", "112");
            sGsmPreferredEccMap.put("460", "911,999");
            sGsmPreferredEccMap.put("OP01", "000,08,118,911,999");
            sCdmaPreferredEccMap.put("460", "110,119,120,122");
            sCdmaPreferredEccMap.put("OP20", "911");
        } catch (XmlPullParserException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static boolean isNumberMatched(String number, String[] eccList) {
        for (String eccNumber : eccList) {
            if (number.equals(eccNumber)) {
                return true;
            }
        }
        return false;
    }

    public static boolean isGsmOnlyNumber(String number) {
        String eccList = "";
        if (sGsmOnlyEccMap.containsKey(sOp)) {
            eccList = sGsmOnlyEccMap.get(sOp);
        } else {
            eccList = getEccListByRegion(sGsmOnlyEccMap);
        }
        boolean bMatched = TextUtils.isEmpty(eccList) ?
                false : isNumberMatched(number, eccList.split(","));
        Log.d(TAG, "isGsmOnlyNumber = " + bMatched + ", eccList = " + eccList);
        return bMatched;
    }

    public static boolean isGsmPreferredNumber(String number) {
        String eccList = "";
        if (sGsmPreferredEccMap.containsKey(sOp)) {
            eccList = sGsmPreferredEccMap.get(sOp);
        } else {
            eccList = getEccListByRegion(sGsmPreferredEccMap);
        }
        boolean bMatched = TextUtils.isEmpty(eccList) ?
                false : isNumberMatched(number, eccList.split(","));
        Log.d(TAG, "isGsmPreferredNumber = " + bMatched + ", eccList = " + eccList);
        return bMatched;
    }

    public static boolean isCdmaPreferredNumber(String number) {
        String eccList = "";
        if (sCdmaPreferredEccMap.containsKey(sOp)) {
            eccList = sCdmaPreferredEccMap.get(sOp);
        } else {
            eccList = getEccListByRegion(sCdmaPreferredEccMap);
        }
        boolean bMatched = TextUtils.isEmpty(eccList) ?
                false : isNumberMatched(number, eccList.split(","));
        Log.d(TAG, "isCdmaPreferredNumber = " + bMatched + ", eccList = " + eccList);
        return bMatched;
    }

    public boolean isGsmOnlyNumber() {
        return mIsGsmOnlyNumber;
    }

    public boolean isGsmPreferredNumber() {
        return mIsGsmPreferredNumber;
    }

    public boolean isCdmaPreferredNumber() {
        return mIsCdmaPreferredNumber;
    }

    private static String getEccListByRegion(HashMap<String, String> eccLists) {
        if (!MTK_C2K_SUPPORT) {
            return "";
        }
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            String plmn = TelephonyManager.getDefault().getNetworkOperatorForPhone(i);
            Log.d(TAG, "getEccListByRegion, getNetworkOperatorForPhone: " + plmn);
            if (TextUtils.isEmpty(plmn)) {
                plmn = MtkTelephonyManagerEx.getDefault().getLocatedPlmn(i);
                Log.d(TAG, "getEccListByRegion, getLocatedPlmn: " + plmn);
            }
            if (TextUtils.isEmpty(plmn)) {
                String country = TelephonyManager.getDefault().getNetworkCountryIsoForPhone(i);
                Log.d(TAG, "getEccListByRegion, getNetworkCountryIsoForPhone: " + country);

                if (TextUtils.isEmpty(country)) {
                    country = getCountryIsoFromEmergencyNumber(i);
                    Log.d(TAG, "getEccListByRegion, getCountryIsoFromEmergencyNumber: " + country);
                }

                for (String str : eccLists.keySet()) {
                    if (MccTable.countryCodeForMcc(str).equals(country)) {
                        plmn = str;
                        break;
                    }
                }
            }
            if (TextUtils.isEmpty(plmn)) {
                plmn = TelephonyManager.getDefault().getSimOperatorNumericForPhone(i);
                Log.d(TAG, "getEccListByRegion, getSimOperatorNumericForPhone: " + plmn);
            }

            String mcc = (plmn != null && plmn.length() >= 3) ? plmn.substring(0, 3) : "";
            if (eccLists.containsKey(mcc)) {
                return eccLists.get(mcc);
            }
        }
        return "";
    }

    static String getCountryIsoFromEmergencyNumber(int phoneId) {
        EmergencyNumberTracker tracker = PhoneFactory.getPhone(phoneId).getEmergencyNumberTracker();
        if (tracker != null) {
            for (EmergencyNumber eNumber : tracker.getEmergencyNumberList()) {
                if (eNumber != null && !TextUtils.isEmpty(eNumber.getCountryIso())) {
                    return eNumber.getCountryIso();
                }
            }
        }
        return null;
    }
}
