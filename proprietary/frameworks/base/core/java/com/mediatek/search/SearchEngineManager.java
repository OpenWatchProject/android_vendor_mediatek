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

/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.search;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Slog;

import com.mediatek.common.search.SearchEngine;
import android.annotation.ProductApi;
import java.util.List;

/**
 * This class provides access to the system search engine services.
 *
 * If you do require direct access to the SearchEngineManager, do not instantiate
 * this class directly. Instead, retrieve it through
 * {@link android.content.Context#getSystemService
 * context.getSystemService(Context.SEARCH_ENGINE_SERVICE)}.
 */
public class SearchEngineManager
{

    private static final boolean DBG = false;
    private static final String TAG = "SearchEngineManager";
    /**
     * Reference to the shared system search service.
     */
    private static ISearchEngineManagerService mService;

    private final Context mContext;

    /**
     * This action use to broadcast for search engine change.
     *
     */
    public static final String ACTION_SEARCH_ENGINE_CHANGED =
    "com.mediatek.search.SEARCH_ENGINE_CHANGED";


    /**
     * M: Use with {@link #getSystemService} to retrieve a {@link
     * com.mediatek.search.SearchEngineManager} for handling searches.
     *
     * @see #getSystemService
     * @see com.mediatek.search.SearchEngineManager
     * @hide
     * @internal
     */
   public static final String SEARCH_ENGINE_SERVICE = "search_engine_service";



    public SearchEngineManager(Context context) {
        mContext = context;
        mService = ISearchEngineManagerService.Stub.asInterface(
                ServiceManager.getService(SEARCH_ENGINE_SERVICE));
    }

    /**
     * Returns a list of SearchEngine that can be used by all applications to do web search
     * @return list of all SearchEngine in current locale
     *
     */
	@ProductApi  
    public List<SearchEngine> getAvailables() {
        try {
            return mService.getAvailables();
        } catch (RemoteException e) {
            Slog.e(TAG, "getSearchEngineInfos() failed: " + e);
            return null;
        }
    }

    /**
     * Get search engine by name or favicon. If could not find search engine by name,
     * then find search engine by favicon.
     * @param name the search engine name
     * @param favicon the search engine favicon
     * @return if found then return the search engine, else return null
     *
     */
	@ProductApi  
    public SearchEngine getBestMatch(String name, String favicon) {
        try {
            return mService.getBestMatch(name, favicon);
        } catch (RemoteException e) {
            Slog.e(TAG, "getBestMatch() failed: " + e);
            return null;
        }
    }

    /**
     * Get search engine by favicon uri.
     * @param favicon the search engine favicon
     * @return the search engine
     *
     */
    public SearchEngine getByFavicon(String favicon) {
        return getSearchEngine(SearchEngine.FAVICON, favicon);
    }

    /**
     * Get search engine by name.
     * @param name the search engine name
     * @return the search engine
     *
     */
	@ProductApi  
    public SearchEngine getByName(String name) {
        return getSearchEngine(SearchEngine.NAME, name);
    }

    /**
     * Get search engine through specified field and value.
     * @param field the field of SearchEngine
     * @param value the value of the field
     * @return the search engine
     *
     */
    public SearchEngine getSearchEngine(int field, String value) {
        try {
            return mService.getSearchEngine(field, value);
        } catch (RemoteException e) {
            Slog.e(TAG, "getSearchEngine(int field, String value) failed: " + e);
            return null;
        }
    }

    /**
     * Get system default search engine.
     * @return the search engine
     *
     */
	@ProductApi 
    public SearchEngine getDefault() {
        try {
            return mService.getDefault();
        } catch (RemoteException e) {
            Slog.e(TAG, "getSystemDefaultSearchEngine() failed: " + e);
            return null;
        }
    }

    /**
     * Set default search engine for system.
     * @param engine the search engine to set
     * @return if set success then return true, else return false
     *
     */
    public boolean setDefault(SearchEngine engine) {
        try {
            return mService.setDefault(engine);
        } catch (RemoteException e) {
            Slog.e(TAG, "getSystemDefaultSearchEngine() failed: " + e);
            return false;
        }
    }
}
