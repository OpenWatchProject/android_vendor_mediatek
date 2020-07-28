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

package com.mediatek.dialer.search;

import android.text.TextUtils;
import android.util.Log;

import com.android.dialer.common.LogUtil;
import com.android.dialer.smartdial.map.SmartDialMap;
import com.google.common.collect.Lists;
import com.mediatek.dialer.search.SmartDialPadMap;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import android.content.Context;
import com.android.dialer.smartdial.map.CompositeSmartDialMap;

/**
 * Smart Dial utility class to find prefixes of contacts. It contains both methods to find supported
 * prefix combinations for contact names, and also methods to find supported prefix combinations for
 * contacts' phone numbers. Each contact name is separated into several tokens, such as first name,
 * middle name, family name etc. Each phone number is also separated into country code, NANP area
 * code, and local number if such separation is possible.
 */
public class SmartDialPrefixEx {
    private static final String TAG = SmartDialPrefixEx.class.getSimpleName();
    private static final boolean DEBUG = false;

    /** Dialpad mapping. */
    private static final SmartDialMap mMap = new SmartDialPadMap();

    // The max length for generate name prefix.
    private static final int MAX_NAME_LENGTH = 4;

    public static final String PREFIX_NA = "NA";
    /**
     * Parses a contact's name into a list of separated tokens.
     *
     * @param contactName Contact's name stored in string.
     * @return A list of name tokens, for example separated first names, last name, etc.
     */
    public static ArrayList<String> parseToIndexTokens(Context context, String contactName) {
        if (DEBUG) {
            Log.i(TAG, "parseToIndexTokens, contactName=" + contactName);
        }
        final ArrayList<String> result = Lists.newArrayList();
        // Normally the contact name can not be empty.
        if (TextUtils.isEmpty(contactName)) {
            if (DEBUG) {
                Log.i(TAG, "parseToIndexTokens, contactName empty");
            }
            return result;
        }
        final int length = contactName.length();
        final StringBuilder currentIndexToken = new StringBuilder();
        String word;
        byte bc;
        char c;
        int len;
        for (int i = 0; i < length; i = i + len + 1) {
            len = (int) contactName.charAt(i);
            word = contactName.substring(i + 1, i + len + 1);
            for (int j = 0; j < word.length(); j++) {
                c = CompositeSmartDialMap.normalizeCharacter(context,word.charAt(j));
                //c = mMap.normalizeCharacter(word.charAt(j));
                bc = CompositeSmartDialMap.getDialpadIndex(context,c);
                //bc = mMap.getDialpadIndex(c);
                if (bc == -1) {
                    currentIndexToken.append(c);
                } else {
                    currentIndexToken.append(bc);
                }
            }
            if (!TextUtils.isEmpty(currentIndexToken.toString())) {
                result.add(currentIndexToken.toString());
            } else {
                Log.v(TAG, "parseToIndexTokens,something maybe wrong, contactName="
                        + LogUtil.sanitizePii(contactName));
            }
            currentIndexToken.delete(0, currentIndexToken.length());
        }
        return result;
    }

    public static SmartDialMap getMap() {
        return mMap;
    }

    public static ArrayList<String> generateNamePrefixes(Context context, String name) {
        long start = System.currentTimeMillis();
        final ArrayList<String> result = Lists.newArrayList();

        /** Parses the name into a list of tokens. */
        final ArrayList<String> indexTokens = parseToIndexTokens(context,name);
        if (DEBUG) {
            Log.i(TAG, "parse tokens," + indexTokens.toString());
        }
        if (null != indexTokens && indexTokens.size() > MAX_NAME_LENGTH) {
            if (DEBUG) {
                Log.w(TAG, "name is too long:" + name);
            }
            result.add(PREFIX_NA);
            return result;
        }

        if (indexTokens.size() > 0) {
            List<String> indexes = Lists.newArrayList();
            List<String> fResult = Lists.newArrayList();
            List<String> mixedResult = Lists.newArrayList();
            for (int i = 0; i < indexTokens.size(); i++) {
                indexes.add(String.valueOf(i));
            }
            /**
             * Generate all the order sequence of the name words by the word's position(0,1,2,..)
             * in name string. And then take out the initial and full-spell string of the word
             * separately, to compose the prefix.
             */
            List<String> indexesList = generateDisorderIndex(indexes);
            for (String comb : indexesList) {
                int length = comb.length();
                List<String> partialList = Lists.newArrayList();
                List<String> removeList = Lists.newArrayList();
                int currentPos;
                int tempSize;
                String fullspell = "";
                /** [F] */
                for (int i = length - 1; i >= 0; i--) {
                    currentPos = comb.charAt(i) - '0';
                    fullspell = indexTokens.get(currentPos) + fullspell;
                }
                int lastPos = comb.charAt(length - 1) - '0';
                partialList.add(indexTokens.get(lastPos));
                for (int i = length - 2; i >= 0; i--) {
                    currentPos = comb.charAt(i) - '0';
                    // handle exception case.@{
                    if (indexTokens.get(currentPos).length() <= 0) {
                        // if this word is empty string, just skip it.
                        Log.w(TAG, "something maybe wrong, name is " + name +
                                ", current word is " + indexTokens.get(currentPos));
                        continue;
                    }
                    // @}
                    /** Mixed match prefix: [I+F], [F+I]. */
                    tempSize = partialList.size();
                    for (int j = 0; j < tempSize; j++) {
                        String partial = partialList.get(j);
                        partialList.add(indexTokens.get(currentPos).substring(0, 1) + partial);// I
                        partialList.add(indexTokens.get(currentPos) + partial);// F
                        removeList.add(partial);
                    }
                    partialList.removeAll(removeList);
                }
                if (!fResult.contains(fullspell)) {
                    fResult.add(fullspell);
                }
                partialList.remove(fullspell);
                mixedResult.addAll(partialList);
            }
            result.addAll(fResult);
            for (String mixed : mixedResult) {
                if (!result.contains(mixed)) {
                    result.add(mixed);
                }
            }
        }

        StringBuilder prefixes = new StringBuilder();
        for (String prefix : result) {
            prefixes.append(" ");
            prefixes.append(prefix);
        }
        result.clear();
        result.add(prefixes.toString());

        if (DEBUG) {
            Log.d(TAG, String.format("GenNamePrefix spend %dms to handle [%s]. result.size[%d]",
                    (System.currentTimeMillis() - start), name, result.size()));
        }
        return result;
    }

    /**
     * Generate the disorder Prefixes for tokens
     * @param indexes if the name is "li xiao long", the tokens is [li,xiao,long]
     * @return
     */
    private static List<String> generateDisorderIndex(List<String> indexes) {
        if (null == indexes || indexes.size() == 0) {
            return new ArrayList<String>();
        }
        List<String> result = new ArrayList<String>();
        return getFullPermutation(result, indexes, "");
    }

    private static List<String> getFullPermutation(List<String> result, List<String> indexes,
            String prefix) {
        if (indexes.isEmpty()) {
            result.add(prefix);
        }
        for (int i = 0; i < indexes.size(); i++) {
            List subcandidate = new LinkedList(indexes);
            getFullPermutation(result, subcandidate, prefix + subcandidate.remove(i));
        }

        return result;
    }
}
