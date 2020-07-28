/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.mediatek.rcse.settings;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.SystemProperties;

import java.util.ArrayList;

/**
 * RCS settings provider.
 */
public class AppSettingsProvider extends ContentProvider {
    /**
     * Database table.
     */
    private static final String TABLE = "appsettings";

    // Create the constants used to differentiate between the different URI
    // requests
    private static final int APP_SETTINGS = 1;

    // Allocate the UriMatcher object, where a URI ending in 'settings'
    // will correspond to a request for all settings, and 'settings'
    // with a trailing '/[rowID]' will represent a single settings row.
    private static final UriMatcher uriMatcher = new UriMatcher(
            UriMatcher.NO_MATCH);
    static {
        uriMatcher.addURI("com.mediatek.rcs.appsettings", "appsettings",
                APP_SETTINGS);
    }

    /**
     * Database helper class.
     */
    private SQLiteOpenHelper mOpenHelper;

    /**
     * Database name.
     */
    public static final String DATABASE_NAME = "app_settings.db";

    /**
     * Helper class for opening, creating and managing database version control.
     */
    private static class DatabaseHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 86;

        private Context mCtx;

        /**
         * Instantiates a new database helper.
         *
         * @param ctx the ctx
         */
        public DatabaseHelper(Context ctx) {
            super(ctx, DATABASE_NAME, null, DATABASE_VERSION);

            this.mCtx = ctx;
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE " + TABLE + " (" + AppSettingsData.KEY_ID
                    + " integer primary key autoincrement,"
                    + AppSettingsData.KEY_KEY + " TEXT,"
                    + AppSettingsData.KEY_VALUE + " TEXT);");

            // Insert default values for parameters

            addParameter(db, AppSettingsData.PRESENCE_INVITATION_RINGTONE, "");
            addParameter(db, AppSettingsData.CSH_INVITATION_RINGTONE, "");
            addParameter(db, AppSettingsData.CSH_INVITATION_VIBRATE,
                    AppSettingsData.TRUE);
            addParameter(db, AppSettingsData.FILETRANSFER_INVITATION_RINGTONE,
                    "");
            addParameter(db, AppSettingsData.FILETRANSFER_INVITATION_VIBRATE,
                    AppSettingsData.TRUE);
            addParameter(db, AppSettingsData.CHAT_INVITATION_RINGTONE, "");
            addParameter(db, AppSettingsData.IMAGE_COMPRESS_SIZE, getDefaultCompressImageQuality());
            addParameter(db, AppSettingsData.CHAT_INVITATION_VIBRATE,
                    AppSettingsData.TRUE);
            addParameter(db, AppSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING,
                    AppSettingsData.FALSE);
            addParameter(db, AppSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING,
                    AppSettingsData.TRUE);
            addParameter(db, AppSettingsData.RCSE_COMPRESSING_IMAGE,
                    getDefaultCompressImageStatus());
            addParameter(db, AppSettingsData.RCSE_COMPRESSING_VIDEO,
                    getDefaultCompressImageStatus());
            addParameter(db, AppSettingsData.COMPRESS_IMAGE_HINT,
                    AppSettingsData.TRUE);
            addParameter(db, AppSettingsData.WARNING_LARGE_IMAGE_HINT,
                    AppSettingsData.TRUE);
            addParameter(db, AppSettingsData.RCSE_CHAT_WALLPAPER, "");
            addParameter(db,
                    AppSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED,
                    AppSettingsData.FALSE);
            addParameter(db, AppSettingsData.JOYN_DISABLE_STATUS, "0");
            addParameter(db, AppSettingsData.WARN_SF_SERVICE, AppSettingsData.FALSE);
            addParameter(db, AppSettingsData.IM_CAPABILITY_ALWAYS_ON, AppSettingsData.TRUE);

        }

        public static String getDefaultCompressImageStatus() {
            String status = AppSettingsData.FALSE;

            if (RcsSettings.getInstance().isSupportOP07()) {
                //needs to be extracted from API provided by GBA for SIM
                status = AppSettingsData.TRUE;
            }
            return status;
        }

        public static String getDefaultCompressImageQuality() {
            String status = "Best Available";
            if (RcsSettings.getInstance().isSupportOP07() || RcsSettings.getInstance().isSupportOP08()) {
                //needs to be extracted from API provided by GBA for SIM
                status = "Small(10%)";
            }
            return status;
        }

        /**
         * Add a parameter in the database.
         *
         * @param db            Database
         * @param key            Key
         * @param value            Value
         */
        private void addParameter(SQLiteDatabase db, String key, String value) {
            String sql = "INSERT INTO " + TABLE + " ("
                    + AppSettingsData.KEY_KEY + "," + AppSettingsData.KEY_VALUE
                    + ") VALUES ('" + key + "','" + value + "');";
            db.execSQL(sql);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion,
                int currentVersion) {
            // Get old data before deleting the table
            Cursor oldDataCursor = db.query(TABLE, null, null, null, null,
                    null, null);

            // Get all the pairs key/value of the old table to insert them back
            // after update
            ArrayList<ContentValues> valuesList = new ArrayList<ContentValues>();
            while (oldDataCursor.moveToNext()) {
                String key = null;
                String value = null;
                int index = oldDataCursor
                        .getColumnIndex(AppSettingsData.KEY_KEY);
                if (index != -1) {
                    key = oldDataCursor.getString(index);
                }
                index = oldDataCursor.getColumnIndex(AppSettingsData.KEY_VALUE);
                if (index != -1) {
                    value = oldDataCursor.getString(index);
                }
                if (key != null && value != null) {
                    ContentValues values = new ContentValues();
                    values.put(AppSettingsData.KEY_KEY, key);
                    values.put(AppSettingsData.KEY_VALUE, value);
                    valuesList.add(values);
                }
            }
            oldDataCursor.close();

            // Delete old table
            db.execSQL("DROP TABLE IF EXISTS " + TABLE);

            // Recreate table
            onCreate(db);

            // Put the old values back when possible
            for (int i = 0; i < valuesList.size(); i++) {
                ContentValues values = valuesList.get(i);
                String whereClause = AppSettingsData.KEY_KEY + "=" + "\""
                        + values.getAsString(AppSettingsData.KEY_KEY) + "\"";
                // Update the value with this key in the newly created database
                // If key is not present in the new version, this won't do
                // anything
                db.update(TABLE, values, whereClause, null);
            }
        }
    }

    @Override
    public boolean onCreate() {
        mOpenHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public String getType(Uri uri) {
        int match = uriMatcher.match(uri);
        switch (match) {
        case APP_SETTINGS:
            return "vnd.android.cursor.dir/com.orangelabs.rcs.appsettings";
        default:
            throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }

    @Override
    public Cursor query(Uri uri, String[] projectionIn, String selection,
            String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE);

        // Generate the body of the query
        int match = uriMatcher.match(uri);
        switch (match) {
        case APP_SETTINGS:
            break;
        default:
            throw new IllegalArgumentException("Unknown URI " + uri);
        }

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor c = qb.query(db, projectionIn, selection, selectionArgs, null,
                null, sort);

        // Register the contexts ContentResolver to be notified if
        // the cursor result set changes.
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }

        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String where,
            String[] whereArgs) {
        int count;
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();

        int match = uriMatcher.match(uri);
        switch (match) {
        case APP_SETTINGS:
            count = db.update(TABLE, values, where, null);
            break;
        default:
            throw new UnsupportedOperationException("Cannot update URI " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        throw new UnsupportedOperationException("Cannot insert URI " + uri);
    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        throw new UnsupportedOperationException();
    }
}
