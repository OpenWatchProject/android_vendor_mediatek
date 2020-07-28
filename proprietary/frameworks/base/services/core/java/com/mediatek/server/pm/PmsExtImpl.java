/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confINDEXial and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.server.pm;

import android.app.AppGlobals;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageInfoLite;
import android.content.pm.PackageManager;
import android.content.pm.PackageParser;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Environment;
import android.os.Process;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.text.TextUtils;
import android.util.ArrayMap;
import android.util.Slog;

import com.android.internal.util.ArrayUtils;
import com.android.server.pm.PackageManagerException;
import com.android.server.pm.PackageManagerService;
import com.android.server.pm.PackageSetting;
import com.android.server.pm.PackageSettingBase;
import com.android.server.pm.Settings;
import com.android.server.pm.UserManagerService;
import com.mediatek.powerhalwrapper.PowerHalWrapper;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlSerializer;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

public class PmsExtImpl extends PmsExt {
    static final String TAG = "PmsExtImpl";
    private static boolean sLogEnabled = false;
    private static final String SCAN_NAME_NO_DEX = "SCAN_NO_DEX";
    private static final int PARSE_IS_OPERATOR = PackageParser.PARSE_ENFORCE_CODE << 1;
    private static int sScanNoDex;
    private PackageManagerService mPms;
    private UserManagerService mUms;
    private ApplicationInfo mMediatekApplication = null;

    // Runtime Switchable Configuration support
    // Support overlay, framework, priv-app, app, plugin sub dir
    private static String sSysRscPath = SystemProperties.get(
            "ro.sys.current_rsc_path", "");
    private static String sVndRscPath = SystemProperties.get(
            "ro.vendor.vnd.current_rsc_path", "");
    private static String sProductRscPath = SystemProperties.get(
            "ro.product.current_rsc_path", "");
    private static final String SYS_RSC_PATH_CAP = "/system";
    private static final String VND_RSC_PATH_CAP = "/vendor";
    private static final String PRODUCT_RSC_PATH_CAP = "/product";
    private static File mAppLib32InstallDir;

    // Removable system app
    private static HashSet<String> sRemovableSystemAppSet = new HashSet<String>();
    // Define a bak removable sys app list, handle whitelist OTA upgade,
    // maybe one package is changed from removable to unremovable
    private static HashSet<String> sRemovableSystemAppSetBak = new HashSet<String>();
    private static boolean sRemovableSysAppEnabled = SystemProperties.getInt(
            "persist.vendor.pms_removable", 0) == 1;
    private static final File REMOVABLE_SYS_APP_LIST_SYSTEM = Environment
            .buildPath(Environment.getRootDirectory(), "etc", "permissions",
                    "pms_sysapp_removable_system_list.txt");
    private static final File REMOVABLE_SYS_APP_LIST_VENDOR = Environment
            .buildPath(Environment.getVendorDirectory(), "etc", "permissions",
                    "pms_sysapp_removable_vendor_list.txt");
    // Bak removable sys app file path definition
    private static final File REMOVABLE_SYS_APP_LIST_BAK = Environment
            .buildPath(Environment.getDataDirectory(), "system",
                    "pms_sysapp_removable_list_bak.txt");
    private static HashSet<String> sUninstallerAppSet = new HashSet<String>();

    // Support telephony add-on disable
    private static boolean sSkipScanAppEnabled = SystemProperties.getInt(
            "ro.vendor.mtk_telephony_add_on_policy", 0) == 1;
    private static HashSet<String> sSkipScanAppSet = new HashSet<String>();

    static final int SCAN_NO_DEX = 1<<0;
    static final int SCAN_AS_SYSTEM = 1<<17;
    static final int SCAN_AS_PRIVILEGED = 1<<18;
    static final int SCAN_AS_OEM = 1<<19;
    static final int SCAN_AS_VENDOR = 1<<20;
    static final int SCAN_AS_PRODUCT = 1<<21;

    private PowerHalWrapper mPowerHalWrapper = null;
    private static final String KEY_WORD1 = "benchmark";

    public PmsExtImpl() {
        super();
        //Can not bootup, first mark it disable.
        //mPowerHalWrapper = PowerHalWrapper.getInstance();
        sScanNoDex = ReflectionHelper.getIntValue(PackageManagerService.class,
                SCAN_NAME_NO_DEX);
        if (SYS_RSC_PATH_CAP.equals(sSysRscPath)) {
            sSysRscPath = "";
        }
        if (VND_RSC_PATH_CAP.equals(sVndRscPath)) {
            sVndRscPath = "";
        }
        if (PRODUCT_RSC_PATH_CAP.equals(sProductRscPath)) {
            sProductRscPath = "";
        }
        File dataDir = Environment.getDataDirectory();
        mAppLib32InstallDir = new File(dataDir, "app-lib");
    }

    @Override
    public void init(PackageManagerService pms, UserManagerService ums) {
        mPms = pms;
        mUms = ums;
    }

