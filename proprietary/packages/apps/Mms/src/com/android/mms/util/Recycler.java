/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.android.mms.util;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.provider.BaseColumns;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Conversations;
import android.provider.Telephony.Threads;
import android.util.Log;

import com.android.mms.MmsConfig;
import com.android.mms.ui.MessageUtils;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.setting.GeneralPreferenceActivity;

import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.WapPush;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
//import android.provider.Telephony.WapPush;

/**
 * The recycler is responsible for deleting old messages.
 */
public abstract class Recycler {
    private static final boolean LOCAL_DEBUG = false;
    private static final String TAG = "Recycler";

    // Default preference values
    private static final boolean DEFAULT_AUTO_DELETE  = false;

    private static SmsRecycler sSmsRecycler;
    private static MmsRecycler sMmsRecycler;
    private static WapPushRecycler sWapPushRecycler;

    private static boolean sAutoDeleteRun = false;

    protected static final Executor sExecutor = Executors.newFixedThreadPool(1);

    public static SmsRecycler getSmsRecycler() {
        if (sSmsRecycler == null) {
            sSmsRecycler = new SmsRecycler();
        }
        return sSmsRecycler;
    }

    public static MmsRecycler getMmsRecycler() {
        if (sMmsRecycler == null) {
            sMmsRecycler = new MmsRecycler();
        }
        return sMmsRecycler;
    }

    public static boolean checkForThreadsOverLimit(Context context) {
        Recycler smsRecycler = getSmsRecycler();
        Recycler mmsRecycler = getMmsRecycler();
        /// M:
        Recycler wappushRecycler = getWapPushRecycler();
        /// M: add wappush
        return smsRecycler.anyThreadOverLimit(context) ||
               mmsRecycler.anyThreadOverLimit(context) ||
               wappushRecycler.anyThreadOverLimit(context);
    }

    public void deleteOldMessages(Context context) {
        if (!isAutoDeleteEnabled(context)) {
            return;
        }

        //don't enter this when it already running
        if (!sAutoDeleteRun) {
            Cursor cursor = getAllThreads(context);
            try {
                final int limit = getMessageLimit(context);
                while (cursor.moveToNext()) {
                    final long threadId = getThreadId(cursor);
                    /// M: @{
                    sAutoDeleteRun = true;
                    /// @}
                    sExecutor.execute(new Runnable() {
                        @Override
                        public void run() {
                            deleteMessagesForThread(context, threadId, limit);
                        }
                    });
                }
            } finally {
                /// M: @{
                sAutoDeleteRun = false;
                /// @}
                cursor.close();
            }
        }
    }

    public void deleteOldMessagesByThreadId(Context context, final long threadId) {
        if (LOCAL_DEBUG) {
            Log.v(TAG, "Recycler.deleteOldMessagesByThreadId this: " + this +
                    " threadId: " + threadId);
        }
        if (!isAutoDeleteEnabled(context)) {
            return;
        }
        sExecutor.execute(new Runnable() {
            @Override
            public void run() {
                deleteMessagesForThread(context, threadId, getMessageLimit(context));
            }
        });
    }

    public static boolean isAutoDeleteEnabled(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return prefs.getBoolean(GeneralPreferenceActivity.AUTO_DELETE,
                DEFAULT_AUTO_DELETE);
    }

    abstract public int getMessageLimit(Context context);

    abstract public void setMessageLimit(Context context, int limit);

    public int getMessageMinLimit() {
        return MmsConfig.getMinMessageCountPerThread();
    }

    public int getMessageMaxLimit() {
        return MmsConfig.getMaxMessageCountPerThread();
    }

    abstract protected long getThreadId(Cursor cursor);

    abstract protected Cursor getAllThreads(Context context);

    abstract protected void deleteMessagesForThread(Context context, long threadId, int keep);

    abstract protected boolean anyThreadOverLimit(Context context);

    public static class SmsRecycler extends Recycler {

        static private final String[] SMS_MESSAGE_PROJECTION = new String[] {
            BaseColumns._ID,
            Sms.DATE,
        };

        // The indexes of the default columns which must be consistent
        // with above PROJECTION.
        static private final int COLUMN_ID                  = 0;
        static private final int COLUMN_SMS_DATE            = 1;

        private final String MAX_SMS_MESSAGES_PER_THREAD = "MaxSmsMessagesPerThread";

