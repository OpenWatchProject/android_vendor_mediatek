package com.mediatek.apmonitor.mdmi;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.util.Log;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class MdmiProvider extends ContentProvider {
    private static final String TAG = "MDMI-Provider";
    static final String AUTHORITY = "com.gsma.mdmi.db";
    static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/");
    static final Uri MAP_TABLE_URI = Uri.parse("content://" + AUTHORITY +
            "/" + MdmiDatabaseHelper.TABLE_NAME + "/");
    private SQLiteOpenHelper mOpenHelper;

    @Override
    public boolean onCreate() {
        Log.d(TAG, "onCreate()");
        mOpenHelper = new MdmiDatabaseHelper(getContext());
        init();
        return true;
    }

    private void init() {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        if (!isDatabaseInitialized()) {

            MdmiCategoryMapLoader loader = new MdmiCategoryMapLoader(true);
            Map<String, String> map = loader.getMdmiCategoryMap();

            Iterator iter = map.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry entry = (Map.Entry)iter.next();
                String key = (String) entry.getKey();
                String value = map.get(key);
                insertMdmiData(db, key, value);
            }
            Log.d(TAG, "init done");
        } else {
            Log.d(TAG, "skip init");
        }
    }

    private boolean isDatabaseInitialized() {
        Cursor c = query(MAP_TABLE_URI, null, null, null, null);
        if (c != null && c.getCount() > 0) {
            Log.d(TAG, "count: " + c.getCount());
            return true;
        } else {
            Log.d(TAG, "return false");
            return false;
        }
    }

    private long insertMdmiData(SQLiteDatabase db, String category, String library) {
        ContentValues cv = new ContentValues();
        cv.put(MdmiDatabaseHelper.Column.CATEGORY, category);
        cv.put(MdmiDatabaseHelper.Column.LIBRARY, library);
        long rowId = db.insert(MdmiDatabaseHelper.TABLE_NAME, null, cv);

        if (rowId < 0) {
            Log.e(TAG, "insert fail, rowId: " + rowId);
        } else {
            Log.d(TAG, "insert success, rowId: " + rowId);
        }
        return rowId;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(MdmiDatabaseHelper.TABLE_NAME);

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, null);
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), CONTENT_URI);
        }
        return c;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        throw new UnsupportedOperationException("insert not supported");
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("delete not supported");
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("update not supported");
    }

    @Override
    public String getType(Uri uri) {
        return null;
    }
}
