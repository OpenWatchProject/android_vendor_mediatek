/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.provider;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;

import com.mediatek.rcse.api.Logger;

/**
 * This class provided a ContentProvider to other class for accessing.
 */
public class UnregMessageProvider extends ContentProvider {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "UnregMessageProvider";
    // Content Uri
    /**
     * The Constant CONTENT_URI.
     */
    public static final Uri CONTENT_URI = Uri
            .parse("content://com.mediatek.rcse.unreg_messages");
    // UregMessageHelper instance
    /**
     * The m msg helper.
     */
    private UregMessageHelper mMsgHelper = null;
    // Database version
    /**
     * The Constant DM_VERSION.
     */
    public static final int DM_VERSION = 6;
    // Table name
    /**
     * The Constant TABLE_NAME.
     */
    public static final String TABLE_NAME = "messages";
    // Table fields
    /**
     * The Constant KEY_CHAT_TAG.
     */
    public static final String KEY_CHAT_TAG = "chat_tag";
    /**
     * The Constant KEY_MESSAGE_TAG.
     */
    public static final String KEY_MESSAGE_TAG = "msg_tag";
    /**
     * The Constant KEY_MESSAGE.
     */
    public static final String KEY_MESSAGE = "msg";
    /**
     * The Constant KEY_CHAT_ID.
     */
    public static final String KEY_CHAT_ID = "chat_id";
    /**
     * The Constant KEY_TIMESTAMP.
     */
    public static final String KEY_TIMESTAMP = "msg_timestamp";
    // Database name
    /**
     * The Constant DATABASE_NAME.
     */
    private static final String DATABASE_NAME = "messages.db";
    // SQliteDatabase instance
    /**
     * The m db.
     */
    private SQLiteDatabase mDb = null;

    /* (non-Javadoc)
     * @see android.content.ContentProvider#onCreate()
     */
    @Override
    public boolean onCreate() {
        mMsgHelper = new UregMessageHelper(getContext());
        mDb = mMsgHelper.getWritableDatabase();
        return true;
    }
    /* (non-Javadoc)
     * @see android.content.ContentProvider#delete
     * (android.net.Uri, java.lang.String, java.lang.String[])
     */
    @Override
    public int delete(Uri uri, String selection,
            String[] selectionArgs) {
        int number = mDb.delete(TABLE_NAME, selection, selectionArgs);
        return number;
    }
    /* (non-Javadoc)
     * @see android.content.ContentProvider#insert
     * (android.net.Uri, android.content.ContentValues)
     */
    @Override
    public Uri insert(Uri uri, ContentValues values) {
        long id = mDb.insert(TABLE_NAME, null, values);
        Logger.i(TAG, "insert id = " + id);
        return Uri.parse(uri.toString() + "/" + String.valueOf(id));
    }
    /* (non-Javadoc)
     * @see android.content.ContentProvider#query
     * (android.net.Uri, java.lang.String[], java.lang.String, java.lang.String[]
     * , java.lang.String)
     */
    @Override
    public Cursor query(Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder) {
        Cursor cursor = mDb.query(TABLE_NAME, projection, selection,
                selectionArgs, null, null, sortOrder);
        return cursor;
    }
    /**
     * Close the SQLiteDatabase automatic invoked by system Comment: This method
     * used by ICS.
     *
     * @param uri the uri
     * @param values the values
     * @param selection the selection
     * @param selectionArgs the selection args
     * @return the int
     */
    /*
    public void shutdown() throws SQLException{
        Logger.i(TAG, "Content provider shut down");
        mDb.close();
    }
    */
    /* (non-Javadoc)
     * @see android.content.ContentProvider#update
     * (android.net.Uri, android.content.ContentValues,
     *  java.lang.String, java.lang.String[])
     */
    @Override
    public int update(Uri uri, ContentValues values,
            String selection, String[] selectionArgs) {
        // This is not necessary for this feature
        return 0;
    }
    /* (non-Javadoc)
     * @see android.content.ContentProvider#getType(android.net.Uri)
     */
    @Override
    public String getType(Uri uri) {
        // This is not necessary for this feature
        return null;
    }

    /**
     * This class used to create database and manage table.
     */
    public static class UregMessageHelper extends SQLiteOpenHelper {
        // SQL command to create table
        /**
         * The Constant CREATE_TABLE.
         */
        private static final String CREATE_TABLE = "CREATE TABLE IF NOT EXISTS "
                + TABLE_NAME
                + " ("
                + KEY_CHAT_TAG
                + " text not null, "
                + KEY_MESSAGE_TAG
                + " integer not null unique, "
                + KEY_CHAT_ID
                + " text not null, "
                + KEY_TIMESTAMP
                + " long,"
                + KEY_MESSAGE
                + " text not null);";
        // SQL command to drop table
        /**
         * The Constant DROP_TABLE.
         */
        private static final String DROP_TABLE = "drop table IF EXISTS "
                + TABLE_NAME;
        /**
         * The m db version.
         */
        private int mDbVersion = -1;

        /**
         * Instantiates a new ureg message helper.
         *
         * @param context the context
         */
        public UregMessageHelper(Context context) {
            super(context, DATABASE_NAME, null, DM_VERSION);
            mDbVersion = DM_VERSION;
        }
        /* (non-Javadoc)
         * @see android.database.sqlite.SQLiteOpenHelper#onCreate
         * (android.database.sqlite.SQLiteDatabase)
         */
        @Override
        public void onCreate(SQLiteDatabase db) {
            try {
                db.execSQL(CREATE_TABLE);
                db.setVersion(mDbVersion);
            } catch (SQLException e) {
                e.printStackTrace();
                Logger.e(TAG, "Create table messages failed");
            }
        }
        /* (non-Javadoc)
         * @see android.database.sqlite.SQLiteOpenHelper#onUpgrade
         * (android.database.sqlite.SQLiteDatabase, int, int)
         */
        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVer,
                int newVer) {
            try {
                mDbVersion = newVer;
                db.execSQL(DROP_TABLE);
            } catch (SQLException e) {
                e.printStackTrace();
                Logger.e(TAG, "Drop table messages failed");
            }
            onCreate(db);
        }
    }
}
