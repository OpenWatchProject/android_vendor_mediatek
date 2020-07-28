/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2015 Sony Mobile Communications Inc.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.mediatek.rcs.provisioning.https;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.NetworkRequest.Builder;
import android.net.wifi.WifiManager;
import android.os.SystemClock;
import android.telephony.SubscriptionManager;

import com.mediatek.rcs.provisioning.ProvisioningFailureReasons;
import com.mediatek.rcs.utils.logger.Logger;

import java.io.IOException;

/**
 * HTTPS provisioning connection management
 *
 * @author Orange
 */
public class HttpsProvisioningConnection {

    /**
     * HttpsProvisioningManager manages HTTP and SMS reception to load provisioning from network
     */
    private HttpsProvisioningManager mProvisioningManager;

    private BroadcastReceiver mNetworkStateListener;

    private ConnectivityManager mConnectionManager;

    /**
     * Wifi disabling listener
     */
    private BroadcastReceiver mWifiDisablingListener;

    private final Context mContext;

    private static final Logger sLogger = Logger.getLogger(HttpsProvisioningConnection.class
            .getName());

    /**
     * Constructor
     *
     * @param httpsProvisioningManager HTTP provisioning manager
     * @param context The context
     */
    public HttpsProvisioningConnection(HttpsProvisioningManager httpsProvisioningManager,
            Context context) {
        mProvisioningManager = httpsProvisioningManager;
        mContext = context;
        mConnectionManager = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
    }

    /**
     * Get connection manager
     *
     * @return connection manager
     */
    public ConnectivityManager getConnectionMngr() {
        return mConnectionManager;
    }

    /**
     * Register the broadcast receiver for network state
     */
    // protected void registerNetworkStateListener() {
    //     // Check if network state listener is already registered
    //     if (mNetworkStateListener != null) {
    //         if (sLogger.isActivated()) {
    //             sLogger.debug("Network state listener already registered");
    //         }
    //         return;
    //     }

    //     if (sLogger.isActivated()) {
    //         sLogger.debug("Registering network state listener");
    //     }

    //     // Instantiate the network state listener
    //     mNetworkStateListener = new BroadcastReceiver() {
    //         @Override
    //         public void onReceive(final Context context, final Intent intent) {
    //             mProvisioningManager.scheduleProvisioningOperation(new Runnable() {
    //                 @Override
    //                 public void run() {
    //                     String action = intent.getAction();
    //                     try {
    //                         if (sLogger.isActivated()) {
    //                             sLogger.debug("Network state listener - Received broadcast: "
    //                                     + action);
    //                         }
    //                         mProvisioningManager.connectionEvent(action);
    //                     // } catch (RcsAccountException e) {
    //                     //     sLogger.error("Unable to handle connection event for intent action: "
    //                     //             + action, e);
    //                     } catch (IOException e) {
    //                         if (sLogger.isActivated()) {
    //                             sLogger.debug(new StringBuilder(
    //                                     "Unable to handle connection event for intent action: ")
    //                                     .append(action).append(", Message=").append(e.getMessage())
    //                                     .toString());
    //                         }
    //                         /* Start the RCS service */
    //                         if (mProvisioningManager.isFirstProvisioningAfterBoot()) {
    //                             /* Reason: No configuration present */
    //                             if (sLogger.isActivated()) {
    //                                 sLogger.debug("Initial provisioning failed!");
    //                             }
    //                             mProvisioningManager
    //                                     .provisioningFails(ProvisioningFailureReasons.CONNECTIVITY_ISSUE);
    //                             mProvisioningManager.retry();
    //                         }
    //                         // else {
    //                         //     mProvisioningManager.tryLaunchRcsCoreService(context, -1);
    //                         // }
    //                     } catch (RuntimeException e) {
    //                         /*
    //                          * Normally we are not allowed to catch runtime exceptions as these are
    //                          * genuine bugs which should be handled/fixed within the code. However
    //                          * the cases when we are executing operations on a thread unhandling
    //                          * such exceptions will eventually lead to exit the system and thus can
    //                          * bring the whole system down, which is not intended.
    //                          */
    //                         sLogger.error("Unable to handle connection event for intent action: "
    //                                 + action, e);
    //                     }
    //                 }
    //             });
    //         }
    //     };

