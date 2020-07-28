package com.mediatek.dataprotection;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.content.Context;

import com.mediatek.dataprotection.utils.Log;

import java.util.Iterator;
import java.util.Queue;

public class DataProtectNotifier {

    /*
     * private static final int TYPE_ACTIVE = 1; private static final int
     * TYPE_WAITING = 2; private static final int TYPE_COMPLETE = 3;
     */
    public static final int PROGRESS_ID = 0xffffffff;

    private static final int TYPE_LOCK_COMPLETED = 1;
    private static final int TYPE_LOCK_CANCELLED = 2;
    private static final int TYPE_LOCKING = 3;
    private static final int TYPE_UNLOCK_COMPLETED = 4;
    private static final int TYPE_UNLOCK_CANCELLED = 5;
    private static final int TYPE_UNLOCKING = 6;

    public static final int ACTION_CLEAR_LOCK_COMPLETED_NOTIFICATION = 100;
    public static final int ACTION_CLEAR_UNLOCK_COMPLETED_NOTIFICATION = 101;
    private static final String TAG = "DataProtectionNotifier";

    private final Context mContext;
    private final NotificationManager mNotifManager;
    private final NotificationChannel mChannel;
    private int mNotifiId = 0;
    private final int mDecryptFailId = -1;

    private static final String NOTIFICATION_CHANNEL = "DataProtection";

