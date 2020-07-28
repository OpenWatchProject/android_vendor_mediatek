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

package com.orangelabs.rcs.provider.eab;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.text.TextUtils;

import com.orangelabs.rcs.core.ims.service.ContactInfo;
import com.orangelabs.rcs.provider.settings.RcsSettingsProvider;
import com.orangelabs.rcs.provider.sharing.ImageSharingProvider;
import com.orangelabs.rcs.provider.sharing.VideoSharingProvider;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Rich address book provider
 *
 * <br>This provider contains the list of the RCS contacts and their status
 * <br>It is used by the AddressBookManager to keep the synchronization between the native address book and the RCS contacts.
 * 
 * <br>It also contains the list of aggregations between native raw contacts and rcs raw contacts
 */
public class ContactsBackup extends ContentProvider {
    // Database table
    public static final String CONTACTS_TABLE = "cb";
    
    // Create the constants used to differentiate between the different URI requests
    private static final int CONTACTS = 1;
    private static final int CONTACT_ID=2;
    
    // Allocate the UriMatcher object
    private static final UriMatcher uriMatcher;
    static {
        uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        uriMatcher.addURI("com.orangelabs.rcs.cb", "cb", CONTACTS);
        uriMatcher.addURI("com.orangelabs.rcs.cb", "cb/#", CONTACT_ID);
        
    }

    /**
     * Database helper class
     */
    private DatabaseHelper openHelper;
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    /**
     * Helper class for opening, creating and managing database version control
     */
    private static class DatabaseHelper extends SQLiteOpenHelper{
        
        private static final String DATABASE_NAME = "cb.db";
        private static final int DATABASE_VERSION = 21;
        
        public DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }
            
        @Override
        public void onCreate(SQLiteDatabase db) {
            // Create the eab_contacts table
            createDb(db);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            db.execSQL("DROP TABLE IF EXISTS " + CONTACTS_TABLE);
            onCreate(db);
        }

