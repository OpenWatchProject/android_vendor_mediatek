/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.mediatek.cellbroadcastreceiver;

import android.content.ComponentName;
import android.content.ContentProvider;
import android.content.ContentProviderClient;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.Telephony;
import android.provider.BaseColumns;
import android.telephony.CellBroadcastMessage;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.text.TextUtils;
import android.util.Log;

import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkCellBroadcastMessage;

/**
 * ContentProvider for the database of received cell broadcasts.
 */
public class CellBroadcastContentProvider extends ContentProvider {
    private static final String TAG = "CellBroadcastContentProvider";

    /** URI matcher for ContentProvider queries. */
    private static final UriMatcher sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);

    /** Authority string for content URIs. */
    static final String CB_AUTHORITY = "com.mediatek.cellbroadcastreceiver.cellbroadcasts";

    /** Content URI for notifying observers. */
    static final Uri CONTENT_URI = Uri.parse("content://com.mediatek.cellbroadcastreceiver.cellbroadcasts/");

    /** URI matcher type to get all cell broadcasts. */
    private static final int CB_ALL = 0;

    /** URI matcher type to get a cell broadcast by ID. */
    private static final int CB_ALL_ID = 1;

    /** MIME type for the list of all cell broadcasts. */
    private static final String CB_LIST_TYPE = "vnd.android.cursor.dir/cellbroadcast";

    /** MIME type for an individual cell broadcast. */
    private static final String CB_TYPE = "vnd.android.cursor.item/cellbroadcast";

    static {
        sUriMatcher.addURI(CB_AUTHORITY, null, CB_ALL);
        sUriMatcher.addURI(CB_AUTHORITY, "#", CB_ALL_ID);
    }

    /**
     * Table Contains received SMS cell broadcast messages.
     * @hide
     */
    public static final class CMASCellBroadcasts implements BaseColumns {

        /**
         * Not instantiable.
         * @hide
         */
        private CMASCellBroadcasts() {}

        /**
         * The {@code content://} URI for this table.
         */
        public static final Uri CONTENT_URI = Uri.parse("content://com.mediatek.cellbroadcastreceiver.cellbroadcasts");

        /**
         * Message geographical scope.
         * <P>Type: INTEGER</P>
         */
        public static final String GEOGRAPHICAL_SCOPE = "geo_scope";

        /**
         * Message serial number.
         * <P>Type: INTEGER</P>
         */
        public static final String SERIAL_NUMBER = "serial_number";

        /**
         * PLMN of broadcast sender. {@code SERIAL_NUMBER + PLMN + LAC + CID} uniquely identifies
         * a broadcast for duplicate detection purposes.
         * <P>Type: TEXT</P>
         */
        public static final String PLMN = "plmn";

        /**
         * Location Area (GSM) or Service Area (UMTS) of broadcast sender. Unused for CDMA.
         * Only included if Geographical Scope of message is not PLMN wide (01).
         * <P>Type: INTEGER</P>
         */
        public static final String LAC = "lac";

        /**
         * Cell ID of message sender (GSM/UMTS). Unused for CDMA. Only included when the
         * Geographical Scope of message is cell wide (00 or 11).
         * <P>Type: INTEGER</P>
         */
        public static final String CID = "cid";

        /**
         * Message code. <em>OBSOLETE: merged into SERIAL_NUMBER.</em>
         * <P>Type: INTEGER</P>
         */
        public static final String V1_MESSAGE_CODE = "message_code";

        /**
         * Message identifier. <em>OBSOLETE: renamed to SERVICE_CATEGORY.</em>
         * <P>Type: INTEGER</P>
         */
        public static final String V1_MESSAGE_IDENTIFIER = "message_id";

        /**
         * Service category (GSM/UMTS: message identifier; CDMA: service category).
         * <P>Type: INTEGER</P>
         */
        public static final String SERVICE_CATEGORY = "service_category";

        /**
         * Message language code.
         * <P>Type: TEXT</P>
         */
        public static final String LANGUAGE_CODE = "language";

        /**
         * Message body.
         * <P>Type: TEXT</P>
         */
        public static final String MESSAGE_BODY = "body";

        /**
         * Message delivery time.
         * <P>Type: INTEGER (long)</P>
         */
        public static final String DELIVERY_TIME = "date";

        /**
         * Has the message been viewed?
         * <P>Type: INTEGER (boolean)</P>
         */
        public static final String MESSAGE_READ = "read";

        // MTK-START
        /**
         * Sepcify SIM identity for cell braodcast message
         * The sim card id the messge come from.
         * <p>Type: INTEGER</p>
         *
         * @hide
         */
        public static final String SUB_ID = "sub_id";
        // MTK-END

        /**
         * Message format (3GPP or 3GPP2).
         * <P>Type: INTEGER</P>
         */
        public static final String MESSAGE_FORMAT = "format";

        /**
         * Message priority (including emergency).
         * <P>Type: INTEGER</P>
         */
        public static final String MESSAGE_PRIORITY = "priority";

        /**
         * ETWS warning type (ETWS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String ETWS_WARNING_TYPE = "etws_warning_type";

        /**
         * CMAS message class (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_MESSAGE_CLASS = "cmas_message_class";

        /**
         * CMAS category (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_CATEGORY = "cmas_category";

        /**
         * CMAS response type (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_RESPONSE_TYPE = "cmas_response_type";

        /**
         * CMAS severity (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_SEVERITY = "cmas_severity";

        /**
         * CMAS urgency (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_URGENCY = "cmas_urgency";

        /**
         * CMAS certainty (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_CERTAINTY = "cmas_certainty";

        /**
         * CMAS message dispalyed or not (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_DISPLAYED = "message_dispalyed";

        /**
         * CMAS location timer (CMAS alerts only).
         * <P>Type: INTEGER</P>
         */
        public static final String CMAS_TIMER = "location_timer";

        /** The default sort order for this table. */
        public static final String DEFAULT_SORT_ORDER = DELIVERY_TIME + " DESC";

        /**
         * Query columns for instantiating {@link android.telephony.CellBroadcastMessage} objects.
         */
        public static final String[] QUERY_COLUMNS = {
                _ID,
                GEOGRAPHICAL_SCOPE,
                PLMN,
                LAC,
                CID,
                SERIAL_NUMBER,
                SERVICE_CATEGORY,
                LANGUAGE_CODE,
                MESSAGE_BODY,
                DELIVERY_TIME,
                MESSAGE_READ,
                SUB_ID, // MTK-add
                MESSAGE_FORMAT,
                MESSAGE_PRIORITY,
                ETWS_WARNING_TYPE,
                CMAS_MESSAGE_CLASS,
                CMAS_CATEGORY,
                CMAS_RESPONSE_TYPE,
                CMAS_SEVERITY,
                CMAS_URGENCY,
                CMAS_CERTAINTY
        };
    }
    /* Database Table end */
    
    
    /** The database for this content provider. */
    private CellBroadcastDatabaseHelper mOpenHelper;

    /**
     * Initialize content provider.
     * @return true if the provider was successfully loaded, false otherwise
     */
    @Override
    public boolean onCreate() {
        mOpenHelper = new CellBroadcastDatabaseHelper(getContext());
        return true;
    }

    /**
     * Return a cursor for the cell broadcast table.
     * @param uri the URI to query.
     * @param projection the list of columns to put into the cursor, or null.
     * @param selection the selection criteria to apply when filtering rows, or null.
     * @param selectionArgs values to replace ?s in selection string.
     * @param sortOrder how the rows in the cursor should be sorted, or null to sort from most
     *  recently received to least recently received.
     * @return a Cursor or null.
     */
    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(CellBroadcastDatabaseHelper.TABLE_NAME);

        int match = sUriMatcher.match(uri);
        switch (match) {
            case CB_ALL:
                // get all broadcasts
                qb.appendWhere('(' + CMASCellBroadcasts.CMAS_DISPLAYED + "=1)");
                break;

            case CB_ALL_ID:
                // get broadcast by ID
                qb.appendWhere("(_id=" + uri.getPathSegments().get(0) + ')');
                break;

            default:
                Log.e(TAG, "Invalid query: " + uri);
                throw new IllegalArgumentException("Unknown URI: " + uri);
        }

        String orderBy;
        if (!TextUtils.isEmpty(sortOrder)) {
            orderBy = sortOrder;
        } else {
            orderBy = Telephony.CellBroadcasts.DEFAULT_SORT_ORDER;
        }

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), CONTENT_URI);
            //notifyUnreadCount();
        }
        return c;
    }

    /**
     * Return the MIME type of the data at the specified URI.
     * @param uri the URI to query.
     * @return a MIME type string, or null if there is no type.
     */
    @Override
    public String getType(Uri uri) {
        int match = sUriMatcher.match(uri);
        switch (match) {
            case CB_ALL:
                return CB_LIST_TYPE;

            case CB_ALL_ID:
                return CB_TYPE;

            default:
                return null;
        }
    }

    /**
     * Insert a new row. This throws an exception, as the database can only be modified by
     * calling custom methods in this class, and not via the ContentProvider interface.
     * @param uri the content:// URI of the insertion request.
     * @param values a set of column_name/value pairs to add to the database.
     * @return the URI for the newly inserted item.
     */
    @Override
    public Uri insert(Uri uri, ContentValues values) {
        throw new UnsupportedOperationException("insert not supported");
    }

    /**
     * Delete one or more rows. This throws an exception, as the database can only be modified by
     * calling custom methods in this class, and not via the ContentProvider interface.
     * @param uri the full URI to query, including a row ID (if a specific record is requested).
     * @param selection an optional restriction to apply to rows when deleting.
     * @return the number of rows affected.
     */
    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("delete not supported");
    }

    /**
     * Update one or more rows. This throws an exception, as the database can only be modified by
     * calling custom methods in this class, and not via the ContentProvider interface.
     * @param uri the URI to query, potentially including the row ID.
     * @param values a Bundle mapping from column names to new column values.
     * @param selection an optional filter to match rows to update.
     * @return the number of rows affected.
     */
    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("update not supported");
    }

    /**
     * Internal method to insert a new Cell Broadcast into the database and notify observers.
     * @param message the message to insert
     * @return true if the broadcast is new, false if it's a duplicate broadcast.
     */
    boolean insertNewBroadcast(CellBroadcastMessage message) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        ContentValues cv = message.getContentValues();

        // Note: this method previously queried the database for duplicate message IDs, but this
        // is not compatible with CMAS carrier requirements and could also cause other emergency
        // alerts, e.g. ETWS, to not display if the database is filled with old messages.
        // Use duplicate message ID detection in CellBroadcastAlertService instead of DB query.

        long rowId = db.insert(CellBroadcastDatabaseHelper.TABLE_NAME, null, cv);
        if (rowId == -1) {
            Log.e(TAG, "failed to insert new broadcast into database");
            // Return true on DB write failure because we still want to notify the user.
            // The CellBroadcastMessage will be passed with the intent, so the message will be
            // displayed in the emergency alert dialog, or the dialog that is displayed when
            // the user selects the notification for a non-emergency broadcast, even if the
            // broadcast could not be written to the database.
        }
        return true;    // broadcast is not a duplicate
    }

    long addNewBroadcast(CellBroadcastMessage message, int subId) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String queryStr = Telephony.CellBroadcasts.SERIAL_NUMBER + "=" + message.getSerialNumber()
        + " AND " + Telephony.CellBroadcasts.SERVICE_CATEGORY + "=" + message.getServiceCategory()
        + " AND " + CMASCellBroadcasts.CMAS_DISPLAYED + "=" + 2;

        String[] column = {"_id"};
        Cursor cursor  = db.query(CellBroadcastDatabaseHelper.TABLE_NAME,
            column, queryStr, null, null, null, null);
        //update not display as display;
        if (cursor != null && cursor.moveToFirst()) {
            long rowId = cursor.getLong(cursor.getColumnIndexOrThrow("_id"));
            ContentValues cv = new ContentValues();
            cv.put(CMASCellBroadcasts.CMAS_DISPLAYED, 1);
            cv.put(MtkTelephony.MtkCellBroadcasts.SUBSCRIPTION_ID, subId);
            String whereCluse = "_id=?";
            String[] whereArgs = new String[]{Long.toString(rowId)};
            int rowCount = db.update(CellBroadcastDatabaseHelper.TABLE_NAME,
                                     cv, whereCluse, whereArgs);
            if (rowCount != 1) {
                Log.w(TAG, "updateOldBroadcast fail rowId = " + rowId);
                return rowId;
            }
            Log.d(TAG, "update displayed message success rowId = " + rowId);
            return rowId;
        }

        ContentValues cv = message.getContentValues();
        cv.put(MtkTelephony.MtkCellBroadcasts.SUBSCRIPTION_ID, subId);
        cv.put(CMASCellBroadcasts.CMAS_DISPLAYED, 1);
        // Note: this method previously queried the database for duplicate message IDs, but this
        // is not compatible with CMAS carrier requirements and could also cause other emergency
        // alerts, e.g. ETWS, to not display if the database is filled with old messages.
        // Use duplicate message ID detection in CellBroadcastAlertService instead of DB query.

        long rowId = db.insert(CellBroadcastDatabaseHelper.TABLE_NAME, null, cv);
        if (rowId == -1) {
            Log.e(TAG, "failed to insert new broadcast into database");
            // Return true on DB write failure because we still want to notify the user.
            // The CellBroadcastMessage will be passed with the intent, so the message will be
            // displayed in the emergency alert dialog, or the dialog that is displayed when
            // the user selects the notification for a non-emergency broadcast, even if the
            // broadcast could not be written to the database.
        }
        return rowId;    // broadcast is not a duplicate
    }

    long addMessageAsNotDisplayed(MtkCellBroadcastMessage message) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();

        String queryStr = Telephony.CellBroadcasts.SERIAL_NUMBER + "=" + message.getSerialNumber()
        + " AND " + Telephony.CellBroadcasts.SERVICE_CATEGORY + "=" + message.getServiceCategory();

        String[] column = {"_id"};
        Cursor cursor  = db.query(CellBroadcastDatabaseHelper.TABLE_NAME,
            column, queryStr, null, null, null, null);

        if (cursor != null && cursor.moveToFirst()) {
            long ret = cursor.getLong(cursor.getColumnIndexOrThrow("_id"));
            Log.e(TAG, "already add this not displayed msg");
            return ret;
        }

        ContentValues cv = message.getContentValues();

        cv.put(CMASCellBroadcasts.CMAS_DISPLAYED, 2);
        cv.put(CMASCellBroadcasts.CMAS_TIMER, message.getMaxWaitTime());

        // Note: this method previously queried the database for duplicate message IDs, but this
        // is not compatible with CMAS carrier requirements and could also cause other emergency
        // alerts, e.g. ETWS, to not display if the database is filled with old messages.
        // Use duplicate message ID detection in CellBroadcastAlertService instead of DB query.

        long rowId = db.insert(CellBroadcastDatabaseHelper.TABLE_NAME, null, cv);
        if (rowId == -1) {
            Log.e(TAG, "failed to insert new broadcast into database");
            // Return true on DB write failure because we still want to notify the user.
            // The CellBroadcastMessage will be passed with the intent, so the message will be
            // displayed in the emergency alert dialog, or the dialog that is displayed when
            // the user selects the notification for a non-emergency broadcast, even if the
            // broadcast could not be written to the database.
        }
        return rowId;    // broadcast is not a duplicate
    }

    boolean updateMessageDisplayed(long rowId, int displayed) {
        Log.i(TAG, "updateOldBroadcast, rowId = " + rowId);
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        ContentValues cv = new ContentValues();
        String whereCluse = "_id=?";
        String[] whereArgs = new String[]{Long.toString(rowId)};
        int rowCount = db.update(CellBroadcastDatabaseHelper.TABLE_NAME, cv, whereCluse, whereArgs);
        if (rowCount != 1) {
            Log.w(TAG, "updateOldBroadcast fail");
            return false;
        }

        return true;
    }

    boolean updateOldBroadcast(CellBroadcastMessage message, long rowId) {
        Log.i(TAG, "updateOldBroadcast, rowId = " + rowId);
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        ContentValues cv = message.getContentValues();

        String whereCluse = "_id=?";
        String[] whereArgs = new String[]{Long.toString(rowId)};
        int rowCount = db.update(CellBroadcastDatabaseHelper.TABLE_NAME, cv, whereCluse, whereArgs);
        if (rowCount != 1) {
            Log.w(TAG, "updateOldBroadcast fail");
            return false;
        }

        return true;
    }

    long getRowId(CellBroadcastMessage msg) {
        long ret = -1;
//
//        int lac = -1;
//        int cid = -1;
//
//        SmsCbLocation location = msg.getLocation();
//        String plmn = location.getPlmn();
//        lac = location.getLac();
//        cid = location.getCid();

        String queryStr = Telephony.CellBroadcasts.SERIAL_NUMBER + "=" + msg.getSerialNumber()
        + " AND " + Telephony.CellBroadcasts.SERVICE_CATEGORY + "=" + msg.getServiceCategory()
        + " AND " + Telephony.CellBroadcasts.LANGUAGE_CODE + "='" + msg.getLanguageCode() + "'"
        + " AND " + Telephony.CellBroadcasts.MESSAGE_BODY + "='" + msg.getMessageBody() + "'";

//        StringBuilder sb = new StringBuilder(queryStr);
//
//        if (plmn != null) {
//            sb.append(" AND " + Telephony.CellBroadcasts.PLMN + "='" + plmn + "'");
//        }
//
//        if (lac != -1) {
//            sb.append(" AND " + Telephony.CellBroadcasts.LAC + "=" + lac);
//        }
//
//        if (cid != -1) {
//            sb.append(" AND " + Telephony.CellBroadcasts.CID + "=" + cid);
//        }
//
//        queryStr = sb.toString();

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String[] column = {"_id"};
        Cursor cursor  = db.query(CellBroadcastDatabaseHelper.TABLE_NAME, column, queryStr, null, null, null, null);

        if (cursor != null && cursor.moveToFirst()) {
            ret = cursor.getLong(cursor.getColumnIndexOrThrow("_id"));
        }
        return ret;
    }

    /**
     * Internal method to delete a cell broadcast by row ID and notify observers.
     * @param rowId the row ID of the broadcast to delete
     * @return true if the database was updated, false otherwise
     */
    boolean deleteBroadcast(long rowId) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();

        int rowCount = db.delete(CellBroadcastDatabaseHelper.TABLE_NAME,
                Telephony.CellBroadcasts._ID + "=?",
                new String[]{Long.toString(rowId)});
        if (rowCount != 0) {
            return true;
        } else {
            Log.e(TAG, "failed to delete broadcast at row " + rowId);
            return false;
        }
    }

    /**
     * Internal method to delete all cell broadcasts and notify observers.
     * @return true if the database was updated, false otherwise
     */
    boolean deleteAllBroadcasts() {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();

        int rowCount = db.delete(CellBroadcastDatabaseHelper.TABLE_NAME,
            CMASCellBroadcasts.CMAS_DISPLAYED + "=?", new String[]{Long.toString(1)});
        if (rowCount != 0) {
            return true;
        } else {
            Log.e(TAG, "failed to delete all broadcasts");
            return false;
        }
    }

    /**
     * Internal method to mark a broadcast as read and notify observers. The broadcast can be
     * identified by delivery time (for new alerts) or by row ID. The caller is responsible for
     * decrementing the unread non-emergency alert count, if necessary.
     *
     * @param columnName the column name to query (ID or delivery time)
     * @param columnValue the ID or delivery time of the broadcast to mark read
     * @return true if the database was updated, false otherwise
     */
    boolean markBroadcastRead(String columnName, long columnValue) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();

        ContentValues cv = new ContentValues(1);
        cv.put(Telephony.CellBroadcasts.MESSAGE_READ, 1);

        String whereClause = columnName + "=?";
        String[] whereArgs = new String[]{Long.toString(columnValue)};

        int rowCount = db.update(CellBroadcastDatabaseHelper.TABLE_NAME, cv, whereClause, whereArgs);
        if (rowCount != 0) {
            return true;
        } else {
            Log.e(TAG, "failed to mark broadcast read: " + columnName + " = " + columnValue);
            return false;
        }
    }

    public int getUnreadCellBroadcastCount() {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        Cursor cursor = db.query(CellBroadcastDatabaseHelper.TABLE_NAME, null, Telephony.CellBroadcasts.MESSAGE_READ + " =0",
                null, null, null, null);
        int count = 0;
        if (cursor != null) {
            count = cursor.getCount();
            cursor.close();
        }
        return count;
    }

    public Cursor getCellBroadcastCursorByTriggerMessage(String where) {

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        Cursor cursor = db.query(CellBroadcastDatabaseHelper.TABLE_NAME,
            null, where, null, null, null,
                null);
        return cursor;
    }

    public Cursor getAllCellBroadcastCursor() {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String queryStr = CMASCellBroadcasts.CMAS_DISPLAYED + "=" + 1;
        Cursor cursor = db.query(CellBroadcastDatabaseHelper.TABLE_NAME,
                Telephony.CellBroadcasts.QUERY_COLUMNS, queryStr, null, null, null,
                Telephony.CellBroadcasts.DELIVERY_TIME + " DESC");
        return cursor;
    }

