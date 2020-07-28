/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
package com.mediatek.powerhalservice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Message;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.util.Log;


import android.os.IRemoteCallback;
import com.mediatek.powerhalwrapper.PowerHalWrapper;

public class PowerHalWifiMonitor {

    private final String TAG = "PowerHalWifiMonitor";

    // Handler Command
    private final static int CMD_CALLBACK_APP_EVENT = 0;

    private final static int DUPLICATE_PACKET_PREDICTION_BUSY_TIMEOUT = 5; // ms
    // APP event
    private final static int APP_EVENT_WIFI_UNAVAILABLE = -1;
    private final static int APP_EVENT_DUPLICATE_PACKET_PREDICTION_BUSY = -1;
    private final static int APP_EVENT_DUPLICATE_PACKET_PREDICTION_OFF = 0;
    private final static int APP_EVENT_DUPLICATE_PACKET_PREDICTION_ON = 1;

    private final static String APP_EVENT_BUNDLE_KEY_STATE = "STATE";

    // Driver event
    private final static int DRIVER_EVENT_TX_DUP_OFF = 100;
    private final static int DRIVER_EVENT_TX_DUP_ON = 101;
    private final static int DRIVER_EVENT_TX_DUP_CERT_CHANGE = 102;

    private static PowerHalWifiMonitor sInstance = null;

    private Context mContext;
    private HandlerThread mHandlerThread;
    private Handler mHandler;

    private boolean mDppStarted;
    private boolean mWifiEnabled;
    private boolean mWifiConnected;

    private int mDppPowerHdl = 0;

    private final RemoteCallbackList<IRemoteCallback> mListeners = new RemoteCallbackList<>();
    private static PowerHalWrapper mPowerHalWrap = null;

    public PowerHalWifiMonitor(Context context) {
        mContext = context;
        mPowerHalWrap = PowerHalWrapper.getInstance();

        registerForBroadcast();

        initHandlerThread();

        // save instance to static variable
        sInstance = this;
    }

    public static PowerHalWifiMonitor getInstance() {
        return sInstance;
    }

    public void startDuplicatePacketPrediction() {
        logd("startDuplicatePacketPrediction() mDppStarted:" + mDppStarted +
            ", mWifiEnabled:" + mWifiEnabled);

        int rscList[] = {
            PowerHalWrapper.PERF_RES_NET_WIFI_SMART_PREDICT,
            1,
            PowerHalWrapper.PERF_RES_POWERHAL_SCREEN_OFF_STATE,
            PowerHalWrapper.SCREEN_OFF_WAIT_RESTORE};

        if (!mDppStarted) {

            mDppStarted = true;

            if (mWifiEnabled) {

                // start driver
                if (mDppPowerHdl != 0) {
                    mPowerHalWrap.perfLockRelease(mDppPowerHdl);
                    mDppPowerHdl = 0;
                }
                mDppPowerHdl = mPowerHalWrap.perfLockAcquire(mDppPowerHdl, 0, rscList);
            }

            // Callback unavailable
            if (!mWifiEnabled || !mWifiConnected) {
                postStateChange(APP_EVENT_WIFI_UNAVAILABLE);
            }

        }
    }

    public void stopDuplicatePacketPrediction() {
        logd("stopDuplicatePacketPrediction() mDppStarted:" + mDppStarted +
            ", mWifiEnabled:" + mWifiEnabled);

        if (mDppStarted) {
            mDppStarted = false;

            if (mWifiEnabled) {

                // stop driver
                mPowerHalWrap.perfLockRelease(mDppPowerHdl);
                mDppPowerHdl = 0;
            }
        }
    }

    public boolean isDupPacketPredictionStarted() {
        logd("isDupPacketPredictionStarted() mDppStarted:" + mDppStarted);

        return mDppStarted;
    }

