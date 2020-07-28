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


import com.mediatek.search.SearchEngineManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;


public class ChangeSearchEngineReceiver extends BroadcastReceiver {
    private static final String XLOGTAG = "browser/ChangeSearchEngineReceiver";
    private static final String ACTION_BROWSER_SEARCH_ENGINE_CHANGED =
        "com.android.browser.SEARCH_ENGINE_CHANGED";

    @Override
    public void onReceive(Context context, Intent intent) {
        SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = p.edit();
        SearchEngineManager searchEngineManager = (SearchEngineManager) context
                .getSystemService(SearchEngineManager.SEARCH_ENGINE_SERVICE);
        com.mediatek.common.search.SearchEngine searchEngineInfo;
        String searchEngineName;
        String searchEngineFavicon = "";
        String action = intent.getAction();

        if (ACTION_BROWSER_SEARCH_ENGINE_CHANGED.equals(action)) {
            if (null == intent.getExtras()) {
                return;
            }

            searchEngineName = intent.getExtras().getString(BrowserSettings.PREF_SEARCH_ENGINE);
            searchEngineInfo = searchEngineManager.getByName(searchEngineName);
            if (searchEngineInfo != null) {
                searchEngineFavicon = searchEngineInfo.getFaviconUri();
            }
            editor.putString(BrowserSettings.PREF_SEARCH_ENGINE, searchEngineName);
            editor.putString(BrowserSettings.PREF_SEARCH_ENGINE_FAVICON, searchEngineFavicon);
            editor.commit();
            Log.d("@M_" + XLOGTAG, "ChangeSearchEngineReceiver (browser): "
                + BrowserSettings.PREF_SEARCH_ENGINE
                + "---" + intent.getExtras().getString(BrowserSettings.PREF_SEARCH_ENGINE));
        }
    }
}
