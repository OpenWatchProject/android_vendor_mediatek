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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.util;

import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Customize the web URL parsing.
 *
 */
public class MtkPatterns {
    private static final String TAG = "MtkPatterns";
    private final static String[] MTK_WEB_PROTOCOL_NAMES = new String[] { "http://",
        "https://", "rtsp://", "ftp://" };
    /**
     * Make new WebProtocolNames for new URL need.
     *
     * @return new WebProtocolNames for extend URL.
     */
    public static String[] getWebProtocolNames(String[] defaultProtocols) {
        //Log.d("@M_" + TAG, "getWebProtocolNames");
        String[] protocols = defaultProtocols;
        protocols = MTK_WEB_PROTOCOL_NAMES;
        return protocols;
    }

    // The regular expressions for filtering out bad url starting and ending
    private static final String mValidCharRegex = "a-zA-Z0-9\\-_";
    // First group: Find the invalid url characters before "xxxx:// or xxx.".
    // Second group: "xxxx:// or xxx."
    private static final String mBadFrontRemovingRegex = String.format(
            "(^[^.]*[^%s.://#&=]+)(?:[a-zA-Z]+://|[%s]+.)", mValidCharRegex, mValidCharRegex);
    // First group: Find the top level domain, ex:".xxx".
    //              The ending ")" is for google map url, which may end with a coordinate,
    //              ex:"(32.012345, 118.54321)"
    // Second group: The rest characters after the TLD to the end of the string,
    //               except some special character may appear in url, ex: "./?=&%"
    private static final String mBadEndRemovingRegex = String.format(
            "([\\.\\:][%s)]+[/%s]*)([\\.\\:]?[^%s\\.\\:\\s/]+[^\\.=&%%/]*$)",
            mValidCharRegex, mValidCharRegex, mValidCharRegex);

    /*
     * Key for Bundle return from @getWebUrl.
     * value -- String
     * start -- int
     * end -- int
     */
    public static final String KEY_URLDATA_VALUE = "value";
    public static final String KEY_URLDATA_START = "start";
    public static final String KEY_URLDATA_END = "end";
    // Helper function for exgular expression group replacing.
    private static final String replaceGroup(String regex, String source, int groupToReplace,
            String replacement) {
        return replaceGroup(regex, source, groupToReplace, 1, replacement);
    }

    // Helper function for exgular expression group replacing.
    private static final String replaceGroup(String regex, String source, int groupToReplace,
            int groupOccurrence, String replacement) {
        Matcher m = Pattern.compile(regex).matcher(source);
        for (int i = 0; i < groupOccurrence; i++) {
            if (!m.find()) {
                return source;
            }
        }
        return new StringBuilder(source).replace(m.start(groupToReplace), m.end(groupToReplace),
            replacement).toString();
    }
    /**
     * Post processing the url string for customization.
     * @param urlStr for url string
     * @param start for start position
     * @param end for end position
     * @return new pattern for extend URL.
     */
    public static Bundle getWebUrl(String urlStr, int start, int end) {
        Log.d("@M_" + TAG, "getWebUrl, " +" start=" + start + " end=" + end);
            if (urlStr != null) {
                /// Filter out invalid characters at the begin of the url
                Pattern p1 = Pattern.compile(mBadFrontRemovingRegex);
                Matcher m1 = p1.matcher(urlStr);
                if (m1.find()) {
                    urlStr = replaceGroup(mBadFrontRemovingRegex, urlStr, 1, "");
                    start = end - urlStr.length();
                }
                /// Filter out invalid characters at the end of the url
                Pattern p2 = Pattern.compile(mBadEndRemovingRegex);
                Matcher m2 = p2.matcher(urlStr);
                if (m2.find()) {
                    urlStr = replaceGroup(mBadEndRemovingRegex, urlStr, 2, "");
                    end = start + urlStr.length();
                }
            }
        //Log.d("@M_" + TAG, "getWebUrl, return: " + " start=" + start + " end=" + end);
        Bundle data = new Bundle();
        data.putString(KEY_URLDATA_VALUE, urlStr);
        data.putInt(KEY_URLDATA_START, start);
        data.putInt(KEY_URLDATA_END, end);
        return data;
    }

    /**
     * Get web url pattern for China characters.
     */
    public static final Pattern getMtkWebUrlPattern(Pattern defaultPattern) {
        Pattern ret = defaultPattern;
        //Log.d("@M_" + TAG, "getMtkWebUrlPattern()");
        ret = ChinaPatterns.CHINA_AUTOLINK_WEB_URL;
        return ret;
    }

}
