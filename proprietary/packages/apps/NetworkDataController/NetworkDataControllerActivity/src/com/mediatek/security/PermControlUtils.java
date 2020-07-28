
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.security;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.NotificationChannel;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.Drawable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.Process;
import android.os.UserHandle;
import android.provider.Settings;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public class PermControlUtils {
    private static final String TAG = "PermControlUtils";

    public static final String PACKAGE_NAME = "exta_package_name";
    public static final String UID = "exta_uid";
    public static final String CHECKED_PERM_RECORD = "exta_CheckedPermRecord";

    public static final String PERM_CONTROL_AUTOBOOT_UPDATE =
    "com.mediatek.security.action.AUTOBOOT_UPDATE";

    public static final String NETWORK_DATA_UPDATE =
      "com.mediatek.security.action.NETWORK_DATA_UPDATE";

    public static final String EXTRA_STATE = "state";
    public static final String CELLULAR_CONTROL_STATE = "cellulardata_control_state";
    public static final String IS_HANDLE_CHECK_PERM = "IS_HANDLE_CHECK_PERM";

    public static final int MAX_WATI_TIME = 20000; // in mill-seconds
    public static final int NOTIFY_FOREGROUND_ID = 1201;

    public static int GET_ALL_PACKAGE = 1;
    public static int GET_3TH_PACKAGE = 2;
    public static int GET_MAIN_LAUNCH_PACKAGE = 3;

    public static int RESULT_START_ACTIVITY = 0;
    public static int RESULT_FINISH_ITSELF = 1;

    public static final String PKG_NAME = "package_name";
    public static final String NOTIFYCATION_CHANNEL = "PermControl";

    /**
     * Get application name by passing the package name
     *
     * @param context
     *            Context
     * @param pkgName
     *            package name
     * @return the application name
     */
    public static String getApplicationName(Context context, String pkgName) {
        if (pkgName == null) {
            return null;
        }
        String appName = null;
        try {
            PackageManager pkgManager = context.getPackageManager();
            ApplicationInfo info = pkgManager.getApplicationInfo(pkgName, 0);
            appName = String.valueOf(pkgManager.getApplicationLabel(info));
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        return appName == null ? pkgName : appName;
    }

    /**
     * Get application icon by passing the package name
     *
     * @param context
     *            Context
     * @param pkgName
     *            package name
     * @return the application icon
     */
    public static Drawable getApplicationIcon(Context context, String pkgName) {
	    PackageManager pm = context.getPackageManager();
        Drawable icon = null;
        try {
            icon = pm.getApplicationIcon(pkgName);
            //need test if pass pkgName can get icon
        } catch (NameNotFoundException ex) {
            icon = pm.getDefaultActivityIcon();
            Log.e(TAG, pkgName + " app icon not found, using generic app icon");
        }
        return icon;
    }

   public static List<PackageInfo> getInstalledPackages(Context context,
            int type) {
        Log.d(TAG, "getInstalledPackages()");
        List<PackageInfo> installedPackages = new ArrayList<PackageInfo>();
        Collection<ApplicationInfo> applicationInfos = null;
        PackageManager pm = context.getPackageManager();
        try {
            applicationInfos = pm
                    .getInstalledApplications(PackageManager.GET_UNINSTALLED_PACKAGES
                            | PackageManager.GET_DISABLED_COMPONENTS);

            if (applicationInfos != null) {
                Iterator<ApplicationInfo> iterator = applicationInfos
                        .iterator();
                while (iterator.hasNext()) {
                    ApplicationInfo applicationInfo = (ApplicationInfo) iterator
                            .next();
                    if (!(type == GET_3TH_PACKAGE && isSystemApp(applicationInfo))) {
                        PackageInfo packageInfo = pm.getPackageInfo(
                                applicationInfo.packageName,
                                PackageManager.GET_PERMISSIONS);
                        if (packageInfo != null) {
                            installedPackages.add(packageInfo);
                        }
                    }
                }
            }
        } catch (NameNotFoundException e) {
            Log.e(TAG, "getPackageInfo() failed!", e);
        }
        Log.d(TAG, "getInstalledPackages(), size = " + installedPackages.size());
        return installedPackages;
    }

        /**
     * Get whether the pkg is system or not
     *
     * @param ApplicationInfo
     * @return boolean
     */

    public static boolean isSystemApp(ApplicationInfo applicationInfo) {
        if ((applicationInfo != null)
                && (applicationInfo.uid < Process.FIRST_APPLICATION_UID)) {
            return true;
        } else {
            Log.d(TAG,
                    "isSystemApp() return false with null packageName or uid >= 10000");
            return false;
        }
    }
}
