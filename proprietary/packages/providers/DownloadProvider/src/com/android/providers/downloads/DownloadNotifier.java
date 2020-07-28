/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.providers.downloads;

import static android.app.DownloadManager.Request.VISIBILITY_VISIBLE;
import static android.app.DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED;
import static android.app.DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_ONLY_COMPLETION;
import static android.provider.Downloads.Impl.STATUS_QUEUED_FOR_WIFI;
import static android.provider.Downloads.Impl.STATUS_RUNNING;
import static com.android.providers.downloads.Constants.DEBUG;
import static com.android.providers.downloads.Constants.TAG;

import android.app.DownloadManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.SystemClock;
import android.provider.Downloads;
import android.service.notification.StatusBarNotification;
import android.text.TextUtils;
import android.text.format.DateUtils;
import android.util.ArrayMap;
import android.util.IntArray;
import android.util.Log;
import android.util.LongSparseLongArray;

import com.android.internal.util.ArrayUtils;
import com.mediatek.mtkdownloadmanager.ext.IDownloadProviderFeatureExt;

import java.net.MalformedURLException;
import java.net.URL;
import java.text.NumberFormat;
import java.util.HashMap;
import java.util.HashSet;

import javax.annotation.concurrent.GuardedBy;

/**
 * Update {@link NotificationManager} to reflect current download states.
 * Collapses similar downloads into a single notification, and builds
 * {@link PendingIntent} that launch towards {@link DownloadReceiver}.
 */
public class DownloadNotifier {

    private static final int TYPE_ACTIVE = 1;
    private static final int TYPE_WAITING = 2;
    private static final int TYPE_COMPLETE = 3;

    private static final String CHANNEL_ACTIVE = "active";
    private static final String CHANNEL_WAITING = "waiting";
    private static final String CHANNEL_COMPLETE = "complete";

    private final Context mContext;
    private final NotificationManager mNotifManager;

    /// M: add to fix 452723. @{
    private static HashMap <Long, Intent> sPendingIntents;
    /// @}

    /// M: add to fix 1918727. @{
    private static HashSet <Long> sUpdateDoneItems;
    /// @}

    private static IDownloadProviderFeatureExt sDownloadProviderFeatureExt;

    /**
     * Currently active notifications, mapped from clustering tag to timestamp
     * when first shown.
     *
     * @see #buildNotificationTag(Cursor)
     */
    @GuardedBy("mActiveNotifs")
    private final ArrayMap<String, Long> mActiveNotifs = new ArrayMap<>();

    /**
     * Current speed of active downloads, mapped from download ID to speed in
     * bytes per second.
     */
    @GuardedBy("mDownloadSpeed")
    private final LongSparseLongArray mDownloadSpeed = new LongSparseLongArray();

    /**
     * Last time speed was reproted, mapped from download ID to
     * {@link SystemClock#elapsedRealtime()}.
     */
    @GuardedBy("mDownloadSpeed")
    private final LongSparseLongArray mDownloadTouch = new LongSparseLongArray();

    public DownloadNotifier(Context context) {
        mContext = context;
        mNotifManager = context.getSystemService(NotificationManager.class);

        /// M: add to fix 452723. @{
        sPendingIntents = new HashMap<Long, Intent>();
        /// @}
        /// M: add to fix 1918727. @{
        sUpdateDoneItems = new HashSet<Long>();
        /// @}

        // Ensure that all our channels are ready to use
        mNotifManager.createNotificationChannel(new NotificationChannel(CHANNEL_ACTIVE,
                context.getText(R.string.download_running),
                NotificationManager.IMPORTANCE_LOW));
        mNotifManager.createNotificationChannel(new NotificationChannel(CHANNEL_WAITING,
                context.getText(R.string.download_queued),
                NotificationManager.IMPORTANCE_DEFAULT));
        mNotifManager.createNotificationChannel(new NotificationChannel(CHANNEL_COMPLETE,
                context.getText(com.android.internal.R.string.done_label),
                NotificationManager.IMPORTANCE_DEFAULT));
    }

    public void init() {
        synchronized (mActiveNotifs) {
            mActiveNotifs.clear();
            final StatusBarNotification[] notifs = mNotifManager.getActiveNotifications();
            if (!ArrayUtils.isEmpty(notifs)) {
                for (StatusBarNotification notif : notifs) {
                    mActiveNotifs.put(notif.getTag(), notif.getPostTime());
                }
            }
        }
    }

