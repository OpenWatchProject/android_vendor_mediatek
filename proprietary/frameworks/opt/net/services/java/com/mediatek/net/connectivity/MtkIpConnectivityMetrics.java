/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.mediatek.net.connectivity;

import android.content.Context;
import android.net.IIpConnectivityMetrics;
import android.net.INetdEventCallback;
import android.net.Network;
import android.os.Binder;
import android.os.IBinder;
import android.os.Process;
import android.os.RemoteException;
import android.util.Log;
import android.util.SparseBooleanArray;

import com.android.server.connectivity.NetdEventListenerService;
import com.android.server.net.BaseNetdEventCallback;

import com.mediatek.server.MtkSystemServiceFactory;
import com.mediatek.server.powerhal.PowerHalManager;

/**
 * Implement Medaitek IpConnectivityMetrics service.
 * {@hide}
 *
*/
final public class MtkIpConnectivityMetrics {
    private static final String TAG = MtkIpConnectivityMetrics.class.getSimpleName();
    private static final boolean DBG = true;

    private NetdEventListenerService mNetdEventListenerService;

    private static final boolean FEATURE_SUPPORTED = true;

    /* The implementation of IpConnectivityMetrics */
    public Impl mImpl;

    private Context mContext;
    private PowerHalManager mPowerHalManager =
            MtkSystemServiceFactory.getInstance().makePowerHalManager();

    /* Construction function of MtkIpConnectivityMetrics. */
    public MtkIpConnectivityMetrics(Context ctx, NetdEventListenerService service) {
        Log.d(TAG, "MtkIpConnectivityMetrics is created:" + FEATURE_SUPPORTED);
        mContext = ctx;
        mNetdEventListenerService = service;
        if (FEATURE_SUPPORTED) {
            mImpl = new Impl(mContext);
            try {
                mNetdEventListenerService.addNetdEventCallback(
                        INetdEventCallback.CALLBACK_CALLER_CTA_NETWORK_MONITOR,
                        mNetdEventListener);
            } catch(Exception e) {
                Log.e(TAG, "MtkIpConnectivityMetrics addNetdEventCallback:" + e);
            }
        }
    }

    public IBinder getMtkIpConnSrv() {
        return (IBinder) mImpl;
    }

    private final INetdEventCallback mNetdEventListener = new BaseNetdEventCallback() {

        // Called concurrently by multiple binder threads.
        // This method must not block or perform long-running operations.
        @Override
        public void onDnsEvent(int netId, int eventType, int returnCode, String hostname,
               String[] ipAddresses, int ipAddressesCount, long timestamp, int uid) {
            mImpl.onCtaDnsEvent(netId, uid);
            mImpl.onMonitorDnsEvent(netId, eventType, returnCode, hostname, ipAddressesCount, uid);
        }

        @Override
        public synchronized void onConnectEvent(String ipAddr, int port, long timestamp, int uid) {
            mImpl.onCtaConnectEvent(uid);
            mImpl.onMonitorConnectEvent(uid);
        }
    };


    public final class Impl extends IMtkIpConnectivityMetrics.Stub {
        private Context mContext;
        private INetdEventCallback mNetdEventCallback;
        private INetdEventCallback mSocketEventCallback;
        private SparseBooleanArray mUidSocketRules =
                    new SparseBooleanArray();
        final Object mUidSockeRulestLock = new Object();

        /* Construction function. */
        public Impl(Context ctx) {
            mContext = ctx;
        }

        @Override
        public boolean registerMtkNetdEventCallback(INetdEventCallback callback) {
            // Check package name
            if (isPermissionAllowed()) {
                Log.d(TAG, "registerMtkNetdEventCallback");
                mNetdEventCallback = callback;
                return true;
            }
            return false;
        }

        @Override
        public boolean unregisterMtkNetdEventCallback() {
            if (isPermissionAllowed()) {
                Log.d(TAG, "unregisterMtkNetdEventCallback");
                mNetdEventCallback = null;
                return true;
            }
            return false;
        }

