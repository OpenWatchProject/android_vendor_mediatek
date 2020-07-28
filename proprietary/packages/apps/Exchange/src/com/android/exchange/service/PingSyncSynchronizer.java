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

package com.android.exchange.service;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;
import android.support.v4.util.LongSparseArray;
import android.text.format.DateUtils;

import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.Mailbox;
import com.android.exchange.Eas;
import com.android.exchange.eas.EasOperation;
import com.android.exchange.eas.EasPing;
import com.android.mail.utils.LogUtils;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Bookkeeping for handling synchronization between pings and other sync related operations.
 * "Ping" refers to a hanging POST or GET that is used to receive push notifications. Ping is
 * the term for the Exchange command, but this code should be generic enough to be extended to IMAP.
 *
 * Basic rules of how these interact (note that all rules are per account):
 * - Only one operation (ping or other active sync operation) may run at a time.
 * - For shorthand, this class uses "sync" to mean "non-ping operation"; most such operations are
 *   sync ops, but some may not be (e.g. EAS Settings).
 * - Syncs can come from many sources concurrently; this class must serialize them.
 *
 * WHEN A SYNC STARTS:
 * - If nothing is running, proceed.
 * - If something is already running: wait until it's done.
 * - If the running thing is a ping task: interrupt it.
 *
 * WHEN A SYNC ENDS:
 * - If there are waiting syncs: signal one to proceed.
 * - If there are no waiting syncs and this account is configured for push: start a ping.
 * - Otherwise: This account is now idle.
 *
 * WHEN A PING TASK ENDS:
 * - A ping task loops until either it's interrupted by a sync (in which case, there will be one or
 *   more waiting syncs when the ping terminates), or encounters an error.
 * - If there are waiting syncs, and we were interrupted: signal one to proceed.
 * - If there are waiting syncs, but the ping terminated with an error: TODO: How to handle?
 * - If there are no waiting syncs and this account is configured for push: This means the ping task
 *   was terminated due to an error. Handle this by sending a sync request through the SyncManager
 *   that doesn't actually do any syncing, and whose only effect is to restart the ping.
 * - Otherwise: This account is now idle.
 *
 * WHEN AN ACCOUNT WANTS TO START OR CHANGE ITS PUSH BEHAVIOR:
 * - If nothing is running, start a new ping task.
 * - If a ping task is currently running, restart it with the new settings.
 * - If a sync is currently running, do nothing.
 *
 * WHEN AN ACCOUNT WANTS TO STOP GETTING PUSH:
 * - If nothing is running, do nothing.
 * - If a ping task is currently running, interrupt it.
 */
public class PingSyncSynchronizer {

    private static final String TAG = Eas.LOG_TAG;

    /// M: Set back off time to 2 mins to reduce ping frequency
    private static final long SYNC_ERROR_BACKOFF_MILLIS =  DateUtils.MINUTE_IN_MILLIS * 2;

    // Enable this to make pings get automatically renewed every hour. This
    // should not be needed, but if there is a software error that results in
    // the ping being lost, this is a fallback to make sure that messages are
    // not delayed more than an hour.
    private static final boolean SCHEDULE_KICK = true;
    private static final long KICK_SYNC_INTERVAL_SECONDS =
            DateUtils.HOUR_IN_MILLIS / DateUtils.SECOND_IN_MILLIS;

    /// M: Keeps track of which services require a wake lock (by account id)
    private final HashMap<Long, Long> mWakeLocks = new HashMap<Long, Long>();
    /// M: Keeps track of which services have held a wake lock (by account id)
    private final HashMap<Long, Long> mWakeLocksHistory = new HashMap<Long, Long>();
    /// M: The actual WakeLock obtained by Exchange
    private WakeLock mWakeLock = null;

    /**
     * This class handles bookkeeping for a single account.
     */
    private class AccountSyncState {
        /** The currently running {@link PingTask}, or null if we aren't in the middle of a Ping. */
        private PingTask mPingTask;

