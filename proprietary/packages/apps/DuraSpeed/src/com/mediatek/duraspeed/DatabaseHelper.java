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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class DatabaseHelper extends SQLiteOpenHelper {
    private static final String TAG = "DatabaseHelper";

    private static final int DB_VERSION = 1;
    // db name
    private static final String DB_NAME = "app_list.db";
    // table name
    private static final String TABLE_APP_LIST = "app_list";
    // item list
    private static final String FIELD_ID = "_id";
    private static final String FIELD_NAME_PACKAGE = "package_name";
    private static final String FIELD_NAME_STATUS = "status";

    private SQLiteDatabase mDb;

    public DatabaseHelper(Context context) {
        super(context, DB_NAME, null, DB_VERSION);
        mDb = getWritableDatabase();
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        String sql = "CREATE TABLE IF NOT EXISTS " + TABLE_APP_LIST
                + " (" + FIELD_ID + " INTEGER primary key autoincrement," + " "
                + FIELD_NAME_PACKAGE + " text," + " "
                + FIELD_NAME_STATUS + " INTEGER)";
        db.execSQL(sql);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        Log.d(TAG, "onUpgrade from " + oldVersion + " to " + newVersion);
    }

    public void update(String pkgName, int status) {
        String where = FIELD_NAME_PACKAGE + "= ?";
        String[] whereValue = new String[]{pkgName};
        ContentValues cv = new ContentValues();
        cv.put(FIELD_NAME_STATUS, status);
        mDb.update(TABLE_APP_LIST, cv, where, whereValue);
    }

    public void delete(String pkgName) {
        String where = FIELD_NAME_PACKAGE + "= ?";
        String[] whereValue = new String[]{pkgName};
        mDb.delete(TABLE_APP_LIST, where, whereValue);
    }

    public long insert(String pkgName, int status) {
        ContentValues cv = new ContentValues();
        cv.put(FIELD_NAME_PACKAGE, pkgName);
        cv.put(FIELD_NAME_STATUS, status);
        long row = mDb.insert(TABLE_APP_LIST, null, cv);
        return row;
    }

    public List<AppRecord> initDataCache() {
        Cursor cursor = null;
        List<AppRecord> appRecordsCache = new ArrayList<AppRecord>();
        try {
            cursor = getCursor(TABLE_APP_LIST);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    String packageName = cursor.getString(
                            cursor.getColumnIndex(FIELD_NAME_PACKAGE));
                    String status = cursor.getString(
                            cursor.getColumnIndex(FIELD_NAME_STATUS));
                    appRecordsCache.add(new AppRecord(packageName, Integer.valueOf(status)));
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return appRecordsCache;
    }

    public Cursor getCursor(String tableName) {
        Cursor cursor = mDb.query(tableName, null, null, null, null, null, null);
        return cursor;
    }
}