/* remove as the feature had been removed
    public void notifyUnreadCount() {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_UNREAD_CHANGED);
        intent.putExtra(Intent.EXTRA_UNREAD_NUMBER, getUnreadCellBroadcastCount());
        intent.putExtra(Intent.EXTRA_UNREAD_COMPONENT, new ComponentName(
                "com.mediatek.cellbroadcastreceiver",
                "com.mediatek.cellbroadcastreceiver.CellBroadcastListActivity"));
        getContext().sendBroadcast(intent);
    }
*/
    /** Callback for users of AsyncCellBroadcastOperation. */
    interface CellBroadcastOperation {
        /**
         * Perform an operation using the specified provider.
         * @param provider the CellBroadcastContentProvider to use
         * @return true if any rows were changed, false otherwise
         */
        boolean execute(CellBroadcastContentProvider provider);
    }

    /**
     * Async task to call this content provider's internal methods on a background thread.
     * The caller supplies the CellBroadcastOperation object to call for this provider.
     */
    static class AsyncCellBroadcastTask extends AsyncTask<CellBroadcastOperation, Void, Void> {
        /** Reference to this app's content resolver. */
        private ContentResolver mContentResolver;

        AsyncCellBroadcastTask(ContentResolver contentResolver) {
            mContentResolver = contentResolver;
        }

        /**
         * Perform a generic operation on the CellBroadcastContentProvider.
         * @param params the CellBroadcastOperation object to call for this provider
         * @return void
         */
        @Override
        protected Void doInBackground(CellBroadcastOperation... params) {
            ContentProviderClient cpc = mContentResolver.acquireContentProviderClient(
                    CellBroadcastContentProvider.CB_AUTHORITY);
            if (cpc == null) {
                Log.d(TAG, "doInBackground() cpc is null");
                mContentResolver = null;
                return null;
            }
            CellBroadcastContentProvider provider = (CellBroadcastContentProvider)
                    cpc.getLocalContentProvider();

            if (provider != null) {
                try {
                    boolean changed = params[0].execute(provider);
                    if (changed) {
                        Log.d(TAG, "database changed: notifying observers...");
                        mContentResolver.notifyChange(CONTENT_URI, null, false);
                        //provider.notifyUnreadCount();
                    }
                } finally {
                    cpc.release();
                }
            } else {
                Log.e(TAG, "getLocalContentProvider() returned null");
            }

            mContentResolver = null;    // free reference to content resolver
            return null;
        }
    }
}