    //     // Register network state listener
    //     IntentFilter intentFilter = new IntentFilter();
    //     intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
    //     mContext.registerReceiver(mNetworkStateListener, intentFilter);
    // }

    /**
     * Unregister the broadcast receiver for network state
     */
    // protected void unregisterNetworkStateListener() {
    //     if (mNetworkStateListener != null) {
    //         if (sLogger.isActivated()) {
    //             sLogger.debug("Unregistering network state listener");
    //         }
    //         mContext.unregisterReceiver(mNetworkStateListener);
    //         mNetworkStateListener = null;
    //     }
    // }

    /**
     * Register the broadcast receiver for wifi disabling
     */
    // protected void registerWifiDisablingListener() {
    //     if (mWifiDisablingListener != null) {
    //         if (sLogger.isActivated()) {
    //             sLogger.debug("WIFI disabling listener already registered");
    //         }
    //         return;
    //     }

    //     if (sLogger.isActivated()) {
    //         sLogger.debug("Registering WIFI disabling listener");
    //     }

    //     mWifiDisablingListener = new BroadcastReceiver() {
    //         @Override
    //         public void onReceive(final Context context, final Intent intent) {
    //             mProvisioningManager.scheduleProvisioningOperation(new Runnable() {
    //                 @Override
    //                 public void run() {
    //                     try {
    //                         if (sLogger.isActivated()) {
    //                             sLogger.debug("Wifi disabling listener - Received broadcast: "
    //                                     + intent.toString());
    //                         }

    //                         if (intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
    //                                 WifiManager.WIFI_STATE_UNKNOWN) == WifiManager.WIFI_STATE_DISABLED) {
    //                             //mProvisioningManager.resetCounters();
    //                             registerNetworkStateListener();
    //                             unregisterWifiDisablingListener();
    //                         }
    //                     } catch (RuntimeException e) {
    //                         /*
    //                          * Normally we are not allowed to catch runtime exceptions as these are
    //                          * genuine bugs which should be handled/fixed within the code. However
    //                          * the cases when we are executing operations on a thread unhandling
    //                          * such exceptions will eventually lead to exit the system and thus can
    //                          * bring the whole system down, which is not intended.
    //                          */
    //                         sLogger.error(new StringBuilder(
    //                                 "Unable to handle wifi state change event for action : ")
    //                                 .append(intent.getAction()).toString(), e);
    //                     }
    //                 }
    //             });
    //         }
    //     };

    //     IntentFilter intentFilter = new IntentFilter();
    //     intentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
    //     mContext.registerReceiver(mWifiDisablingListener, intentFilter);
    // }

    /**
     * Unregister the broadcast receiver for wifi disabling
     */
    // protected void unregisterWifiDisablingListener() {
    //     if (mWifiDisablingListener != null) {
    //         if (sLogger.isActivated()) {
    //             sLogger.debug("Unregistering WIFI disabling listener");
    //         }
    //         try {
    //             mContext.unregisterReceiver(mWifiDisablingListener);
    //         } catch (IllegalArgumentException e) {
    //             // Nothing to do
    //         }
    //         mWifiDisablingListener = null;
    //     }
    // }

    private int mRequestCount = 0;
    private boolean mIsNetworkLost;
    private Network mNetwork = null;
    private ConnectivityManager.NetworkCallback mNetworkCallback =  new NetworkRequestCallback();
    private static final int NETWORK_ACQUIRE_TIMEOUT_MILLIS = 30 * 1000;
    private boolean mIsNetworkReleased = true;

