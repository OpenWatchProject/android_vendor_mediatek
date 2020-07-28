/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.cb.cbmsg;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.media.AudioManager;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.text.TextUtils;

import com.android.mms.LogTag;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;
import com.google.android.mms.util.SqliteWrapper;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.NotificationPreferenceActivity;

import java.util.Comparator;
import java.util.HashSet;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

import mediatek.telephony.MtkTelephony;

/**
 * M:
 * This class is used to update the notification indicator. It will check whether
 * there are unread messages. If yes, it would show the notification indicator,
 * otherwise, hide the indicator.
 */
public class CbMessagingNotification {
    private static final String TAG = LogTag.APP;

    public static final int NOTIFICATION_ID = 125;

    private static final Uri URL_MESSAGES = MtkTelephony.SmsCb.CONTENT_URI;

    // This must be consistent with the column constants below.
    private static final String[] CB_STATUS_PROJECTION = new String[] {
            MtkTelephony.SmsCb.THREAD_ID, MtkTelephony.SmsCb.DATE,
            MtkTelephony.SmsCb.CHANNEL_ID, MtkTelephony.SmsCb.BODY, MtkTelephony.SmsCb._ID };

    private static final int COLUMN_THREAD_ID   = 0;
    private static final int COLUMN_DATE        = 1;
    private static final int COLUMN_CB_CHANNEL = 2;
    private static final int COLUMN_CB_BODY    = 3;
    //add this for cmcc dir ui mode
    private static final int COLUMN_CB_ID      = 4;

    private static final String NEW_INCOMING_SM_CONSTRAINT = "(" + "seen" + " = 0)";

    private static final CBNotificationInfoComparator INFO_COMPARATOR =
            new CBNotificationInfoComparator();

    private CbMessagingNotification() {
    }

    /**
     * Checks to see if there are any unread messages or delivery
     * reports.  Shows the most recent notification if there is one.
     *
     * @param context the context to use
     */
    public static void updateNewMessageIndicator(Context context) {
        if (MmsConfig.isSmsEnabled(context)) {
            updateNewMessageIndicator(context, false);
        }
    }

    /**
     * Checks to see if there are any unread messages or delivery
     * reports.  Shows the most recent notification if there is one.
     *
     * @param context the context to use
     * @param isNew if notify a new message comes, it should be true, otherwise, false.
     */
    public static void updateNewMessageIndicator(Context context, boolean isNew) {
        SortedSet<CBNotificationInfo> accumulator =
                new TreeSet<CBNotificationInfo>(INFO_COMPARATOR);
        Set<Long> threads = new HashSet<Long>(4);
        int count = 0;
        count += accumulateNotificationInfo(
                accumulator, getCBNewMessageNotificationInfo(0, context, threads));

        cancelNotification(context, NOTIFICATION_ID);
        if (!accumulator.isEmpty()) {
            MmsLog.d(TAG, "CB message notification is to be delivered!");
            accumulator.first().deliver(0, context, isNew, count, threads.size());
        }
    }

    /**
     * Checks to see if there are any unread messages or delivery
     * reports.  Shows the most recent notification if there is one.
     *
     * @param context the context to use
     * @param isNew if notify a new message comes, it should be true, otherwise, false.
     * @param subId sub Id of new message
     */
    public static void updateNewMessageIndicator(int subId, Context context, boolean isNew) {
        SortedSet<CBNotificationInfo> accumulator =
                new TreeSet<CBNotificationInfo>(INFO_COMPARATOR);
        Set<Long> threads = new HashSet<Long>(4);
        int count = 0;
        count += accumulateNotificationInfo(
                accumulator, getCBNewMessageNotificationInfo(subId, context, threads));

        cancelNotification(context, NOTIFICATION_ID);
        if (!accumulator.isEmpty()) {
            MmsLog.d(TAG, "CB message notification is to be delivered!");
            accumulator.first().deliver(subId, context, isNew, count, threads.size());
        }
    }

    /**
     * Updates all pending notifications, clearing or updating them as
     * necessary.  This task is completed in the background on a worker
     * thread.
     */
    public static void updateAllNotifications(final Context context) {
        new Thread(new Runnable() {
            public void run() {
                updateNewMessageIndicator(context);
            }
        }).start();
    }

