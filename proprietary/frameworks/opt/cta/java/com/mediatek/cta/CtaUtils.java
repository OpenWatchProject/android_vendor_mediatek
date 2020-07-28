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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.cta;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.ActivityManagerNative;
import android.app.AppGlobals;
import android.app.AppOpsManager;
import android.app.IActivityManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Binder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.text.TextUtils;
import android.util.ArrayMap;
import android.util.Log;

import com.android.internal.app.IAppOpsService;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

final class CtaUtils {

    private static final String TAG = "CTA_CtaUtils";

    private static final boolean FEATURE_SUPPORTED =
            SystemProperties.getInt("ro.vendor.mtk_mobile_management", 0) == 1;

    private static final String MTK_OS_PKG = "com.mediatek";
    private static final String OS_PKG = "android";

    private static IPackageManager sPackageManager;
    private static IAppOpsService sAppOpsService;

    private static final String PROPERTY_NAME = "persist.vendor.sys.disable.moms" ;
    private static final String DISABLE_MOMS = "1" ;
    private static final String REENABLE_MOMS = "0" ;

    private static final String ENHANCE_LOG_PROPERTY_NAME = "vendor.cta.log.enable";
    private static final String ENABLE_ENHANCE_LOG = "1" ;

    static boolean isCtaSupported() {
        boolean isDisabled =
                SystemProperties.getInt("persist.vendor.sys.disable.moms", 0) == 1;
        if (isCtaEnhanceLogEnable()) {
            Log.v(TAG, "FEATURE_SUPPORTED= " + FEATURE_SUPPORTED + " isDisabled= " + isDisabled);
        }
        return FEATURE_SUPPORTED && !isDisabled;
    }

    static void setCtaSupported(boolean enable) {
        if (enable) {
            Log.d(TAG, "set persist.sys.mtk.disable.moms 0") ;
            SystemProperties.set(PROPERTY_NAME, REENABLE_MOMS);
        } else {
            Log.d(TAG, "set persist.sys.mtk.disable.moms 1") ;
            SystemProperties.set(PROPERTY_NAME, DISABLE_MOMS);
        }
    }

    static boolean isCtaOnlyPermission(String perm) {
        if (!isCtaSupported()) return false;
        return CtaPermissions.CTA_ONLY_PERMISSIONS.contains(perm);
    }

    static ArrayMap<String, String> getCtaPlatformPerms() {
        return CtaPermissions.CTA_PLATFORM_PERMISSIONS;
    }

    static boolean isCtaMonitoredPerms(String perm) {
        if (!isCtaSupported()) return false;
        return CtaPermissions.CTA_MONITOR_PERMISSIONS.contains(perm);
    }

    static boolean isPlatformPermissionGroup(String pkgName, String groupName) {
        Log.v(TAG, "isPlatformPermissionGroup  pkgName = " + pkgName + " groupName= " + groupName);
        if (OS_PKG.equals(pkgName)) {
            return true;
        }
        if (isCtaSupported()
                && MTK_OS_PKG.equals(pkgName)
                && isCtaAddedPermGroup(groupName)) {
            return true;
        }
        return false;
    }

    static String[] getCtaAddedPermissionGroups() {
        if (!isCtaSupported()) return null;
        return CtaPermissions.CTA_ADDED_PERMISSION_GROUPS.toArray(
                new String[CtaPermissions.CTA_ADDED_PERMISSION_GROUPS.size()]);
    }

    static boolean enforceCheckPermission(final String permission, final String action) {
        int uid = Binder.getCallingUid();
        int pid = Binder.getCallingPid();
        String callingPackage = getCallingPkgName(pid, uid);
        Log.d(TAG, "enforceCheckPermission callingPackage = " + callingPackage);
        return enforceCheckPermission(callingPackage, permission, action);
    }

    static boolean isPlatformPermission(String pkgName, String permName) {
        if (OS_PKG.equals(pkgName)) {
            return true;
        }
        if (isCtaSupported()
                && MTK_OS_PKG.equals(pkgName)
                && isCtaOnlyPermission(permName)) {
            return true;
        }
        return false;
    }

