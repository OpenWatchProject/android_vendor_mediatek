/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
package com.mediatek.presence.core.ims.service.presence;

import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;

import javax2.sip.header.SIPETagHeader;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.LocalBroadcastManager;

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.platform.registry.RegistryFactory;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipTransactionContext;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Publish manager for sending current user presence status
 * with de-bounce timer
 *
 * it's just for AT&T
 */
public class PublishManagerEx extends PublishManager {
    private static Logger logger = Logger.getLogger("PublishManagerEx");
    private Context mContext = AndroidFactory.getApplicationContext();
    public static final String SIP_489_BAD_EVENT_STATE =
            "com.mediatek.presence.SIP_489_BAD_EVENT_STATE";
    final static public String ACTION_RESET_489_STATE = "android.intent.presence.RESET_489_STATE";
    final static public String EXTRA_489_EXPIRED_TIME = "489ExpiredTime";

    //de-bounce support for AT&T
    private static long BAD_EVENT_TIME = 72 * 60 * 60 * 1000;

    private PublishManagerExTimer mDeBounceTimer = null;
    private PublishManagerExTimer mBadEventTimer = null;
    private BroadcastReceiver m489StateReceiver;
    private PublishManagerExReceiver mPublishManagerExReceiver = new PublishManagerExReceiver();

    private String mPublishInfo;
    private long mLastPublishTime;
    private int mSlotId = 0;

    public PublishManagerEx(final ImsModule parent) {
        super(parent);
        mSlotId = parent.getSlotId();
        BAD_EVENT_TIME = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .get489BadEventExpiredTime();
        if (logger.isActivated()) {
            logger.debug("BAD_EVENT_TIME: " + BAD_EVENT_TIME / 1000);
        }
        mBadEventTimer = new PublishManagerExTimer();

        // because only publish can quit bad event state
        // so start timer here.
        this.mPublishInfo = null;
        this.mLastPublishTime = 0;
        mDeBounceTimer = new PublishManagerExTimer();

        initPublishManagerExReceiver();
    }

