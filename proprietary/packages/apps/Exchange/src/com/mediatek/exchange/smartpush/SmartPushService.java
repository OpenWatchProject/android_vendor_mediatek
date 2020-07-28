package com.mediatek.exchange.smartpush;


import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.database.Cursor;
import android.os.BatteryManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

import com.android.emailcommon.Logging;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.HostAuth;
import com.android.emailcommon.provider.ProviderUnavailableException;
import com.android.emailcommon.provider.SmartPush;
import com.android.emailcommon.provider.EmailContent.AccountColumns;
import com.android.emailcommon.provider.EmailContent.HostAuthColumns;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;

import com.mediatek.protect.exchange.SmartPushCalculator;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * SmartPushService handles all aspects of Smart Push, such as
 * regularly wipe stale habit data, calculate sync interval from
 * the habit data for each account, change the sync interval on
 * schedule for each account, etc. It mainly relies on
 * AccountObserver to observer account's change to work)
 */
public class SmartPushService extends Service implements Runnable {
    private static final String TAG = "SmartPushService";

    private static final String WHERE_PROTOCOL_EAS = HostAuthColumns.PROTOCOL + "=\"" +
    HostAuth.LEGACY_SCHEME_EAS + "\"";

    // Sync frequency
    public static final int SYNC_FREQUENCY_HIGH = 2;
    public static final int SYNC_FREQUENCY_MEDIUM = 1;
    public static final int SYNC_FREQUENCY_LOW = 0;

    private static final int SECOND = 1000;
    private static final int MINUTE = 60 * SECOND;
    private static final int HOUR = 60 * MINUTE;
    private static final int DAY = 24 * HOUR;
    private static final int WEEK = 7 * DAY;

    // The singleton SmartPushService object, with its thread
    private static SmartPushService INSTANCE;
    private static Thread sServiceThread = null;

    private WakeLock mWakeLock = null;
    private PendingIntent mPendingIntent = null;

    // The millseconds of today's start time (GMT)
    private static long sTodayStartTime;
    // Keeps track of the calculatable accounts and the days of their habit data
    private HashMap<Long, Integer> mAccountMap = new HashMap<Long, Integer>();

    private static volatile boolean sStartingUp = false;
    private static volatile boolean sStop = false;

    // We synchronize on this for all actions affecting the service and error maps
    private static final Object sSyncLock = new Object();

    // Whether we have an unsatisfied "kick" pending
    private boolean mKicked = false;

    // Account observer to monitor the eas account deletion and sync interval changing.
    private final Handler mHandler = new Handler();
    private AccountObserver mAccountObserver;

    // Keep our cached list of active Accounts here
    public final AccountList mAccountList = new AccountList();

    // Power connection observer to monitor the power connection change
    private PowerConnectionReceiver mPowerConnectionReceiver;
    // The device is connected to power or not
    private boolean mDeviceIsConnectedToPower = false;
    // Keeps track of the accounts sync frequency change records
    // Record if the current is changed to push or not
    private HashMap<Long, Boolean> mSyncFrequencyChangedMap = new HashMap<Long, Boolean>();

    private static void startSmartPushService(Context context) {
        context.startService(new Intent(context, SmartPushService.class));
    }

