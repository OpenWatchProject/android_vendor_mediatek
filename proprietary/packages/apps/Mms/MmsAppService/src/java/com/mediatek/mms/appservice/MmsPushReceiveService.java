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

package com.mediatek.mms.appservice;


import android.app.Notification;
import android.app.Service;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.IBinder;
import android.os.SystemProperties;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Mms.Inbox;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
//import com.android.mms.MmsApp;
//import com.android.mms.MmsConfig;
//import com.android.mms.util.MmsLog;
//import com.android.mms.widget.MmsWidgetProvider;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.DeliveryInd;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.ReadOrigInd;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduParser;
import com.mediatek.android.mms.pdu.MtkPduPersister;
//import com.mediatek.ipmsg.util.IpMessageUtils;
//import com.mediatek.mms.ipmessage.IIpSpamMsgReceiverExt;
//import com.mediatek.opmsg.util.OpMessageUtils;
//import com.mediatek.setting.MmsPreferenceActivity;

import mediatek.telephony.MtkTelephony.WapPush;

import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_DELIVERY_IND;
import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;
import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_READ_ORIG_IND;


    /*
     * service for receive the push message.
    */
public class MmsPushReceiveService extends Service {

    private static final String TAG = "MmsPushReceiveService";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;
    private static int  serviceRunningNum = 0;
    private static final boolean MTK_LOW_RAM_SUPPORT = SystemProperties.get("ro.config.low_ram").equals("true");
//    private IIpSpamMsgReceiverExt mIpSpamMsgReceiver;

    @Override
    public void onCreate() {
        super.onCreate();
        /// M: raise priority, decrease the LMK killing rate @{
//        Notification noti = new Notification(0, null, System.currentTimeMillis());
//        noti.flags |= Notification.FLAG_NO_CLEAR;
//        startForeground(1, noti);
        /// @}
        Log.d(LogTag.TXN_TAG, "MmsPushReceiveService.onCreate");
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(LogTag.TXN_TAG, "MmsPushReceiveService.onBind");
        return null;
    }

    public boolean onUnbind(Intent intent) {
        super.onUnbind(intent);
        Log.d(LogTag.TXN_TAG, "MmsPushReceiveService.onUnbind");
        return true;
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        Log.d(LogTag.TXN_TAG, "MmsPushReceiveService.onStartCommand");

        /*if (!MTK_LOW_RAM_SUPPORT) {
            Log.d(LogTag.TXN_TAG, "It is not low ram project, not execute");
            return START_NOT_STICKY;
        }*/
        MmsConfig.setMmsConfig(intent);
        new ReceivePushTask(this).execute(intent);

        //return null;
        return START_STICKY_COMPATIBILITY;
    }

    public void onRebind(Intent intent) {
        super.onRebind(intent);
        Log.d(LogTag.TXN_TAG, "onRebind");
    }

    public void onDestroy() {
        super.onDestroy();
        /// M: cancel raise priority, it should stopForeground here, when the service is running,
        //     and then another sms comes, onStartCommond will be called, so the service can not
        //     stop itself, if stopForeground is called before stopSelf, the priority of process
        //     will down to 9, LMK can easily kill it @{
        stopForeground(true);
        /// @}
        Log.d(LogTag.TXN_TAG, "onDestroy");

    }

