/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.mail.utils;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

import com.google.common.collect.Lists;

import java.util.ArrayList;

/**
 * Encapsulates functionality of ConnectivityManager for use in the Email application.  In
 * particular, this class provides callbacks for connectivity lost, connectivity restored, and
 * background setting changed, as well as providing a method that waits for connectivity
 * to be available without holding a wake lock
 *
 * To use, EmailConnectivityManager mgr = new EmailConnectivityManager(context, "Name");
 * When done, mgr.unregister() to unregister the internal receiver
 *
 * TODO: Use this class in ExchangeService
 */
public class EmailConnectivityManager extends BroadcastReceiver {
    private static final String TAG = "EmailConnectivityMgr";

    // Loop time while waiting (stopgap in case we don't get a broadcast)
    private static final int CONNECTIVITY_WAIT_TIME = 10 * 60 * 1000;

    // Sentinel value for "no active network"
    public static final int NO_ACTIVE_NETWORK = -1;

    // The name of this manager (used for logging)
    private final String mName;
    // The monitor lock we use while waiting for connectivity
    private final Object mLock = new Object();
    // The instantiator's context
    private final Context mContext;
    // The wake lock used while running (so we don't fall asleep during execution/callbacks)
    private final WakeLock mWakeLock;
    private final android.net.ConnectivityManager mConnectivityManager;

    // Set when we abort waitForConnectivity() via stopWait
    private boolean mStop = false;
    // The thread waiting for connectivity
    private Thread mWaitThread;
    // Whether or not we're registered with the system connectivity manager
    private boolean mRegistered = true;

    /// M: Record listeners of Connectivity state changes
    private final ArrayList<EmailConntectvityStateChangeListener> mListeners = Lists.newArrayList();

    public EmailConnectivityManager(Context context, String name)  {
        mContext = context;
        mName = name;
        mConnectivityManager =
            (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, name);
        mContext.registerReceiver(this, new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));
    }

    public boolean isAutoSyncAllowed() {
        return ContentResolver.getMasterSyncAutomatically();
    }

    public void stopWait() {
        mStop = true;
        Thread thread = mWaitThread;
        if (thread != null) {
            thread.interrupt();
        }
    }

    /**
     * M: Dispatches a change event for connectivity state
     * Always happens in the UI thread.
     */
    private void dispatchStateChange(NetworkInfo networkInfo) {
        LogUtils.d(TAG, "EmailConntectvityStateChangeListener dispatchStateChange networkInfo: %s ",
                networkInfo);
        ArrayList<EmailConntectvityStateChangeListener> list = new ArrayList<EmailConntectvityStateChangeListener>(mListeners);
        for (EmailConntectvityStateChangeListener listener : list) {
            assert (listener != null);
            listener.onConnectivityStateChanged(networkInfo);
        }
    }

    /**
     * M: Adds a listener from this Activity.
     * Must happen in the UI thread.
     * @param listener of Connectivity state changes
     */
    public void addListener(EmailConntectvityStateChangeListener listener) {
        mListeners.add(listener);
        LogUtils.d(TAG, "EmailConntectvityStateChangeListener add: %s, with size: %d",
                listener.toString(), mListeners.size());
    }

    /**
     * M: Remove a listener from this Activity
     * @param listener  of Connectivity state changes
     */
    public void removeListener(EmailConntectvityStateChangeListener listener) {
        mListeners.remove(listener);
        LogUtils.d(TAG, "EmailConntectvityStateChangeListener remove: %s, with size: %d",
                listener.toString(), mListeners.size());
    }

    /**
     * M: Called when network connectivity has been changed;
     * NOTE: CALLED ON UI THREAD
     * To use:
     *     1. EmailConnectivityManager mgr = getActivity.getConnMgr();
     *     2. mgr.addListener(listener)
     *     3. Override and implement listener onConnectivityStateChanged
     *     4. mgr.removeListener(listener)
     * @param networkInfo as defined by ConnectivityManager
     */
    public interface EmailConntectvityStateChangeListener {
        public void onConnectivityStateChanged(NetworkInfo networkInfo);
    }

    /**
     * Called when network connectivity has been restored; this method should be overridden by
     * subclasses as necessary. NOTE: CALLED ON UI THREAD
     * @param networkType as defined by ConnectivityManager
     */
    public void onConnectivityRestored(int networkType) {
    }

    /**
     * Called when network connectivity has been lost; this method should be overridden by
     * subclasses as necessary. NOTE: CALLED ON UI THREAD
     * @param networkType as defined by ConnectivityManager
     */
    public void onConnectivityLost(int networkType) {
    }

    public void unregister() {
        try {
            mContext.unregisterReceiver(this);
        } catch (RuntimeException e) {
            // Don't crash if we didn't register
        } finally {
            mRegistered = false;
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.getAction().equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
            Bundle extras = intent.getExtras();
            if (extras != null) {
                NetworkInfo networkInfo =
                    (NetworkInfo) extras.get(ConnectivityManager.EXTRA_NETWORK_INFO);
                if (networkInfo == null) return;
                State state = networkInfo.getState();
                if (state == State.CONNECTED) {
                    synchronized (mLock) {
                        mLock.notifyAll();
                    }
                    onConnectivityRestored(networkInfo.getType());
                } else if (state == State.DISCONNECTED) {
                    onConnectivityLost(networkInfo.getType());
                }
                /// M: dispatchStateChange to listeners.
                dispatchStateChange(networkInfo);
            }
        }
    }

    /**
     * Request current connectivity status
     * @return whether there is connectivity at this time
     */
    public boolean hasConnectivity() {
        NetworkInfo info = mConnectivityManager.getActiveNetworkInfo();
        return (info != null);
    }

    /**
     * Get the type of the currently active data network
     * @return the type of the active network (or NO_ACTIVE_NETWORK)
     */
    public int getActiveNetworkType() {
        return getActiveNetworkType(mConnectivityManager);
    }

    static public int getActiveNetworkType(Context context) {
        ConnectivityManager cm =
            (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        return getActiveNetworkType(cm);
    }

    static public int getActiveNetworkType(ConnectivityManager cm) {
        NetworkInfo info = cm.getActiveNetworkInfo();
        if (info == null) return NO_ACTIVE_NETWORK;
        return info.getType();
    }

    public void waitForConnectivity() {
        // If we're unregistered, throw an exception
        if (!mRegistered) {
            throw new IllegalStateException("ConnectivityManager not registered");
        }
        boolean waiting = false;
        mWaitThread = Thread.currentThread();
        // Acquire the wait lock while we work
        mWakeLock.acquire();
        try {
            while (!mStop) {
                NetworkInfo info = mConnectivityManager.getActiveNetworkInfo();
                if (info != null) {
                    // We're done if there's an active network
                    if (waiting) {
                        LogUtils.d(TAG, mName + ": Connectivity wait ended");
                    }
                    return;
                } else {
                    if (!waiting) {
                        LogUtils.d(TAG, mName + ": Connectivity waiting...");
                        waiting = true;
                    }
                    // Wait until a network is connected (or 10 mins), but let the device sleep
                    synchronized (mLock) {
                        // Don't hold a lock during our wait
                        mWakeLock.release();
                        try {
                            mLock.wait(CONNECTIVITY_WAIT_TIME);
                        } catch (InterruptedException e) {
                            // This is fine; we just go around the loop again
                        }
                        // Get the lock back and check again for connectivity
                        mWakeLock.acquire();
                    }
                }
            }
        } finally {
            // Make sure we always release the wait lock
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }
            mWaitThread = null;
        }
    }
}
