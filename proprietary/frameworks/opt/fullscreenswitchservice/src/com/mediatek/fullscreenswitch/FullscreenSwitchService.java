/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.fullscreenswitch;

import android.app.ActivityManager;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;
import android.text.TextUtils;
import android.util.Log;
import android.provider.Settings;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class FullscreenSwitchService extends IFullscreenSwitchManager.Stub {

    class H extends Handler {

        public static final int LOAD_ALL = 999999;
        public static final int APP_QUERY_ALL = 999998;
        public static final int APP_ADDED = 999997;
        public static final int APP_REMOVED = 999996;
        public static final int REFRESH = 999995;
        public static final int APP_MODE_UPDATED = 999994;

        public H(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.v(TAG, "handleMessage: entry what=" + msg.what);
            switch (msg.what) {
            case LOAD_ALL:
                refreshList();
                break;
            case APP_QUERY_ALL:
                loadAllPackages();
                break;
            case APP_ADDED:
                String addedPkgName = (String) msg.obj;
                if (addedPkgName != null) {
                    mFullscreenDataHelper.onPackageAdded(mPackageManager,
                            addedPkgName);
                }
                break;
            case APP_REMOVED:
                String removedPkgName = (String) msg.obj;
                if (removedPkgName != null) {
                    mFullscreenDataHelper.onPackageRemoved(mPackageManager,
                            removedPkgName);
                }
                break;
            case REFRESH:
                if (mModeCache.size() == 0) {
                    refreshList();
                }
                break;
            case APP_MODE_UPDATED:
                Integer id = (Integer) msg.obj;
                refreshModeCache(id);
                break;
            default:
                Log.e(TAG, "unknow message: " + msg.what);
                break;
            }
        }
    }

    private void loadAllPackages() {
        if (!isAppinfoLoaded()) {
            Log.v(TAG, "load app info at the 1st time!");
            setAppinfoLoaded(true);
            mFullscreenDataHelper.queryAppsAndInsert(mPackageManager);
        }
        if (mModeCache.size() == 0) {
            refreshList();
        }
    }

    private void refreshModeCache(int id) {
        FullscreenMode mode = mFullscreenDataHelper.getFullscreenMode(id);
        synchronized (mModeCache) {
            mModeCache.put(mode.packageName, mode);
        }
    }

    class PackageReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.v(TAG, "PackageReceiver receive: " + action);
            Uri appData = intent.getData();
            String pkgName = appData != null ? appData.getSchemeSpecificPart()
                    : null;
            if (Intent.ACTION_PACKAGE_ADDED.equals(action)) {
                if (pkgName != null) {
                    onPackageAdded(pkgName);
                }
            } else if (Intent.ACTION_PACKAGE_REMOVED.equals(action)) {
                if (pkgName != null) {
                    onPackageRemoved(pkgName);
                }
            }
        }

        private void onPackageRemoved(String pkgName) {
            Message msg = mBackgroundHander.obtainMessage(H.APP_REMOVED);
            msg.obj = pkgName;
            msg.sendToTarget();
        }

        private void onPackageAdded(String pkgName) {
            Message msg = mBackgroundHander.obtainMessage(H.APP_ADDED);
            msg.obj = pkgName;
            msg.sendToTarget();
        }
    }

    public static final String TAG = FullscreenSwitchService.class
            .getSimpleName();
    public static final boolean DEBUG = false;
    public static final String APP_LOAEED_NAME = "app_loaded_name";
    public static final int APP_LOAD_INIT_VALUE = 0;
    public static final int APP_LOADED_VALUE = 1;
    private static final long DELAY_REFRESH_TIME = 1000L * 25;

    public FullscreenDataHelper mFullscreenDataHelper;
    private Context mContext;
    protected HashMap<String, FullscreenMode> mModeCache;
    private HandlerThread mBackgroundThread;
    private Handler mBackgroundHander;
    private List<IFullscreenModeChangeListener> mModeChangeListeners;
    private final PackageManager mPackageManager;
    private final BroadcastReceiver mIntentReceiver;

    public FullscreenSwitchService(Context context) {
        Log.d(TAG, "FullscreenModeService ");

        this.mContext = context;
        mModeCache = new HashMap<String, FullscreenMode>();
        mBackgroundThread = new HandlerThread(
                "FullscreenModeService background thread");
        mBackgroundThread.start();
        mBackgroundHander = new H(mBackgroundThread.getLooper());
        mFullscreenDataHelper = new FullscreenDataHelper(this, context,
                mBackgroundHander);
        mModeChangeListeners = new ArrayList<IFullscreenModeChangeListener>();
        mPackageManager = context.getPackageManager();
        mIntentReceiver = new PackageReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
        intentFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        intentFilter.addDataScheme("package");
        context.registerReceiverAsUser(mIntentReceiver, UserHandle.ALL,
                intentFilter, null, mBackgroundHander);
        registerBootEvent();
        mBackgroundHander.sendEmptyMessageDelayed(H.LOAD_ALL,
                DELAY_REFRESH_TIME);
    }

    private void registerBootEvent() {
        IntentFilter filter = new IntentFilter(Intent.ACTION_BOOT_COMPLETED);
        filter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY);
        mContext.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.v(TAG, "BOOT_COMPLETED received");
                mBackgroundHander.sendEmptyMessage(H.APP_QUERY_ALL);
                mContext.unregisterReceiver(this);
            }
        }, filter);
    }

    public boolean isAppinfoLoaded() {
        return APP_LOAD_INIT_VALUE != Settings.System.getInt(
                mContext.getContentResolver(), APP_LOAEED_NAME,
                APP_LOAD_INIT_VALUE);
    }

    public void setAppinfoLoaded(boolean load) {
        Settings.System.putInt(mContext.getContentResolver(), APP_LOAEED_NAME,
                load ? APP_LOADED_VALUE : APP_LOAD_INIT_VALUE);
    }

    public void refreshList() {
        HashMap<String, FullscreenMode> map = mFullscreenDataHelper.loadModes();
        synchronized (mModeCache) {
            mModeCache = map;
            // reset load status if load apps info failed.
            if (mModeCache.size() == 0) {
                setAppinfoLoaded(false);
                Log.e(TAG, "load apps info failed!");
            }
        }
    }

    private void forceStopPackage(String packageName) {
        Log.d(TAG, "forceStopPackage " + packageName);
        final ActivityManager am = (ActivityManager) mContext
                .getSystemService(Context.ACTIVITY_SERVICE);
        am.forceStopPackage(packageName);
    }

    protected void getFullscreenModesAsync() {
        mBackgroundHander.removeMessages(H.LOAD_ALL);
        mBackgroundHander.sendEmptyMessage(H.LOAD_ALL);
    }

    protected void notifyAppListChanged() {
        for (IFullscreenModeChangeListener listener : mModeChangeListeners) {
            try {
                listener.onModeChanged();
            } catch (RemoteException e) {
                Log.e(TAG, "notifyAppListChanged" + e);
            }
        }
    }

    protected void notifyAppModeChanged(int id) {
        mBackgroundHander.removeMessages(H.APP_MODE_UPDATED);
        Message msg = mBackgroundHander.obtainMessage(H.APP_MODE_UPDATED);
        msg.obj = (Integer) id;
        msg.sendToTarget();
    }

    @Override
    public void registerModeChangeLister(IFullscreenModeChangeListener listener)
            throws RemoteException {
        if (!mModeChangeListeners.contains(listener)) {
            mModeChangeListeners.add(listener);
        } else {
            Log.e(TAG, "registerModeChangeLister: listener already registered "
                    + listener);
        }
    }

    @Override
    public void unregisterModeChangeLister(
            IFullscreenModeChangeListener listener) throws RemoteException {
        mModeChangeListeners.remove(listener);
    }

    /**
     * @param packageName
     *            of APP
     * @return True for default, false only APP is at 16:9 mode
     */
    @Override
    public int getFullscreenMode(String packageName) throws RemoteException {
        int result = FullscreenMode.APP_FULL_SCREEN_MODE;
        try {
            if (!TextUtils.isEmpty(packageName)) {
                FullscreenMode mode = mModeCache.get(packageName);
                if (mode != null) {
                    result = mode.mode;
                } else {
                    Log.e(FullscreenSwitchService.TAG, packageName
                            + " getFullscreenMode error, mode can't be found");
                }
            }
        } catch (Exception e) {
            Log.e(FullscreenSwitchService.TAG, packageName
                    + " getFullscreenMode error: " + e.getMessage());
        }
        return result;
    }

    @Override
    public boolean setFullscreenMode(String packageName, boolean mode)
            throws RemoteException {
        if (TextUtils.isEmpty(packageName)) {
            Log.e(TAG, "updateFullscreenMode, packageName is null!");
            return false;
        }

        boolean result = mFullscreenDataHelper.updateFullscreenMode(
                packageName, mode);
        if (result) {
            forceStopPackage(packageName);
        } else {
            Log.d(TAG, "updateFullscreenMode failed, package =  " + packageName
                    + " , mode =" + mode);
        }
        return result;
    }

    @Override
    public List<FullscreenMode> getAllScreenSwitchModels()
            throws RemoteException {
        if (mModeCache.size() == 0) {
            mBackgroundHander.sendEmptyMessage(H.LOAD_ALL);
        }
        List<FullscreenMode> list = new ArrayList();
        for (FullscreenMode mode : mModeCache.values()) {
            if (mode.type == FullscreenMode.TYPE_HAS_LAUNCHER_ACTIVITY) {
                list.add(mode);
            }
        }
        return list;
    }
}