    /**
     * Notify the current speed of an active download, used for calculating
     * estimated remaining time.
     */
    public void notifyDownloadSpeed(long id, long bytesPerSecond) {
        synchronized (mDownloadSpeed) {
            if (bytesPerSecond != 0) {
                mDownloadSpeed.put(id, bytesPerSecond);
                mDownloadTouch.put(id, SystemClock.elapsedRealtime());
            } else {
                mDownloadSpeed.delete(id);
                mDownloadTouch.delete(id);
            }
        }
    }

    private interface UpdateQuery {
        final String[] PROJECTION = new String[] {
                Downloads.Impl._ID,
                Downloads.Impl.COLUMN_STATUS,
                Downloads.Impl.COLUMN_VISIBILITY,
                Downloads.Impl.COLUMN_NOTIFICATION_PACKAGE,
                Downloads.Impl.COLUMN_CURRENT_BYTES,
                Downloads.Impl.COLUMN_TOTAL_BYTES,
                Downloads.Impl.COLUMN_DESTINATION,
                Downloads.Impl.COLUMN_TITLE,
                Downloads.Impl.COLUMN_DESCRIPTION,
                /// M: Add for operator feature get the notificationtext for completing download. @{
                Downloads.Impl._DATA,
                Downloads.Impl.COLUMN_MIME_TYPE,
                /// @}
        };

        final int _ID = 0;
        final int STATUS = 1;
        final int VISIBILITY = 2;
        final int NOTIFICATION_PACKAGE = 3;
        final int CURRENT_BYTES = 4;
        final int TOTAL_BYTES = 5;
        final int DESTINATION = 6;
        final int TITLE = 7;
        final int DESCRIPTION = 8;
        /// M: Add for operator feature get the notificationtext for completing download. @{
        final int _DATA = 9;
        final int MIME_TYPE = 10;
        /// @}
    }

    public void update() {
        try (Cursor cursor = mContext.getContentResolver().query(
                Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI, UpdateQuery.PROJECTION,
                Downloads.Impl.COLUMN_DELETED + " == '0'", null, null)) {
            synchronized (mActiveNotifs) {
                updateWithLocked(cursor);
            }
        }
    }

