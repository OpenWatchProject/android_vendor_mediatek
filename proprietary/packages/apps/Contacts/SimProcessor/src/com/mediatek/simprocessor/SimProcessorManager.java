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
package com.mediatek.simprocessor;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
//import android.util.Log;

import com.mediatek.simprocessor.ProcessorBase;

import com.mediatek.simprocessor.SimServiceUtils.SimProcessorState;
import com.mediatek.simprocessor.Log;

import java.util.concurrent.ConcurrentHashMap;

public class SimProcessorManager implements SimProcessorState {
    private static final String TAG = "SimProcessorManager";

    public interface ProcessorManagerListener {
        public void addProcessor(long scheduleTime, ProcessorBase processor);

        public void onAllProcessorsFinished();
    }

    public interface ProcessorCompleteListener {
        public void onProcessorCompleted(Intent intent);
    }

    private ProcessorManagerListener mListener;
    private Handler mHandler;
    private ConcurrentHashMap<Integer, SimProcessorBase> mImportRemoveProcessors;
    private ConcurrentHashMap<Integer, SimProcessorBase> mOtherProcessors;

    private static final int MSG_SEND_STOP_SERVICE = 1;

    public SimProcessorManager(Context context, ProcessorManagerListener listener) {
        Log.i(TAG, "[SimProcessorManager]new...");
        mListener = listener;
        mImportRemoveProcessors = new ConcurrentHashMap<Integer, SimProcessorBase>();
        mOtherProcessors = new ConcurrentHashMap<Integer, SimProcessorBase>();
        SimServiceUtils.setSimProcessorState(this);
        mHandler = new Handler(context.getMainLooper()) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_SEND_STOP_SERVICE:
                    callStopService();
                    break;
                default:
                    break;
                }
            }
        };
    }

    public void handleProcessor(Context context, int subId, int workType, Intent intent) {
        Log.i(TAG, "[handleProcessor] subId=" + subId + ",time=" + System.currentTimeMillis());
        SimProcessorBase processor = createProcessor(context, subId, workType, intent);
        if (processor != null && mListener != null) {
            Log.d(TAG, "[handleProcessor]Add processor [subId=" + subId + "] to threadPool.");
            mListener.addProcessor(/* 1000 + slotId * 300 */0, processor);
        }
    }

    public boolean isImportRemoveRunning(int subId) {
        if ((mImportRemoveProcessors != null) && (mImportRemoveProcessors.containsKey(subId))) {
            SimProcessorBase processor = mImportRemoveProcessors.get(subId);
            if (processor == null) {
                Log.i(TAG, "[isImportRemoveRunning]processor is null, return false.");
                return false;
            }
            if (processor.isRunning()) {
                Log.i(TAG, "[isImportRemoveRunning]has exist running processor, return true.");
                return true;
            }
        }

        return false;
    }

    private SimProcessorBase createProcessor(Context context, int subId, int workType,
            Intent intent) {
        Log.d(TAG, "[createProcessor]subId = " + subId + ",workType = " + workType);
        SimProcessorBase processor = null;
        /**
         * [ALPS01224227]the mImportRemoveProcessors is likely to be accessed by
         * main thread and sub thread at the same time, we should protect the
         * race condition
         */
        synchronized (mProcessorRemoveLock) {
            // The rule to check whether or not create new processor
            if (mImportRemoveProcessors.containsKey(subId)) {
                processor = mImportRemoveProcessors.get(subId);
                if (processor != null
                        && (workType == SimServiceUtils.SERVICE_WORK_IMPORT ||
                        workType == SimServiceUtils.SERVICE_WORK_REMOVE)) {
                    if (processor.isRunning() && processor.getType() == workType) {
                        Log.w(TAG, "[createProcessor]has exist running processor, return null.");
                        if (subId >= 0 && workType == SimServiceUtils.SERVICE_WORK_IMPORT) {
                            queryIccProviderOnce(context, subId);
                        }
                        return null;
                    }
                    Log.d(TAG, "[createProcessor]cancel processor.");
                    processor.cancel(false);
                    mImportRemoveProcessors.remove(subId);
                }
            } else {
                Log.i(TAG, "[createProcessor]no processor for subId: " + subId);
            }

            processor = createProcessor(context, subId, workType, intent, mProcessoListener);

            if (workType == SimServiceUtils.SERVICE_WORK_IMPORT
                    || workType == SimServiceUtils.SERVICE_WORK_REMOVE) {
                mImportRemoveProcessors.put(subId, processor);
            } else {
                mOtherProcessors.put(subId, processor);
            }
        }

        return processor;
    }

    private SimProcessorBase createProcessor(Context context, int subId, int workType,
            Intent intent, ProcessorCompleteListener listener) {
        Log.d(TAG, "[createProcessor] create new processor for subId: " + subId + ", workType: "
                + workType);
        SimProcessorBase processor = null;

        if (workType == SimServiceUtils.SERVICE_WORK_IMPORT) {
            processor = new SimImportProcessor(context, subId, intent, listener);
        } else if (workType == SimServiceUtils.SERVICE_WORK_REMOVE) {
            processor = new SimRemoveProcessor(context, subId, intent, listener);
        }

        return processor;
    }

    private ProcessorCompleteListener mProcessoListener = new ProcessorCompleteListener() {

        @Override
        public void onProcessorCompleted(Intent intent) {
            if (intent != null) {
                int subId = intent.getIntExtra(SimServiceUtils.SERVICE_SUBSCRIPTION_KEY, 0);
                int workType = intent.getIntExtra(SimServiceUtils.SERVICE_WORK_TYPE, -1);
                Log.d(TAG,
                        "[onProcessorCompleted] subId = " + subId + ",time="
                                + System.currentTimeMillis() + ", workType = " + workType);
                /**
                 * [ALPS01224227]the mImportRemoveProcessors is likely to be
                 * accessed by main thread and sub thread at the same time, we
                 * should protect the race condition
                 */
                synchronized (mProcessorRemoveLock) {
                    if ((workType == SimServiceUtils.SERVICE_WORK_IMPORT ||
                            workType == SimServiceUtils.SERVICE_WORK_REMOVE)
                            && mImportRemoveProcessors.containsKey(subId)) {
                        Log.d(TAG,
                                "[onProcessorCompleted] remove import/remove processor subId="
                                        + subId);
                        /**
                         * [ALPS01224227]when we're going to remove the
                         * processor, in seldom condition, it might have already
                         * removed and replaced with another processor. in this
                         * case, we should not remove it any more.
                         */
                        if (mImportRemoveProcessors.get(subId).identifyIntent(intent)) {
                            mImportRemoveProcessors.remove(subId);
                            checkStopService();
                        } else {
                            Log.w(TAG,
                                    "[onProcessorCompleted] race condition");
                        }
                    } else if (mOtherProcessors.containsKey(subId)) {
                        Log.d(TAG, "[onProcessorCompleted] remove other processor subId=" + subId);
                        /**
                         * [ALPS01224227]when we're going to remove the
                         * processor, in seldom condition, it might have already
                         * removed and replaced with another processor. in this
                         * case, we should not remove it any more.
                         */
                        if (mOtherProcessors.get(subId).identifyIntent(intent)) {
                            mOtherProcessors.remove(subId);
                            checkStopService();
                        } else {
                            Log.w(TAG,
                                    "[onProcessorCompleted] race condition2");
                        }
                    } else {
                        Log.w(TAG, "[onProcessorCompleted] slotId processor not found");
                    }
                }
            }
        }
    };

    private void checkStopService() {
        Log.v(TAG, "[checkStopService]...");
        if (mImportRemoveProcessors.size() == 0 && mOtherProcessors.size() == 0) {
            if (mHandler != null) {
                Log.v(TAG, "[checkStopService] send stop service message.");
                mHandler.removeMessages(MSG_SEND_STOP_SERVICE);
                /* Fix [ALPS03435956] should send stop message immediately
                 * without delay, to avoid new start request of SimProcessorService
                 * coming just before stopping SimProcessorService in stop message.
                 * Otherwise, this new start request will do nothing because
                 * SimProcessorService will be stopped soon.
                 */
                mHandler.sendEmptyMessage(MSG_SEND_STOP_SERVICE);
            }
        }
    }

    private void callStopService() {
        Log.d(TAG, "[callStopService]...");
        if (mListener != null && mImportRemoveProcessors.size() == 0
                && mOtherProcessors.size() == 0) {
            mListener.onAllProcessorsFinished();
        }
    }

    private void queryIccProviderOnce(Context context, int subId) {
        final AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Log.d(TAG, "[queryIccProviderOnce] subId: " + subId);
                try {
                    Cursor cursor = context.getContentResolver().query(
                            SubInfoUtils.getIccProviderUri(subId),
                                new String[] {"index"}, null, null, null);
                    if (cursor != null) {
                        Log.d(TAG, "[queryIccProviderOnce] count:" + cursor.getCount());
                        cursor.close();
                    }
                } catch (java.lang.NullPointerException e) {
                    Log.e(TAG, "[queryIccProviderOnce]exception:" + e.toString());
                }
                return null;
            }
        };
        task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    /**
     * [ALPS01224227]the lock for synchronized
     */
    private final Object mProcessorRemoveLock = new Object();

}
