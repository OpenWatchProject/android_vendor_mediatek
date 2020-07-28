package com.mediatek.location.lppe.main;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class LPPeBootUpReceiver extends BroadcastReceiver {

    public final static String TAG = "LPPeService";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            log("LPPeBootUpReceiver boot completed received");
            Intent in = new Intent(context, LPPeServiceWrapper.class);
            context.startService(in);
        }
    }

    public void log(Object msg) {
        Log.d(TAG, "" + msg);
    }
}