    private void updateWithLocked(Cursor cursor) {
        final Resources res = mContext.getResources();

        // Cluster downloads together
        final ArrayMap<String, IntArray> clustered = new ArrayMap<>();
        /// M: Add for null cursor check. @{
        if (cursor == null) {
            return;
        }
        /// @}
        while (cursor.moveToNext()) {
            final String tag = buildNotificationTag(cursor);
            if (tag != null) {
                IntArray cluster = clustered.get(tag);
                if (cluster == null) {
                    cluster = new IntArray();
                    clustered.put(tag, cluster);
                }
                cluster.add(cursor.getPosition());
            }
        }

        // Build notification for each cluster
        for (int i = 0; i < clustered.size(); i++) {
            final String tag = clustered.keyAt(i);
            final IntArray cluster = clustered.valueAt(i);
            final int type = getNotificationTagType(tag);

            final Notification.Builder builder;
            if (type == TYPE_ACTIVE) {
                builder = new Notification.Builder(mContext, CHANNEL_ACTIVE);
                builder.setSmallIcon(android.R.drawable.stat_sys_download);
            } else if (type == TYPE_WAITING) {
                builder = new Notification.Builder(mContext, CHANNEL_WAITING);
                builder.setSmallIcon(android.R.drawable.stat_sys_warning);
            } else if (type == TYPE_COMPLETE) {
                builder = new Notification.Builder(mContext, CHANNEL_COMPLETE);
                builder.setSmallIcon(android.R.drawable.stat_sys_download_done);
            } else {
                continue;
            }

            builder.setColor(res.getColor(
                    com.android.internal.R.color.system_notification_accent_color));

            // Use time when cluster was first shown to avoid shuffling
            final long firstShown;
            if (mActiveNotifs.containsKey(tag)) {
                firstShown = mActiveNotifs.get(tag);
            } else {
                firstShown = System.currentTimeMillis();
                mActiveNotifs.put(tag, firstShown);
            }
            builder.setWhen(firstShown);
            builder.setOnlyAlertOnce(true);

            // Build action intents
            if (type == TYPE_ACTIVE || type == TYPE_WAITING) {
                final long[] downloadIds = getDownloadIds(cursor, cluster);

                // build a synthetic uri for intent identification purposes
                final Uri uri = new Uri.Builder().scheme("active-dl").appendPath(tag).build();
                final Intent intent = new Intent(Constants.ACTION_LIST,
                        uri, mContext, DownloadReceiver.class);
                intent.putExtra(DownloadManager.EXTRA_NOTIFICATION_CLICK_DOWNLOAD_IDS,
                        downloadIds);
                builder.setContentIntent(PendingIntent.getBroadcast(mContext,
                        0, intent, PendingIntent.FLAG_UPDATE_CURRENT));
                if (type == TYPE_ACTIVE) {
                    builder.setOngoing(true);
                }

                // Add a Cancel action
                final Uri cancelUri = new Uri.Builder().scheme("cancel-dl").appendPath(tag).build();
                final Intent cancelIntent = new Intent(Constants.ACTION_CANCEL,
                        cancelUri, mContext, DownloadReceiver.class);
                cancelIntent.putExtra(DownloadReceiver.EXTRA_CANCELED_DOWNLOAD_IDS, downloadIds);
                cancelIntent.putExtra
                (DownloadReceiver.EXTRA_CANCELED_DOWNLOAD_NOTIFICATION_TAG, tag);

                builder.addAction(
                    android.R.drawable.ic_menu_close_clear_cancel,
                    res.getString(R.string.button_cancel_download),
                    PendingIntent.getBroadcast(mContext,
                            0, cancelIntent, PendingIntent.FLAG_UPDATE_CURRENT));

            } else if (type == TYPE_COMPLETE) {
                cursor.moveToPosition(cluster.get(0));
                final long id = cursor.getLong(UpdateQuery._ID);
                final int status = cursor.getInt(UpdateQuery.STATUS);
                final int destination = cursor.getInt(UpdateQuery.DESTINATION);

                final Uri uri = ContentUris.withAppendedId(
                        Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI, id);
                builder.setAutoCancel(true);

                final String action;
                if (Downloads.Impl.isStatusError(status)) {
                    action = Constants.ACTION_LIST;
                } else {
                    if (destination != Downloads.Impl.DESTINATION_SYSTEMCACHE_PARTITION) {
                        action = Constants.ACTION_OPEN;
                    } else {
                        action = Constants.ACTION_LIST;
                    }
                }

                final Intent intent = new Intent(action, uri, mContext, DownloadReceiver.class);
                intent.putExtra(DownloadManager.EXTRA_NOTIFICATION_CLICK_DOWNLOAD_IDS,
                        getDownloadIds(cursor, cluster));
                builder.setContentIntent(PendingIntent.getBroadcast(mContext,
                        0, intent, PendingIntent.FLAG_UPDATE_CURRENT));

                /// M: add to fix 452723. @{
                if (!sPendingIntents.containsKey(id)) {
                     sPendingIntents.put(id, intent);
                     Log.i(Constants.TAG, "sPendingIntents.put(), id: " + id);
                }
                /// @}

                final Intent hideIntent = new Intent(Constants.ACTION_HIDE,
                        uri, mContext, DownloadReceiver.class);
                builder.setDeleteIntent(PendingIntent.getBroadcast(mContext, 0, hideIntent, 0));
            }

            // Calculate and show progress
            String remainingText = null;
            String percentText = null;
            if (type == TYPE_ACTIVE) {
                long current = 0;
                long total = 0;
                long speed = 0;
                synchronized (mDownloadSpeed) {
                    for (int j = 0; j < cluster.size(); j++) {
                        cursor.moveToPosition(cluster.get(j));

                        final long id = cursor.getLong(UpdateQuery._ID);
                        final long currentBytes = cursor.getLong(UpdateQuery.CURRENT_BYTES);
                        final long totalBytes = cursor.getLong(UpdateQuery.TOTAL_BYTES);

                        if (totalBytes != -1) {
                            current += currentBytes;
                            total += totalBytes;
                            speed += mDownloadSpeed.get(id);
                        }
                    }
                }

                if (total > 0) {
                    percentText =
                            NumberFormat.getPercentInstance().format((double) current / total);

                    if (speed > 0) {
                        final long remainingMillis = ((total - current) * 1000) / speed;
                        remainingText = res.getString(R.string.download_remaining,
                                DateUtils.formatDuration(remainingMillis));
                    }

                    final int percent = (int) ((current * 100) / total);
                    builder.setProgress(100, percent, false);
                } else {
                    builder.setProgress(100, 0, true);
                }
            }

            // Build titles and description
            final Notification notif;
            if (cluster.size() == 1) {
                cursor.moveToPosition(cluster.get(0));
                builder.setContentTitle(getDownloadTitle(res, cursor));

                if (type == TYPE_ACTIVE) {
                    final String description = cursor.getString(UpdateQuery.DESCRIPTION);
                    if (!TextUtils.isEmpty(description)) {
                        builder.setContentText(description);
                    } else {
                        builder.setContentText(remainingText);
                    }
                    builder.setContentInfo(percentText);

                } else if (type == TYPE_WAITING) {
                    builder.setContentText(
                            res.getString(R.string.notification_need_wifi_for_size));

                } else if (type == TYPE_COMPLETE) {
                    final int status = cursor.getInt(UpdateQuery.STATUS);
                    final String packageName = cursor.getString(UpdateQuery.NOTIFICATION_PACKAGE);
                    final String fileName = cursor.getString(UpdateQuery._DATA);
                    final String mimeType = Intent.normalizeMimeType(cursor.getString
                            (UpdateQuery.MIME_TYPE));
                    if (Downloads.Impl.isStatusError(status)) {
                        builder.setContentText(res.getText(R.string.notification_download_failed));
                    } else if (Downloads.Impl.isStatusSuccess(status)) {
                        /// M: Operator Feature get the notificationtext for completing download. @{
                        String caption = null;
                        if (packageName != null && mimeType != null) {
                            sDownloadProviderFeatureExt = PluginFactory.getDefault(mContext);
                            caption = sDownloadProviderFeatureExt.getNotificationText(packageName,
                                    mimeType, fileName);
                        }
                        if (caption == null) {
                        builder.setContentText(
                                res.getText(R.string.notification_download_complete));
                        } else {
                            builder.setContentText(caption);
                        }
                        /// @}
                    }
                }

                notif = builder.build();

            } else {
                final Notification.InboxStyle inboxStyle = new Notification.InboxStyle(builder);

                for (int j = 0; j < cluster.size(); j++) {
                    cursor.moveToPosition(cluster.get(j));
                    inboxStyle.addLine(getDownloadTitle(res, cursor));
                }

                if (type == TYPE_ACTIVE) {
                    builder.setContentTitle(res.getQuantityString(
                            R.plurals.notif_summary_active, cluster.size(), cluster.size()));
                    builder.setContentText(remainingText);
                    builder.setContentInfo(percentText);
                    inboxStyle.setSummaryText(remainingText);

                } else if (type == TYPE_WAITING) {
                    builder.setContentTitle(res.getQuantityString(
                            R.plurals.notif_summary_waiting, cluster.size(), cluster.size()));
                    builder.setContentText(
                            res.getString(R.string.notification_need_wifi_for_size));
                    inboxStyle.setSummaryText(
                            res.getString(R.string.notification_need_wifi_for_size));
                }

                notif = inboxStyle.build();
            }
            /// M: add to fix 1918727. @{
            if (type == TYPE_COMPLETE) {
                cursor.moveToPosition(cluster.get(0));
                final long id = cursor.getLong(UpdateQuery._ID);
                if (sUpdateDoneItems.contains(id)) {
                    if (Constants.DEBUG) Log.i(Constants.TAG,
                        "sUpdateDoneItems contains,will continue, id: "
                            + id);
                    continue;
                }
            }
            /// @}

            mNotifManager.notify(tag, 0, notif);

            /// M: add to fix 1918727. @{
            if (type == TYPE_COMPLETE) {
                cursor.moveToPosition(cluster.get(0));
                final long id = cursor.getLong(UpdateQuery._ID);
                sUpdateDoneItems.add(id);
            }
            /// @}
        }

        // Remove stale tags that weren't renewed
        for (int i = 0; i < mActiveNotifs.size(); ) {
            final String tag = mActiveNotifs.keyAt(i);
            if (clustered.containsKey(tag)) {
                i++;
            } else {
                mNotifManager.cancel(tag, 0);
                mActiveNotifs.removeAt(i);
            }
        }
    }

