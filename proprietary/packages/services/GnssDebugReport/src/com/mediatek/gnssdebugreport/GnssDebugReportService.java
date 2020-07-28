package com.mediatek.gnssdebugreport;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;



public class GnssDebugReportService extends Service {

    private static final String TAG = "GnssDebugReportService";
    private GnssDebugReportServiceStub mService = null;
    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        mService = new GnssDebugReportServiceStub();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mService;
    }

    @Override
    public void onDestroy() {
        // must release native memory
        Log.i(TAG, "onDestroy");
/*        if (mService != null) {
            mService.stopDebug();
        }*/
    }

}
