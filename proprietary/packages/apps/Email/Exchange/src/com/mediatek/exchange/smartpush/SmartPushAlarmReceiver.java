package com.mediatek.exchange.smartpush;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class SmartPushAlarmReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        SmartPushService.alarmSmartPushService(context);
    }

}
