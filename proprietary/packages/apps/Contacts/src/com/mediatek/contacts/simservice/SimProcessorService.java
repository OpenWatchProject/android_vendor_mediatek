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
package com.mediatek.contacts.simservice;

import static android.Manifest.permission.WRITE_CONTACTS;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import com.android.contacts.vcard.ProcessorBase;

import com.mediatek.contacts.simservice.SimProcessorManager.ProcessorManagerListener;
import com.mediatek.contacts.util.Log;

import com.android.contacts.util.PermissionsUtil;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class SimProcessorService extends Service {
    private final static String TAG = "SIMProcessorService";

    private static final int CORE_POOL_SIZE = 2;
    private static final int MAX_POOL_SIZE = 10;
    private static final int KEEP_ALIVE_TIME = 10; // 10 seconds
    public static final String EXTRA_CALLBACK_INTENT = "callbackIntent";

    private SimProcessorManager mProcessorManager;
    private AtomicInteger mNumber = new AtomicInteger();
    private final ExecutorService mExecutorService = createThreadPool(CORE_POOL_SIZE);

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "[onCreate]...");
        mProcessorManager = new SimProcessorManager(this, mListener);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int id) {
        super.onStartCommand(intent, flags, id);
        processIntent(intent);
        return START_REDELIVER_INTENT;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "[onDestroy]...");
    }

    private void processIntent(Intent intent) {
        if (intent == null) {
            Log.w(TAG, "[processIntent] intent is null.");
            return;
        }
        if (!PermissionsUtil.hasPermission(this, WRITE_CONTACTS)) {
            Log.w(TAG, "No WRITE_CONTACTS permission, unable to handle intent:" + intent);
            return;
        }
        int subId = intent.getIntExtra(SimServiceUtils.SERVICE_SUBSCRIPTION_KEY, 0);
        int workType = intent.getIntExtra(SimServiceUtils.SERVICE_WORK_TYPE, -1);

        mProcessorManager.handleProcessor(getApplicationContext(), subId, workType, intent);
    }

    private SimProcessorManager.ProcessorManagerListener mListener =
            new ProcessorManagerListener() {
        @Override
        public void addProcessor(long scheduleTime, ProcessorBase processor) {
            if (processor != null) {
                try {
                    mExecutorService.execute(processor);
                } catch (RejectedExecutionException e) {
                    Log.w(TAG, "[addProcessor] RejectedExecutionException: " + e.toString());
                    // ALPS03527261: callback to UI for doing some action, e.g.change status
                    if (processor instanceof SimProcessorBase) {
                        mProcessorManager.onAddProcessorFail((SimProcessorBase) processor);
                    }
                }
            }
        }

        @Override
        public void onAllProcessorsFinished() {
            Log.d(TAG, "[onAllProcessorsFinished]...");
            stopSelf();
            mExecutorService.shutdown();
        }
    };

    private ExecutorService createThreadPool(int initPoolSize) {
        return new ThreadPoolExecutor(initPoolSize, MAX_POOL_SIZE, KEEP_ALIVE_TIME,
                TimeUnit.SECONDS,
                new SynchronousQueue<Runnable>(), new ThreadFactory() {
                    @Override
                    public Thread newThread(Runnable r) {
                        String threadName = "SIM Service - " + mNumber.getAndIncrement();
                        Log.d(TAG, "[createThreadPool]thread name:" + threadName);
                        return new Thread(r, threadName);
                    }
                });
    }
}
