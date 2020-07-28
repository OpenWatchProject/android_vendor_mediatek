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
 * MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.security.datamanager;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.Manifest;
import android.os.SystemProperties;

import com.mediatek.security.service.Log;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.security.service.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class DatabaseManager {
    private static final String TAG = "DatabaseManager";

    private DatabaseHelper dataBaseHelper;
    private HashMap<Integer, CheckedPermRecord> mPkgPermissionsCache
            = new HashMap<Integer, CheckedPermRecord>();

    /**
     * Init database and cache must be called at first and in async process
     *
     * @param context
     *            Context of process
     */
    public void initDataBase(Context context) {
        Log.d(TAG, "initDataBase");
        createDataBase(context);
        updateDataBase(context);
        setDataIntoCache();
    }

    private void createDataBase(Context context) {
        if (dataBaseHelper == null) {
            Log.d(TAG, "new DatabaseHelper");
            dataBaseHelper = new DatabaseHelper(context);
        }
    }

    private void updateDataBase(Context context) {

        // get all packages use internet inculde system packages
        HashMap<Integer, String> packageList = PermControlUtils
                .getPkgsUseInternet(context);
        if (packageList != null) {
            // get all packages in the database
            HashSet<Integer> storedPackages = dataBaseHelper
                    .getNetworkDataPermFromDB();
            Log.d(TAG,
                    "getInstalledPackages: " + packageList.size()
                            + ", getpackage in database,sets: "
                            + storedPackages.size());
            Iterator<Integer> interator = packageList.keySet().iterator();
            while (interator.hasNext()) {
                int uid = interator.next();
                if (storedPackages.contains(uid)) {
                    interator.remove();
                    storedPackages.remove(uid);
                }
            }
            Log.d(TAG, "all = " + packageList.size() + ", old = "
                    + storedPackages.size());
            Set<Integer> uids = packageList.keySet();
            for (Integer uid : uids) {
                dataBaseHelper.addNetworkDataPerm(new CheckedPermRecord(
                        packageList.get(uid), uid,
                        CheckedPermRecord.STATUS_FIRST_CHECK));
            }
            // delete all uninstalled package in the databases.
            for (int uid : storedPackages) {
                dataBaseHelper.deleteNetworkDataPerm(uid);
            }
        }
    }

    public void setDataIntoCache() {
        mPkgPermissionsCache = dataBaseHelper.initNetworkDataPermCache();
        Log.d(TAG, "setDataIntoCache" + "cachesize: "
                + mPkgPermissionsCache.size());
    }

    /**
     * Add a new install pkg permission into database. Since it will write
     * database, call it in Asynchronous.
     *
     * @param uid
     *            user id of apk
     * @param packageName
     *            package name of the apk
     */
    public boolean add(int uid, String packageName) {
        Log.d(TAG, "add" + " uid: " + uid);
        boolean isAdded = false;
        CheckedPermRecord premRecord = getNetworkDataRecord(uid);
        if (premRecord == null) {
            dataBaseHelper
                    .addNetworkDataPerm(new CheckedPermRecord(packageName,
                           uid, CheckedPermRecord.STATUS_FIRST_CHECK));
            isAdded = true;
        } else if (!premRecord.mPackageName.contains(packageName)) {
            premRecord.mergePkgName(packageName);
            updateNetworkDataRecord(new CheckedPermRecord(premRecord));
        }
        return isAdded;
    }

    /**
     * Delete the specific package from database. Since it will write database,
     * call it in Asynchronous.
     *
     * @param uid
     *            user id of apk
     * @param packageName
     *            package name of apk
     */
    public boolean delete(int uid, String packageName) {
        boolean isDeleted = false;
        Log.d(TAG, "delete" + " uid: " + uid);

        if ((uid != -1) && (packageName != null)) {
            CheckedPermRecord record = getNetworkDataRecord(uid);
            if (record != null) {
                if (packageName.equals(record.mPackageName)) {
                    isDeleted = dataBaseHelper.deleteNetworkDataPerm(uid);
                } else if (record.mPackageName.contains(packageName)) {
                    record.removePkgName(packageName);
                    updateNetworkDataRecord(new CheckedPermRecord(record));
                }
            }
        }
        return isDeleted;
    }

    /**
     * Update the specific package in database. Since it will write database,
     * call it in Asynchronous.
     *
     * @param info
     *            CheckedPermRecord of the new record
     */
    public boolean updateNetworkDataRecord(CheckedPermRecord info) {
        CheckedPermRecord target = null;
        synchronized (PermControlUtils.mLock) {
            target = mPkgPermissionsCache.get(info.mUid);
         }
        if (target == null) {
            return false;
        }
        Log.i(TAG, " updateNetworkDataRecord: package name: uid: "
                + info.mUid + ", old status: " + target.getStatus()
                + "new status: " + info.getStatus());
        dataBaseHelper.modifyNetworkDataPerm(info);

        return true;
    }

    public List<CheckedPermRecord> getNetworkDataRecordList() {
        Log.i(TAG, "getNetworkDataRecordList");
        List<CheckedPermRecord> list = new ArrayList<CheckedPermRecord>();
        synchronized (PermControlUtils.mLock) {
            Iterator<CheckedPermRecord> iterator = mPkgPermissionsCache
                    .values().iterator();
            while (iterator.hasNext()) {
                list.add(new CheckedPermRecord(iterator.next()));
            }
        }
        if (list == null) {
            Log.e(TAG, "getNetworkDataRecordList error return null");
        }
        return list;
    }

    public CheckedPermRecord getNetworkDataRecord(int uid) {
        Log.i(TAG, "getNetworkDataRecord, uid: " + uid);
        CheckedPermRecord record = null;
        synchronized (PermControlUtils.mLock) {
            record = mPkgPermissionsCache.get(uid);
        }
        if (record != null) {
            return new CheckedPermRecord(record);
        } else {
            Log.e(TAG, "getNetworkDataRecord can't get uid= " + uid);
        }
        return null;

    }
}