    /**
     * Publish presence status
     *
     * @param info Presence info
     * @return Boolean
     */
    @Override
    public synchronized boolean publish(String info) {
        //in 489 bad event && not first time
        if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .is489BadEventState() && !(mLastPublishTime == 0)) {
            if (logger.isActivated()) {
                logger.debug("Not Published in 489 state");
            }
            if (mDeBounceTimer.isStarted())
                mDeBounceTimer.stop();
            return false;
        }
        if (mDeBounceTimer.isStarted()) {
            mPublishInfo = info;
            if (logger.isActivated()) {
                logger.debug("Save Publish Info");
            }
            return true;
        } else {
            long currentTime = System.currentTimeMillis();
            long waitTime = currentTime - mLastPublishTime;
            long throttleTime = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .getSourceThrottlePublish() * 1000;

            if (logger.isActivated()) {
                logger.debug("publish: current/lastPublish " + currentTime + "/" +
                    mLastPublishTime + ", wait=" + waitTime + " throttle:" + throttleTime);
            }
            if (waitTime >= throttleTime) {
                mLastPublishTime = currentTime;
                return super.publish(info);
            } else {
                if (logger.isActivated()) {
                    logger.debug("Start DeBounceTimer");
                }
                mPublishInfo = info;
                ThrottleTask throttleTask = new ThrottleTask();
                mDeBounceTimer.start(throttleTask, throttleTime - waitTime);
                return true;
            }
        }
    }

    @Override
    public synchronized void unPublish() {
        if (mDeBounceTimer.isStarted())
            mDeBounceTimer.stop();
        super.unPublish();
    }

    /**
     * Start the timer
     *
     * @param expirePeriod Expiration period in seconds
     */
    @Override
    public void startTimer(int expirePeriod) {
        startTimer(expirePeriod, 0.8);
        return;
    }

    public void init() {
        super.init();
        logger.debug("PublishManagerEx init()");
        //refresh it when init.
        BAD_EVENT_TIME = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .get489BadEventExpiredTime();
        if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .is489BadEventState()) {
            long badEventTime = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .get489BadEventTime();
            long current = System.currentTimeMillis();
            if (current - badEventTime >= BAD_EVENT_TIME) {
                RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                        .set489BadEventState(false);
            }
            if (logger.isActivated()) {
                logger.debug("PublishManagerEx Constructor in 489 state:" + current
                    + "/" + badEventTime);
            }
        }
        if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .is489BadEventState()) {
            startQuitBadEventTimer();
        } else {
            m489StateReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    int slotId = intent.getIntExtra("slotId", -1);
                    logger.debug("Receive SIP_489_BAD_EVENT_STATE, slotId: "
                            + slotId + " ,mSlotId: " + mSlotId);
                    if (mSlotId != slotId) {
                        return;
                    }
                    if (!RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                            .is489BadEventState()) {
                        RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                                .set489BadEventState(true);
                        RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                                .set489BadEventTime(System.currentTimeMillis());
                        startQuitBadEventTimer();
                    }
                    mContext.unregisterReceiver(this);
                    m489StateReceiver = null;
                }
            };
            LocalBroadcastManager.getInstance(mContext).registerReceiver(
                    m489StateReceiver, new IntentFilter(SIP_489_BAD_EVENT_STATE));
        }
        initPublishManagerExReceiver();
    }

    private void initPublishManagerExReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_RESET_489_STATE);
        mPublishManagerExReceiver.register(mContext, filter);
    }

    /**
     * Terminate manager
     */
    @Override
    public void terminate() {
        try {
            mLastPublishTime = 0;
            if (mDeBounceTimer.isStarted())
                mDeBounceTimer.stop();
            if (mBadEventTimer.isStarted())
                mBadEventTimer.stop();
            if (m489StateReceiver != null)
                LocalBroadcastManager.getInstance(mContext).unregisterReceiver(
                        m489StateReceiver);

            mPublishManagerExReceiver.unregister(mContext);
            super.terminate();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void handle200OK(SipTransactionContext ctx) {
        // 200 OK response received
        if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .is489BadEventState()) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .set489BadEventState(false);
            if (mBadEventTimer.isStarted())
                mBadEventTimer.stop();
        }
        super.handle200OK(ctx);
    }

    @Override
    protected void handle489BadEvent(SipTransactionContext ctx) throws Exception {
        //
        if (logger.isActivated()) {
            logger.debug("handle489BadEvent");
        }

        published = false;

        if (!RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .is489BadEventState()) {
            logger.debug("set 489 event time.");
            RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .set489BadEventState(true);
            RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .set489BadEventTime(System.currentTimeMillis());
            startQuitBadEventTimer();
        }
        saveEntityTag(null);

        return;
    }

    /**
     * Create a new dialog path
     *
     * @return Dialog path
     */
    @Override
    protected SipDialogPath createDialogPath() {
        // Set Call-Id
        String callId = imsModule.getSipManager().getSipStack().generateCallId();

        if (logger.isActivated()) {
            logger.debug("imsModule.IMS_USER_PROFILE : " +imsModule.IMS_USER_PROFILE.toString());
        }

        // Set target
        String target = ImsModule.IMS_USER_PROFILE.getPublicUri();

        // Set local party
        String localParty = ImsModule.IMS_USER_PROFILE.getPublicUri();

        // Set remote party
        String remoteParty = ImsModule.IMS_USER_PROFILE.getPublicUri();

        // Set the route path
        Vector<String> route = imsModule.getSipManager().getSipStack().getServiceRoutePath();

        // Create a dialog path
        SipDialogPath dialog = new SipDialogPath(
                imsModule.getSipManager().getSipStack(),
                callId,
                1,
                target,
                localParty,
                remoteParty,
                route);
        return dialog;
    }

    private void startQuitBadEventTimer() {
        long badEventTime = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .get489BadEventTime();
        long current = System.currentTimeMillis();
        BadEventTask badEventTask = new BadEventTask();
        mBadEventTimer.start(badEventTask,
                BAD_EVENT_TIME - (current - badEventTime));
    }

    class ThrottleTask extends TimerTask {
        @ Override
        public void run() {
            logger.debug("throttle timeout, resume publish");
            mDeBounceTimer.setTimerStarted(false);
            publish(mPublishInfo);
        }
    }

    class BadEventTask extends TimerTask {
        @ Override
        public void run() {
            logger.debug("489 bad event timeout, publish capabilities");
            RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .set489BadEventState(false);
            mBadEventTimer.setTimerStarted(false);
            imsModule.getPresenceService().publishCapability();
        }
    }

    static class PublishManagerExTimer {
        private Context mContext;

        private volatile boolean mTimerStarted = false;
        private Timer mTimer;
        private TimerTask mTask;

        public PublishManagerExTimer() {}

        public synchronized void start(TimerTask task, long timer) {
            logger.debug("PublishManagerExTimer.start(), task:"
                    + task + " timer:" + timer + " started:" + mTimerStarted);

            if (mTimerStarted)
                return;

            clearInfo();
            mTimer = new Timer();
            mTask = task;

            mTimer.schedule(mTask,  timer);
            mTimerStarted = true;
        }

        public synchronized void stop() {
            logger.debug("PublishManagerExTimer.stop(), task:"
                    + mTask + " timer:" + mTimer + " started:" + mTimerStarted);

            if (!mTimerStarted)
                return;

            clearInfo();
            mTimerStarted = false;
        }

        private synchronized void clearInfo() {
            logger.debug("PublishManagerExTimer.clearInfo()");

            if (mTimer != null) {
                mTimer.cancel();
                mTimer = null;
            }

            if (mTask != null) {
                mTask.cancel();
                mTask = null;
            }
        }

        public synchronized boolean isStarted() {
            return mTimerStarted == true;
        }

        public synchronized void setTimerStarted(boolean start) {
            logger.debug("PublishManagerExTimer.setTimerStarted, start:" + start);
            mTimerStarted = start;
        }
    }

    private class PublishManagerExReceiver extends BroadcastReceiver {
        private boolean isRegistered;

        /**
         * register receiver
         * @param context - Context
         * @param filter - Intent Filter
         * @return see Context.registerReceiver(BroadcastReceiver,IntentFilter)
         */
        public Intent register(Context context, IntentFilter filter) {
            try {
                return !isRegistered
                       ? context.registerReceiver(this, filter)
                       : null;
            } finally {
                isRegistered = true;
            }
        }

        /**
         * unregister received
         * @param context - context
         * @return true if was registered else false
         */
        public boolean unregister(Context context) {
            return isRegistered
                   && unregisterInternal(context);
        }

        private boolean unregisterInternal(Context context) {
            context.unregisterReceiver(this);
            isRegistered = false;
            return true;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int slotId = intent.getIntExtra("slotId", 0);
            logger.debug("onReceive, intent action is " + action + " and slotId is " + slotId);

            if (action.equalsIgnoreCase(ACTION_RESET_489_STATE)
                    && slotId == mSlotId) {
                BAD_EVENT_TIME = intent.getLongExtra(EXTRA_489_EXPIRED_TIME, BAD_EVENT_TIME);

                if (logger.isActivated()) {
                    logger.debug("Receive RESET 489 state intent, set BAD_EVENT_TIME time "
                                 + BAD_EVENT_TIME / 1000);
                }
                if (mBadEventTimer.isStarted()) {
                    mBadEventTimer.stop();
                }
                if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                        .is489BadEventState()) {
                    long badEventTime = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                            .get489BadEventTime();
                    long current = System.currentTimeMillis();
                    if (logger.isActivated()) {
                        logger.debug("Reset 489 state:" + current
                                     + "/" + badEventTime);
                    }
                    if (current - badEventTime >= BAD_EVENT_TIME) {
                        RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                                .set489BadEventState(false);
                        imsModule.getPresenceService().publishCapability();
                    } else {
                        startQuitBadEventTimer();
                    }
                }
            }
        }
    }
}
