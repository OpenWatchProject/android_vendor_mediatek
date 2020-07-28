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

public class DialerSearchNameMatcher {

    private static final String TAG = "DialerSearchNameMatcher";
    private static final boolean DEBUG = false;

    /**
     * The match result offset is consist of: matchType + (char) 0 + (char) 0,
     * These two '(char) 0' is useless, its just a placeholder, so currently mark it as "00".
     */
    private static final String MATCH_TYPE_SUFFIX = "00";

    // Match state.
    public static final int DS_NOT_MATCH = 0;

    // Match type.
    public static final int DS_INITIALS_MATCH = 17;
    public static final int DS_FULLSPELL_MATCH = 15;
    public static final int DS_NUMBER_MATCH = 13;
    public static final int DS_MIXED_MATCH = 11;

    /**
     * Add for CT NEW FEATURE to show items follow order 09-004
     * DS_FULL_INITIALS_MATCH, DS_FULL_FULLSPEL_MATCH
     *
     * Fully [F] match, such as "Xiao" full [F] match "Xiao",
     * but "Xi" partial [F] match "Xiao".
     */
    public static final int DS_FULL_FULLSPEL_MATCH = 16;
    public static final int DS_FULL_INITIALS_MATCH = 20;

    /**
     * This order is the default match type order and it contains all the valid match types.
     * If you would like to using your own order, please make sure to use the match types in
     * this default match type order, others will not be recognized.
     */
    private static final int[] DEFAULT_MATCH_TYPE_ORDER = {
        DS_FULL_INITIALS_MATCH, DS_INITIALS_MATCH,
        DS_FULL_FULLSPEL_MATCH, DS_FULLSPELL_MATCH,
        DS_NUMBER_MATCH,
        DS_MIXED_MATCH
    };

    private class MatchItem {
        String name;
        String nameOffset;
        int matchState = DS_NOT_MATCH;
        int matchLength;

    }

    private int[] mMatchTypeOrder = DEFAULT_MATCH_TYPE_ORDER;

    private String mQuery;
    private int mQueryLength;

    private String mPhoneNumber;

    // The normalized contact name.
    private String mName;
    // The contact name's offset.
    private String mNameOffset;
    private MatchItem[] mMatchItems;

    private boolean mNeedDoNameMatch = false;

    // Match result
    // the matched offset for full name.
    private String mMatchNameOffset;
    // the matched offset for full number.
    private String mMatchNumberOffset;
    private int mMatchType = DS_NOT_MATCH;

    public int getMatchType() {
        return mMatchType;
    }

    public String getNameMatchOffset() {
        return mMatchNameOffset;
    }

    public String getNumberMatchOffset() {
        return mMatchNumberOffset;
    }

    public int[] getMatchTypeOrder() {
        return mMatchTypeOrder;
    }

    /**
     * The match type order which will present the match result in this order. If not specified,
     * then default match type order will be used.
     *
     * @param matchTypeOrder
     *            which MUST be the type in DialerSearchNameMatcher.DEFAULT_MATCH_TYPE_ORDER
     */
    public void setMatchTypeOrder(int[] matchTypeOrder) {
        if (null != matchTypeOrder && matchTypeOrder.length > 0) {
            mMatchTypeOrder = matchTypeOrder;
        }
    }

    /**
     * Do match for the phoneNumber.
     * @param query The user input query String
     * @param phoneNumber The phone number to match.
     * @return true if number matched.
     */
    public boolean doNumberMatch(String query, String phoneNumber) {
        return doNameAndNumberMatch(query, null, null, phoneNumber);
    }

    /**
     * Do match for the normalizedName.
     * @param query The user input query String
     * @param name The normalized name to match.
     * @param nameOffset The normalized name's offset.
     * @return true if name matched.
     */
    public boolean doNameMatch(String query, String name, String nameOffset) {
        return doNameAndNumberMatch(query, name, nameOffset, null);
    }

