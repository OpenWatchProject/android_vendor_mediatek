package com.mediatek.mms.plugin;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;

import com.mediatek.mms.ext.DefaultOpWappushMessagingNotificationExt;

/**
 * Op01WappushMessagingNotificationExt.
 *
 */
public class Op01WappushMessagingNotificationExt extends
        DefaultOpWappushMessagingNotificationExt {

    /**
     * Construction.
     * @param context Context
     */
    public Op01WappushMessagingNotificationExt(Context context) {
        super(context);
    }

    @Override
    public boolean updateNotification(Context context, Notification.Builder noti) {
            /// M: for fix ALPS01668438,Rollback CR ALPS01638362 for cmcc case 5.7.
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.setType("vnd.android-dir/wappush");
        PendingIntent pIntent = PendingIntent.getActivityAsUser(
                context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT,
                null, UserHandle.CURRENT_OR_SELF);

        if (pIntent != null) {
            noti.setContentIntent(pIntent);
        }
        return true;
    }
}
