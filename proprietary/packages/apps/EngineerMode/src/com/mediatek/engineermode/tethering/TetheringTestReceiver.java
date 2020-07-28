package com.mediatek.engineermode.tethering;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.provider.Settings;

import com.mediatek.engineermode.Elog;

/**
 * Broadcast receiver to receive command to enable telephony Elog.
 *
 */
public class TetheringTestReceiver extends BroadcastReceiver {
    private static final String TAG = "TetheringTestReceiver";
    private static final String ACTION_ENABLE_NSIOT =
                        "com.mediatek.intent.action.ACTION_ENABLE_NSIOT_TESTING";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null) {
            Elog.e(TAG, "Intent is null");
            return;
        }
        boolean enabled = false;
        Elog.i(TAG, "TetheringTestReceiver:" + intent.getAction());

        if (ACTION_ENABLE_NSIOT.equals(intent.getAction())) {
            enabled = intent.getBooleanExtra("nsiot_enabled", true);
            Elog.i(TAG, "enabled:" + enabled);
            Intent i = new Intent(context, TetheringTestService.class);
            if (enabled) {
                context.startService(i);
            } else {
                context.stopService(i);
            }
        }
    }
}
