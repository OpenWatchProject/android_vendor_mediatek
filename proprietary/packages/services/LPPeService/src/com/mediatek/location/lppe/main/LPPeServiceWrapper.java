package com.mediatek.location.lppe.main;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import com.mediatek.location.agps.apn.AgpsApn;

public class LPPeServiceWrapper extends Service {

    public final static String TAG = "LPPeService";
    LPPeService mLppe = null;
    private AgpsApn mAgpsApn;

    public LPPeServiceWrapper() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        log("LPPeServiceWrapper.onCreate(), enable AgpsApn.");
        if (mLppe == null) {
            try {
                mLppe = new LPPeService(this);
                mAgpsApn = new AgpsApn(this);
            } catch (RuntimeException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onStart(Intent intent, int startId) {
        log("LPPeServiceWrapper.onStart()");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        log("LPPeServiceWrapper.onDestroy()");
        if (mLppe != null) {
            mLppe.cleanup();
        }
    }

    public void log(Object msg) {
        Log.d(TAG, "" + msg);
    }

}
