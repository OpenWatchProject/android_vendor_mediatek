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

package com.mediatek.mms.appservice;

import android.os.Bundle;
import android.telephony.SmsManager;
/**
 * Util methods for MtkPduParser
 */
public class PduParserUtil {
    /**
     * Get the config of whether Content-Disposition header is supported
     * for default carrier using new SmsManager API
     *
     * @return true if supported, false otherwise
     */
    public static boolean shouldParseContentDisposition(int subId) {
        SmsManager smsManager = SmsManager.getSmsManagerForSubscriptionId(subId);
        if (smsManager != null) {
            Bundle bundle = smsManager.getCarrierConfigValues();
            if (bundle != null) {
                return bundle.getBoolean(SmsManager.MMS_CONFIG_SUPPORT_MMS_CONTENT_DISPOSITION,
                        true);
            }
        }
        return true;
        //M: google original code
        /*
        return SmsManager
                .getSmsManagerForSubscriptionId(subId)
                .getCarrierConfigValues()
                .getBoolean(SmsManager.MMS_CONFIG_SUPPORT_MMS_CONTENT_DISPOSITION, true);
        */
    }
}