    public DataProtectNotifier(Context context) {
        mContext = context;
        mNotifManager = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);
        String name = context.getString(R.string.channel_name);
        String description = context.getString(R.string.channel_description);
        mChannel = new NotificationChannel(NOTIFICATION_CHANNEL, name,
                NotificationManager.IMPORTANCE_LOW);
        mChannel.setDescription(description);
        mNotifManager.createNotificationChannel(mChannel);
    }

    public Notification updateTaskProgressReturn(DataProtectionTask task) {
        if (!task.needShowNotification()) {
            Log.d(TAG, "not show notification.");
            return null;
        }
        final Notification.Builder builder = new Notification.Builder(
                mContext).setChannel(NOTIFICATION_CHANNEL);
        builder.setWhen(task.getCreateTime());
        if (task.getIcon() != 0) {
            builder.setSmallIcon(task.getIcon());
        }

        builder.setProgress(100, task.getProgress(), false);
        String title = task.getNotificationTitle();
        if (title != null) {
            builder.setContentTitle(title);
        }
        PendingIntent cIntent = task.getContentIntent();
        if (cIntent != null) {
            builder.setContentIntent(cIntent);
        }
        String contentText = task.getNotificationContentText();
        if (contentText != null) {
            builder.setContentText(contentText);
        }

        String contentInfo = task.getNotificationContentInfo();
        if (contentInfo != null) {
            builder.setContentInfo(contentInfo);
        }

        builder.setOngoing(true);

        Notification notif = builder.build();
        // notif.setLatestEventInfo(context, contentTitle, contentText,
        // contentIntent)
        int id = (int) task.getCreateTime();
        Log.d(TAG, "notification id: " + id);
        //mNotifManager.notify((int) task.getCreateTime(), notif);
        mNotifManager.cancel((int) task.getCreateTime());
        return notif;
    }

    public Notification updateTaskQueue(DataProtectionTask task) {
        if (!task.needShowNotification()) {
            Log.d(TAG, "not show notification.");
            return null;
        }
        final Notification.Builder builder = new Notification.Builder(
                mContext).setChannel(NOTIFICATION_CHANNEL);
        builder.setWhen(task.getCreateTime());
        if (task.getIcon() != 0) {
            builder.setSmallIcon(task.getIcon());
        }

        builder.setProgress(-1, task.getProgress(), true);
        String title = task.getNotificationTitle();
        if (title != null) {
            builder.setContentTitle(title);
        }
        PendingIntent cIntent = task.getContentIntent();
        if (cIntent != null) {
            builder.setContentIntent(cIntent);
        }
        String contentText = task.getNotificationContentText();
        if (contentText != null) {
            builder.setContentText(contentText);
        }

        String contentInfo = task.getNotificationContentInfo();
        if (contentInfo != null) {
            builder.setContentInfo(contentInfo);
        }

        builder.setOngoing(true);

        Notification notif = builder.build();
        // notif.setLatestEventInfo(context, contentTitle, contentText,
        // contentIntent)
        int id = (int) task.getCreateTime();
        Log.d(TAG, "notification id: " + id);
        mNotifManager.notify((int) task.getCreateTime(), notif);
        return notif;
    }
    //public Notification updateTask

    public void updateTaskProgress(DataProtectionTask task) {
        if (!task.needShowNotification()) {
            Log.d(TAG, "not show notification.");
            return;
        }
        final Notification.Builder builder = new Notification.Builder(
                mContext).setChannel(NOTIFICATION_CHANNEL);
        builder.setWhen(task.getCreateTime());
        if (task.getIcon() != 0) {
            builder.setSmallIcon(task.getIcon());
        }

        builder.setProgress(100, task.getProgress(), false);
        String title = task.getNotificationTitle();
        if (title != null) {
            builder.setContentTitle(title);
        }
        PendingIntent cIntent = task.getContentIntent();
        if (cIntent != null) {
            builder.setContentIntent(cIntent);
        }
        String contentText = task.getNotificationContentText();
        if (contentText != null) {
            builder.setContentText(contentText);
        }

        String contentInfo = task.getNotificationContentInfo();
        if (contentInfo != null) {
            builder.setContentInfo(contentInfo);
        }
        builder.setOngoing(true);

        Notification notif = builder.build();
        // notif.setLatestEventInfo(context, contentTitle, contentText,
        // contentIntent)
        //mNotifManager.cancel((int)task.getCreateTime());
        mNotifManager.notify(PROGRESS_ID, notif);
        //mNotifManager.notify((int) task.getCreateTime(), notif);
    }

    /**
     * This method update progress.
     * @param tasks DataProtectionTask queue
     */
    public void updateNotification(Queue<DataProtectionTask> tasks) {
        updateTaskProgressLocked(tasks);
    }

    private void updateTaskProgressLocked(Queue<DataProtectionTask> tasks) {

        Iterator<DataProtectionTask> iter = tasks.iterator();
        while (iter.hasNext()) {
            final DataProtectionTask task = iter.next();
            if (!task.needShowNotification()) {
                Log.d(TAG, "not show notification.");
                continue;
            }
            final Notification.Builder builder = new Notification.Builder(
                    mContext).setChannel(NOTIFICATION_CHANNEL);
            builder.setWhen(task.getCreateTime());
            if (task.getIcon() != 0) {
                builder.setSmallIcon(task.getIcon());
            }

            builder.setProgress(100, task.getProgress(),
                    task.state == DataProtectionTask.STATE_TODO);
            String title = task.getNotificationTitle();
            if (title != null) {
                builder.setContentTitle(title);
            }
            PendingIntent cIntent = task.getContentIntent();
            if (cIntent != null) {
                builder.setContentIntent(cIntent);
            }
            String contentText = task.getNotificationContentText();
            if (contentText != null) {
                builder.setContentText(contentText);
            }
            String contentInfo = task.getNotificationContentInfo();
            if (contentInfo != null) {
                builder.setContentInfo(contentInfo);
            }
            builder.setOngoing(true);

            Notification notif = builder.build();
            // notif.setLatestEventInfo(context, contentTitle, contentText,
            // contentIntent)
            if (task.state == DataProtectionTask.STATE_ONGOING) {
                mNotifManager.notify(PROGRESS_ID, notif);
            } else {
                mNotifManager.notify((int) task.getCreateTime(), notif);
            }
        }
    }

    public void updateDecryptFailNotification() {

    }

    public void updateTaskCompleted(DataProtectionTask task) {
        Log.d(TAG, "updateTaskCompleted... start");
        final Notification.Builder builder = new Notification.Builder(
                mContext).setChannel(NOTIFICATION_CHANNEL);
        builder.setWhen(task.getCreateTime());
        if (task.getIcon() != 0) {
            builder.setSmallIcon(task.getIcon());
        }

        String title = task.getNotificationTitle();
        if (title != null) {
            builder.setContentTitle(title);
        }
        PendingIntent cIntent = task.getContentIntent();
        if (cIntent != null) {
            builder.setContentIntent(cIntent);
        }
        String contentText = task.getNotificationContentText();
        if (contentText != null) {
            builder.setContentText(contentText);
        }
        builder.setAutoCancel(true);
        builder.setTicker(title);

        Notification notif = builder.build();
        mNotifManager.notify((int) task.getCreateTime(), notif);
        Log.d(TAG, "updateTaskCompleted...end: notifiId: " + ((int)
                task.getCreateTime()) + " title " + title +
                " intent: " + cIntent + " content: " + contentText);
    }

    public void updateTaskCancelled(DataProtectionTask task) {
        Log.d(TAG, "updateTaskCancelled: cancelled" + task.getCreateTime());
        final Notification.Builder builder = new Notification.Builder(
                mContext).setChannel(NOTIFICATION_CHANNEL);
        builder.setWhen(task.getCreateTime());
        if (task.getIcon() != 0) {
            builder.setSmallIcon(task.getIcon());
        }

        // builder.setProgress(100, task.getProgress(), false);
        String title = task.getNotificationTitle();
        if (title != null) {
            builder.setContentTitle(title);
        }
        PendingIntent cIntent = task.getContentIntent();
        if (cIntent != null) {
            builder.setContentIntent(cIntent);
        }
        String contentText = task.getNotificationContentText();
        if (contentText != null) {
            builder.setContentText(contentText);
        }
        builder.setAutoCancel(true);
        builder.setTicker(title);

        Notification notif = builder.build();

        mNotifManager.notify((int) task.getCreateTime(), notif);
        Log.d(TAG, "updateTaskCancelled...end: notifiId: " + ((int)
                task.getCreateTime()) + " title " + title +
                " intent: " + cIntent + " content: " + contentText);
    }

    public void updateTaskUnmoundted(DataProtectionTask task) {
        Log.d(TAG, "updateTaskUnmoundted: start " + task.getCreateTime());
        final Notification.Builder builder = new Notification.Builder(
                mContext).setChannel(NOTIFICATION_CHANNEL);
        builder.setWhen(task.getCreateTime());
        if (task.getIcon() != 0) {
            builder.setSmallIcon(task.getIcon());
        }

        // builder.setProgress(100, task.getProgress(), false);
        String title = task.getNotificationTitle();
        if (title != null) {
            builder.setContentTitle(title);
        }
        builder.setTicker(title);
        PendingIntent cIntent = task.getContentIntent();
        if (cIntent != null) {
            builder.setContentIntent(cIntent);
        }
        String contentText = task.getNotificationContentText();
        if (contentText != null) {
            builder.setContentText(contentText);
        }
        builder.setAutoCancel(true);

        Notification notif = builder.build();

        mNotifManager.notify((int) task.getCreateTime(), notif);
        Log.d(TAG, "updateTaskUnmoundted...end: notifiId: " + ((int)
                task.getCreateTime()) + " title " + title +
                " intent: " + cIntent + " content: " + contentText);
    }

    public void updateDecryptFailNotification(String title, String contentInfo,
            long when, PendingIntent cntIntent) {
        Log.d(TAG, "updateDecryptFailNotification: " + when);
        final Notification.Builder builder = new Notification.Builder(
                mContext).setChannel(NOTIFICATION_CHANNEL);
        builder.setWhen(System.currentTimeMillis());

        // builder.setProgress(100, task.getProgress(), false);
        //builder.setSmallIcon(R.drawable.ic_dataprotection_alert_notify);
        builder.setSmallIcon(android.R.drawable.stat_notify_error);

        if (title != null) {
            builder.setContentTitle(title);
        }
        builder.setTicker(title);
        if (cntIntent != null) {
            builder.setContentIntent(cntIntent);
        }
        String contentText = contentInfo;
        if (contentText != null) {
            builder.setContentText(contentText);
        }
        builder.setAutoCancel(true);

        //builder.setTicker("unlocking failed...");

        Notification notif = builder.build();

        mNotifManager.notify((int) when, notif);
    }

    public static void cancelNotification(Context context, int id) {
        NotificationManager notifiManager = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);
        notifiManager.cancel(id);
    }

    public void cancelNotification(int id) {
        mNotifManager.cancel(id);
    }

    public void cancelAll() {
        mNotifManager.cancelAll();
    }
}
