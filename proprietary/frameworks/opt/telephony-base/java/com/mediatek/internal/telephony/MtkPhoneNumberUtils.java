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

import com.android.i18n.phonenumbers.NumberParseException;
import com.android.i18n.phonenumbers.Phonemetadata.PhoneMetadata;
import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.android.i18n.phonenumbers.Phonenumber.PhoneNumber;
import com.android.i18n.phonenumbers.ShortNumberInfo;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.location.Country;
import android.location.CountryDetector;
import android.net.Uri;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Contacts;
import android.provider.ContactsContract;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.TtsSpan;
import android.util.SparseIntArray;

import static com.android.internal.telephony.TelephonyProperties.PROPERTY_OPERATOR_IDP_STRING;

import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.internal.telephony.cdma.pluscode.IPlusCodeUtils;
import com.mediatek.internal.telephony.cdma.pluscode.PlusCodeProcessor;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.HashMap;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

/**
 * Various utilities for dealing with MTK phone number strings.
 */
public class MtkPhoneNumberUtils {
    static final String LOG_TAG = "MtkPhoneNumberUtils";
    private static final boolean VDBG = false;
    private static final int MIN_MATCH = 7;
    private static final int MIN_MATCH_CTA = 11;

    private static IPlusCodeUtils sPlusCodeUtils = null;

    // AOSP adapter @{
    /*
     * Special characters
     *
     * (See "What is a phone number?" doc)
     * 'p' --- GSM pause character, same as comma
     * 'n' --- GSM wild character
     * 'w' --- GSM wait character
     */
    public static final char PAUSE = ',';
    public static final char WAIT = ';';
    public static final char WILD = 'N';

    /*
     * Calling Line Identification Restriction (CLIR)
     */
    private static final String CLIR_ON = "*31#";
    private static final String CLIR_OFF = "#31#";

    /*
     * TOA = TON + NPI
     * See TS 24.008 section 10.5.4.7 for details.
     * These are the only really useful TOA values
     */
    public static final int TOA_International = 0x91;
    public static final int TOA_Unknown = 0x81;

    /** The current locale is unknown, look for a country code or don't format */
    public static final int FORMAT_UNKNOWN = 0;
    /** NANP formatting */
    public static final int FORMAT_NANP = 1;
    /** Japanese formatting */
    public static final int FORMAT_JAPAN = 2;

    /** List of country codes for countries that use the NANP */
    private static final String[] NANP_COUNTRIES = new String[] {
        "US", // United States
        "CA", // Canada
        "AS", // American Samoa
        "AI", // Anguilla
        "AG", // Antigua and Barbuda
        "BS", // Bahamas
        "BB", // Barbados
        "BM", // Bermuda
        "VG", // British Virgin Islands
        "KY", // Cayman Islands
        "DM", // Dominica
        "DO", // Dominican Republic
        "GD", // Grenada
        "GU", // Guam
        "JM", // Jamaica
        "PR", // Puerto Rico
        "MS", // Montserrat
        "MP", // Northern Mariana Islands
        "KN", // Saint Kitts and Nevis
        "LC", // Saint Lucia
        "VC", // Saint Vincent and the Grenadines
        "TT", // Trinidad and Tobago
        "TC", // Turks and Caicos Islands
        "VI", // U.S. Virgin Islands
    };

    private static final char PLUS_SIGN_CHAR = '+';
    /**
     * @hide
     */
    private static final String PLUS_SIGN_STRING = "+";

    // Call Forwarding
    private static final String SC_CFU     = "21";
    private static final String SC_CFB     = "67";
    private static final String SC_CFNRy   = "61";
    private static final String SC_CFNR    = "62";
    private static final String SC_CF_All  = "002";
    private static final String SC_CF_All_Conditional = "004";

    private static final String[] CF_HEADERS = {
        "*72", "*720", //cfu enable and disable
        "*90", "*900", //cfb enable and disable
        "*92", "*920", //cfnr enable and disable
        "*68", "*680", //cfdf enable and disable
        "*730" //cfall disable
    };

    public static byte[] numberToCalledPartyBCD(String number) {
        return PhoneNumberUtils.numberToCalledPartyBCD(number);
    }

