package com.mediatek.digits.utils;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;


public class CallbackScheduler {
    private static final String TAG = "CallbackScheduler";
    private Context mContext;

    private volatile boolean mTimerStarted = false;
    private String mAction = null;
    private Callback mCallback = null;

    private PendingIntent mTimerIntent = null;
    private BroadcastReceiver mTimerReceiver = null;

    public CallbackScheduler(Context context, String action, Callback callback) {
        mCallback = callback;
        init(context, action);
    }

    private void init(Context context, String action) {
        mContext = context;
        mAction = action;

        mTimerIntent = PendingIntent.getBroadcast(
                           mContext,
                           0,
                           new Intent(mAction),
                           0);
        mTimerReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                final String action = intent.getAction();
                Log.d(TAG, "Timer onReceive: " + action);

                new Thread() {
                    @Override
                    public void run() {

                        // Call Thread.stop() will cause UnsupportedOperationException();
                        CallbackScheduler.this.stop();
                        mCallback.run(action);
                    }
                } .start();
            }
        };
    }

    public synchronized void start(long timer) {
        Log.d(TAG, "Timer Start: " + mAction + "/" + mTimerStarted + "/" + timer);
        if (mTimerStarted)
            return;
        if (timer <= 0)
            timer = 0;
        mContext.registerReceiver(mTimerReceiver, new IntentFilter(mAction));
        AlarmManager am = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);
        am.setExact(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + timer, mTimerIntent);
        mTimerStarted = true;
    }

    public synchronized void stop() {
        Log.d(TAG, "Timer Stop: " + mAction + "/" + mTimerStarted);

        if (!mTimerStarted)
            return;
        AlarmManager am = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);
        am.cancel(mTimerIntent);
        mContext.unregisterReceiver(mTimerReceiver);
        mTimerStarted = false;
    }

    public synchronized boolean isStarted() {
        return mTimerStarted == true;
    }

    public interface Callback {
        void run(String action);
    }
}
