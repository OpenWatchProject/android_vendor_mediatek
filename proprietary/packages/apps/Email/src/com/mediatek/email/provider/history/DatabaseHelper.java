package com.mediatek.email.provider.history;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.android.emailcommon.Logging;
import com.mediatek.email.provider.history.EmailAddressContent.AddressColumns;

/**
 * M: Add DatabaseHelper for email address history
 */
public class DatabaseHelper extends SQLiteOpenHelper {
    private static final String TAG = "EmailAddressHisDBHelper";
    public static final int DATABASE_VERSION = 1;

    DatabaseHelper(Context context, String name) {
        super(context, name, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        Logging.d(TAG, "Creating EmailAddressHistory provider database");
        createAddressTable(db);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
    }

    private void createAddressTable(SQLiteDatabase db) {
        String s = " (" + AddressColumns.ID + " integer primary key autoincrement, "
                + AddressColumns.EMAIL_ADDRESS + " text);";
        db.execSQL("create table " + EmailAddress.TABLE_NAME + s);
    }
}
