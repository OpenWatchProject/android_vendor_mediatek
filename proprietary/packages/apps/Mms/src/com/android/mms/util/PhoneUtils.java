/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.mms.util;

import android.content.Context;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;

import com.android.mms.MmsApp;

import java.util.List;

/**
 * Utilities for phone related stuff
 */
public class PhoneUtils {
    /**
     * Check whether there has active SubInfo indicated by given subId on the device.
     * @param subId
     * @return
     */
    public static boolean isValidSubId(Context context, int subId) {
        boolean isValid = false;
        List<SubscriptionInfo> activeSubInfoList = SubscriptionManager
                .from(MmsApp.getApplication()).getActiveSubscriptionInfoList();
        if (activeSubInfoList != null) {
            for (SubscriptionInfo subInfoRecord : activeSubInfoList) {
                if (subInfoRecord.getSubscriptionId() == subId) {
                    isValid = true;
                    break;
                }
            }
        }
        return isValid;
    }
}