    /**
     * Do match for the normalizedName & phoneNumber.
     * @param query The user input query String
     * @param name The normalized name to match.
     * @param nameOffset The normalized name's offset.
     * @param phoneNumber The phone number to match.
     * @return true if name matched or number matched.
     */
    public boolean doNameAndNumberMatch(String query, String name, String nameOffset,
            String phoneNumber) {
        if (TextUtils.isEmpty(query)) {
            return false;
        }

        reset();
        initial(query, name, nameOffset, phoneNumber);

        boolean nameMatched = processNameMatch();
        boolean numberMatched = processNumberMatch();

        if (DEBUG) {
            log("nameMatched: " + nameMatched
                    + " ,numberMatched: " + numberMatched
                    + " ,query: " + query
                    + " ,name: " + name
                    + " ,nameOffset: " + nameOffset
                    + " ,phoneNumber: " + phoneNumber);
        }

        return nameMatched || numberMatched;
    }

    private boolean processNameMatch() {
        if (!mNeedDoNameMatch || getWordLength(mName) < mQueryLength) {
            return false;
        }

        // One char match.
        // If one char match failed, then match failed.
        if (1 == mQueryLength) {
            return isMatched(processOneCharMatch());
        }

        // Initial Match
        if (mMatchItems.length >= mQueryLength) {
            if (isMatched(processInitialMatch())) {
                return true;
            }
        }

        // Mix matched
        resetMatchItems();
        if (isMatched(processMatch())) {
            return true;
        }

        return false;
    }

    private boolean processNumberMatch() {
        if (null == mPhoneNumber) {
            return false;
        }

        /** the phone type number already be normalized when update the db. */
        int pos = mPhoneNumber.indexOf(mQuery);
        if (pos == -1) {
            return false;
        }
        int start = pos;
        int end = pos + mQuery.length() - 1;

        StringBuilder matchOffset = new StringBuilder();
        matchOffset.append((char) DS_NUMBER_MATCH).append(MATCH_TYPE_SUFFIX);
        matchOffset.append((char) start).append((char) end).append((char) (end - start));
        mMatchNumberOffset = matchOffset.toString();

        setMatchType(DS_NUMBER_MATCH);
        return true;
    }

    private String processOneCharMatch() {
        final int len = mName.length();
        char c = mQuery.charAt(0);
        for (int i = 0; i < len; i = i + 1 + mName.charAt(i)) {
            if (charEquals(c, mName.charAt(i + 1))) {
                char cc = mNameOffset.charAt(i + 1);
                StringBuilder result = new StringBuilder(6);
                result.append((char)
                        (1 == mMatchItems.length ? DS_FULL_INITIALS_MATCH : DS_INITIALS_MATCH))
                .append(MATCH_TYPE_SUFFIX)
                .append(cc).append(cc).append((char) 1);
                return result.toString();
            }
        }

        return null;
    }

    private String processInitialMatch() {
        boolean matched = false;
        int matchCount = 0;

        for (int matchIndex = 0; matchIndex < mQueryLength; matchIndex++) {
            matched = false;
            for (MatchItem item : mMatchItems) {
                if (item.matchState == DS_NOT_MATCH
                        && charEquals(mQuery.charAt(matchIndex), item.name.charAt(0))) {
                    item.matchState = DS_INITIALS_MATCH;
                    matchCount++;
                    matched = true;
                    break;
                }
            }

            if (!matched) {
                return null;
            }
        }

        StringBuilder sb = new StringBuilder();
        sb.append((char)
                (matchCount == mMatchItems.length ? DS_FULL_INITIALS_MATCH : DS_INITIALS_MATCH))
                .append(MATCH_TYPE_SUFFIX);
        for (MatchItem item : mMatchItems) {
            if (item.matchState == DS_INITIALS_MATCH) {
                sb.append(item.nameOffset.charAt(0));
                sb.append(item.nameOffset.charAt(0));
                sb.append((char) 1);
            }
        }

        return sb.toString();
    }

