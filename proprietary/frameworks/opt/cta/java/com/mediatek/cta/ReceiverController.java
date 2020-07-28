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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.cta;

import android.content.Context;
import android.content.Intent;
import android.content.pm.IPackageManager;
import android.content.pm.ResolveInfo;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IUserManager;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.util.ArrayMap;
import android.util.Log;

import com.android.internal.content.PackageMonitor;

import java.util.Iterator;
import java.util.List;

class ReceiverController {
    static final String TAG = "Cta_ReceiverController";
    static final boolean DEBUG = false;

    private static Context mContext = null;
    private static ReceiverController sInstance = null;
    private ReceiverRecordHelper mRecordHelper = null;
    private static boolean mMonitorEnabled = true;

    private BootReceiverPolicy mBootReceiverPolicy = null ;
    private HandlerThread mWorkerThread;
    private Worker mWorker;

    static ReceiverController getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new ReceiverController(context);
        }
        return sInstance;
    }

    private ReceiverController(Context context) {
        mContext = context;
        mBootReceiverPolicy = BootReceiverPolicy.getInstance(mContext) ;
        initRecordHelper();

        mPackageMonitor.register(context, mContext.getMainLooper(), UserHandle.ALL, true);

        Log.d(TAG, "startMonitor(Normal Bootup Start)");
        mWorkerThread = new HandlerThread("SaveDataTofile", Process.THREAD_PRIORITY_BACKGROUND);
        mWorkerThread.start();
        mWorker = new Worker(mWorkerThread.getLooper());
    }

    private void initRecordHelper() {
        mRecordHelper = new ReceiverRecordHelper(
                mContext,
                getUserManagerService(),
                getPackageManagerService());
        mRecordHelper.initReceiverList();
        Log.d(TAG, "init ReceiverRecordHelper done.");
    }

    public void filterReceiver(Intent intent, List<ResolveInfo> resolveList, int userId) {
        String action = intent.getAction() ;
        if (mMonitorEnabled == false) {
            return;
        }
        if (action == null) {
            Log.e(TAG, "filterReceiver() ignored with null action");
            return;
        }
        if (resolveList == null) {
            return;
        }
        if (!isValidUserId(userId)) {
            Log.e(TAG, "filterReceiver() ignored with invalid userId: " + userId);
            return;
        }

        // Filter the resolver
        if (mBootReceiverPolicy.match(action)) {
            //refresh the cache since some packages may just be found by PMS
            mRecordHelper.updateReceiverCache();

            String packageName = null;
            Iterator<ResolveInfo> itor = resolveList.iterator();
            while (itor.hasNext()) {
                ResolveInfo info = itor.next();
                if (info.activityInfo != null) {
                    // Activity & Receiver are pasred as ActivityInfo in PMS
                    packageName = info.activityInfo.packageName;
                    Log.d(TAG, "filterReceiver() - package = " + packageName
                        + " has action = " + action) ;
                    if (!checkStrictPolicyAllowed(action, userId, packageName)) {
                        itor.remove();
                    }
                }
            }
        }
    }

    private boolean checkStrictPolicyAllowed(String action, int userId, String packageName) {
        boolean allowed = true;
        synchronized (mRecordHelper) {
            if (!mRecordHelper.getReceiverDataEnabled(userId, packageName)) {
                Log.d(TAG, "checkStrictPolicyAllowed() -  denied " + action +
                        " to package: " + packageName + " at User(" + userId + ")");
                allowed = false;
            }
        }
        // TODO: may post a notification here
        return allowed;
    }

    // auto boot management
    boolean checkAutoBootPermission(String packageName, int userId) {
        if (!isValidUserId(userId)) {
            Log.e(TAG, "filterReceiver() ignored with invalid userId: " + userId);
            return false;
        }
        boolean allowed = true;
        synchronized (mRecordHelper) {
            if (!mRecordHelper.getReceiverDataEnabled(userId, packageName)) {
                Log.d(TAG, "checkAutoBootPermission() -  denied " +
                        " to package: " + packageName + " at User(" + userId + ")");
                allowed = false;
            }
        }
        return allowed;
    }

    public void changeAppAutoBootStatus(String packageName,
                                 boolean status, int userId) {
        mRecordHelper.addReceiverRecord(userId, packageName, status);
    }

    public ArrayMap<String, Boolean> queryAutoBootRecords(int userId) {
        List<ReceiverRecord> records = mRecordHelper.getReceiverList(userId);
        ArrayMap<String, Boolean> mapRecords = new ArrayMap<>(records.size());
        for (int i = 0; i < records.size(); i++) {
            ReceiverRecord record = records.get(i);
            mapRecords.put(record.mPackageName, record.mEnabled);
        }
        return mapRecords;
    }
    /**
     * Get common services's binder
     */
    private static IPackageManager getPackageManagerService() {
        IPackageManager pm = null;
        pm = IPackageManager.Stub.asInterface(ServiceManager.getService("package"));
        if (pm == null) {
            throw new RuntimeException("null package manager service");
        }
        return pm;
    }

    private static IUserManager getUserManagerService() {
        IUserManager um = null;
        um = IUserManager.Stub.asInterface(ServiceManager.getService(Context.USER_SERVICE));
        if (um == null) {
            throw new RuntimeException("null user manager service");
        }
        return um;
    }

    private boolean isValidUserId(int userId) {
        if (userId >= UserHandle.USER_OWNER && userId < UserHandle.PER_USER_RANGE) {
            return true;
        }
        Log.e(TAG, "Invalid userId: " + userId);
        return false;
    }

    private final PackageMonitor mPackageMonitor = new PackageMonitor() {
        @Override
        public void onPackageAdded(String packageName, int uid) {
            Log.d(TAG, "onPackageAdded()");
            if (mRecordHelper != null) {
                mRecordHelper.addReceiverRecord(uid,packageName,true);
                mWorker.removeMessages(Worker.MSG_SAVE_AUTO_BOOT_DATA);
                mWorker.sendEmptyMessage(Worker.MSG_SAVE_AUTO_BOOT_DATA);
            }
        }
        @Override
        public void onPackageRemoved(String packageName, int uid) {
            Log.d(TAG, "onPackageRemoved()");
            if (mRecordHelper != null) {
                mRecordHelper.removeReceiverRecord(uid,packageName);
                mWorker.removeMessages(Worker.MSG_SAVE_AUTO_BOOT_DATA);
                mWorker.sendEmptyMessage(Worker.MSG_SAVE_AUTO_BOOT_DATA);
            }
        }

        @Override
        public void onPackagesAvailable(String[] packages) {
            Log.d(TAG, "onPackagesAvailable()");
            if (mRecordHelper != null) {
                mRecordHelper.updateReceiverCache() ;
                mWorker.removeMessages(Worker.MSG_SAVE_AUTO_BOOT_DATA);
                mWorker.sendEmptyMessage(Worker.MSG_SAVE_AUTO_BOOT_DATA);
            }
        }
        @Override
        public void onPackagesUnavailable(String[] packages) {
            Log.d(TAG, "onPackagesUnavailable()");
            if (mRecordHelper != null) {
                mRecordHelper.updateReceiverCache() ;
                mWorker.removeMessages(Worker.MSG_SAVE_AUTO_BOOT_DATA);
                mWorker.sendEmptyMessage(Worker.MSG_SAVE_AUTO_BOOT_DATA);
             }
         }

    };

    private class Worker extends Handler {
        private static final int MSG_SAVE_AUTO_BOOT_DATA = 1;

        public Worker(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SAVE_AUTO_BOOT_DATA:
                    mRecordHelper.saveAutoBootPackagesInfo();
                    break;
            }
        }
    }
}
