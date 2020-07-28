/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications AB.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications AB.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.mediatek.rcs.provisioning.https;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.mediatek.rcs.utils.AndroidFactory;
import com.mediatek.rcs.utils.LauncherUtils;
import com.mediatek.rcs.utils.RcsSettings;

import java.util.Locale;

/**
 * HTTPS provisioning - utils
 *
 * @author Orange
 */
public class HttpsProvisioningUtils {

    /**
     * Input MSISDN timeout
     */
    /* package private */static final int INPUT_MSISDN_TIMEOUT = 60000;

    static final int[] RETRY_SCHEME = {20 * 60 * 1000,
                                       60 * 60 * 1000,
                                       2 * 60 * 60 * 1000,
                                       4 * 60 * 60 * 1000,
                                       8 * 60 * 60 * 1000};
    /**
     * Retry base timeout - 5min (in milliseconds)
     */
    /* package private */static final int RETRY_BASE_TIMEOUT = 300000;

    /**
     * Retry after an 511 "Network authentication required" timeout (in milliseconds)
     */
    /* package private */static final int RETRY_AFTER_511_ERROR_TIMEOUT = 5000;

    /**
     * The action if a binary SMS received
     */
    /* package private */static final String ACTION_BINARY_SMS_RECEIVED = "android.intent.action.DATA_SMS_RECEIVED";

    /**
     * Char sequence in a binary SMS to indicate a network initiated configuration
     */
    /* package private */static final String RESET_CONFIG_SUFFIX = "-rcscfg";

    /**
     * Retry max count
     */
    /* package private */static final int RETRY_MAX_COUNT = 5;

    /**
     * Retry after 511 "Network authentication required" max count
     */
    /* package private */static final int RETRY_AFTER_511_ERROR_MAX_COUNT = 5;

    /**
     * Get the current device language
     *
     * @return Device language (like fr-FR)
     */
    /* package private */static String getUserLanguage() {
        return Locale.getDefault().getLanguage() + "-" + Locale.getDefault().getCountry();
    }

    /**
     * Returns the RCS version
     *
     * @return String(4)
     */
    /* package private */static String getRcsVersion() {
        //return "5.1B";
        return "6.0";
    }

    /**
     * Returns the RCS profile
     *
     * @return String(15)
     */
    /* package private */static String getRcsProfile() {
        return "joyn_blackbird";
    }

    /**
     * Returns the RCS state
     *
     * @return  the RCS state
     */
    static String getRcsState() {
        RcsSettings settings = RcsSettings.getInstance();
        int version = settings.getProvisioningVersion();
        //if ((version > 0) && !settings.isServiceActivated()) {
        if (!settings.isServiceActivated()) {
            return "-4";
        } else {
            return String.valueOf(version);
        }
    }

    /**
     * Returns the Enrich Calling profile
     *
     * @return String(15)
     */
    /* package private */static String getEnrichCallingProfile() {
        return "enriched_call";
    }

    public static String getMsisdn() {
        // add interface from RcsUaService, UaService set parameter to ACS
        Context ctx = AndroidFactory.getApplicationContext();
        TelephonyManager tm = (TelephonyManager)ctx.getSystemService(Context.TELEPHONY_SERVICE);
        return tm.getLine1Number();
        //return tm.getMsisdn();
    }

    public static String getImsi() {
        Context ctx = AndroidFactory.getApplicationContext();
        TelephonyManager tm = (TelephonyManager)ctx.getSystemService(Context.TELEPHONY_SERVICE);
        return tm.getSubscriberId();
        //return LauncherUtils.getLastUserAccount(AndroidFactory.getApplicationContext());
    }

    public static String getImei() {
        Context ctx = AndroidFactory.getApplicationContext();
        TelephonyManager tm = (TelephonyManager)ctx.getSystemService(Context.TELEPHONY_SERVICE);
        //return tm.getImei();
        return tm.getDeviceId();
    }

}
