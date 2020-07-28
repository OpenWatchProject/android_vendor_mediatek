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

import static android.provider.Telephony.Sms.Intents.WAP_PUSH_DELIVER_ACTION;
import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_DELIVERY_IND;
import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;
import static com.mediatek.android.mms.pdu.MtkPduHeaders.MESSAGE_TYPE_READ_ORIG_IND;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.provider.Telephony.Mms.Inbox;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.android.mms.widget.MmsWidgetProvider;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.DeliveryInd;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduParser;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.google.android.mms.pdu.ReadOrigInd;

import com.mediatek.mms.appserviceproxy.AppServiceProxy;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.MmsPreferenceActivity;
import com.mediatek.wappush.WapPushReceiverService;

import mediatek.telephony.MtkTelephony.WapPush;

/**
 * Receives Intent.WAP_PUSH_RECEIVED_ACTION intents and starts the
 * TransactionService by passing the push-data to it.
 */
public class PushReceiver extends BroadcastReceiver {
    private static final String TAG = "PushReceiver";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;

    private PowerManager.WakeLock mWakeLock;

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
            MmsLog.d(MmsApp.TXN_TAG, "PushReceiver, do In Background, subId= " + subId);
            // Get raw PDU push-data from the message and parse it
            byte[] pushData = intent.getByteArrayExtra("data");
            MtkPduParser parser = new MtkPduParser(
                    pushData, shouldParseContentDisposition(subId));
            int type = 0;
            try {
                GenericPdu pdu = parser.parse();

                if (null == pdu) {
                    Log.e(TAG, "Invalid PUSH data");
                    return null;
                }

                MtkPduPersister p = MtkPduPersister.getPduPersister(mContext);
                ContentResolver cr = mContext.getContentResolver();
                type = pdu.getMessageType();
                long threadId = -1;
                switch (type) {
                    case MESSAGE_TYPE_DELIVERY_IND:
                        /// M:
                        MmsLog.d(MmsApp.TXN_TAG, "type=MESSAGE_TYPE_DELIVERY_IND");
                        /// M: fall through
                    case MESSAGE_TYPE_READ_ORIG_IND: {
                        if (type == MESSAGE_TYPE_READ_ORIG_IND) {
                            MmsLog.d(MmsApp.TXN_TAG, "type=MESSAGE_TYPE_READ_ORIG_IND");
                        }
                        threadId = findThreadId(mContext, pdu, type);
                        if (threadId == -1) {
                            // The associated MtkSendReq isn't found, therefore skip
                            // processing this PDU.
                            break;
                        }

                        /// M: google jb.mr1 patch, group mms
                        Uri uri = p.persist(pdu, Inbox.CONTENT_URI, true,
                                MmsPreferenceActivity.getIsGroupMmsEnabled(mContext), null);
                        // Update thread ID for ReadOrigInd & DeliveryInd.
                        ContentValues values = new ContentValues(2);
                        values.put(Mms.THREAD_ID, threadId);
                        values.put(Mms.SUBSCRIPTION_ID, subId);
                        SqliteWrapper.update(mContext, cr, uri, values, null, null);
                        break;
                    }
                    case MESSAGE_TYPE_NOTIFICATION_IND: {
                        MmsLog.d(MmsApp.TXN_TAG, "type=MESSAGE_TYPE_NOTIFICATION_IND");
                        NotificationInd nInd = (NotificationInd) pdu;

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

                        Uri uri = isDuplicateNotification(mContext, nInd);
                        if (uri == null) {
                            /// M: google jb.mr1 patch, group mms
                            // Save the pdu. If we can start downloading the real pdu immediately,
                            // don't allow persist() to create a thread for the notificationInd
                            // because it causes UI jank.
                            uri = p.persist(pdu, Inbox.CONTENT_URI,
                                    true,
                                    MmsPreferenceActivity.getIsGroupMmsEnabled(mContext), null);
                        } else if (LOCAL_LOGV) {
                            MmsLog.d(MmsApp.TXN_TAG, "Already have a Uri = " + uri);
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
                        MmsLog.dpi(MmsApp.TXN_TAG, "save notification subId=" + subId
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
                                    MmsLog.w(MmsApp.TXN_TAG, "can not find message to set "
                                            + " pending sim id, msgId="
                                            + msgId);
                                }
                            } finally {
                                pendingCursor.close();
                            }
                        }
                        /// @}
                        // Start service to finish the notification transaction.
                        Intent svc = new Intent();
                        svc.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE,
                                MmsConfig.TRANSACTION_SERVICE);
                        svc.putExtra(TransactionBundle.URI, uri.toString());
                        svc.putExtra(TransactionBundle.TRANSACTION_TYPE,
                                TransactionBundle.NOTIFICATION_TRANSACTION);
                        svc.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
                        MmsConfig.setMmsConfig(svc, subId);
                        /// @}
                        mContext.startService(svc);
                        /*
                        Context ct = mContext.getApplicationContext();
                        AppServiceProxy.getInstance(ct).startServiceByParam(ct, svc);
                        */
                        MmsWidgetProvider.notifyDatasetChanged(mContext);
                        break;
                    }
                    default:
                        Log.e(TAG, "Received unrecognized PDU.");
                }
            } catch (MmsException e) {
                Log.e(TAG, "Failed to save the data from PUSH: type=" + type, e);
            } catch (RuntimeException e) {
                Log.e(TAG, "Unexpected RuntimeException.", e);
            }

            if (LOCAL_LOGV) {
                Log.v(TAG, "PUSH Intent processed.");
            }

            return null;
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        /// M: Avoid runtime permission JE @{
        if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
            Log.v(TAG, "onReceive no runtime permissions return, intent: " + intent);
            return;
        }
        /// @}

        // KK migration, for default MMS function.
        // The changes in this project update the in-box SMS/MMS app to use the new
        // intents for SMS/MMS delivery.
        // And caused by just one receiver can receive WAP_PUSH_DELIVER_ACTION broadcast on KK,
        // so move handle WAPPUSH message action from WapPushReceiver to here, and WapPushReceiver
        // been removed, WapPush message directly send to WapPushReceiverService to handle. /// @{
        if (intent.getAction().equals(WAP_PUSH_DELIVER_ACTION)) {
            Log.v(TAG, "Received PUSH Intent: " + intent + " support WAPPUSH? "
                    + FeatureOption.MTK_WAPPUSH_SUPPORT);

            /// M:Code analyze 005, Hold a wake lock for 5 seconds, enough to give any
            /// services we start time to take their own wake locks. @{
            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            if (mWakeLock != null && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                                            "MMS PushReceiver");
            mWakeLock.acquire(5000);
            /// @}
            if (MmsContentType.MMS_MESSAGE.equals(intent.getType())) {
                /*
                boolean isLowRamProject = FeatureOption.MTK_LOW_RAM_SUPPORT;
                Log.d(TAG, "Mms push, low_ram = " + isLowRamProject);
                if (isLowRamProject) {
                */
                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                    SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    MmsConfig.setMmsConfig(intent, subId);
                    intent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE, MmsConfig.MMS_PUSH_RECEIVER_SERVICE);
                    context.startService(intent);
                /*
                } else {
                    new ReceivePushTask(context).execute(intent);
                }
                */
                Log.d(TAG, "Mms push, receiver handled");
            } else if (FeatureOption.MTK_WAPPUSH_SUPPORT) {
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
                boolean isWapPushEnabled = prefs.getBoolean("pref_key_wappush_enable", true);
                Log.d(TAG, "receive wappush isWapPushEnabled: " + isWapPushEnabled);
                if (isWapPushEnabled) {
                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    Log.d(TAG, "onReceive WAPPUSH subId = " + subId
                            + ", Action = "
                            + intent.getAction() + ", result = " + getResultCode());
                    intent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE,
                            MmsConfig.WAPPUSH_RECEIVER_SERVICE);
                    intent.putExtra("result", getResultCode());
                    /* start service*/
                    context.startService(intent);
                    //Context ct = context.getApplicationContext();
                    //AppServiceProxy.getInstance(ct).startServiceByParam(ct, intent);
                    /*
                    new Thread(new Runnable() {
                        public void run() {
                            MmsLog.d(MmsApp.TXN_TAG,
                                "PushReceiver.onReceive: call startServiceByParam");
                            Context ct = context.getApplicationContext();
                            AppServiceProxy.getInstance(ct).startServiceByParam(ct, intent);
                        }
                    }).start();*/
                }
            }
        }
        /// @}
    }

    private static long findThreadId(Context context, GenericPdu pdu, int type) {
        String messageId;

        if (type == MESSAGE_TYPE_DELIVERY_IND) {
            messageId = new String(((DeliveryInd) pdu).getMessageId());
        } else {
            messageId = new String(((ReadOrigInd) pdu).getMessageId());
        }
        MmsLog.d(MmsApp.TXN_TAG, "messageId = " + messageId);

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
                        cursor.moveToFirst();
                        MmsLog.dpi(MmsApp.TXN_TAG, "duplicate notification, location="
                                + location + ", id=" + cursor.getLong(0));
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

    public static boolean shouldParseContentDisposition(int subId) {
        SmsManager smsManager = SmsManager.getSmsManagerForSubscriptionId(subId);
        if (smsManager != null) {
            Bundle bundle = smsManager.getCarrierConfigValues();
            if (bundle != null) {
                return bundle.getBoolean(SmsManager.MMS_CONFIG_SUPPORT_MMS_CONTENT_DISPOSITION,
                        true);
            }
        }
        return true;
        //M: google original code
        /*
        return SmsManager
                .getSmsManagerForSubscriptionId(subId)
                .getCarrierConfigValues()
                .getBoolean(SmsManager.MMS_CONFIG_SUPPORT_MMS_CONTENT_DISPOSITION, true);
        */
    }
}
