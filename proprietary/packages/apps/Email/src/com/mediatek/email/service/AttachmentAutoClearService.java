package com.mediatek.email.service;

import com.android.email.EmailApplication;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.android.mail.utils.LogUtils;
import com.mediatek.email.attachment.AttachmentAutoClearController;

import java.util.HashMap;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;

public class AttachmentAutoClearService extends Service {
    public static final String TAG = "AttachmentAutoClearService";

    /** Time definitions */
    public static final long ONE_DAY_TIME = 24 * 60 * 60 * 1000;
    private static final long CLEAR_CACHE_PERIOD = ONE_DAY_TIME;

    /** Actions for auto internal storage control */
    public static final String ACTION_CLEAR_OLD_ATTACHMENT_ONCE =
            "com.android.email.intent.action.MAIL_SERVICE_CLEAR_OLD_ATTACHMENT_ONCE";
    public static final String ACTION_CLEAR_OLD_ATTACHMENT =
            "com.android.email.intent.action.MAIL_SERVICE_CLEAR_OLD_ATTACHMENT";
    public static final String ACTION_RESCHEDULE_CLEAR_OLD_ATTACHMENT =
            "com.android.email.intent.action.MAIL_SERVICE_RESCHEDULE_CLEAR_OLD_ATTACHMENT";
    public static final String ACTION_CANCEL_CLEAR_OLD_ATTACHMENT =
            "com.android.email.intent.action.MAIL_SERVICE_CANCEL_CLEAR_OLD_ATTACHMENT";

    /** Power Manager Service */
    private PowerManager mPowerManager;
    /** Alarm Manager Service */
    private AlarmManager mAlarmManager;

    /** The real Wakelock to be held */
    private WakeLock mWakeLock;

    /** Keeps track of which action require a wake lock */
    private final HashMap<String, Long> mWakeLocks = new HashMap<String, Long>();

    /**
     * Return a pending intent for starting service to clear old attachments
     * @param isWatchdog
     * @return
     */
    private PendingIntent createAlarmIntentForAutoClear() {
        Intent i = new Intent();
        i.setClass(this, AttachmentAutoClearService.class);
        i.setAction(ACTION_CLEAR_OLD_ATTACHMENT);
        PendingIntent pi = PendingIntent.getService(this, 0, i, PendingIntent.FLAG_UPDATE_CURRENT);
        return pi;
    }

