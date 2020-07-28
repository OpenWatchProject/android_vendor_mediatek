/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
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

package com.android.mms.transaction;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.TaskStackBuilder;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.graphics.Bitmap;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.media.RingtoneManager;
import android.media.ToneGenerator;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.PowerManager;
import android.os.UserHandle;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.StyleSpan;
import android.text.style.TextAppearanceSpan;
import android.util.Log;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.mms.LogTag;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.data.Conversation;
import com.android.mms.data.WorkingMessage;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
/* P0 migration*/
//import com.android.mms.ui.BootActivity;
import com.android.mms.ui.ComposeMessageActivity;
import com.android.mms.ui.ConversationList;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.AddressUtils;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsLog;
import com.android.mms.widget.MmsWidgetProvider;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import com.google.android.mms.pdu.PduBody;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.android.mms.pdu.MtkRetrieveConf;
import com.mediatek.android.mms.pdu.MtkSendReq;
import com.mediatek.cb.cbmsg.CbMessagingNotification;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.ext.IOpMessagingNotificationExt;
import com.mediatek.mms.ipmessage.IIpMessagingNotificationExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.NotificationPreferenceActivity;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.MtkSms;

import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;
import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF;

/* P0 migration*/
import android.media.AudioAttributes;
import android.media.AudioAttributes.Builder;

/**
 * This class is used to update the notification indicator. It will check whether
 * there are unread messages. If yes, it would show the notification indicator,
 * otherwise, hide the indicator.
 */
public class MessagingNotification {

    private static final String TAG = LogTag.APP;
    private static final boolean DEBUG = false;

    public static final int NOTIFICATION_ID = 123;
    public static final int MESSAGE_FAILED_NOTIFICATION_ID = 789;
    public static final int DOWNLOAD_FAILED_NOTIFICATION_ID = 531;
    /**
     * This is the volume at which to play the in-conversation notification sound,
     * expressed as a fraction of the system notification volume.
     */
    private static final float IN_CONVERSATION_NOTIFICATION_VOLUME = 0.25f;

    // This must be consistent with the column constants below.
    private static final String[] MMS_STATUS_PROJECTION = new String[] {
        Mms.THREAD_ID, Mms.DATE, Mms._ID, Mms.SUBJECT, Mms.SUBJECT_CHARSET };

    /// M:Code analyze 001, add a column for msim @{
    // This must be consistent with the column constants below.
    private static final String[] SMS_STATUS_PROJECTION = new String[] {
        Sms.THREAD_ID, Sms.DATE, Sms.ADDRESS, Sms.SUBJECT, Sms.BODY, Sms._ID, MtkSms.IPMSG_ID
    };
    /// @}

    // These must be consistent with MMS_STATUS_PROJECTION and
    // SMS_STATUS_PROJECTION.
    private static final int COLUMN_THREAD_ID   = 0;
    private static final int COLUMN_DATE        = 1;
    private static final int COLUMN_MMS_ID      = 2;
    private static final int COLUMN_SMS_ADDRESS = 2;
    private static final int COLUMN_SUBJECT     = 3;
    private static final int COLUMN_SUBJECT_CS  = 4;
    private static final int COLUMN_SMS_BODY    = 4;

    private static final String[] SMS_THREAD_ID_PROJECTION = new String[] { Sms.THREAD_ID };
    private static final String[] MMS_THREAD_ID_PROJECTION = new String[] { Mms.THREAD_ID };

    private static final String NEW_INCOMING_SM_CONSTRAINT =
        "(" + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_INBOX
        + " AND " + Sms.SEEN + " = 0"
        + " AND " + Sms.READ + " = 0)";

    /// M:Code analyze 002, add two OR conditions[OUTBOX and PENDING] @{
    private static final String NEW_DELIVERY_SM_CONSTRAINT =
        "((" + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_SENT
        + " OR " + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_OUTBOX + ")"
        + " AND " + "(" + Sms.STATUS + " = " + Sms.STATUS_COMPLETE
        + " OR " + Sms.STATUS + " = " + MtkSms.STATUS_REPLACED_BY_SC
        + " OR " + Sms.STATUS + " = " + Sms.STATUS_PENDING + "))";
    /// @}

    private static final String NEW_INCOMING_MM_CONSTRAINT =
            "(" + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_INBOX
            + " AND " + Mms.SEEN + "=0"
            + " AND (" + Mms.MESSAGE_TYPE + "=" + MESSAGE_TYPE_NOTIFICATION_IND
            + " OR " + Mms.MESSAGE_TYPE + "=" + MESSAGE_TYPE_RETRIEVE_CONF + "))";

    private static final NotificationInfoComparator INFO_COMPARATOR =
            new NotificationInfoComparator();

    private static final Uri UNDELIVERED_URI = Uri.parse("content://mms-sms/undelivered");

    /// M: add for ip message
    private static final Uri THREAD_SETTINGS_URI = Uri.parse("content://mms-sms/thread_settings/");

    private final static String NOTIFICATION_DELETED_ACTION =
            "com.android.mms.NOTIFICATION_DELETED_ACTION";