        private void createDb(SQLiteDatabase db) {
            
            db.execSQL("CREATE TABLE IF NOT EXISTS " + CONTACTS_TABLE + " ("
                    + ContactsBackupData.KEY_ID + " integer primary key autoincrement, "
                    + ContactsBackupData.KEY_CONTACT_NAME + " TEXT, "
                    + ContactsBackupData.KEY_CONTACT_NUMBER + " long)");
        }
    }

    @Override 
    public boolean onCreate() {
        openHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        int count = 0;

        SQLiteDatabase db = openHelper.getWritableDatabase();
        switch(uriMatcher.match(uri)){
            case CONTACTS:
                count = db.delete(CONTACTS_TABLE, where, whereArgs);
                break;
            case CONTACT_ID:
                String segment = uri.getPathSegments().get(1);
                count = db.delete(CONTACTS_TABLE, RichAddressBookData.KEY_ID + "="
                        + segment
                        + (!TextUtils.isEmpty(where) ? " AND ("    + where + ')' : ""),
                        whereArgs);
                break;
            default:
                throw new IllegalArgumentException("Unsupported URI: " + uri);
        }

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public String getType(Uri uri) {
        switch(uriMatcher.match(uri)){
            case CONTACTS:
                return "vnd.android.cursor.item/cb";
            case CONTACT_ID:
                return "vnd.android.cursor.item/cb";    
            default:
                throw new IllegalArgumentException("Unsupported URI " + uri);
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        switch(uriMatcher.match(uri)){
            case CONTACTS:
            case CONTACT_ID:    
                // Insert the new row, will return the row number if successful
                long rowID = db.insert(CONTACTS_TABLE, null, initialValues);
                // Return a URI to the newly inserted row on success
                if (rowID > 0) {
                    Uri newUri = ContentUris.withAppendedId(AggregationData.CONTENT_URI, rowID);
                    getContext().getContentResolver().notifyChange(newUri, null);
                    return newUri;
                }
                
                break;
            
        }
        
        throw new SQLException("Failed to insert row into " + uri);
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

        String orderBy = sort;
        
        // Generate the body of the query
        int match = uriMatcher.match(uri);
        switch(match) {
            case CONTACTS:
                qb.setTables(CONTACTS_TABLE);
                if (TextUtils.isEmpty(sort)){
                    orderBy = ContactsBackupData.KEY_CONTACT_NUMBER;
                }
                break;
            case CONTACT_ID:
                qb.setTables(CONTACTS_TABLE);
                qb.appendWhere(ContactsBackupData.KEY_ID + "=" + uri.getPathSegments().get(1));
                if (TextUtils.isEmpty(sort)){
                    orderBy = RichAddressBookData.KEY_CONTACT_NUMBER;
                }
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        // Apply the query to the underlying database.
        SQLiteDatabase db = openHelper.getWritableDatabase();
        Cursor c = qb.query(db, 
                projection, 
                selection, selectionArgs, 
                null, null,
                orderBy);

        // Register the contexts ContentResolver to be notified if
        // the cursor result set changes.
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }
        
        // Return a cursor to the query result
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        int count = 0;
        SQLiteDatabase db = openHelper.getWritableDatabase();

        int match = uriMatcher.match(uri);
        switch (match) {
            case CONTACTS:
                count = db.update(CONTACTS_TABLE, values, where, whereArgs);
                break;
            case CONTACT_ID:
                String segment = uri.getPathSegments().get(1);
                count = db.update(CONTACTS_TABLE, values, ContactsBackupData.KEY_ID + "="
                        + segment
                        + (!TextUtils.isEmpty(where) ? " AND (" + where + ')' : ""),
                        whereArgs);
                break;    
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        if (uriMatcher.match(uri) != CONTACT_ID) {
            throw new IllegalArgumentException("URI not supported for directories");
        }
        
        try {
            return this.openFileHelper(uri, mode);
        } catch (FileNotFoundException e) {
            if (logger.isActivated()) {
                logger.error("File not found exception", e);
            }
            throw new FileNotFoundException();
        }
    }

    static final String DATABASE_LOCATION = "/data/" + "com.orangelabs.rcs" + "/databases/";
    public static void backupContactsDatabase(String account) {
        try {
            String dbFile = Environment.getDataDirectory() + DATABASE_LOCATION + DatabaseHelper.DATABASE_NAME;

            File backupFileRoot = new File(Environment.getDataDirectory() + DATABASE_LOCATION);
            backupFileRoot.mkdirs();
            File backupFile = new File(backupFileRoot, DatabaseHelper.DATABASE_NAME + "_" + account + ".db");
            OutputStream outStream = new FileOutputStream(backupFile, false);
            InputStream inStream = new FileInputStream(dbFile);
            byte[] buffer = new byte[1024];
            int length;
            while ((length = inStream.read(buffer)) > 0) {
                outStream.write(buffer, 0, length);
            }
            outStream.flush();
            outStream.close();
            inStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void restoreContactsDatabase(String account) {
        try {
            String dbFile = Environment.getDataDirectory() + DATABASE_LOCATION + DatabaseHelper.DATABASE_NAME;
            File backupFileRoot = new File(Environment.getDataDirectory() + DATABASE_LOCATION);
            String restoreFile = backupFileRoot + "/" + ContactsBackup.DatabaseHelper.DATABASE_NAME + "_" + account + ".db";

            File file = new File(restoreFile);
            if (!file.exists()) {
                return;
            }

            //delete the original file
            File dbOriginalFile = new File(dbFile);
            if (!dbOriginalFile.exists()) {
                dbOriginalFile.createNewFile();
            }

            new FileOutputStream(dbFile, false).close();
            OutputStream outStream = new FileOutputStream(dbFile, false);
            InputStream inStream = new FileInputStream(file);
            byte[] buffer = new byte[1024];
            int length;
            while ((length = inStream.read(buffer)) > 0) {
                outStream.write(buffer, 0, length);
            }
            outStream.flush();
            outStream.close();
            inStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
