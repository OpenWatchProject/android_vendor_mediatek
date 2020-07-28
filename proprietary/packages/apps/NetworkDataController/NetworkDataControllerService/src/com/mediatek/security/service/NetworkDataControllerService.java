/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.security.service;

import android.app.ActivityManager;
import android.app.ActivityManager.AppTask;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.ActivityManager.RunningTaskInfo;
import android.app.ActivityManagerNative;
import android.app.ActivityThread;
import android.app.IAppTask;
import android.app.AlertDialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.util.Pair;

import android.net.INetdEventCallback;
import android.Manifest;
import android.net.Uri;

import com.mediatek.net.connectivity.IMtkIpConnectivityMetrics;
import com.mediatek.security.service.R;
import com.mediatek.security.service.Log;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.security.datamanager.CheckedPermRecord;
import com.mediatek.security.datamanager.DatabaseManager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.List;

public class NetworkDataControllerService extends AsyncService {

    private static final String TAG = "NetworkDataControllerService";
    private final String SET_POLICY = "vendor.moms.permission.control.policy.set";
    private final List<String> mPackageManagerActions = Arrays.asList(
            Intent.ACTION_PACKAGE_ADDED, Intent.ACTION_PACKAGE_REMOVED,
            Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE,
            Intent.ACTION_EXTERNAL_APPLICATIONS_UNAVAILABLE);
    private NetworkDataAlertDialog mNetworkDataAlertDialog;
    private DatabaseManager mDatabaseManager = new DatabaseManager();
    private boolean mIsRegisted = false;

    private static final String START_SERVICE_REQUEST = "com.mediatek.security.START_SERVICE";
    private AtomicBoolean mPendingCellular = new AtomicBoolean(false);
    private IMtkIpConnectivityMetrics mIpConnectivityMetrics;

