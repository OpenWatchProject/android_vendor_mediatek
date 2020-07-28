/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.browser;

import android.util.Log;
import android.os.AsyncTask;

public class OutputMemoryInfo extends AsyncTask<TabControl, Void, Void> {
    private static final String LOGTAG = "browser";
    private TabControl tabController = null;
    private boolean logToFile = false;
    private String savedFileName;

    @Override
    protected Void doInBackground(TabControl... params) {
        if (params.length != 2) {
            Log.d(LOGTAG, "Incorrect parameters to OutputMemoryInfo's doInBackground(): "
                           + String.valueOf(params.length));
        } else {
            tabController = params[0];
            if (params.length == 2 && params[1] != null) {
                logToFile = true;
            }

            //output memory info
            String flag = "BrowserMemory" + System.currentTimeMillis();
            savedFileName = Performance.printMemoryInfo(logToFile, flag);
        }
        return null;
    }
}