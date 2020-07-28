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
 */
package com.mediatek.vcalendar.database;

import java.util.List;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract.Calendars;

import com.mediatek.vcalendar.SingleComponentContentValues;
import com.mediatek.vcalendar.SingleComponentCursorInfo;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * Provide database operations for parsing and composing components.
 *
 */
public class DatabaseHelper {
    private static final boolean DEBUG = false;
    private static final String TAG = "DatabaseHelper";

    private final Context mContext;
    private final ContentResolver mContentResolver;

    private ComponentInfoHelper mComponentInfoHelper;

    /**
     * Default Calendar account.
     */
    public static final String ACCOUNT_PC_SYNC = "PC Sync";
    private static final String COLUMN_ID = "_id";

    public DatabaseHelper(Context context) {
        mContext = context;
        mContentResolver = mContext.getContentResolver();
    }

    /**
     * Insert content values into database based on component type.
     *
     * @param sccv
     *            containing component information and content values
     * @return an Uri if succeed, null otherwise.
     */
    public Uri insert(SingleComponentContentValues sccv) {
        if (sccv == null) {
            return null;
        }
        LogUtil.i(TAG, "insert()");

        ComponentInsertHelper cih = ComponentInsertHelper.buildInsertHelper(
                mContext, sccv);
        if (cih == null) {
            LogUtil.e(TAG, "insert(): NOT supported component type");

            return null;
        }

        return cih.insertContentValues(sccv);
    }

    /**
     * Insert content values into database based on component type.
     *
     * @param mutiContentValues
     *            containing component information and content values
     * @return an Uri if succeed, null otherwise.
     */
    public Uri insert(List<SingleComponentContentValues> multiComponentContentValues) {
        LogUtil.i(TAG, "insert() multiComponentContentValues");
        if (multiComponentContentValues.size() <= 0) {
            return null;
        }
        SingleComponentContentValues sccv = multiComponentContentValues.get(0);

        ComponentInsertHelper cih = ComponentInsertHelper.buildInsertHelper(
                mContext, sccv);
        if (cih == null) {
            LogUtil.e(TAG, "insert(): NOT supported component type");
            return null;
        }

        return cih.insertMultiComponentContentValues(multiComponentContentValues);
    }

    /**
     * Query component information from database.
     *
     * @param uri
     *            query URI
     * @param projection
     *            the projection
     * @param selection
     *            the query selection
     * @param selectionArgs
     *            the selection arguments
     * @param sortOrder
     *            sort order
     * @return true if query succeed and the cursor not empty, false otherwise.
     */
    public boolean query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        LogUtil.i(TAG, "query(): selection = \"" + selection + "\"");

        /*
         * We use ComponentInfoHelper to do query and fill one
         * SingleComponentCursorInfo for a specific component.
         */
        mComponentInfoHelper = ComponentInfoHelper.createComponentInfoHelper(
                mContext, uri.toString());
        if (mComponentInfoHelper == null) {
            LogUtil.e(TAG, "query(): NOT supported URI: " + uri.toString());

            return false;
        }

        return mComponentInfoHelper.query(uri, projection, selection,
                selectionArgs, sortOrder);
    }

    /**
     * return components count.
     *
     * @return the count
     */
    public int getComponentCount() {
        if (mComponentInfoHelper == null) {
            LogUtil.e(TAG, "getComponentCount(): MUST query first");

            return -1;
        }

        return mComponentInfoHelper.getComponentCount();
    }

    /**
     * See if has next component.
     *
     * @return true if more components, false otherwise
     */
    public boolean hasNextComponentInfo() {
        if (mComponentInfoHelper == null) {
            LogUtil.e(TAG, "hasNextComponentInfo(): MUST query first");

            return false;
        }

        return mComponentInfoHelper.hasNextComponentInfo();
    }

    /**
     * Do movement at the end, because the cursor has point to the row should be
     * handled
     *
     * @return the Event Info
     */
    public SingleComponentCursorInfo getNextComponentInfo() {
        if (mComponentInfoHelper == null) {
            LogUtil.e(TAG, "getNextComponentInfo(): MUST query first");

            return null;
        }

        return mComponentInfoHelper.getNextComponentInfo();
    }

    /**
     * return the gained calendarIds for the query
     *
     * @return the CalendarId list
     */
    public long getCalendarIdForAccount(String account) {
        LogUtil.i(TAG, "getCalendarIdForAccount()");

        long calendarId = -1;

        if (ACCOUNT_PC_SYNC.equals(account)) {
            calendarId = 1;
        } else {
            String select = Calendars.ACCOUNT_NAME + "=\"" + account + "\"";
            if (DEBUG) {
                LogUtil.d(TAG, "getCalendarIdForAccount(): Select = " + select);
            }
            Cursor cur = mContentResolver.query(Calendars.CONTENT_URI, null,
                    select, null, null);

            try {
                if (cur != null && cur.getCount() > 0) {
                    // only choose the account's first calendar?
                    cur.moveToFirst();
                    calendarId = cur.getLong(cur.getColumnIndexOrThrow(COLUMN_ID));
                }
            } finally {
                if (cur != null) {
                    cur.close();
                }
            }
        }

        return calendarId;
    }
}
