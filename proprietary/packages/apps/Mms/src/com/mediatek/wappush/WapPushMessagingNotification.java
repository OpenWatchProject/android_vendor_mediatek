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
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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

package com.mediatek.wappush;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.TaskStackBuilder;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.media.AudioManager;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.os.UserHandle;
import android.telephony.TelephonyManager;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.TextAppearanceSpan;

import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.NotificationPlayer;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;
import com.google.android.mms.util.SqliteWrapper;

import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.NotificationPreferenceActivity;
import com.mediatek.wappush.ui.WPMessageActivity;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

import mediatek.telephony.MtkTelephony.WapPush;

/**
 * This class is used to update the notification indicator. It will check whether
 * there are unread messages. If yes, it would show the notification indicator,
 * otherwise, hide the indicator.
 */
public class WapPushMessagingNotification {
    private static final String TAG = "Mms/WapPush";

    private static final int NOTIFICATION_ID = 127;
    public static final int SL_AUTOLAUNCH_NOTIFICATION_ID = 5577;
    /**
     * This is the volume at which to play the in-conversation notification sound,
     * expressed as a fraction of the system notification volume.
     */
    private static final float IN_CONVERSATION_NOTIFICATION_VOLUME = 0.25f;

    private static final Uri URL_MESSAGES = WapPush.CONTENT_URI;

    // This must be consistent with the column constants below.
    private static final String[] WAPPUSH_STATUS_PROJECTION = new String[] {
            WapPush.THREAD_ID, WapPush.DATE,
            WapPush._ID, WapPush.URL, WapPush.TEXT, WapPush.ADDR};

    private static final int COLUMN_THREAD_ID   = 0;
    private static final int COLUMN_DATE        = 1;
    private static final int COLUMN_WAPPUSH_ID = 2;
    private static final int COLUMN_WAPPUSH_URL    = 3;
    private static final int COLUMN_WAPPUSH_TEXT    = 4;
    private static final int COLUMN_WAPPUSH_ADDRESS    = 5;

    private static final String[] WAPPUSH_THREAD_ID_PROJECTION =
        new String[] { WapPush.THREAD_ID };
    private static final String NEW_INCOMING_SM_CONSTRAINT =
            "(" + WapPush.SEEN + " = 0)";

    private static final WapPushNotificationInfoComparator INFO_COMPARATOR =
            new WapPushNotificationInfoComparator();

    public static final long THREAD_ALL = -1;
    public static final long THREAD_NONE = -2;
    /**
     * Keeps track of the thread ID of the conversation that's currently displayed to the user
     */
    private static long sCurrentlyDisplayedThreadId;
    private static final Object CURRENTLY_DISPLAYED_THREAD_LOCK = new Object();

    /**
     * mNotificationSet is kept sorted by the incoming message delivery time, with the
     * most recent message first.
     */
    private static SortedSet<WapPushNotificationInfo> sNotificationSet =
            new TreeSet<WapPushNotificationInfo>(INFO_COMPARATOR);

    private static final int MAX_MESSAGES_TO_SHOW = 8;  // the maximum number of new messages to
                                                        // show in a single notification.

    private WapPushMessagingNotification() {
    }

    /**
     * Specifies which message thread is currently being viewed by the user. New messages in that
     * thread will not generate a notification icon and will play the notification sound at a lower
     * volume. Make sure you set this to THREAD_NONE when the UI component that shows the thread is
     * no longer visible to the user (e.g. Activity.onPause(), etc.)
     * @param threadId The ID of the thread that the user is currently viewing. Pass THREAD_NONE
     *  if the user is not viewing a thread, or THREAD_ALL if the user is viewing the conversation
     *  list (note: that latter one has no effect as of this implementation)
     */
    public static void setCurrentlyDisplayedThreadId(long threadId) {
        synchronized (CURRENTLY_DISPLAYED_THREAD_LOCK) {
            sCurrentlyDisplayedThreadId = threadId;
        }
    }

    /**
     * Checks to see if there are any "unseen" messages or delivery
     * reports.  Shows the most recent notification if there is one.
     * Does its work and query in a worker thread.
     *
     * @param context the context to use
     */

