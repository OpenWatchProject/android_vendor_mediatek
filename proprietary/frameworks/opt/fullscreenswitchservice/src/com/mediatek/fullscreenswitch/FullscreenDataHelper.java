/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
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

package com.mediatek.fullscreenswitch;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Log;

import com.mediatek.fullscreenswitch.FullscreenSwitch.FullscreenModeColoums;

import java.util.HashMap;
import java.util.List;

public class FullscreenDataHelper {

    private class FullscreenModeObserver extends ContentObserver {
        private static final String TAG = "FullscreenSwitchService";

        public FullscreenModeObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            if (FullscreenSwitchService.DEBUG) {
                Log.d(TAG, " selfChange = " + selfChange + " uri = " + uri);
            }
            super.onChange(selfChange, uri);
            if (FullscreenSwitch.CONTENT_URI_PACKAGES.equals(uri)) {
                mService.refreshList();
                mService.notifyAppListChanged();
            } else {
                int id = Integer.parseInt(uri.getPathSegments().get(1));
                mService.notifyAppModeChanged(id);
                if (FullscreenSwitchService.DEBUG) {
                    Log.e(TAG, "onDataChanged, id=: " + id);
                }
            }
        }
    }

    public FullscreenDataHelper(FullscreenSwitchService service,
            Context context, Handler handler) {
        mService = service;
        mContext = context;
        mContext.getContentResolver().registerContentObserver(
                FullscreenSwitch.CONTENT_URI_PACKAGES, true,
                new FullscreenModeObserver(handler));
    }

    private Context mContext;
    private FullscreenSwitchService mService;
    // used for record the app state when app upgrading.
    private FullscreenMode mLastRemoveApp;

    public boolean updateFullscreenMode(String packageName, Boolean mode) {
        if (FullscreenSwitchService.DEBUG) {
            Log.d(FullscreenSwitchService.TAG,
                    " updateFullscreenMode, packageName= " + packageName
                            + " mode = " + mode + " start="
                            + System.currentTimeMillis());
        }

        ContentValues values = new ContentValues();
        values.put(FullscreenModeColoums.COLOUM_FULLSCREEN_MODE,
                mode ? FullscreenMode.APP_FULL_SCREEN_MODE
                        : FullscreenMode.APP_CROP_SCREEN_MODE);
        int count = 0;
        try {
            count = mContext.getContentResolver().update(
                    FullscreenSwitch.CONTENT_URI_PACKAGES, values,
                    FullscreenSwitch.SQL_PACKAGE_SELECTION,
                    new String[] { packageName });
        } catch (Exception e) {
            Log.e(FullscreenSwitchService.TAG, " updateFullscreenMode error :"
                    + e.getMessage());
        }

        if (FullscreenSwitchService.DEBUG) {
            Log.d(FullscreenSwitchService.TAG, " updateFullscreenMode ...end= "
                    + System.currentTimeMillis());
        }

        return count > 0;
    }

    private void insertFullscreenMode(FullscreenMode mode) {
        try {
            ContentValues values = new ContentValues();
            values.put(FullscreenModeColoums.COLOUM_PACKAGE, mode.packageName);
            values.put(FullscreenModeColoums.COLOUM_FULLSCREEN_MODE, mode.mode);
            values.put(FullscreenModeColoums.COLOUM_APP_LEVEL, mode.level);
            values.put(FullscreenModeColoums.COLOUM_TYPE, mode.type);
            mContext.getContentResolver().insert(
                    FullscreenSwitch.CONTENT_URI_PACKAGES, values);
        } catch (Exception e) {
            Log.e(FullscreenSwitchService.TAG, " insertFullscreenMode error :"
                    + e);
        }
    }

    public HashMap<String, FullscreenMode> loadModes() {
        HashMap<String, FullscreenMode> map = new HashMap<String, FullscreenMode>();
        try {
            Cursor cursor = mContext.getContentResolver().query(
                    FullscreenSwitch.CONTENT_URI_PACKAGES,
                    FullscreenSwitch.COLOUMS_ALL, null, null,
                    FullscreenSwitch.SQL_ORDER_BY_ID);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    FullscreenMode state = getData(cursor);
                    map.put(state.packageName, state);
                    if (FullscreenSwitchService.DEBUG) {
                        Log.d(FullscreenSwitchService.TAG,
                                " load app info state : " + state);
                    }
                }
                cursor.close();
            }
        } catch (Exception e) {
            Log.e(FullscreenSwitchService.TAG, " loadModes error :"
                    + e.getMessage());
        }
        Log.d(FullscreenSwitchService.TAG,
                "loadModes, all app size : " + map.size());
        return map;
    }

    private FullscreenMode getData(Cursor cursor) {
        FullscreenMode state = new FullscreenMode();
        state.id = cursor
                .getInt(cursor.getColumnIndex(FullscreenModeColoums._ID));
        state.packageName = cursor.getString(cursor
                .getColumnIndex(FullscreenModeColoums.COLOUM_PACKAGE));
        state.mode = cursor.getInt(cursor
                .getColumnIndex(FullscreenModeColoums.COLOUM_FULLSCREEN_MODE));
        state.level = cursor.getInt(cursor
                .getColumnIndex(FullscreenModeColoums.COLOUM_APP_LEVEL));
        state.type = cursor.getInt(cursor
                .getColumnIndex(FullscreenModeColoums.COLOUM_TYPE));
        return state;
    }

    public FullscreenMode getFullscreenMode(int row_id) {
        FullscreenMode newState = null;

        Uri uri = ContentUris.withAppendedId(
                FullscreenSwitch.CONTENT_URI_PACKAGES, row_id);
        try {
            Cursor cursor = mContext.getContentResolver().query(uri,
                    FullscreenSwitch.COLOUMS_ALL, null, null,
                    FullscreenSwitch.SQL_ORDER_BY_ID);
            if (cursor != null) {
                if (cursor.moveToNext()) {
                    newState = getData(cursor);
                    Log.d(FullscreenSwitchService.TAG, " load state : " + newState);
                } else {
                    Log.e(FullscreenSwitchService.TAG, "moveToNext failed, uri=: "
                            + uri);
                }
                cursor.close();
            } else {
                Log.e(FullscreenSwitchService.TAG, "load uri failed,uri= " + uri);
            }
        } catch (Exception e) {
            Log.e(FullscreenSwitchService.TAG, " getFullscreenMode error :"
                    + e.getMessage());
        }

        return newState;
    }

    public void queryAppsAndInsert(PackageManager packageManager) {
        List<ApplicationInfo> installedApps = packageManager
                .getInstalledApplications(PackageManager.GET_UNINSTALLED_PACKAGES
                        | PackageManager.GET_DISABLED_COMPONENTS);
        Intent launchIntent = new Intent(Intent.ACTION_MAIN, null)
                .addCategory(Intent.CATEGORY_LAUNCHER);
        List<ResolveInfo> intents = packageManager.queryIntentActivities(launchIntent,
                PackageManager.GET_DISABLED_COMPONENTS);
        if (installedApps != null) {
            for (ApplicationInfo appInfo : installedApps) {
                initAppMode(packageManager, appInfo, intents);
            }
        }
    }

    private void initAppMode(PackageManager packageManager, ApplicationInfo appInfo,
            List<ResolveInfo> intents) {
        String pkgName = appInfo.packageName;
        if (pkgName == null) {
            Log.e(FullscreenSwitchService.TAG, "initAppMode pkgName is null!");
            return;
        }

        FullscreenMode mode = new FullscreenMode();
        mode.packageName = pkgName;
        if (isBuiltinApp(appInfo)) {
            mode.level = FullscreenMode.APP_LEVEL_SYSTEM;
            mode.mode = FullscreenMode.APP_FULL_SCREEN_MODE;
        } else {
            mode.level = FullscreenMode.APP_LEVEL_THIRD_PARTY;
            if (mLastRemoveApp != null && pkgName.equalsIgnoreCase(mLastRemoveApp.packageName)) {
                mode.mode = mLastRemoveApp.mode;
            } else {
                boolean unReisizeable = (appInfo.privateFlags &
                        ApplicationInfo.PRIVATE_FLAG_ACTIVITIES_RESIZE_MODE_UNRESIZEABLE) != 0;
                Log.d(FullscreenSwitchService.TAG,
                        "initAppMode appReisizeable is = " + unReisizeable
                                + " appInfo.targetSdkVersion = "
                                + appInfo.targetSdkVersion);
                if (unReisizeable) {
                    mode.mode = FullscreenMode.APP_CROP_SCREEN_MODE;
                } else {
                    mode.mode = FullscreenMode.APP_FULL_SCREEN_MODE;
                }
            }
        }

        if (hasLauncherEntry(pkgName, intents)) {
            mode.type = FullscreenMode.TYPE_HAS_LAUNCHER_ACTIVITY;
        }
        insertFullscreenMode(mode);
    }

    public void onPackageAdded(PackageManager packageManager,
            String addedPkgName) {
        try {
            ApplicationInfo appInfo = packageManager.getApplicationInfo(
                    addedPkgName, 0);
            Intent launchIntent = new Intent(Intent.ACTION_MAIN, null)
                    .addCategory(Intent.CATEGORY_LAUNCHER);
            launchIntent.setPackage(addedPkgName);
            List<ResolveInfo> intents = packageManager.queryIntentActivities(
                    launchIntent, PackageManager.GET_DISABLED_COMPONENTS);
            initAppMode(packageManager, appInfo, intents);
        } catch (NameNotFoundException e) {
            Log.d(FullscreenSwitchService.TAG, " not found: " + e.getMessage());
        }
    }

    public void onPackageRemoved(PackageManager packageManager,
            String removedPkgName) {
        mLastRemoveApp = mService.mModeCache.get(removedPkgName);
        try {
            int count = mContext.getContentResolver().delete(
                    FullscreenSwitch.CONTENT_URI_PACKAGES,
                    FullscreenSwitch.SQL_PACKAGE_SELECTION,
                    new String[] { removedPkgName });
        } catch (Exception e) {
            Log.e(FullscreenSwitchService.TAG, " onPackageRemoved error :"
                    + e.getMessage());
        }
    }

    private static boolean isBuiltinApp(ApplicationInfo appInfo) {
        return (appInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0
                || (appInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0;
    }

    private static boolean hasLauncherEntry(String pkgName,
            List<ResolveInfo> intents) {
        if (pkgName == null || intents == null) {
            Log.e(FullscreenSwitchService.TAG, " hasLauncherEntry error, pkgName =" + pkgName);
            return false;
        }
        for (int j = 0; j < intents.size(); j++) {
            String intentPackageName = intents.get(j).activityInfo.packageName;
            if (pkgName.equals(intentPackageName)) {
                return true;
            }
        }
        return false;
    }
}
