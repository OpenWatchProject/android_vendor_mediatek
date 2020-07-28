/*
 * Copyright (C) 2008 Esmertec AG.
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

package com.mediatek.mms.appservice;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.drm.DrmManagerClient;
import android.location.Country;
import android.location.CountryDetector;
import android.location.CountryListener;
import android.os.StrictMode;
import android.preference.PreferenceManager;
import android.provider.SearchRecentSuggestions;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.mms.appservice.DownloadManager;
import com.mediatek.mms.appservice.LogTag;
import com.mediatek.mms.appservice.MmsConfig;
import com.mediatek.mms.appservice.MmsReceiver;
import com.mediatek.mms.appservice.TransactionService;


public class MmsAppService extends Application {
    public static final String LOG_TAG = LogTag.TAG;

    private static MmsAppService sMmsApp = null;

    @Override
    public void onCreate() {
        super.onCreate();
        sMmsApp = this;
        DownloadManager.init(this);
        registerMmsProcessedReceiver(this);
        RateController.init(this);
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
    }

    synchronized public static MmsAppService getApplication() {
        return sMmsApp;
    }

    private void registerMmsProcessedReceiver (Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TransactionService.ACTION_TRANSACION_PROCESSED);
        context.registerReceiver(new MmsReceiver(), intentFilter);
    }
}
