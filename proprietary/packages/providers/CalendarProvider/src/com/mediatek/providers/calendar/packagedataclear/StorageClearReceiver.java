package com.mediatek.providers.calendar.packagedataclear;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.provider.CalendarContract;

import com.mediatek.providers.calendar.LogUtil;

public class StorageClearReceiver extends BroadcastReceiver {
    private static final String TAG = "StorageClearReceiver";
    private static final String PACKAGE_NAME = "packageName";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!"com.mediatek.intent.action.SETTINGS_PACKAGE_DATA_CLEARED".equals(intent.getAction()))
            {
                return;
            }
        String packageName = intent.getStringExtra(PACKAGE_NAME);
        if (packageName != null && packageName.equals(context.getPackageName())) {
            broadStorageCleared(context);
        }
    }

    /**
     * set a broadcast to notify that the CalendarProvider package data was
     * cleared
     *
     * @param context
     */
    private void broadStorageCleared(Context context) {
        LogUtil.d(TAG, "CalendarProvider package data was cleared...");
        Intent intent = new Intent(Intent.ACTION_PROVIDER_CHANGED, CalendarContract.CONTENT_URI);
        context.sendBroadcast(intent, null);
    }
}
