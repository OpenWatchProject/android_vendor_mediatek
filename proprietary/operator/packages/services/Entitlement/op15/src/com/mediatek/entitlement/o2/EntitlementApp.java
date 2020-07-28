package com.mediatek.entitlement.o2;

import android.app.Application;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

public class EntitlementApp extends Application {
    private static final String TAG = "EntitlementApp";

    public EntitlementApp() {
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (!isEntitlementEnabled()) {
            Log.i(TAG, "Entitlement sys property not enabled return directly");
            return;
        }

        Log.d(TAG, "onCreate()");
        Intent startIntent = new Intent(this, EntitlementService.class);
        startService(startIntent);
    }

    private static boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        Log.d(TAG, "isEntitlementEnabled:" + isEntitlementEnabled);
        return isEntitlementEnabled;
    }
}

