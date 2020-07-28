/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.mediatek.rcs.utils;

import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ResolveInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.os.Build;
import android.os.SystemProperties;
import android.provider.Telephony;
import android.telephony.TelephonyManager;

import com.mediatek.rcs.provisioning.ProvisioningInfo;
import com.mediatek.rcs.provisioning.ProvisioningInfo.Version;
import com.mediatek.rcs.utils.RcsSettings.TermsAndConditionsResponse;
import com.mediatek.rcs.utils.logger.Logger;


/**
 * Launcher utility functions
 *
 * @author hlxn7157
 */
public class LauncherUtils {
    public static final String RCS_PREFS_NAME = "RCS";

    /**
     * Last user account used
     */
    public static final String REGISTRY_LAST_USER_ACCOUNT = "LastUserAccount";

    /**
     * Key for storing the latest positive provisioning version
     */
    private static final String REGISTRY_PROVISIONING_VERSION = "ProvisioningVersion";

    /**
     * Key for storing the expiration date of the provisioning
     */
    private static final String REGISTRY_PROVISIONING_EXPIRATION = "ProvisioningExpiration";


    /**
     * Key for storing the latest positive provisioning validity
     */
    private static final String REGISTRY_PROVISIONING_VALIDITY = "ProvisioningValidity";

    private static final long DEFAULT_PROVISIONING_VALIDITY = 24 * 3600 * 1000L;

    private static final Logger sLogger = Logger.getLogger(LauncherUtils.class.getName());

    /**
     * Reset RCS configuration
     *
     * @param ctx Application context
     * @param localContentResolver Local content resolver
     * @param rcsSettings RCS settings accessor
     * @param contactManager Contact manager accessor
     */
    public static void resetRcsConfig(Context ctx, RcsSettings rcsSettings) {
        // if (sLogger.isActivated()) {
        //     sLogger.debug("Reset RCS config");
        // }
        // /* Stop the Core service */
        // ctx.stopService(new Intent(ctx, RcsCoreService.class));

        /* Reset existing configuration parameters */
        rcsSettings.resetConfigParameters();

        /* Clear all entries in chat, message and file transfer tables */

        /* Clear all entries in Rich Call tables (image and video) */
        // RichCallHistory.getInstance(localContentResolver);
        // RichCallHistory.getInstance().deleteAllEntries();

        /*
         * Clean the previous account RCS databases : because they may not be overwritten in the
         * case of a very new account or if the back-up files of an older one have been destroyed.
         */
        //contactManager.deleteRCSEntries();

        /* Remove the RCS account */
        // RcsAccountManager accountUtility = RcsAccountManager.getInstance(ctx, contactManager);
        // accountUtility.removeRcsAccount(null);
        /*
         * Ensure that factory is set up properly to avoid NullPointerException in
         * AccountChangedReceiver.setAccountResetByEndUser
         */
        //AndroidFactory.setApplicationContext(ctx, rcsSettings);
        AndroidFactory.setApplicationContext(ctx);
        //AccountChangedReceiver.setAccountResetByEndUser(false);

        /* Clean terms status */
        rcsSettings.setTermsAndConditionsResponse(TermsAndConditionsResponse.NO_ANSWER);

        /* Set the configuration validity flag to false */
        rcsSettings.setConfigurationValid(false);

        rcsSettings.setConfigFileContent(null);
        //rcsSettings.setProvisioningToken(null);
        rcsSettings.setConfigurationValid(false);
        rcsSettings.setProvisioningVersion(Version.RESETED.toInt());
        if (RcsSettings.ATT_SUPPORT) {
            rcsSettings.setPublishSourceThrottle(RcsSettings.DEFAULT_PUBLISH_SOURCE_THROTTLE);
            rcsSettings.setPollingPeriod(RcsSettings.DEFAULT_CAPABILITY_POLLING_PERIOD);
            rcsSettings.setNonRcscapInfoExpiray(RcsSettings
                                                .DEFAULT_CAPABILITY_NON_RCSCAPINFO_EXPIRAY);
            rcsSettings.setInfoExpiry(RcsSettings.DEFAULT_CAPABILITY_EXPIRY_TIMEOUT);
            rcsSettings.setFtHttpServer(null);
            rcsSettings.setFtHttpLogin(null);
            rcsSettings.setFtHttpPassword(null);
        }

        if (sLogger.isActivated()) {
            sLogger.warn("resetRcsConfig");
        }
    }