    public static String calledPartyBCDFragmentToString(byte[] bytes, int offset, int length) {
        return PhoneNumberUtils.calledPartyBCDFragmentToString(bytes, offset, length);
    }

    public static String calledPartyBCDToString(byte[] bytes, int offset, int length) {
        return PhoneNumberUtils.calledPartyBCDToString(bytes, offset, length);
    }

    public static String stripSeparators(String phoneNumber) {
        return PhoneNumberUtils.stripSeparators(phoneNumber);
    }

    public static String extractNetworkPortion(String phoneNumber) {
        return PhoneNumberUtils.extractNetworkPortion(phoneNumber);
    }

    public static String stringFromStringAndTOA(String s, int TOA) {
        return PhoneNumberUtils.stringFromStringAndTOA(s, TOA);
    }

    public static String convertPreDial(String phoneNumber) {
        return PhoneNumberUtils.convertPreDial(phoneNumber);
    }

    public static boolean isNonSeparator(String address) {
        for (int i = 0, count = address.length(); i < count; i++) {
            if (!PhoneNumberUtils.isNonSeparator(address.charAt(i))) {
                return false;
            }
        }
        return true;
    }

    public static int getFormatTypeFromCountryCode(String country) {
        // Check for the NANP countries
        int length = NANP_COUNTRIES.length;
        for (int i = 0; i < length; i++) {
            if (NANP_COUNTRIES[i].compareToIgnoreCase(country) == 0) {
                return FORMAT_NANP;
            }
        }
        if ("jp".compareToIgnoreCase(country) == 0) {
            return FORMAT_JAPAN;
        }
        return FORMAT_UNKNOWN;
    }

    private static int findDialableIndexFromPostDialStr(String postDialStr) {
        for (int index = 0;index < postDialStr.length();index++) {
             char c = postDialStr.charAt(index);
             if (PhoneNumberUtils.isReallyDialable(c)) {
                return index;
             }
        }
        return -1;
    }

    private static String appendPwCharBackToOrigDialStr(int dialableIndex,
            String origStr, String dialStr) {
        String retStr;

        // There is only 1 P/W character before the dialable characters
        if (dialableIndex == 1) {
            StringBuilder ret = new StringBuilder(origStr);
            ret = ret.append(dialStr.charAt(0));
            retStr = ret.toString();
        } else {
            // It means more than 1 P/W characters in the post dial string,
            // appends to retStr
            String nonDigitStr = dialStr.substring(0,dialableIndex);
            retStr = origStr.concat(nonDigitStr);
        }
        return retStr;
    }

    public static boolean isEmergencyNumber(String number) {
        return PhoneNumberUtils.isEmergencyNumber(number);
    }

    public static boolean isEmergencyNumber(int subId, String number) {
        return PhoneNumberUtils.isEmergencyNumber(subId, number);
    }
    // AOSP function adapter @}

    // Initialization
    static {
        initialize();
    }

    private static void initialize() {
        sPlusCodeUtils = PlusCodeProcessor.getPlusCodeUtils();
    }

    /**
     * This function checks if there is a plus sign (+) in the passed-in dialing number.
     * If there is, it processes the plus sign based on the default telephone
     * numbering plan of the system when the phone is activated and the current
     * telephone numbering plan of the system that the phone is camped on.
     * Currently, we only support the case that the default and current telephone
     * numbering plans are North American Numbering Plan(NANP).
     *
     * The passed-in dialStr should only contain the valid format as described below,
     * 1) the 1st character in the dialStr should be one of the really dialable
     *    characters listed below
     *    ISO-LATIN characters 0-9, *, # , +
     * 2) the dialStr should already strip out the separator characters,
     *    every character in the dialStr should be one of the non separator characters
     *    listed below
     *    ISO-LATIN characters 0-9, *, # , +, WILD, WAIT, PAUSE
     *
     * Otherwise, this function returns the dial string passed in
     *
     * @param dialStr the original dial string
     * @return the converted dial string if the current/default countries belong to NANP,
     * and if there is the "+" in the original dial string. Otherwise, the original dial
     * string returns.
     *
     * This API is for CDMA only
     *
     */
    public static String cdmaCheckAndProcessPlusCode(String dialStr) {
        /// M: @{
        String result = preProcessPlusCode(dialStr);
        if (result != null && !result.equals(dialStr)) {
            return result;
        }
        /// @}
        if (!TextUtils.isEmpty(dialStr)) {
            if (PhoneNumberUtils.isReallyDialable(dialStr.charAt(0)) &&
                isNonSeparator(dialStr)) {
                String currIso = TelephonyManager.getDefault().getNetworkCountryIso();
                String defaultIso = TelephonyManager.getDefault().getSimCountryIso();
                if (!TextUtils.isEmpty(currIso) && !TextUtils.isEmpty(defaultIso)) {
                    return PhoneNumberUtils.cdmaCheckAndProcessPlusCodeByNumberFormat(dialStr,
                            getFormatTypeFromCountryCode(currIso),
                            getFormatTypeFromCountryCode(defaultIso));
                }
            }
        }
        return dialStr;
    }

