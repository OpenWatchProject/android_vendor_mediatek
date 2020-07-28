package com.mediatek.gallery3d.adapter;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.mediatek.gallerybasic.util.MediaUtils;

/**
 * Service which used to initialize FeatureManager.
 * TODO: Maybe this is temp solution for initialize FeatureManager
 */
public class FeatureService extends Service {
    private static final String TAG = "MtkGallery2/FeatureService";

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "<onStartCommand>");
        return Service.START_STICKY;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "<onCreate>");
        super.onCreate();
        FeatureManager.setup(getApplicationContext());
        MediaUtils.getImageColumns(getApplicationContext());
        MediaUtils.getVideoColumns(getApplicationContext());
    }
}
