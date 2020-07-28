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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.mail.parselink;


import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.URLSpan;

import com.android.emailcommon.utility.AsyncTask;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * M: This class is responsible for parsing text content to specific links such as web url, email
 * address, phone number and time event.
 * TODO:
 * 1. Performance: not parse text since the message body is html format.
 */
public class LinkParserTask extends AsyncTask<String, Void, String> {

    // some special code will be converted to ascii code by HtmlSanitizer, e.g. '@'.
    private static final String IGNORE_ASCII_AT = "&#64;";
    private static final int IGNORE_ASCII_AT_LENGTH = IGNORE_ASCII_AT.length();
    private LinkParserCallback mCallback = null;
    private static final String TAG = LogTag.getLogTag();

    private static final String LINK_START_QEURY = "<a href=";
    private static final int LINK_START_QEURY_LENGTH = LINK_START_QEURY.length();

    public LinkParserTask(LinkParserCallback callback) {
        mCallback = callback;
    }

    @Override
    protected String doInBackground(String... params) {
        if (params.length != 1) {
            return null;
        }
        LogUtils.i(TAG, ">>> LinkParserTask.doInBackground");
        String str = params[0];
        return Parser.parseText(str, this);
    }

    @Override
    protected void onPostExecute(String result) {
        super.onPostExecute(result);
        LogUtils.i(TAG, " update result ");
        if (!isCancelled() && result != null) {
            sendCallback(result);
        } else {
            sendCallback(null);
        }
    }

    @Override
    protected void onCancelled() {
        super.onCancelled();
        sendCallback(null);
    }

    private void sendCallback(String result) {
        if (mCallback != null) {
            LogUtils.i(TAG, "LinkParserTask.send callback, sucess ? %s ",
                    (result != null));
            mCallback.onCompleted(result);
        }
    }
    /*
     * extract this class is only to use java2s environment to do unit test.
     */
    public static class Parser {
        private Parser() {}

        private final static String STR_HREF_PATTERN = "(<a href=.+?>.+?</a>)";

        private final static Pattern HREF_PATTERN = Pattern.compile(STR_HREF_PATTERN,
                                                                    Pattern.CASE_INSENSITIVE);

        private final static Pattern FIRST_ROUND_PATTERNS = Pattern.compile(STR_HREF_PATTERN
                + "|" + CustomPattern.WEB_URL.pattern() , Pattern.CASE_INSENSITIVE);

        /**
         * This function is the main entrance for parsing links. It consist of 2 round parsing.
         * Because phone number and time event may have conflict when parsing. So we split the
         * parsing process into 2 steps. First round parses existing hyperlinks(href), image tag,
         * email address and phone number. The second round parses existing hyperlinks, image tag,
         * and time event.
         * @param text
         * @param task
         * @return
         */
        public static String parseText(String text, AsyncTask task) {
            LogUtils.i(TAG, "LinkParserTask.parseText");
            if (task != null && task.isCancelled()) {
                return null;
            }
            return parseText(text);
        }

        /**
         * Parse text and add hyperlinks for web link.
         * @param text input text.
         * @return text with hyperlinks
         */
        public static String parseText(String text) {
            StringBuilder sb = new StringBuilder();
            Matcher matcher = FIRST_ROUND_PATTERNS.matcher(text);
            int startIndex = 0;
            // first round parse
            while (matcher.find()) {
                int start = matcher.start();
                String nextMatchStr = matcher.group();
                sb.append(text.substring(startIndex, start));
                if (HREF_PATTERN.matcher(nextMatchStr).matches()) {
                    sb.append(nextMatchStr);
                } else if (CustomPattern.WEB_URL.matcher(nextMatchStr).matches()
                        && !isWebLink(text, start)
                        && !isSurroundedByAngleBrachets(text, start)
                        // not a email address.
                        && !isEmailAddressDomain(text, start)) {
                    /// M: if start is 0, needn't to check the char. if nextMatchStr is
                    // start with WEB_SCHEME contains type, no need to append 'http://' prifix.
                    if (CustomPattern.WEB_SCHEME.matcher(nextMatchStr).matches()) {
                        sb.append("<a href=\"" + nextMatchStr + "\">" + nextMatchStr + "</a>");
                    } else {
                        sb.append("<a href=\"http://" + nextMatchStr + "\">"
                                + nextMatchStr + "</a>");
                    }
                } else {
                    sb.append(nextMatchStr);
                }
                startIndex = start + nextMatchStr.length();
            }
            sb.append(text.substring(startIndex));
            return sb.toString();
        }

        private static boolean isSurroundedByAngleBrachets(String text, int endIndex) {
            int leftArrowIndex = text.indexOf("<", endIndex);
            int rightArrowIndex = text.indexOf(">", endIndex);
            if (leftArrowIndex == -1) {
                return false;
            } else if (rightArrowIndex != -1 && leftArrowIndex == -1) {
                return true;
            } else if (rightArrowIndex > leftArrowIndex) {
                return false;
            }
            return true;
        }

        /**
         * Check if weblink is already a link, with "href tag".
         */
        private static boolean isWebLink(String text, int start) {
            // There maybe a quote between "<a href=" and link, the length +1.
            if (start < LINK_START_QEURY_LENGTH + 1) {
                return false;
            }
            String query = text.substring((start - LINK_START_QEURY_LENGTH - 1), start);
            if (query.contains(LINK_START_QEURY)) {
                return true;
            }
            return false;
        }

        private static boolean isEmailAddressDomain(String text, int start) {
            // not start with @
            if (start != 0 && text.charAt(start - 1) == '@') {
                return true;
            }

            // not start with ecoded @ "&#64;"
            if (start >= IGNORE_ASCII_AT_LENGTH
                    && text.substring(start - IGNORE_ASCII_AT_LENGTH, start).equalsIgnoreCase(
                            IGNORE_ASCII_AT)) {
                return true;
            }
            return false;
        }
    }

    public interface LinkParserCallback {
        void onCompleted(String paresResult);
    }

    /**
     * Parse SpannableString and add URLSpan if match web link.
     * @param text source SpannableString.
     */
    public static void addWebLinkSpan(SpannableString text) {
        Matcher matcher = Pattern.compile(CustomPattern.WEB_URL.pattern(),
                Pattern.CASE_INSENSITIVE).matcher(text);
        while (matcher.find()) {
            int start = matcher.start();
            String nextMatchStr = matcher.group();
            //ignore start tag and email address.
            if (start == 0 || text.charAt(start - 1) != '@') {
                if (CustomPattern.WEB_SCHEME.matcher(nextMatchStr).matches()) {
                    URLSpan span = new URLSpan(nextMatchStr);
                    text.setSpan(span, start, start + nextMatchStr.length(),
                            Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                } else {
                    // add http scheme to compose a complete url.
                    String url = "http://" + nextMatchStr;
                    URLSpan span = new URLSpan(url);
                    text.setSpan(span, start, start + nextMatchStr.length(),
                            Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                }
            }
        }
    }
}