        /**
         * M: Change pushEnabled to type int to indicate whether account had been initialized
         * Tracks whether this account wants to get push notifications, based on calls to
         * {@link #pushModify} and {@link #pushStop} (i.e. it tracks the last requested push state).
         * @{
         */
        private int mPushEnabled;
        private static final int PUSH_INIT = 0;
        private static final int PUSH_ENABLED = 1;
        private static final int PUSH_DISABLED = 2;
        /** M: @} */

        /**
         * The number of syncs that are blocked waiting for the current operation to complete.
         * Unlike Pings, sync operations do not start their own tasks and are assumed to run in
         * whatever thread calls into this class.
         */
        private int mSyncCount;

        /** The condition on which to block syncs that need to wait. */
        private Condition mCondition;

        /** The accountId for this accountState, used for logging */
        private long mAccountId;

        public AccountSyncState(final Lock lock, final long accountId) {
            mPingTask = null;
            mPushEnabled = PUSH_INIT;
            mSyncCount = 0;
            mCondition = lock.newCondition();
            mAccountId = accountId;
        }

        /**
         * Update bookkeeping for a new sync:
         * - Stop the Ping if there is one.
         * - Wait until there's nothing running for this account before proceeding.
         */
        public void syncStart() throws InterruptedException {
            ++mSyncCount;
            if (mPingTask != null) {
                // Syncs are higher priority than Ping -- terminate the Ping.
                LogUtils.i(TAG, "PSS Sync is pre-empting a ping acct:%d", mAccountId);
                mPingTask.stop();
            }
            if (mPingTask != null || mSyncCount > 1) {
                // There’s something we need to wait for before we can proceed.
                try {
                    LogUtils.i(TAG, "PSS Sync needs to wait: Ping: %s, Pending tasks: %d acct: %d",
                            mPingTask != null ? "yes" : "no", mSyncCount, mAccountId);
                    mCondition.await();
                } catch (InterruptedException e) {
                    /**
                     * M: Sync operation await failed, if not handle it it will signal other
                     * sync operation in the queue.
                     * Handle and throws exception to EasService, but not signal other pending
                     * sync operation. @{
                     */
                    LogUtils.e(TAG, "PSS sync await failed acct:%d, syncCount:%d,"
                            + " exception:%s", mAccountId, mSyncCount, e.getMessage());
                    // skip it
                    --mSyncCount;
                    throw new InterruptedException("Sync await failed");
                    /** @} */
                }
            }
        }

        /**
         * Update bookkeeping when a sync completes. This includes signaling pending ops to
         * go ahead, or starting the ping if appropriate and there are no waiting ops.
         * @return Whether this account is now idle.
         */
        public boolean syncEnd(final boolean lastSyncHadError, final Account account,
                               final PingSyncSynchronizer synchronizer) {
            LogUtils.d(TAG, "SyncEnd with syncCount %d, pushEnabled %d, lastSyncError %s.",
                    mSyncCount, mPushEnabled, lastSyncHadError);
            --mSyncCount;
            if (mSyncCount > 0) {
                LogUtils.i(TAG, "PSS Signalling a pending sync to proceed acct:%d.",
                        account.getId());
                mCondition.signal();
                return false;
            } else {
                /**
                 * M: PushEnabled is not initialized when account created
                 * so that we need to check it from account for that. @{
                 */
                if (account == null) {
                    return true;
                }
                if (mPushEnabled == PUSH_INIT) {
                    mPushEnabled = EasService.pingNeededForAccount(mService, account)
                            ? PUSH_ENABLED : PUSH_DISABLED;
                    LogUtils.d(TAG, "SyncEnd with pushEnabled[%d]", mPushEnabled);
                }
                /** M: @} */

                if (mPushEnabled == PUSH_ENABLED) {
                    if (lastSyncHadError) {
                        /// M: Let EasService to handle delayed ping schedule
                        LogUtils.i(TAG, "PSS last sync had error, scheduling delayed ping acct:%d.",
                                account.getId());
                        scheduleDelayedPing(synchronizer.getContext(), account);
                        return true;
                    } else {
                        LogUtils.i(TAG, "PSS last sync succeeded, starting new ping acct:%d.",
                                account.getId());
                        final android.accounts.Account amAccount =
                                new android.accounts.Account(account.mEmailAddress,
                                        Eas.EXCHANGE_ACCOUNT_MANAGER_TYPE);
                        mPingTask = new PingTask(synchronizer.getContext(), account, amAccount,
                                synchronizer);
                        mPingTask.start();
                        return false;
                    }
                }
            }
            LogUtils.i(TAG, "PSS no push enabled acct:%d.", account.getId());
            return true;
        }

