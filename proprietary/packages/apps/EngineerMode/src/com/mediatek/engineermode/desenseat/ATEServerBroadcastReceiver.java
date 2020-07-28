package com.mediatek.engineermode.desenseat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

public class ATEServerBroadcastReceiver extends BroadcastReceiver {
    public static final String TAG = "ATEServer";
    private static final String START_ACTION = "com.mediatek.NotifyServiceStart";
    private static final String STOP_ACTION = "com.mediatek.NotifyServiceStop";

    @Override
    public void onReceive(Context context, Intent intent) {
        Elog.d(TAG, "ATEServerBroadcastReceiver -> onReceive");
        String action = intent.getAction();

        if (START_ACTION.equalsIgnoreCase(action)) {
            Elog.d(TAG, "ATEServerBroadcastReceiver -> start ");
            if (!EmUtils.checkLocationProxyAppPermission(context, false)) {
                Intent it = new Intent(context, LbsPermissionActivity.class);
                it.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(it);
            } else {
                context.startService(new Intent(context, ATEServer.class));
            }
        } else if (STOP_ACTION.equalsIgnoreCase(action)) {
            Elog.d(TAG, "ATEServerBroadcastReceiver -> stop");
            context.stopService(new Intent(context, ATEServer.class));
        }
    }

}