    private String processMatch() {
        int maxMatchLen = 0;
        int itemMatchLen = 0;
        int nameLength = 0;
        boolean hasChangedStatus = false;
        boolean hasFFMatchedItem = false;//Full FullSpell Matched item.

        for (int matchIndex = 0; matchIndex < mQueryLength; matchIndex += maxMatchLen) {
            maxMatchLen = 0;

            // Collect each item's match length
            for (MatchItem item : mMatchItems) {
                if (item.matchState != DS_NOT_MATCH) {
                    continue;
                }

                itemMatchLen = 0;
                nameLength = item.name.length();
                for (int i = 0, j = matchIndex; i < nameLength && j < mQueryLength; i++, j++) {
                    if (!charEquals(mQuery.charAt(j), item.name.charAt(i))) {
                        break;
                    }
                    itemMatchLen++;
                }

                if (0 != itemMatchLen) {
                    item.matchLength = itemMatchLen;
                    maxMatchLen = maxMatchLen > itemMatchLen ? maxMatchLen : itemMatchLen;
                }
            }

            // No one char matched, fail!!!
            if (0 == maxMatchLen) {
                return null;
            }

            hasChangedStatus = false;
            hasFFMatchedItem = false;
            // Collect if we have full fullSpell match item.
            for (MatchItem item : mMatchItems) {
                if (item.matchState == DS_NOT_MATCH && item.matchLength == maxMatchLen) {
                    if (item.matchLength == item.name.length()) {
                        hasFFMatchedItem = true;
                        break;
                    }
                }
            }

            /**
             * Change the status of the max match item;
             * The full fullSpell should be changed first, than others.
             * Also reset the not matched item's matchLength to 0.
             */
            for (MatchItem item : mMatchItems) {
                if (item.matchState == DS_NOT_MATCH) {
                    if (!hasChangedStatus && maxMatchLen ==item.matchLength
                            && (!hasFFMatchedItem/*First max match one*/
                            || item.name.length() == item.matchLength/*First max FF match one*/)) {
                        item.matchState = (item.matchLength == 1)
                                ? DS_INITIALS_MATCH : DS_FULLSPELL_MATCH;
                        hasChangedStatus = true;
                        continue;
                    }
                    item.matchLength = 0;
                }
            }
        }

        return handleMatchResult();
    }

    private String handleMatchResult() {
        int initialCount = 0;
        int fullWordCount = 0;
        int partialWordCount = 0;

        for (MatchItem item : mMatchItems) {
            if (item.matchState == DS_INITIALS_MATCH
                    || item.matchState == DS_FULL_INITIALS_MATCH) {
                initialCount++;
            } else if (item.matchState == DS_FULLSPELL_MATCH
                    || item.matchState == DS_FULL_FULLSPEL_MATCH) {
                if (item.matchLength == item.name.length()) {
                    fullWordCount++;
                } else {
                    partialWordCount++;
                }
            }
        }

        if (DEBUG) {
            log("I: " + initialCount + " ,F: " + fullWordCount + " ,P: " + partialWordCount);
        }

        /**
         * Valid match result combination:
         * 1. All Initial
         * 2. FullSpell
         * (Notice:
         *  Full FullSpell: match length == word length;
         *  FullSpell:      match length <  word length)
         *      (0~N) Full FullSpell + (0~1) Partial FullSpell
         * 3. Mix
         * (Notice: if have Partial FullSpell, then Partial FullSpell is the end of the user input
         * and only can have 1 Partial FullSpell)
         *      a. (0~N) Full FullSpell + (0~N) Initial
         *      b. (0~N) Initial + (0~1) Partial FullSpell
         *      c. Full FullSpell + Initial + Partial FullSpell
         */
        int matchType = DS_NOT_MATCH;
        /*
        // Initial, This case should never come in, for we had done initial match first.
        if (fullWordCount == 0 && partialWordCount == 0 && initialCount != 0) {
            matchType = DS_INITIALS_MATCH;
        // Full
        } else
        */
        if ((fullWordCount != 0 && partialWordCount <= 1)
                || (fullWordCount == 0 && partialWordCount == 1
                    && initialCount == 0)) {
            matchType = (((fullWordCount + partialWordCount)) == mMatchItems.length)
                    ? DS_FULL_FULLSPEL_MATCH : DS_FULLSPELL_MATCH;
        // Mix
        } else {
            /**
             *  TODO Cover too many case.
             *  1. If it has FullSpell match, then the FullSpell match
             *   could only be the last part of the user input.
             */
            if (partialWordCount > 1) {
                return null;
            } else if (partialWordCount == 1) {
                // find the FullSpell match part is the end of the user input.
                for (MatchItem item : mMatchItems) {
                    // A FullSpell matched item.
                    if (item.matchLength != item.name.length() && item.matchLength > 1) {
                        if (queryEndWith(mQuery, item.name.substring(0, item.matchLength))) {
                            break;
                        } else {
                            return null;
                        }
                    }
                }
            }
            matchType = DS_MIXED_MATCH;
        }

        StringBuilder sb = new StringBuilder();
        sb.append((char) matchType).append(MATCH_TYPE_SUFFIX);
        for (MatchItem item : mMatchItems) {
            if (item.matchState != DS_NOT_MATCH && item.matchLength > 0) {
                sb.append(item.nameOffset.charAt(0));
                sb.append(item.nameOffset.charAt(item.matchLength - 1));
                sb.append((char) item.matchLength);
            }
        }
        return sb.toString();
    }