    public static void nonBlockingUpdateNewMessageIndicator(
            final Context context, final long newMsgThreadId) {
        MmsLog.d(TAG, "nonBlockingUpdateNewMessageIndicator");
        new Thread(new Runnable() {
            @Override
            public void run() {
                blockingUpdateNewMessageIndicator(context, newMsgThreadId);
            }
        }, "WapPushMessagingNotification.nonBlockingUpdateNewMessageIndicator").start();
    }

    /**
     * Checks to see if there are any "unseen" messages or delivery
     * reports and builds a sorted (by delivery date) list of unread notifications.
     *
     * @param context the context to use
     * @param newMsgThreadId The thread ID of a new message that we're to notify about; if there's
     *  no new message, use THREAD_NONE. If we should notify about multiple or unknown thread IDs,
     *  use THREAD_ALL.
     * @param isStatusMessage
     */
    public static void blockingUpdateNewMessageIndicator(Context context, long newMsgThreadId) {
        MmsLog.d(TAG, "blockingUpdateNewMessageIndicator");
        synchronized (CURRENTLY_DISPLAYED_THREAD_LOCK) {
            if (newMsgThreadId > 0 && newMsgThreadId == sCurrentlyDisplayedThreadId) {
                playInConversationNotificationSound(context);
                return;
            }
        }
        sNotificationSet.clear();

        Set<Long> threads = new HashSet<Long>(4);

        int count = 0;
        addWapPushNotificationInfos(context, threads);

        cancelNotification(context, NOTIFICATION_ID);
        if (!sNotificationSet.isEmpty()) {
            MmsLog.d(TAG, "blockingUpdateNewMessageIndicator: count=" + count +
                    ", newMsgThreadId=" + newMsgThreadId);
            updateNotification(context, newMsgThreadId != THREAD_NONE, threads.size());
        }
    }

    /**
     * Play the in-conversation notification sound (it's the regular notification sound, but
     * played at half-volume
     */
    private static void playInConversationNotificationSound(Context context) {
        Uri ringtoneUri = MessagingNotification.getNewMsgNotifyChannelSound(context);
        NotificationPlayer player = new NotificationPlayer(TAG);
        player.stop();
        player.play(context, ringtoneUri, false, AudioManager.STREAM_NOTIFICATION,
                IN_CONVERSATION_NOTIFICATION_VOLUME);
    }

    private static final class WapPushNotificationInfo {
        public final Intent mClickIntent;
        public final String mMessage;
        public final CharSequence mTicker;
        public final long mTimeMillis;
        public final String mTitle;
        public final Contact mSender;
        public final long mThreadId;
        public Uri mUri;

        /**
         * @param clickIntent where to go when the user taps the notification
         * @param message for a single message, this is the message text
         * @param ticker text displayed ticker-style across the notification, typically formatted
         * as sender: message
         * @param timeMillis date the message was received
         * @param title for a single message, this is the sender
         * @param sender contact of the sender
         * @param threadId thread this message belongs to
         */
        public WapPushNotificationInfo(
                Intent clickIntent, String message,
                CharSequence ticker, long timeMillis, String title,
                Contact sender, long threadId, Uri uri) {
            mClickIntent = clickIntent;
            mMessage = message;
            mTicker = ticker;
            mTimeMillis = timeMillis;
            mTitle = title;
            mSender = sender;
            mThreadId = threadId;
            mUri = uri;
        }

        public long getTime() {
            return mTimeMillis;
        }

        // This is the message string used in bigText and bigPicture notifications.
        public CharSequence formatBigMessage(Context context) {
            // Change multiple newlines(with potential white space between), into a single new line
            final String message =
                    !TextUtils.isEmpty(mMessage) ? mMessage.replaceAll("\\n\\s+", "\n") : "";

            SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
            if (mMessage != null) {
                if (spannableStringBuilder.length() > 0) {
                    spannableStringBuilder.append('\n');
                }
                spannableStringBuilder.append(mMessage);
            }
            MmsLog.dpi(TAG, "formatBigMessage spannableStringBuilder = " + spannableStringBuilder);
            return spannableStringBuilder;
        }