    @Override
    public void scanDirLI(int index, int defParseFlags, int defScanFlags,
            long currentTime) {
        File targetFile = null;
        int parseFlags = defParseFlags;
        int scanFlags = defScanFlags;
        switch (index) {
        case INDEX_CIP_FW:
            targetFile = new File("/custom/framework");
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | sScanNoDex
                          | SCAN_AS_SYSTEM,
                         currentTime);
            break;
        case INDEX_VENDOR_FW:
            targetFile = new File(Environment.getVendorDirectory(), "framework");
            try {
                targetFile = targetFile.getCanonicalFile();
            } catch (IOException e) {
                // failed to look up canonical path, continue with original one
            }
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | sScanNoDex
                          | SCAN_AS_SYSTEM,
                         currentTime);
            break;
        case INDEX_VENDOR_PRIV:
            targetFile = new File(Environment.getVendorDirectory(), "priv-app");
            try {
                targetFile = targetFile.getCanonicalFile();
            } catch (IOException e) {
                // failed to look up canonical path, continue with original one
            }
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM
                          | SCAN_AS_PRIVILEGED,
                         currentTime);
            break;
        case INDEX_OP_APP:
            targetFile = new File(Environment.getVendorDirectory(),
                    "/operator/app");
            try {
                targetFile = targetFile.getCanonicalFile();
            } catch (IOException e) {
                // failed to look up canonical path, continue with original one
            }
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PARSE_IS_OPERATOR,
                         defScanFlags,
                         currentTime);
            break;
        case INDEX_ROOT_PLUGIN:
            targetFile = new File(Environment.getRootDirectory(), "plugin");
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM,
                         currentTime);
            break;
        case INDEX_VENDOR_PLUGIN:
            targetFile = new File(Environment.getVendorDirectory(), "plugin");
            try {
                targetFile = targetFile.getCanonicalFile();
            } catch (IOException e) {
                // failed to look up canonical path, continue with original one
            }
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM,
                         currentTime);
            break;
        case INDEX_CUSTOM_APP:
            targetFile = new File("/custom/app");
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM,
                         currentTime);
            break;
        case INDEX_CUSTOM_PLUGIN:
            targetFile = new File("/custom/plugin");
            mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM,
                         currentTime);
            break;
        case INDEX_RSC_OVERLAY:
            if (!sSysRscPath.isEmpty()) {
                targetFile = new File(sSysRscPath, "overlay");
                parseFlags = defParseFlags | PackageParser.PARSE_IS_SYSTEM_DIR;
                scanFlags = defScanFlags | SCAN_AS_SYSTEM;
                mPms.scanDirTracedLI(targetFile, parseFlags, scanFlags, currentTime);
            }
            if (!sVndRscPath.isEmpty()) {
                targetFile = new File(sVndRscPath, "overlay");
                try {
                    targetFile = targetFile.getCanonicalFile();
                } catch (IOException e) {
                    // failed to look up canonical path, continue with original one
                }
                mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM
                          | SCAN_AS_VENDOR,
                         currentTime);
            }
            break;
        case INDEX_RSC_FW:
            if (!sSysRscPath.isEmpty()) {
                targetFile = new File(sSysRscPath, "framework");
                parseFlags = defParseFlags | PackageParser.PARSE_IS_SYSTEM_DIR;
                scanFlags = defScanFlags | SCAN_NO_DEX | SCAN_AS_SYSTEM | SCAN_AS_PRIVILEGED;
                mPms.scanDirTracedLI(targetFile, parseFlags, scanFlags, currentTime);
            }
            if (!sVndRscPath.isEmpty()) {
                targetFile = new File(sVndRscPath, "framework");
                try {
                    targetFile = targetFile.getCanonicalFile();
                } catch (IOException e) {
                    // failed to look up canonical path, continue with original one
                }
                mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_NO_DEX
                          | SCAN_AS_SYSTEM
                          | SCAN_AS_VENDOR
                          | SCAN_AS_PRIVILEGED,
                         currentTime);
            }
            break;
        case INDEX_RSC_PRIV:
            if (!sSysRscPath.isEmpty()) {
                targetFile = new File(sSysRscPath, "priv-app");
                parseFlags = defParseFlags | PackageParser.PARSE_IS_SYSTEM_DIR;
                scanFlags = defScanFlags | SCAN_AS_SYSTEM | SCAN_AS_PRIVILEGED;
                mPms.scanDirTracedLI(targetFile, parseFlags, scanFlags, currentTime);
            }
            if (!sVndRscPath.isEmpty()) {
                targetFile = new File(sVndRscPath, "priv-app");
                try {
                    targetFile = targetFile.getCanonicalFile();
                } catch (IOException e) {
                    // failed to look up canonical path, continue with original one
                }
                mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM
                          | SCAN_AS_VENDOR
                          | SCAN_AS_PRIVILEGED,
                         currentTime);
            }
            if (!sProductRscPath.isEmpty()) {
                targetFile = new File(sProductRscPath, "priv-app");
                try {
                    targetFile = targetFile.getCanonicalFile();
                } catch (IOException e) {
                    // failed to look up canonical path, continue with original one
                }
                mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                           | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                           | SCAN_AS_SYSTEM
                           | SCAN_AS_PRODUCT
                           | SCAN_AS_PRIVILEGED,
                         currentTime);
            }
            break;
        case INDEX_RSC_APP:
            if (!sSysRscPath.isEmpty()) {
                targetFile = new File(sSysRscPath, "app");
                parseFlags = defParseFlags | PackageParser.PARSE_IS_SYSTEM_DIR;
                scanFlags = defScanFlags | SCAN_AS_SYSTEM;
                mPms.scanDirTracedLI(targetFile, parseFlags, scanFlags, currentTime);
            }
            if (!sVndRscPath.isEmpty()) {
                targetFile = new File(sVndRscPath, "app");
                try {
                    targetFile = targetFile.getCanonicalFile();
                } catch (IOException e) {
                    // failed to look up canonical path, continue with original one
                }
                mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM
                          | SCAN_AS_VENDOR,
                         currentTime);
            }
            if (!sProductRscPath.isEmpty()) {
                targetFile = new File(sProductRscPath, "app");
                try {
                    targetFile = targetFile.getCanonicalFile();
                } catch (IOException e) {
                    // failed to look up canonical path, continue with original one
                }
                mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                           | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                           | SCAN_AS_SYSTEM
                           | SCAN_AS_PRODUCT,
                         currentTime);
            }
            break;
        case INDEX_RSC_PLUGIN:
            if (!sSysRscPath.isEmpty()) {
                targetFile = new File(sSysRscPath, "plugin");
                parseFlags = defParseFlags | PackageParser.PARSE_IS_SYSTEM_DIR;
                scanFlags = defScanFlags | SCAN_AS_SYSTEM;
                mPms.scanDirTracedLI(targetFile, parseFlags, scanFlags, currentTime);
            }
            if (!sVndRscPath.isEmpty()) {
                targetFile = new File(sVndRscPath, "plugin");
                try {
                    targetFile = targetFile.getCanonicalFile();
                } catch (IOException e) {
                    // failed to look up canonical path, continue with original one
                }
                mPms.scanDirTracedLI(targetFile,
                         defParseFlags
                          | PackageParser.PARSE_IS_SYSTEM_DIR,
                         defScanFlags
                          | SCAN_AS_SYSTEM
                          | SCAN_AS_VENDOR,
                         currentTime);
            }
            break;
        default:
            Slog.d(TAG, "Unknown index for ext:" + index);
            break;
        }
    }

    @Override
    public void scanMoreDirLi(int defParseFlags, int defScanFlags) {
        scanDirLI(INDEX_VENDOR_FW, defParseFlags, defScanFlags, 0);
        scanDirLI(INDEX_VENDOR_PLUGIN, defParseFlags, defScanFlags, 0);
        scanDirLI(INDEX_OP_APP, defParseFlags, defScanFlags, 0);
        scanDirLI(INDEX_RSC_PLUGIN, defParseFlags, defScanFlags, 0);
        scanDirLI(INDEX_ROOT_PLUGIN, defParseFlags, defScanFlags, 0);
        scanDirLI(INDEX_CIP_FW, defParseFlags, defScanFlags, 0);
        //scanDirLI(INDEX_RSC_PRIV, defParseFlags, defScanFlags, 0);
        //scanDirLI(INDEX_RSC_APP, defParseFlags, defScanFlags, 0);
        //scanDirLI(INDEX_RSC_PLUGIN, defParseFlags, defScanFlags, 0);
        carrierExpressInstall(defParseFlags, defScanFlags, 0);
    }

    @Override
    public void checkMtkResPkg(PackageParser.Package pkg)
            throws PackageManagerException {
        if (pkg.packageName.equals("com.mediatek")) {
            if (mMediatekApplication != null) {
                Slog.w(TAG, "Core mediatek package being redefined. Skipping.");
                throw new PackageManagerException(
                        PackageManager.INSTALL_FAILED_DUPLICATE_PACKAGE,
                        "Core android package being redefined. Skipping.");
            }
            mMediatekApplication = pkg.applicationInfo;
        }
    }

    @Override
    public boolean needSkipScanning(PackageParser.Package pkg,
            PackageSetting updatedPkg, PackageSetting ps) {
       // Support telephony add-on disable
       if (sSkipScanAppEnabled && sSkipScanAppSet.contains(pkg.packageName)) {
           Slog.d(TAG, "Skip scan package:" + pkg.packageName);
           return true;
       }
       if (!mPms.isFirstBoot() && (isRemovableSysApp(pkg.packageName)) &&
           (ps == null && updatedPkg == null)) {
           if (mPms.isDeviceUpgrading()) {
               if (!sRemovableSystemAppSetBak.contains(pkg.packageName)) {
                   Slog.d(TAG, "New added removable sys app by OTA:" + pkg.packageName);
                   return false;
               }
           }
           Slog.d(TAG, "Skip scanning uninstalled sys package " + pkg.packageName);
           return true;
       } else if (ps == null && updatedPkg != null) {
           Slog.d(TAG, "Skip scanning uninstalled package: " + pkg.packageName);
           return true;
       }

       return false;
    }

    @Override
    public boolean needSkipAppInfo(ApplicationInfo ai) {
        if (!sRemovableSysAppEnabled || ai == null) {
            return false;
        }

        boolean ret = false;
        if ((ai.flags & ApplicationInfo.FLAG_INSTALLED) == 0) {
            ret = isRemovableSysApp(ai.packageName);
        }
        return ret;
    }

    /**
     * Called after a new package added to check if the new package is an
     * unintaller app
     *
     * @param packageName
     *            Package name need to check
     * @param userId
     *            Which user that package installed for
     */
    @Override
    public void onPackageAdded(String packageName, PackageSetting pkgSetting, int userId) {
        // if the app is benchmark which we care, set its package name to HAL
        if (pkgSetting != null) {
            checkBenchmark(pkgSetting.getPackage());
        }
        updateUninstallerAppSetWithPkg(packageName, userId);
    }

    /**
     * Actions before PMS scan package start: 1. Build removable system app list
     */
    @Override
    public void initBeforeScan() {
        if (sLogEnabled)
            Slog.d(TAG, "initBeforeScan start");
        if (sRemovableSysAppEnabled) {
            buildRemovableSystemAppSet();
        }
        // Support telephony add-on disable
        if (sSkipScanAppEnabled) {
            buildSkipScanAppSet();
        }
        if (sLogEnabled)
            Slog.d(TAG, "initBeforeScan end");
    }

    /**
     * Actions after PMS scan package DONE: 1. Build uninstaller APP list 2.
     * Maintain whitelist bak 3. Handling whitelist changed after OTA
     */
    @Override
    public void initAfterScan(ArrayMap<String, PackageSetting> settingsPackages) {
        if (sRemovableSysAppEnabled) {
            if (sLogEnabled)
                Slog.d(TAG, "initAfterScan start");
            buildUninstallerAppSet();
            if (mPms.isFirstBoot() || mPms.isDeviceUpgrading()) {
                if (!sRemovableSystemAppSetBak.isEmpty()) {
                    if (onUpgradeRemovableSystemAppList(
                            sRemovableSystemAppSetBak, sRemovableSystemAppSet,
                            settingsPackages)) {
                        sWriteRemovableSystemAppToFile(sRemovableSystemAppSet,
                                REMOVABLE_SYS_APP_LIST_BAK);
                    }
                } else {
                    sWriteRemovableSystemAppToFile(sRemovableSystemAppSet,
                            REMOVABLE_SYS_APP_LIST_BAK);
                }
            }
            if (sLogEnabled)
                Slog.d(TAG, "initAfterScan end");
        }
    }

  @Override
    public int customizeInstallPkgFlags(int installFlags,
            PackageInfoLite pkgLite,
            ArrayMap<String, PackageSetting> settingsPackages, UserHandle user) {
        int ret = installFlags;
        PackageSetting ps = settingsPackages.get(pkgLite.packageName);
        if (ps != null) {
            if (isRemovableSysApp(pkgLite.packageName)) {
                int[] installedUsers = ps.queryInstalledUsers(
                        mUms.getUserIds(), true);
                if (sLogEnabled) {
                    Slog.d(TAG, "getUser()=" + user + " installedUsers="
                            + Arrays.toString(installedUsers));
                }
                // UserHandle.CURRENT ?
                if (user == UserHandle.ALL
                        || !ArrayUtils.contains(installedUsers, user
                                .getIdentifier())) {
                    if (installedUsers != null
                            && installedUsers.length != mUms.getUserIds().length) {
                        Slog
                                .d(TAG,
                                        "built in app, set replace and allow downgrade");
                        // If removable sys app has uninstalled by some users,
                        // or
                        // reinstall for all users, force to install, like add
                        // parameters "-r -d"
                        ret |= PackageManager.INSTALL_ALLOW_DOWNGRADE;
                        ret |= PackageManager.INSTALL_REPLACE_EXISTING;
                    }
                }
            }
        }
        return ret;
    }

    @Override
    public void updatePackageSettings(int userId, String pkgName,
            PackageParser.Package newPackage, PackageSetting ps,
            int[] allUsers, String installerPackageName) {
        if (userId == UserHandle.USER_ALL) {
            if (isRemovableSysApp(pkgName)
                    && (newPackage.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) {
                for (int currentUserId : allUsers) {
                    ps.setInstalled(true, currentUserId);
                    ps.setEnabled(
                            PackageManager.COMPONENT_ENABLED_STATE_DEFAULT,
                            currentUserId, installerPackageName);
                }
            }
        }
    }

    @Override
    public int customizeDeletePkgFlags(int deleteFlags, String packageName) {
        int newDelFlags = deleteFlags;
        if (isRemovableSysApp(packageName)) {
            // Add flag to allow uninstall system app (disable sys app)
            newDelFlags |= PackageManager.DELETE_SYSTEM_APP;
        }
        return newDelFlags;
    }

    @Override
    public int customizeDeletePkg(int[] users, String packageName,
            int versionCode, int delFlags) {
        int returnCode = PackageManager.DELETE_SUCCEEDED;
        int userFlags = delFlags & ~PackageManager.DELETE_ALL_USERS;
        for (int userId : users) {
            returnCode = mPms.deletePackageX(packageName, versionCode, userId,
                    userFlags);
            if (returnCode != PackageManager.DELETE_SUCCEEDED) {
                Slog.w(TAG, "Package delete failed for user " + userId
                        + ", returnCode " + returnCode);
            }
        }
        return returnCode;
    }

    @Override
    public boolean dumpCmdHandle(String cmd, PrintWriter pw, String[] args,
            int opti) {
        boolean ret = true;
        if ("log".equals(cmd)) {
            configLogTag(pw, args, opti);
        } else if ("removable".equals(cmd)) {
            dumpRemovableSysApps(pw, args, opti);
        } else {
            ret = super.dumpCmdHandle(cmd, pw, args, opti);
        }
        return ret;
    }

    /**
     * Update ApplicationInfo if needed: If calling UID is in uninstaller list
     * and get removable system app info, remove SYSTEM flag
     *
     * @param oldAppInfo
     *            Old appliatin info before handling
     * @return New application info after remove SYSTEM flag if needed
     */
    @Override
    public ApplicationInfo updateApplicationInfoForRemovable(
            ApplicationInfo oldAppInfo) {
        if (!sRemovableSysAppEnabled || oldAppInfo == null) {
            return oldAppInfo;
        }
        return updateApplicationInfoForRemovable(mPms.getNameForUid(Binder
                .getCallingUid()), oldAppInfo);
    }

    /**
     * Update ApplicationInfo if needed: If calling UID is in uninstaller list
     * and get removable system app info, remove SYSTEM flag
     *
     * @param nameForUid
     *            Caller app name by calling UID
     * @param oldAppInfo
     *            Old appliatin info before handling
     * @return New application info after remove SYSTEM flag if needed
     */
    @Override
    public ApplicationInfo updateApplicationInfoForRemovable(String nameForUid,
            ApplicationInfo oldAppInfo) {
        if (!sRemovableSysAppEnabled || oldAppInfo == null) {
            return oldAppInfo;
        }
        boolean clearSystemFlag = false;
        ApplicationInfo newAppInfo = null;
        String packageName = oldAppInfo.packageName;
        if (Binder.getCallingPid() != Process.myPid()
                && isRemovableSysApp(packageName)) {
            String name = nameForUid;
            if (name != null) {
                String[] strs = name.split(":");
                if (strs.length == 1) {
                    clearSystemFlag = isUninstallerApp(strs[0]);
                } else if (strs.length > 1) {
                    // Shared system app, like Settings
                    clearSystemFlag = strs[1].equals(Process.SYSTEM_UID + "");
                    if (!clearSystemFlag) {
                        try {
                            String[] pkgs = AppGlobals
                                    .getPackageManager()
                                    .getPackagesForUid(Integer.valueOf(strs[1]));
                            for (String pkg : pkgs) {
                                clearSystemFlag = isUninstallerApp(pkg);
                                if (clearSystemFlag) {
                                    if (sLogEnabled)
                                        Slog.d(TAG, "shared uid=" + strs[1] + " pkg=" + pkg);
                                    break;
                                }
                            }
                        } catch (RemoteException e) {}
                    }
                }
                if (sLogEnabled)
                    Slog.d(TAG, "judge for " + packageName + " name=" + name
                            + " clear ? " + clearSystemFlag);
            }
        }
        if (clearSystemFlag && oldAppInfo != null) {
            newAppInfo = new ApplicationInfo(oldAppInfo);
            newAppInfo.flags &= ~(ApplicationInfo.FLAG_SYSTEM
                    | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP);
            return newAppInfo;
        }
        return oldAppInfo;
    }

    @Override
    public ActivityInfo updateActivityInfoForRemovable(ActivityInfo info)
            throws RemoteException {
        if (info != null) {
            info.applicationInfo = updateApplicationInfoForRemovable(AppGlobals
                    .getPackageManager().getNameForUid(Binder.getCallingUid()),
                    info.applicationInfo);
        }
        return info;
    }

    @Override
    public List<ResolveInfo> updateResolveInfoListForRemovable(
            List<ResolveInfo> apps) throws RemoteException {
        if (apps != null) {
            Iterator<ResolveInfo> it = apps.iterator();
            ResolveInfo info;
            while (it.hasNext()) {
                info = it.next();
                info.activityInfo.applicationInfo = updateApplicationInfoForRemovable(
                        AppGlobals.getPackageManager().getNameForUid(
                                Binder.getCallingUid()),
                        info.activityInfo.applicationInfo);
            }
        }
        return apps;
    }

    /**
     * Update PackageInfo if needed
     *
     * @param oldPkgInfo
     *            Old package info before handling
     * @return New application info after remove SYSTEM flag if needed
     */
    @Override
    public PackageInfo updatePackageInfoForRemovable(PackageInfo oldPkgInfo) {
        if (!sRemovableSysAppEnabled || oldPkgInfo == null) {
            return oldPkgInfo;
        }
        oldPkgInfo.applicationInfo = updateApplicationInfoForRemovable(oldPkgInfo.applicationInfo);
        return oldPkgInfo;
    }


    /**
     * Check package name is a removable system app or not
     *
     * @param pkgName
     *            Packgae name need to check
     * @return True for a removable system app
     */
    @Override
    public boolean isRemovableSysApp(String pkgName) {
        boolean ret = false;
        if (sRemovableSysAppEnabled) {
            ret = sRemovableSystemAppSet.contains(pkgName);
        }
        return ret;
    }

    @Override
    public boolean updateNativeLibDir(ApplicationInfo info, String codePath) {
        if (codePath != null && codePath.contains("vendor/operator/app")) {
            final String apkName =
                  PackageManagerService.deriveCodePathName(codePath);
            info.nativeLibraryRootDir =
                  new File(mAppLib32InstallDir, apkName).getAbsolutePath();
            info.nativeLibraryRootRequiresIsa = false;
            info.nativeLibraryDir = info.nativeLibraryRootDir;
            return true;
        }
        return false;
    }

    private void configLogTag(PrintWriter pw, String[] args, int opti) {
        String tag = null;
        boolean on = false;
        if ((opti + 1) >= args.length) {
            pw.println("  Invalid argument!");
        } else {
            tag = args[opti];
            on = "on".equals(args[opti + 1]);
            if ("a".equals(tag)) {
                PackageManagerService.DEBUG_SETTINGS = on;
                PackageManagerService.DEBUG_PREFERRED = on;
                PackageManagerService.DEBUG_UPGRADE = on;
                PackageManagerService.DEBUG_DOMAIN_VERIFICATION = on;
                PackageManagerService.DEBUG_BACKUP = on;
                PackageManagerService.DEBUG_INSTALL = on;
                PackageManagerService.DEBUG_REMOVE = on;
                PackageManagerService.DEBUG_BROADCASTS = on;
                PackageManagerService.DEBUG_PACKAGE_INFO = on;
                PackageManagerService.DEBUG_INTENT_MATCHING = on;
                PackageManagerService.DEBUG_PACKAGE_SCANNING = on;
                PackageManagerService.DEBUG_VERIFY = on;
                PackageManagerService.DEBUG_PERMISSIONS = on;
                PackageManagerService.DEBUG_SHARED_LIBRARIES = on;
                PackageManagerService.DEBUG_DEXOPT = on;
                PackageManagerService.DEBUG_ABI_SELECTION = on;
                PackageManagerService.DEBUG_INSTANT = on;
                PackageManagerService.DEBUG_APP_DATA = on;
            }
        }
    }

    private void carrierExpressInstall(int defParseFlags, int defScanFlags,
            long currentTime) {
        boolean isCarrierExpressInstallEnabled = "1".equals(SystemProperties
                .get("ro.vendor.mtk_carrierexpress_inst_sup"));
        if (!isCarrierExpressInstallEnabled) {
            // Load custom packages
            scanDirLI(INDEX_CUSTOM_APP, defParseFlags, defScanFlags,
                    currentTime);
            scanDirLI(INDEX_CUSTOM_PLUGIN, defParseFlags, defScanFlags,
                    currentTime);
        } else {
            // Load custom operator packages
            scanOperatorDirLI(defScanFlags);
        }
    }

    private void scanOperatorDirLI(int scanFlags) {
        String opStr = SystemProperties.get("persist.vendor.operator.optr");
        if (opStr == null || opStr.length() <= 0) {
            Slog.d(TAG, "No operater defined.");
        } else {
            String opFileName = "usp-apks-path" + "-" + opStr + ".txt";
            final File customUniDir = new File("/custom/usp");
            if (customUniDir.exists()) {
                scanCxpApp(customUniDir, opFileName, scanFlags);
            } else {
                final File systemUniDir = new File("/system/usp");
                if (systemUniDir.exists()) {
                    scanCxpApp(systemUniDir, opFileName, scanFlags);
                } else {
                    Slog.d(TAG, "No Carrier Express Pack directory.");
                }
            }
        }
    }

    private void scanCxpApp(File uniPath, String opFileName, int scanFlags) {
        File opFilePath = new File(uniPath, opFileName);
        List<String> appPathList = readPathsFromFile(opFilePath);
        long startScanTime = 0;
        long endScanTime = 0;
        for (int i = 0; i < appPathList.size(); i++) {
            String path = appPathList.get(i);
            File file = new File(path);
            int flag = 0;

            if (path.contains("removable")) {
                flag = PARSE_IS_OPERATOR;
            } else {
                flag = SCAN_AS_SYSTEM
                        | PackageParser.PARSE_IS_SYSTEM_DIR;
            }

            startScanTime = SystemClock.uptimeMillis();
            Slog.d(TAG, "scan package: " + file.toString() + " , start at: "
                    + startScanTime + "ms.");

            try {
                mPms.scanPackageTracedLI(file, flag
                        | PackageParser.PARSE_MUST_BE_APK, scanFlags, 0, null);
            } catch (PackageManagerException e) {
                Slog.w(TAG, "Failed to parse " + file + ": " + e.getMessage());
            }

            endScanTime = SystemClock.uptimeMillis();
            Slog.d(TAG, "scan package: " + file.toString() + " , end at: "
                    + endScanTime + "ms. elapsed time = "
                    + (endScanTime - startScanTime) + "ms.");
        }
    }

    private List<String> readPathsFromFile(File packagePathsFile) {
        int length = (int) packagePathsFile.length();
        byte[] bytes = new byte[length];
        List<String> fileContents = new ArrayList<String>();
        try {
            FileInputStream inputStream = new FileInputStream(packagePathsFile);
            if (inputStream != null) {
                java.io.InputStreamReader inputStreamReader = new java.io.InputStreamReader(
                        inputStream);
                BufferedReader bufferedReader = new BufferedReader(
                        inputStreamReader);
                String receiveString = "";
                while ((receiveString = bufferedReader.readLine()) != null) {
                    fileContents.add(receiveString);
                }
                inputStream.close();
            }
        } catch (java.io.FileNotFoundException e) {
            Slog.d(TAG, "File not found: " + e.toString());
        } catch (IOException e) {
            Slog.d(TAG, "Can not read file: " + e.toString());
        }
        return fileContents;
    }

    private void dumpRemovableSysApps(PrintWriter pw, String[] args, int opti) {
        pw.println(" sRemovableSysAppEnabled: " + sRemovableSysAppEnabled);
        Iterator<String> it = sRemovableSystemAppSet.iterator();
        pw.println(" sRemovableSystemAppSet:");
        while (it.hasNext()) {
            pw.println("  " + it.next());
        }
        it = sUninstallerAppSet.iterator();
        pw.println(" sUninstallerAppSet:");
        while (it.hasNext()) {
            pw.println("  " + it.next());
        }
    }

    private void buildRemovableSystemAppSet() {
        if (sRemovableSysAppEnabled) {
            if (sLogEnabled)
                Slog.d(TAG, "BuildRemovableSystemAppSet start");
            sGetRemovableSystemAppFromFile(sRemovableSystemAppSet,
                    REMOVABLE_SYS_APP_LIST_SYSTEM);
            sGetRemovableSystemAppFromFile(sRemovableSystemAppSet,
                    REMOVABLE_SYS_APP_LIST_VENDOR);
            sGetRemovableSystemAppFromFile(sRemovableSystemAppSetBak,
                    REMOVABLE_SYS_APP_LIST_BAK);
            if (sLogEnabled)
                Slog.d(TAG, "BuildRemovableSystemAppSet end");
        }
    }

    /**
     * Build uninstaller app list. Called after scan APPs complete
     */
    private void buildUninstallerAppSet() {
        if (sRemovableSysAppEnabled) {
            if (sLogEnabled)
                Slog.d(TAG, "buildUninstallerAppSet start");
            final int[] allUserIds = mUms.getUserIds();
            for (int i = 0; i < allUserIds.length; i++) {
                // Setting APPs
                Intent settingIntent = new Intent(
                        android.provider.Settings.ACTION_SETTINGS);
                settingIntent.addCategory(Intent.CATEGORY_DEFAULT);
                getAppSetByIntent(sUninstallerAppSet, settingIntent,
                        allUserIds[i]);
                // Launcher APPs
                Intent launcherIntent = new Intent(Intent.ACTION_MAIN);
                launcherIntent.addCategory(Intent.CATEGORY_HOME);
                launcherIntent.addCategory(Intent.CATEGORY_DEFAULT);
                getAppSetByIntent(sUninstallerAppSet, launcherIntent,
                        allUserIds[i]);
                // Store APPs
                Intent storeIntent = new Intent(Intent.ACTION_MAIN);
                storeIntent.addCategory(Intent.CATEGORY_APP_MARKET);
                storeIntent.addCategory(Intent.CATEGORY_DEFAULT);
                getAppSetByIntent(sUninstallerAppSet, storeIntent,
                        allUserIds[i]);
                // Installer APPs
                Intent installIntent = new Intent(Intent.ACTION_INSTALL_PACKAGE);
                installIntent.addCategory(Intent.CATEGORY_DEFAULT);
                installIntent
                        .setData(Uri.fromParts("package", "foo.bar", null));
                Intent uninstallIntent = new Intent(
                        Intent.ACTION_UNINSTALL_PACKAGE);
                uninstallIntent.addCategory(Intent.CATEGORY_DEFAULT);
                uninstallIntent.setData(Uri.fromParts("package", "foo.bar",
                        null));
                getAppSetByIntent(sUninstallerAppSet, installIntent,
                        allUserIds[i]);
                getAppSetByIntent(sUninstallerAppSet, uninstallIntent,
                        allUserIds[i]);
                // ADB, will not be blocked by SYSTEM flag
                if (sLogEnabled)
                    Slog.d(TAG, "buildUninstallerAppSet end");
            }
        }
    }

    /**
     * Update uninstaller app list with package name
     *
     * @param pkgName
     *            The package name need to check is uninstaller app or not
     * @param userId
     *            The user ID which package is installed
     */
    private void updateUninstallerAppSetWithPkg(String pkgName, int userId) {
        if (sRemovableSysAppEnabled && pkgName != null) {
            if (sUninstallerAppSet.contains(pkgName)) {
                Slog.d(TAG, "already in set:" + pkgName);
                return;
            }
            if (sLogEnabled) {
                Slog.d(TAG, "updateUninstallerAppSetWithPkg for:" + pkgName
                        + " with:" + userId);
            }
            // Launcher APPs
            Intent launcherIntent = new Intent(Intent.ACTION_MAIN);
            launcherIntent.addCategory(Intent.CATEGORY_HOME);
            launcherIntent.addCategory(Intent.CATEGORY_DEFAULT);
            launcherIntent.setPackage(pkgName);
            getAppSetByIntent(sUninstallerAppSet, launcherIntent, userId);
            // Store APPs
            Intent storeIntent = new Intent(Intent.ACTION_MAIN);
            storeIntent.addCategory(Intent.CATEGORY_APP_MARKET);
            storeIntent.setPackage(pkgName);
            getAppSetByIntent(sUninstallerAppSet, storeIntent, userId);
            if (sLogEnabled)
                Slog.d(TAG, "updateUninstallerAppSetWithPkg end");
        }
    }

    /**
     * Get removable system app list from config file
     *
     * @param resultSet
     *            Returned result list
     * @param file
     *            The config file
     */
    private static void sGetRemovableSystemAppFromFile(
            HashSet<String> resultSet, File file) {
        FileReader fr = null;
        BufferedReader br = null;
        try {
            if (file.exists()) {
                fr = new FileReader(file);
            } else {
                Slog.d(TAG, "file in " + file + " does not exist!");
                return;
            }
            br = new BufferedReader(fr);
            String line;
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (!TextUtils.isEmpty(line)) {
                    if (sLogEnabled)
                        Slog.d(TAG, "read line " + line);
                    resultSet.add(line);
                }
            }
        } catch (IOException io) {
            Slog.d(TAG, io.getMessage());
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
                if (fr != null) {
                    fr.close();
                }
            } catch (IOException io) {
                Slog.d(TAG, io.getMessage());
            }
        }
    }

    /**
     * Write a string list to file in storage
     *
     * @param resultSet
     *            The string list that need to write
     * @param file
     *            The output file in storage
     */
    private static void sWriteRemovableSystemAppToFile(
            HashSet<String> resultSet, File file) {
        FileWriter fw = null;
        BufferedWriter bw = null;
        try {
            if (file.exists()) {
                file.delete();
            }
            fw = new FileWriter(file, false);
            bw = new BufferedWriter(fw);
            if (resultSet == null || resultSet.isEmpty()) {
                bw.write("");
                bw.flush();
                return;
            }
            Iterator<String> it = resultSet.iterator();
            while (it.hasNext()) {
                String next = it.next();
                bw.write(next);
                bw.newLine();
            }
            bw.flush();
        } catch (IOException io) {
            Slog.d(TAG, io.getMessage());
        } finally {
            try {
                if (bw != null) {
                    bw.close();
                }
                if (fw != null) {
                    fw.close();
                }
            } catch (IOException io) {
                Slog.d(TAG, io.getMessage());
            }
        }
    }

    /**
     * Check pakage name is an uninstaller app or not
     *
     * @param pkgName
     *            Package name need to check
     * @return True for an uninstaller app
     */
    private static boolean isUninstallerApp(String pkgName) {
        boolean ret = false;
        if (sRemovableSysAppEnabled) {
            ret = sUninstallerAppSet.contains(pkgName);
        }
        return ret;
    }

    /**
     * Query APP list by specified intent
     *
     * @param resultSet
     *            Returned result list
     * @param targetIntent
     *            Specified intent by query
     * @param userId
     *            User ID by query
     */
    private void getAppSetByIntent(HashSet<String> resultSet,
            Intent targetIntent, int userId) {
        final List<ResolveInfo> matches = mPms.queryIntentActivitiesInternal(
                targetIntent, null, PackageManager.MATCH_DIRECT_BOOT_AWARE
                        | PackageManager.MATCH_DIRECT_BOOT_UNAWARE
                        | PackageManager.MATCH_DISABLED_COMPONENTS, userId);
        int size = matches.size();
        if (sLogEnabled)
            Slog.d(TAG, "getAppSetByIntent:" + targetIntent + " size=" + size);
        if (size >= 1) {
            for (int i = 0; i < size; i++) {
                resultSet.add(matches.get(i).getComponentInfo().packageName);
            }
        }
    }

    /**
     * Handling whitelist changed after OTA
     *
     * @param oldSet
     *            Whitelist before OTA
     * @param newSet
     *            Whitelist after OTA
     * @return Whitelist is changed or not
     */
    private boolean onUpgradeRemovableSystemAppList(HashSet<String> oldSet,
            HashSet<String> newSet,
            ArrayMap<String, PackageSetting> settingsPackages) {
        HashSet<String> added = new HashSet<String>();
        HashSet<String> removed = new HashSet<String>();
        added.addAll(newSet);
        added.removeAll(oldSet);
        removed.addAll(oldSet);
        removed.removeAll(newSet);
        if (sLogEnabled)
            Slog.d(TAG, "onUpgradeRemovableSystemAppList: add=" + added.size()
                    + " removed=" + removed.size());
        int[] allUserIds = mUms.getUserIds();
        Iterator<String> it = removed.iterator();
        boolean updated = false;
        while (it.hasNext()) {
            String removedPkg = it.next();
            PackageSetting ps = settingsPackages.get(removedPkg);
            if (ps != null) {
                int[] uninstalledUsers = ps.queryInstalledUsers(allUserIds,
                        false);
                if (uninstalledUsers.length > 0) {
                    for (int i = 0; i < uninstalledUsers.length; i++) {
                        ps.setInstalled(true, uninstalledUsers[i]);
                        ps.setEnabled(
                                PackageManager.COMPONENT_ENABLED_STATE_DEFAULT,
                                uninstalledUsers[i], "android");
                        updated = true;
                    }
                }
            }
        }
        if (updated) {
            mPms.scheduleWriteSettingsLocked();
        }
        return (removed.size() > 0 || added.size() > 0);
    }

    public void checkBenchmark(PackageParser.Package pkg) {
        boolean isNeedAdd = false;
        String pkgName = pkg.packageName;
        //judge package name
        if (pkgName.contains(KEY_WORD1)) {
            Slog.d(TAG, "care package name is " + pkg.packageName);
            isNeedAdd = true;
        }
        //judge request permission
        if (!isNeedAdd) {
           for (String requestedPermission : pkg.requestedPermissions) {
               if (requestedPermission.contains(KEY_WORD1)) {
                   Slog.d(TAG, pkgName + " requestedPermission = "
                        + requestedPermission);
                  isNeedAdd = true;
                break;
               }
           }
        }
        //judge activity className
        if (!isNeedAdd) {
            for (PackageParser.Activity activity : pkg.activities) {
                 String className = activity.className;
                if (className.contains(KEY_WORD1)) {
                    Slog.d(TAG, pkgName + " ActivityClassName = "
                        + className);
                   isNeedAdd = true;
                   break;
                }
            }
        }
        //judge receiver intent's category
        if (!isNeedAdd) {
            for (PackageParser.Activity receiver : pkg.receivers) {
                for (PackageParser.ActivityIntentInfo intent : receiver.intents) {
                    int count = intent.countCategories();
                    for (int i = 0; i < count; i++) {
                        String category = intent.getCategory(i);
                        if (category.contains(KEY_WORD1)) {
                            Slog.d(TAG,
                                    "care package name is " + pkgName + " category =" + category);
                            isNeedAdd = true;
                            break;
                        }
                    }
                }
            }
        }
        // if the app is benchmark which we care, set its package name to HAL
        if (isNeedAdd) {
           if (mPowerHalWrapper != null) {
               Slog.d(TAG, "setSportsApk " + pkgName);
               //mPowerHalWrapper.setSportsApk(pkgName);
           }
        }
    }

    // Support telephony add-on disable
    private void buildSkipScanAppSet() {
        File targetFile = null;
        String fileNameStr = SystemProperties.get("ro.vendor.mtk_telephon_add_on_pkg_file");
        if (sSkipScanAppEnabled) {
            if (sLogEnabled)
                Slog.d(TAG, "BuildSkipScanAppSet start");
            targetFile = new File(sSysRscPath, fileNameStr);
            sGetRemovableSystemAppFromFile(sSkipScanAppSet,
                    targetFile);
            if (sLogEnabled)
                Slog.d(TAG, "BuildSkipScanAppSet end");
        }
    }
}

