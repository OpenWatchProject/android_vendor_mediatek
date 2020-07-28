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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.op18.mms;
//package com.mediatek.op18.mms;


import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Notification;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.Binder;
import android.os.IBinder;

import com.mediatek.op18.mms.BackupEngine;
import com.mediatek.op18.mms.BackupEngine.BackupResultType;
import com.mediatek.op18.mms.BackupEngine.OnBackupDoneListner;
import com.mediatek.op18.mms.ResultDialog.ResultEntity;
import com.mediatek.op18.mms.modules.Composer;
import com.mediatek.op18.mms.utils.Constants;
import com.mediatek.op18.mms.utils.Constants.State;
import com.mediatek.op18.mms.utils.ModuleType;
import com.mediatek.op18.mms.utils.MyLogger;
import com.mediatek.op18.mms.R;

/**
 * @author mtk81330
 *
 */
public class BackupService extends Service implements ProgressReporter, OnBackupDoneListner {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupService";
    private BackupBinder mBinder = new BackupBinder();
    private int mState;
    private BackupEngine mBackupEngine;
    private ArrayList<ResultEntity> mResultList;
    private BackupProgress mCurrentProgress = new BackupProgress();
    private OnBackupStatusListener mStatusListener;
    private BackupResultType mResultType;
    private ArrayList<ResultEntity> mAppResultList;
    HashMap<Integer, ArrayList<String>> mParasMap = new HashMap<Integer, ArrayList<String>>();
    NewDataNotifyReceiver mNotificationReceiver = null;
    private boolean mComposerResult = true;

    @Override
    public IBinder onBind(Intent intent) {
        MyLogger.logI(CLASS_TAG, "onBind");
        return mBinder;
    }

    /**
     * onUnbind.
     * @return boolean
     */
    public boolean onUnbind(Intent intent) {
        super.onUnbind(intent);
        MyLogger.logI(CLASS_TAG, "onUnbind");
        // If SD card removed or full, kill process
        // Check why this ? (When SD full and switch to restore tab this kills app)
        //SDCardUtils.killProcessIfNecessary(this);
        return true;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mState = State.INIT;
        MyLogger.logI(CLASS_TAG, "onCreate");
    }

