package com.mediatek.entitlement.o2;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.os.SystemClock;
import android.util.Log;

import android.telephony.SubscriptionManager;

/**
 * Manages the Entitlement mobile data network connectivity.
 */
public class EntitlementNetworkManager {
    private static final String TAG = "EntitlementNetworkManager";
    private static final int DATA_REQUEST_TIMEOUT = 60 * 1000;
    private static final int ACQUIRE_WAIT_TIMER =
            DATA_REQUEST_TIMEOUT + (5 * 1000);

    private final Context mContext;
    private volatile ConnectivityManager mConnectivityManager;
    private ConnectivityManager.NetworkCallback mEntitlementNetworkCallback;
    private Network mEntitlementNetwork;
    private int mEntitlementRequestCount;
    private final int mPhoneId;

    public EntitlementNetworkManager(Context context, int phoneId) {
        mContext = context;
        mConnectivityManager = null;
        mEntitlementNetworkCallback = null;
        mEntitlementNetwork = null;
        mEntitlementRequestCount = 0;
        mPhoneId = phoneId;
    }

    /**
     * Acquire the Entitlement mobile data network
     * @return the acquired network
     */
    public Network acquireNetwork() {
        synchronized (this) {
            mEntitlementRequestCount += 1;
            if (mEntitlementNetwork != null) {
                log("acquireNetwork: already available");
                return mEntitlementNetwork;
            }

            if (mEntitlementNetworkCallback == null) {
                log("acquireNetwork: start new network request");
                startNewRequest();
            }

            long acquireWaitTime = ACQUIRE_WAIT_TIMER;
            final long endTime = SystemClock.elapsedRealtime() + ACQUIRE_WAIT_TIMER;
            while (acquireWaitTime > 0) {
                try {
                    this.wait(acquireWaitTime);
                } catch (InterruptedException e) {
                    loge("acquireNetwork: wait request interrupted");
                }
                if (mEntitlementNetwork != null) {
                    log("acquireNetwork: success");
                    return mEntitlementNetwork;
                }
                acquireWaitTime = endTime - SystemClock.elapsedRealtime();
            }
            loge("acquireNetwork: timed out");
            releaseRequest(mEntitlementNetworkCallback);
            return null;
        }
    }

    /**
     * Release the Entitlement network when nobody is holding on to it.
     */
    public void releaseNetwork() {
        synchronized (this) {
            if (mEntitlementRequestCount > 0) {
                mEntitlementRequestCount -= 1;
                log("releaseNetwork: requestCount = " + mEntitlementRequestCount);
                if (mEntitlementRequestCount < 1) {
                    releaseRequest(mEntitlementNetworkCallback);
                }
            }
        }
    }

    private ConnectivityManager getConnectivityManager() {
        if (mConnectivityManager == null) {
            mConnectivityManager = (ConnectivityManager) mContext.getSystemService(
                    Context.CONNECTIVITY_SERVICE);
        }
        return mConnectivityManager;
    }

    private static int getSubIdUsingPhoneId(int phoneId) {
        int [] values = SubscriptionManager.getSubId(phoneId);
        if(values == null || values.length <= 0) {
            return SubscriptionManager.getDefaultSubscriptionId();
        }
        else {
            return values[0];
        }
    }

    /**
     * Start a new {@link android.net.NetworkRequest} for Entitlement
     */
    private void startNewRequest() {
        final ConnectivityManager connectivityManager = getConnectivityManager();
        int subId = getSubIdUsingPhoneId(mPhoneId);
        mEntitlementNetworkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                super.onAvailable(network);
                logi("NetworkCallbackListener.onAvailable: network=" + network);
                synchronized (EntitlementNetworkManager.this) {
                    mEntitlementNetwork = network;
                    EntitlementNetworkManager.this.notifyAll();
                }
            }

            @Override
            public void onLost(Network network) {
                super.onLost(network);
                logi("NetworkCallbackListener.onLost: network=" + network);
                synchronized (EntitlementNetworkManager.this) {
                    releaseRequest(this);
                    EntitlementNetworkManager.this.notifyAll();
                }
            }

            @Override
            public void onUnavailable() {
                super.onUnavailable();
                logi("NetworkCallbackListener.onUnavailable");
                synchronized (EntitlementNetworkManager.this) {
                    releaseRequest(this);
                    EntitlementNetworkManager.this.notifyAll();
                }
            }
        };
        NetworkRequest networkRequest = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP)
                .setNetworkSpecifier(Integer.toString(subId))
                .build();
        connectivityManager.requestNetwork(
                networkRequest, mEntitlementNetworkCallback, DATA_REQUEST_TIMEOUT);
    }

    /**
     * Release the mobile data network for Entitlement
     */
    private void releaseRequest(ConnectivityManager.NetworkCallback callback) {
        final ConnectivityManager connectivityManager = getConnectivityManager();
        if (connectivityManager != null && callback != null) {
            try {
                connectivityManager.unregisterNetworkCallback(callback);
            } catch (IllegalArgumentException e) {
                loge("Unregister network callback exception", e);
            }
        }
        if (connectivityManager != null) {
            connectivityManager.bindProcessToNetwork(null);
        }
        mEntitlementNetworkCallback = null;
        mEntitlementNetwork = null;
        mEntitlementRequestCount = 0;
    }

    private void log(String s) {
        Log.d(TAG,  "[" + mPhoneId + "]" + s);
    }

    private void logi(String s) {
        Log.i(TAG,  "[" + mPhoneId + "]" + s);
    }

    private void loge(String s) {
        Log.e(TAG,  "[" + mPhoneId + "]" + s);
    }

    private void loge(String s, Exception e) {
        Log.e(TAG,  "[" + mPhoneId + "]" + s, e);
    }
}