    @Override
    public void run() {
        sStop = false;

        synchronized (sSyncLock) {
            mAccountObserver = new AccountObserver(mHandler);
            getContentResolver().registerContentObserver(Account.NOTIFIER_URI, true, mAccountObserver);
            // init before we start working for power connection change.
            init();
        }

        try {
            while (!sStop) {
                Logging.v(TAG, "SmartPushService loop one time");
                runAwake();
                // Delete the habit data older than 2 week
                deleteStaleData();
                // check is there any history data and exchange account to do smart push
                long nextCheckTime = shouldRunSmartPushService();
                if (nextCheckTime > 0) {
                    Logging.v(TAG, "No eligible smart push account found");
                    // Wait if no eligible smart push account found
                    runAsleep(nextCheckTime + (10 * SECOND));
                    try {
                        synchronized (this) {
                            // We expect the habit data is enough after this time
                            wait(nextCheckTime + (5 * SECOND));
                        }
                    } catch (InterruptedException e) {
                        // Needs to be caught, but causes no problem
                        Logging.v(TAG, "SmartPushService interrupted");
                    }
                    continue;
                }
                // check the next calculate wait time
                long nextCalculateWait = checkNextCalculateWait();
                if (nextCalculateWait < 10 * MINUTE) {
                    calculate();
                    nextCalculateWait = DAY;
                }
                // check the next action wait time that to change the sync frequency
                long nextActionWait = makeAdjustments();
                long nextWait = nextActionWait < nextCalculateWait ?
                        nextActionWait : nextCalculateWait;
                try {
                    synchronized (this) {
                        if (!mKicked) {
                            if (nextWait < 0) {
                                nextWait = 1 * SECOND;
                            }
                            if (nextWait > 10 * SECOND) {
                                runAsleep(nextWait + (3 * SECOND));
                            }
                            wait(nextWait);
                        }
                    }
                } catch (InterruptedException e) {
                    // Needs to be caught, but causes no problem
                    Logging.w(TAG, "SmartPushService interrupted");
                } finally {
                    synchronized (this) {
                        if (mKicked) {
                            Logging.v(TAG, "Wait deferred due to kick");
                            mKicked = false;
                        }
                    }
                }
            }
        } catch (RuntimeException e) {
           // Crash; this is a completely unexpected runtime error
            Logging.e(TAG, "RuntimeException in SmartPushService", e);
            throw e;
        } catch (Exception e) {
            Logging.e(TAG, "SmartPushService Exception occured", e);
            startService(new Intent(this, SmartPushService.class));
        } finally {
            shutdown();
        }
    }

    /**
     *  Check is there any history data and exchange account to do smart push
     * @return the next check time
     */
    private long shouldRunSmartPushService() {
        long current = System.currentTimeMillis();
        // "MOD" operation can exclude today's time
        long days = current / DAY;
        // Get the start time of today (GMT)
        sTodayStartTime = days * DAY;
        Logging.v(TAG, "Today start time: " + sTodayStartTime);
        mAccountMap.clear();
        long nextCheckTime = 1 * DAY;

        Cursor c = getContentResolver().query(Account.CONTENT_URI, new String[]{AccountColumns._ID,
                AccountColumns.FLAGS}, null, null, null);
        if (c != null) {
            try {
                while (c.moveToNext()) {
                    if ((c.getInt(1) & Account.FLAGS_SMART_PUSH) == 0) {
                        continue;
                    }
                    long accountId = c.getLong(0);
                    // Get the earliest habit data time stamp of the account
                    Long recordTimestamp = Utility.getFirstRowLong(this, SmartPush.CONTENT_URI,
                            new String[]{SmartPush.TIMESTAMP},
                            SmartPush.ACCOUNT_KEY + "=? AND " + SmartPush.EVENT_TYPE + " !=?",
                            new String[]{String.valueOf(accountId), String.valueOf(SmartPush.TYPE_MAIL)}, null, 0);
                    if (recordTimestamp != null) {
                        long timeSpan = sTodayStartTime - recordTimestamp;
                        Logging.v(TAG, "account " + accountId + " has " + timeSpan + "ms habit data");
                        // Only calculate for the account which habit data was recorded over 2 days
                        if (timeSpan >= 2 * DAY) {
                            long day = timeSpan / DAY;
                            Logging.v(TAG, "account " + accountId + " has " + day + " days habit data");
                            mAccountMap.put(accountId, Long.valueOf(day).intValue());
                            nextCheckTime = 0;
                        } else {
                            long timeToEnough = sTodayStartTime + DAY * (2 -
                                    (timeSpan < 0 ? -1 : timeSpan / DAY)) - current;
                            nextCheckTime = Math.min(nextCheckTime, timeToEnough);
                        }
                    } else {
                        Logging.v(TAG, "No habit data record for account " + accountId);
                    }
                }
            } finally {
                c.close();
            }
        }

        Logging.v(TAG, "The habit data will be enough after " + nextCheckTime);
        return nextCheckTime;
    }

    /**
     * Get the next calculate time in the light of the last calculate time
     * @return the remaining time to the next calculation
     */
    private long checkNextCalculateWait() {
        // Get the last calculate time
        SmartPushPreferences prefs = SmartPushPreferences.getPreferences(this);
        long lastCalculateTime = prefs.getLastCalculateTime();

        long sinceLastTime = System.currentTimeMillis() - lastCalculateTime;
        Logging.v(TAG, "since the last calculate time = " + sinceLastTime);
         if (sinceLastTime >= DAY) {
             return 0; // re-calculate now
         } else {
             return DAY - sinceLastTime;
         }
    }

