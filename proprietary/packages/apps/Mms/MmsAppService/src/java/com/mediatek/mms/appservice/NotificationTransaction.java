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

import static com.mediatek.mms.appservice.TransactionState.FAILED;
import static com.mediatek.mms.appservice.TransactionState.SUCCESS;
import static com.mediatek.android.mms.pdu.MtkPduHeaders.STATUS_DEFERRED;

import android.app.Activity;
import android.app.ActivityThread;
import android.app.PendingIntent;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.provider.Telephony;
import android.provider.Telephony.Mms.Inbox;
import android.support.v4.content.FileProvider;
import android.telephony.SmsManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.google.android.mms.InvalidHeaderValueException;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.NotifyRespInd;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.RetrieveConf;
import com.mediatek.android.mms.pdu.MtkPduComposer;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduParser;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.android.mms.pdu.MtkRetrieveConf;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
/**
 * The NotificationTransaction is responsible for handling multimedia
 * message notifications (M-Notification.ind).  It:
 *
 * <ul>
 * <li>Composes the notification response (M-NotifyResp.ind).
 * <li>Sends the notification response to the MMSC server.
 * <li>Stores the notification indication.
 * <li>Notifies the TransactionService about succesful completion.
 * </ul>
 *
 * NOTE: This MMS client handles all notifications with a <b>deferred
 * retrieval</b> response.  The transaction service, upon succesful
 * completion of this transaction, will trigger a retrieve transaction
 * in case the client is in immediate retrieve mode.
 */
public class NotificationTransaction extends Transaction implements Runnable {
    private static final String TAG = LogTag.TAG;
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;
    private static final String NOTIFY_RESP_NAME = "NotifyResp_noti";
    private static final String RETRIEVE_RESULT_NAME = "RetrieveResult_noti";

    private NotificationInd mNotificationInd;
    private String mMessageUri = null;

    private static final String LOCATION_SELECTION =
        Telephony.Mms.MESSAGE_TYPE + "=? AND " + Telephony.Mms.CONTENT_LOCATION + " =?";

    public NotificationTransaction(
            Context context,
            int serviceId,
            String uriString,
            int subId) {
        super(context, serviceId, subId);

        mUri = Uri.parse(uriString);

        try {
            mNotificationInd = (NotificationInd)
                    MtkPduPersister.getPduPersister(context.getApplicationContext()).load(mUri);
        } catch (MmsException e) {
            Log.e(TAG, "Failed to load NotificationInd from: " + uriString, e);
            throw new IllegalArgumentException();
        }

        mContentLocation = new String(mNotificationInd.getContentLocation());
        mId = uriString;

        // Attach the transaction to the instance of RetryScheduler.
        attach(RetryScheduler.getInstance(context.getApplicationContext()));
    }

    /**
     * This constructor is only used for test purposes.
     */
    public NotificationTransaction(
            Context context, int serviceId,
            NotificationInd ind,
            int subId) {
        super(context, serviceId, subId);

        try {
            // Save the pdu. If we can start downloading the real pdu immediately, don't allow
            // persist() to create a thread for the notificationInd because it causes UI jank.
            mUri = MtkPduPersister.getPduPersister(context.getApplicationContext()).persist(
                        ind, Inbox.CONTENT_URI, !allowAutoDownload(context, subId),
                        MmsConfig.getIsGroupMmsEnabled(), null);
        } catch (MmsException e) {
            Log.e(TAG, "Failed to save NotificationInd in constructor.", e);
            throw new IllegalArgumentException();
        }

        mNotificationInd = ind;
        mId = new String(mNotificationInd.getContentLocation());
    }

    /*
     * (non-Javadoc)
     * @see com.google.android.mms.pdu.Transaction#process()
     */
    @Override
    public void process() {
        sExecutor.execute(this);
    }

    public static boolean allowAutoDownload(Context context, int subId) {
        boolean autoDownload = MmsConfig.isAutoDownload(subId);
        boolean dataSuspended = false;
        return autoDownload && !dataSuspended;
    }

