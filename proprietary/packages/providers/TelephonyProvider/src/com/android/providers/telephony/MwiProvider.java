/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.providers.telephony;

import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.os.SystemProperties;
//import android.provider.Telephony.Mwi;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import mediatek.telephony.MtkTelephony.MtkMwi;

public class MwiProvider extends ContentProvider {

    private static final String TAG = "Mms/Provider/Mwi";
    private static final String TABLE_MWI = "mwi";
    private static final Uri NOTIFICATION_URI = Uri.parse("content://mwimsg");
    private SQLiteOpenHelper mMwiOpenHelper;

    public static final boolean MTK_IMS_SUPPORT = SystemProperties.
            get("persist.vendor.ims_support").equals("1");
    public static final boolean MTK_VOLTE_SUPPORT = SystemProperties.
            get("persist.vendor.volte_support").equals("1");
    public static final boolean MTK_MWI_SUPPORT = MTK_IMS_SUPPORT
            && MTK_VOLTE_SUPPORT;

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        MmsSmsProvider.MmsProviderLog.dpi("@M_" + TAG, "delete begin, uri = " + uri + ", selection = "
                + selection);
        // if wap push is not support, should return 0;
        if (!MTK_MWI_SUPPORT) {
            return 0;
        }

        SQLiteDatabase db = mMwiOpenHelper.getWritableDatabase();
        int count = 0;
        switch (URI_MATCHER.match(uri)) {
        case MWI_ALL:
            count = db.delete(TABLE_MWI, selection, selectionArgs);
            break;
        case MWI_ID:
            String idSelection = MtkMwi._ID + "=" + uri.getPathSegments().get(0);
            if (selection == null || selection.equals("")) {
                selection = idSelection;
            } else {
                selection += " and " + idSelection;
            }
            count = db.delete(TABLE_MWI, selection, selectionArgs);
            break;
        default:
            MmsSmsProvider.MmsProviderLog.epi(TAG, "Unknown URI " + uri);
            return 0;
        }
        if (count > 0) {
            notifyChange(uri);
        }
        Log.d(TAG, "delete end, affectedRows = " + count);
        return count;

    }

    @Override
    public String getType(Uri uri) {
        // TODO Auto-generated method stub
        switch (URI_MATCHER.match(uri)) {
        case MWI_ALL:
            return VND_ANDROID_DIR_MWI;
        }
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        MmsSmsProvider.MmsProviderLog.dpi("@M_" + TAG, "insert begin, uri = " + uri + ", values = "
                + values);
        // if wap push is not support, should return null;
        if (!MTK_MWI_SUPPORT) {
            return null;
        }

        if (values == null) {
            return null;
        }

        SQLiteDatabase db = mMwiOpenHelper.getWritableDatabase();

        // get date
        // if(values.getAsLong(Mwi.DATE) == null){
        // values.put(Mwi.DATE, System.currentTimeMillis());
        // }

        // insert into database
        long rowId = db.insert(TABLE_MWI, null, values);
        Uri insertUri = ContentUris.withAppendedId(MtkMwi.CONTENT_URI, rowId);

        if (rowId > 0) {
            notifyChange(uri);
            MmsSmsProvider.MmsProviderLog.dpi(TAG, "insert succeed, uri = " + insertUri);
            return insertUri;
        } else {
            MmsSmsProvider.MmsProviderLog.dpi(TAG, "Failed to insert! " + values.toString());
        }
        return null;
    }

    @Override
    public boolean onCreate() {
        // TODO Auto-generated method stub
        mMwiOpenHelper = MmsSmsDatabaseHelper.getInstanceForCe(getContext());

        // if wap push is not support, should return false;
        if (!MTK_MWI_SUPPORT) {
            return false;
        } else {
            return true;
        }
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        MmsSmsProvider.MmsProviderLog.dpi("@M_" + TAG, "query begin, uri = " + uri + ", selection = "
                + selection);
        // if wap push is not support, should return null;
        if (!MTK_MWI_SUPPORT) {
            return null;
        }

        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE_MWI);

        switch (URI_MATCHER.match(uri)) {
        case MWI_ALL:
            break;
        case MWI_ID:
            qb.appendWhere(MtkMwi._ID + "=" + uri.getPathSegments().get(0));
            break;
        default:
            MmsSmsProvider.MmsProviderLog.epi("@M_" + "TAG", "Unknown URI " + uri);
            return null;
        }

        String finalSortOrder = TextUtils.isEmpty(sortOrder) ? "msg_date ASC"
                : sortOrder;

        SQLiteDatabase db = mMwiOpenHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null,
                null, finalSortOrder);
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(),
                    NOTIFICATION_URI);
        }
        Log.d("@M_" + TAG, "query end");
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
            String[] selectionArgs) {
        MmsSmsProvider.MmsProviderLog.dpi("@M_" + TAG, "update begin, uri = " + uri + ", values = "
                + values + ", selection = " + selection);
        int count = 0;
        SQLiteDatabase db = mMwiOpenHelper.getWritableDatabase();
        switch (URI_MATCHER.match(uri)) {
        case MWI_ID:
            String newIdSelection = MtkMwi._ID
                    + "="
                    + uri.getPathSegments().get(0)
                    + (!TextUtils.isEmpty(selection) ? " AND (" + selection
                            + ')' : "");
            count = db.update(TABLE_MWI, values, newIdSelection, selectionArgs);
            break;
        }
        if (count > 0) {
            notifyChange(uri);
        }
        Log.d("@M_" + TAG, "update end, affectedRows = " + count);
        return count;
    }

    private void notifyChange(Uri uri) {
        ContentResolver cr = getContext().getContentResolver();
        cr.notifyChange(uri, null);
        MmsSmsProvider.MmsProviderLog.ipi("@M_" + TAG, "notifyChange, uri = " + uri);
    }

    private static final String VND_ANDROID_MWI = "vnd.android.cursor.item/mwimsg";
    private static final String VND_ANDROID_DIR_MWI = "vnd.android.cursor.dir/mwimsg";

    private static final int MWI_ALL = 0;
    private static final int MWI_ID = 1;

    private static final UriMatcher URI_MATCHER = new UriMatcher(
            UriMatcher.NO_MATCH);
    static {
        URI_MATCHER.addURI("mwimsg", null, MWI_ALL);
        URI_MATCHER.addURI("mwimsg", "#", MWI_ID);
    }
}