    private class ReceivePushTask extends AsyncTask<Intent, Void, Void> {
        private Context mContext;
        public ReceivePushTask(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Intent... intents) {
            Intent intent = intents[0];
            /// M:
            int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                    SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            Log.d(LogTag.TXN_TAG, "PushReceiver, do In Background, subId= " + subId);
            // Get raw PDU push-data from the message and parse it
            byte[] pushData = intent.getByteArrayExtra("data");
            MtkPduParser parser = new MtkPduParser(pushData,
                    PduParserUtil.shouldParseContentDisposition(subId));
            int type = 0;
            try {
                GenericPdu pdu = parser.parse();
                serviceRunningNum = serviceRunningNum + 1;
                if (null == pdu) {
                    Log.e(TAG, "Invalid PUSH data");
                    return null;
                }

                MtkPduPersister p = MtkPduPersister.getPduPersister(mContext.getApplicationContext());
                ContentResolver cr = mContext.getContentResolver();
                type = pdu.getMessageType();
                long threadId = -1;
                switch (type) {
                    case MESSAGE_TYPE_DELIVERY_IND:
                        /// M:
                        Log.d(LogTag.TXN_TAG, "type=MESSAGE_TYPE_DELIVERY_IND");
                        /// M: fall through
                    case MESSAGE_TYPE_READ_ORIG_IND: {
                        /** M: print log @{ */
                        if (type == MESSAGE_TYPE_READ_ORIG_IND) {
                            Log.d(LogTag.TXN_TAG, "type=MESSAGE_TYPE_READ_ORIG_IND");
                        }
                        /** @} */
                        threadId = findThreadId(mContext, pdu, type);
                        if (threadId == -1) {
                            // The associated MtkSendReq isn't found, therefore skip
                            // processing this PDU.
                            break;
                        }

                        /// M: google jb.mr1 patch, group mms
                        Uri uri = p.persist(pdu, Inbox.CONTENT_URI, true,
                                MmsConfig.getIsGroupMmsEnabled(), null);
                        // Update thread ID for ReadOrigInd & DeliveryInd.
                        ContentValues values = new ContentValues(2);
                        values.put(Mms.THREAD_ID, threadId);
                        values.put(Mms.SUBSCRIPTION_ID, subId);
                        SqliteWrapper.update(mContext, cr, uri, values, null, null);
                        serviceRunningNum = serviceRunningNum - 1;
                        break;
                    }
                    case MESSAGE_TYPE_NOTIFICATION_IND: {
                        /// M:
                        Log.d(LogTag.TXN_TAG, "type=MESSAGE_TYPE_NOTIFICATION_IND");

                        // add for ip spam message
                        /*
                        mIpSpamMsgReceiver =
                            IpMessageUtils.getIpMessagePlugin(mContext).getIpSpamMsgReceiver();
                        if (mIpSpamMsgReceiver.onIpReceiveSpamMsg(mContext, intent, true)) {
                            return null;
                        }
*/
                        NotificationInd nInd = (NotificationInd) pdu;
/*
                        if (MmsConfig.getTransIdEnabled()) {
                            byte [] contentLocation = nInd.getContentLocation();
                            if ('=' == contentLocation[contentLocation.length - 1]) {
                                byte [] transactionId = nInd.getTransactionId();
                                byte [] contentLocationWithId = new byte [contentLocation.length
                                                                          + transactionId.length];
                                System.arraycopy(contentLocation, 0, contentLocationWithId,
                                        0, contentLocation.length);
                                System.arraycopy(transactionId, 0, contentLocationWithId,
                                        contentLocation.length, transactionId.length);
                                nInd.setContentLocation(contentLocationWithId);
                            }
                        }
*/
                        Uri uri = isDuplicateNotification(mContext, nInd);
                        if (uri == null) {
                            /// M: google jb.mr1 patch, group mms
                            // Save the pdu. If we can start downloading the real pdu immediately,
                            // don't allow persist() to create a thread for the notificationInd
                            // because it causes UI jank.
                            uri = p.persist(pdu, Inbox.CONTENT_URI,
                                    true,
                                    MmsConfig.getIsGroupMmsEnabled(), null);
                        } else if (LOCAL_LOGV) {
                            Log.d(LogTag.TXN_TAG, "Already have a Uri = " + uri);
                        }
                        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                            Log.e(TAG, "subId is invalid");
                            break;
                        }
                        /// M:Code analyze 002, add for msim,update pdu and pending messages @{
                        // update pdu
                        ContentValues values = new ContentValues(2);
                        values.put(Mms.SUBSCRIPTION_ID, subId);
                        values.put(WapPush.SERVICE_ADDR,
                                intent.getStringExtra(WapPush.SERVICE_ADDR));
                        SqliteWrapper.update(mContext, cr, uri, values, null, null);
                        MmsLog.dpi(LogTag.TXN_TAG, "save notification subId=" + subId
                                + "\tsc=" + intent.getStringExtra(WapPush.SERVICE_ADDR)
                                + "\taddr=" + intent.getStringExtra(WapPush.ADDR)
                                + "\turi = " + uri);

                        // update pending messages
                        long msgId = 0;
                        Cursor cursor = SqliteWrapper.query(mContext,
                                mContext.getContentResolver(),
                                uri, new String[] {Mms._ID},
                                null, null, null);
                        if (cursor != null) {
                            try {
                                if (cursor.getCount() == 1 && cursor.moveToFirst()) {
                                    msgId = cursor.getLong(0);
                                    Log.d(LogTag.TXN_TAG, "msg id = " + msgId);
                                }
                            } finally {
                                cursor.close();
                            }
                        }

                        Uri.Builder uriBuilder = PendingMessages.CONTENT_URI.buildUpon();
                        uriBuilder.appendQueryParameter("protocol", "mms");
                        uriBuilder.appendQueryParameter("message", String.valueOf(msgId));
                        Cursor pendingCursor =
                            SqliteWrapper.query(mContext, mContext.getContentResolver(),
                                uriBuilder.build(), new String[] {PendingMessages._ID},
                                null, null, null);
                        if (pendingCursor != null) {
                            try {
                                if (pendingCursor.getCount() == 1 && pendingCursor.moveToFirst()) {
                                    ContentValues valuesforPending = new ContentValues();
                                    valuesforPending.put(PendingMessages.SUBSCRIPTION_ID, subId);
                                    long id = pendingCursor.getLong(0);
                                    SqliteWrapper.update(mContext, mContext.getContentResolver(),
                                            PendingMessages.CONTENT_URI,
                                            valuesforPending, PendingMessages._ID + "=" + id, null);
                                } else {
                                    Log.w(LogTag.TXN_TAG, "can not find message to "
                                            + " set pending sim id, msgId="
                                            + msgId);
                                }
                            } finally {
                                pendingCursor.close();
                            }
                        }
                        /// @}
                        // Start service to finish the notification transaction.
                        Intent svc = new Intent(mContext, TransactionService.class);
                        svc.putExtra(TransactionBundle.URI, uri.toString());
                        svc.putExtra(TransactionBundle.TRANSACTION_TYPE,
                                Transaction.NOTIFICATION_TRANSACTION);
                        svc.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
                        /// @}
                        mContext.startService(svc);
                        //MmsWidgetProvider.notifyDatasetChanged(mContext);
                        serviceRunningNum = serviceRunningNum - 1;
                        break;
                    }
                    default:
                        Log.e(TAG, "Received unrecognized PDU.");
                }
            } catch (MmsException e) {
                Log.e(TAG, "Failed to save the data from PUSH: type=" + type, e);
            } catch (RuntimeException e) {
                Log.e(TAG, "Unexpected RuntimeException.", e);
            /// M:Code analyze 004,unknown, add finally @{
            }  finally {
                //raisePriority(mContext, false);
                if (serviceRunningNum == 0) {
                    stopSelf();
                    Log.d(TAG, "stop service");
                }
                Log.d(LogTag.TXN_TAG, "Normal priority");
            /// @}
            }

