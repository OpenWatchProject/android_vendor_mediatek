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
import android.app.PendingIntent;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.provider.Telephony;
import android.support.v4.content.FileProvider;
import android.telephony.SmsManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.android.mms.pdu.MtkPduComposer;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduParser;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.android.mms.pdu.MtkSendConf;
import com.mediatek.android.mms.pdu.MtkSendReq;
import com.google.android.mms.pdu.SendReq;
/**
 * The SendTransaction is responsible for sending multimedia messages
 * (M-Send.req) to the MMSC server.  It:
 *
 * <ul>
 * <li>Loads the multimedia message from storage (Outbox).
 * <li>Packs M-Send.req and sends it.
 * <li>Retrieves confirmation data from the server  (M-Send.conf).
 * <li>Parses confirmation message and handles it.
 * <li>Moves sent multimedia message from Outbox to Sent.
 * <li>Notifies the TransactionService about successful completion.
 * </ul>
 */
public class SendTransaction extends Transaction implements Runnable {
    private static final String TAG = LogTag.TAG;

    private String mMmscUrl;
    private static final String SEND_REQ_NAME = "MtkSendReq_retr";

    public SendTransaction(Context context, int transId, String mmscUrl, String uri, int subId) {
        super(context, transId, subId);
        mUri = Uri.parse(uri);
        mMmscUrl = mmscUrl;
        mId = uri;

        // Attach the transaction to the instance of RetryScheduler.
        attach(RetryScheduler.getInstance(context.getApplicationContext()));
    }

    /*
     * (non-Javadoc)
     * @see com.android.mms.Transaction#process()
     */
    @Override
    public void process() {
        sExecutor.execute(this);
    }

    public void run() {
        Log.d(LogTag.TXN_TAG, "SendTransaction: run");
        RateController rateCtlr = RateController.getInstance();
        if (rateCtlr.isLimitSurpassed() && !rateCtlr.isAllowedByUser()) {
            Log.e(TAG, "Sending rate limit surpassed.");
            return;
        }

        try {
            final MtkPduPersister persister = MtkPduPersister.getPduPersister(mContext.getApplicationContext());
            final SendReq sendReq =  (SendReq) persister.load(mUri);
            if (sendReq == null) {
                Log.w(LogTag.TXN_TAG, "Send req is null!");
                return;
            }
            byte[] datas = new MtkPduComposer(mContext, sendReq).make();
            mPduFile = createPduFile(datas, SEND_REQ_NAME + mUri.getLastPathSegment());
            if (mPduFile == null) {
                Log.w(LogTag.TXN_TAG, "create pdu file req failed!");
                return;
            }

            //Intent intent = new Intent(TransactionService.ACTION_TRANSACION_PROCESSED);
            //intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
            //intent.putExtra(TransactionBundle.URI, mUri.toString());
            Log.d(LogTag.TXN_TAG, "SendTransaction mUri:" + mUri);
            final Intent intent = new Intent(TransactionService.ACTION_TRANSACION_PROCESSED,
                    mUri,
                    mContext,
                    MmsReceiver.class);
            intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
            intent.putExtra(TransactionBundle.TRANSACTION_TYPE, SEND_TRANSACTION);
            PendingIntent sentIntent = PendingIntent.getBroadcast(mContext, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Log.i(LogTag.TXN_TAG, "send MMS with param, mUri = "
            + mUri + " mPdufile = " + mPduFile + ", subId = " + mSubId);
            Uri pduFileUri = FileProvider.getUriForFile(
                    mContext, MMS_FILE_PROVIDER_AUTHORITIES, mPduFile);
            /// M: Add MmsService configure param @{
            SmsManager.getSmsManagerForSubscriptionId(mSubId).sendMultimediaMessage(mContext,
                    pduFileUri, null,
                    MmsConfig.getMmsServiceConfig(), sentIntent);
        /// @}
        } catch (Throwable t) {
            Log.e(TAG, Log.getStackTraceString(t));
            getState().setState(TransactionState.FAILED);
            getState().setContentUri(mUri);
            notifyObservers();
        }
    }

    @Override
    public int getType() {
        return SEND_TRANSACTION;
    }

    public int checkSendResult(byte[] response) {
        try {
            int result = Activity.RESULT_OK;
            if (response == null) {
                Log.d(LogTag.TXN_TAG,
                        "checkSendResult:get response is null, maybe is not a normal sending");
                result = SmsManager.MMS_ERROR_UNSPECIFIED;
            }
            boolean supportContentDisposition = PduParserUtil.shouldParseContentDisposition(mSubId);
            MtkSendConf conf = (MtkSendConf) new MtkPduParser(
                                response, supportContentDisposition).parse();
            if (conf == null) {
                Log.d(LogTag.TXN_TAG, "checkSendResult:No M-Send.conf received.");
                result = SmsManager.MMS_ERROR_UNSPECIFIED;
            } else {
                int respStatus = conf.getResponseStatus();
                Log.d(LogTag.TXN_TAG, "checkSendResult get response  = " + respStatus);
                if (respStatus == MtkPduHeaders.RESPONSE_STATUS_OK) {
                    result = Activity.RESULT_OK;
                } else {
                    result = SmsManager.MMS_ERROR_UNSPECIFIED;
                }
            }

            int messageStatus = result == Activity.RESULT_OK ?
                    Telephony.Mms.MESSAGE_BOX_SENT : Telephony.Mms.MESSAGE_BOX_FAILED;
            final ContentValues values = new ContentValues(3);
            values.put(Telephony.Mms.MESSAGE_BOX, messageStatus);
            if (conf != null) {
                values.put(Telephony.Mms.RESPONSE_STATUS, conf.getResponseStatus());
                values.put(Telephony.Mms.MESSAGE_ID,
                        MtkPduPersister.toIsoString(conf.getMessageId()));
            }
            SqliteWrapper.update(mContext, mContext.getContentResolver(), mUri, values,
                    null, null);
            return result;
        } catch (SQLiteException e) {
            Log.e(LogTag.TXN_TAG, "MtkSendRequest.updateStatus: can not update message", e);
            return SmsManager.MMS_ERROR_UNSPECIFIED;
        } catch (RuntimeException e) {
            Log.e(LogTag.TXN_TAG, "MtkSendRequest.updateStatus: can not parse response", e);
            return SmsManager.MMS_ERROR_UNSPECIFIED;
        } finally {
            if (mPduFile != null) {
                mPduFile.delete();
            }
        }
    }
}
