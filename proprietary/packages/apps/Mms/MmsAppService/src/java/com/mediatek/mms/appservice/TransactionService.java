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

package com.mediatek.mms.appservice;

import android.app.Activity;
import android.app.Service;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.database.sqlite.SQLiteException;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.SystemClock;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduParser;
import com.mediatek.android.mms.pdu.MtkPduPersister;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * The TransactionService of the MMS Client is responsible for handling requests
 * to initiate client-transactions sent from:
 * <ul>
 * <li>The Proxy-Relay (Through Push messages)</li>
 * <li>The composer/viewer activities of the MMS Client (Through intents)</li>
 * </ul>
 * The TransactionService runs locally in the same process as the application.
 * It contains a HandlerThread to which messages are posted from the
 * intent-receivers of this application.
 * <p/>
 * <b>IMPORTANT</b>: This is currently the only instance in the system in
 * which simultaneous connectivity to both the mobile data network and
 * a Wi-Fi network is allowed. This makes the code for handling network
 * connectivity somewhat different than it is in other applications. In
 * particular, we want to be able to send or receive MMS messages when
 * a Wi-Fi connection is active (which implies that there is no connection
 * to the mobile data network). This has two main consequences:
 * <ul>
 * <li>Testing for current network connectivity ({@link android.net.NetworkInfo#isConnected()} is
 * not sufficient. Instead, the correct test is for network availability
 * ({@link android.net.NetworkInfo#isAvailable()}).</li>
 * <li>If the mobile data network is not in the connected state, but it is available,
 * we must initiate setup of the mobile data connection, and defer handling
 * the MMS transaction until the connection is established.</li>
 * </ul>
 */
public class TransactionService extends Service implements Observer {
    private static final String TAG = LogTag.TAG;

    /**
     * Action for the Intent which is sent by Alarm service to launch
     * TransactionService.
     */
    public static final String ACTION_ONALARM = "android.intent.action.ACTION_ONALARM";

    /**
     * Action for Mms Service process a transaction finished. The action will be set into
     * a transaction request. After sending/downloading finished, Mms service will send it back.
     */
    public static final String ACTION_TRANSACION_PROCESSED
            = "com.mediatek.mms.appservice.TRANSACION_PROCESSED";

    private static final int EVENT_TRANSACTION_REQUEST = 1;
    private static final int EVENT_TRANSACTION_PROCESSED = 2;
    private static final int EVENT_NEW_INTENT = 5;
    private static final int EVENT_PROCESS_TIME_OUT = 6;
    private static final int EVENT_QUIT = 100;

    private static final long TRANSACTION_PROCESS_TIME_OUT = 10L * 60 * 1000;

    private ServiceHandler mServiceHandler;
    private Looper mServiceLooper;
    private final ArrayList<Transaction> mProcessing  = new ArrayList<Transaction>();

    private PowerManager.WakeLock mWakeLock;
    private static final int FAILE_TYPE_PERMANENT = 1;
    private static final int FAILE_TYPE_TEMPORARY = 2;
    private static final int FAILE_TYPE_RESTAIN_RETRY = 3;


    @Override
    public void onCreate() {
        Log.d(LogTag.TXN_TAG, "TransactionService.onCreate->this = " + this);
        // Start up the thread running the service.  Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block.
        HandlerThread thread = new HandlerThread("TransactionService");
        thread.start();

        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(LogTag.TXN_TAG, "TransactionService.onStartCommand: intent = " + intent + ", startId = " + startId);
        if (intent != null) {
            Message msg = null;
            Log.d(LogTag.TXN_TAG, "onStartCommand action = " + intent.getAction() + "->this = " + this);
            if (ACTION_TRANSACION_PROCESSED.equals(intent.getAction())) {
                if (mServiceHandler.hasMessages(EVENT_PROCESS_TIME_OUT)) {
                    mServiceHandler.removeMessages(EVENT_PROCESS_TIME_OUT);
                }
                msg = mServiceHandler.obtainMessage(EVENT_TRANSACTION_PROCESSED);
            } else {
                msg = mServiceHandler.obtainMessage(EVENT_NEW_INTENT);
            }
//            Bundle extras = intent.getExtras();
//            if (extras != null) {
//                DefaultRetryScheme.setRetryScheme(extras.getIntArray("retry_scheme"));
//            }
            msg.arg1 = startId;
            msg.obj = intent;
            mServiceHandler.sendMessage(msg);
        }
        return Service.START_NOT_STICKY;
    }

    public void onNewIntent(Intent intent, int serviceId) {
        ConnectivityManager connMgr = (ConnectivityManager) getSystemService(
                Context.CONNECTIVITY_SERVICE);
        if (connMgr == null) {
            Log.d(LogTag.TXN_TAG, "stopSelf1->" + serviceId);
            stopSelf(serviceId);
            Log.e(TAG, "onNewIntent(), ConnMgr null");
            return;
        }

        // save mms configs
        MmsConfig.setMmsConfig(intent);

        NetworkInfo ni = connMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE_MMS);
        boolean noNetwork = false; // ni == null || !ni.isAvailable();

        String action = intent.getAction();
        int type = intent.getIntExtra(TransactionBundle.TRANSACTION_TYPE, -1);
        String msgUri = intent.getStringExtra(TransactionBundle.URI);
        int transactionSubId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        Log.i(LogTag.TXN_TAG, "onNewIntent: action:" + action + ", type:" + type +
                ", msgUri:" + msgUri + ", subId:" + transactionSubId + ", serviceId = " + serviceId);
        if (ACTION_ONALARM.equals(action) || msgUri == null) {
            // Scan database to find all pending operations.
            long current = SystemClock.elapsedRealtime();
            Log.d(LogTag.TXN_TAG, "onNewIntent query pending mms, current time = " + current);

            Cursor cursor = MtkPduPersister.getPduPersister(getApplicationContext()).getPendingMessages(
                    current);
            if (cursor != null) {
                try {
                    int count = cursor.getCount();

                 //   if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                        Log.v(TAG, "onNewIntent: cursor.count=" + count + " action=" + action);
                 //   }

                    if (count == 0) {
                        Log.d(TAG, "onNewIntent: no pending messages. Stopping service.");
                        RetryScheduler.setRetryAlarm(getApplicationContext());
                        stopSelfIfIdle(serviceId);
                        return;
                    }

                    int columnIndexOfMsgId = cursor.getColumnIndexOrThrow(PendingMessages.MSG_ID);
                    int columnIndexOfMsgType = cursor.getColumnIndexOrThrow(
                            PendingMessages.MSG_TYPE);
                    int columnIndexOfSubIndex = cursor
                            .getColumnIndexOrThrow(PendingMessages.SUBSCRIPTION_ID);

                    while (cursor.moveToNext()) {
                        int msgType = cursor.getInt(columnIndexOfMsgType);
                        int transactionType = getTransactionType(msgType);
                        int subId = cursor.getInt(columnIndexOfSubIndex);
                        Uri uri = ContentUris.withAppendedId(
                                Mms.CONTENT_URI,
                                cursor.getLong(columnIndexOfMsgId));
                        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                            Log.v(TAG, "onNewIntent: msgType=" + msgType + " transactionType=" +
                                    transactionType + " subId = " + subId + " uri = " + uri);
                        }
                        if (noNetwork) {
                            onNetworkUnavailable(serviceId, transactionType);
                            return;
                        }
                        switch (transactionType) {
                            case -1:
                                break;
                            case Transaction.RETRIEVE_TRANSACTION:
                                // If it's a transiently failed transaction,
                                // we should retry it in spite of current
                                // downloading mode. If the user just turned on the auto-retrieve
                                // option, we also retry those messages that don't have any errors.
                                int failureType = cursor.getInt(
                                        cursor.getColumnIndexOrThrow(
                                                PendingMessages.ERROR_TYPE));
                                boolean autoDownload = MmsConfig.isAutoDownload(subId);
                                if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                                    Log.v(TAG, "onNewIntent: failureType=" + failureType +
                                            " action=" + action + " isTransientFailure:" +
                                            isTransientFailure(failureType) + " autoDownload=" +
                                            autoDownload + "for subId " + subId);
                                }
                                if (!autoDownload) {
                                    // If autodownload is turned off, don't process the
                                    // transaction.
                                    if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                                        Log.v(TAG, "onNewIntent: skipping - autodownload off");
                                    }
                                    // Re-enable "download" button if auto-download is off
                                    DownloadManager downloadManager =
                                           DownloadManager.getInstance();
                                    int mmsStatus = downloadManager.getState(uri);
                                    Log.i(TAG, "onNewIntent: downloadManager.getState  = "
                                            + mmsStatus);
                                    if (mmsStatus != DownloadManager.STATE_DOWNLOADING) {
                                        downloadManager.markState(uri,
                                                DownloadManager.STATE_SKIP_RETRYING);
                                    }
                                    break;
                                }
                                // Logic is twisty. If there's no failure or the failure
                                // is a non-permanent failure, we want to process the transaction.
                                // Otherwise, break out and skip processing this transaction.
                                if (!(failureType == MmsSms.NO_ERROR ||
                                        isTransientFailure(failureType))) {
                                    if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                                        Log.v(TAG, "onNewIntent: skipping - permanent error");
                                    }
                                    break;
                                }

                                /// M: ALPS00545779, for FT, restart pending receiving mms @ {
                                if (!isPendingMmsNeedRestart(uri, failureType)) {
                                    Log.d(LogTag.TXN_TAG, uri
                                            + "this RETRIEVE not transient failure");
                                    break;
                                } else if (!isTransientFailure(failureType)) {
                                    Log.d(LogTag.TXN_TAG, uri
                                            + "this RETRIEVE not transient failure");
                                    break;
                                }
                                /// @}

                               // fall-through
                            default:
                                TransactionBundle args = new TransactionBundle(
                                        transactionType, uri.toString());
                                /* ALPS03529875, timing issue, a mms may be send twice
                                 *    1. When to send a mms, will do
                                 *       <a1>: insert a pending message in db
                                 *       <a2>: add a mms transaction into mProcessing list,
                                 *                 and send a send request to mmsservice
                                 *    2. After the mms was send success in mms service
                                 *       TRANSACTION_PROCESSED was send back, will do
                                 *      <b1>: remove the mms transaction from mProcessing list
                                 *      <b2>: delete the pending message from db
                                 *    3. When start TransactionService with action is null, will do
                                 *      <c1>: scanning if has any pending message in db
                                 *      <c2>: for pending message, add a mms transaction into
                                 *          mProcessing list, and send a send request to mmsservice
                                 *
                                 *   If the bellow sequence happen, the mms will be send twice
                                 *       <a1><a2><c1><b1><b2><c2>
                                 *   Solution:
                                 *    If find pending message in db, also to check mProcessing list
                                 */
                                boolean isInProcessing = false;
                                synchronized (mProcessing) {
                                    for (Transaction t : mProcessing) {
                                        if (t.getUri() != null && ContentUris.parseId(t.getUri())
                                               == cursor.getLong(columnIndexOfMsgId)) {
                                            Log.d(LogTag.TXN_TAG, "The pending message already"
                                                + " in 'mProcessing' list, uri=" + uri);
                                            isInProcessing = true;
                                            break;
                                        }
                                    }
                                }
                                if (isInProcessing == false) {
                                    if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                                        Log.e(LogTag.TXN_TAG, "onNewIntent invalid subId = "
                                                + subId);
                                    } else {
                                        launchTransaction(serviceId, args, noNetwork, subId);
                                    }
                                }
                                break;
                        }
                    }
                } finally {
                    cursor.close();
                }
            } else {
                if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                    Log.v(TAG, "onNewIntent: no pending messages. Stopping service.");
                }
                RetryScheduler.setRetryAlarm(getApplicationContext());
                stopSelfIfIdle(serviceId);
            }
        } else {
            if (!SubscriptionManager.isValidSubscriptionId(transactionSubId) || type == -1) {
                Log.e(LogTag.TXN_TAG, "onNewIntent subId error or type error");
                Log.d(LogTag.TXN_TAG, "stopSelf9->" + serviceId);
                stopSelfIfIdle(serviceId);
                return;
            }
            TransactionBundle args = new TransactionBundle(type, msgUri);
            launchTransaction(serviceId, args, noNetwork, transactionSubId);
        }
    }

    private void stopSelfIfIdle(int startId) {
        Log.d(TAG, "stopSelfIfIdle->startId = " + startId);
        synchronized (mProcessing) {
            Log.d(TAG, "stopSelfIfIdle->mProcessing len = " + mProcessing.size());
            if (mProcessing.isEmpty()) {
                if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                    Log.v(TAG, "stopSelfIfIdle: STOP!");
                }
                Log.d(TAG, "stopSelfIfIdle->this = " + this);
                stopSelf(startId);
            }
        }
    }

    private static boolean isTransientFailure(int type) {
        return type > MmsSms.NO_ERROR && type < MmsSms.ERR_TYPE_GENERIC_PERMANENT;
    }

    private int getTransactionType(int msgType) {
        switch (msgType) {
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                return Transaction.RETRIEVE_TRANSACTION;
            case MtkPduHeaders.MESSAGE_TYPE_READ_REC_IND:
                return Transaction.READREC_TRANSACTION;
            case MtkPduHeaders.MESSAGE_TYPE_SEND_REQ:
                return Transaction.SEND_TRANSACTION;
            default:
                Log.w(TAG, "Unrecognized MESSAGE_TYPE: " + msgType);
                return -1;
        }
    }

    private void launchTransaction(int serviceId, TransactionBundle txnBundle, boolean noNetwork,
            int subId) {
        if (noNetwork) {
            Log.w(TAG, "launchTransaction: no network error!");
            onNetworkUnavailable(serviceId, txnBundle.getTransactionType());
            return;
        }
        Message msg = mServiceHandler.obtainMessage(EVENT_TRANSACTION_REQUEST);
        msg.arg1 = serviceId;
        if (subId <= Long.MAX_VALUE) {
            msg.arg2 = (int) subId & 0xFFFFFFFF;
        } else {
            Log.e(TAG, "launchTransaction: subId is too large, impossible!!!");
        }
        msg.obj = txnBundle;

        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "launchTransaction: sending message " + msg);
        }
        mServiceHandler.sendMessage(msg);
    }

    private void onNetworkUnavailable(int serviceId, int transactionType) {
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "onNetworkUnavailable: sid=" + serviceId + ", type=" + transactionType);
        }
        if (transactionType == Transaction.RETRIEVE_TRANSACTION) {
            MmsConfig.showToast(this, MmsConfig.DOWNLOAD_LATER);
        } else if (transactionType == Transaction.SEND_TRANSACTION) {
            MmsConfig.showToast(this, MmsConfig.MESSAGE_QUEUED);
        }
        Log.d(LogTag.TXN_TAG, "stopSelf2->" + serviceId);
        stopSelf(serviceId);
    }

    @Override
    public void onDestroy() {
        //if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "Destroying TransactionService, this->" + this);
        //}

        releaseWakeLock();

        mServiceHandler.sendEmptyMessage(EVENT_QUIT);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /**
     * Handle status change of Transaction (The Observable).
     */
    public void update(Observable observable) {
        Transaction transaction = (Transaction) observable;
        int serviceId = transaction.getServiceId();

        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.v(TAG, "update transaction " + serviceId);
        }

        try {
            synchronized (mProcessing) {
                mProcessing.remove(transaction);
            }

            TransactionState state = transaction.getState();
            int result = state.getState();

            switch (result) {
                case TransactionState.SUCCESS:
                    Log.d(LogTag.TXN_TAG, "update: result=SUCCESS");
                    if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                        Log.v(TAG, "Transaction complete: " + serviceId);
                    }

                    // Notify user in the system-wide notification area.
                    switch (transaction.getType()) {
                        case Transaction.NOTIFICATION_TRANSACTION:
                        case Transaction.RETRIEVE_TRANSACTION:
                            // We're already in a non-UI thread called from
                            // NotificationTransacation.run(), so ok to block here.
                             long threadId = getThreadId(this, state.getContentUri());
                             MmsConfig.updateMessagingNotification(this, MmsConfig.RECEIEVE_NEW,
                                     threadId, false, state.getContentUri());
                             MmsConfig.updateMessagingNotification(this,
                                     MmsConfig.CANCEL_DOWNLOAD_FAILED, 0, false, null);
                            break;
                        case Transaction.SEND_TRANSACTION:
                            RateController.getInstance().update();
                            break;
                    }
                    break;
                case TransactionState.FAILED:
                    Log.d(LogTag.TXN_TAG, "update: result=FAILED");
                    if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                        Log.v(TAG, "Transaction failed: " + serviceId);
                    }
                    break;
                default:
                    Log.d(LogTag.TXN_TAG, "update: result=default");
                    if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                        Log.v(TAG, "Transaction state unknown: " +
                                serviceId + " " + result);
                    }
                    break;
            }
        } finally {
            transaction.detach(this);
            stopSelfIfIdle(serviceId);
            String uri = getTransactionId(transaction);
            removeTransactionById(uri);
        }
    }

    private synchronized void createWakeLock() {
        // Create a new wake lock if we haven't made one yet.
        if (mWakeLock == null) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "MMS Connectivity");
            mWakeLock.setReferenceCounted(false);
        }
    }

    private void acquireWakeLock() {
        // It's okay to double-acquire this because we are not using it
        // in reference-counted mode.
        Log.d(TAG, "mms acquireWakeLock");
        mWakeLock.acquire();
    }

    private void releaseWakeLock() {
        // Don't release the wake lock if it hasn't been created and acquired.
        if (mWakeLock != null && mWakeLock.isHeld()) {
            Log.d(TAG, "mms releaseWakeLock");
            mWakeLock.release();
        }
    }

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        private String decodeMessage(Message msg) {
            if (msg.what == EVENT_QUIT) {
                return "EVENT_QUIT";
            } else if (msg.what == EVENT_TRANSACTION_PROCESSED) {
                return "EVENT_TRANSACTION_PROCESSED";
            } else if (msg.what == EVENT_TRANSACTION_REQUEST) {
                return "EVENT_TRANSACTION_REQUEST";
            } else if (msg.what == EVENT_NEW_INTENT) {
                return "EVENT_NEW_INTENT";
            }
            return "unknown message.what";
        }

        private String decodeTransactionType(int transactionType) {
            if (transactionType == Transaction.NOTIFICATION_TRANSACTION) {
                return "NOTIFICATION_TRANSACTION";
            } else if (transactionType == Transaction.RETRIEVE_TRANSACTION) {
                return "RETRIEVE_TRANSACTION";
            } else if (transactionType == Transaction.SEND_TRANSACTION) {
                return "SEND_TRANSACTION";
            } else if (transactionType == Transaction.READREC_TRANSACTION) {
                return "READREC_TRANSACTION";
            }
            return "invalid transaction type";
        }

        /**
         * Handle incoming transaction requests.
         * The incoming requests are initiated by the MMSC Server or by the
         * MMS Client itself.
         */
        @Override
        public void handleMessage(Message msg) {
            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.v(TAG, "handleMessage: " + msg + " = " + decodeMessage(msg));
            }

            Transaction transaction = null;

            switch (msg.what) {
                case EVENT_NEW_INTENT:
                    onNewIntent((Intent) msg.obj, msg.arg1);
                    break;

                case EVENT_QUIT:
                    getLooper().quit();
                    return;

                case EVENT_TRANSACTION_PROCESSED:
                    Log.d(LogTag.TXN_TAG, "EVENT_TRANSACTION_PROCESSED");
                    handleTransactionProcessed((Intent) msg.obj, msg.arg1);
                    break;

                case EVENT_PROCESS_TIME_OUT:
                    handleTransactionTimeout((String) msg.obj);
                    // fall through
                case EVENT_TRANSACTION_REQUEST:
                    int serviceId = msg.arg1;
                    int subId = msg.arg2;
                    try {
                        TransactionBundle args = (TransactionBundle) msg.obj;
                        Log.d(LogTag.TXN_TAG, "EVENT_TRANSACTION_REQUEST Url=" +
                                args.getUri() + " subId = " + subId + ", serviceId = " + serviceId);

                        // Set the connection settings for this transaction.
                        // If these have not been set in args, load the default settings.
                        String mmsc = args.getMmscUrl();

                        int transactionType = args.getTransactionType();

                        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                            Log.v(TAG, "handle EVENT_TRANSACTION_REQUEST: transactionType=" +
                                    transactionType + " " + decodeTransactionType(transactionType));
                        }

                        // Create appropriate transaction
                        switch (transactionType) {
                            case Transaction.NOTIFICATION_TRANSACTION:
                                String uri = args.getUri();
                                Log.d(LogTag.TXN_TAG,
                                        "TRANSACTION REQUEST: NOTIFICATION_TRANSACTION, uri="
                                        + uri);
                                if (uri != null) {
                                    transaction = new NotificationTransaction(
                                            TransactionService.this, serviceId,
                                            uri, subId);
                                } else {
                                    // Now it's only used for test purpose.
                                    byte[] pushData = args.getPushData();
                                    MtkPduParser parser = new MtkPduParser(
                                            pushData,
                                            PduParserUtil.shouldParseContentDisposition(subId));
                                    GenericPdu ind = parser.parse();

                                    int type = MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;
                                    if ((ind != null) && (ind.getMessageType() == type)) {
                                        transaction = new NotificationTransaction(
                                                TransactionService.this, serviceId,
                                                (NotificationInd) ind, subId);
                                    } else {
                                        Log.e(TAG, "Invalid PUSH data.");
                                        transaction = null;
                                        Log.d(LogTag.TXN_TAG, "stopSelf10->" + serviceId);
                                        stopSelfIfIdle(serviceId);
                                        return;
                                    }
                                }
                                break;
                            case Transaction.RETRIEVE_TRANSACTION:
                                Log.d(LogTag.TXN_TAG,
                                        "TRANSACTION REQUEST: RETRIEVE_TRANSACTION uri="
                                        + args.getUri());
                                transaction = new RetrieveTransaction(
                                        TransactionService.this, serviceId,
                                        args.getUri(), subId);
                                break;
                            case Transaction.SEND_TRANSACTION:
                                Log.d(LogTag.TXN_TAG, "TRANSACTION REQUEST: SEND_TRANSACTION");
                                transaction = new SendTransaction(
                                        TransactionService.this, serviceId,
                                        mmsc, args.getUri(), subId);
                                break;
                            case Transaction.READREC_TRANSACTION:
                                Log.d(LogTag.TXN_TAG,
                                        "TRANSACTION REQUEST: READREC_TRANSACTION");
                                transaction = new ReadRecTransaction(
                                        TransactionService.this, serviceId,
                                        mmsc, args.getUri(), subId);
                                break;
                            default:
                                Log.w(TAG, "Invalid transaction type: " + serviceId);
                                transaction = null;
                                Log.d(LogTag.TXN_TAG, "stopSelf11->" + serviceId);
                                stopSelfIfIdle(serviceId);
                                return;
                        }

                        if (!processTransaction(transaction)) {
                            transaction = null;
                            return;
                        }

                        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                            Log.v(TAG, "Started processing of incoming message: " + msg);
                        }
                    } catch (Exception ex) {
                        Log.e(TAG, "Exception occurred while handling message: " + msg, ex);

                        if (transaction != null) {
                            try {
                                transaction.detach(TransactionService.this);
                                if (mProcessing.contains(transaction)) {
                                    synchronized (mProcessing) {
                                        mProcessing.remove(transaction);
                                    }
                                }
                            } catch (Throwable t) {
                                Log.e(TAG, "Unexpected Throwable.", t);
                            } finally {
                                // Set transaction to null to allow stopping the
                                // transaction service.
                                transaction = null;
                            }
                        }
                    } finally {
                        if (transaction == null) {
                            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                                Log.v(TAG, "Transaction was null. Stopping self: " + serviceId);
                            }
                            Log.d(LogTag.TXN_TAG, "stopSelf3->" + serviceId);
                            stopSelf(serviceId);
                        }
                    }
                    return;
                default:
                    Log.w(TAG, "what=" + msg.what);
                    return;
            }
        }

        /**
         * Internal method to begin processing a transaction.
         * @param transaction the transaction. Must not be {@code null}.
         * @return {@code true} if process has begun or will begin. {@code false}
         * if the transaction should be discarded.
         * @throws IOException if connectivity for MMS traffic could not be
         * established.
         */
        private boolean processTransaction(Transaction transaction)
                throws IOException {
            Log.i(LogTag.TXN_TAG, "process Transaction");
            // Check if transaction already processing
            synchronized (mProcessing) {
                for (Transaction t : mProcessing) {
                    if (t.isEquivalent(transaction) ||
                            (t.getContentLocation() != null
                                    && transaction.getContentLocation() != null
                                    && t.getContentLocation().equals(
                                            transaction.getContentLocation()))) {
                        Log.i(LogTag.TXN_TAG,
                                "Duplicated transaction: " + transaction.getServiceId());
                        return true;
                    }
                }
            //    Log.i(LogTag.TXN_TAG, "Adding transaction to 'mProcessing' list: "
            //            + transaction);
                mProcessing.add(transaction);
            }

            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.v(TAG, "processTransaction: starting transaction " + transaction);
            }

            // Attach to transaction and process it
            transaction.attach(TransactionService.this);
            transaction.process();
            if (mServiceHandler.hasMessages(EVENT_PROCESS_TIME_OUT)) {
                mServiceHandler.removeMessages(EVENT_PROCESS_TIME_OUT);
            }

            setTransactionById(transaction);
            Message msg = mServiceHandler.obtainMessage(EVENT_PROCESS_TIME_OUT);
            msg.obj = getTransactionId(transaction);
            mServiceHandler.sendMessageDelayed(msg, TRANSACTION_PROCESS_TIME_OUT);
            return true;
        }
    }

    public Map<String, Transaction> sProcessingTxn = new ConcurrentHashMap<String, Transaction>();
    private void setTransactionById(Transaction transaction) {
        String uri = getTransactionId(transaction);
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.d(LogTag.TXN_TAG, "----setTransaction() uri: " + uri
                    + "  sProcessingTxn: " + sProcessingTxn);
        }
        if (uri != null) {
            synchronized (sProcessingTxn) {
                if (uri != null && !sProcessingTxn.containsKey(uri)) {
                    sProcessingTxn.put(uri, transaction);
                }
            }
        }
    }

    private Transaction getTransactionById(String uri) {
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.d(LogTag.TXN_TAG, "----getTransaction() uri: " + uri
                + " sProcessingTxn: " + sProcessingTxn);
        }

        synchronized (sProcessingTxn) {
            if (uri != null && sProcessingTxn.containsKey(uri)) {
                return sProcessingTxn.get(uri);
            }
        }
        return null;
    }

    private void removeTransactionById(String uri) {
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.d(LogTag.TXN_TAG, "----removeTransaction() uri = " + uri
                    + " sProcessingTxn: " + sProcessingTxn);
        }
        synchronized (sProcessingTxn) {
            if (uri != null && sProcessingTxn.containsKey(uri)) {
                sProcessingTxn.remove(uri);
            }
        }
    }

    private String getTransactionId(Transaction transaction) {
        if (transaction == null) {
            Log.e(LogTag.TXN_TAG, "----getTransactionUri() transaction null ");
            return null;
        }
        String uri = transaction.getId();
        if (uri != null) {
            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.d(LogTag.TXN_TAG, "----getTransactionUri() uri: "
                    + uri + " transaction: " + transaction);
            }
        }
        return uri;
    }

    private void handleTransactionTimeout(String uri) {
        if (!mProcessing.isEmpty()) {
            Log.d(LogTag.TXN_TAG, "----handleTransactionTimeout(),  uri = " + uri);
            Transaction transaction = getTransactionById(uri);
            checkTransactionState(transaction);
        }
    }

    private void handleTransactionProcessed(Intent intent, int serviceId) {
        if (intent.getData() == null) {
            Log.e(LogTag.TXN_TAG, "handleTransactionProcessed(), uri is null!");
            return;
        }
        String bundleUri = intent.getData().toString();
        int result = intent.getIntExtra("result", SmsManager.MMS_ERROR_UNSPECIFIED);
        int transactionType = intent.getIntExtra(TransactionBundle.TRANSACTION_TYPE,
                Transaction.SEND_TRANSACTION);
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        Log.d(LogTag.TXN_TAG, "handleTransactionProcessed(), result = " + result
                + ", bundleUri = " + bundleUri + ", TransactionType = " + transactionType
                + ", subId = " + subId);
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Log.e(LogTag.TXN_TAG, "handleTransactionProcessed subId error, subId = " + subId);
        }

        Transaction transaction = null;
        if (mProcessing.isEmpty()) {
            Log.i(LogTag.TXN_TAG, "Processing List Empty");
            try {
                if (transactionType == Transaction.SEND_TRANSACTION) {
                    transaction = new SendTransaction(
                            TransactionService.this, serviceId,
                            null, bundleUri, subId);
                } else if (transactionType == Transaction.NOTIFICATION_TRANSACTION) {
                    String pduFileStr = intent.getStringExtra("pduFile");
                    Log.i(LogTag.TXN_TAG, " pduFileStr = " + pduFileStr);
                    if (pduFileStr == null) {
                        Log.e(LogTag.TXN_TAG, "NOTIFICATION_TRANSACTION, pduFileStr null");
                        Log.d(LogTag.TXN_TAG, "stopSelf6->" + serviceId);
                        stopSelfIfIdle(serviceId);
                        return;
                    }
                    File pduFile = new File(pduFileStr);
                    transaction = new NotificationTransaction(
                            TransactionService.this, serviceId,
                            bundleUri, subId);
                    transaction.setPduFile(pduFile);
                } else if (transactionType == Transaction.RETRIEVE_TRANSACTION) {
                    String retrivePduFileStr = intent.getStringExtra("pduFile");
                    Log.i(LogTag.TXN_TAG, " retrivePduFileStr = " + retrivePduFileStr);
                    if (retrivePduFileStr == null) {
                        Log.e(LogTag.TXN_TAG, "RETRIEVE_TRANSACTION, retrivePduFileStr null");
                        Log.d(LogTag.TXN_TAG, "stopSelf7->" + serviceId);
                        stopSelfIfIdle(serviceId);
                        return;
                    }
                    File retrivePduFile = new File(retrivePduFileStr);
                    transaction = new RetrieveTransaction(
                            TransactionService.this, serviceId,
                            bundleUri, subId);
                    transaction.setPduFile(retrivePduFile);
                } else if (transactionType == Transaction.READREC_TRANSACTION) {
                    transaction = new ReadRecTransaction(
                            TransactionService.this, serviceId,
                            null, bundleUri, subId);
                }
                transaction.getState().setContentUri(Uri.parse(bundleUri));
            } catch (Exception ex) {
                Log.e(TAG, "Exception occurred while new transaction: ", ex);
                if (transaction == null) {
                    Log.d(LogTag.TXN_TAG, "stopSelf4->" + serviceId);
                    stopSelfIfIdle(serviceId);
                    return;
                }
            }
        } else {
            transaction = getTransactionById(bundleUri);
            Log.d(LogTag.TXN_TAG, "transaction = " + transaction);
            if (transaction != null && transaction.getUri() != null) {
                transaction.getState().setContentUri(transaction.getUri());
            } else {
                Log.w(LogTag.TXN_TAG, "handleTransactionProcessed(), uri not match!");
                if (transaction == null) {
                    Log.w(LogTag.TXN_TAG, "handleTransactionProcessed(), null transaction!");
                    Log.d(LogTag.TXN_TAG, "stopSelf8->" + serviceId);
                    stopSelfIfIdle(serviceId);
                    return;
                }
            }
        }

        if (result == Activity.RESULT_OK) {
            if (transaction instanceof NotificationTransaction) {
                result = ((NotificationTransaction) transaction).checkPduResult();
            } else if (transaction instanceof RetrieveTransaction) {
                result = ((RetrieveTransaction) transaction).checkPduResult();
            } else if (transaction instanceof SendTransaction) {
                result = ((SendTransaction) transaction).checkSendResult(
                        intent.getByteArrayExtra(SmsManager.EXTRA_MMS_DATA));
            }
        }

        if (transaction instanceof ReadRecTransaction) {
            result = ((ReadRecTransaction) transaction).checkSendResult(
                    intent.getByteArrayExtra(SmsManager.EXTRA_MMS_DATA));
        }
        int slotId = SubscriptionManager.getSlotIndex(transaction.mSubId);
        boolean isSubInserted = slotId >= 0
                && slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        final CellConnMgr cellConnMgr = new CellConnMgr(getApplicationContext());
        final int state = cellConnMgr.getCurrentState(transaction.mSubId,
                CellConnMgr.STATE_FLIGHT_MODE | CellConnMgr.STATE_SIM_LOCKED
                        | CellConnMgr.STATE_RADIO_OFF);
        boolean subDisabled = (!isSubInserted)
                || ((state & CellConnMgr.STATE_FLIGHT_MODE)
                        == CellConnMgr.STATE_FLIGHT_MODE)
                || ((state & CellConnMgr.STATE_RADIO_OFF) == CellConnMgr.STATE_RADIO_OFF)
                || ((state & CellConnMgr.STATE_SIM_LOCKED) == CellConnMgr.STATE_SIM_LOCKED);

        if (result == Activity.RESULT_OK) {
            transaction.getState().setState(TransactionState.SUCCESS);
            setTransactionStatus(intent, transaction);
        } else if (subDisabled) {
            setTransactionFail(transaction, FAILE_TYPE_PERMANENT);
        } else {
            switch (result) {
                case SmsManager.MMS_ERROR_UNSPECIFIED:
                    setTransactionFail(transaction, FAILE_TYPE_PERMANENT);
                    break;
                case SmsManager.MMS_ERROR_INVALID_APN:
                    setTransactionFail(transaction, FAILE_TYPE_PERMANENT);
                    break;
                case SmsManager.MMS_ERROR_IO_ERROR:
                    setTransactionFail(transaction, FAILE_TYPE_TEMPORARY);
                    break;
                case SmsManager.MMS_ERROR_HTTP_FAILURE:
                case SmsManager.MMS_ERROR_UNABLE_CONNECT_MMS:
                case SmsManager.MMS_ERROR_CONFIGURATION_ERROR:
                    //add for sync
                    setTransactionFail(transaction, FAILE_TYPE_TEMPORARY);
                    if (isSendOrRetrieveTransaction(transaction)) {
                        boolean incall = isDuringCallForCurrentSim(subId);
                        Log.d(TAG, "incall? " + incall);
                        if (incall) {
                            setTransactionFail(transaction, FAILE_TYPE_RESTAIN_RETRY);
                        } else {
                            setTransactionFail(transaction, FAILE_TYPE_TEMPORARY);
                        }
                    } else {
                        setTransactionFail(transaction, FAILE_TYPE_TEMPORARY);
                    }
                    break;
                default:
                    Log.e(LogTag.TXN_TAG, "Unknown Error type");
                    setTransactionFail(transaction, FAILE_TYPE_PERMANENT);
                    break;
            }
        }
        transaction.notifyObservers();
        /* Here should stopSelf with serviceId*/
        Log.d(LogTag.TXN_TAG, "stopSelf5->" + serviceId + ", this->" + this);
        stopSelfIfIdle(serviceId);
    }

    private void setTransactionStatus(Intent intent, Transaction txn) {
        if (txn instanceof NotificationTransaction || txn instanceof RetrieveTransaction) {
            String newUri = null;
            if (txn instanceof NotificationTransaction) {
                newUri = ((NotificationTransaction) txn).getContentUri();
            } else if (txn instanceof RetrieveTransaction) {
                newUri = ((RetrieveTransaction) txn).getContentUri();
            }
            int result = intent.getIntExtra("result", SmsManager.MMS_ERROR_UNSPECIFIED);
            if (newUri != null) {
                txn.getState().setContentUri(Uri.parse(newUri));
            }

            if (txn instanceof NotificationTransaction) {
                if (result == Activity.RESULT_OK) {
                    ((NotificationTransaction) txn).sendNotifyRespInd(
                        MtkPduHeaders.STATUS_RETRIEVED);
                } else {
                    ((NotificationTransaction) txn).sendNotifyRespInd(
                        MtkPduHeaders.STATUS_DEFERRED);
                }
            } else if (txn instanceof RetrieveTransaction) {
                if ((result == Activity.RESULT_OK)
                        && newUri != null) {
                    ((RetrieveTransaction) txn).sendAcknowledgeInd();
                }
            }

        }
    }

    private void checkTransactionState(Transaction txn) {
        Uri uri = null;
        Intent intent = new Intent(ACTION_TRANSACION_PROCESSED);
        int result = SmsManager.MMS_ERROR_UNSPECIFIED;
        if (txn instanceof SendTransaction) {
            Log.d(LogTag.TXN_TAG, "checkTransactionState. :Send");
            uri = ((SendTransaction) txn).getUri();
            Cursor cursor = null;
            try {
                cursor = SqliteWrapper.query(getApplicationContext(), getApplicationContext()
                        .getContentResolver(), uri, new String[] { Mms.MESSAGE_BOX },
                        null, null, null);
                if (cursor != null && cursor.getCount() == 1) {
                    cursor.moveToFirst();
                    int msgBox = cursor.getInt(0);
                    if (msgBox == Mms.MESSAGE_BOX_SENT) {
                        result = Activity.RESULT_OK;
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }

        } else if (txn instanceof NotificationTransaction || txn instanceof RetrieveTransaction) {
            Log.d(LogTag.TXN_TAG, "checkTransactionState. :Notification/Retrieve");
            if (txn instanceof NotificationTransaction) {
                uri = ((NotificationTransaction) txn).getUri();
            } else {
                uri = ((RetrieveTransaction) txn).getUri();
            }
            Cursor cursor = null;
            try {
                cursor = SqliteWrapper.query(getApplicationContext(), getApplicationContext()
                        .getContentResolver(), uri, new String[] { Mms.MESSAGE_BOX },
                        null, null, null);
                if (cursor == null || cursor.getCount() == 0) {
                    result = Activity.RESULT_OK;
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        } else if (txn instanceof ReadRecTransaction) {
            Log.d(LogTag.TXN_TAG, "set Transaction success. :ReadRec");
            uri = ((ReadRecTransaction) txn).getUri();
            result = Activity.RESULT_OK;
        } else {
            Log.d(LogTag.TXN_TAG, "checkTransactionState. type cann't be recognised");
            return;
        }
        intent.putExtra(TransactionBundle.URI, uri.toString());
        intent.putExtra("result", result);
        handleTransactionProcessed(intent, 0);
    }

    private void setTransactionFail(Transaction txn, int failType) {
        Log.i(LogTag.TXN_TAG, "set Transaction Fail. fail Type=" + failType);

        long msgId = 0;
        Uri uri = null;
        if (txn instanceof SendTransaction) {
            Log.d(LogTag.TXN_TAG, "set Transaction Fail. :Send");
            uri = ((SendTransaction) txn).getUri();
        } else if (txn instanceof NotificationTransaction) {
            Log.d(LogTag.TXN_TAG, "set Transaction Fail. :Notification");
            uri = ((NotificationTransaction) txn).getUri();
        } else if (txn instanceof RetrieveTransaction) {
            Log.d(LogTag.TXN_TAG, "set Transaction Fail. :Retrieve");
            uri = ((RetrieveTransaction) txn).getUri();
        } else if (txn instanceof ReadRecTransaction) {
            Log.d(LogTag.TXN_TAG, "set Transaction Fail. :ReadRec");
            uri = ((ReadRecTransaction) txn).getUri();
            // add this for read report.
            // if the read report is failed to open connection.mark it
            // sent(129).i.e. only try to send once.
            // [or mark 128, this is another policy, this will resend next time
            // into UI and out.]
            ContentValues values = new ContentValues(1);
            values.put(Mms.READ_REPORT, 129);
            SqliteWrapper.update(getApplicationContext(), getApplicationContext()
                    .getContentResolver(), uri, values, null, null);
            txn.mTransactionState.setState(TransactionState.FAILED);
            txn.mTransactionState.setContentUri(uri);
            return;
        } else {
            Log.d(LogTag.TXN_TAG, "set Transaction Fail. type cann't be recognised");
        }

        if (null != uri) {
            txn.mTransactionState.setContentUri(uri);
            msgId = ContentUris.parseId(uri);
        } else {
            Log.e(LogTag.TXN_TAG, "set Transaction Fail. uri is null.");
            return;
        }

        if (txn instanceof NotificationTransaction) {
            boolean autoDownload = MmsConfig.isAutoDownload(txn.mSubId);

            if (!autoDownload) {
                txn.mTransactionState.setState(TransactionState.SUCCESS);
            } else {
                txn.mTransactionState.setState(TransactionState.FAILED);
            }
        } else {
            txn.mTransactionState.setState(TransactionState.FAILED);
        }

        Uri.Builder uriBuilder = PendingMessages.CONTENT_URI.buildUpon();
        uriBuilder.appendQueryParameter("protocol", "mms");
        uriBuilder.appendQueryParameter("message", String.valueOf(msgId));

        Cursor cursor = SqliteWrapper.query(getApplicationContext(), getApplicationContext()
                .getContentResolver(), uriBuilder.build(), null, null, null, null);

        if (cursor != null) {
            try {
                if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                    DefaultRetryScheme scheme =
                        new DefaultRetryScheme(getApplicationContext(), 100);

                    ContentValues values = null;
                    if (FAILE_TYPE_PERMANENT == failType) {
                        values = new ContentValues(2);
                        values.put(PendingMessages.ERROR_TYPE, MmsSms.ERR_TYPE_GENERIC_PERMANENT);
                        values.put(PendingMessages.RETRY_INDEX, scheme.getRetryLimit());

                        int columnIndex = cursor.getColumnIndexOrThrow(PendingMessages._ID);
                        long id = cursor.getLong(columnIndex);

                        SqliteWrapper.update(getApplicationContext(), getApplicationContext()
                                .getContentResolver(), PendingMessages.CONTENT_URI, values,
                                PendingMessages._ID + "=" + id, null);
                    } else if (FAILE_TYPE_RESTAIN_RETRY == failType) {
                        int retryIndex = cursor.getInt(cursor
                                .getColumnIndexOrThrow(PendingMessages.RETRY_INDEX));
                        if (retryIndex > 0) {
                            retryIndex--;
                        }
                        Log.d(TAG, "failType = 3, retryIndex = " + retryIndex);

                        ContentValues valuesRe = new ContentValues(1);
                        valuesRe.put(PendingMessages.RETRY_INDEX, retryIndex);
                        int columnIndexRe = cursor.getColumnIndexOrThrow(PendingMessages._ID);
                        long idRe = cursor.getLong(columnIndexRe);
                        SqliteWrapper.update(getApplicationContext(),
                                getApplicationContext().getContentResolver(),
                                PendingMessages.CONTENT_URI, valuesRe,
                                PendingMessages._ID + "=" + idRe, null);
                    }
                }
            } finally {
                cursor.close();
            }
        }

    }

    private boolean isDuringCall() {
        int[] subIds =
            SubscriptionManager.from(this.getApplication()).getActiveSubscriptionIdList();
        TelephonyManager telephonyManager = (TelephonyManager) getApplicationContext()
                        .getSystemService(Context.TELEPHONY_SERVICE);
        for (int subId : subIds) {
            if (!(telephonyManager.getCallState(subId) == TelephonyManager.CALL_STATE_IDLE)) {
                return true;
            }
        }
        return false;
    }

    public boolean isSendOrRetrieveTransaction(Object transaction) {
        return transaction instanceof SendTransaction || transaction instanceof RetrieveTransaction;
    }

    private static final String[] MMS_THREAD_ID_PROJECTION = new String[] { Mms.THREAD_ID };

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
           return MmsConfig.THREAD_NONE;
       }

       try {
           if (cursor.moveToFirst()) {
               int columnIndex = cursor.getColumnIndex(Mms.THREAD_ID);
               if (columnIndex < 0) {
                   return MmsConfig.THREAD_NONE;
               }
               long threadId = cursor.getLong(columnIndex);
               return threadId;
           } else {
               return MmsConfig.THREAD_NONE;
           }
       } finally {
           cursor.close();
       }
    }

    /// M: ALPS00545779, for FT, restart pending receiving mms @ {
    /**
     * isPendingMmsNeedRestart.
     * @param pduUri Uri
     * @param failureType int
     * @return true if restart.
     */
    private boolean isPendingMmsNeedRestart(Uri pduUri, int failureType) {
        Log.d(TAG, "isPendingMmsNeedRestart, uri=" + pduUri);

        final int pduColumnStatus = 2;
        final String[] pduProjection = new String[] {
            Mms.MESSAGE_BOX,
            Mms.MESSAGE_ID,
            Mms.STATUS,
        };
        Cursor c = null;
        ContentResolver contentResolver = getApplicationContext().getContentResolver();

        try {
            c = contentResolver.query(pduUri, pduProjection, null, null, null);

            if ((c == null) || (c.getCount() != 1) || !c.moveToFirst()) {
                Log.d(TAG, "Bad uri");
                return true;
            }

            int status = c.getInt(pduColumnStatus);
            Log.v(TAG, "status" + status);

            /* This notification is not processed yet, so need restart*/
            if (status == 0) {
                return true;
            }
            /* DEFERRED_MASK is not set, it is auto download*/
            if ((status & 0x04) == 0) {
                return isTransientFailure(failureType);
            }
            /* Reach here means it is manully download*/
            return false;
        } catch (SQLiteException e) {
            Log.e(TAG, "Catch a SQLiteException when query: ", e);
            return isTransientFailure(failureType);
        } finally {
            if (c != null) {
                c.close();
            }
        }
    }
    /// @}

    /* M:Code analyze 004,add for ALPS00081452,check whether the request data connection fail
     * is caused by calling going on. @{
     */
    private boolean isDuringCallForCurrentSim(int subId) {
        TelephonyManager teleManager =
                                    (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        int phoneState = TelephonyManager.CALL_STATE_IDLE;
        if (teleManager != null) {
            phoneState = teleManager.getCallState(subId);
        }
        return phoneState != TelephonyManager.CALL_STATE_IDLE;
    }
}