    public boolean registerDuplicatePacketPredictionEvent(IRemoteCallback listener) {

        if (listener != null) {
            synchronized (mListeners) {
                logd("registerDuplicatePacketPredictionEvent() " + listener.getClass().toString());
                mListeners.register(listener);
            }
            return true;
        }
        return false;
    }
    public boolean unregisterDuplicatePacketPredictionEvent(IRemoteCallback listener) {
        if (listener != null) {
            synchronized (mListeners) {
                logd("unregisterDuplicatePacketPredictionEvent() "
                    + listener.getClass().toString());
                mListeners.unregister(listener);
            }
            return true;
        }
        return false;
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();

        filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);

        filter.addAction("com.mediatek.npp.ev.a");
        filter.addAction("com.mediatek.npp.ev.b");

        mContext.registerReceiver(mReceiver, filter);
    }

    private void resetDuplicatePacketPrediction() {
        logd("resetDuplicatePacketPrediction(), mDppStarted:" + mDppStarted);

        // stop DPP automatically
        mDppStarted = false;

        // Clear IP table
        String linkInfo = "DELETE_ALL";
        mPowerHalWrap.setSysInfo(PowerHalWrapper.SETSYS_NETD_DUPLICATE_PACKET_LINK, linkInfo);
    }


    private void postStateChange(final int event) {
        logd("postStateChange(), event:" + event);

        mHandler.sendMessage(mHandler.obtainMessage(CMD_CALLBACK_APP_EVENT, event, 0));
    }

    private void callbackOnStateChanged(int event) {

        synchronized (mListeners) {
            int i = mListeners.beginBroadcast();

            logOut("callbackOnStateChanged() " + i + " event:" + event);

            Bundle bundle = new Bundle();
            bundle.putInt(APP_EVENT_BUNDLE_KEY_STATE, event);
            while (i > 0) {
                i--;
                try {
                    mListeners.getBroadcastItem(i).sendResult(bundle);
                } catch (RemoteException ignored) {

                }
            }
            mListeners.finishBroadcast();
        }
    }

    private void reStartWifiDriver() {
        logd("reStartWifiDriver()");
        int rscList[] = {
            PowerHalWrapper.PERF_RES_NET_WIFI_SMART_PREDICT,
            1,
            PowerHalWrapper.PERF_RES_POWERHAL_SCREEN_OFF_STATE,
            PowerHalWrapper.SCREEN_OFF_WAIT_RESTORE};

        // re-start driver
        if(mDppPowerHdl != 0) {
            /* for internal state control */
            mPowerHalWrap.perfLockRelease(mDppPowerHdl);
            mDppPowerHdl = 0;
        }
        mDppPowerHdl = mPowerHalWrap.perfLockAcquire(mDppPowerHdl, 0, rscList);
    }

    private void onWifiStateChange(int wifiState) {

        boolean wifiEnable = (wifiState == WifiManager.WIFI_STATE_ENABLED) ? true : false;
        if (mWifiEnabled == wifiEnable) {
            return;
        }

        mWifiEnabled = wifiEnable;

        logd("onWifiStateChange(), mWifiEnabled:" + mWifiEnabled +
            ", mDppStarted:" + mDppStarted);

        if (mWifiEnabled) {
            if (mDppStarted) {

                reStartWifiDriver();
            }
        } else {

            if (mDppStarted) {
                postStateChange(APP_EVENT_WIFI_UNAVAILABLE);
            }
        }

    }

    private void onWifiConnectionStateChange(NetworkInfo info) {
        boolean wifiConnected = info.isConnected();

        if (mWifiConnected == wifiConnected) {
            return;
        }

        mWifiConnected = wifiConnected;

        logd("onWifiConnectionStateChange(), mWifiConnected:" + mWifiConnected +
            ", mDppStarted:" + mDppStarted);

        if (!mWifiConnected) {

            if (mDppStarted) {
                postStateChange(APP_EVENT_WIFI_UNAVAILABLE);
            }
        } else {

            // Report cache value
            if (mDppStarted) {

                // re-start driver to force fw to update and report the latest value
                reStartWifiDriver();
            }
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            logd("onReceive action:" + intent.getAction());

            if (intent.getAction().equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                int wifiState = intent.getIntExtra(
                        WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_UNKNOWN);

                onWifiStateChange(wifiState);
            } else if (intent.getAction().equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                NetworkInfo info = (NetworkInfo) intent.getParcelableExtra(
                                    WifiManager.EXTRA_NETWORK_INFO);

                onWifiConnectionStateChange(info);
            } else if (intent.getAction().equals("com.mediatek.npp.ev.a")) {
                postStateChange(1);
            } else if (intent.getAction().equals("com.mediatek.npp.ev.b")) {
                postStateChange(0);
            }

        }
    };

    private boolean checkIfDuplicatePacketPredictionBusy(int timeEvent) {

        // timeEvent(5 digits) = second (2) + millicond (3)
        long nowMs = System.currentTimeMillis();

        // now(5 digits) = second (2) + millicond (3)
        int now = (int) ((nowMs / 1000 % 60) * 1000 + (nowMs % 1000));

        if (now - timeEvent > DUPLICATE_PACKET_PREDICTION_BUSY_TIMEOUT) {

            logd("checkIfDuplicatePacketPredictionBusy(), now: " + now + ", drv:" + timeEvent);
            // return true;
            return false;
        }

        return false;
    }

    /**********************************************************
     * The API is called from MtkSupplicantStaIfaceHal.java
     * - void onDataStallNotice(int errCode)
     * Example
     * - timestamp: 09:42:54.206157, event: 10154206
     *********************************************************/
    public void supplicantHalCallback(int event) {

        // 3 higher digits
        int dupEvent = event / 100000;

        // 5 lower digits
        int timeEvent = event % 100000;

        boolean busy = checkIfDuplicatePacketPredictionBusy(timeEvent);

        switch (dupEvent) {
            case DRIVER_EVENT_TX_DUP_OFF:

                if (mDppStarted && mWifiConnected) {

                    // Callback directly to save time
                    callbackOnStateChanged(
                        busy ?
                        APP_EVENT_DUPLICATE_PACKET_PREDICTION_BUSY:
                        APP_EVENT_DUPLICATE_PACKET_PREDICTION_OFF);
                }
                break;

            case DRIVER_EVENT_TX_DUP_ON:

                if (mDppStarted && mWifiConnected) {

                    // Callback directly to save time
                    callbackOnStateChanged(
                        busy ?
                        APP_EVENT_DUPLICATE_PACKET_PREDICTION_BUSY:
                        APP_EVENT_DUPLICATE_PACKET_PREDICTION_ON);
                }
                break;

            case DRIVER_EVENT_TX_DUP_CERT_CHANGE:

                if (mPowerHalWrap.getRildCap(-1) == false) {

                    resetDuplicatePacketPrediction();
                }

                break;

            default:
                // ignore
                break;

        }
    }

    private void initHandlerThread() {

        mHandlerThread = new HandlerThread("PowerHalWifiMonitor");
        mHandlerThread.start();

        /** Handler to handle internal message command, run on handler thread */
        mHandler = new Handler(mHandlerThread.getLooper()) {

            @Override
            public void handleMessage(Message msg) {
                logd("handleMessage: " + messageToString(msg) + " " + msg.arg1);

                switch (msg.what) {
                    case CMD_CALLBACK_APP_EVENT:
                        callbackOnStateChanged(msg.arg1);
                        break;
                    default:
                        break;
                }
            }

            private String messageToString(Message msg) {
                switch (msg.what) {
                    case CMD_CALLBACK_APP_EVENT:
                        return "CMD_CALLBACK_APP_EVENT";
                    default:
                        return Integer.toString(msg.what);
                }
            }
        };
    }

    private void logd(String info) {
        Log.d(TAG, info);
    }

    private void logOut(String info) {
        Log.i("NPP", info);
    }
}
