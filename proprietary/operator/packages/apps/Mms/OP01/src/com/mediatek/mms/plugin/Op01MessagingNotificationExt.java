package com.mediatek.mms.plugin;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import com.mediatek.mms.ext.DefaultOpMessagingNotificationExt;

/**
 * Op01MessagingNotificationExt.
 *
 */
public class Op01MessagingNotificationExt extends
        DefaultOpMessagingNotificationExt {
    private static final String TAG = "Op01MessagingNotificationExt";

    /**
     * Construction.
     * @param context Context
     */
    public Op01MessagingNotificationExt(Context context) {
        super(context);
    }

    @Override
    public Uri getUndeliveredMessageCount(Uri uri) {
        uri = Uri.parse("content://mms-sms/undelivered");
        return uri.buildUpon().appendQueryParameter("includeNonPermanent", "false").build();
    }

    @Override
    public boolean updateNotification(Context context, int uniqueThreadCount,
            long mostRecentThreadId, Class<?> cls, int messageCount, boolean isMostRecentSms,
            Uri mostRecentUri, Notification.Builder noti) {
        PendingIntent pIntent = null;
        if (uniqueThreadCount > 1 || mostRecentThreadId <= 0) {
            Intent clickIntent = new Intent(context, cls);
            clickIntent.putExtra("thread_count", uniqueThreadCount);
            clickIntent.putExtra("message_count", messageCount);
            pIntent = PendingIntent.getBroadcast(context,
                    0, clickIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        } else {
            Intent clickIntent = new Intent(context, cls);
            clickIntent.putExtra("thread_count", 1);
            clickIntent.putExtra("message_count", messageCount);
            clickIntent.putExtra("thread_id", mostRecentThreadId);
            clickIntent.putExtra("isSms", isMostRecentSms);
            if (isMostRecentSms && messageCount == 1) {
                clickIntent.setData(mostRecentUri);
            }
            pIntent = PendingIntent.getBroadcast(context,
                    0, clickIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        }

        if (pIntent != null) {
            noti.setContentIntent(pIntent);
        }
        return true;
    }

    @Override
    public boolean notifyFailed(Context context, Class<?> cls,
            boolean allFailedInSameThread, boolean isDownload,
            long threadId, Notification.Builder notification, String title, String description) {
        PendingIntent pIntent = null;
        Intent clickIntent = new Intent(context, cls);
        clickIntent.putExtra("isFailed", true);
        if (allFailedInSameThread) {
            clickIntent.putExtra("thread_count", 1);
        } else {
            clickIntent.putExtra("thread_count", 2);
        }
        if (isDownload) {
            clickIntent.putExtra("failed_download_flag", true);
        } else {
            clickIntent.putExtra("undelivered_flag", true);
        }
        clickIntent.putExtra("thread_id", threadId);
        Log.d(TAG, "notifySendFailed, threadId 2 = " + threadId);
        pIntent = PendingIntent.getBroadcast(context, 0,
                clickIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        notification.setContentTitle(title);
        notification.setContentText(description);
        notification.setContentIntent(pIntent);
        return true;
    }
}
