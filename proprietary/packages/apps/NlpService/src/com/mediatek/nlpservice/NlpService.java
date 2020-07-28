package com.mediatek.nlpservice;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class NlpService extends Service {

    public final static String TAG = "NlpService";
    private LbsNlpUtils mLbsNlpUtils;

    public NlpService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        log("NlpService.onCreate()");
        if (mLbsNlpUtils == null) {
            try {
                mLbsNlpUtils = new LbsNlpUtils(this);
            } catch (RuntimeException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onStart(Intent intent, int startId) {
        log("NlpService.onStart()");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        log("NlpService.onDestroy()");
        if (mLbsNlpUtils != null) {
            mLbsNlpUtils.releaseListeners();
        }
    }

    public void log(Object msg) {
        Log.d(TAG, "" + msg);
    }

}
