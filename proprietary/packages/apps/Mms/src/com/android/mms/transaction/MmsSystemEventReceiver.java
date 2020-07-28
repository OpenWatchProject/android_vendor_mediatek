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
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteDiskIOException;
import android.database.sqlite.SqliteWrapper;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Handler;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.TempFileProvider;
import com.android.mms.ui.ComposeMessageActivity;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.MmsLog;
import com.android.mms.widget.MmsWidgetProvider;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.appserviceproxy.AppServiceProxy;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;

//mem slim

import android.app.Notification;
import android.app.NotificationManager;

import com.android.mms.R;
import com.android.mms.transaction.MessagingNotification;
import com.mediatek.simmessage.ManageSimMessages;

/**
 * MmsSystemEventReceiver receives the
 * {@link android.content.intent.ACTION_BOOT_COMPLETED},
 * {@link com.android.internal.telephony.TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED}
 * and performs a series of operations which may include:
 * <ul>
 * <li>Show/hide the icon in notification area which is used to indicate
 * whether there is new incoming message.</li>
 * <li>Resend the MM's in the outbox.</li>
 * </ul>
 */
public class MmsSystemEventReceiver extends BroadcastReceiver {
    private static final String TAG = "MmsSystemEventReceiver";
    private static MmsSystemEventReceiver sMmsSystemEventReceiver;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        MmsLog.i(TAG, "onReceive action: " + action);

        /// Add for runtime permission @{
        if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
            MmsLog.d(MmsApp.LOG_TAG, "MmsSystemEventReceiver no permissions return !!");
            return;
        }
        /// @}

        if (action.equals(Mms.Intents.CONTENT_CHANGED_ACTION)) {
            final Intent mIntent = intent;
            new Thread(new Runnable() {
                @Override
                public void run() {
                    Uri changed = (Uri) mIntent.getParcelableExtra(Mms.Intents.DELETED_CONTENTS);
                    if (changed != null) {
                        MmsApp.getApplication().getPduLoaderManager().removePdu(changed);
                    }
                    MmsLog.d(MmsApp.TXN_TAG, "Mms.Intents.CONTENT_CHANGED_ACTION: " + changed);
                }
            }).start();
        /// @}
        /// M:Code analyze 007,add for listening device storage full or not @{
        }  else if (action.equals(Intent.ACTION_DEVICE_STORAGE_FULL)) {//mem slim
            handleStorageFull(context, intent, true);
        } else if (action.equals(Intent.ACTION_DEVICE_STORAGE_NOT_FULL)) {//mem slim
            handleStorageFull(context, intent, false);
        /// M: new feature, add default quick_text @{
        } else if (action.equals(Intent.ACTION_LOCALE_CHANGED)) {
            if (MmsConfig.getInitQuickText()) {
                return;
            }
            MmsConfig.getQuicktexts().clear();
            MmsConfig.getQuicktextsId().clear();
        }
        /// @}
    }