        /// M: delay the ping under some case in case of high power consumption
        private boolean needToDelayNextPing(int pingStatus) {
            return pingStatus == EasOperation.RESULT_NETWORK_PROBLEM
                    || pingStatus == EasOperation.RESULT_OTHER_FAILURE;
        }
        /**
         * Update bookkeeping when the ping task terminates, including signaling any waiting ops.
         * @param pingStatus M: result of the ping
         * @return Whether this account is now idle.
         */
        private boolean pingEnd(final Account account, int pingStatus) {
            LogUtils.d(TAG, "pingEnd with syncCount %d, pushEnabled %d.", mSyncCount, mPushEnabled);
            mPingTask = null;
            if (mSyncCount > 0) {
                LogUtils.i(TAG, "PSS pingEnd, syncs still in progress acct:%d.", mAccountId);
                mCondition.signal();
                return false;
            } else {
                if (mPushEnabled == PUSH_ENABLED) {
                    /**
                     * M: To not drain battery to fast when there is something
                     * wrong with server for IOException, ping need to back off
                     * somehow. @{
                     */
                    if (account == null) {
                        // Remove account from accountState if not exist
                        return true;
                    }
                    if (needToDelayNextPing(pingStatus)) {
                        LogUtils.d(TAG, "pingEnd with Errors, back off a litte while " + pingStatus);
                        scheduleDelayedPing(getContext(), account);
                    } else {
                        LogUtils.d(TAG, "pingEnd and request another ping at once");
                        /**
                         * This situation only arises if we encountered some sort of error that
                         * stopped our ping but not due to a sync interruption. In this scenario
                         * we'll leverage the SyncManager to request a push only sync that will
                         * restart the ping when the time is right. */
                        final android.accounts.Account amAccount =
                                new android.accounts.Account(account.mEmailAddress,
                                        Eas.EXCHANGE_ACCOUNT_MANAGER_TYPE);
                        EasPing.requestPing(amAccount);
                    }
                    /** M: @} */
                    return false;
                }
            }
            LogUtils.i(TAG, "PSS pingEnd, no longer need ping acct:%d.", mAccountId);
            return true;
        }

        /**
         * M: Schedule delayed ping by EasService
         * @param context
         * @param account account for ping
         * TODO: Set flexible back off process to delay from 15s to 4mins
         */
        private void scheduleDelayedPing(final Context context,
                                         final Account account) {
            LogUtils.i(TAG, "PSS Scheduling a delayed ping acct:%d.", account.getId());
            final Intent intent = new Intent(context, EasService.class);
            intent.setAction(Eas.EXCHANGE_SERVICE_INTENT_ACTION);
            intent.putExtra(Eas.EXTRA_START_PING, true);
            intent.putExtra(Eas.EXTRA_PING_ACCOUNT, account);
            final PendingIntent pi = PendingIntent.getService(context, 0, intent,
                    PendingIntent.FLAG_ONE_SHOT);
            final AlarmManager am = (AlarmManager)context.getSystemService(
                    Context.ALARM_SERVICE);
            final long atTime = SystemClock.elapsedRealtime() + SYNC_ERROR_BACKOFF_MILLIS;
            am.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, atTime, pi);
        }

