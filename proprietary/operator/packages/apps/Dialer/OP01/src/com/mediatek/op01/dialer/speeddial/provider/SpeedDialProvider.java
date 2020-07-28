package com.mediatek.op01.dialer.speeddial.provider;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.util.Log;

import com.mediatek.op01.dialer.speeddial.provider.SpeedDial;
import com.mediatek.op01.dialer.speeddial.provider.SpeedDialDatabaseHelper.Tables;

public class SpeedDialProvider extends ContentProvider{

    private static final String TAG = "SpeedDialProvider";
    private static final int SPEEDDIAL = 1;
    private SpeedDialDatabaseHelper mDbHelper;

    private static final UriMatcher URIMATCHER = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        URIMATCHER.addURI(SpeedDial.AUTHORITY, "numbers", SPEEDDIAL);
    }

    protected SpeedDialDatabaseHelper getDatabaseHelper(Context context) {
        SpeedDialDatabaseHelper dbHelper = SpeedDialDatabaseHelper.getInstance(context);
        return dbHelper;
    }

    @Override
    public boolean onCreate() {
        Log.i(TAG, "onCreate");
        mDbHelper = getDatabaseHelper(getContext());
        //initSpeedDialTable(mDbHelper.getWritableDatabase());
        return true;
    }

    private void initSpeedDialTable(SQLiteDatabase db) {
        Log.i(TAG, "initSpeedDialTable");
        ContentValues[] valueArray = new ContentValues[10];
        for (int i = 0; i < 10; i++) {
            valueArray[i] = new ContentValues();
            valueArray[i].put(SpeedDial.Numbers.NUMBER, "");
        }

        int numValue = 0;
        db.beginTransaction();
        try {
            numValue = valueArray.length;
            for (int i = 0; i < numValue; i++) {
                insert(SpeedDial.Numbers.CONTENT_URI,valueArray[i]);
            }
            db.setTransactionSuccessful();
        } finally {
            db.endTransaction();
        }
    }

    @Override
    public String getType(Uri uri) {
        int match = URIMATCHER.match(uri);
        switch (match) {
            case SPEEDDIAL:
                return SpeedDial.Numbers.CONTENT_TYPE;
            default:
                throw new IllegalArgumentException("Unknown URL " + uri);
        }
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
                            String[] selectionArgs, String sortOrder) {
        Log.i(TAG, "query");
        int match = URIMATCHER.match(uri);
        switch (match) {
            case SPEEDDIAL:
                break;
            default:
                break;
        }
        final SQLiteDatabase db = mDbHelper.getReadableDatabase();
        Cursor cursor = db.query(Tables.SPEEDDIAL, projection, selection, selectionArgs, null, null, sortOrder, null);
        return cursor;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        Log.i(TAG, "insert");
        final SQLiteDatabase db = mDbHelper.getWritableDatabase();
        long id = db.insert(Tables.SPEEDDIAL, null, values);
        return ContentUris.withAppendedId(uri, id);
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        final SQLiteDatabase db = mDbHelper.getWritableDatabase();
        Log.i(TAG, "update");
        int match = URIMATCHER.match(uri);
        switch(match) {
            case SPEEDDIAL:
                break;
            default:
                break;
        }
        int result = db.update(Tables.SPEEDDIAL, values, selection, selectionArgs);
        return result;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        final SQLiteDatabase db = mDbHelper.getWritableDatabase();

        int match = URIMATCHER.match(uri);
        switch(match) {
            case SPEEDDIAL:
                break;
            default:
                break;
        }
        int result = db.delete(Tables.SPEEDDIAL, selection, selectionArgs);
        return result;
    }
}