    /**
     * Process phone number for CDMA, converting plus code using the home network number format.
     * This is used for outgoing SMS messages.
     *
     * @param dialStr the original dial string
     * @return the converted dial string
     * @hide for internal use
     */
    public static String cdmaCheckAndProcessPlusCodeForSms(String dialStr) {
        /// M: @{
        String result = preProcessPlusCodeForSms(dialStr);
        if (result != null && !result.equals(dialStr)) {
            return result;
        }
        /// @}

        if (!TextUtils.isEmpty(dialStr)) {
            if (PhoneNumberUtils.isReallyDialable(dialStr.charAt(0)) && isNonSeparator(dialStr)) {
                String defaultIso = TelephonyManager.getDefault().getSimCountryIso();
                if (!TextUtils.isEmpty(defaultIso)) {
                    int format = getFormatTypeFromCountryCode(defaultIso);
                    return PhoneNumberUtils.cdmaCheckAndProcessPlusCodeByNumberFormat(
                            dialStr, format, format);
                }
            }
        }
        return dialStr;
    }
    // Modify AOSP END

    // MTK Added Start

    /**
     * Return the extracted phone number.
     *
     * @param phoneNumber Phone number string.
     * @return Return number whiched is extracted the CLIR part.
     */
    public static String extractCLIRPortion(String phoneNumber) {
        if (phoneNumber == null) {
            return null;
        }

        // ex. **61*<any international number>**<timer>#
        Pattern p = Pattern.compile(
                "^([*][#]|[*]{1,2}|[#]{1,2})([0-9]{2,3})([*])([+]?[0-9]+)(.*)(#)$");
        Matcher m = p.matcher(phoneNumber);
        if (m.matches()) {
            return m.group(4); // return <any international number>
        } else if (phoneNumber.startsWith("*31#") || phoneNumber.startsWith("#31#")) {
            vlog(phoneNumber + " Start with *31# or #31#, return " + phoneNumber.substring(4));
            return phoneNumber.substring(4);
        } else if (phoneNumber.indexOf(PLUS_SIGN_STRING) != -1 &&
                   phoneNumber.indexOf(PLUS_SIGN_STRING) ==
                   phoneNumber.lastIndexOf(PLUS_SIGN_STRING)) {
            p = Pattern.compile("(^[#*])(.*)([#*])(.*)(#)$");
            m = p.matcher(phoneNumber);
            if (m.matches()) {
                if ("".equals(m.group(2))) {
                    // Started with two [#*] ends with #
                    // So no dialing number and we'll just return "" a +, this handles **21#+
                    vlog(phoneNumber + " matcher pattern1, return empty string.");
                    return "";
                } else {
                    String strDialNumber = m.group(4);
                    if (strDialNumber != null && strDialNumber.length() > 1
                            && strDialNumber.charAt(0) == PLUS_SIGN_CHAR) {
                        // Starts with [#*] and ends with #
                        // Assume group 4 is a dialing number such as *21*+1234554#
                        vlog(phoneNumber + " matcher pattern1, return " + strDialNumber);
                        return strDialNumber;
                    }
                }
            } else {
                p = Pattern.compile("(^[#*])(.*)([#*])(.*)");
                m = p.matcher(phoneNumber);
                if (m.matches()) {
                    String strDialNumber = m.group(4);
                    if (strDialNumber != null && strDialNumber.length() > 1
                            && strDialNumber.charAt(0) == PLUS_SIGN_CHAR) {
                        // Starts with [#*] and only one other [#*]
                        // Assume the data after last [#*] is dialing number
                        // (i.e. group 4) such as *31#+11234567890.
                        // This also includes the odd ball *21#+
                        vlog(phoneNumber + " matcher pattern2, return " + strDialNumber);
                        return strDialNumber;
                    }
                }
            }
        }

        return phoneNumber;
    }


