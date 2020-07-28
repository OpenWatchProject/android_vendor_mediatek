package com.mediatek.op12.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;


public class RoamingReceiver extends BroadcastReceiver {
    private static final String TAG = "VideoSettingReceiver";

    @Override
    public void onReceive(final Context context, final Intent intent) {
        Log.d(TAG, "PLMN changed");
        intent.setClass(context, RoamingReceiverService.class);
        context.startService(intent);
    }
}