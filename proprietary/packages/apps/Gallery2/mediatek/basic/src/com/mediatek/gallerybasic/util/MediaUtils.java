package com.mediatek.gallerybasic.util;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;

import com.mediatek.galleryportable.TraceHelper;

import java.util.Arrays;
import java.util.List;

public class MediaUtils {
    private static final String TAG = "MtkGallery2/MediaUtils";
    private static List<String> sImageColumns;
    private static List<String> sVideoColumns;

    public static List<String> getImageColumns(Context context) {
        if (sImageColumns == null) {
            // In order to avoid SecurityException when query media database,
            // we query internal content uri to get columns.
            TraceHelper.beginSection(">>>>Gallery2-getColumns-image");
            sImageColumns = getColumns(context, MediaStore.Images.Media.INTERNAL_CONTENT_URI);
            TraceHelper.endSection();

        }
        return sImageColumns;
    }

    public static List<String> getVideoColumns(Context context) {
        if (sVideoColumns == null) {
            // In order to avoid SecurityException when query media database,
            // we query internal content uri to get columns.
            TraceHelper.beginSection(">>>>Gallery2-getColumns-video");
            sVideoColumns = getColumns(context, MediaStore.Video.Media.INTERNAL_CONTENT_URI);
            TraceHelper.endSection();
        }
        return sVideoColumns;
    }

    private static List<String> getColumns(Context context, Uri baseUri) {
        Log.d(TAG, "<getColumns> baseUri = " + baseUri);

        Uri uri = baseUri.buildUpon().appendQueryParameter("limit", "0,1").build();
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        List<String> res = null;
        if (cursor != null) {
            String[] names = cursor.getColumnNames();
            if (names != null) {
                res = Arrays.asList(names);
            }
            cursor.close();
        }
        return res;
    }
}