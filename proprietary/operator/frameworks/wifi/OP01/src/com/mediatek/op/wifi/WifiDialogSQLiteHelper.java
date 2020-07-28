package com.mediatek.op.wifi;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class WifiDialogSQLiteHelper extends SQLiteOpenHelper {

	public WifiDialogSQLiteHelper(Context context) {
		super(context, "wifidialog.db", null, 1);
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
        db.execSQL("create table cmccwifidialog(_id integer primary key autoincrement," +
        		"remind_connect_key char(50)," +
        		"remind_connect_value integer(10)," +
        		"remind_hotspot_key char(50)," +
        		"remind_hotspot_value integer(10))");
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
	}
}
