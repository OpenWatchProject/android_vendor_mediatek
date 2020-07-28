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
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class DatabaseManager {
    private static final String TAG = "DatabaseManager";

    private static DatabaseManager sManager;
    private DatabaseHelper mDataBaseHelper;
    private static Object sLock = new Object();

    private Context mContext;
    private List<String> mPlatformWhitelist;
    private List<String> mInvisibleAppWhitelist;
    private List<String> mDefaultAppWhitelist;
    private List<String> mAppWhitelist = new ArrayList<String>();
    private List<AppRecord> mAppRecordsCache;

    public static DatabaseManager getInstance(Context context) {
        sManager = new DatabaseManager(context);
        return sManager;
    }

    public DatabaseManager(Context context) {
        Log.d(TAG, "init start");
        mContext = context.getApplicationContext();
        try {
            mPlatformWhitelist = Utils.getDuraSpeedManager().getPlatformWhitelist();
        } catch (Exception e) {
            e.printStackTrace();
        }
        mInvisibleAppWhitelist =
                Arrays.asList(mContext.getResources().getStringArray(R.array.app_hidelist));
        mDefaultAppWhitelist =
                Arrays.asList(mContext.getResources().getStringArray(R.array.app_whitelist));

        initDataBase();
        updateDatabase();
        Log.d(TAG, "init end");
    }

    private void initDataBase() {
        synchronized (sLock) {
            if (mDataBaseHelper == null) {
                mDataBaseHelper = new DatabaseHelper(mContext);
                mAppRecordsCache = mDataBaseHelper.initDataCache();
            }
        }
    }

    /**
     * Update database for some app maybe installed.
     */
    public void updateDatabase() {
        // Get all the installed app list, exclude white list
        PackageManager pm = Utils.getPackageManager(mContext);
        int flags = PackageManager.GET_UNINSTALLED_PACKAGES |
                PackageManager.GET_DISABLED_COMPONENTS;
        List<ApplicationInfo> originalAppList = pm.getInstalledApplications(flags);
        for (String data : mInvisibleAppWhitelist) {
            Log.i(TAG, "hide list = " + data);
        }

        Intent launchIntent = new Intent(Intent.ACTION_MAIN, null)
                .addCategory(Intent.CATEGORY_LAUNCHER);
        List<ResolveInfo> intents = pm.queryIntentActivities(
                launchIntent, PackageManager.GET_DISABLED_COMPONENTS);
        synchronized (sLock) {
            // Insert the apps installed before duraspeed app launched.
            List<String> originalPkgList = new ArrayList<String>();
            for (ApplicationInfo appInfo : originalAppList) {
                String pkgName = appInfo.packageName;
                originalPkgList.add(pkgName);
                if (!Utils.isSystemApp(appInfo) &&
                        Utils.hasLauncherEntry(pkgName, intents)) {
                    if (mInvisibleAppWhitelist.contains(pkgName)) {
                        continue;
                    }
                    if (!pkgExistsInCache(pkgName)) {
                        insert(pkgName);
                    }
                }
            }

            // Delete the apps uninstalled before duraspeed app launched.
            Iterator<AppRecord> iter = mAppRecordsCache.iterator();
            while (iter.hasNext()) {
                String packageName = iter.next().getPkgName();
                if (!originalPkgList.contains(packageName)) {
                    iter.remove();
                    mDataBaseHelper.delete(packageName);
                }
            }
            originalPkgList.clear();
        }
        originalAppList.clear();
        intents.clear();
    }

    private boolean pkgExistsInCache(String pkgName) {
        boolean existed = false;
        for (AppRecord appRecord : mAppRecordsCache) {
            if (appRecord.getPkgName().equals(pkgName)) {
                existed = true;
            }
        }
        return existed;
    }

    private boolean isInvisibleWhitelist(String pkgName) {
        return (mInvisibleAppWhitelist.contains(pkgName)
                || (mPlatformWhitelist != null && mPlatformWhitelist.contains(pkgName)));
    }

    public List<AppRecord> getAppRecords() {
        synchronized (sLock) {
            return mAppRecordsCache;
        }
    }

    public List<String> getAppWhitelist() {
        if (mAppWhitelist != null) {
            mAppWhitelist.clear();
        }
        synchronized (sLock) {
            for (AppRecord appRecord : mAppRecordsCache) {
                if (appRecord.getStatus() == AppRecord.STATUS_ENABLED) {
                    mAppWhitelist.add(appRecord.getPkgName());
                }
            }
        }
        mAppWhitelist.addAll(mInvisibleAppWhitelist);
        return mAppWhitelist;
    }

    public boolean modify(String pkgName, int status) {
        synchronized (sLock) {
            Log.d(TAG, "modify package: " + pkgName + ", to status: " + status);
            modifyCache(pkgName, status);
            mDataBaseHelper.update(pkgName, status);
        }
        return true;
    }

    public boolean delete(String pkgName) {
        if (isInvisibleWhitelist(pkgName)) {
            return false;
        }
        synchronized (sLock) {
            Log.d(TAG, "delete package: " + pkgName);
            deleteCache(pkgName);
            mDataBaseHelper.delete(pkgName);
        }
        return true;
    }

    public boolean insert(String pkgName) {
        if (isInvisibleWhitelist(pkgName)) {
            return false;
        }

        synchronized (sLock) {
            int defaultStatus = AppRecord.STATUS_DISABLED;
            if (mDefaultAppWhitelist.contains(pkgName)) {
                defaultStatus = AppRecord.STATUS_ENABLED;
            }
            Log.d(TAG, "insert package: " + pkgName + ", status: " + defaultStatus);
            mAppRecordsCache.add(new AppRecord(pkgName, defaultStatus));
            mDataBaseHelper.insert(pkgName, defaultStatus);
        }
        return true;
    }

    private void deleteCache(String pkgName) {
        Iterator<AppRecord> iter = mAppRecordsCache.iterator();
        while (iter.hasNext()) {
            String packageName = iter.next().getPkgName();
            if (packageName.equals(pkgName)) {
                iter.remove();
            }
        }
    }

    private void modifyCache(String pkgName, int status) {
        for (AppRecord appRecord : mAppRecordsCache) {
            if (appRecord.getPkgName().equals(pkgName)) {
                appRecord.setStatus(status);
            }
        }
    }
}
