/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.mediatek.ims.legacy.ss;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.Looper;
import android.os.HandlerThread;
import android.os.Message;
import android.os.SystemClock;
//import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.data.ApnSetting;
import android.telephony.PhoneStateListener;
import android.telephony.PreciseDataConnectionState;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.ims.SuppSrvConfig;

import java.net.InetAddress;
import java.net.UnknownHostException;

/**
 * Manages the XCAP mobile data network connectivity.
 */
public class XcapMobileDataNetworkManager {
    private static final String LOG_TAG = "XcapMobileDataNetworkManager";

    private static final int MSG_RELEASE_NETWORK = 0;

    private static int mKeepAliveTimer = 0;
    private static int mRequestTimer = 0;
    private static int mDataCoolDownTimer = 0;

    private long mPreviousReleaseTime = -1;

    private Handler mHandlerReleaseNW = null;

    private Context mContext;
    private int mDataUsedPhoneId = -1;
    private Network mNetwork;
    private int mXcapMobileDataNetworkRequestCount;

    private ConnectivityManager.NetworkCallback mNetworkCallback;
    private ConnectivityManager mConnectivityManager;

    private TelephonyManager mTelephonyManager;
    private PhoneStateListener mPhoneStateListener;
    private HandlerThread mHandlerThread;
    private int mXcapDataConnectionState = TelephonyManager.DATA_UNKNOWN;
    private boolean isNotifyByDataDisconnected = false;

    /** APN type for XCAP traffic.
     *  Reference to MtkApnSetting.java
     */
    public static final int TYPE_XCAP = ApnSetting.TYPE_EMERGENCY << 2;

    /**
     * Manages the XCAP mobile data network connectivity.
     * @param context context
     */
    public XcapMobileDataNetworkManager(Context context, Looper looper) {
        mContext = context;
        mNetworkCallback = null;
        mNetwork = null;
        mXcapMobileDataNetworkRequestCount = 0;
        mConnectivityManager = null;

        mHandlerReleaseNW = new NetworkHandler(looper);

        mTelephonyManager = (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
        // need to make new thread to run PhoneStateListener. can't use looper object. or else,
        // PhoneStateListener will run thread the same to aquireNetwork method, and it will be
        // blocked by this.wait();
        mHandlerThread = new HandlerThread("PhoneStateListenerNotify");
        mHandlerThread.start();
    }

    public static void setKeepAliveTimer(int timer) {
        Rlog.d(LOG_TAG, "setKeepAliveTimer: " + timer);
        mKeepAliveTimer = timer;
    }

    public static void setRequestDataTimer(int timer) {
        Rlog.d(LOG_TAG, "setRequestDataTimer: " + timer);
        mRequestTimer = timer;
    }

    public static void setDataCoolDownTimer(int timer) {
        Rlog.d(LOG_TAG, "setDataCoolDownTimer: " + timer);
        mDataCoolDownTimer = timer;
    }

    /**
     * Acquire the XCAP mobile data network.
     * @param phoneId the phone index
     * @return the acquired network
     */
    public Network acquireNetwork(int phoneId) {
        Rlog.d(LOG_TAG, "acquireNetwork: phoneId = " + phoneId);
        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);

        if (!ssConfig.isUseXCAPTypeApn() && !ssConfig.isUseInternetTypeApn()) {
            Rlog.d(LOG_TAG, "not use any APN. No need to acquireNetwork");
            return null;
        }
        synchronized (this) {
            Rlog.d(LOG_TAG, "acquireNetwork start");
            mXcapMobileDataNetworkRequestCount += 1;
            mHandlerReleaseNW.removeMessages(MSG_RELEASE_NETWORK);

            if (mNetwork != null) {
                if(mDataUsedPhoneId != phoneId) {
                    releaseRequest();
                    mXcapMobileDataNetworkRequestCount += 1;
                } else {
                    Rlog.d(LOG_TAG, "already available: mNetwork=" + mNetwork);
                    return mNetwork;
                }
            }

            Rlog.d(LOG_TAG, "start new network request");
            mDataUsedPhoneId = phoneId;
            long current = SystemClock.elapsedRealtime();
            long coolDownDuration = current - mPreviousReleaseTime;
            long waitCoolDown = (coolDownDuration > mDataCoolDownTimer ?
                    0 : mDataCoolDownTimer - coolDownDuration);

            Rlog.d(LOG_TAG, "waitCoolDown=" + waitCoolDown
                    + ", coolDownDuration=" + coolDownDuration
                    + ", mDataCoolDownTimer=" + mDataCoolDownTimer);
            try {
                Thread.sleep(waitCoolDown);
            } catch (InterruptedException e) {
                Rlog.d(LOG_TAG, "wait cool down interrupted");
            }

            newRequest(phoneId);

            Rlog.d(LOG_TAG, "wait request result ... mRequestTimer=" + mRequestTimer);
            try {
                this.wait(mRequestTimer);
            } catch (InterruptedException e) {
                Rlog.d(LOG_TAG, "wait request interrupted");
            }
            Rlog.d(LOG_TAG, "continue ...");

            if (mNetwork != null) {
                Rlog.d(LOG_TAG, "acquireNetwork success: mNetwork=" + mNetwork);
                return mNetwork;
            }
            if(isNotifyByDataDisconnected) {
                Rlog.d(LOG_TAG, "create xcap data connection failed");
                isNotifyByDataDisconnected = false;
            } else {
                Rlog.d(LOG_TAG, "timed out");
            }
            stopListenXcapDataConnectionState();
            releaseRequest();
        }

        return null;
    }