        // This is the message string used in each line of an inboxStyle notification.
        public CharSequence formatInboxMessage(Context context) {
          final TextAppearanceSpan notificationSenderSpan = new TextAppearanceSpan(
                  context, R.style.NotificationPrimaryText);

          final TextAppearanceSpan notificationSubjectSpan = new TextAppearanceSpan(
                  context, R.style.NotificationSubjectText);

          // Change multiple newlines (with potential white space between), into a single new line
          final String message =
                  !TextUtils.isEmpty(mMessage) ? mMessage.replaceAll("\\n\\s+", "\n") : "";

          SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
          final String sender = mSender.getName();
          if (!TextUtils.isEmpty(sender)) {
              spannableStringBuilder.append(sender);
              spannableStringBuilder.setSpan(notificationSenderSpan, 0, sender.length(), 0);
          }
          String separator = context.getString(R.string.notification_separator);
          if (message.length() > 0) {
              if (spannableStringBuilder.length() > 0) {
                  spannableStringBuilder.append(separator);
              }
              int start = spannableStringBuilder.length();
              spannableStringBuilder.append(message);
              spannableStringBuilder.setSpan(notificationSubjectSpan, start,
                      start + message.length(), 0);
          }
          MmsLog.dpi(TAG, "formatInboxMessage spannableStringBuilder = " + spannableStringBuilder);
          return spannableStringBuilder;
        }
    }

    // Return a formatted string with all the sender names separated by commas.
    private static CharSequence formatSenders(Context context,
            ArrayList<WapPushNotificationInfo> senders) {
        final TextAppearanceSpan notificationSenderSpan = new TextAppearanceSpan(
                context, R.style.NotificationPrimaryText);

        String separator = context.getString(R.string.enumeration_comma);   // ", "
        SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
        int len = senders.size();
        for (int i = 0; i < len; i++) {
            if (i > 0) {
                spannableStringBuilder.append(separator);
            }
            spannableStringBuilder.append(senders.get(i).mSender.getName());
        }
        spannableStringBuilder.setSpan(notificationSenderSpan, 0,
                spannableStringBuilder.length(), 0);
        MmsLog.dpi(TAG, "formatSenders spannableStringBuilder = " + spannableStringBuilder);
        return spannableStringBuilder;
    }

    /**
     *
     * Sorts by the time a notification was received in descending order -- newer first.
     *
     */
    private static final class WapPushNotificationInfoComparator
            implements Comparator<WapPushNotificationInfo> {
        @Override
        public int compare(WapPushNotificationInfo info1, WapPushNotificationInfo info2) {
            return Long.signum(info2.getTime() - info1.getTime());
        }
    }

    private static void addWapPushNotificationInfos(
            Context context, Set<Long> threads) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = SqliteWrapper.query(context, resolver, URL_MESSAGES,
                            WAPPUSH_STATUS_PROJECTION, NEW_INCOMING_SM_CONSTRAINT,
                            null, WapPush.DATE + " desc");

        if (cursor == null) {
            MmsLog.w(TAG, "addWapPushNotificationInfos cursor == null");
            return;
        }