    /**
     * Save the latest positive provisioning version in shared preferences
     *
     * @param context Application context
     * @param version the latest positive provisioning version
     */
    public static void saveProvisioningVersion(Context context, int version) {
        if (version > 0) {
            SharedPreferences preferences = context.getSharedPreferences(
                    RCS_PREFS_NAME, Activity.MODE_PRIVATE);
            SharedPreferences.Editor editor = preferences.edit();
            editor.putInt(REGISTRY_PROVISIONING_VERSION, version);
            editor.commit();
        }
    }

    /**
     * Get the latest positive provisioning version
     *
     * @param context Application context
     * @return the latest positive provisioning version
     */
    public static int getProvisioningVersion(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(
                RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        return preferences.getInt(REGISTRY_PROVISIONING_VERSION,
                ProvisioningInfo.Version.RESETED.toInt());
    }

    /**
     * Get the expiration date of the provisioning
     *
     * @param context Application context
     * @return the expiration date in milliseconds or 0 if not applicable
     */
    public static long getProvisioningExpirationDate(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(
                RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        return preferences.getLong(REGISTRY_PROVISIONING_EXPIRATION, 0L);
    }

    /**
     * Get the expiration date of the provisioning
     *
     * @param context Application context
     * @return the expiration date in milliseconds
     */
    public static long getProvisioningValidity(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(
                RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        return preferences.getLong(REGISTRY_PROVISIONING_VALIDITY, DEFAULT_PROVISIONING_VALIDITY);
    }

    /**
     * Save the provisioning validity in shared preferences
     *
     * @param context Context
     * @param validity validity of the provisioning expressed in milliseconds
     */
    public static void saveProvisioningValidity(Context context, long validity) {
        if (validity <= 0L) {
            return;
        }
        /* Calculate next expiration time in milliseconds */
        long next = System.currentTimeMillis() + validity;
        SharedPreferences preferences = context.getSharedPreferences(
                RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putLong(REGISTRY_PROVISIONING_VALIDITY, validity);
        editor.putLong(REGISTRY_PROVISIONING_EXPIRATION, next);
        editor.commit();
    }


    /**
     * Get the last user account
     *
     * @param context Application context
     * @return last user account
     */
    public static String getLastUserAccount(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(
                RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        return preferences.getString(REGISTRY_LAST_USER_ACCOUNT, null);
    }

    /**
     * Set the last user account
     *
     * @param context Application context
     * @param value last user account
     */
    public static void setLastUserAccount(Context context, String value) {
        SharedPreferences preferences = context.getSharedPreferences(
                RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(REGISTRY_LAST_USER_ACCOUNT, value);
        editor.commit();
    }

    /**
     * Get current user account
     *
     * @param context Application context
     * @return current user account
     */
    public static String getCurrentUserAccount(Context context) {
        TelephonyManager mgr = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
        String currentUserAccount = mgr.getSubscriberId();
        if (currentUserAccount == null) {
            if (sLogger.isActivated()) {
                sLogger.warn("Cannot get subscriber ID from telephony manager!");
            }
        }
        return currentUserAccount;
    }


    public static String getDefaultSmsAppPackageName(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            return Telephony.Sms.getDefaultSmsPackage(context);
        } else {
            Intent intent = new Intent(Intent.ACTION_VIEW).addCategory(Intent.CATEGORY_DEFAULT).setType("vnd.android-dir/mms-sms");
            final List<ResolveInfo> resolveInfos = context.getPackageManager()
                .queryIntentActivities(intent, 0);
            if (resolveInfos != null && !resolveInfos.isEmpty()) {
                return resolveInfos.get(0).activityInfo.packageName;
            }

            return null;
        }
    }

    public static String getVersionCode(Context ctx) {
        PackageManager pMgr = ctx.getPackageManager();
        PackageInfo packageInfo;
        String versionCode = "";
        try {
            packageInfo = pMgr.getPackageInfo(ctx.getPackageName(), 0);
            versionCode = packageInfo.versionCode + "";
            int updateVersionCode = SystemProperties.getInt("persist.vendor.mtk_acs_version", -1);
            if (updateVersionCode > 0) {
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("update versionCode " + versionCode  + "->" + updateVersionCode);
                }
                versionCode = String.valueOf(updateVersionCode);
            }
        } catch(PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        return versionCode;
    }
}