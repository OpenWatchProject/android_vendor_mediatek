package com.mediatek.op12.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;



/** Broadcast receiver to receive Boot complete intent, ImsConfig intents
 * & invoke service to set WFC Settings in settings Provider during provisioning & de-provisioning.
 */
public class AdvancedCallingReceiver extends BroadcastReceiver {

    private static final String TAG = "Op12AdvancedCallingReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, " onReceive : " + action);
        if ("com.android.intent.action.IMS_CONFIG_CHANGED".equals(action)) {
            intent.setClass(context, AdvancedCallingReceiverService.class);
            context.startService(intent);
        }
    }
}