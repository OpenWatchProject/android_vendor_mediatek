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

package com.orangelabs.rcs.platform.network;


import android.system.ErrnoException;
import android.system.Os;
import android.system.StructTimeval;

import java.net.Inet6Address;
import java.net.InetSocketAddress;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.network.NetworkException;
import com.orangelabs.rcs.provisioning.https.NameResolver;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Random;

import android.os.Looper;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.SystemProperties;

import android.system.ErrnoException;
import android.system.Os;
import android.system.StructTimeval;
import android.net.NetworkCapabilities;

import static android.system.OsConstants.*;
/**
 * Manages the MMS network connectivity
 */
public class AndroidDefaultNetwork implements NameResolver {
    // Timeout used to call ConnectivityManager.requestNetwork
    private static final int NETWORK_REQUEST_TIMEOUT_MILLIS = 60 * 1000;
    // Wait timeout for this class, a little bit longer than the above timeout
    // to make sure we don't bail prematurely
    private static final int NETWORK_ACQUIRE_TIMEOUT_MILLIS =
            NETWORK_REQUEST_TIMEOUT_MILLIS + (5 * 1000);

    private static final int EVENT_ON_PRECHECK = 1001;
    private static final int EVENT_ON_AVAILABLE = 1002;
    private static final int EVENT_ON_UNAVAILABLE = 1003;
    private static final int EVENT_ON_LOSING = 1004;
    private static final int EVENT_ON_LOST = 1005;
    private static final int EVENT_MSG_INFO = 1006;
    private static final int BUFFER_SIZE = 4096;

   private static final String TAG = "AndroidDefaultNetwork";

    private Context mContext;
    // The requested data {@link android.net.Network} we are holding
    // We need this when we unbind from it. This is also used to indicate if the
    // Data network is available.
    private Network mNetwork;
    // The current count of Data requests that require the Data network
    // If mDataRequestCount is 0, we should release the data network.
    private int mRequestCount;

    // This is really just for using the capability
    /*private NetworkRequest mNetworkRequest = new NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
            .addCapability(NetworkCapabilities.NET_CAPABILITY_MMS)
            .build();*/

    // The callback to register when we request Data network
    private ConnectivityManager.NetworkCallback mNetworkCallback;

    private ConnectivityManager mConnectivityManager;


    // TODO: we need to re-architect this when we support MSIM, like maybe one manager for each SIM?
    public AndroidDefaultNetwork(Context context) {
        mContext = context;
        mNetworkCallback = null;
        mNetwork = null;
        mRequestCount = 0;
        mConnectivityManager = null;
    }


    public Network getNetwork() {
        synchronized (this) {
            return mNetwork;
        }
    }

    /**
     * Acquire the network
     *
     * @throws com.android.service.exception.NetworkException if we fail to acquire it
     */
    public void acquireNetwork(long subId) throws NetworkException {
        if (inAirplaneMode()) {
            // Fast fail airplane mode
            throw new NetworkException("In airplane mode");
        }
        Log.d(TAG, "NetworkManager:1111");
        synchronized (this) {
            mRequestCount += 1;
            if (mNetwork != null) {
                // Already available
                Log.d(TAG, "network already available");
                return;
            }
            Log.d(TAG, "start new network request");
            // Not available, so start a new request
            newRequest(subId);
            final long shouldEnd = SystemClock.elapsedRealtime() + NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            long waitTime = NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            while (waitTime > 0) {
                try {
                    this.wait(waitTime);
                } catch (InterruptedException e) {
                    Log.w(TAG, "acquire network wait interrupted");
                }
                if (mNetwork != null) {
                    Log.d(TAG, "network available");
                    // Success
                    return;
                }
                // Calculate remaining waiting time to make sure we wait the full timeout period
                waitTime = shouldEnd - SystemClock.elapsedRealtime();
            }
            // Timed out, so release the request and fail
            Log.d(TAG, "NetworkManager: timed out");
            releaseRequest(mNetworkCallback);
            resetLocked();
            throw new NetworkException("Acquiring network timed out");
        }
    }

    /**
     * Release the MMS network when nobody is holding on to it.
     */
    public void releaseNetwork() {
        synchronized (this) {
            if (mRequestCount > 0) {
                mRequestCount -= 1;
                Log.d(TAG, "NetworkManager: release, count=" + mRequestCount);
                if (mRequestCount < 1) {
                    releaseRequest(mNetworkCallback);
                    resetLocked();
                }
            }
        }
    }

    /**
     * Start a new {@link android.net.NetworkRequest} for MMS
     */
    private void newRequest(long subId) {
        final ConnectivityManager connectivityManager = getConnectivityManager();
        mNetworkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                super.onAvailable(network);
                Log.d(TAG, "NetworkCallbackListener.onAvailable: network=" + network);
                synchronized (AndroidDefaultNetwork.this) {
                    mNetwork = network;
                    //Message msg = mHandler.obtainMessage(EVENT_ON_AVAILABLE, (Object) network);
                    synchronized(AndroidDefaultNetwork.this) {
                        Log.d(TAG, "notifying the listener thet connection avalaibel");
                        AndroidDefaultNetwork.this.notifyAll();
                    }
                    NetworkCapabilities nc = mConnectivityManager.getNetworkCapabilities(network);
                    Log.d(TAG, " NetworkCapabilities  : " + nc);
                }
            }

            @Override
            public void onLost(Network network) {
                super.onLost(network);
                Log.d(TAG, "NetworkCallbackListener.onLost: network=" + network);
                synchronized (AndroidDefaultNetwork.this) {
                    releaseRequest(this);
                    if (mNetworkCallback == this) {
                        resetLocked();
                    }
                   // AndroidDefaultNetwork.this.notifyAll();
                }
            }

            @Override
            public void onUnavailable() {
                super.onUnavailable();
                Log.d(TAG, "NetworkCallbackListener.onUnavailable");
                synchronized (AndroidDefaultNetwork.this) {
                    releaseRequest(this);
                    if (mNetworkCallback == this) {
                        resetLocked();
                    }
                   // AndroidDefaultNetwork.this.notifyAll();
                }
            }
        };
        Log.d(TAG, "newRequest, subId = " + subId);

        connectivityManager.registerDefaultNetworkCallback(
                mNetworkCallback);
    }

    /**
     * Release the current {@link android.net.NetworkRequest} for
     *
     * @param callback the {@link android.net.ConnectivityManager.NetworkCallback} to unregister
     */
    private void releaseRequest(ConnectivityManager.NetworkCallback callback) {
        if (callback != null) {
            final ConnectivityManager connectivityManager = getConnectivityManager();
            connectivityManager.unregisterNetworkCallback(callback);
        }
    }

    /**
     * Reset the state
     */
    private void resetLocked() {
        mNetworkCallback = null;
        mNetwork = null;
       // mMmsRequestCount = 0;
    }

    @Override
    public InetAddress[] getAllByName(String host) throws UnknownHostException {
        synchronized (this) {
            if (mNetwork != null) {
                return mNetwork.getAllByName(host);
            }
            return new InetAddress[0];
        }
    }

    public ConnectivityManager getConnectivityManager() {
        if (mConnectivityManager == null) {
            mConnectivityManager = (ConnectivityManager) mContext.getSystemService(
                    Context.CONNECTIVITY_SERVICE);
        }
        return mConnectivityManager;
    }

    private boolean inAirplaneMode() {
        return Settings.System.getInt(mContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

}