    /**
     * The entry point of smart push calculation
     */
    private void calculate() {
        if (mAccountMap != null && mAccountMap.size() > 0) {
            Logging.v(TAG, "startCalculate...");
            long startTime = System.currentTimeMillis();

            Cursor[] cursors = new Cursor[mAccountMap.size()];
            int i = 0;
            try {
                for (Map.Entry<Long, Integer> entry : mAccountMap.entrySet()) {
                    cursors[i++] = getContentResolver().query(SmartPush.CONTENT_URI,
                            SmartPush.HABIT_PROJECTION, SmartPush.HABIT_SELECTION,
                            new String[]{String.valueOf(entry.getKey())}, null);
                }
                SmartPushCalculator.getCalculator().startCalculate(this, mAccountMap, cursors);
                Logging.v(TAG, "Calculate end!!! cost: " + (System.currentTimeMillis()
                        - startTime) + "ms");
            } finally {
                for (i = 0; i < cursors.length; i++) {
                    if (cursors[i] != null) {
                        cursors[i].close();
                    }
                }
            }
        }

        // Record the calculate finish time to the preference
        SmartPushPreferences prefs = SmartPushPreferences.getPreferences(this);
        prefs.settLastCalculateTime(System.currentTimeMillis());
    }

    /**
     * Change the sync interval for all the smart push accounts and return the time
     * to make the next adjustments
     * @return the time remaining for doing the next interval change for anyone account
     */
    private long makeAdjustments() {
        if (sStop) {
            return 0;
        }
        Logging.v(TAG, "makeAdjustments...");
        // Which time scale the current time in
        int scale = getCurrentScale();
        Logging.v(TAG, "current time scale: " + scale);
        // The time remaining to the next time scale
        long nextTimeLeast = 2 * HOUR - (System.currentTimeMillis() - sTodayStartTime) % (2 * HOUR);
        Logging.v(TAG, "The time remaining to the next time scale: " + nextTimeLeast);
        long minNextTime = Long.MAX_VALUE;

        // Synchronized here in order to avoid the possible case like:
        // user change the sync interval to non smart push, then ExchangeService's
        // AccountObserver::onAccountChanged change the sync interval, at the same
        // time below code may change the sync interval back.
//        synchronized(mAccountList) {
            Set<Map.Entry<Long, Integer>> entrySet = mAccountMap.entrySet();
            for (Map.Entry<Long, Integer> entry : entrySet) {
                long nextTime = nextTimeLeast;
                // Above all, check if the account is still a smart push one at present
                long accountId = (Long) entry.getKey();
                if (!SmartPush.isSmartPushAccount(this, accountId)) {
                    continue;
                }
                int[] result = SmartPushCalculator.getCalculator().getResult(accountId);
                // If could not get the calculation result, just return and calculate again.
                // It may happen at one account is eligible for calculating right now but the
                // next calcuation time has not reached yet
                if (result == null) {
                    SmartPushPreferences prefs = SmartPushPreferences.getPreferences(this);
                    prefs.removeLastCalculateTime();
                    return 0;
                }

                // Judge if we can change current account's sync frequency and record it
                boolean canChangeSycnFrequency = canChangeSyncFrequencyToPush(result[scale]);
                mSyncFrequencyChangedMap.put(accountId, canChangeSycnFrequency);
                Logging.v(TAG, "makeAdjustments canChangeSyncFrequency: " + canChangeSycnFrequency);

                changeSyncFrequency(canChangeSycnFrequency ?
                        SYNC_FREQUENCY_HIGH : result[scale], accountId);
                // Get the time remaining for doing the next interval change for anyone account,
                // Needless to wakeup to do the change if the next time scale interval is the
                // same to this one
                int i = scale;
                while (i < SmartPushCalculator.getScaleNum() - 1 && result[i] == result[++i]
                        && result[i] == SYNC_FREQUENCY_HIGH) {
                    // if current scale is push and next is also push,
                    // add two hours, otherwise not add. for we maybe had already connect to the power
                    // and no chance to receive the power connected broadcast
                    nextTime += 2 * HOUR;
                }
                minNextTime = Math.min(minNextTime, nextTime);
            }
//        }

        Logging.v(TAG, "The time remaining to the next adjustments: " + minNextTime);
        return minNextTime;
    }