    private static CharSequence getDownloadTitle(Resources res, Cursor cursor) {
        final String title = cursor.getString(UpdateQuery.TITLE);
        if (!TextUtils.isEmpty(title)) {
            return title;
        } else {
            return res.getString(R.string.download_unknown_title);
        }
    }

    private long[] getDownloadIds(Cursor cursor, IntArray cluster) {
        final long[] ids = new long[cluster.size()];
        for (int i = 0; i < cluster.size(); i++) {
            cursor.moveToPosition(cluster.get(i));
            ids[i] = cursor.getLong(UpdateQuery._ID);
        }
        return ids;
    }

    public void dumpSpeeds() {
        synchronized (mDownloadSpeed) {
            for (int i = 0; i < mDownloadSpeed.size(); i++) {
                final long id = mDownloadSpeed.keyAt(i);
                final long delta = SystemClock.elapsedRealtime() - mDownloadTouch.get(id);
                Log.d(TAG, "Download " + id + " speed " + mDownloadSpeed.valueAt(i) + "bps, "
                        + delta + "ms ago");
            }
        }
    }

    /**
     * Build tag used for collapsing several downloads into a single
     * {@link Notification}.
     */
    private static String buildNotificationTag(Cursor cursor) {
        final long id = cursor.getLong(UpdateQuery._ID);
        final int status = cursor.getInt(UpdateQuery.STATUS);
        final int visibility = cursor.getInt(UpdateQuery.VISIBILITY);
        final String notifPackage = cursor.getString(UpdateQuery.NOTIFICATION_PACKAGE);

        if (isQueuedAndVisible(status, visibility)) {
            return TYPE_WAITING + ":" + notifPackage;
        } else if (isActiveAndVisible(status, visibility)) {
            return TYPE_ACTIVE + ":" + notifPackage;
        } else if (isCompleteAndVisible(status, visibility)) {
            // Complete downloads always have unique notifs
            return TYPE_COMPLETE + ":" + id;
        } else {
            return null;
        }
    }

