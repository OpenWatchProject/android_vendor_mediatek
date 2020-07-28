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

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony.Mms;
import android.support.v4.content.FileProvider;
import android.telephony.SmsManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.google.android.mms.InvalidHeaderValueException;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.EncodedStringValue;
import com.mediatek.android.mms.pdu.MtkPduComposer;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.google.android.mms.pdu.ReadRecInd;
import com.google.android.mms.util.SqliteWrapper;

/**
 * The ReadRecTransaction is responsible for sending read report
 * notifications (M-read-rec.ind) to clients that have requested them.
 * It:
 *
 * <ul>
 * <li>Loads the read report indication from storage (Outbox).
 * <li>Packs M-read-rec.ind and sends it.
 * <li>Notifies the TransactionService about succesful completion.
 * </ul>
 */
public class ReadRecTransaction extends Transaction implements Runnable {
    private static final String TAG = LogTag.TAG;
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;
    private static final String READREP_REQ_NAME = "ReadrepReq_retr";
    private String mMmscUrl;

    public ReadRecTransaction(Context context,
            int transId,
            String mmscUrl,
            String uri,
            int subId) {
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
        Log.d(LogTag.TXN_TAG, "ReadRecTransaction: process()");
        // prepare for ReadRec
        int readReportState = 0;
        String messageId = null;
        long msgId = 0;
        EncodedStringValue[] sender = new EncodedStringValue[1];
        Cursor cursor = null;
        cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(), mUri,
                new String[] {
                        Mms.MESSAGE_ID, Mms.READ_REPORT, Mms._ID
                }, null, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    messageId = cursor.getString(0);
                    readReportState = cursor.getInt(1);
                    msgId = cursor.getLong(2);
                }
                // if curosr==null, this means the mms is deleted during
                // processing.
                // exception will happened. catched by out catch clause.
                // so do not catch exception here.
            } finally {
                cursor.close();
            }
        }
        Log.d(LogTag.TXN_TAG, "messageid:" + messageId + ",and readreport flag:"
                + readReportState + ", mSubId = " + mSubId);

        cursor = null;
        cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(),
                Uri.parse("content://mms/" + msgId + "/addr"), new String[] {
                        Mms.Addr.ADDRESS, Mms.Addr.CHARSET
                }, Mms.Addr.TYPE + " = " + MtkPduHeaders.FROM, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    String address = cursor.getString(0);
                    int charSet = cursor.getInt(1);
                    sender[0] = new EncodedStringValue(charSet, MtkPduPersister.getBytes(address));
                }
                // if cursor == null exception will catched by out catch clause.
            } finally {
                cursor.close();
            }
        }
        try {
            ReadRecInd readRecInd = new ReadRecInd(new EncodedStringValue(
                    MtkPduHeaders.FROM_INSERT_ADDRESS_TOKEN_STR.getBytes()), messageId.getBytes(),
                    MtkPduHeaders.CURRENT_MMS_VERSION, MtkPduHeaders.READ_STATUS_READ, // always
                                                                                // set
                                                                                // read.
                    sender);
            readRecInd.setDate(System.currentTimeMillis() / 1000);

            byte[] datas = new MtkPduComposer(mContext, readRecInd).make();
            mPduFile = createPduFile(datas, READREP_REQ_NAME + mUri.getLastPathSegment());
            if (mPduFile == null) {
                Log.e(LogTag.TXN_TAG, "create pdu file req failed!");
                return;
            }

            // once send out, not check send result and move from out box to sent box
            ContentValues values = new ContentValues(1);
            values.put(Mms.MESSAGE_BOX, Mms.MESSAGE_BOX_SENT);
            int updateNum = SqliteWrapper.update(
                    mContext,
                    mContext.getContentResolver(),
                    mUri,
                    values,
                    null, null);

            //Intent intent = new Intent(TransactionService.ACTION_TRANSACION_PROCESSED);
            //intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
            //intent.putExtra(TransactionBundle.URI, mUri.toString());
            Log.d(LogTag.TXN_TAG, "ReadRecTransaction mUri:" + mUri);
            final Intent intent = new Intent(TransactionService.ACTION_TRANSACION_PROCESSED,
                    mUri,
                    mContext,
                    MmsReceiver.class);
            intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
            intent.putExtra(TransactionBundle.TRANSACTION_TYPE, READREC_TRANSACTION);

            PendingIntent sentIntent = PendingIntent.getBroadcast(mContext, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Uri pduFileUri = FileProvider.getUriForFile(
                    mContext, MMS_FILE_PROVIDER_AUTHORITIES, mPduFile);
            SmsManager.getSmsManagerForSubscriptionId(mSubId).sendMultimediaMessage(mContext,
                    pduFileUri, null,
                    null, sentIntent);
        } catch (InvalidHeaderValueException e) {
            Log.e(TAG, "Invalide header value", e);
            getState().setState(FAILED);
            getState().setContentUri(mUri);
            notifyObservers();
        } catch (MmsException e) {
            Log.e(TAG, "Persist message failed", e);
            getState().setState(FAILED);
            getState().setContentUri(mUri);
            notifyObservers();
        } catch (Throwable t) {
            Log.e(TAG, Log.getStackTraceString(t));
            getState().setState(FAILED);
            getState().setContentUri(mUri);
            notifyObservers();
        }
    }

    @Override
    public int getType() {
        return READREC_TRANSACTION;
    }

    public int checkSendResult(byte[] response) {
        if (mPduFile != null) {
            mPduFile.delete();
        }
        return Activity.RESULT_OK;
    }
}
