package com.mediatek.providers.drm;

import static com.mediatek.providers.drm.OmaDrmHelper.DEBUG;

import java.util.Arrays;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.Log;

import com.mediatek.media.MtkMediaStore;

public class DrmRescanReceiver extends BroadcastReceiver{

    private static final String TAG = "DRM/DrmRescanReceiver";
    private static final Uri FILE_URI = MediaStore.Files.getContentUri("external");

    @Override
    public void onReceive(Context context, Intent intent) {
        Bundle extras = intent.getExtras();
        String cid = extras != null ? extras.getString("drm_cid") : null;
        if (cid == null) {
            Log.w(TAG, "given cid is null, skip rescan relate drm file");
            return;
        }
        Cursor cursor = null;
        String[] paths = null;
        try {
            cursor = context.getContentResolver().query(FILE_URI,
                    new String[] { MediaStore.Files.FileColumns.DATA },
                    MtkMediaStore.MediaColumns.DRM_CONTENT_URI + "=?",
                    new String[] { cid },
                    null);

            if (cursor != null && cursor.moveToFirst()) {
                int count = cursor.getCount();
                paths = new String[count];
                for (int i = 0; i < paths.length; i++) {
                    paths[i] = cursor.getString(0);
                    cursor.moveToNext();
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        if (paths != null) {
            MediaScannerConnection.scanFile(context.getApplicationContext(), paths, null, null);
        }
        if (DEBUG) Log.d(TAG, "rescan drm file: cid = " + cid + ", relate files = "
                + Arrays.toString(paths));
    }
}
