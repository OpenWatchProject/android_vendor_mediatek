package com.mediatek.tatf.common;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class TatfBaseService extends Service {

    private static final String TAG = TatfBaseService.class.getSimpleName();
    private Context mContext;

    @Override
    public IBinder onBind(Intent intent) {
        String action = intent.getAction();
        if (action == null) {
            action = intent.getStringExtra("action");
        }
        Log.d(TAG, " onBind action: " + action);
        // TODO Verify source bind call permission.
        // set different Binder for different action.
        return TatfPluginLoader.getInstance(mContext).getBinder(action);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = this;
        Log.d(TAG, " onCreate");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, " onDestroy");
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, " onUnbind");
        return super.onUnbind(intent);
    }
}