    public void run() {
        Log.d(LogTag.TXN_TAG, "NotificationTransaction: run");
        DownloadManager downloadManager = DownloadManager.getInstance();
        boolean autoDownload = allowAutoDownload(mContext, mSubId);
        try {
            if (LOCAL_LOGV) {
                Log.v(TAG, "Notification transaction launched: " + this);
            }

            // By default, we set status to STATUS_DEFERRED because we
            // should response MMSC with STATUS_DEFERRED when we cannot
            // download a MM immediately.
            int status = STATUS_DEFERRED;
            // Don't try to download when data is suspended, as it will fail, so defer download
            if (!autoDownload) {
                // M: change API for ALPS01889178, use sub id.
                downloadManager.markState(mUri, DownloadManager.STATE_UNSTARTED, mSubId);
                sendNotifyRespInd(status);
                getState().setState(SUCCESS);
                getState().setContentUri(mUri);
                notifyObservers();
                return;
            }

            // M: change API for ALPS01889178, use sub id.
            downloadManager.markState(mUri, DownloadManager.STATE_DOWNLOADING, mSubId);

            if (LOCAL_LOGV) {
                Log.v(TAG, "Content-Location: " + mContentLocation);
            }
            mPduFile = createPduFile(null, RETRIEVE_RESULT_NAME + mUri.getLastPathSegment());
            mPduFile.setWritable(true, false);

            Log.d(LogTag.TXN_TAG, "NotificationTransaction mUri:" + mUri);
            final Intent intent = new Intent(TransactionService.ACTION_TRANSACION_PROCESSED,
                    mUri,
                    mContext,
                    MmsReceiver.class);
            intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
            intent.putExtra(TransactionBundle.TRANSACTION_TYPE, NOTIFICATION_TRANSACTION);
            intent.putExtra("pduFile", mPduFile.getAbsolutePath());
            PendingIntent downloadedIntent = PendingIntent.getBroadcast(mContext, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);

            SmsManager manager = SmsManager.getSmsManagerForSubscriptionId(mSubId);
            MmsLog.dpi(LogTag.TXN_TAG, "download MMS with param, mContentLocation = " + mContentLocation
                    + ", mUri = " + mUri + ", subId" + mSubId);

            /// M: Add MmsService configure param @{
            Uri pduFileUri = FileProvider.getUriForFile(
                    mContext, MMS_FILE_PROVIDER_AUTHORITIES, mPduFile);
            manager.downloadMultimediaMessage(mContext, mContentLocation, pduFileUri,
                    MmsConfig.getMmsServiceConfig(), downloadedIntent);
            /// @}

            // sendNotifyRespInd(status);

            // Make sure this thread isn't over the limits in message count.
            Recycler.getMmsRecycler().deleteOldMessagesInSameThreadAsMessage(mContext, mUri);
        //    MmsWidgetProvider.notifyDatasetChanged(mContext);
        } catch (Throwable t) {
            getState().setState(FAILED);
            getState().setContentUri(mUri);
            notifyObservers();
            Log.e(TAG, Log.getStackTraceString(t));
        }
    }

    public void sendNotifyRespInd(int status) {
        Log.i(LogTag.TXN_TAG, "NotificationTransaction: sendNotifyRespInd()");
        // Create the M-NotifyResp.ind
        NotifyRespInd notifyRespInd = null;
        try {
            notifyRespInd = new NotifyRespInd(MtkPduHeaders.CURRENT_MMS_VERSION,
                    mNotificationInd.getTransactionId(), status);
        } catch (InvalidHeaderValueException ex) {
            ex.printStackTrace();
            return;
        }

        /// M:Code analyze 014, this paragraph below is using for judging if it is allowed
        /// to send delivery report,at present,we don't support delivery report in MMS @{
      //  mOpNotificationTransactionExt.sendNotifyRespInd(mContext, mSubId, notifyRespInd);
        boolean reportAllowed = MmsConfig.isEnableSendDeliveryReport(mSubId);

        try {
            notifyRespInd.setReportAllowed(reportAllowed ? PduHeaders.VALUE_YES
                    : PduHeaders.VALUE_NO);
        } catch (InvalidHeaderValueException ihve) {
            // do nothing here
            Log.e(TAG, "notifyRespInd.setReportAllowed Failed !!");
        }

        byte[] datas = new MtkPduComposer(mContext, notifyRespInd).make();
        File pduFile = createPduFile(datas, NOTIFY_RESP_NAME + mUri.getLastPathSegment());
        if (pduFile == null) {
            return;
        }

        SmsManager manager = SmsManager.getSmsManagerForSubscriptionId(mSubId);

        // Pack M-NotifyResp.ind and send it
        Uri pduFileUri = FileProvider.getUriForFile(
                mContext, MMS_FILE_PROVIDER_AUTHORITIES, pduFile);
        if (MmsConfig.getNotifyWapMMSC()) {
            manager.sendMultimediaMessage(mContext, pduFileUri, mContentLocation,
                    null, null);
        } else {
            manager.sendMultimediaMessage(mContext, pduFileUri, null, null, null);
        }
//         if (pduFile != null) {
//             pduFile.delete();
//         }
    }