//mem slim
/*
    public static void registerForConnectionStateChanges(Context context) {
        /// M:
        MmsLog.d(MmsApp.TXN_TAG, "registerForConnectionStateChanges");
        unRegisterForConnectionStateChanges(context);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "registerForConnectionStateChanges");
        }
        if (sMmsSystemEventReceiver == null) {
            sMmsSystemEventReceiver = new MmsSystemEventReceiver();
        }

        context.registerReceiver(sMmsSystemEventReceiver, intentFilter);
    }

    public static void unRegisterForConnectionStateChanges(Context context) {
        /// M:
        MmsLog.d(MmsApp.TXN_TAG, "unRegisterForConnectionStateChanges");
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "unRegisterForConnectionStateChanges");
        }
        if (sMmsSystemEventReceiver != null) {
            try {
                context.unregisterReceiver(sMmsSystemEventReceiver);
            } catch (IllegalArgumentException e) {
                // Allow un-matched register-unregister calls
            }
        }
    }
*/
    /// M:Code analyze 009,add for setting the pending mms failed,
    /// mainly using after boot complete @{
    public static void setPendingMmsFailed(final Context context) {
        MmsLog.d(MmsApp.TXN_TAG, "setPendingMmsFailed");
        Cursor cursor = MtkPduPersister.getPduPersister(context).getPendingMessages(
                Long.MAX_VALUE/*System.currentTimeMillis()*/);
        if (cursor != null) {
            try {
                int count = cursor.getCount();
                MmsLog.d(MmsApp.TXN_TAG, "setPendingMmsFailed: Pending Message Size=" + count);

                if (count == 0) {
                    return;
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

                    MmsLog.d(MmsApp.TXN_TAG, "setPendingMmsFailed: type=" + type + "; MsgId=" + id);

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
                MessagingNotification.notifySendFailed(context, true);
            } catch (SQLiteDiskIOException e) {
                // Ignore
                MmsLog.e(MmsApp.TXN_TAG,
                        "SQLiteDiskIOException caught while set pending message failed", e);
            } finally {
                cursor.close();
            }
        } else {
            MmsLog.d(MmsApp.TXN_TAG, "setPendingMmsFailed: no pending MMS.");
        }
    }
    /// @}

    /// M:Code analyze 011,add for setting the mms being downloading when shutdown to unrecognized
    /// after boot complete again,have to manual download @{
    public static void setNotificationIndUnstarted(final Context context) {
        MmsLog.d(MmsApp.TXN_TAG, "setNotificationIndUnstarted");
        Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(), Mms.CONTENT_URI,
                new String[] {Mms._ID, Mms.STATUS, Mms.THREAD_ID},
                Mms.MESSAGE_TYPE + "=" + MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND, null, null);
        if (cursor != null) {
            try {
                int count = cursor.getCount();
                MmsLog.d(MmsApp.TXN_TAG, "setNotificationIndUnstarted: Message Size=" + count);

                if (count == 0) {
                    return;
                }

                ContentValues values = null;
                int id = 0;
                int status = 0;
                while (cursor.moveToNext()) {
                    id = cursor.getInt(0);
                    status = cursor.getInt(1);
                    MmsLog.d(MmsApp.TXN_TAG,
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
                // Ignore
                MmsLog.e(MmsApp.TXN_TAG,
                        "SQLiteDiskIOException caught while set notification ind unstart", e);
            } finally {
                cursor.close();
            }
        } else {
            MmsLog.d(MmsApp.TXN_TAG, "setNotificationIndUnstarted: no pending messages.");
        }
    }
    /// @}

    //mem slim
    private final int NOTIFICATION_STORAGE_LIMITED_ID = -1;

    private void handleStorageFull(Context context, Intent intent, boolean full) {
        if (full == true) {
            MmsConfig.setDeviceStorageFullStatus(true);
            notifyReachStorageLimited(context);
        } else {
            MmsConfig.setDeviceStorageFullStatus(false);
            MessagingNotification.cancelNotification(context,
                    SmsRejectedReceiver.SMS_REJECTED_NOTIFICATION_ID);
            cancelStorageLimitedWarning(context);
        }
    }

    private void notifyReachStorageLimited(Context context) {
        NotificationManager notificationManager =
            (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        Notification.Builder mBuilder =
                new Notification.Builder(context)
                        .setSmallIcon(R.mipmap.ic_launcher_smsmms)
                        .setContentTitle(context.getString(R.string.storage_warning_title))
                        .setChannelId(MessagingNotification.CHANNEL_ID)
                        .setContentText(context.getString(R.string.storage_warning_content))
                        .setDefaults(Notification.DEFAULT_ALL)
                        .setOngoing(true);
        notificationManager.notify(NOTIFICATION_STORAGE_LIMITED_ID, mBuilder.build());
    }

    private void cancelStorageLimitedWarning(Context context) {
        NotificationManager notificationManager =
            (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.cancel(NOTIFICATION_STORAGE_LIMITED_ID);
    }

}
