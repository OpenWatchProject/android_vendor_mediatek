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

import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.ContentUris;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

public class FullscreenSwitchProvider extends ContentProvider {

    private static final String TAG = "ResolutionModeService";
    public static final boolean DEBUG = true;

    private static final UriMatcher sMatcher = new UriMatcher(
            UriMatcher.NO_MATCH);
    private static final int URL_COMMON = 1;
    private static final int URL_PACKAGE_ID = 2;

    private static final String TYPE_PACKAGE = "vnd.android.cursor.dir/download";
    private static final String TYPE_PACKAGE_ID = "vnd.android.cursor.item/package";
    private DatabaseHelper mDatabaseHelper;
    private ContentResolver mResolver;

    public static final String SQL_WHERE_BY_ID = FullscreenSwitch.FullscreenModeColoums._ID
            + "=";

    static {
        sMatcher.addURI(FullscreenSwitch.AUTHORITIES,
                FullscreenSwitch.PACKAGES, URL_COMMON);
        sMatcher.addURI(FullscreenSwitch.AUTHORITIES,
                FullscreenSwitch.PACKAGE_ITEM, URL_PACKAGE_ID);
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        int count;

        int match = sMatcher.match(uri);
        SQLiteDatabase db;
        switch (match) {
        case URL_COMMON:
            db = mDatabaseHelper.getWritableDatabase();
            count = db
                    .delete(DatabaseHelper.DB_TABLE, selection, selectionArgs);
            mResolver.notifyChange(FullscreenSwitch.CONTENT_URI_PACKAGES, null);
            break;
        case URL_PACKAGE_ID:
            db = mDatabaseHelper.getWritableDatabase();
            String id = uri.getPathSegments().get(1);
            count = db.delete(DatabaseHelper.DB_TABLE, SQL_WHERE_BY_ID + id,
                    null);
            mResolver.notifyChange(FullscreenSwitch.CONTENT_URI_PACKAGES, null);
            break;
        default:
            throw new IllegalArgumentException("Unknown uri: " + uri);
        }
        return count;
    }

    @Override
    public String getType(Uri uri) {
        int match = sMatcher.match(uri);
        switch (match) {
        case URL_COMMON:
            return TYPE_PACKAGE;
        case URL_PACKAGE_ID:
            return TYPE_PACKAGE_ID;
        default:
            throw new IllegalArgumentException("Unknown uri: " + uri);
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        Uri newUri = null;

        int match = sMatcher.match(uri);
        switch (match) {
        case URL_COMMON:
            String pkg = values
                    .getAsString(FullscreenSwitch.FullscreenModeColoums.COLOUM_PACKAGE);
            if (!TextUtils.isEmpty(pkg)) {
                SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();
                long rowID = db.insert(DatabaseHelper.DB_TABLE, null, values);
                newUri = ContentUris.withAppendedId(
                        FullscreenSwitch.CONTENT_URI_PACKAGE_ITEM, rowID);
                Log.d(TAG, "...... insert = " + newUri);
                mResolver.notifyChange(FullscreenSwitch.CONTENT_URI_PACKAGES,
                        null);
            } else {
                Log.e(TAG, "package is null when insert.");
            }
            break;
        default:
            throw new IllegalArgumentException("Unknown uri: " + uri);
        }
        return newUri;
    }

    @Override
    public boolean onCreate() {
        if (DEBUG) {
            Log.e(TAG, "FullscreenSwitchProvider onCreate.");
        }
        mDatabaseHelper = new DatabaseHelper(this.getContext());
        mResolver = getContext().getContentResolver();
        return false;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        Cursor cursor = null;

        int match = sMatcher.match(uri);
        SQLiteDatabase db;
        switch (match) {
        case URL_COMMON:
            db = mDatabaseHelper.getReadableDatabase();
            cursor = db.query(DatabaseHelper.DB_TABLE, projection, selection,
                    selectionArgs, null, null, sortOrder);
            break;
        case URL_PACKAGE_ID:
            db = mDatabaseHelper.getReadableDatabase();
            String id = uri.getPathSegments().get(1);
            String idSelection = SQL_WHERE_BY_ID + id;
            selection = TextUtils.isEmpty(selection) ? idSelection : selection
                    + " and " + idSelection;
            cursor = db.query(DatabaseHelper.DB_TABLE, projection, selection,
                    selectionArgs, null, null, sortOrder);
            break;
        default:
            throw new IllegalArgumentException("Unknown uri: " + uri);
        }
        return cursor;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
            String[] selectionArgs) {
        int count = 0;
        int match = sMatcher.match(uri);
        SQLiteDatabase db;
        switch (match) {
        case URL_PACKAGE_ID:
            db = mDatabaseHelper.getWritableDatabase();
            String id = uri.getPathSegments().get(1);
            count = db.update(DatabaseHelper.DB_TABLE, values, SQL_WHERE_BY_ID,
                    null);
            Log.d(TAG, values + " update uri =" + uri + ", count = " + count);
            mResolver.notifyChange(uri, null);
            break;
        case URL_COMMON:
            db = mDatabaseHelper.getWritableDatabase();
            count = db.update(DatabaseHelper.DB_TABLE, values, selection,
                    selectionArgs);
            Log.d(TAG, values + " update uri =" + uri + ", count = " + count);
            mResolver.notifyChange(uri, null);
            break;
        default:
            throw new IllegalArgumentException("Unknown uri: " + uri);
        }
        return count;
    }
}