        /**
         * Modifies or starts a ping for this account if no syncs are running.
         */
        public void pushModify(final Account account, final PingSyncSynchronizer synchronizer) {
            LogUtils.i(LogUtils.TAG, "PSS pushModify acct:%d", account.getId());
            mPushEnabled = PUSH_ENABLED;
            final android.accounts.Account amAccount =
                    new android.accounts.Account(account.mEmailAddress,
                            Eas.EXCHANGE_ACCOUNT_MANAGER_TYPE);
            LogUtils.d(TAG, "pushModify with syncCount %d.", mSyncCount);
            if (mSyncCount == 0) {
                if (mPingTask == null) {
                    // No ping, no running syncs -- start a new ping.
                    LogUtils.i(LogUtils.TAG, "PSS starting ping task acct:%d", account.getId());
                    mPingTask = new PingTask(synchronizer.getContext(), account, amAccount,
                            synchronizer);
                    mPingTask.start();
                } else {
                    // Ping is already running, so tell it to restart to pick up any new params.
                    LogUtils.i(LogUtils.TAG, "PSS restarting ping task acct:%d", account.getId());
                    mPingTask.restart();
                }
            } else {
                LogUtils.i(LogUtils.TAG, "PSS syncs still in progress acct:%d", account.getId());
            }
            if (SCHEDULE_KICK) {
                final Bundle extras = new Bundle(1);
                extras.putBoolean(Mailbox.SYNC_EXTRA_PUSH_ONLY, true);
                ContentResolver.addPeriodicSync(amAccount, EmailContent.AUTHORITY, extras,
                        KICK_SYNC_INTERVAL_SECONDS);
            }
        }

