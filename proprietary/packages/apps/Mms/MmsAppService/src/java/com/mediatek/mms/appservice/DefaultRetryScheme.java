/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
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

import android.content.Context;
import android.util.Config;
import android.util.Log;

/**
 * Default retry scheme, based on specs.
 */
public class DefaultRetryScheme {
    private static final String TAG = "MmsAppService/DefaultRetryScheme";

    private static final int[] sDefaultRetryScheme = {
        0, 1 * 60 * 1000, 5 * 60 * 1000, 10 * 60 * 1000, 30 * 60 * 1000};

    private static final int[] OP01DEFAULTRETRYSCHEME = {
        0, 5 * 1000, 15 * 1000, 30 * 1000, 5 * 60 * 1000, 10 * 60 * 1000, 30 * 60 * 1000};

    private static int[] sRetryScheme = null;
    private int mRetriedTimes;

    public DefaultRetryScheme(Context context, int retriedTimes) {
        mRetriedTimes = retriedTimes;
        sRetryScheme = OP01DEFAULTRETRYSCHEME;

        mRetriedTimes = mRetriedTimes < 0 ? 0 : mRetriedTimes;
        mRetriedTimes = mRetriedTimes >= sRetryScheme.length
                ? sRetryScheme.length - 1 : mRetriedTimes;
    }

    public int getRetryLimit() {
        return sRetryScheme.length;
    }

    public long getWaitingInterval() {
        return sRetryScheme[mRetriedTimes];
    }

    public void setRetryScheme(int[] retryScheme) {
        Log.v(TAG, "retryScheme: " + retryScheme);
        if (retryScheme != null) {
            sRetryScheme = retryScheme;
        }
    }
}