        public int getMessageLimit(Context context) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
            return prefs.getInt(MAX_SMS_MESSAGES_PER_THREAD,
                    MmsConfig.getDefaultSMSMessagesPerThread());
        }

        public void setMessageLimit(Context context, int limit) {
            SharedPreferences.Editor editPrefs =
                PreferenceManager.getDefaultSharedPreferences(context).edit();
            editPrefs.putInt(MAX_SMS_MESSAGES_PER_THREAD, limit);
            editPrefs.apply();
        }

        protected long getThreadId(Cursor cursor) {
            return cursor.getLong(0);
        }

        protected Cursor getAllThreads(Context context) {
            ContentResolver resolver = context.getContentResolver();
            /// M: change google default
            Cursor cursor = SqliteWrapper.query(context, resolver,
                    Uri.parse("content://sms/all_threadid"),
                    null, null, null, Conversations.DEFAULT_SORT_ORDER);

            return cursor;
        }

        protected void deleteMessagesForThread(Context context, long threadId, int keep) {
            if (LOCAL_DEBUG) {
                Log.v(TAG, "SMS: deleteMessagesForThread, threadId = " + threadId);
            }
            ContentResolver resolver = context.getContentResolver();
            Cursor cursor = null;

            String notDraft = Sms.TYPE + "<>" + Sms.MESSAGE_TYPE_DRAFT;
            String notIpMessage = MtkTelephony.MtkSms.IPMSG_ID + "<=0";
            String notLocked = Sms.LOCKED + "=0";
            String where = notDraft + " AND " + notLocked + " AND " + notIpMessage;

            try {
                cursor = SqliteWrapper.query(context, resolver,
                        ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, threadId),
                        SMS_MESSAGE_PROJECTION,
                        where,
                        null, "date ASC");     // get in oldest to newest order
                if (cursor == null || cursor.getCount() == 0) {
                    Log.w(TAG, "SMS: deleteMessagesForThread got back null cursor");
                    return;
                }
                int count = cursor.getCount();
                int numberToDelete = count - keep;
                if (LOCAL_DEBUG) {
                    MmsLog.vpi(TAG, "SMS: deleteMessagesForThread count: " + count +
                            " numberToDelete: " + numberToDelete);
                }
                if (numberToDelete <= 0) {
                    return;
                }
                // Move to the keep limit and then delete everything older than that one.
                cursor.moveToPosition(numberToDelete);
                long latestDate = cursor.getLong(COLUMN_SMS_DATE);
                //get all the sms id which will be deleted.
                String[] argsSms = new String[numberToDelete];
                cursor.moveToFirst();
                for (int i = 0; i < numberToDelete; i++) {
                    argsSms[i] = Integer.toString(cursor.getInt(COLUMN_ID));
                    cursor.moveToNext();
                }
                cursor.close();
                int delete = SqliteWrapper.delete(context, resolver,
                        ContentUris.withAppendedId(Uri.parse("content://sms/auto_delete"),
                                threadId),
                        "date<" + latestDate + " AND " + where,
                        argsSms);
                Log.v(TAG, "SMS: deleteMessagesForThread delete: " + delete);
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }

        @Override
        protected boolean anyThreadOverLimit(Context context) {
            Cursor cursor = getAllThreads(context);
            if (cursor == null) {
                return false;
            }
            int limit = getMessageLimit(context);
            try {
                while (cursor.moveToNext()) {
                    long threadId = getThreadId(cursor);
                    ContentResolver resolver = context.getContentResolver();
                    Cursor msgs = SqliteWrapper.query(context, resolver,
                            ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, threadId),
                            SMS_MESSAGE_PROJECTION,
                            "locked=0 AND " + MtkTelephony.MtkSms.IPMSG_ID
                            + "<=0 AND " + Sms.TYPE + "<>" + Sms.MESSAGE_TYPE_DRAFT,
                            null, "date DESC");     // get in newest to oldest order
                    if (msgs == null) {
                        return false;
                    }
                    try {
                        if (msgs.getCount() >= limit) {
                            return true;
                        }
                    } finally {
                        msgs.close();
                    }
                }
            } finally {
                cursor.close();
            }
            return false;
        }
    }

    public static class MmsRecycler extends Recycler {
        private static final String[] ALL_MMS_THREADS_PROJECTION = {
            "thread_id", "count(*) as msg_count"
        };

        private static final int ID             = 0;
        private static final int MESSAGE_COUNT  = 1;

        static private final String[] MMS_MESSAGE_PROJECTION = new String[] {
            BaseColumns._ID,
            Conversations.THREAD_ID,
            Mms.DATE,
        };

        // The indexes of the default columns which must be consistent
        // with above PROJECTION.
        static private final int COLUMN_ID                  = 0;
        static private final int COLUMN_THREAD_ID           = 1;
        static private final int COLUMN_MMS_DATE            = 2;

        private final String MAX_MMS_MESSAGES_PER_THREAD = "MaxMmsMessagesPerThread";

        private static final String MMS_CONVERSATION_CONSTRAINT = "(" +
                Mms.MESSAGE_BOX + " != " + Mms.MESSAGE_BOX_DRAFTS + " AND (" +
                Mms.MESSAGE_TYPE + " = " + MtkPduHeaders.MESSAGE_TYPE_SEND_REQ + " OR " +
                Mms.MESSAGE_TYPE + " = " + MtkPduHeaders.MESSAGE_TYPE_RETRIEVE_CONF + " OR " +
                Mms.MESSAGE_TYPE + " = " + MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND + "))";

        public int getMessageLimit(Context context) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
            return prefs.getInt(MAX_MMS_MESSAGES_PER_THREAD,
                    MmsConfig.getDefaultMMSMessagesPerThread());
        }

        public void setMessageLimit(Context context, int limit) {
            SharedPreferences.Editor editPrefs =
                PreferenceManager.getDefaultSharedPreferences(context).edit();
            editPrefs.putInt(MAX_MMS_MESSAGES_PER_THREAD, limit);
            editPrefs.apply();
        }

        protected long getThreadId(Cursor cursor) {
            return cursor.getLong(ID);
        }

        protected Cursor getAllThreads(Context context) {
            ContentResolver resolver = context.getContentResolver();
            Cursor cursor = SqliteWrapper.query(context, resolver,
                    Uri.withAppendedPath(Telephony.Mms.CONTENT_URI, "threads"),
                    ALL_MMS_THREADS_PROJECTION, null, null, Conversations.DEFAULT_SORT_ORDER);

            return cursor;
        }

        public void deleteOldMessagesInSameThreadAsMessage(Context context, Uri uri) {
            if (LOCAL_DEBUG) {
                Log.v(TAG, "MMS: deleteOldMessagesByUri");
            }
            if (!isAutoDeleteEnabled(context)) {
                return;
            }
            Cursor cursor = null;
            try {
                String msgId = uri.getLastPathSegment();
                ContentResolver resolver = context.getContentResolver();
                cursor = SqliteWrapper.query(context, resolver,
                        Telephony.Mms.CONTENT_URI,
                        MMS_MESSAGE_PROJECTION,
                        "thread_id in (select thread_id from pdu where _id=" + msgId +
                            ") AND locked=0" + " AND " + MMS_CONVERSATION_CONSTRAINT,
                        null, "date DESC");     // get in newest to oldest order
                if (cursor == null) {
                    Log.e(TAG, "MMS: deleteOldMessagesInSameThreadAsMessage got back null cursor");
                    return;
                }

                int count = cursor.getCount();
                int keep = getMessageLimit(context);
                final int numberToDelete = count - keep;
                if (LOCAL_DEBUG) {
                    MmsLog.vpi(TAG, "MMS: deleteOldMessagesByUri keep: " + keep +
                            " count: " + count +
                            " numberToDelete: " + numberToDelete);
                }
                if (numberToDelete <= 0) {
                    return;
                }
                // Move to the keep limit and then delete everything older than that one.
                cursor.move(keep);
                final long latestDate = cursor.getLong(COLUMN_MMS_DATE);
                final long threadId = cursor.getLong(COLUMN_THREAD_ID);

                if (threadId != 0) {
                    sExecutor.execute(new Runnable() {
                        @Override
                        public void run() {
                            deleteMessagesOlderThanDate(context, threadId, latestDate,
                                    numberToDelete);
                        }
                    });
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }

        protected void deleteMessagesForThread(Context context, long threadId, int keep) {
            if (LOCAL_DEBUG) {
                Log.v(TAG, "MMS: deleteMessagesForThread");
            }
            if (threadId == 0) {
                return;
            }
            Cursor cursor = null;
            long latestDate = 0;
            /// M: @{
            int numberToDelete = 0;
            /// @}
            try {
                ContentResolver resolver = context.getContentResolver();
                cursor = SqliteWrapper.query(context, resolver,
                        Telephony.Mms.CONTENT_URI,
                        MMS_MESSAGE_PROJECTION,
                        "thread_id=" + threadId + " AND locked=0"
                        + " AND " + MMS_CONVERSATION_CONSTRAINT,
                        null, "date ASC");     // get in newest to oldest order
                /// M: change google default.
                if (cursor == null || cursor.getCount() == 0) {
                    Log.e(TAG, "MMS: deleteMessagesForThread got back null cursor");
                    return;
                }

                int count = cursor.getCount();
                numberToDelete = count - keep;
                if (LOCAL_DEBUG) {
                    Log.v(TAG, "MMS: deleteMessagesForThread keep: " + keep +
                            " count: " + count +
                            " numberToDelete: " + numberToDelete);
                }
                if (numberToDelete <= 0) {
                    return;
                }
                // Move to the keep limit and then delete everything older than that one.
                cursor.moveToPosition(numberToDelete);
                latestDate = cursor.getLong(COLUMN_MMS_DATE);
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
            /// M: change google default.
            deleteMessagesOlderThanDate(context, threadId, latestDate, numberToDelete);
        }

        private void deleteMessagesOlderThanDate(Context context, long threadId,
                long latestDate, int numberToDelete) {
            long cntDeleted = SqliteWrapper.delete(context, context.getContentResolver(),
                    Telephony.Mms.CONTENT_URI,
                    "thread_id=" + threadId + " AND locked=0 AND date<" + latestDate,
                    null);
            /// M: @{
            if (cntDeleted != numberToDelete) {
                Cursor cursor = null;
                try {
                    cursor = SqliteWrapper.query(context, context.getContentResolver(),
                            Telephony.Mms.CONTENT_URI,
                            MMS_MESSAGE_PROJECTION,
                            "thread_id=" + threadId + " AND locked=0",
                            null, "_id ASC");     // get in newest to oldest order
                    numberToDelete = numberToDelete - (int) cntDeleted;
                    if (LOCAL_DEBUG) {
                        Log.v(TAG, "MMS: numberToDelete: " + numberToDelete
                                + "cursor count " + cursor.getCount());
                    }
                    if (cursor.getCount() == 0 || numberToDelete <= 0) {
                        return;
                    }

                    long delId = 0;
                    if (cursor.moveToPosition(numberToDelete)) {
                        delId = cursor.getLong(COLUMN_ID);
                    }
                    cntDeleted = SqliteWrapper.delete(context, context.getContentResolver(),
                            Telephony.Mms.CONTENT_URI,
                            "locked=0 AND _id<" + delId, null);
                    if (LOCAL_DEBUG) {
                        Log.v(TAG, "MMS: deleteMessagesOlderThanDate cntDeleted: " + cntDeleted);
                    }
                } finally {
                    if (cursor != null) {
                        cursor.close();
                    }
                }
            }
            /// @}
        }

        @Override
        protected boolean anyThreadOverLimit(Context context) {
            Cursor cursor = getAllThreads(context);
            if (cursor == null) {
                return false;
            }
            int limit = getMessageLimit(context);
            try {
                while (cursor.moveToNext()) {
                    long threadId = getThreadId(cursor);
                    ContentResolver resolver = context.getContentResolver();
                    Cursor msgs = SqliteWrapper.query(context, resolver,
                            Telephony.Mms.CONTENT_URI,
                            MMS_MESSAGE_PROJECTION,
                            "thread_id=" + threadId + " AND locked=0",
                            null, "date DESC");     // get in newest to oldest order

                    if (msgs == null) {
                        return false;
                    }
                    try {
                        if (msgs.getCount() >= limit) {
                            return true;
                        }
                    } finally {
                        msgs.close();
                    }
                }
            } finally {
                cursor.close();
            }
            return false;
        }
    }

    /// M:
    public static class WapPushRecycler extends Recycler {
        private static final String[] ALL_WAPPUSH_THREADS_PROJECTION = {
            Threads._ID,
            Threads.MESSAGE_COUNT
        };

        private static final int ID             = 0;
        private static final int MESSAGE_COUNT  = 1;

        static private final String[] WAPPUSH_MESSAGE_PROJECTION = new String[] {
            WapPush._ID,
            WapPush.DATE,
        };

        // The indexes of the default columns which must be consistent
        // with above PROJECTION.
        private static final int COLUMN_ID                  = 0;
        private static final int COLUMN_WAPPUSH_DATE        = 1;

        /// M: wappush limit use the same settings as sms.
        public int getMessageLimit(Context context) {
            return getSmsRecycler().getMessageLimit(context);
        }

        public void setMessageLimit(Context context, int limit) {
            getSmsRecycler().setMessageLimit(context, limit);
        }

        protected long getThreadId(Cursor cursor) {
            return cursor.getLong(ID);
        }

        protected Cursor getAllThreads(Context context) {
            Uri.Builder uriBuilder = Threads.CONTENT_URI.buildUpon();
            uriBuilder.appendQueryParameter("simple", "true");
            uriBuilder.appendQueryParameter("thread_type",
                    String.valueOf(MtkTelephony.MtkThreads.WAPPUSH_THREAD));

            ContentResolver resolver = context.getContentResolver();
            Cursor cursor = SqliteWrapper.query(context, resolver,
                    uriBuilder.build(),
                    ALL_WAPPUSH_THREADS_PROJECTION,
                    null,
                    null, WapPush.DEFAULT_SORT_ORDER);
            return cursor;
        }

        protected void deleteMessagesForThread(Context context, long threadId, int keep) {
            if (LOCAL_DEBUG) {
                Log.v(TAG, "WAPPUSH: deleteMessagesForThread");
            }
            ContentResolver resolver = context.getContentResolver();
            Cursor cursor = null;
            try {
                cursor = SqliteWrapper.query(context, resolver,
                        ContentUris.withAppendedId(WapPush.CONTENT_URI_THREAD, threadId),
                        WAPPUSH_MESSAGE_PROJECTION,
                        "locked=0",
                        null, "date ASC");     // get in oldest to newest order
                if (cursor == null || cursor.getCount() == 0) {
                    Log.e(TAG, "WAPPUSH: deleteMessagesForThread got back null cursor");
                    return;
                }
                int count = cursor.getCount();
                int numberToDelete = count - keep;
                if (LOCAL_DEBUG) {
                    Log.v(TAG, "WAPPUSH: deleteMessagesForThread keep: " + keep +
                            " count: " + count +
                            " numberToDelete: " + numberToDelete);
                }
                if (numberToDelete <= 0) {
                    return;
                }
                // Move to the keep limit and then delete everything older than that one.
                cursor.moveToPosition(numberToDelete);
                long latestDate = cursor.getLong(COLUMN_WAPPUSH_DATE);
                long delId = cursor.getLong(COLUMN_ID);

                long cntDeleted = SqliteWrapper.delete(context, resolver,
                        ContentUris.withAppendedId(WapPush.CONTENT_URI_THREAD, threadId),
                        "locked=0 AND date<" + latestDate,
                        null);
                if (LOCAL_DEBUG) {
                    Log.v(TAG, "WAPPUSH: deleteMessagesForThread cntDeleted: " + cntDeleted);
                }

                if (cntDeleted != numberToDelete) {
                    cntDeleted = SqliteWrapper.delete(context, resolver,
                            ContentUris.withAppendedId(WapPush.CONTENT_URI_THREAD, threadId),
                            "locked=0 AND _id<" + delId,
                            null);
                }
                if (LOCAL_DEBUG) {
                    Log.v(TAG, "WAPPUSH: deleteMessagesForThread cntDeleted: " + cntDeleted);
                }


            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }

        @Override
        protected boolean anyThreadOverLimit(Context context) {
            Cursor cursor = getAllThreads(context);
            Cursor msgs = null;
            int limit = getMessageLimit(context);
            if (cursor != null) {
                try {
                    long threadId = 0L;
                    while (cursor.moveToNext()) {
                        threadId = getThreadId(cursor);
                        ContentResolver resolver = context.getContentResolver();
                        msgs = SqliteWrapper.query(context, resolver,
                                ContentUris.withAppendedId(WapPush.CONTENT_URI_THREAD, threadId),
                                WAPPUSH_MESSAGE_PROJECTION,
                                "locked=0",
                                null, "date DESC");     // get in newest to oldest order
                        if (msgs != null && msgs.getCount() >= limit) {
                            return true;
                        }
                    }
                } finally {
                    cursor.close();
                    if (msgs != null) {
                        msgs.close();
                    }
                }
            }
            return false;
        }
    }

    public static WapPushRecycler getWapPushRecycler() {
        if (sWapPushRecycler == null) {
            sWapPushRecycler = new WapPushRecycler();
        }
        return sWapPushRecycler;
    }
}