    public int checkPduResult() {
        if (!mPduFile.exists()) {
            Log.e(LogTag.TXN_TAG, "Fail, no pduFile = " + mPduFile);
            return SmsManager.MMS_ERROR_UNSPECIFIED;
        }
        FileChannel channel = null;
        FileInputStream fs = null;
        //MtkRetrieveConf retrieveConf;
        try {
            fs = new FileInputStream(mPduFile);
            channel = fs.getChannel();
            ByteBuffer byteBuffer = ByteBuffer.allocate((int) channel.size());
            while ((channel.read(byteBuffer)) > 0) {
                // do nothing
                // System.out.println("reading");
            }
            final GenericPdu pdu = (new MtkPduParser(byteBuffer.array(),
                    PduParserUtil.shouldParseContentDisposition(mSubId))).parse();
            if (pdu == null || !(pdu instanceof RetrieveConf)) {
                Log.e(LogTag.TXN_TAG, "checkPduResult: invalid parsed PDU");
                return SmsManager.MMS_ERROR_UNSPECIFIED;
            }
            //retrieveConf = (MtkRetrieveConf) pdu;

            // Store the downloaded message
            MtkPduPersister persister = MtkPduPersister.getPduPersister(mContext.getApplicationContext());
            Uri messageUri = persister.persist(
                    pdu,
                    Telephony.Mms.Inbox.CONTENT_URI,
                    true/*createThreadId*/,
                    true/*groupMmsEnabled*/,
                    null/*preOpenedFiles*/,
                    mContentLocation);
            if (messageUri == null) {
                Log.e(LogTag.TXN_TAG, "checkPduResult: can not persist message");
                return SmsManager.MMS_ERROR_UNSPECIFIED;
            }
            mMessageUri = messageUri.toString();
            // Update some of the properties of the message
            final ContentValues values = new ContentValues();
            values.put(Telephony.Mms.DATE, System.currentTimeMillis() / 1000L);
            String creator = ActivityThread.currentPackageName();
            if (!TextUtils.isEmpty(creator)) {
                values.put(Telephony.Mms.CREATOR, creator);
            }
            values.put(Telephony.Mms.SUBSCRIPTION_ID, mSubId);
            if (SqliteWrapper.update(
                    mContext,
                    mContext.getContentResolver(),
                    messageUri,
                    values,
                    null/*where*/,
                    null/*selectionArg*/) != 1) {
                Log.e(LogTag.TXN_TAG, "persistIfRequired: can not update message");
            }
            return Activity.RESULT_OK;
        } catch (IOException e) {
            e.printStackTrace();
            return SmsManager.MMS_ERROR_UNSPECIFIED;
        } catch (MmsException e) {
            e.printStackTrace();
            return SmsManager.MMS_ERROR_UNSPECIFIED;
        } catch (SQLiteException e) {
            e.printStackTrace();
            return SmsManager.MMS_ERROR_UNSPECIFIED;
        } catch (RuntimeException e) {
            e.printStackTrace();
            return SmsManager.MMS_ERROR_UNSPECIFIED;
        } finally {
            if (mPduFile != null) {
                mPduFile.delete();
            }
            try {
                if (channel != null) {
                    channel.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                if (fs != null) {
                    fs.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public int getType() {
        return NOTIFICATION_TRANSACTION;
    }

    public String getContentUri() {
        return mMessageUri;
    }
}
