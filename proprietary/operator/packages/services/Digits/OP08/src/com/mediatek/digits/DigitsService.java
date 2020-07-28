package com.mediatek.digits;


import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.digits.DigitsManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class DigitsService extends Service {
    static protected final String TAG = "DigitsService";

    private DigitsServiceImpl mImpl;


    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind DigitsService");
        return mImpl;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate()");

        // Return directly if Digits isn't supported for the operator
        if (!MtkTelephonyManagerEx.getDefault().isDigitsSupported()) {

            Log.d(TAG, "Digits is NOT supported");

            mImpl = null;
        } else {
            Log.d(TAG, "Digits is supported");

            mImpl = new DigitsServiceImpl(this);
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy(), check why ?");
    }
}