        @Override
        public boolean registerMtkSocketEventCallback(INetdEventCallback callback) {
            // Check package name
            if (isPermissionAllowed()) {
                Log.d(TAG, "registerMtkSocketEventCallback");
                mSocketEventCallback = callback;
                return true;
            }
            return false;
        }

        @Override
        public boolean unregisterMtkSocketEventCallback() {
            if (isPermissionAllowed()) {
                Log.d(TAG, "unregisterMtkSocketEventCallback");
                mSocketEventCallback = null;
                return true;
            }
            return false;
        }

        @Override
        public void updateCtaAppStatus(int uid, boolean isNotified) {
            if (isPermissionAllowed()) {
                if (uid < Process.FIRST_APPLICATION_UID) {
                    return;
                }
                synchronized (mUidSockeRulestLock) {
                    Log.d(TAG, "updateCtaAppStatus:" + uid + ":" + isNotified);
                    mUidSocketRules.put(uid, isNotified);
                }
            }
        }

        @Override
        public void setSpeedDownload(int timeoutMs) {
            if (mPowerHalManager != null) {
                Log.d(TAG, "setSpeedDownload:" + timeoutMs);
                mPowerHalManager.setSpeedDownload(timeoutMs);
            }
        }

        private void onCtaDnsEvent(int netId, int uid) {
            if (mNetdEventCallback == null) return;
            if (uid < android.os.Process.FIRST_APPLICATION_UID) {
                return;
            }

            boolean isNotified = true;
            synchronized (mUidSockeRulestLock) {
                isNotified = mUidSocketRules.get(uid, true);
                if (DBG) Log.d(TAG, "onDnsEvent:" + "uid=" + uid + ", netId=" + netId +
                        ", isNotified=" + isNotified);
                if (isNotified) {
                    try {
                        mNetdEventCallback.onDnsEvent(netId, 0, 0, "", null, 0, 0L, uid);
                    } catch (Exception e) {
                        Log.d(TAG, "onCtaDnsEvent:" + e);
                    }
                }
            }
        }

        private void onCtaConnectEvent(int uid) {
            if (mNetdEventCallback == null) return;

            if (uid < android.os.Process.FIRST_APPLICATION_UID) {
                return;
            }
            boolean isNotified = true;
            synchronized (mUidSockeRulestLock) {
                isNotified = mUidSocketRules.get(uid, true);
                if (DBG) Log.d(TAG, "onDnsEvent:" + uid + ":" + isNotified);
                if (isNotified) {
                    try {
                        mNetdEventCallback.onConnectEvent("", 0, 0L, uid);
                    } catch (Exception e) {
                        Log.d(TAG, "onCtaConnectEvent:" + e);
                    }
                }
            }
        }

        private void onMonitorDnsEvent(int netId, int eventType, int returnCode, String hostname,
                int ipAddressesCount, int uid) {
            if (mSocketEventCallback == null) return;
            try {
                mSocketEventCallback.onDnsEvent(netId, eventType, returnCode,hostname, null,
                        ipAddressesCount, 0L, uid);
            } catch (Exception e) {
                Log.d(TAG, "onMonitorDnsEvent:" + e);
            }
        }

        private void onMonitorConnectEvent(int uid) {
            if (mSocketEventCallback == null) return;
            try {
                mSocketEventCallback.onConnectEvent("", 0, 0L, uid);
            } catch (Exception e) {
                Log.d(TAG, "onMonitorConnectEvent:" + e);
            }
        }

        private boolean isPermissionAllowed() {
            enforceNetworkMonitorPermission();
            if (Binder.getCallingUid() != Process.SYSTEM_UID) {
                Log.d(TAG, "No permission:" + Binder.getCallingUid());
                return false;
            }
            return true;
        }

        private void enforceNetworkMonitorPermission() {
            final int uid = Binder.getCallingUid();
            if (uid != Process.SYSTEM_UID) {
                throw new SecurityException(String.format("Uid %d has no permission to change watchlist"
                        + " setting.", uid));
            }
        }
    }
}