    /**
     * Compare the character
     * @param input the input's char
     * @param name the name's char
     */
    private boolean charEquals(char input, char name) {
        /* Dialer search matching algorithm */
        return (((input < 'a' ? input : (input > 'z') ? input :
            Character.toUpperCase(input)) == name) ? true :
            (
            (input =='3' && (name>='D' && name<='F')) ||
            (input =='5' && (name>='J' && name<='L')) ||
            (input =='9' && (name>='W' && name<='Z')) ||
            (input =='8' && (name>='T' && name<='V')) ||
            (input =='6' && (name>='M' && name<='O')) ||
            (input =='2' && (name>='A' && name<='C')) ||
            (input =='4' && (name>='G' && name<='I')) ||
            (input =='7' && (name>='P' && name<='S'))
            )? true:false);
    }

    /**
     * Parses a contact's name into an array of separated tokens.
     *
     * @param name Contact's name stored in string.
     * @param nameOffset Contact's search offset.
     * @return
     */
    private MatchItem[] parseToSearchArray(String name, String nameOffset) {
        final int length = name.length();
        int wordCount = getWordCount(name);
        MatchItem[] items = new MatchItem[wordCount];
        int len;
        int itemIndex = 0;

        MatchItem item;
        for (int i = 0; i < length && itemIndex < wordCount; i = i + len + 1) {
            len = (int) name.charAt(i);
            item = new MatchItem();
            item.name = name.substring(i + 1, i + len + 1);
            item.nameOffset = nameOffset.substring(i + 1, i + len + 1);
            items[itemIndex] = item;
            itemIndex++;
        }

        return items;
    }

    private boolean isMatched(String offset) {
        if (TextUtils.isEmpty(offset)) {
            return false;
        }

        mMatchNameOffset = offset;
        setMatchType((int) mMatchNameOffset.charAt(0));

        return true;
    }

    private void setMatchType(int newMatchType) {
        if (mMatchType < newMatchType) {
            mMatchType = newMatchType;
        }
    }

    private void initial(String query, String name, String nameOffset, String phoneNumber) {
        mQuery = query;
        mQueryLength = query.length();

        mPhoneNumber = phoneNumber;

        if (!TextUtils.isEmpty(name) && !TextUtils.isEmpty(nameOffset)) {
            mName = name;
            mNameOffset = nameOffset;
            mMatchItems = parseToSearchArray(name, nameOffset);

            mNeedDoNameMatch = true;
        }
    }

    private void reset() {
        mMatchNameOffset = null;
        mMatchNumberOffset = null;
        mMatchType = DS_NOT_MATCH;
        mNeedDoNameMatch = false;
    }

    private void resetMatchItems() {
        for (MatchItem item : mMatchItems) {
            item.matchState = DS_NOT_MATCH;
            item.matchLength = 0;
        }
    }

    private int getWordCount(String normalizedName) {
        final int length = normalizedName.length();
        int wordCount = 0;
        for (int i = 0; i < length; i += normalizedName.charAt(i) + 1) {
            wordCount++;
        }
        return wordCount;
    }

    private int getWordLength(String normalizedName) {
        final int length = normalizedName.length();
        int wordLength = 0;
        for (int i = 0; i < length; i += normalizedName.charAt(i) + 1) {
            wordLength += normalizedName.charAt(i);
        }
        return wordLength;
    }

    // true if the endPart is the end part of the query.
    private boolean queryEndWith(String query, String endPart) {
        for (int i = endPart.length() - 1, j = query.length() - 1; i >= 0; i--, j--) {
            if (!charEquals(query.charAt(j), endPart.charAt(i))) {
                return false;
            }
        }
        return true;
    }

    private void log(String msg) {
        Log.d(TAG, msg);
    }

}