    private static int accumulateNotificationInfo(
            SortedSet set, CBNotificationInfo info) {
        if (info != null) {
            set.add(info);
            return info.mCount;
        }
        return 0;
    }

    private static final class CBNotificationInfo {
        public Intent mClickIntent;
        public String mDescription;
        public int mIconResourceId;
        public CharSequence mTicker;
        public long mTimeMillis;
        public String mTitle;
        public int mCount;
        //add for cmcc dir ui mode
        public Uri mUri;

        public CBNotificationInfo(
                Intent clickIntent, String description, int iconResourceId,
                CharSequence ticker, long timeMillis, String title, int count, Uri uri) {
            mClickIntent = clickIntent;
            mDescription = description;
            mIconResourceId = iconResourceId;
            mTicker = ticker;
            mTimeMillis = timeMillis;
            mTitle = title;
            mCount = count;
            mUri = uri;
        }

        public void deliver(int subId, Context context, boolean isNew, int count,
                      int uniqueThreads) {
            updateNotification(
                    context, mClickIntent, mDescription, mIconResourceId, isNew,
                    (isNew ? mTicker : null), // only display the ticker if the message is new
                    mTimeMillis, mTitle, count, uniqueThreads, mUri, subId);
        }

        public long getTime() {
            return mTimeMillis;
        }
    }

    private static final class CBNotificationInfoComparator
            implements Comparator<CBNotificationInfo> {
        public int compare(
                CBNotificationInfo info1, CBNotificationInfo info2) {
            return Long.signum(info2.getTime() - info1.getTime());
        }
    }

    public static final CBNotificationInfo getCBNewMessageNotificationInfo(
            int subId, Context context, Set<Long> threads) {
        return getCBNewMessageNotificationInfoBase(subId, context, threads);
    }

    public static final CBNotificationInfo getCBNewMessageNotificationInfoBase(
            int subId, Context context, Set<Long> threads) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = SqliteWrapper.query(context, resolver, URL_MESSAGES,
                            CB_STATUS_PROJECTION, NEW_INCOMING_SM_CONSTRAINT,
                            null, MtkTelephony.SmsCb.DATE + " desc");

        if (cursor == null) {
            return null;
        }