            if (LOCAL_LOGV) {
                Log.v(TAG, "PUSH Intent processed.");
            }

            return null;
        }
    }


    private static long findThreadId(Context context, GenericPdu pdu, int type) {
        String messageId;

        if (type == MESSAGE_TYPE_DELIVERY_IND) {
            messageId = new String(((DeliveryInd) pdu).getMessageId());
        } else {
            messageId = new String(((ReadOrigInd) pdu).getMessageId());
        }

        StringBuilder sb = new StringBuilder('(');
        sb.append(Mms.MESSAGE_ID);
        sb.append('=');
        sb.append(DatabaseUtils.sqlEscapeString(messageId));
        sb.append(" AND ");
        sb.append(Mms.MESSAGE_TYPE);
        sb.append('=');
        sb.append(MtkPduHeaders.MESSAGE_TYPE_SEND_REQ);
        // TODO ContentResolver.query() appends closing ')' to the selection argument
        // sb.append(')');

        Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(),
                            Mms.CONTENT_URI, new String[] { Mms.THREAD_ID },
                            sb.toString(), null, null);
        if (cursor != null) {
            try {
                if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                    return cursor.getLong(0);
                }
            } finally {
                cursor.close();
            }
        }

        return -1;
    }

    private static Uri isDuplicateNotification(
            Context context, NotificationInd nInd) {
        byte[] rawLocation = nInd.getContentLocation();
        if (rawLocation != null) {
            String location = new String(rawLocation);
            String selection = Mms.CONTENT_LOCATION + " = ?";
            String[] selectionArgs = new String[] { location };
            Cursor cursor = SqliteWrapper.query(
                    context, context.getContentResolver(),
                    Mms.CONTENT_URI, new String[] { Mms._ID },
                    selection, selectionArgs, null);
            if (cursor != null) {
                try {
                    if (cursor.getCount() > 0) {
                        /// M:Code analyze 006,add log if duplicate @{
                        cursor.moveToFirst();
                        MmsLog.dpi(LogTag.TXN_TAG, "duplicate, location="
                                + location + ", id=" + cursor.getLong(0));
                        /// @}
                        // We already received the same notification before.
                        return Uri.withAppendedPath(Inbox.CONTENT_URI,
                                Long.toString(cursor.getLong(0)));
                    }
                } finally {
                    cursor.close();
                }
            }
        }
        return null;
    }

}