    /**
     * Prepend plus to the number.
     * @param number The original number.
     * @return The number with plus sign.
     * @hide
     */
    public static String prependPlusToNumber(String number) {
        // This is an "international number" and should have
        // a plus prepended to the dialing number. But there
        // can also be Gsm MMI codes as defined in TS 22.030 6.5.2
        // so we need to handle those also.
        //
        // http://web.telia.com/~u47904776/gsmkode.htm is a
        // has a nice list of some of these GSM codes.
        //
        // Examples are:
        //   **21*+886988171479#
        //   **21*8311234567#
        //   **21*+34606445635**20#
        //   **21*34606445635**20#
        //   *21#
        //   #21#
        //   *#21#
        //   *31#+11234567890
        //   #31#+18311234567
        //   #31#8311234567
        //   18311234567
        //   +18311234567#
        //   +18311234567
        // Odd ball cases that some phones handled
        // where there is no dialing number so they
        // append the "+"
        //   *21#+
        //   **21#+
        StringBuilder ret;
        String retString = number.toString();
        Pattern p = Pattern.compile(
                "^([*][#]|[*]{1,2}|[#]{1,2})([0-9]{2,3})([*])([0-9]+)(.*)(#)$");
        Matcher m = p.matcher(retString);
        if (m.matches()) {
            ret = new StringBuilder();
            ret.append(m.group(1));
            ret.append(m.group(2));
            ret.append(m.group(3));
            ret.append("+");
            ret.append(m.group(4));
            ret.append(m.group(5));
            ret.append(m.group(6));
        } else {
            p = Pattern.compile("(^[#*])(.*)([#*])(.*)(#)$");
            m = p.matcher(retString);
            if (m.matches()) {
                if ("".equals(m.group(2))) {
                    // Started with two [#*] ends with #
                    // So no dialing number and we'll just
                    // append a +, this handles **21#+
                    ret = new StringBuilder();
                    ret.append(m.group(1));
                    ret.append(m.group(3));
                    ret.append(m.group(4));
                    ret.append(m.group(5));
                    ret.append("+");
                } else {
                    // Starts with [#*] and ends with #
                    // Assume group 4 is a dialing number
                    // such as *21*+1234554#
                    ret = new StringBuilder();
                    ret.append(m.group(1));
                    ret.append(m.group(2));
                    ret.append(m.group(3));
                    ret.append("+");
                    ret.append(m.group(4));
                    ret.append(m.group(5));
                }
            } else {
                p = Pattern.compile("(^[#*])(.*)([#*])(.*)");
                m = p.matcher(retString);
                if (m.matches()) {
                    // Starts with [#*] and only one other [#*]
                    // Assume the data after last [#*] is dialing
                    // number (i.e. group 4) such as *31#+11234567890.
                    // This also includes the odd ball *21#+
                    ret = new StringBuilder();
                    ret.append(m.group(1));
                    ret.append(m.group(2));
                    ret.append(m.group(3));
                    ret.append("+");
                    ret.append(m.group(4));
                } else {
                    // Does NOT start with [#*] just prepend '+'
                    ret = new StringBuilder();
                    ret.append('+');
                    ret.append(retString);
                }
            }
        }
        return ret.toString();
    }