    /**
     * Return the cluster type of the given tag, as created by
     * {@link #buildNotificationTag(Cursor)}.
     */
    private static int getNotificationTagType(String tag) {
        return Integer.parseInt(tag.substring(0, tag.indexOf(':')));
    }

    private static boolean isQueuedAndVisible(int status, int visibility) {
        return status == STATUS_QUEUED_FOR_WIFI &&
                (visibility == VISIBILITY_VISIBLE
                || visibility == VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
    }

    private static boolean isActiveAndVisible(int status, int visibility) {
        return status == STATUS_RUNNING &&
                (visibility == VISIBILITY_VISIBLE
                || visibility == VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
    }

    private static boolean isCompleteAndVisible(int status, int visibility) {
        return Downloads.Impl.isStatusCompleted(status) &&
                (visibility == VISIBILITY_VISIBLE_NOTIFY_COMPLETED
                || visibility == VISIBILITY_VISIBLE_NOTIFY_ONLY_COMPLETION);
    }

    /// M: add to fix 452723. @{
    HashMap getPendingIntentsMap() {
        return sPendingIntents;
    }

    Context getNotificationContext() {
        return mContext;
    }
    /// @}

    /// M: add to fix 1918727. @{
    HashSet getUpdateDoneItems() {
        return sUpdateDoneItems;
    }
    /// @}

    void onDownloadDeleted(final DownloadInfo info) {
        Helpers.getAsyncHandler().post(new Runnable() {
            @Override
            public void run() {
                final long id = info.mId;
              /// M: add to fix 452723. @{
                Intent contentIntent = (Intent) getPendingIntentsMap().get(id);
                if (contentIntent != null) {
                    if (PendingIntent.getBroadcast(getNotificationContext(), 0,
                            contentIntent, PendingIntent.FLAG_NO_CREATE) != null) {
                        PendingIntent.getBroadcast(getNotificationContext(), 0,
                                contentIntent, PendingIntent.FLAG_UPDATE_CURRENT).cancel();
                    }

                    Intent deleteIntent = new Intent(Constants.ACTION_HIDE);
                    deleteIntent.setClassName("com.android.providers.downloads",
                            DownloadReceiver.class.getName());
                    deleteIntent.setData(contentIntent.getData());

                    if (PendingIntent.getBroadcast(getNotificationContext(), 0,
                            deleteIntent, PendingIntent.FLAG_NO_CREATE) != null) {
                        PendingIntent.getBroadcast(getNotificationContext(), 0,
                                deleteIntent, PendingIntent.FLAG_UPDATE_CURRENT).cancel();
                    }

                    getPendingIntentsMap().remove(id);
                }
                /// @}
                /// M: add to fix 1918727. @{
                getUpdateDoneItems().remove(id);
                /// @}

                Log.d(Constants.TAG, "update notifier from onDownloadDeleted");
                update();

                /// M: Add this to support OMA DL
                /// When user click "Cancel" on dd file AlertDialog, and dd file contain install url
                /// The download item is deleted and will notify to web server
                /// If user cancel download the media object, will notify to web server. @{
                if (!MtkDownloads.Impl.OMA_DOWNLOAD_SUPPORT) {
                    return;
                }
                if (DEBUG) Log.i(Constants.LOG_OMA_DL,
                        "DownloadService: deleteDownload before notifyUrl: info.mStatus is :"
                                + info.mStatus + "info.mFileName is: " + info.mFileName
                                + " info.mOmaDownload is:" + info.mOmaDownload
                                + " info.mOmaDownloadInsNotifyUrl is:"
                                + info.mOmaDownloadInsNotifyUrl
                                + " info.mOmaDownloadStatus is: " + info.mOmaDownloadStatus);

                if ((((info.mStatus == Downloads.Impl.STATUS_CANCELED ||
                        info.mStatus == Downloads.Impl.STATUS_PAUSED_BY_APP)
                        && (info.mOmaDownload == 1 && info.mOmaDownloadInsNotifyUrl != null)
                        && (info.mOmaDownloadStatus
                                < MtkDownloads.Impl.OMADL_STATUS_DOWNLOAD_COMPLETELY
                        || info.mOmaDownloadStatus
                        == MtkDownloads.Impl.OMADL_STATUS_PARSE_DDFILE_SUCCESS)))
                        || (info.mOmaDownloadInsNotifyUrl != null
                        && (info.mOmaDownloadStatus
                                == MtkDownloads.Impl.OMADL_STATUS_ERROR_USER_CANCELLED
                        || info.mOmaDownloadStatus
                        == MtkDownloads.Impl.OMADL_STATUS_ERROR_NON_ACCEPTABLE_CONTENT))) {
                    final int omaDownloadStatus = info.mOmaDownloadStatus;
                    try {
                        final URL notifyUrl = new URL(info.mOmaDownloadInsNotifyUrl);
                        Thread nofiyUrlThread = new Thread() {
                            public void run() {
                                OmaDescription omaDescription = new OmaDescription();
                                omaDescription.setInstallNotifyUrl(notifyUrl);
                                // installNotify need return value and Handler may not used
                                if (omaDownloadStatus
                                        ==
                                    MtkDownloads.Impl.OMADL_STATUS_ERROR_NON_ACCEPTABLE_CONTENT) {
                                    omaDescription.setStatusCode(
                                            OmaStatusHandler.NON_ACCEPTABLE_CONTENT);
                                    Log.i(Constants.LOG_OMA_DL,
                                            "DownloadService: deleteDownload(): " +
                                            "nofiyUrlThread: before install notify:"
                                            + "NON_ACCEPTABLE_CONTENT");
                                } else {
                                    omaDescription.setStatusCode(OmaStatusHandler.USER_CANCELLED);
                                    Log.i(Constants.LOG_OMA_DL,
                                            "DownloadService: deleteDownload(): " +
                                            "nofiyUrlThread: before install notify:USER_CANCELLED");
                                }
                                // handler set to null will occur exception
                                OmaDownload.installNotify(omaDescription, null);
                            }
                        };
                        nofiyUrlThread.start();
                    } catch (MalformedURLException e) {
                        Log.e(Constants.TAG, "MalformedURLException: " + e);
                    }
                }
                /// @}
            }
        });
    }
}
