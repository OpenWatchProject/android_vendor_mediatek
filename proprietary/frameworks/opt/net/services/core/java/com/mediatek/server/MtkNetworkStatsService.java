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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.server;

import static android.net.NetworkStats.TAG_ALL;
import static android.net.NetworkStats.TAG_NONE;
import static android.net.NetworkStats.UID_ALL;
import static android.net.NetworkStats.STATS_PER_UID;
import static android.net.NetworkStats.STATS_PER_IFACE;
import static android.net.TrafficStats.MB_IN_BYTES;
import static android.net.TrafficStats.PB_IN_BYTES;
import static android.Manifest.permission.READ_NETWORK_USAGE_HISTORY;
import static android.provider.Settings.Global.NETSTATS_GLOBAL_ALERT_BYTES;

import android.app.AlarmManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkIdentity;
import android.net.NetworkStats;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.INetworkManagementService;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.TelephonyManager;
import android.util.ArrayMap;
import android.util.Slog;
import java.time.Clock;

import static com.android.internal.telephony.TelephonyIntents.ACTION_BACKGROUND_MOBILE_DATA_USAGE;

import com.android.server.net.NetworkStatsFactory;
import com.android.server.net.NetworkStatsObservers;
import com.android.server.net.NetworkStatsService;
import com.android.server.net.NetworkStatsService.NetworkStatsSettings;

import com.mediatek.telephony.MtkTelephonyManagerEx;
import java.io.File;
import java.util.List;
import java.util.HashMap;

public class MtkNetworkStatsService extends NetworkStatsService {
    private static final String TAG = MtkNetworkStatsService.class.getSimpleName();

    private static final int SUBSCRIPTION_OR_SIM_CHANGED = 0;
    private static final int PHONE_STATE_CHANGED = 1;
    private static final int UPDATE_LATENCY_STATS = 2;
    private static final int REMOVE_PHONE_STATE_LISTENER = 3;

    private static final int PHONE_MESSAGE_DELAYED = 10000;
    private int mPhoneState = TelephonyManager.CALL_STATE_IDLE;
    private volatile NetworkStats mXtVtDataUsage;
    private volatile NetworkStats mUidVtDataUsage;
    private final Object mXtUidStatsLock = new Object();
    private HashMap<Integer, StatsPhoneStateListener> mPhoneStateListeners =
            new HashMap<Integer, StatsPhoneStateListener>();

    private Context mContext;
    private HandlerThread mHandlerThread;
    private InternalHandler mHandler;
    private long mEmGlobalAlert = 2 * MB_IN_BYTES;
    private NetworkStats mLatencyStats;
    private final Object mLatencyStatsLock = new Object();

    public MtkNetworkStatsService(Context context, INetworkManagementService networkManager,
            AlarmManager alarmManager, PowerManager.WakeLock wakeLock, Clock clock,
            TelephonyManager teleManager, NetworkStatsSettings settings,
            NetworkStatsObservers statsObservers, File systemDir, File baseDir) {
        super(context, networkManager, alarmManager, wakeLock, clock, teleManager,
            settings, statsObservers, systemDir, baseDir);
        Slog.d(TAG, "MtkNetworkStatsService starting up");
        mContext = context;
        initDataUsageIntent(context);
    }

    private void initDataUsageIntent(Context context) {
        mHandlerThread = new HandlerThread("NetworkStatInternalHandler");
        mHandlerThread.start();
        mHandler = new InternalHandler(mHandlerThread.getLooper());

        SubscriptionManager.from(context)
            .addOnSubscriptionsChangedListener(mOnSubscriptionsChangedListener);
    }


    // M: Add support for Multiple ViLTE
    protected void rebuildActiveVilteIfaceMap() {
        //Do not use subId anymore.
    }

