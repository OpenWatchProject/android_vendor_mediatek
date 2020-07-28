package com.mediatek.engineermode.rfdesense;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.engineermode.Elog;


public class RfDesenseBroadcastReceiver extends BroadcastReceiver {
    public final String TAG = "RfDesense/BroadcastReceiver";
    public static String ARGUMENTS = "argument";
    private String START_ACTION = "com.mediatek.engineermode.rfdesenseServiceStart";
    private String STOP_ACTION = "com.mediatek.engineermode.rfdesenseServiceStop";

    @Override
    public void onReceive(Context context, Intent intent) {
        Elog.d(TAG, "onReceive");
        String action = intent.getAction();

        if (START_ACTION.equalsIgnoreCase(action)) {
            Intent intent_service = new Intent(context, RfDesenseService.class);
            intent_service.putExtra(ARGUMENTS, intent.getStringExtra(ARGUMENTS));
            Elog.d(TAG, "RfDesenseService start ");
            context.startService(intent_service);
        } else if (STOP_ACTION.equalsIgnoreCase(action)) {
            Elog.d(TAG, "RfDesenseService stop");
            context.stopService(new Intent(context, RfDesenseService.class));
        }
    }

}