        try {
            while (cursor.moveToNext()) {
                // TODO get the description by query the table.
                long messageId = cursor.getLong(COLUMN_WAPPUSH_ID);
                String url = cursor.getString(COLUMN_WAPPUSH_URL);
                String text = cursor.getString(COLUMN_WAPPUSH_TEXT);
                String address = cursor.getString(COLUMN_WAPPUSH_ADDRESS);
                Contact contact = Contact.get(address, false);
                long threadId = cursor.getLong(COLUMN_THREAD_ID);
                long timeMillis = cursor.getLong(COLUMN_DATE);

                //get body
                String body = "";
                if (text != null) {
                    body += text;
                }
                if (url != null) {
                    body += url;
                }

                WapPushNotificationInfo info = getNewMessageNotificationInfo(
                        context, address, body,
                        threadId, timeMillis, contact,
                        URL_MESSAGES.buildUpon().appendPath(Long.toString(messageId)).build());

                /*
                 * for auto-lanuch;
                 * if sl or si's text is null, it should be automatically lanuched.
                 */
                if (text == null || text.equals("")) {
                    int messageCount = cursor.getCount();
                    if (messageCount == 1) {
                        info.mClickIntent.putExtra("URL", url);
                    }
                }
                MmsLog.dpi(TAG, "addWapPushNotificationInfos info = " + info);
                sNotificationSet.add(info);
                threads.add(threadId);
                threads.add(cursor.getLong(COLUMN_THREAD_ID));
            }
        } finally {
            cursor.close();
        }
    }

    private static WapPushNotificationInfo getNewMessageNotificationInfo(
            Context context,
            String address,
            String body,
            long threadId,
            long timeMillis,
            Contact contact,
            Uri uri) {
        Intent clickIntent = WPMessageActivity.createIntent(context, threadId);
        clickIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);

        String senderInfo = buildTickerMessage(
                context, address, null, null).toString();
        String senderInfoName = senderInfo.substring(
                0, senderInfo.length() - 2);
        CharSequence ticker = buildTickerMessage(
                context, address, null, body);
        MmsLog.dpi(TAG, "getNewMessageNotificationInfo clickIntent = " + clickIntent + "threadId = "
                + threadId + "uri = " + uri);
        return new WapPushNotificationInfo(
                clickIntent, body, ticker, timeMillis,
                senderInfoName, contact, threadId, uri);
    }

    public static void cancelNotification(Context context, int notificationId) {
        NotificationManager nm = (NotificationManager) context.getSystemService(
                Context.NOTIFICATION_SERVICE);
        MmsLog.d(TAG, "cancelNotification notificationId = " + notificationId);
        //memslim
        //nm.createNotificationChannel(MessagingNotification.mDefaultNotificationChannel);
        nm.cancelAsUser(TAG, notificationId, UserHandle.CURRENT_OR_SELF);
    }


    /**
     * updateNotification is *the* main function for building the actual notification handed to
     * the NotificationManager
     * @param context
     * @param isNew if we've got a new message, show the ticker
     * @param uniqueThreadCount
     */
    private static void updateNotification(
            Context context,
            boolean isNew,
            int uniqueThreadCount) {
        // If the user has turned off notifications in settings, don't do any notifying.
        if (!MessagingNotification.isNotificationEnable(context)) {
            return;
        }

        // Figure out what we've got -- whether all sms's, mms's, or a mixture of both.
        int messageCount = sNotificationSet.size();

        /// M: check the notification count again.
        if (messageCount == 0) {
            MmsLog.w(TAG, "updateNotification.messageCount is 0.");
            return;
        }

        WapPushNotificationInfo mostRecentNotification = sNotificationSet.first();

        final Notification.Builder noti = new Notification.Builder(context)
                .setWhen(mostRecentNotification.mTimeMillis);

        if (isNew) {
            noti.setTicker(mostRecentNotification.mTicker);
        }
        TaskStackBuilder taskStackBuilder = TaskStackBuilder.create(context);

        // If we have more than one unique thread, change the title (which would
        // normally be the contact who sent the message) to a generic one that
        // makes sense for multiple senders, and change the Intent to take the
        // user to the conversation list instead of the specific thread.

        // Cases:
        //   1) single message from single thread - intent goes to WPMessageActivity
        //   2) multiple messages from single thread - intent goes to WPMessageActivity
        //   3) messages from multiple threads - intent goes to ConversationList

        final Resources res = context.getResources();
        String title = null;
        Bitmap avatar = null;
        MmsLog.d(TAG, "updateNotification uniqueThreadCount = " + uniqueThreadCount);
        if (uniqueThreadCount > 1) {    // messages from multiple threads
            MmsLog.d(TAG, "updateNotification messages from multiple threads");
            Intent mainActivityIntent = new Intent(Intent.ACTION_VIEW);

            mainActivityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_SINGLE_TOP
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);

            mainActivityIntent.setType("vnd.android-dir/wappush");
            taskStackBuilder.addNextIntent(mainActivityIntent);
            title = context.getString(R.string.message_count_notification, messageCount);
        } else {    // same thread, single or multiple messages
            title = mostRecentNotification.mTitle;
            BitmapDrawable contactDrawable = (BitmapDrawable) mostRecentNotification.mSender
                    .getAvatar(context, null, -1);
            if (contactDrawable != null) {
                // Show the sender's avatar as the big icon. Contact bitmaps are 96x96 so we
                // have to scale 'em up to 128x128 to fill the whole notification large icon.
                avatar = contactDrawable.getBitmap();
                if (avatar != null) {
                    final int idealIconHeight =
                        res.getDimensionPixelSize(android.R.dimen.notification_large_icon_height);
                    final int idealIconWidth =
                         res.getDimensionPixelSize(android.R.dimen.notification_large_icon_width);
                    if (avatar.getHeight() < idealIconHeight) {
                        // Scale this image to fit the intended size
                        avatar = Bitmap.createScaledBitmap(
                                avatar, idealIconWidth, idealIconHeight, true);
                    }
                    if (avatar != null) {
                        MmsLog.d(TAG, "updateNotification createScaledBitmap avatar success!");
                        noti.setLargeIcon(avatar);
                    }
                }
            }

            taskStackBuilder.addParentStack(WPMessageActivity.class);
            taskStackBuilder.addNextIntent(mostRecentNotification.mClickIntent);
        }
        // Always have to set the small icon or the notification is ignored
        noti.setSmallIcon(R.drawable.stat_notify_wappush);

        NotificationManager nm = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);
        //memslim
        //nm.createNotificationChannel(MessagingNotification.mDefaultNotificationChannel);

        if (!OpMessageUtils.getOpMessagePlugin()
                .getOpWappushMessagingNotificationExt().updateNotification(context, noti)) {
            noti.setContentIntent(taskStackBuilder.getPendingIntent(0,
                PendingIntent.FLAG_UPDATE_CURRENT, null, UserHandle.CURRENT_OR_SELF));
        }
        noti.setContentTitle(title)
            .setPriority(Notification.PRIORITY_DEFAULT)
            .setChannelId(MessagingNotification.CHANNEL_ID);

        int defaults = 0;

        if (isNew) {
            /** M: when calling still have sound @{ */
            int state = ((TelephonyManager) context.getSystemService(
                    Context.TELEPHONY_SERVICE)).getCallState();
            if (state != TelephonyManager.CALL_STATE_IDLE) {
                noti.setChannelId(MessagingNotification.CHANNEL_ID);
            }
            /** @} */
        } else {
            noti.setChannelId(MessagingNotification.MUTE_CHANNEL_ID);
        }

        defaults |= Notification.DEFAULT_LIGHTS;

        noti.setDefaults(defaults);

        final Notification notification;

        MmsLog.d(TAG, "updateNotification messageCount = " + messageCount);
        if (messageCount == 1) {
            // We've got a single message

            // This sets the text for the collapsed form:
            noti.setContentText(mostRecentNotification.formatBigMessage(context));

            // Show a single notification -- big style with the text of the whole message
            notification = new Notification.BigTextStyle(noti)
                .bigText(mostRecentNotification.formatBigMessage(context))
                .build();
        } else {
            // We've got multiple messages
            MmsLog.d(TAG, "updateNotification uniqueThreadCount = " + uniqueThreadCount);
            if (uniqueThreadCount == 1) {
                // We've got multiple messages for the same thread.
                // Starting with the oldest new message, display the full text of each message.
                // Begin a line for each subsequent message.
                SpannableStringBuilder buf = new SpannableStringBuilder();
                WapPushNotificationInfo infos[] =
                        sNotificationSet.toArray(
                                new WapPushNotificationInfo[sNotificationSet.size()]);
                int len = infos.length;
                for (int i = len - 1; i >= 0; i--) {
                    WapPushNotificationInfo info = infos[i];
                    buf.append(info.formatBigMessage(context));
                    if (i != 0) {
                        buf.append('\n');
                    }
                }

                noti.setContentText(context.getString(R.string.message_count_notification,
                        messageCount));

                // Show a single notification -- big style with the text of all the messages
                notification = new Notification.BigTextStyle(noti)
                    .bigText(buf)
                    // Forcibly show the last line, with the app's smallIcon in it, if we
                    // kicked the smallIcon out with an avatar bitmap
                    .setSummaryText((avatar == null) ? null : " ")
                    .build();
            } else {
                // Build a set of the most recent notification per threadId.
                HashSet<Long> uniqueThreads = new HashSet<Long>(sNotificationSet.size());
                ArrayList<WapPushNotificationInfo> mostRecentNotifPerThread =
                        new ArrayList<WapPushNotificationInfo>();
                Iterator<WapPushNotificationInfo> notifications = sNotificationSet.iterator();
                while (notifications.hasNext()) {
                    WapPushNotificationInfo notificationInfo = notifications.next();
                    if (!uniqueThreads.contains(notificationInfo.mThreadId)) {
                        uniqueThreads.add(notificationInfo.mThreadId);
                        mostRecentNotifPerThread.add(notificationInfo);
                    }
                }
                // When collapsed, show all the senders like this:
                //     Fred Flinstone, Barry Manilow, Pete...
                noti.setContentText(formatSenders(context, mostRecentNotifPerThread));
                Notification.InboxStyle inboxStyle = new Notification.InboxStyle(noti);

                // We have to set the summary text to non-empty so the content text doesn't show
                // up when expanded.
                inboxStyle.setSummaryText(" ");

                // At this point we've got multiple messages in multiple threads. We only
                // want to show the most recent message per thread, which are in
                // mostRecentNotifPerThread.
                int uniqueThreadMessageCount = mostRecentNotifPerThread.size();
                int maxMessages = Math.min(MAX_MESSAGES_TO_SHOW, uniqueThreadMessageCount);

                for (int i = 0; i < maxMessages; i++) {
                    WapPushNotificationInfo info = mostRecentNotifPerThread.get(i);
                    inboxStyle.addLine(info.formatInboxMessage(context));
                }
                notification = inboxStyle.build();
            }
        }
        MmsLog.d(TAG, "updateNotification notify notification = " + notification);
        nm.notifyAsUser(TAG, NOTIFICATION_ID, notification, UserHandle.CURRENT);
        /** M: add for CTA 5.3.3 @{ */
        if (isNew) {
            //MessageUtils.handleNewNotification(context, messageCount);
        }
        /** @} */
    }

    protected static CharSequence buildTickerMessage(
            Context context, String address, String subject, String body) {
        String displayAddress = Contact.get(address, true).getName();
        return MessageUtils.formatMsgContent(subject, body, displayAddress);
    }


    //SL auto lanuch notification

    /**
     * Get the thread ID of the wappush message with the given URI
     * @param context The context
     * @param uri The URI of the wappush message
     * @return The thread ID, or THREAD_NONE if the URI contains no entries
     */
    public static long getWapPushThreadId(Context context, Uri uri) {
        Cursor cursor = SqliteWrapper.query(
            context,
            context.getContentResolver(),
            uri,
            WAPPUSH_THREAD_ID_PROJECTION,
            null,
            null,
            null);

        if (cursor == null) {
            MmsLog.w(TAG, "getWapPushThreadId cursor == null");
            return THREAD_NONE;
        }

        try {
            if (cursor.moveToFirst()) {
                long wappushThreadId = cursor.getLong(cursor.getColumnIndex(WapPush.THREAD_ID));
                MmsLog.d(TAG, "getWapPushThreadId wappushThreadId = " + wappushThreadId);
                return wappushThreadId;
            } else {
                return THREAD_NONE;
            }
        } finally {
            cursor.close();
        }
    }

    public static boolean notifySlAutoLanuchMessage(Context context, String url) {
        MmsLog.d(TAG, "notifySlAutoLanuchMessage");
        if (!MessagingNotification.isNotificationEnable(context)) {
            return false;
        }
        MessagingNotification.processNotificationSound(context);
        Notification.Builder builder = new Notification.Builder(context);
        builder.setChannelId(MessagingNotification.CHANNEL_ID);
        builder.setTicker(url);
        builder.setDefaults(Notification.FLAG_SHOW_LIGHTS);
        builder.setLights(0xff00ff00, 500, 2000);
        NotificationManager nm = (NotificationManager) context.getSystemService(
                Context.NOTIFICATION_SERVICE);
        //memslim nm.createNotificationChannel(MessagingNotification.mDefaultNotificationChannel);
        nm.notifyAsUser(TAG, SL_AUTOLAUNCH_NOTIFICATION_ID, builder.build(), UserHandle.CURRENT);
        return true;
    }
}