    public static class OnDeletedReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                Log.d(TAG, "[MessagingNotification] clear notification: mark all msgs seen");
            }
            /// M:Code analyze 003, method parameter is changed @{
            Conversation.markAllConversationsAsSeen(context,
                    Conversation.MARK_ALL_MESSAGE_AS_SEEN_WITHOUT_WAPPUSH);
            /// @}
        }
    }

    public static final long THREAD_ALL = -1;
    public static final long THREAD_NONE = -2;
    /**
     * Keeps track of the thread ID of the conversation that's currently displayed to the user
     */
    private static long sCurrentlyDisplayedThreadId;
    private static final Object sCurrentlyDisplayedThreadLock = new Object();

    private static OnDeletedReceiver sNotificationDeletedReceiver = new OnDeletedReceiver();
    private static Intent sNotificationOnDeleteIntent;
    private static Handler sToastHandler = new Handler();
    private static MtkPduPersister sMtkPduPersister;
    private static final int MAX_BITMAP_DIMEN_DP = 360;
    private static float sScreenDensity;
    private static Context sContext;

    private static final int MAX_MESSAGES_TO_SHOW = 8;  // the maximum number of new messages to
                                                        // show in a single notification.

    /// M:Code analyze 004,add for class 0 of sms types,class 0 means the sms will not be save into
    /// phone or simCard,but only show on the phone @{
    public static final int CLASS_ZERO_NOTIFICATION_ID = 5566;
    /// @}

    /// M:Code analyze 005,add for getting value from corresponding column from database @{
    private static final int COLUMN_SMS_ID = 5;
    /// @}

    /// M:Code analyze 20,add a lock prevent multithreads accessing the same resources meantime @{
    private static final Object objectLock = new Object();
    /// @}

    /// M: For stop in conversation notification sound
    private static final Object sPlayingInConversationSoundLock = new Object();
    private static NotificationPlayer sNotificationPlayer = null;
    /// @}

    /// M: Add for RCSE, play ringtone only once when many messages comming
    private static long sLastNotificationTime = 0;
    private final static int RINGTONE_WAIT_TIME = 500;

    /// M: Add for DTMF tone
    private static final String DTMF_TONE = "SetWarningTone=16";

    /// M: add for checking bluetoothHandset;
    private static BluetoothHeadset sBluetoothHeadset;

    // ToneGenerator instance for mute mode
    private static ToneGenerator sMuteModeToneGenerator;
    /// M: The STREAM_VOICE_CALL tone volume
    private static final int TONE_FULL_VOLUME = 100;
    /** The length of STREAM_VOICE_CALL tones in milliseconds */
    private static final int MUTE_TONE_LENGTH_MS = 500;

    private static IIpMessagingNotificationExt mIpMessagingNotification;

    // add for op
    private static IOpMessagingNotificationExt sOpMessagingNotification;

    ///M: WFC @ {
    private static final int WFC_NOTIFICATION_ID = 625;
    /// @}

    //memslim public static NotificationChannel mDefaultNotificationChannel;
    //New message notification
    public static final String CHANNEL_ID = "mms_channel";
    public static final String CHANNEL_NAME = "mms_channel";
    //Mute new message notification
    public static final String MUTE_CHANNEL_ID = "mms_mute_channel";
    public static final String MUTE_CHANNEL_NAME = "mms_mute_channel";

    public static final int ID_CHAT_DETAILS_BY_THREAD_ID    = 7;
    public static final String IP_CHAT_DETAILS_BY_THREAD_ID = "content://chat_details_by_thread_id/"
            + Integer.toString(ID_CHAT_DETAILS_BY_THREAD_ID);

    private MessagingNotification() {
    }

    public static void init(Context context) {
        sContext = context;
        // set up the intent filter for notification deleted action
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(NOTIFICATION_DELETED_ACTION);

        // TODO: should we unregister when the app gets killed?
        context.registerReceiver(sNotificationDeletedReceiver, intentFilter);

        // initialize the notification deleted action
        sNotificationOnDeleteIntent = new Intent(NOTIFICATION_DELETED_ACTION);

        sScreenDensity = context.getResources().getDisplayMetrics().density;

        try {
            synchronized (sToneGeneratorLock) {
                if (sToneGenerator == null) {
                    sToneGenerator = new ToneGenerator(MMS_TONE_STREAM_TYPE, TONE_RELATIVE_VOLUME);
                }
            }
        } catch (RuntimeException e) {
            Log.e(TAG, "Exception caught while creating local tone generator: " + e);
            sToneGenerator = null;
        }

        try {
            synchronized (sToneGeneratorLock) {
                if (sMuteModeToneGenerator == null) {
                    sMuteModeToneGenerator = new ToneGenerator(AudioManager.STREAM_VOICE_CALL,
                            TONE_FULL_VOLUME);
                }
            }
        } catch (RuntimeException e) {
            Log.e(TAG, "Exception caught while creating mute tone generator: " + e);
            sMuteModeToneGenerator = null;
        }

        // add for ipmessage
        mIpMessagingNotification = IpMessageUtils.getIpMessagePlugin(context)
                .getIpMessagingNotification();
        mIpMessagingNotification.ipMessagingNotificationInit(context);
        sOpMessagingNotification = OpMessageUtils.getOpMessagePlugin()
                .getOpMessagingNotificationExt();

        NotificationManager notificationManager =
            (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        //memslim
        NotificationChannel defaultNotificationChannel = new NotificationChannel(
                CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_HIGH);
        defaultNotificationChannel.setVibrationPattern(new long[]{500, 500, 500, 500, 500});
        defaultNotificationChannel.enableLights(true);
        notificationManager.createNotificationChannel(defaultNotificationChannel);

        NotificationChannel muteNotificationChannel = new NotificationChannel(
                MUTE_CHANNEL_ID, MUTE_CHANNEL_NAME, NotificationManager.IMPORTANCE_LOW);
        notificationManager.createNotificationChannel(muteNotificationChannel);
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
        synchronized (sCurrentlyDisplayedThreadLock) {
            Log.e(TAG, "setCurrentlyDisplayedThreadId = " + threadId);
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
    public static void nonBlockingUpdateNewMessageIndicator(final Context context,
            final long newMsgThreadId,
            final boolean isStatusMessage) {
        /// M:
        Log.d(TAG, "nonBlockingUpdateNewMessageIndicator, newMsgThreadId = "
                + newMsgThreadId + "," +
                " isStatusMessage = " + isStatusMessage) ;
        new Thread(new Runnable() {
            @Override
            public void run() {
                blockingUpdateNewMessageIndicator(context, newMsgThreadId, isStatusMessage, null);
            }
        }, "MessagingNotification.nonBlockingUpdateNewMessageIndicator").start();
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
     * @param statusMessageUri Specify uri of statusMessage for showing delivery toast.
     */
    public static void blockingUpdateNewMessageIndicator(Context context, long newMsgThreadId,
            boolean isStatusMessage, Uri statusMessageUri) {

        boolean isContinuousComming = false;
        long currentTime = System.currentTimeMillis();
        if (newMsgThreadId != THREAD_NONE && currentTime >= sLastNotificationTime) {
            isContinuousComming = currentTime <= sLastNotificationTime + RINGTONE_WAIT_TIME;
            Log.d(TAG, "isContinuousComming = " + isContinuousComming +
                    " currentTime " + currentTime + "  " + sLastNotificationTime);
        }
        final boolean isDefaultSmsApp = MmsConfig.isSmsEnabled(context);
        if (!isDefaultSmsApp) {
            cancelNotification(context, NOTIFICATION_ID);
            if (DEBUG || Log.isLoggable(LogTag.APP, Log.VERBOSE))
                Log.d(TAG, "blockingUpdateNewMessageIndicator: not the default sms app - skipping "
                        + "notification");
            return;
        }

        synchronized (sCurrentlyDisplayedThreadLock) {
            Log.d(TAG, "newMsgThreadId = " + newMsgThreadId +
                    "sCurrentlyDisplayedThreadId = " + sCurrentlyDisplayedThreadId);
            if (newMsgThreadId > 0 && newMsgThreadId == sCurrentlyDisplayedThreadId) {
                if (DEBUG) {
                    Log.d(TAG, "blockingUpdateNewMessageIndicator: newMsgThreadId == " +
                            "sCurrentlyDisplayedThreadId so NOT showing notification," +
                            " but playing soft sound. threadId: " + newMsgThreadId);
                }
                if (!isContinuousComming) {
                    playInConversationNotificationSound(context);
                    sLastNotificationTime = currentTime;
                }
                return;
            }
        }

        SortedSet<NotificationInfo> notificationSet =
                new TreeSet<NotificationInfo>(INFO_COMPARATOR);

        Set<Long> threads = new HashSet<Long>(4);

        int count = 0;
        int mmsUnReadCount = addMmsNotificationInfos(context, threads, notificationSet);
        int smsUnReadCount = addSmsNotificationInfos(context, threads, notificationSet);
        smsUnReadCount += mIpMessagingNotification.blockingUpdateNewMessageIndicator(context,
                                threads, notificationSet, objectLock);


        if (notificationSet.isEmpty()) {
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                Log.d(TAG, "blockingUpdateNewMessageIndicator: notificationSet is empty, " +
                "canceling existing notifications");
            }
            cancelNotification(context, NOTIFICATION_ID);
        } else {
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                Log.d(TAG, "blockingUpdateNewMessageIndicator: count=" + count +
                        ", newMsgThreadId=" + newMsgThreadId);
            }
            updateNotification(context,
                    newMsgThreadId != THREAD_NONE, threads.size(), notificationSet,
                    newMsgThreadId, isContinuousComming, mmsUnReadCount, smsUnReadCount);
            sLastNotificationTime = currentTime;
        }

        // And deals with delivery reports (which use Toasts). It's safe to call in a worker
        // thread because the toast will eventually get posted to a handler.
        MmsSmsDeliveryInfo delivery = getSmsNewDeliveryInfo(context, statusMessageUri);
        if (delivery != null) {
            delivery.deliver(context, isStatusMessage);
        }

        notificationSet.clear();
        threads.clear();
    }

    /**
     * Play the in-conversation notification sound (it's the regular notification sound, but
     * played at half-volume
     * @param context the context to use
     */
    private static void playInConversationNotificationSound(Context context) {

        final AudioManager audioManager = (AudioManager) context
                .getSystemService(Context.AUDIO_SERVICE);
        final boolean silenced =
                audioManager.getRingerMode() != AudioManager.RINGER_MODE_NORMAL;
        Log.d(TAG, "playInConversationNotificationSound silenced" + silenced);
        if (silenced) {
            // Nothing to play
            return;
        }

        Uri ringtoneUri = getNewMsgNotifyChannelSound(context);
        Log.d(TAG, "playInConversationNotificationSound get ringtoneUri=" + ringtoneUri);

        /// M: Save NotificationPlayer instance so that we can stop it when composer is pausing
        synchronized (sPlayingInConversationSoundLock) {
            if (sNotificationPlayer == null) {
                sNotificationPlayer = new NotificationPlayer(LogTag.APP);
            }
            sNotificationPlayer.stop();
            sNotificationPlayer.play(context, ringtoneUri, false, AudioManager.STREAM_NOTIFICATION,
                    IN_CONVERSATION_NOTIFICATION_VOLUME);
        }
        /// @}
    }

    /**
     * Updates all pending notifications, clearing or updating them as
     * necessary.
     */
    public static void blockingUpdateAllNotifications(final Context context) {
        nonBlockingUpdateNewMessageIndicator(context, THREAD_NONE, false);
        nonBlockingUpdateSendFailedNotification(context);
        updateDownloadFailedNotification(context);
        /// M:Code analyze 006,Checks to see if there are any unread messages or delivery
        /// reports.Shows the most recent notification if there is one. @{
        CbMessagingNotification.updateNewMessageIndicator(context);
        /// @}
        MmsWidgetProvider.notifyDatasetChanged(context);
    }

    /**
     * Updates all pending notifications except send fail
     */
    public static void blockingUpdateAllNotificationsExceptFailed(final Context context) {
        nonBlockingUpdateNewMessageIndicator(context, THREAD_NONE, false);
        cancelSendFailedNotificationIfNeed(context);
        updateDownloadFailedNotification(context);
        /// M:Code analyze 006,Checks to see if there are any unread messages or delivery
        /// reports.Shows the most recent notification if there is one. @{
        CbMessagingNotification.updateNewMessageIndicator(context);
        /// @}
        MmsWidgetProvider.notifyDatasetChanged(context);
    }

    /**
     * Cancel send failed notification.
     */
    public static void cancelSendFailedNotificationIfNeed(final Context context) {
        new AsyncTask<Void, Void, Integer>() {
            protected Integer doInBackground(Void... none) {
                return getUndeliveredMessageCount(context, null);
            }

            protected void onPostExecute(Integer result) {
                if (result < 1) {
                    cancelNotification(context, MESSAGE_FAILED_NOTIFICATION_ID);
                }
            }
        }.execute();
    }

    private static final class MmsSmsDeliveryInfo {
        public CharSequence mTicker;
        public long mTimeMillis;

        public MmsSmsDeliveryInfo(CharSequence ticker, long timeMillis) {
            mTicker = ticker;
            mTimeMillis = timeMillis;
        }

        public void deliver(Context context, boolean isStatusMessage) {
            updateDeliveryNotification(
                    context, isStatusMessage, mTicker, mTimeMillis);
        }
    }

    /**
     * This class will be called in plugin, so change to public. If want to change this class,
     * please notify plugin team.
     *
     */
    public static final class NotificationInfo {
        public final Intent mClickIntent;
        public final String mMessage;
        public final CharSequence mTicker;
        public final long mTimeMillis;
        public final String mTitle;
        public final Bitmap mAttachmentBitmap;
        public final Contact mSender;
        public final boolean mIsSms;
        public final int mAttachmentType;
        public final String mSubject;
        public final long mThreadId;
        /// M:Code analyze 007,add for storing the uri of sms or mms @{
        public Uri mUri;
        /// @}

        /**
         * @param isSms true if sms, false if mms
         * @param clickIntent where to go when the user taps the notification
         * @param message for a single message, this is the message text
         * @param subject text of mms subject
         * @param ticker text displayed ticker-style across the notification, typically formatted
         * as sender: message
         * @param timeMillis date the message was received
         * @param title for a single message, this is the sender
         * @param attachmentBitmap a bitmap of an attachment, such as a picture or video
         * @param sender contact of the sender
         * @param attachmentType of the mms attachment
         * @param threadId thread this message belongs to
         */
        public NotificationInfo(boolean isSms,
                Intent clickIntent, String message, String subject,
                CharSequence ticker, long timeMillis, String title,
                Bitmap attachmentBitmap, Contact sender,
                int attachmentType, long threadId, Uri uri) {
            mIsSms = isSms;
            mClickIntent = clickIntent;
            mMessage = message;
            mSubject = subject;
            mTicker = ticker;
            mTimeMillis = timeMillis;
            mTitle = title;
            mAttachmentBitmap = attachmentBitmap;
            mSender = sender;
            mAttachmentType = attachmentType;
            mThreadId = threadId;
            /// M:Code analyze 007,add for storing the uri of sms or mms @{
            mUri = uri;
            /// @}
        }

        public long getTime() {
            return mTimeMillis;
        }

        // This is the message string used in bigText and bigPicture notifications.
        public CharSequence formatBigMessage(Context context) {
            final TextAppearanceSpan notificationSubjectSpan = new TextAppearanceSpan(
                    context, R.style.NotificationPrimaryText);

            // Change multiple newlines (with potential white space between),
            // into a single new line
            final String message =
                    !TextUtils.isEmpty(mMessage) ? mMessage.replaceAll("\\n\\s+", "\n") : "";

            SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
            if (!TextUtils.isEmpty(mSubject)) {
                spannableStringBuilder.append(mSubject);
                spannableStringBuilder.setSpan(notificationSubjectSpan, 0, mSubject.length(), 0);
            }
            if (mAttachmentType > WorkingMessage.TEXT) {
                if (spannableStringBuilder.length() > 0) {
                    spannableStringBuilder.append('\n');
                }
                spannableStringBuilder.append(getAttachmentTypeString(context, mAttachmentType));
            }
            if (mMessage != null) {
                if (spannableStringBuilder.length() > 0) {
                    spannableStringBuilder.append('\n');
                }
                spannableStringBuilder.append(IpMessageUtils.formatIpMessage(mMessage,
                        false, null));
            }
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
          String sender = mSender.getName();
          // add for ipmessage
          sender = mIpMessagingNotification.onIpFormatBigMessage(mSender.getNumber(), sender);

          if (!TextUtils.isEmpty(sender)) {
              spannableStringBuilder.append(sender);
              spannableStringBuilder.setSpan(notificationSenderSpan, 0, sender.length(), 0);
          }
          String separator = context.getString(R.string.notification_separator);
          if (!mIsSms) {
              if (!TextUtils.isEmpty(mSubject)) {
                  if (spannableStringBuilder.length() > 0) {
                      spannableStringBuilder.append(separator);
                  }
                  int start = spannableStringBuilder.length();
                  spannableStringBuilder.append(mSubject);
                  spannableStringBuilder.setSpan(notificationSubjectSpan, start,
                          start + mSubject.length(), 0);
              }
              if (mAttachmentType > WorkingMessage.TEXT) {
                  if (spannableStringBuilder.length() > 0) {
                      spannableStringBuilder.append(separator);
                  }
                  spannableStringBuilder.append(getAttachmentTypeString(context, mAttachmentType));
              }
          }
          if (message.length() > 0) {
              if (spannableStringBuilder.length() > 0) {
                  spannableStringBuilder.append(separator);
              }
              int start = spannableStringBuilder.length();
              spannableStringBuilder.append(IpMessageUtils.formatIpMessage(message, false, null));
              spannableStringBuilder.setSpan(notificationSubjectSpan, start,
                      start + message.length(), 0);
          }
          return spannableStringBuilder;
        }

        // This is the summary string used in bigPicture notifications.
        public CharSequence formatPictureMessage(Context context) {
            final TextAppearanceSpan notificationSubjectSpan = new TextAppearanceSpan(
                    context, R.style.NotificationPrimaryText);

            // Change multiple newlines(with potential white space between), into a single new line
            final String message =
                    !TextUtils.isEmpty(mMessage) ? mMessage.replaceAll("\\n\\s+", "\n") : "";

            // Show the subject or the message (if no subject)
            SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
            if (!TextUtils.isEmpty(mSubject)) {
                spannableStringBuilder.append(mSubject);
                spannableStringBuilder.setSpan(notificationSubjectSpan, 0, mSubject.length(), 0);
            }
            if (message.length() > 0 && spannableStringBuilder.length() == 0) {
                spannableStringBuilder.append(message);
                spannableStringBuilder.setSpan(notificationSubjectSpan, 0, message.length(), 0);
            }
            return spannableStringBuilder;
        }
    }

    // Return a formatted string with all the sender names separated by commas.
    private static CharSequence formatSenders(Context context,
            ArrayList<NotificationInfo> senders) {
        final TextAppearanceSpan notificationSenderSpan = new TextAppearanceSpan(
                context, R.style.NotificationPrimaryText);

        String separator = context.getString(R.string.enumeration_comma);   // ", "
        SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
        int len = senders.size();
        for (int i = 0; i < len; i++) {
            if (i > 0) {
                spannableStringBuilder.append(separator);
            }
//TODO I think new ip interface map the request
/*
            String sender = IpMessageUtils.getContactManager(context).getNameByNumber(
                    senders.get(i).mSender.getNumber());
            if (!sender.isEmpty()) {
                Log.d(TAG, "formatSenders ipmessage sender:" + sender);
            } else {
                sender = senders.get(i).mSender.getName();
                Log.d(TAG, "formatSenders ipmessage sender empty:" + sender);
            }
            spannableStringBuilder.append(sender);
*/
            spannableStringBuilder.append(senders.get(i).mSender.getName());
        }
        spannableStringBuilder.setSpan(notificationSenderSpan, 0,
                spannableStringBuilder.length(), 0);
        return spannableStringBuilder;
    }

    // Return a formatted string with the attachmentType spelled out as a string. For
    // no attachment (or just text), return null.
    private static CharSequence getAttachmentTypeString(Context context, int attachmentType) {
        final TextAppearanceSpan notificationAttachmentSpan = new TextAppearanceSpan(
                context, R.style.NotificationSecondaryText);
        int id = 0;
        switch (attachmentType) {
            case WorkingMessage.AUDIO: id = R.string.attachment_audio; break;
            case WorkingMessage.VIDEO: id = R.string.attachment_video; break;
            case WorkingMessage.SLIDESHOW: id = R.string.attachment_slideshow; break;
            case WorkingMessage.IMAGE: id = R.string.attachment_picture; break;
        }
        if (id > 0) {
            final SpannableString spannableString = new SpannableString(context.getString(id));
            spannableString.setSpan(notificationAttachmentSpan,
                    0, spannableString.length(), 0);
            return spannableString;
        }
        return null;
     }

    /**
     *
     * Sorts by the time a notification was received in descending order -- newer first.
     *
     */
    private static final class NotificationInfoComparator
            implements Comparator<NotificationInfo> {
        @Override
        public int compare(
                NotificationInfo info1, NotificationInfo info2) {
            return Long.signum(info2.getTime() - info1.getTime());
        }
    }

    private static final int addMmsNotificationInfos(
            Context context, Set<Long> threads, SortedSet<NotificationInfo> notificationSet) {
        ContentResolver resolver = context.getContentResolver();

        // This query looks like this when logged:
        // I/Database(  147): elapsedTime4Sql|/data/data/com.android.providers.telephony/databases/
        // mmssms.db|0.362 ms|SELECT thread_id, date, _id, sub, sub_cs FROM pdu WHERE ((msg_box=1
        // AND seen=0 AND (m_type=130 OR m_type=132))) ORDER BY date desc

        Cursor cursor = SqliteWrapper.query(context, resolver, Mms.CONTENT_URI,
                            MMS_STATUS_PROJECTION, NEW_INCOMING_MM_CONSTRAINT,
                            null, Mms.DATE + " desc");

        if (cursor == null) {
            return 0;
        }

        int addNotifyCount = 0;
        Set<Long> mmsThreads = new HashSet<Long>(4);
        try {
            while (cursor.moveToNext()) {

                long threadId = cursor.getLong(COLUMN_THREAD_ID);
                //for ALPS00812092 SMS received process is behind sent too much
                 /*
                 * our Notification show has 3 case:
                 * 1. 1 thread and 1 item => show the item
                 * 2. 1 thread and many items => show all items of this thread
                 * 3. many threads => show 1 item for every thread
                 * But total items can show out is <= MAX_MESSAGES_TO_SHOW
                 */
                if (mmsThreads.size() > MAX_MESSAGES_TO_SHOW) {
                    //has enough item of many thread, no need check cursor anymore
                    break;
                } else if (mmsThreads.contains(threadId) && mmsThreads.size() > 1) {
                    //threads count > 1, only need 1 item for every thread
                    continue;
                } else if (mmsThreads.contains(threadId)
                        && addNotifyCount > MAX_MESSAGES_TO_SHOW) {
                    //only 1 thread, but has enough items for this thread
                    continue;
                }

                long msgId = cursor.getLong(COLUMN_MMS_ID);
                Uri msgUri = Mms.CONTENT_URI.buildUpon().appendPath(
                        Long.toString(msgId)).build();
                String address = AddressUtils.getFrom(context, msgUri);

                Contact contact = Contact.get(address, false);
                if (contact.getSendToVoicemail()) {
                    // don't notify, skip this one
                    continue;
                }

                String subject = getMmsSubject(
                        cursor.getString(COLUMN_SUBJECT), cursor.getInt(COLUMN_SUBJECT_CS));
                /// M: google jb.mr1 patch
                subject = MessageUtils.cleanseMmsSubject(context, subject);

                long timeMillis = cursor.getLong(COLUMN_DATE) * 1000;

                if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                    MmsLog.dpi(TAG, "addMmsNotificationInfos: count=" + cursor.getCount() +
                            ", addr = " + address + ", thread_id=" + threadId);
                }

                // Extract the message and/or an attached picture from the first slide
                Bitmap attachedPicture = null;
                String messageBody = null;
                int attachmentType = WorkingMessage.TEXT;
                try {
                    sMtkPduPersister = MtkPduPersister.getPduPersister(sContext);
                    GenericPdu pdu = sMtkPduPersister.load(msgUri);
                    if (pdu != null && (pdu instanceof MtkSendReq
                            || pdu instanceof MtkRetrieveConf)) {
                        PduBody pb = null;
                        if (pdu instanceof MtkSendReq) {
                            pb = ((MtkSendReq)pdu).getBody();
                        } else if (pdu instanceof MtkRetrieveConf) {
                            pb = ((MtkRetrieveConf)pdu).getBody();
                        }
                        SlideshowModel slideshow = SlideshowModel.createFromPduBody(context, pb);
                        attachmentType = getAttachmentType(slideshow);
                        SlideModel firstSlide = slideshow.get(0);
                        if (firstSlide != null) {
                            if (cursor.getCount() <= 1) {
                                if (firstSlide.hasImage()) {
                                    int maxDim = dp2Pixels(MAX_BITMAP_DIMEN_DP);
                                    attachedPicture =
                                        firstSlide.getImage().getBitmap(maxDim, maxDim);
                                }
                            }
                            if (firstSlide.hasText()) {
                                messageBody = firstSlide.getText().getText();
                            }
                        }
                    }
                } catch (final MmsException e) {
                    Log.e(TAG, "MmsException loading uri: " + msgUri, e);
                    continue;   // skip this bad boy -- don't generate an empty notification
                }

                /// M:Code analyze 008,add a parameter msgUri for storing uri of message @{
                NotificationInfo info = getNewMessageNotificationInfo(context,
                        false /* isSms */,
                        address,
                        messageBody, subject,
                        threadId,
                        timeMillis,
                        attachedPicture,
                        contact,
                        attachmentType,
                        msgUri);
                /// @}
                /// M:Code analyze 20,add a lock prevent multithreads
                /// accessing the same resources meantime
                /// avoid function sNotificationSet.add accessing
                /// by many threads at the same time @{
                synchronized (objectLock) {
                    notificationSet.add(info);
                    addNotifyCount ++;
                }
                /// @}

                threads.add(threadId);
                mmsThreads.add(threadId);
            }
            return cursor.getCount();
        } finally {
            cursor.close();
        }
    }

    // Look at the passed in slideshow and determine what type of attachment it is.
    private static int getAttachmentType(SlideshowModel slideshow) {
        int slideCount = slideshow.size();

        if (slideCount == 0) {
            return WorkingMessage.TEXT;
        } else if (slideCount > 1) {
            return WorkingMessage.SLIDESHOW;
        } else {
            SlideModel slide = slideshow.get(0);
            if (slide.hasImage()) {
                return WorkingMessage.IMAGE;
            } else if (slide.hasVideo()) {
                return WorkingMessage.VIDEO;
            } else if (slide.hasAudio()) {
                return WorkingMessage.AUDIO;
            }
        }
        return WorkingMessage.TEXT;
    }

    private static final int dp2Pixels(int dip) {
        return (int) (dip * sScreenDensity + 0.5f);
    }

    private static final MmsSmsDeliveryInfo getSmsNewDeliveryInfo(
            Context context,
            Uri statusMessageUri) {
        // Using statusMessageUri can avoid showing wrong number in toast when
        // multi delivery report coming at the same time
        if (statusMessageUri == null) {
            statusMessageUri = Sms.CONTENT_URI;
        }
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = SqliteWrapper.query(context, resolver, statusMessageUri,
                    SMS_STATUS_PROJECTION, NEW_DELIVERY_SM_CONSTRAINT,
                    null, Sms.DATE);

        if (cursor == null) {
            return null;
        }

        try {
            if (!cursor.moveToLast()) {
                return null;
            }

            String address = cursor.getString(COLUMN_SMS_ADDRESS);
            long timeMillis = 3000;

            Contact contact = Contact.get(address, false);
            String name = contact.getNameAndNumber();

            return new MmsSmsDeliveryInfo(context.getString(R.string.delivery_toast_body, name),
                timeMillis);

        } catch (Exception e) {
            Log.d(TAG, "exception catched" + e);
            return null;
        } finally {
            cursor.close();
        }
    }

    private static final int addSmsNotificationInfos(
            Context context, Set<Long> threads, SortedSet<NotificationInfo> notificationSet) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = SqliteWrapper.query(context, resolver, Sms.CONTENT_URI,
                            SMS_STATUS_PROJECTION, NEW_INCOMING_SM_CONSTRAINT,
                            null, Sms.DATE + " desc");

        if (cursor == null) {
            return 0;
        }

        int addNotifyCount = 0;
        Set<Long> smsThreads = new HashSet<Long>(4);
        try {
            while (cursor.moveToNext()) {

                long threadId = cursor.getLong(COLUMN_THREAD_ID);
                //for ALPS00812092 SMS received process is behind sent too much
                 /*
                 * our Notification show has 3 case:
                 * 1. 1 thread and 1 item => show the item
                 * 2. 1 thread and many items => show all items of this thread
                 * 3. many threads => show 1 item for every thread
                 * But total items can show out is <= MAX_MESSAGES_TO_SHOW
                 */
                if (smsThreads.size() > MAX_MESSAGES_TO_SHOW) {
                    //has enough item of many thread, no need check cursor anymore
                    break;
                } else if (smsThreads.contains(threadId) && smsThreads.size() > 1) {
                    //threads count > 1, only need 1 item for every thread
                    continue;
                } else if (smsThreads.contains(threadId)
                        && addNotifyCount > MAX_MESSAGES_TO_SHOW) {
                    //only 1 thread, but has enough items for this thread
                    continue;
                }

                String address = cursor.getString(COLUMN_SMS_ADDRESS);

                Contact contact = Contact.get(address, false);
                if (contact.getSendToVoicemail()) {
                    // don't notify, skip this one
                    continue;
                }

                String message = cursor.getString(COLUMN_SMS_BODY);
                long timeMillis = cursor.getLong(COLUMN_DATE);
                int msgId = cursor.getInt(COLUMN_SMS_ID);
                boolean isSms = true;
                Bitmap attachmentBitmap = null;

                isSms = !mIpMessagingNotification.isIpAttachMessage(msgId, cursor);
                attachmentBitmap = mIpMessagingNotification.getIpBitmap(msgId, cursor);

                if (Log.isLoggable(LogTag.APP, Log.VERBOSE))
                {
                    MmsLog.dpi(TAG, "addSmsNotificationInfos: count=" + cursor.getCount() +
                            ", addr=" + address + ", thread_id=" + threadId);
                }

                /// M:Code analyze 009,add a parameter for storing uri of message @{
                NotificationInfo info = getNewMessageNotificationInfo(context, isSms /* isSms */,
                        address, message, null /* subject */,
                        threadId, timeMillis, attachmentBitmap /* attachmentBitmap */,
                        contact, WorkingMessage.TEXT,
                        Sms.CONTENT_URI.buildUpon()
                            .appendPath(Long.toString(cursor.getLong(COLUMN_SMS_ID))).build());
                /// @}
                /// M:Code analyze 20,add a lock prevent multithreads
                /// accessing the same resources meantime
                /// avoid function sNotificationSet.add accessing
                /// by many threads at the same time @{
                synchronized (objectLock) {
                    notificationSet.add(info);
                }
                addNotifyCount ++;
                /// @}

                threads.add(threadId);
                threads.add(cursor.getLong(COLUMN_THREAD_ID));
                smsThreads.add(threadId);
            }
            return cursor.getCount();
        } finally {
            cursor.close();
        }
    }

    /// M:Code analyze 010,add a parameter uri for storing uri of message.
    /// the function will be called in plugin, so change to public; when modify this function,
    /// please notify plugin team @{
    public static final NotificationInfo getNewMessageNotificationInfo(
            Context context,
            boolean isSms,
            String address,
            String message,
            String subject,
            long threadId,
            long timeMillis,
            Bitmap attachmentBitmap,
            Contact contact,
            int attachmentType,
            Uri uri) {
        /*MmsLog.dpi(TAG, "getNewMessageNotificationInfo" +
                "\n\t isSms \t = " + isSms +
                "\n\t address \t= " + address +
                "\n\t threadId \t= " + threadId +
                "\n\t uri \t = " + uri);*/

        Intent clickIntent = null;
        Conversation conv = Conversation.get(context, threadId, true);
        String number = null;
        if (conv != null && conv.getRecipients() != null && conv.getRecipients().size() == 1) {
            number = conv.getRecipients().get(0).getNumber();
        }

        if (mIpMessagingNotification.onIpgetNewMessageNotificationInfo(number, threadId)) {
            clickIntent = new Intent(IP_CHAT_DETAILS_BY_THREAD_ID);
            clickIntent.putExtra("thread_id", threadId);
            clickIntent.putExtra("need_new_task", false);
        } else {
            clickIntent = ComposeMessageActivity.createIntent(context, threadId);
            clickIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            clickIntent.putExtra("thread_id_from_notification", threadId);
            clickIntent.putExtra("from_notification", true);
        }
//        Intent clickIntent = ComposeMessageActivity.createIntent(context, threadId);
//        clickIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
//                | Intent.FLAG_ACTIVITY_SINGLE_TOP
//                | Intent.FLAG_ACTIVITY_CLEAR_TOP);

        String senderInfo = buildTickerMessage(
                context, address, null, null).toString();
        String senderInfoName = senderInfo.substring(
                0, senderInfo.length() - 2);
        CharSequence ticker = buildTickerMessage(
                context, address, subject, message);

        return new NotificationInfo(isSms,
                clickIntent, message, subject, ticker, timeMillis,
                senderInfoName, attachmentBitmap, contact, attachmentType, threadId, uri);
    }
    /// @}

    public static void cancelNotification(Context context, int notificationId) {
        /// M:
        Log.d(TAG, "cancelNotification, notificationId:" + notificationId);
        NotificationManager nm = (NotificationManager) context.getSystemService(
                Context.NOTIFICATION_SERVICE);

        nm.cancelAsUser(TAG, notificationId, UserHandle.CURRENT_OR_SELF);
        /// M: ALPS00436165, cancel NotificationManagerPlus shown in other AP @{
        /*
        if (notificationId == NOTIFICATION_ID) {
            EncapsulatedNotificationManagerPlus.cancel(context, 1);
        }
        */
        /// @}
    }

    private static void updateDeliveryNotification(final Context context,
                                                   boolean isStatusMessage,
                                                   final CharSequence message,
                                                   final long timeMillis) {
        if (!isStatusMessage) {
            return;
        }

        if (!isNotificationEnable(context)) {
            return;
        }

        sToastHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, message, (int) timeMillis).show();
            }
        });
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
            int uniqueThreadCount,
            SortedSet<NotificationInfo> notificationSet,
            long threadId,
            boolean isContinuousComming,
            int mmsUnReadCount,
            int smsUnReadCount) {
        Log.d(TAG, "isNew=" + isNew + "\tuniqueThreadCount="
                + uniqueThreadCount + "\tthreadId=" + threadId);

        if (!isNotificationEnable(context)) {
            Log.d(TAG, "updateNotification: notifications turned off in prefs, bailing");
            return;
        }

        // Figure out what we've got -- whether all sms's, mms's, or a mixture of both.
        NotificationInfo mostRecentNotification = null;
        int messageCount = 0;
        synchronized (objectLock) {
            messageCount = mmsUnReadCount + smsUnReadCount;
            /// M:Code analyze 011, check the notification count again. @{
            if (messageCount == 0) {
                MmsLog.w(TAG, "updateNotification.messageCount is 0.");
                return;
            }
            /// @}
            mostRecentNotification = notificationSet.first();
        }

        final Notification.Builder noti = new Notification.Builder(context)
                .setWhen(mostRecentNotification.mTimeMillis);

        if (isNew && !isContinuousComming) {
            noti.setTicker(mostRecentNotification.mTicker);
        }
        TaskStackBuilder taskStackBuilder = TaskStackBuilder.create(context);

        // If we have more than one unique thread, change the title (which would
        // normally be the contact who sent the message) to a generic one that
        // makes sense for multiple senders, and change the Intent to take the
        // user to the conversation list instead of the specific thread.

        // Cases:
        //   1) single message from single thread - intent goes to ComposeMessageActivity
        //   2) multiple messages from single thread - intent goes to ComposeMessageActivity
        //   3) messages from multiple threads - intent goes to ConversationList

        final Resources res = context.getResources();
        String title = null;
        Bitmap avatar = null;

        if (uniqueThreadCount > 1 || mostRecentNotification.mThreadId <= 0) {
            //Intent mainActivityIntent = new Intent(Intent.ACTION_MAIN);
            /* P0 migration*/
            //Intent mainActivityIntent = new Intent(context, BootActivity.class);
            Intent mainActivityIntent = new Intent(context, ConversationList.class);
            mainActivityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_SINGLE_TOP
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);

            mainActivityIntent.setType("vnd.android-dir/mms-sms");
            taskStackBuilder.addNextIntent(mainActivityIntent);
            title = context.getString(R.string.message_count_notification, messageCount);
        } else {    // same thread, single or multiple messages
            title = mostRecentNotification.mTitle;
            // add for ipmessage
            title = mIpMessagingNotification.getIpNotificationTitle(
                    mostRecentNotification.mSender.getNumber(), threadId, title);
            Drawable defaultContactImage = context.getResources()
                    .getDrawable(R.drawable.ic_default_contact);
            BitmapDrawable contactDrawable = (BitmapDrawable) mostRecentNotification.mSender
                    .getAvatar(context, defaultContactImage, threadId);
            //add for ipmessage for override the contactDrawable if needed @{
            contactDrawable = mIpMessagingNotification.getIpNotificationDrawable(context,
                    mostRecentNotification.mSender.getNumber(), threadId, contactDrawable);
            /// @}
            if (contactDrawable != null) {
                // Show the sender's avatar as the big icon. Contact bitmaps are 96x96 so we
                // have to scale 'em up to 128x128 to fill the whole notification large icon.
                avatar = contactDrawable.getBitmap();
                avatar = MessageUtils.getCircularBitmap(avatar);
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
                        noti.setLargeIcon(avatar);
                    }
                }
            }

            taskStackBuilder.addParentStack(ComposeMessageActivity.class);
            taskStackBuilder.addNextIntent(mostRecentNotification.mClickIntent);
        }

        noti.setContentIntent(taskStackBuilder.getPendingIntent(0,
                PendingIntent.FLAG_UPDATE_CURRENT, null, UserHandle.CURRENT_OR_SELF));

        // Always have to set the small icon or the notification is ignored
        noti.setSmallIcon(R.drawable.stat_notify_sms);
        // add for ipmessage
        mIpMessagingNotification.setIpSmallIcon(noti, mostRecentNotification.mSender.getNumber());

        NotificationManager nm = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);

        noti.setContentTitle(title)
                .setCategory(Notification.CATEGORY_MESSAGE)
                .setPriority(Notification.PRIORITY_DEFAULT);
                //.setChannelId(CHANNEL_ID);

        // Tag notification with all senders.
        for (NotificationInfo info : notificationSet) {
            Uri peopleReferenceUri = info.mSender.getPeopleReferenceUri();
            if (peopleReferenceUri != null) {
                noti.addPerson(peopleReferenceUri.toString());
            }
        }

        int defaults = 0;

        if (isNew && !isContinuousComming) {
            /// M:Code analyze 014, for brazil request, when calling still have sound @{
            int state = ((TelephonyManager) context.
                    getSystemService(Context.TELEPHONY_SERVICE)).getCallState();
            if (state != TelephonyManager.CALL_STATE_IDLE) {
                playTone(context, ToneGenerator.TONE_SUP_DIAL);
                noti.setChannelId(MUTE_CHANNEL_ID);
            } else {
                /* P0 migration*/
                //noti.setSound(TextUtils.isEmpty(ringtoneStr) ? null : Uri.parse(ringtoneStr));
                noti.setChannelId(CHANNEL_ID);
            }
            /// @}
            if (DEBUG) {
                Log.d(TAG, "updateNotification: new message, adding sound to the notification");
            }
        } else {
            noti.setChannelId(MUTE_CHANNEL_ID);
        }

        defaults |= Notification.DEFAULT_LIGHTS;

        noti.setDefaults(defaults);

        // set up delete intent
        noti.setDeleteIntent(PendingIntent.getBroadcast(context, 0,
                sNotificationOnDeleteIntent, 0));

        final Notification notification;

        if (messageCount == 1) {
            // We've got a single message

            // This sets the text for the collapsed form:
            noti.setContentText(mostRecentNotification.formatBigMessage(context));

            if (mostRecentNotification.mAttachmentBitmap != null) {
                // The message has a picture, show that

                notification = new Notification.BigPictureStyle(noti)
                    .bigPicture(mostRecentNotification.mAttachmentBitmap)
                    // This sets the text for the expanded picture form:
                    .setSummaryText(mostRecentNotification.formatPictureMessage(context))
                    .build();
            } else {
                // Show a single notification -- big style with the text of the whole message
                notification = new Notification.BigTextStyle(noti)
                    .bigText(mostRecentNotification.formatBigMessage(context))
                    .build();
            }
        } else {
            // We've got multiple messages
            if (uniqueThreadCount == 1) {
                // We've got multiple messages for the same thread.
                // Starting with the oldest new message, display the full text of each message.
                // Begin a line for each subsequent message.
                SpannableStringBuilder buf = new SpannableStringBuilder();
                NotificationInfo infos[] =
                        notificationSet.toArray(new NotificationInfo[notificationSet.size()]);
                int len = infos.length;
                for (int i = len - 1; i >= 0; i--) {
                    NotificationInfo info = infos[i];

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
                /// M:Code analyze 20,add a lock prevent multithreads
                /// accessing the same resources meantime,
                /// because the paragraph below is in multithreads branch,
                /// more likely to be accessed by more
                /// than one thread at the same time @{
                HashSet<Long> uniqueThreads = new HashSet<Long>(notificationSet.size());
                ArrayList<NotificationInfo> mostRecentNotifPerThread =
                        new ArrayList<NotificationInfo>();
                Iterator<NotificationInfo> notifications = notificationSet.iterator();
                while (notifications.hasNext()) {
                    NotificationInfo notificationInfo = notifications.next();
                    if (!uniqueThreads.contains(notificationInfo.mThreadId)) {
                        uniqueThreads.add(notificationInfo.mThreadId);
                        mostRecentNotifPerThread.add(notificationInfo);
                    }
                }
                /// @}
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
                    NotificationInfo info = mostRecentNotifPerThread.get(i);
                    inboxStyle.addLine(info.formatInboxMessage(context));
                }
                notification = inboxStyle.build();

                uniqueThreads.clear();
                mostRecentNotifPerThread.clear();

                if (DEBUG) {
                    Log.d(TAG, "updateNotification: multi messages," +
                            " showing inboxStyle notification");
                }
            }
        }
        nm.notifyAsUser(TAG, NOTIFICATION_ID, notification, UserHandle.CURRENT);
        // add for OP
        sOpMessagingNotification.onUpdateNotification(isNew);
    }

    protected static CharSequence buildTickerMessage(
            Context context, String address, String subject, String body) {
        String displayAddress = null;
        displayAddress = Contact.get(address, true).getName();
        // add for ipmessage
        displayAddress = mIpMessagingNotification.ipBuildTickerMessage(address, displayAddress);

        StringBuilder buf = new StringBuilder(
                displayAddress == null
                ? ""
                : "\u202a" + displayAddress.replace('\n', ' ').replace('\r', ' ') + "\u202a");
        buf.append(':').append(' ');

        int offset = buf.length();
        if (!TextUtils.isEmpty(subject)) {
            subject = subject.replace('\n', ' ').replace('\r', ' ');
            buf.append(subject);
            buf.append(' ');
        }

        if (!TextUtils.isEmpty(body)) {
            body = body.replace('\n', ' ').replace('\r', ' ');
            buf.append(body);

            // add for ipmessage
            buf = new StringBuilder(IpMessageUtils.formatIpMessage(body, false, buf));
        }

        SpannableString spanText = new SpannableString(buf.toString());
        spanText.setSpan(new StyleSpan(Typeface.BOLD), 0, offset,
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        return spanText;
    }

    private static String getMmsSubject(String sub, int charset) {
        return TextUtils.isEmpty(sub) ? ""
                : new EncodedStringValue(charset, MtkPduPersister.getBytes(sub)).getString();
    }

    public static void notifyDownloadFailed(Context context, long threadId) {
        /// M:
        Log.d(TAG, "notifyDownloadFailed");
        notifyFailed(context, true, threadId, false);
    }

    public static void notifySendFailed(Context context) {
        /// M:
        Log.d(TAG, "notifySendFailed");
        notifyFailed(context, false, 0, false);
    }

    public static void notifySendFailed(Context context, boolean noisy) {
        /// M:
        Log.d(TAG, "notifySendFailed, noisy = " + noisy);
        notifyFailed(context, false, 0, noisy);
    }

    private static void notifyFailed(Context context, boolean isDownload, long threadId,
                                     boolean noisy) {
        if (!isNotificationEnable(context)) {
            return;
        }

        // Strategy:
        // a. If there is a single failure notification, tapping on the notification goes
        //    to the compose view.
        // b. If there are two failure it stays in the thread view. Selecting one undelivered
        //    thread will dismiss one undelivered notification but will still display the
        //    notification.If you select the 2nd undelivered one it will dismiss the notification.

        long[] msgThreadId = {0, 1};    // Dummy initial values, just to initialize the memory
        int totalFailedCount = getUndeliveredMessageCount(context, msgThreadId);
        Log.d(TAG, "notifySendFailed, threadId = " + threadId
                + " totalFailedCount = " + totalFailedCount
                + " isDownload = " + isDownload);
        if (totalFailedCount == 0 && !isDownload) {
            return;
        }
        // The getUndeliveredMessageCount method puts a non-zero value in msgThreadId[1] if all
        // failures are from the same thread.
        // If isDownload is true, we're dealing with 1 specific failure; therefore "all failed" are
        // indeed in the same thread since there's only 1.
        boolean allFailedInSameThread = (msgThreadId[1] != 0) || isDownload;
        Log.d(TAG, "notifySendFailed, allFailedInSameThread = " + allFailedInSameThread);
        Intent failedIntent;
        PendingIntent pIntent = null;
        Notification.Builder builder = new Notification.Builder(context);
        String title;
        String description;
        if (totalFailedCount > 1) {
            if (isDownload) {
                title = context.getString(R.string.message_download_failed_title);
                description = context.getString(R.string.message_failed_body);
            } else {
                description = context.getString(R.string.notification_failed_multiple,
                        Integer.toString(totalFailedCount));
                title = context.getString(R.string.notification_failed_multiple_title);
            }
        } else {
            title = isDownload ?
                        context.getString(R.string.message_download_failed_title) :
                        context.getString(R.string.message_send_failed_title);

            description = context.getString(R.string.message_failed_body);
        }

        TaskStackBuilder taskStackBuilder = TaskStackBuilder.create(context);
        if (allFailedInSameThread) {
            failedIntent = new Intent(context, ComposeMessageActivity.class);
            if (isDownload) {
                // When isDownload is true, the valid threadId is passed into this function.
                failedIntent.putExtra("failed_download_flag", true);
            } else {
                threadId = msgThreadId[0];
                Log.d(TAG, "notifySendFailed, threadId 1 = " + threadId);
                failedIntent.putExtra("undelivered_flag", true);
            }
            failedIntent.putExtra("thread_id", threadId);
            taskStackBuilder.addParentStack(ComposeMessageActivity.class);
        } else {
            failedIntent = new Intent(context, ConversationList.class);
        }

        taskStackBuilder.addNextIntent(failedIntent);

        builder.setSmallIcon(R.drawable.stat_notify_sms_failed);
        builder.setTicker(title);

        builder.setContentTitle(title);
        builder.setContentText(description);
        builder.setContentIntent(
                taskStackBuilder.getPendingIntent(0, PendingIntent.FLAG_UPDATE_CURRENT));

        if (noisy) {
            processNotificationSound(context);
            builder.setChannelId(CHANNEL_ID);
            /// @}
        } else {
            builder.setChannelId(MUTE_CHANNEL_ID);
        }

        NotificationManager notificationMgr = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);

        if (isDownload) {
            notificationMgr.notifyAsUser(TAG, DOWNLOAD_FAILED_NOTIFICATION_ID,
                builder.build(), UserHandle.CURRENT);
        } else {
            notificationMgr.notifyAsUser(TAG, MESSAGE_FAILED_NOTIFICATION_ID,
                builder.build(), UserHandle.CURRENT);
        }
    }

    /**
     * Query the DB and return the number of undelivered messages (total for both SMS and MMS)
     * @param context The context
     * @param threadIdResult A container to put the result in, according to the following rules:
     *  threadIdResult[0] contains the thread id of the first message.
     *  threadIdResult[1] is nonzero if the thread ids of all the messages are the same.
     *  You can pass in null for threadIdResult.
     *  You can pass in a threadIdResult of size 1 to avoid the comparison of each thread id.
     */
    private static int getUndeliveredMessageCount(Context context, long[] threadIdResult) {
        Log.d(TAG, "getUndeliveredMessageCount");
        /// M:Code analyze 018,add for cmcc dir mode @{
        String where = "read=0";

        /// M: ALPS00837193, query undelivered mms with non-permanent fail ones or not @{
        Uri queryUri = sOpMessagingNotification.getUndeliveredMessageCount(UNDELIVERED_URI);
        Cursor undeliveredCursor = SqliteWrapper.query(context, context.getContentResolver(),
                queryUri, MMS_THREAD_ID_PROJECTION, where, null, null);
        /// @}
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
        //add for ipmessage
        count += mIpMessagingNotification.getUndeliveredMessageCount(context, threadIdResult);
        return count;
    }

    public static void nonBlockingUpdateSendFailedNotification(final Context context) {
        Log.d(TAG, "getDownloadFailedMessageCount");

        new AsyncTask<Void, Void, Integer>() {
            protected Integer doInBackground(Void... none) {
                return getUndeliveredMessageCount(context, null);
            }

            protected void onPostExecute(Integer result) {
                if (result < 1) {
                    cancelNotification(context, MESSAGE_FAILED_NOTIFICATION_ID);
                } else {
                    // rebuild and adjust the message count if necessary.
                    notifySendFailed(context);
                }
            }
        }.execute();
    }


    private static int getDownloadFailedMessageCount(Context context, long[] threadIdResult) {
        // Look for any messages in the MMS Inbox that are of the type
        // NOTIFICATION_IND (i.e. not already downloaded) and in the
        // permanent failure state.  If there are none, cancel any
        // failed download notification.
        Log.d(TAG, "getDownloadFailedMessageCount");
        Cursor c = SqliteWrapper.query(context, context.getContentResolver(),
                Mms.Inbox.CONTENT_URI, MMS_THREAD_ID_PROJECTION,
                Mms.MESSAGE_TYPE + "=" +
                    String.valueOf(MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) +
                " AND " + Mms.STATUS + "=" +
                    String.valueOf(DownloadManager.STATE_PERMANENT_FAILURE) +
                " AND " + Mms.READ + "=0",
                null, null);
        if (c == null) {
            Log.d(TAG, "getDownloadFailedMessageCount = 0");
            return 0;
        }

        int count = c.getCount();
        try {
            if (threadIdResult != null && c.moveToFirst()) {
                threadIdResult[0] = c.getLong(0);

                if (threadIdResult.length >= 2) {
                    // Test to see if all the download failed messages belong to the same thread.
                    long firstId = threadIdResult[0];
                    while (c.moveToNext()) {
                        if (c.getLong(0) != firstId) {
                            firstId = 0;
                            break;
                        }
                    }
                    threadIdResult[1] = firstId;    // non-zero if all ids are the same
                }
            }
        } finally {
            c.close();
        }
        Log.d(TAG, "getDownloadFailedMessageCount = " + count);
        return count;
    }

    public static void updateDownloadFailedNotification(Context context) {
        long[] msgThreadId = {0, 0};
        if (getDownloadFailedMessageCount(context, msgThreadId) < 1) {
            cancelNotification(context, DOWNLOAD_FAILED_NOTIFICATION_ID);
        }
    }

    /**
     *  If all the download failed messages belong to "threadId", cancel the notification.
     */
    public static void updateSendFailedNotificationForThread(Context context, long threadId) {
        long[] msgThreadId = {0, 0};
        if (getUndeliveredMessageCount(context, msgThreadId) > 0
                && msgThreadId[0] == threadId
                && msgThreadId[1] != 0) {
            Log.d(TAG, "updateSendFailedNotificationForThread threadId = " + threadId);
            cancelNotification(context, MESSAGE_FAILED_NOTIFICATION_ID);
        }
    }

    public static void updateDownloadFailedNotificationForThread(Context context, long threadId) {
        long[] msgThreadId = {0, 0};
        if (getDownloadFailedMessageCount(context, msgThreadId) > 0
                && msgThreadId[0] == threadId
                && msgThreadId[1] != 0) {
            Log.d(TAG, "updateDownloadFailedNotificationForThread threadId = " + threadId);
            cancelNotification(context, DOWNLOAD_FAILED_NOTIFICATION_ID);
        }
    }

    public static boolean isFailedToDeliver(Intent intent) {
        return (intent != null) && intent.getBooleanExtra("undelivered_flag", false);
    }

    public static boolean isFailedToDownload(Intent intent) {
        return (intent != null) && intent.getBooleanExtra("failed_download_flag", false);
    }

    /**
     * Get the thread ID of the SMS message with the given URI
     * @param context The context
     * @param uri The URI of the SMS message
     * @return The thread ID, or THREAD_NONE if the URI contains no entries
     */
    public static long getSmsThreadId(Context context, Uri uri) {
        Cursor cursor = SqliteWrapper.query(
            context,
            context.getContentResolver(),
            uri,
            SMS_THREAD_ID_PROJECTION,
            null,
            null,
            null);

        if (cursor == null) {
            return THREAD_NONE;
        }

        try {
            if (cursor.moveToFirst()) {
                int columnIndex = cursor.getColumnIndex(Sms.THREAD_ID);
                if (columnIndex < 0) {
                    if (DEBUG) {
                        MmsLog.dpi(TAG, "getSmsThreadId uri: " + uri +
                                " Couldn't read row 0, col -1! returning THREAD_NONE");
                    }
                    return THREAD_NONE;
                }
                long threadId = cursor.getLong(columnIndex);
                return threadId;
            } else {
                return THREAD_NONE;
            }
        } finally {
            cursor.close();
        }
    }

    /**
     * Get the thread ID of the MMS message with the given URI
     * @param context The context
     * @param uri The URI of the SMS message
     * @return The thread ID, or THREAD_NONE if the URI contains no entries
     */
    public static long getThreadId(Context context, Uri uri) {
        Cursor cursor = SqliteWrapper.query(
                context,
                context.getContentResolver(),
                uri,
                MMS_THREAD_ID_PROJECTION,
                null,
                null,
                null);

        if (cursor == null) {
            return THREAD_NONE;
        }

        try {
            if (cursor.moveToFirst()) {
                int columnIndex = cursor.getColumnIndex(Mms.THREAD_ID);
                if (columnIndex < 0) {
                    if (DEBUG) {
                        Log.d(TAG, "getThreadId uri: " + uri +
                                " Couldn't read row 0, col -1! returning THREAD_NONE");
                    }
                    return THREAD_NONE;
                }
                long threadId = cursor.getLong(columnIndex);
                return threadId;
            } else {
                return THREAD_NONE;
            }
        } finally {
            cursor.close();
        }
    }

    /// M: the new methods
    /// M:Code analyze 004,add for class 0 of sms types,class 0 means the sms will not be save into
    /// phone or simCard,but only show on the phone @{
    public static boolean notifyClassZeroMessage(Context context, String address) {
        Log.d(TAG, "notifyClassZeroMessage");
        if (!isNotificationEnable(context)) {
            return false;
        }

        NotificationManager nm = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);

        Notification.Builder builder = new Notification.Builder(context)
                .setSmallIcon(R.drawable.stat_notify_sms);
        processNotificationSound(context);
        builder.setTicker(address);
        int defaults = 0;
        defaults |= Notification.FLAG_SHOW_LIGHTS;
        defaults |= Notification.FLAG_AUTO_CANCEL;
        builder.setDefaults(defaults);
        builder.setLights(0xff00ff00, 500, 2000);
        /* P0 migration*/
        builder.setChannelId(CHANNEL_ID);
        nm.notifyAsUser(TAG, CLASS_ZERO_NOTIFICATION_ID, builder.build(), UserHandle.CURRENT);
        return true;
    }
    /// @}


    /// M:Code analyze 019,new method add for setting the ringtone of notification @{
    public static void processNotificationSound(Context context) {
        int state = ((TelephonyManager) context.getSystemService(
                Context.TELEPHONY_SERVICE)).getCallState();
        if (state != TelephonyManager.CALL_STATE_IDLE) {
            /* in call or in ringing */
            playTone(context, ToneGenerator.TONE_SUP_DIAL);
        }
    }
    /// @}

    /** The length of DTMF tones in milliseconds */
    private static final int TONE_LENGTH_MS = 150;

    /** The DTMF tone volume relative to other sounds in the stream */
    private static final int TONE_RELATIVE_VOLUME = 80;

    /** Stream type used to play the DTMF tones off call, and mapped to the volume control keys */
    private static final int MMS_TONE_STREAM_TYPE = AudioManager.STREAM_DTMF;

    private static ToneGenerator sToneGenerator;
    private static final Object sToneGeneratorLock = new Object();

    /**
     * Plays the specified tone for TONE_LENGTH_MS milliseconds. The tone is
     * played locally, using the audio stream for phone calls. Tones are played
     * only if the "Audible touch tones" user preference is checked, and are NOT
     * played if the device is in silent mode.
     *
     * @param tone a tone code from {@link ToneGenerator}
     */
    private static void playTone(Context context, int tone) {

        // Also do nothing if the phone is in silent mode.
        // We need to re-check the ringer mode for *every* playTone()
        // call, rather than keeping a local flag that's updated in
        // onResume(), since it's possible to toggle silent mode without
        // leaving the current activity (via the ENDCALL-longpress menu.)
        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        int ringerMode = audioManager.getRingerMode();
        if ((ringerMode == AudioManager.RINGER_MODE_SILENT)
                || (ringerMode == AudioManager.RINGER_MODE_VIBRATE)) {
            return;
        }

        if (sToneGenerator == null) {
            Log.w(TAG, "playTone: sToneGenerator == null, tone: " + tone);
            return;
        }
        // Start the new tone (will stop any playing tone)
        sToneGenerator.startTone(tone, TONE_LENGTH_MS);

        if (FeatureOption.MTK_C2K_SUPPORT) {
            /// M: Play DTMF tone
            /// Only for CDMA talk, for GSM talk case ,this will not become effective.
            audioManager.setParameters(DTMF_TONE);
        }

    }

    public static boolean isNotificationEnable(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return  prefs.getBoolean(NotificationPreferenceActivity.NOTIFICATION_ENABLED, true);
    }

    /// M: Composer stop notification sound if it is playing
    public static void stopInConversationNotificationSound() {
        synchronized (sPlayingInConversationSoundLock) {
            if (sNotificationPlayer == null) {
                return;
            }
            sNotificationPlayer.stop();
            sNotificationPlayer = null;
        }
    }
    /// @}

    /**
     * M: Wake up screen
     * @param context
     */
    private static void wakeUpScreen(Context context) {
        if (context == null) {
            return;
        }
        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        boolean hasInsertedHeadSet = audioManager.isWiredHeadsetOn();
        boolean headsetIsOn = isBluetoothHandsetOn(context);
        MmsLog.d(TAG, "hasInsertedHeadSet:" + hasInsertedHeadSet + "\tHeadsetIsOn:" + headsetIsOn);
        if (hasInsertedHeadSet || headsetIsOn) {
            PowerManager powerManager =
                (PowerManager) (context.getSystemService(Context.POWER_SERVICE));
            PowerManager.WakeLock wakeLock = null;
            wakeLock = powerManager.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP
                    | PowerManager.ON_AFTER_RELEASE, "MMS_wake_lock");
            long wakeUpTime = 0;
        try {
                ContentResolver cr = context.getContentResolver();
                wakeUpTime = android.provider.Settings.System.getInt(cr,
                        Settings.System.SCREEN_OFF_TIMEOUT);
            } catch (SettingNotFoundException e) {
                MmsLog.e(TAG, "Exception occured in wakeupScreen()");
            }
            wakeLock.acquire(wakeUpTime);

            /// M: Play tone when mute or vibrate mode
            if (isMuteOrVibrate(context)) {
                MmsLog.d(TAG, "Mute and Vibrate mode...");
                if (sMuteModeToneGenerator != null) {
                    sMuteModeToneGenerator.startTone(ToneGenerator.TONE_SUP_DIAL,
                            MUTE_TONE_LENGTH_MS);
                }
            }
        }
    }

    /**
     * M: Check the bluetoothHandset whether has been connected or not.
     * @param context
     * @return
     */
    private static boolean isBluetoothHandsetOn(Context context) {
        ///M: get default bluetoothAdapter
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        ///M: bluetoothprofile service listener
        BluetoothProfile.ServiceListener profileListener = new BluetoothProfile.ServiceListener() {
            public void onServiceConnected(int profile, BluetoothProfile proxy) {
                if (profile == BluetoothProfile.HEADSET) {
                    sBluetoothHeadset = (BluetoothHeadset) proxy;
                }
            }
            public void onServiceDisconnected(int profile) {
                if (profile == BluetoothProfile.HEADSET) {
                    sBluetoothHeadset = null;
                }
            }
        };
        // Establish connection to the proxy
        bluetoothAdapter.getProfileProxy(context, profileListener, BluetoothProfile.HEADSET);
        boolean headsetIsOn = false;
        if (sBluetoothHeadset != null) {
            List<BluetoothDevice> devicess = sBluetoothHeadset.getConnectedDevices();
            if (devicess != null && devicess.size() > 0) {
                MmsLog.d(TAG, "headset device's number:" + devicess.size());
                for (BluetoothDevice device : devicess) {
                    int connectState = sBluetoothHeadset.getConnectionState(device);
                    if (connectState == BluetoothHeadset.STATE_DISCONNECTED
                            || connectState == BluetoothHeadset.STATE_DISCONNECTING) {
                        headsetIsOn = false;
                    } else {
                        headsetIsOn = true;
                        break;
                    }
                }
            } else {
                MmsLog.d(TAG, "headset device's number:0");
                headsetIsOn = false;
            }
        } else {
            headsetIsOn = false;
        }
        try {
            // Close proxy connection after use.
            bluetoothAdapter.closeProfileProxy(BluetoothProfile.HEADSET, sBluetoothHeadset);
        } catch (IllegalArgumentException e) {
            MmsLog.e(TAG, e.getMessage(), e);
        }
        return headsetIsOn;
    }

    private static boolean isMuteOrVibrate(Context context) {
        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        MmsLog.d(TAG, "Audio manager ringer mode = " + audioManager.getRingerMode()
                + " stream volume = " + audioManager.getStreamVolume(AudioManager.STREAM_RING));
        return AudioManager.RINGER_MODE_SILENT == audioManager.getRingerMode()
                || AudioManager.RINGER_MODE_VIBRATE == audioManager.getRingerMode()
                || 0 == audioManager.getStreamVolume(AudioManager.STREAM_RING);
    }

