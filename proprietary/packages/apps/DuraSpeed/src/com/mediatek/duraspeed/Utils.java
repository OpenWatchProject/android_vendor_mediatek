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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
package com.mediatek.duraspeed;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.graphics.drawable.Drawable;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.duraspeed.manager.IDuraSpeedService;

import java.util.List;

public final class Utils {
    private static final String TAG = "Utils";
    private static final String SETTING_DURASPEED_ENABLED = "setting.duraspeed.enabled";
    private static final int sDuraSpeedDefaultStatus =
            SystemProperties.getInt("persist.vendor.duraspeed.app.on", 0);
    public static final boolean sLowRamDevice =
            SystemProperties.getBoolean("ro.config.low_ram", false);
    public static boolean sStarted = false;

    public static final boolean DURASPEED_ML_SUPPORT =
            SystemProperties.getInt("persist.vendor.duraspeed.ml.support", 0) == 1;
    public static final String DURASPEED_ML_PREFERENCE_KEY = "key_duraspeed_ml";
    private static final String SHARED_PREFERENCE_DS = "DSSharedPreference";
    private static final String DURASPEED_ML_PACKAGE = "com.mediatek.duraspeedml";
    private static final String DURASPEED_ML_SERVICE_START_ACTION =
            "com.mediatek.duraspeedml.prediction_action_alarm";

    private static PackageManager sPackageManager;
    private static IDuraSpeedService sDuraSpeedManager;
    public static DatabaseManager sDatabaseManager;

    public static void setDuraSpeedStatus(Context context, boolean isEnable) {
        Settings.System.putInt(context.getContentResolver(),
                SETTING_DURASPEED_ENABLED, isEnable ? 1 : 0);
        Settings.Global.putInt(context.getContentResolver(),
                SETTING_DURASPEED_ENABLED, isEnable ? 1 : 0);
    }

    public static boolean getDuraSpeedStatus(Context context) {
        int value = Settings.System.getInt(context.getContentResolver(),
                SETTING_DURASPEED_ENABLED, sDuraSpeedDefaultStatus);
        return value == 1;
    }

    public static PackageManager getPackageManager(Context context) {
        if (sPackageManager == null) {
            sPackageManager = context.getPackageManager();
        }
        return sPackageManager;
    }

    public static boolean isSystemApp(ApplicationInfo appInfo) {
        boolean isSystem = false;
        if ((appInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0 ||
                (appInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) {
            isSystem = true;
        }
        return isSystem;
    }

    public static boolean hasLauncherEntry(String pkgName, List<ResolveInfo> intents) {
        for (int j = 0; j < intents.size(); j++) {
            if (pkgName != null && intents.get(j).activityInfo != null) {
                String intentPackageName = intents.get(j).activityInfo.packageName;
                if (pkgName.equals(intentPackageName)) {
                    return true;
                }
            }
        }
        return false;
    }

    public static String getAppLabel(Context context, String pkg) {
        PackageManager pm = getPackageManager(context);
        int retrieveFlags = PackageManager.GET_DISABLED_COMPONENTS;
        ApplicationInfo appInfo;
        try {
            appInfo = pm.getApplicationInfo(pkg, retrieveFlags);
        } catch (PackageManager.NameNotFoundException e) {
            Log.w(TAG, "ApplicationInfo cannot be found for pkg:" + pkg, e);
            return "";
        }
        return pm.getApplicationLabel(appInfo).toString();
    }

    public static Drawable getAppDrawable(Context context, String pkg) {
        PackageManager pm = getPackageManager(context);
        int retrieveFlags = PackageManager.GET_DISABLED_COMPONENTS;
        ApplicationInfo appInfo;
        try {
            appInfo = pm.getApplicationInfo(pkg, retrieveFlags);
        } catch (PackageManager.NameNotFoundException e) {
            Log.w(TAG, "ApplicationInfo cannot be found for pkg:" + pkg, e);
            return null;
        }
        return pm.getApplicationIcon(appInfo);
    }

    public static IDuraSpeedService getDuraSpeedManager() {
        if (sDuraSpeedManager == null) {
            sDuraSpeedManager = IDuraSpeedService.Stub.asInterface(
                        ServiceManager.getService("duraspeed"));
        }
        return sDuraSpeedManager;
    }

    public static void setAppWhitelist(List<String> list) {
        IDuraSpeedService manager = getDuraSpeedManager();
        try {
            manager.setAppWhitelist(list);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void createDatabaseManager(Context context) {
        if (sDatabaseManager == null ) {
            sDatabaseManager = DatabaseManager.getInstance(context);
        } else {
            // some app may be installed when DuraSpeed app process is alive but
            // main activity don't exist that will not receive PACKAGE_ADD
            // broadcast. so we should call update database to update new added
            // package icon when user enter DuraSpeed from setting.
            sDatabaseManager.updateDatabase();
        }
    }

    /**
     * Set DuraSpeedML status.
     *
     * @param context Application context.
     * @param status True means DuraSpeedML is on, otherwise is off.
     */
    public static void setDuraSpeedMLStatus(Context context, boolean status) {
        SharedPreferences sharedPref = context.getSharedPreferences(
                SHARED_PREFERENCE_DS, Context.MODE_PRIVATE);
        sharedPref.edit().putBoolean(DURASPEED_ML_PREFERENCE_KEY, status).apply();
    }

    /**
     * Get DuraSpeedML status.
     *
     * @param context Application context.
     * @return True means DuraSpeedML is on, otherwise is off.
     */
    public static boolean getDuraSpeedMLStatus(Context context) {
        try {
            SharedPreferences sharedPref = context.getSharedPreferences(
                    SHARED_PREFERENCE_DS, Context.MODE_PRIVATE);
            return sharedPref.getBoolean(DURASPEED_ML_PREFERENCE_KEY, false);
        } catch (IllegalStateException e) {
            // In some case like: create a new user and delete it a little later in
            // multi user. Whe create a new user, DuraSpeed will receive the boot
            // broadcast and query DuraSpeedML status in shared preferences, during
            // this time, user deletes the this user, users state will be changed to
            // STOPPED, which maybe throw exception from getSharedPreferences in
            // ContexImpl class.
            return false;
        }
    }

    /**
     * Start DuraSpeedML service.
     *
     * @param context Application context.
     */
    public static void startDuraSpeedMLService(Context context) {
        Intent intent = new Intent();
        intent.setClassName("com.mediatek.duraspeedml","com.mediatek.duraspeedml.InferenceService");
        context.startServiceAsUser(intent, UserHandle.CURRENT);
    }

    /**
     * Stop DuraSpeedML service.
     *
     * @param context Application context.
     */
    public static void stopDuraSpeedMLService(Context context) {
        Intent intent = new Intent();
        intent.setClassName("com.mediatek.duraspeedml","com.mediatek.duraspeedml.InferenceService");
        context.stopServiceAsUser(intent, UserHandle.CURRENT);
    }
}
