/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.android.browser;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import com.android.browser.provider.BrowserProvider2;

import java.util.ArrayList;
import java.util.HashMap;

public class OmacpSettingReceiver extends BroadcastReceiver {
    private static final boolean DEBUG = true;
    private static final String XLOG = "browser/OmacpSettingReceiver";

    private static final String BROWSER_APP_ID = "w2";
    private static final String APP_ID_KEY = "appId";
    private static final String APP_RESULT = "result";
    private static final String APP_SETTING_ACTION = "com.mediatek.omacp.settings";
    private static final String APP_SETTING_RESULT_ACTION = "com.mediatek.omacp.settings.result";
    private static final String APP_CAPABILITY_ACTION = "com.mediatek.omacp.capability";
    private static final String APP_CAPABILITY_RESULT_ACTION
                                    = "com.mediatek.omacp.capability.result";

    //Omacp key of setting app
    private static final String FOLDER_NAME = "NAME"; //application/NAME
    private static final String RESOURCE = "RESOURCE"; //resource
    private static final String BOOKMARK_URI = "URI"; //resource/URI
    private static final String BOOKMARK_NAME = "NAME"; //resource/NAME
    private static final String STARTPAGE = "STARTPAGE"; //resource/STARTPAGE
    private static final String STARTPAGE_TRUE = "1"; //STARTPAGE = true

    //Omacp key of capability item
    private static final String BROWSER = "browser";
    private static final String BROWSER_BOOKMARK_FOLDER = "browser_bookmark_folder";
    private static final String BROWSER_TO_PROXY = "browser_to_proxy";
    private static final String BROWSER_TO_NAPID = "browser_to_napid";
    private static final String BROWSER_BOOKMARK_NAME = "browser_bookmark_name";
    private static final String BROWSER_BOOKMARK = "browser_bookmark";
    private static final String BROWSER_USERNAME = "browser_username";
    private static final String BROWSER_PASSWORD = "browser_password";
    private static final String BROWSER_HOMEPAGE = "browser_homepage";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (DEBUG) {
            Log.d("@M_" + XLOG, "OmacpSettingReceiver action:" + intent.getAction());
        }
        final ContentResolver cr = context.getContentResolver();
        if (APP_SETTING_ACTION.equals(intent.getAction())) {
            boolean result = false;
            String folderName = intent.getStringExtra(FOLDER_NAME);
            if (null == folderName) {
                result = setBookmarkAndHomePage(context, intent, BrowserProvider2.FIXED_ID_ROOT);
            } else {
                Log.i("@M_" + XLOG, "folderName isn't null");
                long folderId = AddBookmarkPage.addFolderToRoot(context, folderName);
                result = setBookmarkAndHomePage(context, intent, folderId);
            }
            //send setting result to Omacp
            sendSettingResult(context, result);
        }
        if (APP_CAPABILITY_ACTION.equals(intent.getAction())) {
            //send capability result to Omacp
            sendCapabilityResult(context);
        }
    }
    private boolean setBookmarkAndHomePage(Context context, Intent intent, long folderId) {
        boolean result = false;
        if (-1 == folderId) {
            return result;
        }

        final ContentResolver cr = context.getContentResolver();
        ArrayList<HashMap<String, String>> resourceMapList =
            (ArrayList<HashMap<String, String>>) intent.getSerializableExtra(RESOURCE);
        if (null == resourceMapList) {
            Log.i("@M_" + XLOG, "resourceMapList is null");
        } else {
            if (DEBUG) {
                 Log.i("@M_" + XLOG, "resourceMapList size:" + resourceMapList.size());
            }
            boolean hasSetStartPage = false;
            for (HashMap<String, String> item : resourceMapList) {
                String url = item.get(BOOKMARK_URI);
                String name = item.get(BOOKMARK_NAME);
                String startPage = item.get(STARTPAGE);
                if (null == url) {
                    continue;
                }
                String formattedUrl = UrlUtils.fixUrl(url);
                if (null == formattedUrl) {
                    continue;
                }
                if (null == name) {
                    name = formattedUrl;
                }
                formattedUrl = UrlUtils.smartUrlFilter(formattedUrl);
                Bookmarks.addBookmark(context, false, formattedUrl, name, null, folderId);

                if (!hasSetStartPage && null != startPage && startPage.equals(STARTPAGE_TRUE)) {
                    setHomePage(context, formattedUrl);
                    hasSetStartPage = true;
                }
                if (DEBUG) {
                    Log.i("@M_" + XLOG, "BOOKMARK_URI: " + formattedUrl);
                    Log.i("@M_" + XLOG, "BOOKMARK_NAME: " + name);
                    Log.i("@M_" + XLOG, "STARTPAGE: " + startPage);
                }
            }
            result = true;
        }
        return result;
    }
    private boolean setHomePage(Context context, String url) {
        if (null == url || url.length() <= 0) {
            return false;
        }
        if (!url.startsWith("http:")) {
            url = "http://" + url;
        }
        SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = p.edit();
        editor.putString(BrowserSettings.PREF_HOMEPAGE, url);
        editor.commit();
        return true;
    }
    private void sendSettingResult(Context context, boolean result) {
        Intent intent = new Intent(APP_SETTING_RESULT_ACTION);
        intent.putExtra(APP_ID_KEY, BROWSER_APP_ID);
        intent.putExtra(APP_RESULT, result);

        if (DEBUG) {
            Log.i("@M_" + XLOG, "Setting Broadcasting: " + intent);
        }
        context.sendBroadcast(intent);
    }
    private void sendCapabilityResult(Context context) {
        Intent intent = new Intent(APP_CAPABILITY_RESULT_ACTION);
        intent.putExtra(APP_ID_KEY, BROWSER_APP_ID);
        intent.putExtra(BROWSER, true);
        intent.putExtra(BROWSER_BOOKMARK_FOLDER, true);
        intent.putExtra(BROWSER_TO_PROXY, false);
        intent.putExtra(BROWSER_TO_NAPID, false);
        intent.putExtra(BROWSER_BOOKMARK_NAME, true);
        intent.putExtra(BROWSER_BOOKMARK, true);
        intent.putExtra(BROWSER_USERNAME, false);
        intent.putExtra(BROWSER_PASSWORD, false);
        intent.putExtra(BROWSER_HOMEPAGE, true);

        if (DEBUG) {
            Log.i("@M_" + XLOG, "Capability Broadcasting: " + intent);
        }
        context.sendBroadcast(intent);
    }
}