    protected void acquireNetwork() throws Exception {
        Builder builder = new NetworkRequest.Builder();
        builder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        builder.addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP);
        builder.setNetworkSpecifier(String.valueOf(SubscriptionManager.getDefaultDataSubscriptionId()));
        NetworkRequest nwRequest = builder.build();
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("acquireNetwork, nwRequest" + nwRequest
                          + ",mRequestCount:" + mRequestCount);
        }

        synchronized (this) {
            mRequestCount += 1;
            mIsNetworkLost = false;
            mIsNetworkReleased = false;
            if (mNetwork != null) {
                return;
            }

            startNewNetworkRequestLocked(nwRequest);

            final long shouldEnd = SystemClock.elapsedRealtime() + NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            long waitTime = NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            while (waitTime > 0) {
                try {
                    this.wait(waitTime);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                if (mNetwork != null) {
                    // Success
                    if (sLogger.isActivatedDebug()) {
                        sLogger.debug("acquireNetwork mNetwork:" + mNetwork);
                    }
                    return;
                }

                // if the network lost, no need to wait.
                if (mIsNetworkLost) {
                    break;
                }

                waitTime = shouldEnd - SystemClock.elapsedRealtime();
            }

            releaseNetwork();
            this.notifyAll();
            throw new Exception("Acquiring network timed out");
        }
    }


    public void releaseNetwork() {
        synchronized (this) {
            if (sLogger.isActivatedDebug()) {
                sLogger.debug("releaseNetwork, mRequestCount:" + mRequestCount);
            }
            if (mRequestCount > 0) {
                mRequestCount -= 1;

                if (mRequestCount < 1) {
                    releaseRequestLocked();
                    try {
                        ConnectivityManager.setProcessDefaultNetwork(null);
                    } catch (IllegalStateException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    private void startNewNetworkRequestLocked(NetworkRequest nwRequest) {
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("startNewNetworkRequestLocked");
        }
        try {
            mConnectionManager.requestNetwork(nwRequest, mNetworkCallback, NETWORK_ACQUIRE_TIMEOUT_MILLIS);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
    }

    private void releaseRequestLocked() {
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("releaseRequestLocked");
        }
        try {
            mConnectionManager.unregisterNetworkCallback(mNetworkCallback);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }

        mIsNetworkLost = true;
        mIsNetworkReleased = true;
        mNetwork = null;
    }

    private class NetworkRequestCallback extends ConnectivityManager.NetworkCallback {
        @Override
        public void onAvailable(Network network) {
            super.onAvailable(network);
            if (sLogger.isActivated()) {
                sLogger.info("onAvailable + network:" + network
                             + ",mIsNetworkReleased:" + mIsNetworkReleased);
            }
            synchronized (HttpsProvisioningConnection.this) {
                if (!mIsNetworkReleased) {
                    mNetwork = network;
                    try {
                        ConnectivityManager cm = (ConnectivityManager)
                            mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                        cm.bindProcessToNetwork(network);
                    } catch (IllegalStateException e) {
                        e.printStackTrace();
                    }
                    HttpsProvisioningConnection.this.notifyAll();
                }
            }
        }

        @Override
        public void onLost(Network network) {
            super.onLost(network);
            if (sLogger.isActivated()) {
                sLogger.info("onLost:" + network);
            }
            try {
                ConnectivityManager.setProcessDefaultNetwork(null);
            } catch (IllegalStateException e) {
                e.printStackTrace();
            }
            synchronized (HttpsProvisioningConnection.this) {
                releaseNetwork();
                HttpsProvisioningConnection.this.notifyAll();
            }
        }

        @Override
        public void onUnavailable() {
            super.onUnavailable();
            if (sLogger.isActivated()) {
                sLogger.info("onUnavailable");
            }
            synchronized (HttpsProvisioningConnection.this) {
                releaseNetwork();
                HttpsProvisioningConnection.this.notifyAll();
            }
        }
    }

}