    class NetworkHandler extends Handler {
        public NetworkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_RELEASE_NETWORK: {
                    Rlog.d(LOG_TAG, "Ready to release network: " + mNetwork);
                    synchronized (XcapMobileDataNetworkManager.this) {
                        releaseRequest();
                    }
                    return;
                }
            }
        }
    };

    /**
     * Release the XCAP mobile data network when nobody is holding on to it.
     */
    public void releaseNetwork() {
        synchronized (this) {
            if (mXcapMobileDataNetworkRequestCount > 0) {
                mXcapMobileDataNetworkRequestCount -= 1;
                Rlog.d(LOG_TAG, "releaseNetwork count=" + mXcapMobileDataNetworkRequestCount);
                if (mXcapMobileDataNetworkRequestCount < 1) {
                    if (mNetwork == null) {
                        Rlog.d(LOG_TAG, "No dedicate network here, release directly.");
                        releaseRequest();
                    } else {
                        Rlog.d(LOG_TAG, "Delay release network.");
                        Message msg = mHandlerReleaseNW.obtainMessage(MSG_RELEASE_NETWORK);
                        mHandlerReleaseNW.sendMessageDelayed(msg, mKeepAliveTimer);
                    }
                }
            }
        }
    }

    /**
     * Start a new {@link android.net.NetworkRequest} for XCAP mobile data network.
     */
    private void newRequest(int phoneId) {
        final ConnectivityManager connectivityManager = getConnectivityManager();
        // ToDo: O migration
        int subId = getSubIdUsingPhoneId(phoneId);
        mNetworkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                super.onAvailable(network);
                Rlog.d(LOG_TAG, "NetworkCallbackListener.onAvailable: network=" + network);
                stopListenXcapDataConnectionState();
                synchronized (XcapMobileDataNetworkManager.this) {
                    mNetwork = network;
                    XcapMobileDataNetworkManager.this.notifyAll();
                }
            }

            @Override
            public void onLost(Network network) {
                super.onLost(network);
                Rlog.d(LOG_TAG, "NetworkCallbackListener.onLost: network=" + network);
                stopListenXcapDataConnectionState();
                synchronized (XcapMobileDataNetworkManager.this) {
                    releaseRequest();
                    XcapMobileDataNetworkManager.this.notifyAll();
                }
            }

            @Override
            public void onUnavailable() {
                super.onUnavailable();
                Rlog.d(LOG_TAG, "NetworkCallbackListener.onUnavailable");
                stopListenXcapDataConnectionState();
                synchronized (XcapMobileDataNetworkManager.this) {
                    releaseRequest();
                    XcapMobileDataNetworkManager.this.notifyAll();
                }
            }
        };
        Rlog.d(LOG_TAG, "newRequest, subId=" + subId);
        NetworkRequest.Builder networkBuilder = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(Integer.toString(subId));

        //if (SystemProperties.getInt("persist.mtk_epdg_support", 0) == 1) {
        //    networkBuilder.addTransportType(NetworkCapabilities.TRANSPORT_EPDG);
        //}

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);

        if (ssConfig.isUseInternetTypeApn()) {
            networkBuilder.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET);
            networkBuilder.addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED);
        } else {
            networkBuilder.addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP);
        }
        startListenXcapDataConnectionState(phoneId);
        NetworkRequest networkRequest = networkBuilder.build();
        connectivityManager.requestNetwork(networkRequest, mNetworkCallback, mRequestTimer);
    }

    private void releaseRequest() {
        Rlog.d(LOG_TAG, "releaseRequest: mNetwork=" + mNetwork
                + ", mNetworkCallback=" + mNetworkCallback);
        final ConnectivityManager connectivityManager = getConnectivityManager();
        if (mNetworkCallback != null) {
            connectivityManager.unregisterNetworkCallback(mNetworkCallback);
        }
        if (mNetwork != null) {
            mPreviousReleaseTime = SystemClock.elapsedRealtime();
            Rlog.d(LOG_TAG, "Release time: " + mPreviousReleaseTime);
        }
        if (connectivityManager != null) {
            Rlog.d(LOG_TAG, "UnBind process network");
            connectivityManager.bindProcessToNetwork(null);
        }
        mNetworkCallback = null;
        mNetwork = null;
        mXcapMobileDataNetworkRequestCount = 0;
        mDataUsedPhoneId = -1;
    }

    private ConnectivityManager getConnectivityManager() {
        if (mConnectivityManager == null) {
            mConnectivityManager = (ConnectivityManager) mContext.getSystemService(
                    Context.CONNECTIVITY_SERVICE);
        }
        return mConnectivityManager;
    }

    private String dataStateToString(int state) {
        switch (state) {
            case TelephonyManager.DATA_UNKNOWN:
                return "DATA_UNKNOWN";
            case TelephonyManager.DATA_DISCONNECTED:
                return "DATA_DISCONNECTED";
            case TelephonyManager.DATA_CONNECTING:
                return "DATA_CONNECTING";
            case TelephonyManager.DATA_CONNECTED:
                return "DATA_CONNECTED";
            case TelephonyManager.DATA_SUSPENDED:
                return "DATA_SUSPENDED";
            default:
                return "Error";
        }
    }

    private void startListenXcapDataConnectionState(int phoneId) {
        // ToDo: O migration
        int subId = getSubIdUsingPhoneId(phoneId);
        Rlog.d(LOG_TAG, "startListenXcapDataConnectionState: subid=" + subId);
        mPhoneStateListener = new PhoneStateListener(mHandlerThread.getLooper()){
            public void onPreciseDataConnectionStateChanged(PreciseDataConnectionState state) {
                int newState = state.getDataConnectionState();
                int apnType = state.getDataConnectionApnTypeBitMask();
                Rlog.d(LOG_TAG, "onPreciseDataConnectionStateChanged: apnType=" + apnType
                        + ", newState=" + dataStateToString(newState)
                        + ", currentXcapState=" + dataStateToString(mXcapDataConnectionState));
                if(apnType != TYPE_XCAP) {
                    return;
                }
                if(mXcapDataConnectionState == TelephonyManager.DATA_CONNECTING
                        && mXcapDataConnectionState != newState
                        && newState == TelephonyManager.DATA_DISCONNECTED) {
                    isNotifyByDataDisconnected = true;
                    stopListenXcapDataConnectionState();
                    synchronized (XcapMobileDataNetworkManager.this) {
                        XcapMobileDataNetworkManager.this.notifyAll();
                    }
                    return;
                }
                if(newState != TelephonyManager.DATA_UNKNOWN) {
                    mXcapDataConnectionState = newState;
                }
            }
        };

        if(mTelephonyManager != null) {
            mTelephonyManager.listen(mPhoneStateListener,
                    PhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE);
        }
    }

    private void stopListenXcapDataConnectionState() {
        Rlog.d(LOG_TAG, "stopListenXcapDataConnectionState: listener=" + mPhoneStateListener);

        if(mTelephonyManager != null && mPhoneStateListener != null) {
            mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        }
        mPhoneStateListener = null;
        mXcapDataConnectionState = TelephonyManager.DATA_UNKNOWN;
    }

    private int getSubIdUsingPhoneId(int phoneId) {
        int[] subIds = SubscriptionManager.getSubId(phoneId);
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        if (subIds.length != 0) {
            subId = subIds[0];
        }
        return subId;
    }
}
