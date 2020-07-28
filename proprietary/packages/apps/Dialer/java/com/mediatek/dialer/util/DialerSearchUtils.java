/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.dialer.util;

import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashSet;

import com.android.dialer.common.LogUtil;

/**
 * M: [MTK Dialer Search] utilities class
 *
 */
public class DialerSearchUtils {

    private static final String TAG = "DialerSearchUtils";

    private static final HashSet<Character> HYPHEN_CHARACTERS = new HashSet<Character>();
    static {
        HYPHEN_CHARACTERS.add(' ');
        HYPHEN_CHARACTERS.add('-');
        HYPHEN_CHARACTERS.add('(');
        HYPHEN_CHARACTERS.add(')');
    }

    public static ArrayList<Integer> adjustHighlitePositionForHyphen(String number,
            String numberMatchedOffsets, String originNumber) {
        ArrayList<Integer> res = new ArrayList<Integer>();
        try {
            int highliteBegin = (int) numberMatchedOffsets.charAt(0);
            int highliteEnd = (int) numberMatchedOffsets.charAt(1);
            int originNumberBegin = 0;
            String targetTemp = "";
            for (int i = 0; i < number.length(); i++) {
                char c = number.charAt(i);
                if (HYPHEN_CHARACTERS.contains(c)) {
                    continue;
                }
                targetTemp += c;
            }
            originNumberBegin = originNumber.indexOf(targetTemp);

            if (highliteBegin > highliteEnd) {
                return res;
            }

            if ((originNumberBegin >= highliteEnd) && highliteEnd >= 1) {
                highliteEnd = 0;
            }

            if (highliteEnd > originNumberBegin) {
                highliteEnd = highliteEnd - originNumberBegin;
            }

            if (highliteBegin >= originNumberBegin) {
                highliteBegin = highliteBegin - originNumberBegin;
            }

            for (int i = 0; i <= highliteBegin; i++) {
                char c = number.charAt(i);
                if (HYPHEN_CHARACTERS.contains(c)) {
                    highliteBegin++;
                    highliteEnd++;
                }
            }

            for (int i = highliteBegin + 1; (i <= highliteEnd && i < number.length()); i++) {
                char c = number.charAt(i);
                if (HYPHEN_CHARACTERS.contains(c)) {
                    highliteEnd++;
                }
            }

            if (highliteEnd >= number.length()) {
                highliteEnd = number.length() - 1;
            }
            res.add(highliteBegin);
            res.add(highliteEnd);
        } catch (Exception e) {
            Log.i(TAG, "number= " + LogUtil.sanitizePhoneNumber(number) + " numberMatchedOffsets= "
                    + LogUtil.sanitizePhoneNumber(numberMatchedOffsets) + " originNumber= "
                    + LogUtil.sanitizePhoneNumber(originNumber));
            e.printStackTrace();
            return null;
        }
        return res;
    }

    /**
     * In telephony, ' ' and '-' always used to Separator a phone number, such as "131-2345-6789"
     * or "136 1234 5678", so when use these case to dial or query need strip ' ' and '-'
     * @param string the origin input
     * @return the strip result
     * */
    public static String stripTeleSeparators(String string) {
        if (TextUtils.isEmpty(string) || string.contains("@")) {
            return string;
        }

        StringBuilder sb = new StringBuilder();
        int len = string.length();

        for (int i = 0; i < len; i++) {
            char c = string.charAt(i);
            if (c != ' ' && c != '-' && c != '(' && c != ')' && c != '.' && c != '/') {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    /**
     * M: strip special char in the number(phone type number).
     * @param number number
     * @return number
     */
    public static String normalizeNumber(String number) {
        if (number == null) {
            return null;
        }
        int len = number.length();
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < len; i++) {
            char c = number.charAt(i);

            /// M:fix CR ALPS00758625, need to consider 'p' or 'w', not to filter these character.
            /// These character will be valid as phone number.
            if (PhoneNumberUtils.isNonSeparator(c)
                    || c == 'p' || c == 'w' || c == 'P' || c == 'W') {
                sb.append(c);
            } else if (c == ' ' || c == '-' || c == '(' || c == ')') {
                // strip blank and hyphen
            } else {
                /*
                 * Bug fix by Mediatek begin CR ID: ALPS00293790 Description:
                 * Original Code: break;
                 */
                continue;
            }
        }
        return sb.toString();
    }
}