        /**
         * Stop the currently running ping.
         */
        public void pushStop() {
            LogUtils.i(LogUtils.TAG, "PSS pushStop acct:%d", mAccountId);
            mPushEnabled = PUSH_DISABLED;
            if (mPingTask != null) {
                mPingTask.stop();
            }
        }
    }

    /**
     * Lock for access to {@link #mAccountStateMap}, also used to create the {@link Condition}s for
     * each Account.
     */
    private final ReentrantLock mLock;

    /**
     * Map from account ID -> {@link AccountSyncState} for accounts with a running operation.
     * An account is in this map only when this account is active, i.e. has a ping or sync running
     * or pending. If an account is not in the middle of a sync and is not configured for push,
     * it will not be here. This allows to use emptiness of this map to know whether the service
     * needs to be running, and is also handy when debugging.
     */
    private final LongSparseArray<AccountSyncState> mAccountStateMap;

    /** The {@link Service} that this object is managing. */
    private final Service mService;

    public PingSyncSynchronizer(final Service service) {
        mLock = new ReentrantLock();
        mAccountStateMap = new LongSparseArray<AccountSyncState>();
        mService = service;
    }

    public Context getContext() {
        return mService;
    }

    /**
     * Gets the {@link AccountSyncState} for an account.
     * The caller must hold {@link #mLock}.
     * @param accountId The id for the account we're interested in.
     * @param createIfNeeded If true, create the account state if it's not already there.
     * @return The {@link AccountSyncState} for that account, or null if the account is idle and
     *         createIfNeeded is false.
     */
    private AccountSyncState getAccountState(final long accountId, final boolean createIfNeeded) {
        assert mLock.isHeldByCurrentThread();
        AccountSyncState state = mAccountStateMap.get(accountId);
        if (state == null && createIfNeeded) {
            LogUtils.i(TAG, "PSS adding account state for acct:%d", accountId);
            state = new AccountSyncState(mLock, accountId);
            mAccountStateMap.put(accountId, state);
            // TODO: Is this too late to startService?
            if (mAccountStateMap.size() == 1) {
                LogUtils.i(TAG, "PSS added first account, starting service");
                mService.startService(new Intent(mService, mService.getClass()));
            }
        }
        return state;
    }

    /**
     * Remove an account from the map. If this was the last account, then also stop this service.
     * The caller must hold {@link #mLock}.
     * @param accountId The id for the account we're removing.
     */
    private void removeAccount(final long accountId) {
        assert mLock.isHeldByCurrentThread();
        LogUtils.i(TAG, "PSS removing account state for acct:%d", accountId);
        mAccountStateMap.delete(accountId);
        if (mAccountStateMap.size() == 0) {
            LogUtils.i(TAG, "PSS removed last account; stopping service.");
            mService.stopSelf();
        }
    }

    public void syncStart(final long accountId) throws InterruptedException{
        mLock.lock();
        try {
            LogUtils.i(TAG, "PSS syncStart for account acct:%d", accountId);
            final AccountSyncState accountState = getAccountState(accountId, true);
            accountState.syncStart();
        } finally {
            mLock.unlock();
        }
    }

    /**
     * M: The parameter would be a null object, if the EasOperation failed to
     * initialize the Account object. For example, the EasOperation
     * initialization ran behind the account delete operation.
     *
     * @param lastSyncHadError sync error id
     * @param account Account object maybe null
     * @param accountId accountId corresponding the account parameter.
     */
    public void syncEnd(final boolean lastSyncHadError, final Account account,
            final long accountId) {
        mLock.lock();
        try {
            LogUtils.d(TAG, "PSS syncEnd for account %d", accountId);
            final AccountSyncState accountState = getAccountState(accountId, false);
            if (accountState == null) {
                LogUtils.w(TAG, "PSS syncEnd for account %d but no state found", accountId);
                return;
            }
            if (accountState.syncEnd(lastSyncHadError, account, this)) {
                removeAccount(accountId);
            }
        } finally {
            mLock.unlock();
        }
    }

    public void pingEnd(final long accountId, final Account account,
            /** M: Result of the ping */
            int pingStatus) {
        LogUtils.d(TAG, "ready to PSS pingEnd for account");
        mLock.lock();
        try {
            LogUtils.i(TAG, "PSS pingEnd for account %d", accountId);
            final AccountSyncState accountState = getAccountState(accountId, false);
            if (accountState == null) {
                LogUtils.w(TAG, "PSS pingEnd for account %d but no state found", accountId);
                return;
            }
            /// M: Add result of the ping to save battery life
            if (accountState.pingEnd(account, pingStatus)) {
                removeAccount(accountId);
            }
        } finally {
            /// M: release wake lock for ping of current account
            releaseWakeLock(accountId);
            mLock.unlock();
        }
    }

    public void pushModify(final Account account) {
        LogUtils.d(TAG, "ready to PSS pingModify for account");
        mLock.lock();
        try {
            final long accountId = account.getId();
            LogUtils.i(TAG, "PSS pushModify acct:%d", accountId);
            final AccountSyncState accountState = getAccountState(accountId, true);
            accountState.pushModify(account, this);
        } finally {
            mLock.unlock();
        }
    }

    public void pushStop(final long accountId) {
        LogUtils.d(TAG, "ready to PSS pingStop for account");
        mLock.lock();
        try {
            LogUtils.i(TAG, "PSS pushStop acct:%d", accountId);
            final AccountSyncState accountState = getAccountState(accountId, false);
            if (accountState != null) {
                accountState.pushStop();
            }
        } finally {
            mLock.unlock();
        }
    }

    /**
     * Stops our service if our map contains no active accounts.
     */
    public void stopServiceIfIdle() {
        mLock.lock();
        try {
            LogUtils.i(TAG, "PSS stopIfIdle");
            if (mAccountStateMap.size() == 0) {
                LogUtils.i(TAG, "PSS has no active accounts; stopping service.");
                mService.stopSelf();
            }
        } finally {
            mLock.unlock();
        }
    }

    /**
     * Tells all running ping tasks to stop.
     */
    public void stopAllPings() {
        LogUtils.d(TAG, "ready to PSS StopAllPing");
        mLock.lock();
        try {
            for (int i = 0; i < mAccountStateMap.size(); ++i) {
                mAccountStateMap.valueAt(i).pushStop();
            }
        } finally {
            mLock.unlock();
        }
    }

    /**
     * M: Check wake lock held by a specific account
     * @param id id of a account
     * @return true if held, otherwise false
     */
    public boolean hasWakeLock(long id) {
        synchronized (mWakeLocks) {
            return mWakeLocks.get(id) != null;
        }
    }

    /**
     * M: Acquire wake lock if necessary for a specific account
     * @param id id of a account
     */
    public void acquireWakeLock(long id) {
        synchronized (mWakeLocks) {
            LogUtils.i(TAG, "ACQUIRE wake lock for account[%d]", id);
            Long lock = mWakeLocks.get(id);
            if (lock == null) {
                if (mWakeLock == null) {
                    PowerManager pm = (PowerManager) getContext().getSystemService(Context.POWER_SERVICE);
                    mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "EAS_PUSH");
                    mWakeLock.acquire();
                    // STOPSHIP Remove
                    LogUtils.i(TAG, "+WAKE LOCK ACQUIRED");
                }
                mWakeLocks.put(id, System.currentTimeMillis());
             }
        }
    }

    /**
     * M: Release wake lock when need to sleep
     * @param id id of a account
     */
    public void releaseWakeLock(long id) {
        synchronized (mWakeLocks) {
            Long lock = mWakeLocks.get(id);
            if (lock != null) {
                LogUtils.i(TAG, "RELEASED wake lock for account[%d]", id);
                Long startTime = mWakeLocks.remove(id);
                Long historicalTime = mWakeLocksHistory.get(id);
                if (historicalTime == null) {
                    historicalTime = 0L;
                }
                mWakeLocksHistory.put(id,
                        historicalTime + (System.currentTimeMillis() - startTime));
                if (mWakeLocks.isEmpty()) {
                    if (mWakeLock != null) {
                        mWakeLock.release();
                    }
                    mWakeLock = null;
                    // STOPSHIP Remove
                    LogUtils.i(TAG, "+WAKE LOCK RELEASED");
                } else {
                    LogUtils.i(TAG, "Release request for lock not held: " + id);
                }
            }
        }
    }

    /**
     * M: dump debug information
     * @param fd
     * @param writer
     * @param args
     */
    protected void dump(FileDescriptor fd, PrintWriter writer, String[] args) {
        synchronized (mWakeLocks) {
            // Dump for debugging wake lock histories
            if (mWakeLock != null) {
                writer.println("  Holding WakeLock");
                writeWakeLockTimes(writer, mWakeLocks, false);
            } else {
                writer.println("  Not holding WakeLock");
            }
            if (!mWakeLocksHistory.isEmpty()) {
                writer.println("  Historical times");
                writeWakeLockTimes(writer, mWakeLocksHistory, true);
            }
        }
    }

    /**
     * M: dump for debugging wake lock histories
     * @param pw
     * @param map
     * @param historical
     */
    public void writeWakeLockTimes(PrintWriter pw, HashMap<Long, Long> map, boolean historical) {
        long now = System.currentTimeMillis();
        for (long accountId : map.keySet()) {
            Long time = map.get(accountId);
            if (time == null) {
                // Just in case...
                continue;
            }

            Account account = Account.restoreAccountWithId(mService, accountId);
            StringBuilder sb = new StringBuilder();
            if (accountId == -1) {
                sb.append("    EasSync");
            } else if (account == null) {
                sb.append("    Account " + accountId + " (deleted?)");
            } else {
                String protocol = Account.getProtocol(mService, accountId);
                sb.append("    Account " + accountId + " (" + protocol + ")");
            }
            long logTime = historical ? time : (now - time);
            sb.append(" held for " + (logTime / 1000) + "s");
            pw.println(sb.toString());
        }
    }
}