    static boolean isSystemApp(Context context ,String pkgName) {
        PackageManager pm = context.getPackageManager();
        boolean isSystemApp = false;
        int uid = Binder.getCallingUid();
        int userId = UserHandle.getUserId(uid);
        try {
            ApplicationInfo applicationInfo
                    = pm.getApplicationInfoAsUser(pkgName, 0 /* no flags */, userId);
            if ((applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0
                    || (applicationInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0) {
                isSystemApp = true;
            }
        } catch (PackageManager.NameNotFoundException e) {
            Log.d(TAG, "isSystemApp is false for NameNotFoundException= " + e);
        }
        if (isCtaEnhanceLogEnable()) {
            Log.d(TAG, "pkgName= " + pkgName + " uid= " + uid +
                       " userId= " + userId + " isSystemApp= " + isSystemApp);
        }
        return isSystemApp;
    }

    static String[] getCtaOnlyPermissions() {
        if (!isCtaSupported()) return null;
        return CtaPermissions.CTA_ONLY_PERMISSIONS.toArray(
                new String[CtaPermissions.CTA_ONLY_PERMISSIONS.size()]);
    }

    static boolean needGrantCtaRuntimePerm(boolean isUpdated, int targetSdkVersion) {
        return isCtaSupported() && !isUpdated && targetSdkVersion >= Build.VERSION_CODES.M;
    }

    static boolean isCtaEnhanceLogEnable() {
        return ENABLE_ENHANCE_LOG.equals(SystemProperties.get(ENHANCE_LOG_PROPERTY_NAME));
    }

    static boolean enforceCheckPermission(final String pkgName, final String permission,
            final String action) {
        if (!isCtaSupported()) return true;
        if (sPackageManager == null) {
            sPackageManager = AppGlobals.getPackageManager();
        }
        if (sAppOpsService == null) {
            sAppOpsService = IAppOpsService.Stub.asInterface(
                    ServiceManager.getService(Context.APP_OPS_SERVICE));
        }
        int uid = Binder.getCallingUid();
        int pid = Binder.getCallingPid();
        Log.d(TAG, "enforceCheckPermission uid = " + uid + ", pid = " + pid +
               ", pkgName = " + pkgName + ", permission = " + permission);
        if (TextUtils.isEmpty(pkgName)) {
            return true;
        }
        if (!isCtaOnlyPermission(permission)) {
            return true;
        }
        if (!isPackageRequestPermission(pkgName, permission, UserHandle.getUserId(uid))) {
            Log.d(TAG, "enforceCheckPermission(): pkg = " + pkgName + "does not request "
                    + "permission = " + permission + ", bypass the check");
            return true;
        }
        try {
            int appop = AppOpsManager.permissionToOpCode(permission);
            boolean pmsAllowed = sPackageManager.checkUidPermission(permission, uid) ==
                    PERMISSION_GRANTED;
            boolean appopsAllowed = sAppOpsService.noteOperation(appop, uid, pkgName) ==
                    AppOpsManager.MODE_ALLOWED;
            Log.d(TAG, "enforceCheckPermission pmsAllowed = " + pmsAllowed +
                    ", appopsAllowed = " + appopsAllowed);
            if (!pmsAllowed || !appopsAllowed) {
                throw new SecurityException("Permission Denial: " + action + " requires "
                        + permission);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "enforceCheckPermission RemoteException", e);
            return false;
        }
        return true;
    }

    public static boolean isCtaAddedPermGroup(String group) {
        if (!isCtaSupported()) return false;
        return CtaPermissions.CTA_ADDED_PERMISSION_GROUPS.contains(group);
    }

    public static String getCallingPkgName(int pid, int uid) {
        IActivityManager am = ActivityManagerNative.getDefault();
        List runningProcesses = null;
        try {
            runningProcesses = am.getRunningAppProcesses();
        } catch (RemoteException e) {
            return null;
        }
        Iterator iterator = runningProcesses.iterator();
        while (iterator.hasNext()) {
            RunningAppProcessInfo info = (RunningAppProcessInfo) iterator.next();
            try {
                if (info.uid == uid && info.pid == pid) {
                    return info.processName;
                }
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    public static String getCallingPkgName(int uid) {
        IActivityManager am = ActivityManagerNative.getDefault();
        List runningProcesses = null;
        try {
            runningProcesses = am.getRunningAppProcesses();
        } catch (RemoteException e) {
            return null;
        }
        Iterator iterator = runningProcesses.iterator();
        while (iterator.hasNext()) {
            RunningAppProcessInfo info = (RunningAppProcessInfo) iterator.next();
            try {
                if (info.uid == uid) {
                    return info.processName;
                }
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    private static boolean isPackageRequestPermission(final String pkgName, final String perm,
            final int userId) {
        try {
            PackageInfo pkgInfo = sPackageManager.getPackageInfo(pkgName,
                    PackageManager.GET_PERMISSIONS, userId);
            if (pkgInfo == null) {
                Log.w(TAG, "isPackageRequestPermission pkgInfo is null for pkg: " + pkgName);
                return false;
            }
            return Arrays.asList(pkgInfo.requestedPermissions).contains(perm);
        } catch (RemoteException e) {
            Log.w(TAG, "isPackageRequestPermission RemoteException", e);
            return false;
        }
    }

}
