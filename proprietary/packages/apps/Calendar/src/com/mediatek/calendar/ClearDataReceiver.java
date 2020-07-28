package com.mediatek.calendar;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class ClearDataReceiver extends BroadcastReceiver {

    private static final String PACKAGE_NAME = "packageName";
    private static final String TAG = "ClearDataReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!"com.mediatek.intent.action.SETTINGS_PACKAGE_DATA_CLEARED".equals(intent.getAction()))
            {
                return;
            }

        LogUtil.v(TAG, "action = " + intent.getAction());
        String clearedPackage = intent.getStringExtra(PACKAGE_NAME);
        /// M: clearedPackage may be null
        if (clearedPackage != null && clearedPackage.equals(context.getPackageName())) {
            LogUtil.i(TAG, clearedPackage + ": Calendar App data was cleared. " +
                    "clear the unread messages");
            MTKUtils.writeUnreadReminders(context, 0);
        }
    }

}
