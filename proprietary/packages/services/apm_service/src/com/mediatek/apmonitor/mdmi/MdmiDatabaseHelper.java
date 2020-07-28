package com.mediatek.apmonitor.mdmi;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;

import android.util.Log;

public class MdmiDatabaseHelper extends SQLiteOpenHelper {
    private static final String TAG = "MDMI-Provider";
    static final String DATABASE_NAME = "com.gsma.mdmi.db";
    static final String TABLE_NAME = "mibdiscovery";
    static final int DATABASE_VERSION = 1;

    public static class Column {
        public static final String _ID = "_id";

        public static final String CATEGORY = "oid";
        public static final String LIBRARY = "libraryname";

        //public static final String CATEGORY = "category_name";
        //public static final String LIBRARY = "library_name";
    }

    MdmiDatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + TABLE_NAME + " ("
                + Column._ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                + Column.CATEGORY + " TEXT,"
                + Column.LIBRARY + " TEXT);");
        Log.d(TAG, "onCreate");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
    }
}