    /**
     * reschedule action: clear old attachment
     * @param alarmMgr
     */
    private void rescheduleAutoClear() {
        PendingIntent pi = createAlarmIntentForAutoClear();
        long timeNow = SystemClock.elapsedRealtime();
        mAlarmManager.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, timeNow + CLEAR_CACHE_PERIOD, pi);
        LogUtils.d(TAG, "MailService reschedule clear old attachments: alarm set at "
                + (timeNow + CLEAR_CACHE_PERIOD));
    }

    /**
     * Do the clear work asynchronously, on a specific serial executor, and release wakelock after
     * @param action the Action to be done
     */
    private void autoClearAsyncAndReleaseWakeLock(final String action) {
        new EmailAsyncTask<String, Void, String>(null) {

            @Override
            protected String doInBackground(String... action) {
                String result = action[0];
                // if remaining space size <= 20% of total space
                if (Utility.mayLowStorage()) {
                    // This will clear the attachment files in messages 3 days before
                    // @NOTE: some special cases
                    // @NOTE: we don't need to care about attachments in outbox/sentbox/draft,
                    // cause they'll be cleared automatically after the message has been sent,
                    // and you can't delete them when they are in outbox/draft
                    LogUtils.d(TAG, "Start to clear internal attachments");
                    AttachmentAutoClearController.deleteInternalAttachmentsDaysBefore(
                            AttachmentAutoClearService.this, 3);
                }
                return result;
            }

            protected void onSuccess(String action) {
                // Release wakelock of the action when finished
                releaseWakeLock(action);
            };
            protected void onCancelled(String action) {
                // Release wakelock of the action when finished
                releaseWakeLock(action);
            };
        } .executeExecutor(EmailAsyncTask.SERIAL_EXECUTOR_FOR_AUTO_CLEAR_ATTACH, action);
    }

    /**
     * Cancel the alarm for clear old attachment
     */
    private void cancelAutoClear() {
        PendingIntent pi = this.createAlarmIntentForAutoClear();
        mAlarmManager.cancel(pi);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        // Skip on invalid intent
        if (intent == null) {
            return START_NOT_STICKY;
        }
        String action = intent.getAction();
        EmailApplication.checkAndStartForeground(this, 3, "AttachmentAutoClearService, onStartCommand");
        // Get AlarmManager at very beginning
        if (mAlarmManager == null) {
            mAlarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        }

        // Just return if the action has already been run
        if (isWakelockHeld(action)) {
            return START_NOT_STICKY;
        }
        // Grab the wakelock to make sure our service running
        acquireWakeLock(action);

        boolean releaseAtOnce = false;
        LogUtils.d(TAG, "AttachmentAutoClearService action: " + action);
        if (ACTION_CLEAR_OLD_ATTACHMENT_ONCE.equals(action)) {
            autoClearAsyncAndReleaseWakeLock(action);
        } else if (ACTION_CLEAR_OLD_ATTACHMENT.equals(action)) {
            rescheduleAutoClear();
            autoClearAsyncAndReleaseWakeLock(action);
        } else if (ACTION_RESCHEDULE_CLEAR_OLD_ATTACHMENT.equals(action)) {
            rescheduleAutoClear();
            releaseAtOnce = true;
        } else if (ACTION_CANCEL_CLEAR_OLD_ATTACHMENT.equals(action)) {
            cancelAutoClear();
            releaseAtOnce = true;
        }

        if (releaseAtOnce) {
            // Release wakelock of the actions were finished
            releaseWakeLock(action);
        }
        EmailApplication.checkAndStopForeground(this, "AttachmentAutoClearService, onStartCommand");
        stopSelf(startId);
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /**
     * Check whether an action has already a wakelock been holden
     * @param action  the Action to be done
     * @return true if held, otherwise false
     */
    private boolean isWakelockHeld(String action) {
        synchronized (mWakeLocks) {
            return mWakeLocks.get(action) != null;
        }
    }

    /**
     * Acquire wake lock as needed
     * We just hold one real wakelock here and record others
     * @param action the Action to be done
     */
    private void acquireWakeLock(String action) {
        synchronized (mWakeLocks) {
            Long lock = mWakeLocks.get(action);
            if (lock == null) {
                if (mWakeLock == null) {
                    PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
                    mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "ATTACHMENT_AUTO_CLEAR");
                    mWakeLock.acquire();
                    LogUtils.d(TAG, "+ATTACHMENT_AUTO_CLEAR WAKE LOCK ACQUIRED");
                }
                // Set current time as value of wakelock
                long startTime = System.currentTimeMillis();
                mWakeLocks.put(action, startTime);
                LogUtils.d(TAG, ">>>>>>> Acquire request for lock action:%s at:[%d]", action, startTime);
             }
        }
    }

    /**
     * Release wakelock when not even one lock held
     * @param action The Action just be done
     */
    private void releaseWakeLock(String action) {
        synchronized (mWakeLocks) {
            Long lock = mWakeLocks.get(action);
            if (lock != null) {
                // Remove lock record from map
                Long startTime = mWakeLocks.remove(action);
                if (mWakeLocks.isEmpty()) {
                    if (mWakeLock != null) {
                        // Release real wakelock when is no more used
                        mWakeLock.release();
                    }
                    mWakeLock = null;
                    LogUtils.d(TAG, "-ATTACHMENT_AUTO_CLEAR WAKE LOCK RELEASED");
                } else {
                    LogUtils.d(TAG, "<<<<<<< Release request for lock action:%s at:[%d]", action, startTime);
                }
            }
        }
    }
    public void onDestroy() {
        LogUtils.d(TAG, "onDestroy");
        EmailApplication.checkAndStopForeground(this, "AttachmentAutoClearService, onDestroy");
    }
}
