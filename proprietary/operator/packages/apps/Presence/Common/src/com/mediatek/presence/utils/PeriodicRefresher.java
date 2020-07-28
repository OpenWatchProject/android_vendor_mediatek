/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright © 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.mediatek.presence.utils;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.utils.logger.Logger;

import java.util.Timer;
import java.util.TimerTask;

/**
 * Periodic refresher
 *
 * @author JM. Auffret
 */
public abstract class PeriodicRefresher {

    private Timer mPeriodicTimer;
    private PeriodicTask mPeriodicTask;

    /**
     * Polling period
     */
    private int pollingPeriod;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * M: WakeLock @{
     */
    //The Wakelock acquired when alarm time is up
    private WakeLock mPeriodWakeLock = null;
    //The Wakelock acquired when receive data from network
    private WakeLock mNetworkWakeLock = null;
    //Tag
    protected static final String PERIOD_WAKELOCK = "PeriodWakeLock";
    protected static final String NETWORK_WAKELOCK = "NetworkWakeLock";
    /**
     * @}
     */

    /**
     * Constructor
     */
    public PeriodicRefresher(String requester) {
        /**
         * M: Initialize WakeLock @{
         */
        PowerManager pm = (PowerManager) AndroidFactory.getApplicationContext().getSystemService(
                Context.POWER_SERVICE);
        mPeriodWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,PERIOD_WAKELOCK);
        mNetworkWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,NETWORK_WAKELOCK);
        /**
         * @}
         */
    }

    /**
     * Periodic processing
     */
    public abstract void periodicProcessing();

    /**
     * Start the timer
     *
     * @param expirePeriod Expiration period in seconds
     */
    public void startTimer(int expirePeriod) {
        startTimer(expirePeriod, 1.0);
    }

    /**
     * M: Modified to resolve the java exception issue. @{
     */
    /**
     * Start the timer
     *
     * @param expirePeriod Expiration period in seconds
     * @param delta Delta to apply on the expire period in percentage
     */
    public void startTimer(int expirePeriod, double delta) {
        // Check expire period
        if (expirePeriod <= 0) {
            // Expire period is null
            if (logger.isActivated()) {
                logger.debug("Timer is deactivated");
            }
            return;
        }

        // Calculate the effective refresh period
        pollingPeriod = (int)(expirePeriod * delta);
        if (logger.isActivated()) {
            logger.debug("Start timer at period=" + pollingPeriod +  "s (expiration=" + expirePeriod + "s)");
        }

        clearTimer();
        mPeriodicTimer = new Timer();
        mPeriodicTask = new PeriodicTask();
        mPeriodicTimer.schedule(mPeriodicTask,  pollingPeriod * 1000,  pollingPeriod * 1000);
    }

    /**
     * Stop the timer
     */
    public void stopTimer() {
        logger.debug("stopTimer() entry, current PeriodicRefresher is " + this);
        clearTimer();
    }

    private void clearTimer() {
        if (mPeriodicTimer != null) {
            mPeriodicTimer.cancel();
            mPeriodicTimer = null;
        }
        if (mPeriodicTask != null) {
            mPeriodicTask.cancel();
            mPeriodicTask = null;
        }
    }

    class PeriodicTask extends TimerTask {
        @ Override
        public void run() {
            logger.debug("start periodicProcessing");
            periodicProcessing();
        }
    }

    /**
     * M: Initialize WakeLock @{
     */
    protected void acquirePeriodWakeLock() {
        if (logger.isActivated()) {
            logger.debug("acquirePeriodWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (!mPeriodWakeLock.isHeld()) {
            mPeriodWakeLock.acquire();
        } else {
            if (logger.isActivated()) {
                logger.debug("mPeriodWakeLock is held, so do not acquire");
            }
        }
    }

    protected void acquireNetworkWakeLock() {
        if (logger.isActivated()) {
            logger.debug("acquireNetworkWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (!mNetworkWakeLock.isHeld()) {
            mNetworkWakeLock.acquire();
        } else {
            if (logger.isActivated()) {
                logger.debug("mNetworkWakeLock is held, so do not acquire");
            }
        }
    }

    protected void releasePeriodWakeLock() {
        if (logger.isActivated()) {
            logger.debug("releasePeriodWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (mPeriodWakeLock.isHeld()) {
            mPeriodWakeLock.release();
        } else {
            if (logger.isActivated()) {
                logger.debug("mPeriodWakeLock is not held, so do not release");
            }
        }
    }

    protected void releaseNetworkWakeLock() {
        if (logger.isActivated()) {
            logger.debug("releaseeNetworkWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (mNetworkWakeLock.isHeld()) {
            mNetworkWakeLock.release();
        } else {
            if (logger.isActivated()) {
                logger.debug("mPeriodWakeLock is not held, so do not release");
            }
        }
    }
    /**
     * @}
     */
}
