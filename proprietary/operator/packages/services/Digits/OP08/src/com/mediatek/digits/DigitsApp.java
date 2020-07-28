package com.mediatek.digits;

import android.app.Application;
import android.content.Intent;
import android.content.Context;
import android.util.Log;

public class DigitsApp extends Application {

    private static final String TAG = "DigitsApp";

    public static Context sContext;

    public DigitsApp() {

    }

    @Override
    public void onCreate() {
        super.onCreate();

        Log.d(TAG, "onCreate()");
        Intent startIntent = new Intent(this, DigitsService.class);
        startService(startIntent);

        sContext = this;
    }

}



