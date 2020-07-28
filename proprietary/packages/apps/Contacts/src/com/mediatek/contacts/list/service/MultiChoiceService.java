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
package com.mediatek.contacts.list.service;

import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.vcard.ProcessorBase;

import com.mediatek.contacts.ContactsApplicationEx;
import com.mediatek.contacts.util.Log;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.RejectedExecutionException;

/**
 * The class responsible for handling all of multiple choice requests. This
 * Service creates one MultiChoiceRequest object (as Runnable) per request and
 * push it to {@link ExecutorService} with single thread executor. The executor
 * handles each request one by one, and notifies users when needed.
 */
public class MultiChoiceService extends Service {
    private static final String TAG = "MultiChoiceService";

    // Should be single thread, as we don't want to simultaneously handle import
    // and export requests.
    private final ExecutorService mExecutorService = ContactsApplicationEx.getContactsApplication()
            .getApplicationTaskService();

    // Stores all unfinished import/export jobs which will be executed by
    // mExecutorService. Key is jobId.
    private static final Map<Integer, ProcessorBase> RUNNINGJOBMAP =
            new HashMap<Integer, ProcessorBase>();

    public static final int TYPE_COPY = 1;
    public static final int TYPE_DELETE = 2;

    private static int sCurrentJobId;

    private MyBinder mBinder;

    public class MyBinder extends Binder {
        public MultiChoiceService getService() {
            return MultiChoiceService.this;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mBinder = new MyBinder();
        Log.d(TAG, "[onCreate]Multi-choice Service is being created.");

        // / change for low_memory kill Contacts process CR.
        // startForeground(1, new Notification());
    }

    /**
     * M: change for low_memory kill Contacts process @{ reference CR:
     * ALPS00564966,ALPS00567689,ALPS00567905
     **/
    @Override
    public void onDestroy() {
        // stopForeground(true);
        super.onDestroy();
    }

    /** @} */

    @Override
    public int onStartCommand(Intent intent, int flags, int id) {
        // / M: change START_STICKY to START_NOT_STICKY for Service slim
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    /**
     * Tries to call {@link ExecutorService#execute(Runnable)} toward a given
     * processor.
     *
     * @return true when successful.
     */
    private synchronized boolean tryExecute(ProcessorBase processor) {
        try {
            Log.d(TAG, "[tryExecute]Executor service status: shutdown: "
                            + mExecutorService.isShutdown() + ", terminated: "
                            + mExecutorService.isTerminated());
            mExecutorService.execute(processor);
            RUNNINGJOBMAP.put(sCurrentJobId, processor);
            return true;
        } catch (RejectedExecutionException e) {
            Log.w(TAG, "[tryExecute]Failed to excetute a job:" + e);
            return false;
        }
    }

    public synchronized void handleDeleteRequest(List<MultiChoiceRequest> requests,
            MultiChoiceHandlerListener listener) {
        sCurrentJobId++;
        Log.i(TAG, "[handleDeleteRequest]sCurrentJobId:" + sCurrentJobId);
        if (tryExecute(new DeleteProcessor(this, listener, requests, sCurrentJobId))) {
            if (listener != null) {
                listener.onProcessed(TYPE_DELETE, sCurrentJobId, 0, -1,
                        requests.get(0).mContactName);
            }
        }
    }

    public synchronized void handleCopyRequest(List<MultiChoiceRequest> requests,
            MultiChoiceHandlerListener listener, final AccountWithDataSet sourceAccount,
            final AccountWithDataSet destinationAccount) {
        sCurrentJobId++;
        Log.i(TAG, "[handleCopyRequest]sCurrentJobId:" + sCurrentJobId);
        if (tryExecute(new CopyProcessor(this, listener, requests, sCurrentJobId, sourceAccount,
                destinationAccount))) {
            if (listener != null) {
                listener.onProcessed(TYPE_COPY, sCurrentJobId, 0, -1, requests.get(0).mContactName);
            }
        }
    }

    public synchronized void handleCancelRequest(MultiChoiceCancelRequest request) {
        final int jobId = request.jobId;
        Log.i(TAG, "[handleCancelRequest]jobId:" + jobId);
        final ProcessorBase processor = RUNNINGJOBMAP.remove(jobId);

        if (processor != null) {
            processor.cancel(true);
        } else {
            Log.w(TAG, "[handleCancelRequest]"
                            + String.format("Tried to remove unknown job (id: %d)", jobId));
            // [ALPS03792783] This may be a completed job whose complete notification failed
            // to be refreshed due to NotificationManager throttling mechanism. We should
            // clear the notification here.
            NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            // [ALPS03868677] We need to specify tag otherwise we cannot cancel it.
            nm.cancel(MultiChoiceHandlerListener.DEFAULT_NOTIFICATION_TAG, jobId);
        }
        stopServiceIfAppropriate();
    }

    /**
     * Checks job list and call {@link #stopSelf()} when there's no job and no
     * scanner connection is remaining. A new job (import/export) cannot be
     * submitted any more after this call.
     */
    private synchronized void stopServiceIfAppropriate() {
        if (RUNNINGJOBMAP.size() > 0) {
            for (final Map.Entry<Integer, ProcessorBase> entry : RUNNINGJOBMAP.entrySet()) {
                final int jobId = entry.getKey();
                final ProcessorBase processor = entry.getValue();
                if (processor.isDone()) {
                    RUNNINGJOBMAP.remove(jobId);
                } else {
                    Log.i(
                            TAG,
                            "[stopServiceIfAppropriate]"
                                    + String.format("Found unfinished job (id: %d)", jobId));
                    return;
                }
            }
        }

        Log.i(TAG, "[stopServiceIfAppropriate]No unfinished job. Stop this service.");
        // mExecutorService.shutdown();
        stopSelf();
    }

    public synchronized void handleFinishNotification(int jobId, boolean successful) {
        Log.i(TAG, "[handleFinishNotification]jobId = " + jobId + ",successful = "
                + successful);
        if (RUNNINGJOBMAP.remove(jobId) == null) {
            Log.w(
                    TAG,
                    "[handleFinishNotification]"
                            + String.format("Tried to remove unknown job (id: %d)", jobId));
        }
        stopServiceIfAppropriate();
    }

    public static synchronized boolean isProcessing(int requestType) {
        if (RUNNINGJOBMAP.size() <= 0) {
            Log.w(TAG, "[isProcessing] size is <=0,return false!");
            return false;
        }

        if (RUNNINGJOBMAP.size() > 0) {
            for (final Map.Entry<Integer, ProcessorBase> entry : RUNNINGJOBMAP.entrySet()) {
                final ProcessorBase processor = entry.getValue();
                if (processor.getType() == requestType) {
                    Log.i(TAG, "[isProcessing]return true,requestType = " + requestType);
                    return true;
                }
            }
        }

        return false;
    }
}