    private static String preProcessPlusCode(String dialStr) {
        if (!TextUtils.isEmpty(dialStr)) {
            if (PhoneNumberUtils.isReallyDialable(dialStr.charAt(0)) && isNonSeparator(dialStr)) {
                String currIso = TelephonyManager.getDefault().getNetworkCountryIso();
                String defaultIso = TelephonyManager.getDefault().getSimCountryIso();
                boolean needToFormat = true;
                if (!TextUtils.isEmpty(currIso) && !TextUtils.isEmpty(defaultIso)) {
                    int currFormat = getFormatTypeFromCountryCode(currIso);
                    int defaultFormat = getFormatTypeFromCountryCode(defaultIso);
                    needToFormat = !((currFormat == defaultFormat) && (currFormat == FORMAT_NANP));
                }
                if (needToFormat) {
                    vlog("preProcessPlusCode, before format number:" + dialStr);
                    String retStr = dialStr;
                    // Checks if the plus sign character is in the passed-in dial string
                    if (dialStr.lastIndexOf(PLUS_SIGN_STRING) != -1) {
                        String postDialStr = null;
                        String tempDialStr = dialStr;

                        // Sets the retStr to null since the conversion will be performed below.
                        retStr = null;
                        do {
                            String networkDialStr;
                            networkDialStr = PhoneNumberUtils.extractNetworkPortionAlt(
                                    tempDialStr);
                            if (networkDialStr != null &&
                                    networkDialStr.charAt(0) == PLUS_SIGN_CHAR &&
                                    networkDialStr.length() > 1) {
                                if (sPlusCodeUtils.canFormatPlusToIddNdd()) {
                                    networkDialStr = sPlusCodeUtils.replacePlusCodeWithIddNdd(
                                            networkDialStr);
                                } else {
                                    dlog("preProcessPlusCode, can't format plus code.");
                                    return dialStr;
                                }
                            }

                            vlog("preProcessPlusCode, networkDialStr:" + networkDialStr);
                            // Concatenates the string that is converted from network portion
                            if (!TextUtils.isEmpty(networkDialStr)) {
                                if (retStr == null) {
                                    retStr = networkDialStr;
                                } else {
                                    retStr = retStr.concat(networkDialStr);
                                }
                            } else {
                                Rlog.e(LOG_TAG, "preProcessPlusCode, null newDialStr:"
                                        + networkDialStr);
                                return dialStr;
                            }
                            postDialStr = PhoneNumberUtils.extractPostDialPortion(tempDialStr);
                            if (!TextUtils.isEmpty(postDialStr)) {
                                int dialableIndex = findDialableIndexFromPostDialStr(postDialStr);

                                // dialableIndex should always be greater than 0
                                if (dialableIndex >= 1) {
                                    retStr = appendPwCharBackToOrigDialStr(dialableIndex,
                                             retStr, postDialStr);
                                    // Skips the P/W character, extracts the dialable portion
                                    tempDialStr = postDialStr.substring(dialableIndex);
                                } else {
                                    if (dialableIndex < 0) {
                                        postDialStr = "";
                                    }
                                    Rlog.e(LOG_TAG, "preProcessPlusCode, wrong postDialStr:"
                                            + postDialStr);
                                }
                            }
                            vlog("preProcessPlusCode, postDialStr:" + postDialStr
                                    + ", tempDialStr:" + tempDialStr);
                        } while (!TextUtils.isEmpty(postDialStr)
                                && !TextUtils.isEmpty(tempDialStr));
                    }
                    dialStr = retStr;
                    vlog("preProcessPlusCode, after format number:" + dialStr);
                } else {
                    dlog("preProcessPlusCode, no need format, currIso:" + currIso
                            + ", defaultIso:" + defaultIso);
                }
            }
        }
        return dialStr;
    }

    private static String preProcessPlusCodeForSms(String dialStr) {
        dlog("preProcessPlusCodeForSms ENTER.");
        if (!TextUtils.isEmpty(dialStr) && dialStr.startsWith("+")) {
            if (PhoneNumberUtils.isReallyDialable(dialStr.charAt(0)) && isNonSeparator(dialStr)) {
                String defaultIso = TelephonyManager.getDefault().getSimCountryIso();
                if (getFormatTypeFromCountryCode(defaultIso) != FORMAT_NANP) {
                    if (sPlusCodeUtils.canFormatPlusCodeForSms()) {
                        String retAddr = sPlusCodeUtils.replacePlusCodeForSms(dialStr);
                        if (TextUtils.isEmpty(retAddr)) {
                            dlog("preProcessPlusCodeForSms," +
                                    " can't handle the plus code by PlusCodeUtils");
                        } else {
                            vlog("preProcessPlusCodeForSms, "
                                    + "new dialStr = " + retAddr);
                            dialStr = retAddr;
                        }
                    }
                }
            }
        }
        return dialStr;
    }

