package com.mediatek.lbs.em2.ui;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

public class LocationEMService extends Service{
    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        log("onCreate");
        // TODO Auto-generated method stub
        super.onCreate();
        setServiceForeground();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
     // TODO Auto-generated method stub
     log("onStartCommand");
     return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onStart(Intent intent, int startId) {
        log("onStart");
    }
    public void onDestroy(){
        log("onDestroy");
        super.onDestroy();
    }

    private Intent getLaunchIntent(Context context) {
        Intent intent = new Intent(context, MyTabActivity.class);
        return intent;
    }

    private void setServiceForeground() {
        log("setServiceForeground");
        String channelId = "mtk_app_channel_id";
        CharSequence channelName = "MTK APP";

        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        int importance = NotificationManager.IMPORTANCE_LOW;
        NotificationChannel notificationChannel =
                new NotificationChannel(channelId, channelName, importance);
        notificationChannel.enableLights(false);
        notificationChannel.setLightColor(Color.RED);
        notificationChannel.enableVibration(false);
        notificationManager.createNotificationChannel(notificationChannel);

        PendingIntent p_intent = PendingIntent.getActivity(this, 0,
                getLaunchIntent(this), 0);
        Notification notification = new Notification.Builder(this)
         .setAutoCancel(true)
         .setContentTitle("LocationEM Service")
         .setContentText("Service is Running! Tap to launch Activity.")
         .setContentIntent(p_intent)
         .setSmallIcon(R.drawable.ic_launcher2)
         .setWhen(System.currentTimeMillis())
         .setChannel(channelId)
         .build();

        log(String.format("notification = %s", notification));
        startForeground(0x5678, notification);   // notification ID
    }

    protected static void log(Object msg) {
        Log.d("LocationEM_LocationEMService", "" + msg);
    }
}
