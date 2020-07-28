package com.mediatek.entitlement;

import android.app.Application;
import android.content.Intent;
import android.content.Context;
import android.util.Log;

public class EntitlementApp extends Application {

    private static final String TAG = "EntitlementApp";

    public static Context sContext;

    public EntitlementApp() {

    }

    @Override
    public void onCreate() {
        super.onCreate();

        Log.d(TAG, "onCreate()");
        Intent startIntent = new Intent(this, EntitlementService.class);
        startService(startIntent);

        sContext = this;
    }

}



