package com.orangelabs.rcs.provider.messaging;

import java.io.File;
import java.io.FileInputStream;
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
import android.text.TextUtils;

/**
 * File transfer content provider
 * 
 * @author Jean-Marc AUFFRET
 */
public class MultiFileTransferProvider extends ContentProvider {
    /**
     * Database table
     */
    private static final String TABLE = "multift";

    // Create the constants used to differentiate between the different URI requests
    private static final int FILETRANSFERS = 1;
    private static final int FILETRANSFER_ID = 2;
    private static final int RCSAPI = 3;
    private static final int RCSAPI_ID = 4;

    // Allocate the UriMatcher object
    private static final UriMatcher uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        uriMatcher.addURI("com.orangelabs.rcs.multift", "multift", FILETRANSFERS);
        uriMatcher.addURI("com.orangelabs.rcs.multift", "multift/#", FILETRANSFER_ID);
        uriMatcher.addURI("com.gsma.services.rcs.provider.multift", "multift", RCSAPI);
        uriMatcher.addURI("com.gsma.services.rcs.provider.multift", "multift/#", RCSAPI_ID);
    }

    /**
     * Database helper class
     */
    private SQLiteOpenHelper openHelper;
    
    /**
     * Database name
     */
    public static final String DATABASE_NAME = "multift.db";

    /**
     * Helper class for opening, creating and managing database version control
     */
    private static class DatabaseHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 4;

        public DatabaseHelper(Context ctx) {
            super(ctx, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE " + TABLE + " ("
                    + MultiFileTransferData.KEY_ID + " integer primary key autoincrement,"
                    + MultiFileTransferData.KEY_FT_ID + " TEXT,"
                    + MultiFileTransferData.KEY_CONTACT + " TEXT,"
                    + MultiFileTransferData.KEY_NAME + " TEXT,"
                    + MultiFileTransferData.KEY_CHAT_ID + " TEXT,"
                    + MultiFileTransferData.KEY_MSG_ID + " TEXT,"
                    + MultiFileTransferData.KEY_MIME_TYPE + " TEXT,"
                    + MultiFileTransferData.KEY_STATUS + " integer,"
                    + MultiFileTransferData.KEY_DIRECTION + " integer,"
                    + MultiFileTransferData.KEY_TIMESTAMP + " long,"
                    + MultiFileTransferData.KEY_TIMESTAMP_SENT + " long,"
                    + MultiFileTransferData.KEY_TIMESTAMP_DELIVERED + " long,"
                    + MultiFileTransferData.KEY_TIMESTAMP_DISPLAYED + " long,"
                    + MultiFileTransferData.KEY_SIZE + " long,"
                    + MultiFileTransferData.KEY_DURATION + " long,"
                    + MultiFileTransferData.KEY_FILEICON + " TEXT,"
                    + MultiFileTransferData.KEY_PARTICIPANTS_LIST + " TEXT,"
                    + MultiFileTransferData.KEY_TOTAL_SIZE + " long);");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
            db.execSQL("DROP TABLE IF EXISTS " + TABLE);
            onCreate(db);
        }
    }

    @Override
    public boolean onCreate() {
        openHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public String getType(Uri uri) {
        int match = uriMatcher.match(uri);
        switch(match) {
            case FILETRANSFERS:
            case RCSAPI:
                return "vnd.android.cursor.dir/multift";
            case FILETRANSFER_ID:
            case RCSAPI_ID:
                return "vnd.android.cursor.item/multift";
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }

    @Override
    public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE);

        // Generate the body of the query
        int match = uriMatcher.match(uri);
        switch(match) {
            case FILETRANSFERS:
            case RCSAPI:
                break;
            case FILETRANSFER_ID:
            case RCSAPI_ID:
                qb.appendWhere(MultiFileTransferData.KEY_ID + "=");
                qb.appendWhere(uri.getPathSegments().get(1));
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        SQLiteDatabase db = openHelper.getReadableDatabase();
        Cursor c = qb.query(db, projectionIn, selection, selectionArgs, null, null, sort);

        // Register the contexts ContentResolver to be notified if the cursor result set changes
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }

        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        int count;
        SQLiteDatabase db = openHelper.getWritableDatabase();

        int match = uriMatcher.match(uri);
        switch (match) {
            case FILETRANSFERS:
            case RCSAPI:
                count = db.update(TABLE, values, where, null);
                break;
            case FILETRANSFER_ID:
            case RCSAPI_ID:
                String segment = uri.getPathSegments().get(1);
                int id = Integer.parseInt(segment);
                count = db.update(TABLE, values, MultiFileTransferData.KEY_ID + "=" + id, null);
                break;
            default:
                throw new UnsupportedOperationException("Cannot update URI " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        switch(uriMatcher.match(uri)){
            case FILETRANSFERS:
            case FILETRANSFER_ID:
                long rowId = db.insert(TABLE, null, initialValues);
                uri = ContentUris.withAppendedId(MultiFileTransferData.CONTENT_URI, rowId);
                break;
            default:
                throw new SQLException("Failed to insert row into " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return uri;
    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        int count = 0;
        switch(uriMatcher.match(uri)){
            case FILETRANSFERS:
            case RCSAPI:
                count = db.delete(TABLE, where, whereArgs);
                break;
            case FILETRANSFER_ID:
            case RCSAPI_ID:
                String segment = uri.getPathSegments().get(1);
                count = db.delete(TABLE, MultiFileTransferData.KEY_ID + "="
                        + segment
                        + (!TextUtils.isEmpty(where) ? " AND ("    + where + ')' : ""),
                        whereArgs);
                
                break;
                
            default:
                throw new SQLException("Failed to delete row " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;    
    }

    static final String DATABASE_LOCATION = "/data/" + "com.orangelabs.rcs" + "/databases/";

    public static void backupMultiFileTransferDatabase(String account) {
        try {
            String dbFile = Environment.getDataDirectory() + DATABASE_LOCATION + FileTransferProvider.DATABASE_NAME;
            File file = new File(dbFile);
            if (file.exists()) {
                File backupFileRoot = new File(Environment.getDataDirectory() + DATABASE_LOCATION);
                backupFileRoot.mkdirs();
                File backupFile = new File(backupFileRoot, FileTransferProvider.DATABASE_NAME + "_" + account + ".db");

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
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void restoreMultiFileTransferDatabase(String account) {
        try {
            String dbFile = Environment.getDataDirectory() + DATABASE_LOCATION + FileTransferProvider.DATABASE_NAME;
            File backupFileRoot = new File(Environment.getDataDirectory() + DATABASE_LOCATION);
            File file = new File(backupFileRoot, FileTransferProvider.DATABASE_NAME + "_" + account + ".db");
            if (!file.exists()) {
                return;
            }

            //delete the original file
            File dbOriginalFile = new File(dbFile);
            if (!dbOriginalFile.exists()) {
                dbOriginalFile.createNewFile();
            }

            new FileOutputStream(dbFile, false).close();
            // dbOriginalFile.createNewFile();
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
