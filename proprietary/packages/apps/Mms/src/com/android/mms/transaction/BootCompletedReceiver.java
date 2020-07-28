/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
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
package com.android.mms.transaction;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDiskIOException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Handler;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Outbox;
import android.telephony.SmsManager;
import android.util.Log;

import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.MmsLog;
import com.android.mms.widget.MmsWidgetProvider;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.util.PermissionCheckUtil;


/**
 * BootCompletedReceiver receives the
 * {@link android.content.intent.ACTION_BOOT_COMPLETED}
 * and performs a series of operations which may include:
 * <ul>
 * <li>Show/hide the icon in notification area which is used to indicate
 * whether there is new incoming message. or send/download failed message</li>
 * <li>Resend the MM's in the outbox.</li>
 * </ul>
 */
public class BootCompletedReceiver extends BroadcastReceiver {
    private static final String TAG = "BootCompletedReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        MmsLog.i(TAG, "onReceive action: " + action);

        if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
            MmsLog.d(TAG, "BootCompletedReceiver, no permissions return !!");
            return;
        }

        if (action != null && action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            final Context mContext = context;
            new Thread(new Runnable() {
                public void run() {
                    // add for ipmessage
                    IpMessageUtils.onIpBootCompleted(context.getApplicationContext());

                    int pendingMms = setPendingMmsFailed(mContext);
                    int pendingSms = setPendingSmsFailed(mContext);
                    if ((pendingMms + pendingSms) > 0) {
                        MessagingNotification.notifySendFailed(
                                context.getApplicationContext(), true);
                    }

                    setNotificationIndUnstarted(mContext);

                    MessagingNotification.blockingUpdateNewMessageIndicator(
                           mContext, MessagingNotification.THREAD_ALL, false, null);

                   // update widget
                   MmsWidgetProvider.notifyDatasetChanged(context.getApplicationContext());
                }
            }).start();
        }
    }

    /// add for setting the pending mms failed,
    /// mainly using after boot complete @{
    private int setPendingMmsFailed(Context context) {
        int count = 0;
        Cursor cursor = MtkPduPersister.getPduPersister(context).getPendingMessages(
                Long.MAX_VALUE/*System.currentTimeMillis()*/);
        if (cursor != null) {
            try {
                count = cursor.getCount();
                MmsLog.d(TAG, "setPendingMmsFailed: count = " + count);
                if (count == 0) {
                    return 0;
                }

                ContentValues values = null;
                int columnIndex = 0;
                int columnType = 0;
                int id = 0;
                int type = 0;
                while (cursor.moveToNext()) {
                    columnIndex = cursor.getColumnIndexOrThrow(PendingMessages._ID);
                    id = cursor.getInt(columnIndex);

                    columnType = cursor.getColumnIndexOrThrow(PendingMessages.MSG_TYPE);
                    type = cursor.getInt(columnType);

                    MmsLog.d(TAG, "setPendingMmsFailed: id=" + id + ", type=" + type);

                    if (type == MtkPduHeaders.MESSAGE_TYPE_SEND_REQ
                            || type == MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
                        values = new ContentValues(2);
                        values.put(PendingMessages.ERROR_TYPE,  MmsSms.ERR_TYPE_GENERIC_PERMANENT);
                        values.put(PendingMessages.RETRY_INDEX, 3 /*scheme.getRetryLimit()*/);
                        SqliteWrapper.update(context,
                                context.getContentResolver(),
                                PendingMessages.CONTENT_URI,
                                values, PendingMessages._ID + "=" + id, null);

                        columnIndex = cursor.getColumnIndexOrThrow(PendingMessages.MSG_ID);
                        int msgId = cursor.getInt(columnIndex);
                        Uri msgUri = Uri.withAppendedPath(Mms.CONTENT_URI, Integer.toString(msgId));
                        ContentValues readValues = new ContentValues(1);
                        readValues.put(Mms.READ, 0);
                        SqliteWrapper.update(context, context.getContentResolver(), msgUri,
                                readValues, null, null);
                    }
                }
            } catch (SQLiteDiskIOException e) {
                MmsLog.e(TAG,
                        "SQLiteDiskIOException caught while set pending message failed", e);
            } finally {
                cursor.close();
            }
        } else {
            MmsLog.d(TAG, "setPendingMmsFailed: no pending MMS.");
        }
        return count;
    }
    /// @}

    /// add for setting the mms being downloading when shutdown to unrecognized
    /// after boot complete again,have to manual download @{
    public void setNotificationIndUnstarted(final Context context) {
        MmsLog.d(TAG, "setNotificationIndUnstarted");
        Cursor cursor = SqliteWrapper.query(context,
                context.getContentResolver(),
                Mms.CONTENT_URI,
                new String[] {Mms._ID, Mms.STATUS, Mms.THREAD_ID},
                Mms.MESSAGE_TYPE + "=" + MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND,
                null, null);
        if (cursor != null) {
            try {
                int count = cursor.getCount();
                MmsLog.d(TAG, "setNotificationIndUnstarted: Message Size=" + count);

                if (count == 0) {
                    return;
                }

                ContentValues values = null;
                int id = 0;
                int status = 0;
                while (cursor.moveToNext()) {
                    id = cursor.getInt(0);
                    status = cursor.getInt(1);
                    MmsLog.d(TAG,
                            "setNotificationIndUnstarted: MsgId=" + id + "; status=" + status);

                    if (DownloadManager.STATE_DOWNLOADING
                            == (status & ~DownloadManager.DEFERRED_MASK)) {
                        values = new ContentValues(1);
                        values.put(Mms.STATUS, DownloadManager.STATE_PERMANENT_FAILURE);
                        values.put(Mms.READ, 0);
                        SqliteWrapper.update(context,
                                context.getContentResolver(),
                                Mms.CONTENT_URI,
                                values, Mms._ID + "=" + id, null);
                        MessagingNotification.notifyDownloadFailed(context, cursor.getInt(2));
                    }
                }
            } catch (SQLiteDiskIOException e) {
                MmsLog.e(TAG,
                        "SQLiteDiskIOException caught while set notification ind unstart", e);
            } finally {
                cursor.close();
            }
        } else {
            MmsLog.d(TAG, "setNotificationIndUnstarted: no pending messages.");
        }
    }
    /// @}

    private int setPendingSmsFailed(Context context) {
        int count = 0;
        StringBuilder mStringBuilder = new StringBuilder();
        String where = "(" + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_QUEUED +
                        " OR " + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_OUTBOX + ")";

        Cursor cursor = SqliteWrapper.query(
                context,
                context.getContentResolver(),
                Sms.CONTENT_URI,
                new String[] { Sms._ID },
                where,
                null,
                null);
        if (cursor != null) {
            try {
                count = cursor.getCount();
                MmsLog.i(TAG, "setPendingSmsFailed, count = " + count);
                if (count == 0) {
                    return 0;
                }

                mStringBuilder.append(Sms._ID + " IN(");
                while (cursor.moveToNext()) {
                    long id = cursor.getLong(0);
                    mStringBuilder.append(" " + id);
                    if (!cursor.isLast()) {
                        mStringBuilder.append(',');
                    }
                }
                mStringBuilder.append(')');
            } finally {
                cursor.close();
            }
        }

        ContentValues values = new ContentValues(3);
        values.put(Sms.TYPE, Sms.MESSAGE_TYPE_FAILED);
        values.put(Sms.ERROR_CODE, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        values.put(Sms.READ, Integer.valueOf(0));
        SqliteWrapper.update(
                context, context.getContentResolver(),
                Sms.CONTENT_URI, values, mStringBuilder.toString(), null);
        return count;
    }
}
