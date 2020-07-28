package com.mediatek.location.lppe.main;

import android.app.Application;
import android.content.Intent;
import android.os.UserHandle;
import android.util.Log;

public class LPPeServiceApplication extends Application {
    private static final String TAG = "LPPeService";
    private LPPeServiceWrapper mService;

    public LPPeServiceApplication() {
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "LPPeServiceApplication() onCreate()");
        //int myUserId = UserHandle.myUserId();
        //Log.d(TAG, "LPPeServiceApplication() onCreate() myUserId=" + myUserId);
        //if (myUserId == 0) {
        //    Intent in = new Intent(this, LPPeServiceWrapper.class);
        //    this.startService(in);
        //}
    }

    @Override
    public void onTerminate() {
        Log.d(TAG, "LPPeServiceApplication() onTerminate()");
    }
}