package com.mediatek.dataprotection.utils;

import java.util.ArrayList;

import com.mediatek.dataprotection.FileInfo;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteConstraintException;
import android.database.sqlite.SQLiteFullException;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.provider.MediaStore;
import android.text.TextUtils;

import com.mediatek.dataprotection.utils.Log;

public class DataBaseHelper {

    private static final String TAG = "DataBaseHelper";
    private static final Uri FILE_URI = MediaStore.Files.getContentUri("external");

    public static void closeQuietly(Cursor cursor) {
        if (cursor != null) {
            cursor.close();
        }
    }

    public static boolean updateInMediaStore(Context context, String newPath,
            String oldPath, boolean needUpdate) {

        if (context != null && !TextUtils.isEmpty(newPath)
                && !TextUtils.isEmpty(newPath)) {
            Uri uri = MediaStore.Files.getMtpObjectsUri("external");
            uri = uri.buildUpon()
                    .appendQueryParameter("need_update_media_values", "true")
                    .build();

            String where = MediaStore.Files.FileColumns.DATA + "=?";
            String[] whereArgs = new String[] { oldPath };

            ContentResolver cr = context.getContentResolver();
            ContentValues values = new ContentValues();
            values.put(MediaStore.Files.FileColumns.DATA, newPath);
            whereArgs = new String[] { oldPath };
            if (needUpdate) {
                try {
                    cr.update(uri, values, where, whereArgs);
                } catch (NullPointerException e) {

                } catch (SQLiteFullException e) {

                } catch (UnsupportedOperationException e) {

                }
            }

            /*
             * if (!OptionsUtils.isMtkSDSwapSurpported()) { try {
             * //LogUtils.d(TAG, "updateInMediaStore,update."); if (needUpdate)
             * { cr.update(uri, values, where, whereArgs); }
             *
             * // mediaProvicer.update() only update data columns of //
             * database, it is need to other fields of the database, so scan the
             * // new path after update(). see ALPS00416588
             * scanPathforMediaStore(newPath); } catch (NullPointerException e)
             * { LogUtils.e(TAG, "Error, NullPointerException:" + e +
             * ",update db may failed!!!"); } catch (SQLiteFullException e) {
             * LogUtils.e(TAG, "Error, database or disk is full!!!" + e); if
             * (mBaseAsyncTask != null) { mBaseAsyncTask.cancel(true); } } catch
             * (UnsupportedOperationException e) { LogUtils.e(TAG,
             * "Error, database is closed!!!"); } } else { try { LogUtils.d(TAG,
             * "updateInMediaStore,update."); if (needUpdate) { cr.update(uri,
             * values, where, whereArgs); }
             *
             * // mediaProvicer.update() only update data columns of //
             * database, it is need to other fields of the database, so scan the
             * // new path after update(). see ALPS00416588
             * scanPathforMediaStore(newPath); } catch
             * (UnsupportedOperationException e) { LogUtils.e(TAG,
             * "Error, database is closed!!!"); } catch (NullPointerException e)
             * { LogUtils.e(TAG, "Error, NullPointerException:" + e +
             * ",update db may failed!!!"); } catch (SQLiteFullException e) {
             * LogUtils.e(TAG, "Error, database or disk is full!!!" + e); if
             * (mBaseAsyncTask != null) { mBaseAsyncTask.cancel(true); } } }
             */
        }
        return true;
    }

    public static boolean updateInMediaStore(Context context, String newPath,
            String oldPath, int is_drm, long modified, long size) {

        if (context != null && !TextUtils.isEmpty(newPath)
                && !TextUtils.isEmpty(newPath)) {
            Uri uri = MediaStore.Files.getMtpObjectsUri("external");
            uri = uri.buildUpon()
                    .appendQueryParameter("need_update_media_values", "true")
                    .build();

            String where = MediaStore.Files.FileColumns.DATA + "=?";
            String[] whereArgs = new String[] { oldPath };

            ContentResolver cr = context.getContentResolver();
            ContentValues values = new ContentValues();
            values.put(MediaStore.Files.FileColumns.DATA, newPath);
            // values.put(FileInfo.COLUMN_IS_DRM, is_drm);
            values.put(FileInfo.COLUMN_LAST_MODIFIED, modified);
            values.put(FileInfo.COLUMN_SIZE, size);
            Log.s(TAG, "updateInMediaStore " + oldPath + " newPath");
            whereArgs = new String[] { oldPath };
            try {
                deleteFileInMediaStore(context, oldPath);
                scanFile(context, newPath);
            } catch (NullPointerException e) {

            } catch (SQLiteFullException e) {

            } catch (UnsupportedOperationException e) {

            } catch (SQLiteConstraintException e) {
                e.printStackTrace();
            }
        }
        return true;
    }

    public static void scanFile(Context context, String filePath) {
        if (context != null && !TextUtils.isEmpty(filePath)) {
            String[] paths = { filePath };
            MediaScannerConnection.scanFile(context, paths, null, null);
        }
    }

    public static void deleteFileInMediaStore(Context context, String path) {
        if (TextUtils.isEmpty(path)) {
            return;
        }
        Uri uri = MediaStore.Files.getContentUri("external");
        String where = MediaStore.Files.FileColumns.DATA + "=?";
        String[] whereArgs = new String[] { path };
        if (context != null) {
            ContentResolver cr = context.getContentResolver();
            try {
                cr.delete(uri, where, whereArgs);
            } catch (SQLiteFullException e) {
            }
        }
    }

    public static void deleteFilesInMediaStore(Context context,
            ArrayList<String> paths) {
        Uri uri = MediaStore.Files.getContentUri("external");
        StringBuilder whereClause = new StringBuilder();
        whereClause.append("?");
        for (int i = 0; i < paths.size() - 1; i++) {
            whereClause.append(",?");
        }
        String where = MediaStore.Files.FileColumns.DATA + " IN("
                + whereClause.toString() + ")";
        // notice that there is a blank before "IN(".
        if (context != null && !paths.isEmpty()) {
            ContentResolver cr = context.getContentResolver();
            String[] whereArgs = new String[paths.size()];
            paths.toArray(whereArgs);
            try {
                cr.delete(uri, where, whereArgs);
            } catch (SQLiteFullException e) {

            } catch (UnsupportedOperationException e) {

            }
        }

    }

    public static Uri convertPathToUri(Context context, String path) {
        Cursor cursor = null;
        Uri uri = null;
        try {
            cursor = context.getContentResolver().query(FILE_URI,
                    new String[] { MediaStore.Files.FileColumns._ID },
                    MediaStore.Files.FileColumns.DATA + "=?",
                    new String[] { path },
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                uri =  ContentUris.withAppendedId(FILE_URI, cursor.getInt(0));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        return uri;
    }
}
