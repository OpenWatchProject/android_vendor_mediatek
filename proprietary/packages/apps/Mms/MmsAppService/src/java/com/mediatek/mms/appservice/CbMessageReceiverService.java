/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
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

import android.app.Service;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.provider.Telephony;
import android.telephony.SmsCbMessage;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.widget.Toast;

//import com.android.mms.transaction.LogTag;

import com.android.internal.telephony.PhoneConstants;
import mediatek.telephony.MtkTelephony;

/**
 * M:
 * This service essentially plays the role of a "worker thread", allowing us to store
 * incoming messages to the database, update notifications, etc. without blocking the
 * main thread that SmsReceiver runs on.
 */
public class CbMessageReceiverService extends Service {
    private static final String TAG = "MmsCB/CBMessageReceiverService";

    private ServiceHandler mServiceHandler;
    private Looper mServiceLooper;

    private static final Uri MESSAGE_URI = MtkTelephony.SmsCb.CONTENT_URI;
    private static final int DEFAULT_SUB_ID = 1;

    // This must match SEND_PROJECTION.
    private int mResultCode;

    @Override
    public void onCreate() {
        // Start up the thread running the service.  Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block.
        HandlerThread thread = new HandlerThread(TAG, Process.THREAD_PRIORITY_BACKGROUND);
        thread.start();

        mServiceLooper = thread.getLooper();
        if (null != mServiceLooper) {
            mServiceHandler = new ServiceHandler(mServiceLooper);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mResultCode = intent != null ? intent.getIntExtra("result", 0) : 0;

        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        return Service.START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        mServiceLooper.quit();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        /**
         * Handle incoming transaction requests.
         * The incoming requests are initiated by the MMSC Server or by the MMS Client itself.
         */
        @Override
        public void handleMessage(Message msg) {
            int serviceId = msg.arg1;
            Intent intent = (Intent) msg.obj;
            if (intent != null) {
                String action = intent.getAction();
                // NEED Replace with CB ACTION
                if (Telephony.Sms.Intents.SMS_CB_RECEIVED_ACTION.equals(action)) {
                    handleCBMessageReceived(intent);
                }
            }
            // NOTE: We MUST not call stopSelf() directly, since we need to
            // make sure the wake lock acquired by AlertReceiver is released.
         // mark by hongduo
    //        CbMessageReceiver.finishStartingService(CbMessageReceiverService.this, serviceId);
        }
    }

    private void handleCBMessageReceived(Intent intent) {
        // TODO need replace with cb message.
        Bundle extras = intent.getExtras();
        if (null == extras) {
            Log.e(LogTag.TXN_TAG, "Intents.getMessagesFromIntent return null !!");
            return;
        }

        SmsCbMessage message = (SmsCbMessage) extras.get("message");
        if (null == message) {
            Log.e(LogTag.TXN_TAG, "received SMS_CB_RECEIVED_ACTION with no extras!");
            return;
        }

        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                SubscriptionManager.INVALID_SUBSCRIPTION_ID);

        Uri messageUri = insertMessage(subId, this, message);
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            MmsLog.vpi(TAG, "handleSmsReceived" +
                    " messageUri: " + messageUri +
                    ", body: " + message.getMessageBody());
        }

        if (messageUri != null) {
         // mark by hongduo
     //       CbMessagingNotification.updateNewMessageIndicator(subId, this, true);
           MmsConfig.updateCbNotification(this, subId);
        }
    }

    public static final String CLASS_ZERO_BODY_KEY = "CLASS_ZERO_BODY";

    private static final int REPLACE_COLUMN_ID = 0;

    /**
     * If the message is a class-zero message, display it immediately
     * and return null.  Otherwise, store it using the
     * <code>ContentResolver</code> and return the
     * <code>Uri</code> of the thread containing this message
     * so that we can use it for notification.
     */
    // TODO Need replace with CBMessage
    private Uri insertMessage(int subId, Context context, SmsCbMessage msg) {
        return storeCBMessage(subId, context, msg);
    }

    // TODO Need replace with CB message
    private Uri storeCBMessage(int subId, Context context, SmsCbMessage msg) {
        // Store the message in the content provider.
        String body = msg.getMessageBody();
        ContentResolver resolver = context.getContentResolver();
        ContentValues values = getCBContentValue(subId, msg, body);
        MmsLog.ipi(LogTag.TXN_TAG, "CB Message: " + msg + " and subid: " + subId);
        return resolver.insert(MESSAGE_URI, values);
    }

    // TODO  Need replace with CB Message
    private ContentValues getCBContentValue(int subId, SmsCbMessage msg, String body) {
        ContentValues values = new ContentValues();
        // TODO just use default SUB ID, need improve when two sub cards.
        values.put(MtkTelephony.SmsCb.SUBSCRIPTION_ID, subId);
        values.put(MtkTelephony.SmsCb.DATE, Long.valueOf(System.currentTimeMillis()));
        // Channel ID is getting from getMessageID
        values.put(MtkTelephony.SmsCb.CHANNEL_ID, msg.getServiceCategory());
        values.put(MtkTelephony.SmsCb.READ, Integer.valueOf(0));
        values.put(MtkTelephony.SmsCb.BODY, body);
        return values;
    }
}
