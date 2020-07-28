package com.mediatek.apn;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;


public class OmacpApnReceiver extends BroadcastReceiver {
    private static final String TAG = "OmacpApnReceiver";
    // action
    private static final String ACTION_OMACP = "com.mediatek.omacp.settings";

    @Override
    public void onReceive(Context context, Intent intent) {

        String action = intent.getAction();
        Log.d(TAG, "get action = " + action);

        if (context.getContentResolver() == null) {
            Log.e(TAG, "FAILURE unable to get content resolver..");
            return;
        }

        if (ACTION_OMACP.equals(action)) {
            startOmacpService(context, intent);
        }
    }

    private void startOmacpService(Context context, Intent broadcastIntent) {
        // Launch the Service
        Intent i = new Intent(context, OmacpApnReceiverService.class);
        i.setAction(ApnUtils.ACTION_START_OMACP_SERVICE);
        i.putExtra(Intent.EXTRA_INTENT, broadcastIntent);
        Log.d(TAG, "startService");
        context.startService(i);
    }

}
