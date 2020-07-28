/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.android.camera;

import java.util.LinkedList;
import java.util.List;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.IBinder;

public class FileSaverService extends Service {
    private static final String TAG = "FileSaverService";

    private static final int SAVE_TASK_LIMIT = 3;
    private int mTaskNumber;

    private List<SaveRequest> mQueue = new LinkedList<SaveRequest>();
    private final Binder mBinder = new LocalBinder();

    private SaveTask mContinuousSaveTask;
    private Object mListnerObject = new Object();

    public interface FileSaverListener {
        public void onQueueStatus(boolean full);

        public void onFileSaved(SaveRequest r);

        public void onSaveDone();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flag, int startId) {
        return START_STICKY;
    }

    @Override
    public void onCreate() {
        mTaskNumber = 0;
    }

    public boolean isNoneSaveTask() {
        return mTaskNumber == 0;
    }

    public long getWaitingDataSize() {
        long totalToWrite = 0;
        // LinkedList is not saved list, so mQueue should be sync in multi
        // thread
        synchronized (mQueue) {
            for (SaveRequest r : mQueue) {
                totalToWrite += r.getDataSize();
            }
        }

//        Log.d(TAG, "[getWaitingDataSize]totalToWrite = " + totalToWrite);

        return totalToWrite;
    }

    public int getWaitingCount() {
        synchronized (mQueue) {
            return mQueue.size();
        }
    }

    public boolean isQueueFull() {
        Log.v(TAG, "isQueueFull, mTaskNumber= " + mTaskNumber);
        return mTaskNumber >= SAVE_TASK_LIMIT;
    }

    // run in main thread
    public void addSaveRequest(SaveRequest request) {
//        Log.d(TAG, "[addSaveRequest]...begin,the queue number is = " + mQueue.size()
//                + "mContinuousSaveTask:" + mContinuousSaveTask);
        synchronized (mQueue) {
            mQueue.add(request);
        }
        if (mContinuousSaveTask == null) {
            mContinuousSaveTask = new SaveTask();
            mTaskNumber++;
            mContinuousSaveTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
            Log.d(TAG, "[addSaveRequest]execute continuous AsyncTask.");
        }
//        Log.d(TAG, "[addSaveRequest]...end,the queue number is = " + mQueue.size());

    }

    class LocalBinder extends Binder {
        public FileSaverService getService() {
            return FileSaverService.this;
        }
    }

    // this AsyncTask is used to save continous shot images.
    private class SaveTask extends AsyncTask<Void, Void, Void> {
        SaveRequest r;

        public SaveTask() {
        }

        @Override
        protected void onPreExecute() {
        }

        @Override
        protected Void doInBackground(Void... v) {
//            Log.d(TAG, "[SaveTask]doInBackground...,queue is empty = " + mQueue.isEmpty());
            FileSaverListener lastFileSaverListener = null;
            while (!mQueue.isEmpty()) {
                r = mQueue.get(0);
                //different cameraActivity use different listener
                // notify old listener save done info
                if (lastFileSaverListener != null
                        && r.getFileSaverListener() != lastFileSaverListener) {
                    lastFileSaverListener.onSaveDone();
                }

                if (Storage.isStorageReady()) {
                    try {
                        r.saveRequest();
                    } catch (RuntimeException e) {
                        e.printStackTrace();
                    }
                }
                r.notifyListener();
                // LinkedList is not saved list, so mQueue should be sync in
                // multi thread
                synchronized (mQueue) {
                    mQueue.remove(0);
                }
                synchronized (mListnerObject) {
                    r.getFileSaverListener().onFileSaved(r);
                }
                lastFileSaverListener = r.getFileSaverListener();
            }
            mContinuousSaveTask = null;
            mTaskNumber--;
            synchronized (mListnerObject) {
                r.getFileSaverListener().onSaveDone();
            }
//            Log.d(TAG, "[SaveTask]doInBackground...,end ");
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
//            Log.d(TAG, "[onPostExecute]...");
        }
    }
}