    /**
     * @param intent
     * @param flags
     * @param startId
     * @return int
     */
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        MyLogger.logI(CLASS_TAG, "onStartCommand");
        return START_NOT_STICKY;
    }

    /**
     * onRebind.
     */
    public void onRebind(Intent intent) {
        super.onRebind(intent);
        MyLogger.logI(CLASS_TAG, "onRebind");
    }

    /**
     * onDestroy.
     */
    public void onDestroy() {
        super.onDestroy();
        stopForeground(true);
        MyLogger.logI(CLASS_TAG, "onDestroy");
        if (mBackupEngine != null && mBackupEngine.isRunning()) {
            mBackupEngine.setOnBackupDoneListner(null);
            mBackupEngine.cancel();
        }

        if (mNotificationReceiver != null) {
            unregisterReceiver(mNotificationReceiver);
            mNotificationReceiver = null;
        }
    }

    /**
     * @author mtk81330
     *
     */
    public static class BackupProgress {
        Composer mComposer;
        int mType;
        int mMax;
        int mCurNum;
    }

    /**
     * @author mtk81330
     *
     */
    public class BackupBinder extends Binder {
        public int getState() {
            MyLogger.logI(CLASS_TAG, "getState: " + mState);
            return mState;
        }

        /**
         * @param list list
         */
        public void setBackupModelList(ArrayList<Integer> list) {
            reset();
            if (mBackupEngine == null) {
                mBackupEngine = new BackupEngine(BackupService.this, BackupService.this);
            }
            mBackupEngine.setBackupModelList(list);
        }

        /**
         * @param itemType type
         * @param paraList list
         */
        public void setBackupItemParam(int itemType, ArrayList<String> paraList) {
            MyLogger.logD(
                CLASS_TAG,
                "Param List Size is " + (paraList == null ? 0 : paraList.size()));
            mParasMap.put(itemType, paraList);
            mBackupEngine.setBackupItemParam(itemType, paraList);
        }

        /**
         * @param itemType type
         * @return ArrayList<String>
         */
        public ArrayList<String> getBackupItemParam(int itemType) {
            return mParasMap.get(itemType);
        }

        /**
         * @param folderName folder
         * @return boolean
         */
        public boolean startBackup(String folderName) {
            stayForeground();
            boolean ret = false;
            mBackupEngine.setOnBackupDoneListner(BackupService.this);
            ret = mBackupEngine.startBackup(folderName);
            if (ret) {
                mState = State.RUNNING;
            } else {
                mBackupEngine.setOnBackupDoneListner(null);
            }
            MyLogger.logD(CLASS_TAG, "startBackup: " + ret);
            return ret;
        }

        /**
         * stopForeground.
         */
        public void stopForeground() {
            BackupService.this.stopForeground(true);
            MyLogger.logD(CLASS_TAG, "stopFreground");
        }

        /**
         * pauseBackup.
         */
        public void pauseBackup() {
            mState = State.PAUSE;
            if (mBackupEngine != null) {
                mBackupEngine.pause();
            }
            MyLogger.logD(CLASS_TAG, "pauseBackup");
        }

        /**
         * cancelBackup.
         */
        public void cancelBackup() {
            mState = State.CANCELLING;
            if (mBackupEngine != null) {
                mBackupEngine.cancel();
            }
            MyLogger.logD(CLASS_TAG, "cancelBackup");
        }

        /**
         * continueBackup.
         */
        public void continueBackup() {
            mState = State.RUNNING;
            if (mBackupEngine != null) {
                mBackupEngine.continueBackup();
            }
            MyLogger.logD(CLASS_TAG, "continueBackup");
        }

        /**
         * reset.
         */
        public void reset() {
            mState = State.INIT;
            if (mResultList != null) {
                mResultList.clear();
            }
            if (mAppResultList != null) {
                mAppResultList.clear();
            }
            if (mParasMap != null) {
                mParasMap.clear();
            }
        }

        public BackupProgress getCurBackupProgress() {
            return mCurrentProgress;
        }

        public void setOnBackupChangedListner(OnBackupStatusListener listener) {
            mStatusListener = listener;
        }

        public ArrayList<ResultEntity> getBackupResult() {
            return mResultList;
        }

        public BackupResultType getBackupResultType() {
            return mResultType;
        }

        public ArrayList<ResultEntity> getAppBackupResult() {
            return mAppResultList;
        }
    }

    @Override
    public void onStart(Composer composer) {
        mCurrentProgress.mComposer = composer;
        mCurrentProgress.mType = composer.getModuleType();
        mCurrentProgress.mMax = composer.getCount();
        mCurrentProgress.mCurNum = 0;
        if (mStatusListener != null) {
            mStatusListener.onComposerChanged(composer);
        }
        if (mCurrentProgress.mMax != 0) {
            //NotifyManager.getInstance(BackupService.this).setMaxPercent(mCurrentProgress.mMax);
        }
    }

    @Override
    public void onOneFinished(Composer composer, boolean result) {
        mCurrentProgress.mCurNum++;
        if (composer.getModuleType() == ModuleType.TYPE_APP) {
            if (mAppResultList == null) {
                mAppResultList = new ArrayList<ResultEntity>();
            }
            int type = result ? ResultEntity.SUCCESS : ResultEntity.FAIL;
            ResultEntity entity = new ResultEntity(ModuleType.TYPE_APP, type);
            entity.setKey(mParasMap.get(ModuleType.TYPE_APP).get(mCurrentProgress.mCurNum - 1));
            mAppResultList.add(entity);
        }
        if (mStatusListener != null) {
            mStatusListener.onProgressChanged(composer, mCurrentProgress.mCurNum);
        }

        if (mCurrentProgress.mMax != 0 && mState != State.PAUSE) {
            /*NotifyManager.getInstance(BackupService.this).showBackupNotification(
                    ModuleType.getModuleStringFromType(this, composer.getModuleType()),
                    composer.getModuleType(), mCurrentProgress.mCurNum);*/
        } else {
            MyLogger.logD(CLASS_TAG, "Not show notification, due to progress is "
                    + mCurrentProgress.mMax + " or state " + mState);
        }
    }

    @Override
    public void onEnd(Composer composer, boolean result) {
        int resultType = ResultEntity.SUCCESS;
        if (mResultList == null) {
            mResultList = new ArrayList<ResultEntity>();
        }
        if (result == false) {
            if (composer.getCount() == 0) {
                resultType = ResultEntity.NO_CONTENT;
            } else {
                resultType = ResultEntity.FAIL;
                mComposerResult = false;
            }
        }
        MyLogger.logD(CLASS_TAG, "one Composer end: type = " + composer.getModuleType()
                + ", result = " + resultType);
        ResultEntity item = new ResultEntity(composer.getModuleType(), resultType);
        mResultList.add(item);
    }

    @Override
    public void onErr(IOException e) {
        MyLogger.logD(CLASS_TAG, "onErr " + e.getMessage());
        if (mStatusListener != null) {
            mStatusListener.onBackupErr(e);
        }
    }

    /**
     * onFinishBackup.
     */
    public void onFinishBackup(BackupResultType result) {
        MyLogger.logD(CLASS_TAG, "onFinishBackup result = " + result);
        mResultType = result;
        if (mStatusListener != null) {
            if (mState == State.CANCELLING) {
                result = BackupResultType.Cancel;
            }
            if (result != BackupResultType.Success && result != BackupResultType.Cancel) {
                for (ResultEntity item : mResultList) {
                    if (item.getResult() == ResultEntity.SUCCESS) {
                        item.setResult(ResultEntity.FAIL);
                    }
                }
            }
            mState = State.FINISH;
            mStatusListener.onBackupEnd(result, mResultList, mAppResultList);
        } else {
            mState = State.FINISH;
        }
        if (mResultType != BackupResultType.Cancel && mComposerResult) {
            //NotifyManager.getInstance(BackupService.this).showFinishNotification(
                    //NotifyManager.NOTIFY_BACKUPING, true);
        } else if (!mComposerResult) {
            //NotifyManager.getInstance(BackupService.this).showFinishNotification(
                    //NotifyManager.NOTIFY_BACKUPING, false);
            mComposerResult = true;
        } else {
            //NotifyManager.getInstance(BackupService.this).clearNotification();
        }
    }

    /**
     * @author mtk81330
     *
     */
    public interface OnBackupStatusListener {

        /**
         * @param composer
         */
        public void onComposerChanged(Composer composer);

        /**
         * @param composer composer
         * @param progress progress
         */
        public void onProgressChanged(Composer composer, int progress);

        /**
         * @param resultCode.
         * @param resultRecord.
         * @param appResultRecord.
         */
        public void onBackupEnd(final BackupResultType resultCode,
                final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord);

        /**
         * @param e IOException
         */
        public void onBackupErr(IOException e);
    }

    /**
     * @author mtk81330
     *
     */
    class NewDataNotifyReceiver extends BroadcastReceiver {
        public static final String CLASS_TAG = "NotificationReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {
            if (Constants.ACTION_NEW_DATA_DETECTED.equals(intent.getAction())) {
                MyLogger.logD(CLASS_TAG, "BackupService ------>ACTION_NEW_DATA_DETECTED received");
                int type = intent.getIntExtra(Constants.NOTIFY_TYPE, 0);
                String folder = intent.getStringExtra(Constants.FILENAME);
                if (mBackupEngine != null && mBackupEngine.isRunning()) {
                    /*NotifyManager.getInstance(context).showNewDetectionNotification(type, folder);
                    Toast.makeText(getApplicationContext(),
                            getString(R.string.backup_running_toast), Toast.LENGTH_SHORT).show();*/
                    abortBroadcast();
                }
            }
        }

    }

    private void stayForeground() {
        Notification notification = new Notification.Builder(this)
                                        .setSmallIcon(R.drawable.ic_application)
                                        .build();
//        notification.flags |= Notification.FLAG_HIDE_NOTIFICATION;
        startForeground(1, notification);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        MyLogger.logD(CLASS_TAG, "onConfigurationChanged: setRefreshFlag");
        //NotifyManager.getInstance(this).setRefreshFlag();
    }
}