class ReflectionHelper {

    public static Class getNonPublicInnerClass(Class targetCls,
            String innerClsName) {
        Class innerCls = null;
        Class[] innerClasses = targetCls.getDeclaredClasses();
        for (Class cls : innerClasses) {
            if (cls.toString().contains(innerClsName)) {
                innerCls = cls;
                break;
            }
        }
        return innerCls;
    }

    public static Field getNonPublicField(Class cls, String fieldName) {
        Field field = null;
        try {
            field = cls.getDeclaredField(fieldName);
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        }
        return field;
    }

    public static Object getObjectValue(Field field, Object targetObject) {
        field.setAccessible(true);
        Object result = null;
        try {
            result = field.get(targetObject);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return result;
    }

    public static boolean getBooleanValue(Field field, Object tarObject) {
        field.setAccessible(true);
        boolean result = false;
        try {
            result = field.getBoolean(tarObject);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return result;
    }

    public static int getIntValue(Field field, Object tarObject) {
        field.setAccessible(true);
        int result = 0;
        try {
            result = field.getInt(tarObject);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return result;
    }

    public static Object getObjectValue(Class cls, String fieldName,
            Object targetObject) {
        Field field = getNonPublicField(cls, fieldName);
        return getObjectValue(field, targetObject);
    }

    public static boolean getBooleanValue(Class cls, String fieldName,
            Object tarObject) {
        Field field = getNonPublicField(cls, fieldName);
        return getBooleanValue(field, tarObject);
    }

    public static int getIntValue(Class cls, String fieldName) {
        Field field = getNonPublicField(cls, fieldName);
        return getIntValue(field, cls);
    }

    public static Method getMethod(Class cls, String methodName,
            Class... params) {
        Method retMethod = null;
        try {
            retMethod = cls.getDeclaredMethod(methodName, params);
            if (retMethod != null) {
                retMethod.setAccessible(true);
            }
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
        return retMethod;
    }

    public static Object callMethod(Method method, Object object,
            Object... params) {
        Object ret = null;
        if (method != null) {
            try {
                ret = method.invoke(object, params);
                Slog.d(PmsExtImpl.TAG, "callMethod:" + method.getName()
                        + " ret=" + ret);
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        }
        return ret;
    }

    public static void setFieldValue(Class cls, Object obj, String fieldName,
            Object value) {
        Field field = null;
        try {
            field = cls.getDeclaredField(fieldName);
            field.setAccessible(true);
            field.set(obj, value);
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }

    public static void setFieldValue(Object obj, String fieldName, Object value) {
        setFieldValue(obj.getClass(), obj, fieldName, value);
    }
}
