/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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


package com.mediatek.mms.appservice.mwi;

import android.app.Activity;
import android.app.Service;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.mms.appservice.LogTag;
import com.mediatek.mms.appservice.MmsConfig;
import com.mediatek.mms.appservice.MmsLog;

import java.util.ArrayList;
import java.util.HashMap;

import mediatek.telephony.MtkTelephony.MtkMwi;

import static android.content.Intent.ACTION_BOOT_COMPLETED;


/**
 * This service essentially plays the role of a "worker thread", allowing us to store
 * incoming messages to the database, update notifications, etc. without blocking the
 * main thread that MwiReceiver runs on.
 */
public class MwiReceiverService extends Service {
    private static final String TAG = "Mms/Mwi";
    private static final boolean DEBUG = false;

    private ServiceHandler mServiceHandler;
    private Looper mServiceLooper;
    private boolean mSending;

    public static boolean sSmsSent = true;
    private static final Uri MWI_URI = MtkMwi.CONTENT_URI;

    private int mResultCode;

    /**
     * VoLTE feature option
     */
    private static final boolean MTK_IMS_SUPPORT = SystemProperties.
            get("persist.vendor.ims_support").equals("1");
    private static final boolean MTK_VOLTE_SUPPORT = SystemProperties.
            get("persist.vendor.volte_support").equals("1");
    private static final boolean MTK_MWI_SUPPORT = MTK_IMS_SUPPORT && MTK_VOLTE_SUPPORT;

    @Override
    public void onCreate() {
        // Start up the thread running the service.  Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block.
        HandlerThread thread = new HandlerThread(TAG, Process.THREAD_PRIORITY_BACKGROUND);
        thread.start();

        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // Temporarily removed for this duplicate message track down.

        mResultCode = intent != null ? intent.getIntExtra("result", 0) : 0;

        if (mResultCode != 0) {
            Log.i(TAG, "onStart: #" + startId + " mResultCode: " + mResultCode +
                    " = " + translateResultCode(mResultCode));
        }

        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        return Service.START_NOT_STICKY;
    }

    private static String translateResultCode(int resultCode) {
        switch (resultCode) {
            case Activity.RESULT_OK:
                return "Activity.RESULT_OK";
            default:
                return "Unknown error code";
        }
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
            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                Log.v(TAG, "handleMessage serviceId: " + serviceId + " intent: " + intent);
            }
            if (intent != null) {
                String action = intent.getAction();

                int error = intent.getIntExtra("errorCode", 0);

                if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                    Log.v(TAG, "handleMessage action: " + action + " error: " + error);
                }

                if ("mediatek.intent.action.lte.mwi".equals(action)) {
                    handleMwiReceived(intent, error);
                } else if (ACTION_BOOT_COMPLETED.equals(action)) {
                    handleBootCompleted();
                }
            }

            sSmsSent = true;
            // NOTE: We MUST not call stopSelf() directly, since we need to
            // make sure the wake lock acquired by AlertReceiver is released.
            /* mark by hongduo
            MwiReceiver.finishStartingService(MwiReceiverService.this, serviceId);
            */
        }
    }

    private void handleMwiReceived(Intent intent, int error) {
        if (!MTK_MWI_SUPPORT) {
            return;
        }

        Log.i(TAG, "handleMwiReceived: " + intent.getAction());

        String data = intent.getStringExtra("lte_mwi_body");
        if (DEBUG) {
            MmsLog.dpi(TAG, "onReceive data: " + data);
        }
        if (data == null) {
            Log.w(TAG, "onReceive data is null");
            return;
        }
        ArrayList<MwiMessage> msgList = MwiParser.parseMwi(this, data);
        if (msgList == null || msgList.size() == 0) {
            return;
        }
        /* mark by hongduo
        MwiListAdapter.updateChangeCount(msgList.size());
        */
        ContentResolver cr = this.getContentResolver();
        Uri uri = MWI_URI;
        HashMap<Uri, MwiMessage> mapMessages = new HashMap<Uri, MwiMessage>();
        for (int i = 0; i < msgList.size(); i++) {
            MwiMessage msg = msgList.get(i);
            ContentValues cv = new ContentValues();
            cv.put(MwiMessage.Columns.MsgAccount.getColumnName(), msg.getMsgAccount());
            cv.put(MwiMessage.Columns.To.getColumnName(), msg.getTo());
            cv.put(MwiMessage.Columns.From.getColumnName(), msg.getFrom());
            cv.put(MwiMessage.Columns.Subject.getColumnName(), msg.getSubject());
            cv.put(MwiMessage.Columns.Date.getColumnName(), msg.getDate());
            cv.put(MwiMessage.Columns.Priority.getColumnName(), msg.getPriorityId());
            cv.put(MwiMessage.Columns.MsgId.getColumnName(), msg.getMsgId());
            cv.put(MwiMessage.Columns.MsgContext.getColumnName(), msg.getMsgContextId());
            cv.put(MwiMessage.Columns.Seen.getColumnName(), msg.getSeen());
            cv.put(MwiMessage.Columns.GotContent.getColumnName(), msg.getGotContent());
            Uri insertUri = cr.insert(uri, cv);
            MmsLog.dpi(TAG, "insertUri = " + insertUri);
            msg.setUri(insertUri);
        }
        MmsConfig.updateMwiNotification(this, true);
    }

    private void handleBootCompleted() {
        // Called off of the UI thread so ok to block.
        if (MTK_MWI_SUPPORT) {
            /* mark by hongduo
            MwiMessagingNotification.blockingUpdateNewMessageIndicator(this, false);
            */
        }
    }

}