/*
    ///M: WFC: Show Tricker  @ {
    public static void showWfcNotification(Context context) {
        final Context cntxt = context;
//         Show a notification on this sceen in certain conditions
        if (doShowWfcPopup(SubscriptionManager.INVALID_SUBSCRIPTION_ID, context)) {
            Notification noti = new Notification.Builder(context)
                .setContentTitle(context.getResources().getString(R.string.wfc_notification_title))
                .setSmallIcon(com.mediatek.internal.R.drawable.wfc_notify_registration_success)
                .setTicker(context.getResources().getString(R.string.wfc_notification_title))
                .setOngoing(true)
                .build();
                NotificationManager mNotifMgr = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
                mNotifMgr.notify(WFC_NOTIFICATION_ID, noti);
        }
    }
*/
    public static void stopWfcNotification(Context context) {
        NotificationManager mNotifMgr =
            (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        MmsLog.d(TAG, "stopNotification:" + WFC_NOTIFICATION_ID);
        //memslim mNotifMgr.createNotificationChannel(mDefaultNotificationChannel);
        mNotifMgr.cancel(WFC_NOTIFICATION_ID);
    }

    /* Checks whether any of RAT present: 2G/3G/LTE/Wi-Fi */
    public static boolean doShowWfcPopup(int selectedSubId, Context context) {
        // Return if WFC is disabled by user or SIM is not present
        if (!ImsManager.isWfcEnabledByUser(context) || !MessageUtils.isSimPresent(context)) {
            MmsLog.d(TAG, "Wfc disabled by user or SIM not present");
            return false;
        }
        // show pop up here
        if (!MessageUtils.isWifiConnected(context)) {
            if (ImsManager.getWfcMode(context)
                    == ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY) {
                MmsLog.d(TAG, "Wifi-only & Wifi off or not connected:show pop-up");
                return true;
            }
            if (MessageUtils.getCellularState(selectedSubId, context) !=
                                        ServiceState.STATE_IN_SERVICE) {
                MmsLog.d(TAG, "No RAT present:show pop-up");
                return true;
            }
        }
        MmsLog.d(TAG, "some RAT present: do not show pop-up");
        return false;
    }
    /// @}
	/* P0 migration*/
    public static String getNewMsgNotifyChannelId() {
        Log.d(TAG, "getNewMsgNotifyChannelId = " + CHANNEL_ID);
        return CHANNEL_ID;
    }

    public static Uri getNewMsgNotifyChannelSound(Context context) {
        NotificationManager nm = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);
        NotificationChannel channel = nm.getNotificationChannel(CHANNEL_ID);
        if (channel == null) {
            Log.e(TAG, "getNewMsgNotifyChannelSound, channel null");
            return null;
        }

        Uri channelRingtoneUri = channel.getSound();
        Log.d(TAG, "channelRingtoneUri return " + channelRingtoneUri);
        return channelRingtoneUri;
    }
}