        try {
            if (!cursor.moveToFirst()) {
                return null;
            }
            // TODO get the description by query the channel table.
            int channelId = cursor.getInt(COLUMN_CB_CHANNEL);
            String body = cursor.getString(COLUMN_CB_BODY);
            long threadId = cursor.getLong(COLUMN_THREAD_ID);
            long timeMillis = cursor.getLong(COLUMN_DATE);
            //add for cmcc dir ui mode
            long id = cursor.getLong(COLUMN_CB_ID);
            //String address = Conversation.getChannelNameFromId(context, channel_id);
            String address;
            address = CbMessage.getCBChannelName(subId, channelId);
            address = address + "(" + channelId + ")";
            CBNotificationInfo info = getNewMessageNotificationInfo(
                    address, body, context, R.drawable.stat_notify_sms,
                    null, threadId, timeMillis, cursor.getCount(),
                    URL_MESSAGES.buildUpon().appendPath(Long.toString(id)).build());

            threads.add(threadId);
            while (cursor.moveToNext()) {
                threads.add(cursor.getLong(COLUMN_THREAD_ID));
            }

            return info;
        } finally {
            cursor.close();
        }
    }

    private static CBNotificationInfo getNewMessageNotificationInfo(
            String address,
            String body,
            Context context,
            int iconResourceId,
            String subject,
            long threadId,
            long timeMillis,
            int count,
            Uri uri) {
        Intent clickIntent = CbMessageListActivity.createIntent(context, threadId);
        clickIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);

        String senderInfo = buildTickerMessage(
                context, address, null, null).toString();
        String senderInfoName = senderInfo.substring(
                0, senderInfo.length() - 2);
        CharSequence ticker = buildTickerMessage(
                context, address, subject, body);

        return new CBNotificationInfo(
                clickIntent, body, iconResourceId, ticker, timeMillis,
                senderInfoName, count, uri);
    }

    public static void cancelNotification(Context context, int notificationId) {
        NotificationManager nm = (NotificationManager) context.getSystemService(
                Context.NOTIFICATION_SERVICE);
        MmsLog.d(TAG, "Cancel notification notificationId = " + notificationId);
        nm.cancel(notificationId);
    }

    private static void updateNotification(
            Context context,
            Intent clickIntent,
            String description,
            int iconRes,
            boolean isNew,
            CharSequence ticker,
            long timeMillis,
            String title,
            int messageCount,
            int uniqueThreadCount,
            Uri uri,
            int subId) {
        if (!MessagingNotification.isNotificationEnable(context)) {
            MmsLog.d(TAG, "app notification set disabled!");
            return;
        }

        Notification.Builder builder = new Notification.Builder(context);
        builder.setSmallIcon(iconRes);
        builder.setTicker(ticker);
        builder.setWhen(timeMillis);

        // If we have more than one unique thread, change the title (which would
        // normally be the contact who sent the message) to a generic one that
        // makes sense for multiple senders, and change the Intent to take the
        // user to the conversation list instead of the specific thread.
        if (uniqueThreadCount > 1) {
            MmsLog.d(TAG, "ConversationList will be resumed because uniqueThreadCount = "
                    + uniqueThreadCount);
            title = context.getString(R.string.notification_multiple_cb_title);
            clickIntent = new Intent(Intent.ACTION_MAIN);
            clickIntent.setFlags(
                    Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            clickIntent.setClassName("com.android.mms", "com.android.mms.ui.ConversationList");

        }

        // If there is more than one message, change the description (which
        // would normally be a snippet of the individual message text) to
        // a string indicating how many unread messages there are.
        if (messageCount > 1) {
            description = context.getString(R.string.notification_multiple_cb,
                    Integer.toString(messageCount));
        }

        // Make a startActivity() PendingIntent for the notification.
        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, clickIntent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        // Update the notification.
        builder.setContentTitle(title);
        builder.setContentText(description);
        builder.setContentIntent(pendingIntent);
        builder.setChannelId(MessagingNotification.CHANNEL_ID);
        int defaults = 0;
        if (isNew) {
            MmsLog.d(TAG, "The received CB message is new.");
            MessagingNotification.processNotificationSound(context);
        }
        defaults |= Notification.FLAG_SHOW_LIGHTS;
        builder.setDefaults(defaults);
        builder.setLights(0xff00ff00, 500, 2000);

        NotificationManager nm = (NotificationManager)
            context.getSystemService(Context.NOTIFICATION_SERVICE);
        //nm.createNotificationChannel(MessagingNotification.mDefaultNotificationChannel);
        nm.notify(NOTIFICATION_ID, builder.build());
    }

    protected static CharSequence buildTickerMessage(
            Context context, String address, String subject, String body) {
        return MessageUtils.formatMsgContent(subject, body, address);
    }

    // threadIdResult[0] contains the thread id of the first message.
    // threadIdResult[1] is nonzero if the thread ids of all the messages are the same.
    // You can pass in null for threadIdResult.
    // You can pass in a threadIdResult of size 1 to avoid the comparison of each thread id.
    private static int getUndeliveredMessageCount(Context context, long[] threadIdResult) {
        Cursor undeliveredCursor = SqliteWrapper.query(context, context.getContentResolver(),
                URL_MESSAGES, new String[] { MtkTelephony.SmsCb.THREAD_ID }, "read=0", null, null);
        if (undeliveredCursor == null) {
            return 0;
        }
        int count = undeliveredCursor.getCount();
        try {
            if (threadIdResult != null && undeliveredCursor.moveToFirst()) {
                threadIdResult[0] = undeliveredCursor.getLong(0);
                if (threadIdResult.length >= 2) {
                    // Test to see if all the undelivered messages belong to the same thread.
                    long firstId = threadIdResult[0];
                    while (undeliveredCursor.moveToNext()) {
                        if (undeliveredCursor.getLong(0) != firstId) {
                            firstId = 0;
                            break;
                        }
                    }
                    threadIdResult[1] = firstId;    // non-zero if all ids are the same
                }
            }
        } finally {
            undeliveredCursor.close();
        }
        MmsLog.d(TAG, "UndeliveredMessageCount count = " + count);
        return count;
    }

    public static boolean isFailedToDeliver(Intent intent) {
        return (intent != null) && intent.getBooleanExtra("undelivered_flag", false);
    }

    public static boolean isFailedToDownload(Intent intent) {
        return (intent != null) && intent.getBooleanExtra("failed_download_flag", false);
    }
}
