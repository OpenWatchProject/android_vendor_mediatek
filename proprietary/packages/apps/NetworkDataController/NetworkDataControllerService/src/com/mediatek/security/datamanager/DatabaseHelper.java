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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.mediatek.security.service.Log;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.security.service.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

public class DatabaseHelper extends SQLiteOpenHelper {

    public static final String TAG = "DatabaseHelper";
    private static final String DB_NAME = "NetworkDataController.db";
    private static final int DB_VERSION = 1;

    private static final String FIELD_ID = "_id";
    private static final String FIELD_NAME_PACKAGE = "packages_name";
    private static final String FIELD_NAME_UID = "packages_uid";

    private static final String TABLE_PACKAGE_PERMISSIONS = "package_permission";
    private static final String FIELD_NAME_NETWORK_DATA = "network_data";

    private HashMap<Integer, CheckedPermRecord> mPkgPermissionsCache = new HashMap<Integer, CheckedPermRecord>();
    private Context mContext;
    private SQLiteDatabase mDb;

    /**
     * Construct of DatabaseHelper, this construct to init database and access
     * data base
     *
     * @param context
     */
    public DatabaseHelper(Context context) {
        super(context, DB_NAME, null, DB_VERSION);
        mContext = context;
        mDb = getWritableDatabase();
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        String sql = "CREATE TABLE IF NOT EXISTS " + TABLE_PACKAGE_PERMISSIONS
                + " (" + FIELD_ID + " INTEGER primary key autoincrement," + " "
                + FIELD_NAME_PACKAGE + " text," + " " + FIELD_NAME_UID
                + " int," + " " + FIELD_NAME_NETWORK_DATA + " int)";
        Log.i(TAG, "onCreate: " + sql);
        db.execSQL(sql);
    }

    /**
     * Because current there is no new add into db so do nothing
     */
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        Log.d(TAG, "onUpgrade oldVersion = " + oldVersion + ", newVersion = "
                + newVersion);
        if (oldVersion != newVersion) {
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_PACKAGE_PERMISSIONS);
            onCreate(db);
        }
    }

    /**
     * Get the cursor of table
     *
     * @param tableName
     *            name of table
     * @return cursor of the table
     */
    public Cursor getCursor(String tableName) {
        Cursor cursor = mDb
                .query(tableName, null, null, null, null, null, null);
        return cursor;
    }

    public HashSet<Integer> getNetworkDataPermFromDB() {
        Log.i(TAG, " getNetworkDataPermFromDB ");
        Cursor cursor = null;
        HashSet<Integer> set = new HashSet<Integer>();
        try {
            cursor = getCursor(TABLE_PACKAGE_PERMISSIONS);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    int uid = cursor.getInt(cursor
                            .getColumnIndex(FIELD_NAME_UID));
                    set.add(uid);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return set;
    }

    public boolean addNetworkDataPerm(CheckedPermRecord info) {
        ContentValues cv = new ContentValues();
        cv.put(FIELD_NAME_PACKAGE, info.mPackageName);
        cv.put(FIELD_NAME_UID, info.mUid);
        cv.put(FIELD_NAME_NETWORK_DATA, info.getStatus());
        mDb.insert(TABLE_PACKAGE_PERMISSIONS, null, cv);
        synchronized (PermControlUtils.mLock) {
            mPkgPermissionsCache.put(info.mUid, info);
        }
        return true;
    }

    public boolean deleteNetworkDataPerm(int uid) {
        Log.i(TAG, " deleteNetworkDataPerm, uid: " + uid);
        String where = FIELD_NAME_UID + "= ?";
        String[] whereValue = new String[] { Integer.toString(uid) };
        mDb.delete(TABLE_PACKAGE_PERMISSIONS, where, whereValue);
        synchronized (PermControlUtils.mLock) {
            mPkgPermissionsCache.remove(uid);
        }
        return true;
    }

    public boolean modifyNetworkDataPerm(CheckedPermRecord info) {
        ContentValues cv = new ContentValues();
        cv.put(FIELD_NAME_NETWORK_DATA, info.getStatus());
        String where = FIELD_NAME_UID + "= ?";
        String[] whereValue = new String[] { Integer.toString(info.mUid) };
        mDb.update(TABLE_PACKAGE_PERMISSIONS, cv, where, whereValue);
        synchronized (PermControlUtils.mLock) {
            // replace the old value of exist uid
            mPkgPermissionsCache.put(info.mUid, info);
        }
        return true;
    }

    public HashMap<Integer, CheckedPermRecord> initNetworkDataPermCache() {
        Log.i(TAG, " initNetworkDataPermCache ");
        Cursor cursor = null;
        synchronized (PermControlUtils.mLock) {
            try {
                cursor = getCursor(TABLE_PACKAGE_PERMISSIONS);
                if (cursor != null) {
                    while (cursor.moveToNext()) {
                        String packageName = cursor.getString(cursor
                                .getColumnIndex(FIELD_NAME_PACKAGE));
                        int uid = cursor.getInt(cursor
                                .getColumnIndex(FIELD_NAME_UID));
                        int networkData = cursor.getInt(cursor
                                .getColumnIndex(FIELD_NAME_NETWORK_DATA));
                        CheckedPermRecord record = new CheckedPermRecord(
                                packageName, uid, networkData);
                        Log.i(TAG, " initNetworkDataPermCache " + packageName
                                + ", uid: " + uid + ", status: " + networkData);
                        mPkgPermissionsCache.put(uid, record);
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }

            Log.d(TAG, "initNetworkDataPermCache" + "cachesize: "
                + mPkgPermissionsCache.size());
        }
        return mPkgPermissionsCache;
    }
}
