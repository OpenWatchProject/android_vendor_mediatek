package com.mediatek.sensorhub.service;

import com.mediatek.sensorhub.settings.Utils;

import android.app.Application;
import android.content.Intent;
import android.util.Log;

public class SensorhubApplication extends Application {

    private static final String TAG = "SensorhubApplication";
    Intent mIntent;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "start sensorhub application.");
        Utils utilsPlus = Utils.getInstance(this);
        mIntent = new Intent();
        mIntent.setClass(this, SensorEventListenerService.class);
        startForegroundService(mIntent);
    }

    @Override
    public void onLowMemory() {
        Log.d(TAG, "low memory sensorhub application.");
        stopService(mIntent);
        super.onLowMemory();
    }

    @Override
    public void onTerminate() {
        stopService(mIntent);
        Log.d(TAG, "terminate sensorhub application.");
        super.onTerminate();
    }
}
