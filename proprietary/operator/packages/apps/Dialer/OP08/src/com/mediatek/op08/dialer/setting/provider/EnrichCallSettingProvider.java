package com.mediatek.op08.dialer.setting.provider;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.util.SparseBooleanArray;
import android.util.SparseArray;
import android.util.Log;

import com.mediatek.op08.dialer.setting.SettingConstant;
import com.mediatek.op08.dialer.setting.SharedPreferencesHelper;

public class EnrichCallSettingProvider extends ContentProvider{

    private static final String TAG = "EnrichCallSettingProvider";

    public static final String _ID = "id";
    public static final String TYPE = "type";
    public static final String STATUS = "status";
    public static final String CONTENT = "content";

    public static final String[] COLUMNS_NAMES = {
        _ID,
        TYPE,
        STATUS,
        CONTENT
    };

    @Override
    public boolean onCreate() {
        Log.i(TAG, "onCreate");
        return true;
    }

    @Override
    public String getType(Uri uri) {
         return SettingConstant.AUTHORITY;
    }

    //{int _id, int type,boolean status, String content}
    /*
     * 1, master_switch,
     * 2. subject
     * 3. image
     * 4. location
     */
    /*
     * this provider is not a real related database, so you can set all params null
     */
    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
                            String[] selectionArgs, String sortOrder) {
        Log.i(TAG, "query");
        SparseArray<String> content = new SparseArray<String>();
        SparseBooleanArray status = new SparseBooleanArray();

        SharedPreferencesHelper.readStatusFromPref(this.getContext(), status);
        SharedPreferencesHelper.readContentsFromPref(this.getContext(), content);

        if (!status.valueAt(SettingConstant.TYPE_MASTER_SWITCH)) {
            Log.i(TAG, "query MASTER_SWITCH is off, return null");
            return null;
        }

        MatrixCursor cursor = new MatrixCursor(COLUMNS_NAMES);
        if (status.valueAt(SettingConstant.TYPE_IMPORTANT_INDICATOR)) {
            cursor.addRow(new Object[] { 1, SettingConstant.TYPE_IMPORTANT_INDICATOR,
                    (status.valueAt(SettingConstant.TYPE_IMPORTANT_INDICATOR)? 1:0),
                    null});
        }

        if (status.valueAt(SettingConstant.TYPE_SUBJECT)) {
            cursor.addRow(new Object[] { 2, SettingConstant.TYPE_SUBJECT,
                    (status.valueAt(SettingConstant.TYPE_SUBJECT)? 1:0),
                    content.valueAt(SettingConstant.TYPE_SUBJECT)});
        }

        if (status.valueAt(SettingConstant.TYPE_IMAGE)) {
            cursor.addRow(new Object[] { 3, SettingConstant.TYPE_IMAGE,
                    (status.valueAt(SettingConstant.TYPE_IMAGE)? 1:0),
                    content.valueAt(SettingConstant.TYPE_IMAGE)});
        }

        if (status.valueAt(SettingConstant.TYPE_LOCATION)) {
            cursor.addRow(new Object[] { 4, SettingConstant.TYPE_LOCATION,
                    (status.valueAt(SettingConstant.TYPE_LOCATION)? 1:0),
                    content.valueAt(SettingConstant.TYPE_LOCATION)});
        }

        Log.i(TAG, "query count:" + cursor.getCount());
        cursor.moveToFirst();
        Log.i(TAG, "query type:" + cursor.getInt(1));
        Log.i(TAG, "query status:" + cursor.getInt(2));
        Log.i(TAG, "query content:" + cursor.getString(3));
        return cursor;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        Log.i(TAG, "insert");
        return null;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        Log.i(TAG, "update");
        return 0;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }
}