    // return false to use AOSP procedure
    protected boolean findOrCreateMultipleVilteNetworkIdentitySets(NetworkIdentity vtIdent) {
        // Also check MtkImsPhoneCallTracker for interface modification
        findOrCreateNetworkIdentitySet(mActiveIfaces,
            getVtInterface(vtIdent.getSubscriberId())).add(vtIdent);
        findOrCreateNetworkIdentitySet(mActiveUidIfaces,
            getVtInterface(vtIdent.getSubscriberId())).add(vtIdent);
        return true;
    }

    private String getVtInterface(String subscribeId) {
        return VT_INTERFACE + subscribeId;
    }

    // M: ALPS04668496 improve network performance
    private BroadcastReceiver mMobileReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (ACTION_BACKGROUND_MOBILE_DATA_USAGE.equals(intent.getAction())) {
                Slog.i(TAG, "mMobileReceiver, update LatencyStats");
                mHandler.sendEmptyMessage(UPDATE_LATENCY_STATS);
            }
        }
    };

    @Override
    public void systemReady() {
        super.systemReady();
        final IntentFilter mobileFilter = new IntentFilter(ACTION_BACKGROUND_MOBILE_DATA_USAGE);
        mContext.registerReceiver(mMobileReceiver,
                                  mobileFilter, READ_NETWORK_USAGE_HISTORY, mHandler);

        mHandler.sendEmptyMessage(SUBSCRIPTION_OR_SIM_CHANGED);
    }

    @Override
    protected void shutdownLocked() {
        super.shutdownLocked();
        mContext.unregisterReceiver(mMobileReceiver);
        mHandler.sendEmptyMessage(REMOVE_PHONE_STATE_LISTENER);
    }

    @Override
    protected NetworkStats getNetworkStatsUidDetail(String[] ifaces) throws RemoteException {
        // TODO: remove 464xlat adjustments from NetworkStatsFactory and apply all at once here.
        final NetworkStats uidSnapshot = mNetworkManager.getNetworkStatsUidDetail(UID_ALL,
                ifaces);

        // fold tethering stats and operations into uid snapshot
        final NetworkStats tetherSnapshot = getNetworkStatsTethering(STATS_PER_UID);
        tetherSnapshot.filter(UID_ALL, ifaces, TAG_ALL);
        NetworkStatsFactory.apply464xlatAdjustments(uidSnapshot, tetherSnapshot,
                mUseBpfTrafficStats);
        uidSnapshot.combineAllValues(tetherSnapshot);

        // fold video calling data usage stats into uid snapshot
        final NetworkStats vtStats = getVtDataUsageInternal(STATS_PER_UID);
        if (vtStats != null) {
            vtStats.filter(UID_ALL, ifaces, TAG_ALL);
            NetworkStatsFactory.apply464xlatAdjustments(uidSnapshot, vtStats,
                    mUseBpfTrafficStats);
            uidSnapshot.combineAllValues(vtStats);
        }

        uidSnapshot.combineAllValues(mUidOperations);

        final NetworkStats latencyStats = getLatencyStats();
        if (latencyStats != null) {
            latencyStats.filter(UID_ALL, ifaces, TAG_ALL);
            NetworkStatsFactory.apply464xlatAdjustments(uidSnapshot, latencyStats,
                    mUseBpfTrafficStats);
            uidSnapshot.combineAllValues(latencyStats);
        }
        uidSnapshot.combineAllValues(mUidOperations);
        return uidSnapshot;
    }

    @Override
    protected NetworkStats getNetworkStatsXt() throws RemoteException {
        final NetworkStats xtSnapshot = mNetworkManager.getNetworkStatsSummaryXt();

        final NetworkStats vtSnapshot = getVtDataUsageInternal(STATS_PER_IFACE);
        if (vtSnapshot != null) {
            xtSnapshot.combineAllValues(vtSnapshot);
        }

        final NetworkStats latencyStats = getLatencyStats();
        if (latencyStats != null) {
            xtSnapshot.combineAllValues(latencyStats);
        }
        return xtSnapshot;
    }

    private void updateLatencyStats() {
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        int phoneId = SubscriptionManager.getPhoneId(subId);
        NetworkStats stats = MtkTelephonyManagerEx.getDefault().getMobileDataUsage(phoneId);
        NetworkStats latencyStats = null;

        synchronized (mLatencyStatsLock) {
            if (mLatencyStats != null) {
                latencyStats = mLatencyStats.clone();
            }
        }

        if (latencyStats != null && stats != null) {
            for (int i=0; i<stats.size(); i++) {
                NetworkStats.Entry entry = stats.getValues(i, null);
                int index = latencyStats.findIndex(entry.iface, entry.uid, entry.set, entry.tag,
                        entry.metered, entry.roaming, entry.defaultNetwork);
                if (index == -1) {
                    continue;
                }
                NetworkStats.Entry hentry = latencyStats.getValues(index, null);

                if (entry.txBytes < hentry.txBytes) {
                    Slog.e(TAG, "updateLatencyStats found nagative netstats!"
                                + "iface = " + entry.iface
                                + "entry.txBytes = " + entry.txBytes
                                + "hentry.txBytes = " + hentry.txBytes);
                    MtkTelephonyManagerEx.getDefault().setMobileDataUsageSum(phoneId,
                            hentry.txBytes, hentry.txPackets, hentry.rxBytes,
                            hentry.rxPackets);
                    stats = MtkTelephonyManagerEx.getDefault().getMobileDataUsage(phoneId);
                }
            }
        }
        Slog.i(TAG, "updateLatencyStats subId:" + subId + ", phoneId:" + phoneId
                    + ", NetworkStats : " + stats);
        synchronized (mLatencyStatsLock) {
            mLatencyStats = stats;
        }
    }

    private NetworkStats getLatencyStats() {
        NetworkStats latencyStats = null;
        synchronized (mLatencyStatsLock) {
            if (mLatencyStats != null) {
                latencyStats = mLatencyStats.clone();
            }
        }
        return latencyStats;
    }
    // end

    // M: ALPS04924086 fix getVtDataUsage SWT
    private void getVtDataUsageFromTelephony() {
        NetworkStats xtVtDataUsage = new NetworkStats(SystemClock.elapsedRealtime(), 10);
        NetworkStats uidVtDataUsage = new NetworkStats(SystemClock.elapsedRealtime(), 10);
        for (Integer subId : mPhoneStateListeners.keySet()) {
            StatsPhoneStateListener listener = mPhoneStateListeners.get(subId);
            NetworkStats stats = listener.getTelephonyManager().getVtDataUsage(STATS_PER_IFACE);
            if (stats != null) {
                xtVtDataUsage.combineAllValues(stats);
            }
            Slog.d(TAG, "getVtDataUsage, IFACE, subId = " + subId + ", stas = " + stats);
            stats = listener.getTelephonyManager().getVtDataUsage(STATS_PER_UID);
            if (stats != null) {
                uidVtDataUsage.combineAllValues(stats);
            }
        }

        synchronized (mXtUidStatsLock) {
            mXtVtDataUsage = xtVtDataUsage;
            mUidVtDataUsage = uidVtDataUsage;
        }
    }

    private NetworkStats getVtDataUsageInternal(int how) {
        NetworkStats stats = null;
        synchronized (mXtUidStatsLock) {
            if (how == STATS_PER_IFACE) {
                if (mXtVtDataUsage != null) {
                    stats = mXtVtDataUsage.clone();
                }
            } else if (how == STATS_PER_UID) {
                if (mUidVtDataUsage != null) {
                    stats = mUidVtDataUsage.clone();
                }
            } else {
                Slog.e(TAG, "getVtDataUsageInternal, invailed how = " + how);
            }
        }
        return stats;
    }

    private void registeAndUpdateStateListener() {
        Slog.d(TAG, "registerStateListener");
        SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        List<SubscriptionInfo> infos =
                subscriptionManager.getActiveSubscriptionInfoList();
        if (infos == null) {
            return;
        }

        //We should remove all the item in Listener, and then add the listener again.
        removePhoneStateListener();

        final TelephonyManager tm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        for (SubscriptionInfo info : infos) {
            int subId = info.getSubscriptionId();
            if (!mPhoneStateListeners.containsKey(subId)) {
                TelephonyManager telephonyManager =
                        tm.createForSubscriptionId(subId);
                StatsPhoneStateListener listener =
                        new StatsPhoneStateListener(mHandlerThread.getLooper(), telephonyManager);
                mPhoneStateListeners.put(subId, listener);
                telephonyManager.listen(listener, PhoneStateListener.LISTEN_CALL_STATE);
            }
        }
    }

    private void removePhoneStateListener() {
        for (Integer id : mPhoneStateListeners.keySet()) {
            int subId = id.intValue();
            StatsPhoneStateListener listener = mPhoneStateListeners.get(id);
            listener.getTelephonyManager().listen(listener, PhoneStateListener.LISTEN_NONE);
        }
        mPhoneStateListeners.clear();
    }

    private class StatsPhoneStateListener extends PhoneStateListener {
        private TelephonyManager telephonyManager;
        public StatsPhoneStateListener(Looper looper, TelephonyManager telephonyManager) {
            super(looper);
            this.telephonyManager = telephonyManager;
        }

        @Override
        public void onCallStateChanged(@TelephonyManager.CallState int state, String phoneNumber) {
            Slog.i(TAG, "onCallStateChanged state:" + state);
            mPhoneState = state;
            if (mPhoneState == TelephonyManager.CALL_STATE_OFFHOOK) {
                mHandler.sendEmptyMessageDelayed(PHONE_STATE_CHANGED,
                                                 PHONE_MESSAGE_DELAYED);
            }
        }

        public TelephonyManager getTelephonyManager() {
            return telephonyManager;
        }
    }
    //end

    private class InternalHandler extends Handler {
        public InternalHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SUBSCRIPTION_OR_SIM_CHANGED:
                    handleSimChange();
                    registeAndUpdateStateListener();
                    break;
                case PHONE_STATE_CHANGED:
                    getVtDataUsageFromTelephony();
                    if (mPhoneState == TelephonyManager.CALL_STATE_OFFHOOK) {
                        mHandler.sendEmptyMessageDelayed(PHONE_STATE_CHANGED,
                                                         PHONE_MESSAGE_DELAYED);
                    }
                    break;
                case UPDATE_LATENCY_STATS:
                    updateLatencyStats();
                    break;
                case REMOVE_PHONE_STATE_LISTENER:
                    removePhoneStateListener();
                    break;
                default:
                    break;
            }
        }
    }

    private void handleSimChange() {
        boolean isTestSim = isTestSim();
        mEmGlobalAlert = Settings.Global.getLong(mContext.getContentResolver(),
                NETSTATS_GLOBAL_ALERT_BYTES, 0);
        if (isTestSim) {
            if (mEmGlobalAlert != 2 * PB_IN_BYTES) {
                Settings.Global.putLong(mContext.getContentResolver(),
                    NETSTATS_GLOBAL_ALERT_BYTES, 2 * PB_IN_BYTES);
                advisePersistThreshold(Long.MAX_VALUE / 1000);
                Slog.d(TAG, "Configure for test sim with 2TB");
            }
        } else {
            if (mEmGlobalAlert == 2 * PB_IN_BYTES) {
                Settings.Global.putLong(mContext.getContentResolver(),
                    NETSTATS_GLOBAL_ALERT_BYTES, 2 * MB_IN_BYTES);
                advisePersistThreshold(Long.MAX_VALUE / 1000);
                Slog.d(TAG, "Restore for test sim with 2MB");
            }
        }
    }

    private final OnSubscriptionsChangedListener mOnSubscriptionsChangedListener =
            new OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            mHandler.sendEmptyMessage(SUBSCRIPTION_OR_SIM_CHANGED);
        }
    };

    public static boolean isTestSim() {
        boolean isTestSim = false;
        isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
        return isTestSim;
    }

}
