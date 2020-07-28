package com.mediatek.op.wifi;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.util.Log;

public class WifiDialogContentProvider extends ContentProvider {

	private static final String TAG = "WifiDialogContentProvider";
	private SQLiteDatabase db;

	@Override
	public boolean onCreate() {
		WifiDialogSQLiteHelper wifiDialogsqlHelper = new WifiDialogSQLiteHelper(
				getContext());
		db = wifiDialogsqlHelper.getWritableDatabase();
		return false;
	}

	@Override
	public int delete(Uri arg0, String arg1, String[] arg2) {
		return 0;
	}

	@Override
	public String getType(Uri uri) {
		return null;
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {
		Log.i(TAG, "WifiDialogContentProvider insert");
		long insertNumber = db.insert("cmccwifidialog", null, values);
		Log.i(TAG, "WifiDialogContentProvider insert insertNumber = " + insertNumber);

		return uri;
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection,
			String[] selectionArgs, String sortOrder) {
		Log.i(TAG, "WifiDialogContentProvider query");
		Cursor cursor = db.query("cmccwifidialog", projection, selection, selectionArgs, null, null, sortOrder);
		return cursor;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection,
			String[] selectionArgs) {
		Log.i(TAG, "WifiDialogContentProvider update");
		int updateNumber = db.update("cmccwifidialog", values, selection, selectionArgs);
		return updateNumber;
	}

}