    /**
     * Helper function for isEmergencyNumber(String, String) and
     * isPotentialEmergencyNumber(String, String).
     *
     * Mediatek revise for retry ECC with Phone type (GSM or CDMA)
     *
     * @param subId the subscription id of the SIM.
     * @param number the number to look up.
     * @param defaultCountryIso the specific country which the number should be checked against
     * @param useExactMatch if true, consider a number to be an emergency
     *           number only if it *exactly* matches a number listed in
     *           the RIL / SIM.  If false, a number is considered to be an
     *           emergency number if it simply starts with the same digits
     *           as any of the emergency numbers listed in the RIL / SIM.
     *
     * @return true if the number is an emergency number for the specified country.
     * @hide
     */
    public static boolean isEmergencyNumberExt(int subId, String number,
            String defaultCountryIso, boolean useExactMatch) {
        log("[isEmergencyNumberExt] Warning: Deprecated!");
        return PhoneNumberUtils.isEmergencyNumber(subId, number);
    }

    /**
     * Returns phone number minimum match length.
     */
    public static int getMinMatch() {
        // getMinMatch may called before initialize so init before using
        boolean isCtaSupport = "1".equals(SystemProperties.get("ro.vendor.mtk_cta_support"));
        boolean isOP09Support = "OP09".equals(SystemProperties.get("persist.vendor.operator.optr"))
                && ("SEGDEFAULT".equals(SystemProperties.get("persist.vendor.operator.seg"))
                || "SEGC".equals(SystemProperties.get("persist.vendor.operator.seg")));
        if (isOP09Support || isCtaSupport) {
            vlog("[DBG] getMinMatch return 11 for CTA/OP09");
            return MIN_MATCH_CTA;
        } else {
            vlog("[DBG] getMinMatch return 7");
            return MIN_MATCH;
        }
    }

    public static boolean isCallForwardCode(String number) {
        // Maybe need check phone type or SIM type by subId in the futher.
        if (TextUtils.isEmpty(number)) {
            return false;
        }
        String dialPart = number;
        if (!PhoneNumberUtils.isUriNumber(number)) {
            dialPart = PhoneNumberUtils.extractNetworkPortionAlt(PhoneNumberUtils.
                    stripSeparators(number));
        }
        return isGsmCallForwardCode(number) || isCdmaCallForwardCode(number);
    }

    private static boolean isGsmCallForwardCode(String number) {
        if (TextUtils.isEmpty(number)) {
            return false;
        }
        Pattern p = Pattern.compile(
                "((\\*|#|\\*#|\\*\\*|##)(\\d{2,3})(\\*([^*#]*)(\\*([^*#]*)(\\*([^*#]*)(\\*([^*#]*))?)?)?)?#)(.*)");
        Matcher m = p.matcher(number);
        if (m.matches()) {
            String sc = m.group(3);
            if (sc != null && sc.length() == 0) {
                sc = null;
            }
            boolean isCf = sc != null &&
                    (sc.equals(SC_CFU)
                    || sc.equals(SC_CFB) || sc.equals(SC_CFNRy)
                    || sc.equals(SC_CFNR) || sc.equals(SC_CF_All)
                    || sc.equals(SC_CF_All_Conditional));
            log("[isGsmCallForwardCode] sc = " + sc + "isCf = " + isCf);
            return isCf;
        }
        return false;
    }

    private static boolean isCdmaCallForwardCode(String number) {
        if (TextUtils.isEmpty(number)) {
            return false;
        }
        for (String header : CF_HEADERS) {
            if (number.indexOf(header) == 0) {
                log("[isCdmaCallForwardCode] This is cdma call forward code");
                return true;
            }
        }
        return false;
    }

    // print log in all load (eng/user/userdebug)
    private static void log(String msg) {
        Rlog.i(LOG_TAG, msg);
    }

    // print log only in eng load
    private static void dlog(String msg) {
        Rlog.d(LOG_TAG, msg);
    }

    // need to turn on VDBG manually before print log
    private static void vlog(String msg) {
        if (VDBG) {
            dlog(msg);
        }
    }
}