    private void changeSyncFrequency(int syncFrequency, long accountId) {
        int syncInterval = Account.CHECK_INTERVAL_PUSH;
        switch(syncFrequency) {
            case SYNC_FREQUENCY_HIGH:
                syncInterval = Account.CHECK_INTERVAL_PUSH;
                break;
            case SYNC_FREQUENCY_MEDIUM:
                syncInterval = 60;
                break;
            case SYNC_FREQUENCY_LOW:
                syncInterval = Account.CHECK_INTERVAL_NEVER;
                break;
            default:
                break;
        }

        ContentValues cv = new ContentValues();
        cv.put(AccountColumns.SYNC_INTERVAL, syncInterval);
        // Suppose following sequency: 1. makeAdjustment find acount A is smart push. 2. User change the
        // the sync interval in AccountSettingsFragment. 3. makeAdjustment re-change the sync interval unexpectly.
        // The temp solution is just judge at the very end, even though this can not settle it thoroughly.
        // TODO: Synchronize this update with AccountSettingsFragment's update.
        if (SmartPush.isSmartPushAccount(this, accountId)) {
            getContentResolver().update(Account.CONTENT_URI, cv, AccountColumns._ID + "=?", new String[]{String.valueOf(accountId)});
        }
        Logging.v(TAG, "changeSyncFrequency to " + syncInterval + " for account " + accountId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        /** M: Stop the exchange service in the low storage mode @{ */
        if (Utility.mayLowStorage() ||
                Utility.isStorageLowExt(getApplicationContext())) {
            Logging.d(TAG, "Stops service in the low storage mode");
            stopSelf();
            return START_NOT_STICKY;
        }
        /** M: @} */
        if (!sStartingUp && INSTANCE == null) {
            sStartingUp = true;
            try {
                synchronized (sSyncLock) {
                    if ((sServiceThread == null || !sServiceThread.isAlive())
                            && EmailContent.count(this, HostAuth.CONTENT_URI,
                                    WHERE_PROTOCOL_EAS, null) > 0) {
                        // Should not start this thread if has no exchange account
                        sServiceThread = new Thread(this, "SmartPushService");
                        INSTANCE = this;
                        // If device rebooted, the calculation result will lose.
                        // Remove the last calculate time record when starting SmartPushService
                        // in order to recalculate again
                        SmartPushPreferences prefs = SmartPushPreferences.getPreferences(this);
                        prefs.removeLastCalculateTime();
                        Logging.v(TAG, "SmartPushService thread start to run");
                        sServiceThread.start();
                    }

                    if (sServiceThread == null) {
                        stopSelf();
                    }
                }
            } finally {
                sStartingUp = false;
            }
        }
        return Service.START_STICKY;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        EmailAsyncTask.runAsyncParallel(new Runnable() {
            @Override
            public void run() {
                // Quick checks first, before getting the lock
                if (sStartingUp) {
                    return;
                }
                synchronized (sSyncLock) {
                    Logging.v("!!! SmartPushService, onCreate");
                    // Try to start up properly; we might be coming back from a crash that the Email
                    // application isn't aware of.
                    startService(new Intent(SmartPushService.this, SmartPushService.class));
                    if (sStop) {
                        return;
                    }
                }
            }
        });
    }

    @Override
    public void onDestroy() {
        Logging.v(TAG, "SmartPushService onDestroy");
        // Unregister the previously registered mPowerConnectionReceiver.
        // Now it is no need to listen to the power connection change
        unregisterPowerConnectionReceiver();
        // Handle shutting down off the UI thread
        EmailAsyncTask.runAsyncParallel(new Runnable() {
            @Override
            public void run() {
                synchronized (sSyncLock) {
                    if (INSTANCE != null && sServiceThread != null) {
                    // Stop the smart push thread and return
                        sStop = true;
                        sServiceThread.interrupt();
                    }
                }
            }
        });
        super.onDestroy();
    }

    public static void alarmSmartPushService(Context context) {
        SmartPushService smartPushService = INSTANCE;
        if (smartPushService != null) {
            synchronized (smartPushService) {
                smartPushService.mKicked = true;
                Logging.v(TAG, "Alarm received: Kick");
                smartPushService.notify();
            }
        } else {
            Logging.v(TAG, "Alarm received: start smartpush service");
            startSmartPushService(context);
        }
    }

    private static void runAwake() {
        SmartPushService smartPushService = INSTANCE;
        if (smartPushService != null) {
            smartPushService.acquireWakeLock();
            smartPushService.clearAlarm();
        }
    }

    private static void runAsleep(long millis) {
        SmartPushService smartPushService = INSTANCE;
        if (smartPushService != null) {
            smartPushService.setAlarm(millis);
            smartPushService.releaseWakeLock();
        }
    }

    private void shutdown() {
        synchronized (sSyncLock) {
            sStop = false;
            INSTANCE = null;
            sServiceThread = null;

            if (mAccountObserver != null) {
                getContentResolver().unregisterContentObserver(mAccountObserver);
                mAccountObserver = null;
            }

            clearAlarm();

            // In extreme condition, this service may be killed (Low memory).
            // without releaseing the wakelock.
            if (mWakeLock != null && mWakeLock.isHeld()) {
                mWakeLock.release();
                mWakeLock = null;
            }
            Logging.v(TAG, "Goodbye");
        }
    }

    private void acquireWakeLock() {
        if (mWakeLock == null) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "SMARTPUSH_SERVICE");
            mWakeLock.acquire();
            Logging.v(TAG, "+SMARTPUSH_SERVICE WAKE LOCK ACQUIRED");
        }
    }

    private void releaseWakeLock() {
        if (mWakeLock != null) {
            mWakeLock.release();
        }
        mWakeLock = null;
        Logging.v(TAG, "-SMARTPUSH_SERVICE WAKE LOCK RELEASED");
    }

    private void clearAlarm() {
        if (mPendingIntent != null) {
            AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            alarmManager.cancel(mPendingIntent);
            Logging.v(TAG, "-Alarm cleared");
        }
    }

    private void setAlarm(long millis) {
        Intent i = new Intent(this, SmartPushAlarmReceiver.class);
        mPendingIntent = PendingIntent.getBroadcast(this, 0, i, 0);
        AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        alarmManager.set(AlarmManager.RTC_WAKEUP,
                System.currentTimeMillis() + millis, mPendingIntent);
        Logging.v(TAG, "+Alarm set for " + millis / 1000 + "s");
    }

    // Delete the habit data older than 2 week in case of the database expansion
    private void deleteStaleData() {
        long timeAfter = System.currentTimeMillis() - (2 * WEEK + 1);
        int deleted = getContentResolver().delete(SmartPush.CONTENT_URI,
                    SmartPush.TIMESTAMP + " < ?", new String[]{String.valueOf(timeAfter)});
        Logging.v(TAG, deleted + " rows stale habit data were deleted");
    }

    public static void kick(String reason) {
        SmartPushService smartPushService = INSTANCE;
        if (smartPushService != null) {
             synchronized (smartPushService) {
                 smartPushService.mKicked = true;
                 Logging.v(TAG, "Kick: " + reason);
                 smartPushService.notify();
             }
        }
    }

    class AccountObserver extends ContentObserver {
        private Thread mLastThread;

        // Runs when ExchangeService first starts
        public AccountObserver(Handler handler) {
            super(handler);
            Context context = getContext();
            try {
                collectEasAccounts(context, mAccountList);
            } catch (ProviderUnavailableException e) {
                // Just leave if EmailProvider is unavailable
                return;
            }
        }

        @Override
        public void onChange(boolean selfChange) {
            if (mLastThread == null || mLastThread.getState() == Thread.State.TERMINATED) {
                mLastThread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        onAccountChanged();
                    }
                }, "Account Observer");
                mLastThread.start();
            }
        }

        private void onAccountChanged() {
            Logging.v(TAG, "On account changed");
            try {
                Context context = getContext();
                if (context == null) {
                    Logging.d(TAG, "onAccountChanged but context is null");
                    return;
                }

                // Collect current accounts
                AccountList currentAccounts = new AccountList();
                try {
                    collectEasAccounts(context, currentAccounts);
                } catch (ProviderUnavailableException e) {
                    // Just leave if EmailProvider is unavailable
                    return;
                }

                // Stop SmartPushService since no EAS account exists
                if (currentAccounts.isEmpty()) {
                    stopSelf();
                    return;
                }

//                synchronized (mAccountList) {
                    // Get the newest version of each account
                    for (Account account : mAccountList) {
                        Account updatedAccount = currentAccounts.getById(account.mId);
                        if (updatedAccount == null) {
                            continue;
                        }

                        // Kick if the updated sync interval changed to smart push
                        if ((updatedAccount.mFlags & Account.FLAGS_SMART_PUSH) != 0
                                && (account.mFlags & Account.FLAGS_SMART_PUSH) == 0) {
                            Logging.v(TAG, "the updated sync interval changed to smart push");
                            SmartPushPreferences prefs = SmartPushPreferences.getPreferences(SmartPushService.this);
                            prefs.removeLastCalculateTime();
                            SmartPushService.kick("account changed to smart push");
                        }
                    }

                    // Look for new accounts
                    for (Account account : currentAccounts) {
                        if (!mAccountList.contains(account.mId) && SmartPush.isSmartPushAccount(SmartPushService.this, account.mId)) {
                            // Kick the SmartPushService to calculate again for the new smart push account
                            SmartPushPreferences prefs = SmartPushPreferences.getPreferences(SmartPushService.this);
                            prefs.removeLastCalculateTime();
                            SmartPushService.kick("smart push account added");
                        }
                    }
                    mAccountList.clear();
                    mAccountList.addAll(currentAccounts);
//                }
            } catch (ProviderUnavailableException e) {
                Logging.d(TAG, "Observer failed; provider unavailable");
            }
        }
    }

    /**
     * Return a list of all Accounts in EmailProvider.  Because the result of this call may be used
     * in account reconciliation, an exception is thrown if the result cannot be guaranteed accurate
     * @param context the caller's context
     * @param accounts a list that Accounts will be added into
     * @return the list of Accounts
     * @throws ProviderUnavailableException if the list of Accounts cannot be guaranteed valid
     */
    private static void collectEasAccounts(Context context, AccountList accounts) {
        ContentResolver resolver = context.getContentResolver();
        Cursor c = resolver.query(Account.CONTENT_URI, Account.CONTENT_PROJECTION, null, null,
                null);
        // We must throw here; callers might use the information we provide for reconciliation, etc.
        if (c == null) {
            throw new ProviderUnavailableException();
        }
        try {
            while (c.moveToNext()) {
                long hostAuthId = c.getLong(Account.CONTENT_HOST_AUTH_KEY_RECV_COLUMN);
                if (hostAuthId > 0) {
                    HostAuth ha = HostAuth.restoreHostAuthWithId(context, hostAuthId);
                    if (ha != null && ha.mProtocol.equals("eas")) {
                        Account account = new Account();
                        account.restore(c);
                        // Cache the HostAuth
//                        account.mHostAuthRecv = ha;
                        accounts.add(account);
                    }
                }
            }
        } finally {
            c.close();
        }
    }

    static class AccountList extends ArrayList<Account> {
        private static final long serialVersionUID = 1L;

        @Override
        public boolean add(Account account) {
            super.add(account);
            return true;
        }

        public boolean contains(long id) {
            for (Account account : this) {
                if (account.mId == id) {
                    return true;
                }
            }
            return false;
        }

        public Account getById(long id) {
            for (Account account : this) {
                if (account.mId == id) {
                    return account;
                }
            }
            return null;
        }

        public Account getByName(String accountName) {
            for (Account account : this) {
                if (account.mEmailAddress.equalsIgnoreCase(accountName)) {
                    return account;
                }
            }
            return null;
        }
    }

    public static Context getContext() {
        return INSTANCE;
    }

    private void init() {
        //Clear the map before we can start our work
        mSyncFrequencyChangedMap.clear();
        // Before register the receiver, first to check if we have already connected to power
        initPowerConnectionStatus();
        //register for power change
        registerPowerConnectionReceiver();
    }

     //Initialize the status of whether the device is connected to power
    private void initPowerConnectionStatus() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = registerReceiver(null, filter);
        int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        int chargePlug = batteryStatus.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
        mDeviceIsConnectedToPower = (status == BatteryManager.BATTERY_STATUS_CHARGING
                          || chargePlug == BatteryManager.BATTERY_PLUGGED_AC
                          || chargePlug == BatteryManager.BATTERY_PLUGGED_USB);
        Logging.v(TAG, "mDeviceIsConnectedToPower: " + mDeviceIsConnectedToPower);
    }

    // Register a receiver to know whether the device is connected to power
    private void registerPowerConnectionReceiver() {
        mPowerConnectionReceiver = new PowerConnectionReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_POWER_DISCONNECTED);
        intentFilter.addAction(Intent.ACTION_POWER_CONNECTED);
        registerReceiver(mPowerConnectionReceiver, intentFilter);
    }

    // Unregister the previously registered mPowerConnectionReceiver.
    // Now it is no need to listen to the power connection change
    private void unregisterPowerConnectionReceiver() {
        if (null != mPowerConnectionReceiver) {
            unregisterReceiver(mPowerConnectionReceiver);
            mPowerConnectionReceiver = null;
        }
    }

    /**
     * Receiver for getting the power connection changed information
     * when we are connected to power, we can maximize the rate of background
     * updates for it cost less power at this time
     */
    private class PowerConnectionReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            PowerManager pm = null;
            WakeLock wakeLock = null;
            try {
                pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
                wakeLock = pm.newWakeLock(
                        PowerManager.PARTIAL_WAKE_LOCK, "SMARTPUSH_SERVICE_POWER");
                wakeLock.acquire();
                Logging.v(TAG, "BEGIN SMARTPUSH_SERVICE_CHARGING WAKE LOCK ACQUIRED");

                String action = intent.getAction();
                if (Intent.ACTION_POWER_CONNECTED.equals(action)) {
                    mDeviceIsConnectedToPower = true;
                } else if (Intent.ACTION_POWER_DISCONNECTED.equals(action)) {
                    mDeviceIsConnectedToPower = false;
                }
                changeSyncStatus();
            } catch (Exception e) {
                Logging.e(TAG, "PowerConnectionReceiver Exception occured", e);
            } finally {
                if (null != wakeLock) {
                    wakeLock.release();
                    Logging.v(TAG, "END SMARTPUSH_SERVICE_CHARGING WAKE LOCK RELEASED");
                }
            }
        }
    }

    /**
     * return true means we can change syncInterval to push, otherwise return false.
     * only when match below conditions shall we return true:<br >
     * 1. The device is charging<br >
     * 2. Current time is not sleeping time(00:00 am - 8:00 am)<br >
     * 3. Current scale is not SYNC_FREQUENCY_HIGH
     */
    private boolean canChangeSyncFrequencyToPush(int currentSyncFrequency) {
        return mDeviceIsConnectedToPower
                && currentSyncFrequency != SYNC_FREQUENCY_HIGH
                && currentSyncFrequency != SYNC_FREQUENCY_LOW
                && (Calendar.getInstance().get(Calendar.HOUR_OF_DAY) < 8 ? false : true);
    }

    /**
     * Change the sync frequency when power connection changed,
     * change to high when we are connected to power, otherwise to what it should be.
     */
    private void changeSyncStatus() {
        if (sStop) {
            return;
        }

        if (mAccountMap != null && mAccountMap.size() > 0) {
            HashMap<Long, Boolean> syncFrequencyChangedMap = new HashMap<Long, Boolean>();
            int scale = getCurrentScale();
            Logging.v(TAG, "current time scale: " + scale);
            Set<Map.Entry<Long, Integer>> entrySet = mAccountMap.entrySet();
            for (Map.Entry<Long, Integer> entry : entrySet) {
                long accountId = (Long) entry.getKey();
                if (!SmartPush.isSmartPushAccount(this, accountId)) {
                    continue;
                }
                int[] result = SmartPushCalculator.getCalculator().getResult(accountId);
                if (result == null) {
                    return ;
                }

                boolean canChangeSyncFrequency = canChangeSyncFrequencyToPush(result[scale]);
                boolean lastTimechanged = (null == mSyncFrequencyChangedMap.get(accountId))
                        ? false : mSyncFrequencyChangedMap.get(accountId);
                if (canChangeSyncFrequency && !lastTimechanged) {
                    changeSyncFrequency(SYNC_FREQUENCY_HIGH, accountId);
                } else if (!canChangeSyncFrequency && lastTimechanged) {
                    changeSyncFrequency(result[scale], accountId);
                }
                Logging.v(TAG, "changeSyncStatus Can change: " + canChangeSyncFrequency
                        + " ,Last time changed: " + lastTimechanged);
                syncFrequencyChangedMap.put(accountId, canChangeSyncFrequency);
            }
            //Update the map, remove the account is not smart push account
            mSyncFrequencyChangedMap = syncFrequencyChangedMap;
        }
    }

    private int getCurrentScale() {
        int scale = Long.valueOf((System.currentTimeMillis() - sTodayStartTime) / (2 * HOUR))
                .intValue();
        // In a kind of extreme case, the scale value may be 12, take it as 11 in this case
        if (scale == 12) {
            scale = 11;
        }
        return scale;
    }
}