    private BroadcastReceiver mPackageChangedReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }
            String action = intent.getAction();
            if (mPackageManagerActions.indexOf(action) != -1) {
                handleMessage(MSG_PACKAGE_CHANGED, intent);
            }
        }
    };

    private INetdEventCallback mNetdEventCallback = new INetdEventCallback.Stub() {

        private synchronized void handleNetdEvent(int uid) {
            Log.i(TAG, "mNetdEventCallback,handleNetdEvent uid: " + uid
                    + "mPendingCellular: " + mPendingCellular);
            if (!(UserHandle.myUserId() == ActivityManager.getCurrentUser())) {
                Log.i(TAG, "new user id, no need notify user");
                PermControlUtils.updateCtaAppStatus(uid, false);
                return;
            }
            if ((!mPendingCellular.getAndSet(true))
                    && (uid >= Process.FIRST_APPLICATION_UID)) {
                handleMessage(NETWORK_DATA_REQUEST, uid);
                PermControlUtils.updateCtaAppStatus(uid, false);
                Log.i(TAG, "handle:NETWORK_DATA_REQUEST, mPendingCellular: " + mPendingCellular);
            } else {
                Log.i(TAG, "do nothing");
            }
        }

        public void onDnsEvent(int netId, int eventType, int returnCode,
            String hostname, String[] ipAddress,int ipAddressesCount, long timeStamp, int uid) {
            Log.i(TAG, "onDnsEvent uid:" + uid);
            handleNetdEvent(uid);
        }

        @Override
        public void onConnectEvent(String ipAddr, int port, long timestamp,
                int uid) {
            Log.i(TAG, "onConnectEvent uid:" + uid);
            handleNetdEvent(uid);
        }

        @Override
        public void onPrivateDnsValidationEvent(int uid, String ipAddress,
                String hostname, boolean validated) {
            Log.i(TAG, "onPrivateDnsValidationEvent uid:" + uid);
        }

        @Override
        public void onNat64PrefixEvent(int netId, boolean added, String prefixString,
            int prefixLength){
            Log.i(TAG, "onNat64PrefixEvent");
        }
    };

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_SHOW_CONF_DLG) {
                handleConfirmDlgMsg();
            } else {
                Log.e(TAG, "handleMessage unkown:" + msg.what);
            }
        }
    };

    public NetworkDataControllerService() {
        super("NetworkDataControllerService");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate()");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return new PermControlBinder();
    }

    @Override
    protected void onHandleMessage(Message message) {
        Log.d(TAG, "onHandleMessage: " + message.what);
        switch (message.what) {
        case MSG_INTENT:
            Log.d(TAG, "onHandleMessage MSG_INTENT");
            initService();
            break;

        case NETWORK_DATA_REQUEST:
            Log.d(TAG, "onHandleMessage NETWORK_DATA_REQUEST");
            handleCellularDataRequest((int) message.obj);
            break;

        case NETWORK_DATA_MODIFY:
            Log.d(TAG, "onHandleMessage NETWORK_DATA_MODIFY");
            CheckedPermRecord record = (CheckedPermRecord) message.obj;
            // get old record before it changed
            CheckedPermRecord oldRecord = mDatabaseManager
                    .getNetworkDataRecord(record.mUid);
            if ((oldRecord != null) && mDatabaseManager.updateNetworkDataRecord(record)) {
                PermControlUtils.setFirewallPolicy(
                        NetworkDataControllerService.this, record.mUid,
                        oldRecord.getStatus(), record.getStatus());
                sendNetworkUpdateBroadcast();
            }
            break;
        case MSG_PACKAGE_CHANGED:
            Log.d(TAG, "onHandleMessage MSG_PACKAGE_CHANGED");
            Intent PackageChangedIntent = (Intent) message.obj;
            HandlePackageChangedEvent(PackageChangedIntent);
            break;
        default:
            Log.e(TAG, "do not know the message " + message.what);
            initService();
        }
    }

    private String getPackageName(Intent intent) {
        Uri uri = intent.getData();
        String pkg = uri != null ? uri.getSchemeSpecificPart() : null;
        return pkg;
    }

    public void HandlePackageChangedEvent(Intent PackageChangedIntent) {
        String action = PackageChangedIntent.getAction();
        Log.d(TAG, "HandlePackageChangedEvent, action: " + action);
        int uid = -1;
        String pkgName = null;
        String[] pkgList = null;
        int[] uidList = null;
        switch (action) {
        case Intent.ACTION_PACKAGE_ADDED:
            uid = PackageChangedIntent.getIntExtra(Intent.EXTRA_UID, 0);
            pkgName = getPackageName(PackageChangedIntent);
            addPackage(uid, pkgName);
            break;

        case Intent.ACTION_PACKAGE_REMOVED:
            uid = PackageChangedIntent.getIntExtra(Intent.EXTRA_UID, 0);
            pkgName = getPackageName(PackageChangedIntent);
            removePackage(uid, pkgName);
            break;

        case Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE:
            pkgList = PackageChangedIntent
                    .getStringArrayExtra(Intent.EXTRA_CHANGED_PACKAGE_LIST);
            uidList = PackageChangedIntent
                    .getIntArrayExtra(Intent.EXTRA_CHANGED_UID_LIST);
            if (pkgList != null && uidList != null) {
                int i = 0;
                for (String packagename : pkgList) {
                    addPackage(uidList[i], packagename);
                    i++;
                }
            }
            break;
        case Intent.ACTION_EXTERNAL_APPLICATIONS_UNAVAILABLE:
            pkgList = PackageChangedIntent
                    .getStringArrayExtra(Intent.EXTRA_CHANGED_PACKAGE_LIST);
            uidList = PackageChangedIntent
                    .getIntArrayExtra(Intent.EXTRA_CHANGED_UID_LIST);
            if (pkgList != null && uidList != null) {
                int i = 0;
                for (String packagename : pkgList) {
                    removePackage(uidList[i], packagename);
                    i++;
                }
            }
            break;

        default:
            Log.e(TAG, "do not know the action, do noting");
        }
        sendNetworkUpdateBroadcast();
    }

    private boolean addPackage(int uid, String packageName) {
        Log.d(TAG, " addPackage,  uid: " + uid + "; packageName: "
                + packageName);
        if (mDatabaseManager.add(uid, packageName)) {
            PermControlUtils.setFirewallPolicy(this, uid, -1,
                    CheckedPermRecord.STATUS_FIRST_CHECK);
            PermControlUtils.updateCtaAppStatus(uid, true);
        }
        return true;
    }

    private boolean removePackage(int uid, String packageName) {
        Log.d(TAG, " removePackage,  uid: " + uid + "; packageName: "
                + packageName);
        CheckedPermRecord permRecord =  mDatabaseManager.getNetworkDataRecord(uid);
        if(permRecord == null) {
            return false;
        }
        int oldStatus = permRecord.getStatus();
        if (mDatabaseManager.delete(uid, packageName)) {
            PermControlUtils.setFirewallPolicy(this, uid, oldStatus,
                    CheckedPermRecord.STATUS_GRANTED);
        }
        return true;
    }

    @Override
    public synchronized void onDestroy() {
        super.onDestroy();
        Log.d(TAG, " onDestroy");
        if (mIsRegisted) {
            unregisterReceiver(mPackageChangedReceiver);
            mIsRegisted = false;
        }
        if (mIpConnectivityMetrics != null) {
            try {
                mIpConnectivityMetrics.unregisterMtkNetdEventCallback();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    private IMtkIpConnectivityMetrics getIMtkIpConnectivityMetrics() {
        return IMtkIpConnectivityMetrics.Stub.asInterface(ServiceManager
                .getService("mtkconnmetrics"));
    }

    private synchronized void initService() {
        Log.d(TAG, "initService");
        initDatabaseManager();
        try {
            mIpConnectivityMetrics = getIMtkIpConnectivityMetrics();
            mIpConnectivityMetrics
                    .registerMtkNetdEventCallback(mNetdEventCallback);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        registerReceiver();
        int system_status = mDatabaseManager.getNetworkDataRecord(1000)
                .getStatus();
        int phone_status = mDatabaseManager.getNetworkDataRecord(1001)
                .getStatus();
        Log.d(TAG, "system_status: " + system_status + "; phone_status: "
                + phone_status);
        if (!(system_status == CheckedPermRecord.STATUS_GRANTED
            && phone_status == CheckedPermRecord.STATUS_GRANTED)) {
            displaySystemAppRequestDlg(this);
        }
    }

    private synchronized void initDatabaseManager() {
        mDatabaseManager.initDataBase(this);
        setInitPolicy();
    }

    private void setInitPolicy() {
        boolean hasSet = (SystemProperties.getInt(SET_POLICY, 0) == 1);
        Log.d(TAG, "hasSet = " + hasSet);
        if (!hasSet) {
            // set all Network Data control policy
            List<CheckedPermRecord> recordList = mDatabaseManager
                    .getNetworkDataRecordList();
            if(recordList == null || recordList.size() ==0) {
                return;
            }
            for (CheckedPermRecord record : recordList) {

                PermControlUtils.setFirewallPolicy(this, record.mUid, -1,
                        record.getStatus());

                // set to network callback inform again.
                if (record.getStatus() == CheckedPermRecord.STATUS_GRANTED) {
                    PermControlUtils.updateCtaAppStatus(record.mUid, false);
                } else {
                    PermControlUtils.updateCtaAppStatus(record.mUid, true);
                }
            }
            SystemProperties.set(SET_POLICY, "1");
        }
    }

    private void registerReceiver() {
        if (!mIsRegisted) {
            IntentFilter pkgChangedIntent = new IntentFilter();
            for (String action : mPackageManagerActions) {
                pkgChangedIntent.addAction(action);
            }
            pkgChangedIntent.addDataScheme("package");
            registerReceiver(mPackageChangedReceiver, pkgChangedIntent);
            Log.d(TAG, "registerReceiver");
            mIsRegisted = true;
        }
    }

    private void sendNetworkUpdateBroadcast() {
        Intent intent = new Intent(PermControlUtils.NETWORK_DATA_UPDATE);
        sendBroadcast(intent);
    }

    private void displaySystemAppRequestDlg(Context context) {
        Intent intent = new Intent("com.mediatek.security.REVIEW_PERMISSIONS");
        intent.setClassName("com.mediatek.security",
                "com.mediatek.security.SystemPermissionRequestActivity");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    /*
     * Because the system dialog need to show in main thread of service so show
     * the dialog via a handler
     */
    private void showConfirmDlg() {
        Message msg = Message.obtain();
        msg.what = MSG_SHOW_CONF_DLG;
        mHandler.sendMessage(msg);
    }

    /*
     * Synchronized the function of handleCheckCase, whenever one permission
     * confirm thread hold the lock other permission thread need to wait
     * previous release otherwise wait
     */
    private synchronized void handleCheckCase(CheckedPermRecord record, int uid) {
        synchronized (PermControlUtils.mUserConfirmLock) {
            try{
                mNetworkDataAlertDialog = new NetworkDataAlertDialog(this, record,
                    new PermControlBinder());
                showConfirmDlg();
                PermControlUtils.mUserConfirmLock.wait();
            } catch (InterruptedException e) {
                Log.d(TAG, "InterruptedException");
            }
        }
    }

    private void handleCellularDataRequest(int uid) {
        String packageName = null;
        CheckedPermRecord record = null;
        try {
            if (uid != -1) {
                record = mDatabaseManager.getNetworkDataRecord(uid);
            }
            if (record == null
                    || record.getStatus() == CheckedPermRecord.STATUS_DENIED
                    || record.getStatus() == CheckedPermRecord.STATUS_GRANTED) {

                if (record == null) {
                    Log.e(TAG, "record == null");
                } else {
                    Log.e(TAG, "status == " + record.getStatus());
                }
                return;
            }
            handleCheckCase(record, uid);
        } finally {
            mPendingCellular.set(false);
            Log.e(TAG, "mPendingCellular: " + mPendingCellular);
        }
    }

    /**
     * Show a system confirm dialog from service
     *
     */

    private void handleConfirmDlgMsg() {
        Log.d(TAG, "Show confirm dialog");
        mNetworkDataAlertDialog.ShowDialog();
    }

    class PermControlBinder extends INetworkDataControllerService.Stub {

        /**
         * Check caller of INetworkDataControlService only applications whitch
         * share system uid can call it.
         */
        public boolean checkCaller() {
            String callingApp = getPackageManager().getNameForUid(
                    Binder.getCallingUid());
            return (("com.mediatek.security".equals(callingApp))
                || ("android.uid.system:1000".equals(callingApp)));
        }

        /**
         * Need write data base so send msg to sync thread to handle
         *
         */
        @Override
        public boolean modifyNetworkDateRecord(CheckedPermRecord record) {
            if (!checkCaller()) {
                Log.d(TAG, "Check caller fail");
                return false;
            }
            handleMessage(NETWORK_DATA_MODIFY, record);
            return true;
        }

        @Override
        public CheckedPermRecord getNetworkDataRecord(int uid) {
            if (!checkCaller()) {
                Log.d(TAG, "Check caller fail");
                return null;
            }
            return mDatabaseManager.getNetworkDataRecord(uid);
        }

        @Override
        public List<CheckedPermRecord> getNetworkDataRecordList() {
            if (!checkCaller()) {
                Log.d(TAG, "Check caller fail");
                return null;
            }
            return mDatabaseManager.getNetworkDataRecordList();
        }
    }
}
