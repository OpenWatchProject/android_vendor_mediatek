package com.mediatek.amplus;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.ContentObserver;
import android.hardware.usb.UsbManager;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplayStatus;
import android.os.BatteryManager;
import android.os.Build;
import android.os.Handler;
import android.os.SystemClock;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Slog;
import android.os.Binder;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.AlarmManager.RTC;
import static android.app.AlarmManager.ELAPSED_REALTIME_WAKEUP;
import static android.app.AlarmManager.ELAPSED_REALTIME;
import com.mediatek.provider.MtkSettingsExt;
public class PowerSavingUtils {

    private boolean mScreenOff = false;
    private long mScreenOffTime = 0;
    private static final long SCREENOFF_TIME_INTERVAL_THRESHOLD = 5 * 60 * 1000;
    private static final int NEW_POWER_SAVING_MODE = 2;
    //private static final long SCREENOFF_TIME_INTERVAL_THRESHOLD = 5*1000;
    private boolean mIsEnabled = false;
    private boolean mIsUsbConnected = false;
    private boolean mIsWFDConnected = false;
    private PowerSavingEnableObserver mPowerSavingEnableObserver = null;
    private final Context mContext;
    private static final String TAG = "AlarmManager";
    private static final String FILEPATH = "/system/etc/alarmplus.config";
    private PowerSavingReceiver mPowerSavingReceiver;
    final ArrayList<String> mWhitelist = new ArrayList<String>();
    private int mSavingMode = 0;
    public PowerSavingUtils(Context context) {
        mContext = context;
        init();
    }

    private void init() {
        /// M: Apply JPE protection code @{
        com.mediatek.common.jpe.a aa = new com.mediatek.common.jpe.a();
        aa.a();
        /// M: Apply JPE protection code @}
        readList();
        mPowerSavingReceiver = new PowerSavingReceiver();
        mPowerSavingEnableObserver = new PowerSavingEnableObserver(null);
    }
   /**
     *for reading white list
     */
    private void readList() {
        File WhitelistFile = new File(FILEPATH);
        if (!WhitelistFile.exists()) {
            return;
        }
        try {
            BufferedReader br = new BufferedReader(new FileReader(WhitelistFile));
            String line = br.readLine();
            while (line != null) {
                mWhitelist.add(line);
                line = br.readLine();
            }
            br.close();
        } catch (IOException e) {
            //Log.e(PackageManagerService.TAG, "IO Exception happened while reading resmon whitelist");
            e.printStackTrace();
        }
    }
   /**
     *for judging if the alarm need to align or not.
     */
    private boolean isAlarmNeedAlign(int type, PendingIntent operation, boolean isExactAlarm) {

        boolean isAlarmNeedAlign = false;

        do {
            if (!isPowerSavingStart()) {
                break;
            }

            if (mSavingMode != NEW_POWER_SAVING_MODE) {
                // M: mark for alarm grouping phase 2
                if ((type != RTC_WAKEUP
                        && type != ELAPSED_REALTIME_WAKEUP)) {
                    break;
                }
            }
            // null operation is set with android, need skip
            if (operation == null) {
                Slog.v(TAG, "isAlarmNeedAlign : operation is null");
                break;
            }

            String packageName = operation.getTargetPackage();
            if (packageName == null) {
                Slog.v(TAG, "isAlarmNeedAlign : packageName is null");
                break;
            }

            for (int i = 0; i < mWhitelist.size(); i++) {
                if (mWhitelist.get(i).equals(packageName)) {
                    Slog.v(TAG, "isAlarmNeedAlign : packageName = " + packageName + "is in whitelist");
                    return false;
                }
            }
            if (isExactAlarm) {
                PackageManager pm  = mContext.getPackageManager();
                try {
                    final long origId = Binder.clearCallingIdentity();
                    ApplicationInfo info = pm.getApplicationInfo(packageName, 0);
                    Binder.restoreCallingIdentity(origId);
                    //M: android system app will skip only
                    if (((info.flags & ApplicationInfo.FLAG_SYSTEM) != 0)
                        && (packageName.startsWith("com.android")
                        || packageName.startsWith("com.google.android.deskclock")
                        || packageName.startsWith("android"))) {
                        if (Build.TYPE.equals("eng"))
                            Slog.v(TAG, "isAlarmNeedAlign : " + packageName + " skip!");
                        break;
                    }

                } catch (NameNotFoundException e) {
                    Slog.v(TAG, "isAlarmNeedAlign : packageName not fount");
                    break;
                }
            }
            isAlarmNeedAlign = true;
        } while(false);
        return isAlarmNeedAlign;
    }

   /**
     *This API for get the mtk alarm grouping's max trigger time.
     */
    private long getMTKMaxTriggerTime(int type, PendingIntent operation, long triggerAtTime, boolean needGrouping) {
        if (!needGrouping) {
            return triggerAtTime;
        }
        if (isAlarmNeedAlign(type, operation, true) && needGrouping) {
            return triggerAtTime + SCREENOFF_TIME_INTERVAL_THRESHOLD;
        }
        return (0 - triggerAtTime);
    }
/*
 * This API for check AlarmGrouping start working or not
*/
    public boolean isPowerSavingStart() {
        if (!mIsEnabled) {
            return false;
        }

        if (mIsUsbConnected) {
            return false;
        }

        if (mIsWFDConnected) {
            return false;
        }

        if (mScreenOff) {
            long currentTime = System.currentTimeMillis();
            long screenOffThreshold = 0;
            if (mSavingMode != NEW_POWER_SAVING_MODE) {
                screenOffThreshold = 5 * 60 * 1000;
            } else {
                screenOffThreshold = 1 * 60 * 1000;
            }
            if ((currentTime - mScreenOffTime) < screenOffThreshold) {
                if (Build.TYPE.equals("eng")) {
                    Slog.v(TAG, "mScreenOff time is not enough");
                }
                return false;
            }
        } else {
            return false;
        }
        return true;
    }
   /**
     *This API for adjust the KK's max trigger time.
     */
    private static final long MIN_FUZZABLE_INTERVAL = 10000;
    private long adjustMaxTriggerTime(long now, long triggerAtTime, long interval, PendingIntent operation, int type, boolean needGrouping, boolean isExactAlarm) {
        long futurity = (interval == 0)
                ? (triggerAtTime - now)
                : interval;
        if (futurity < MIN_FUZZABLE_INTERVAL) {
            futurity = 0;
        }

        long maxTriggerAtTime = triggerAtTime + (long) (.75 * futurity);

        if (mSavingMode != NEW_POWER_SAVING_MODE) {
            if (!needGrouping) {
                return maxTriggerAtTime;
            }
            if (isAlarmNeedAlign(type, operation, true)) {
                if ((maxTriggerAtTime - triggerAtTime) < SCREENOFF_TIME_INTERVAL_THRESHOLD) {
                    return triggerAtTime + SCREENOFF_TIME_INTERVAL_THRESHOLD;
                } else {
                    return maxTriggerAtTime;
                }
            } else {
                return (0 - maxTriggerAtTime);
            }
        } else {
            if (isAlarmNeedAlign(type, operation, isExactAlarm)
                    && ((maxTriggerAtTime - triggerAtTime) < SCREENOFF_TIME_INTERVAL_THRESHOLD)) {
                return triggerAtTime + SCREENOFF_TIME_INTERVAL_THRESHOLD;
            } else {
                return (0 - maxTriggerAtTime);
            }
        }
    }

    /**
     * This API for alarmManagerService to get the max trigger time
     *
     * @param type One of {@link #ELAPSED_REALTIME}, {@link #ELAPSED_REALTIME_WAKEUP},
     *        {@link #RTC}, or {@link #RTC_WAKEUP}.
     * @param triggerElapsed time in milliseconds that the alarm should first
     *        go off, using the appropriate clock (depending on the alarm type).
     * @param windowLength The length of the requested delivery window,
     *        in milliseconds.  The alarm will be delivered no later than this many
     *        milliseconds after {@code windowStartMillis}.  Note that this parameter
     *        is a <i>duration,</i> not the timestamp of the end of the window.
     * @param intervalMillis interval in milliseconds between subsequent repeats
     *        of the alarm.
     * @param operation Action to perform when the alarm goes off;
     *        typically comes from {@link PendingIntent#getBroadcast
     *        IntentSender.getBroadcast()}.
     */
    public long getMaxTriggerTime(int type, long triggerElapsed, long windowLength, long interval,
        PendingIntent operation , int mAlarmMode,boolean needGrouping) {
        mSavingMode = mAlarmMode;
        final long nowElapsed = SystemClock.elapsedRealtime();
        final long maxElapsed;


        if (windowLength == AlarmManager.WINDOW_EXACT) {
            maxElapsed = getMTKMaxTriggerTime(type, operation, triggerElapsed, needGrouping);
        } else if (windowLength == AlarmManager.WINDOW_HEURISTIC) {
            maxElapsed = adjustMaxTriggerTime(nowElapsed, triggerElapsed, interval, operation, type, needGrouping, false);
        } else {
            maxElapsed = triggerElapsed + windowLength;
        }
        return maxElapsed;
    }

    private void setPowerSavingEnable() {
        int isEnabled = 0;
        isEnabled =  Settings.System.getInt(mContext.getContentResolver(),
                                 MtkSettingsExt.System.BG_POWER_SAVING_ENABLE, 1);
        mIsEnabled = (isEnabled != 0) ? true : false;
    }

    class PowerSavingReceiver extends BroadcastReceiver {
        public PowerSavingReceiver() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_SCREEN_OFF);
            filter.addAction(Intent.ACTION_SCREEN_ON);
            filter.addAction(UsbManager.ACTION_USB_STATE);
            // mIsWFDConnected registerReceiver
            filter.addAction(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);
            mContext.registerReceiver(this, filter);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (Intent.ACTION_SCREEN_OFF.equals(intent.getAction())) {
                mScreenOff = true;
                mScreenOffTime = System.currentTimeMillis();
            } else if (Intent.ACTION_SCREEN_ON.equals(intent.getAction())) {
                mScreenOff = false;
                mScreenOffTime = 0;
            } else if (UsbManager.ACTION_USB_STATE.equals(intent.getAction())) {
                mIsUsbConnected = intent.getBooleanExtra(UsbManager.USB_CONNECTED, false);
            } else if (DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED.
                       equals(intent.getAction())) {
                WifiDisplayStatus wfdStatus = (WifiDisplayStatus)
                intent.getParcelableExtra(DisplayManager.EXTRA_WIFI_DISPLAY_STATUS);
                mIsWFDConnected =
                (WifiDisplayStatus.DISPLAY_STATE_CONNECTED == wfdStatus.getActiveDisplayState());
                Slog.v(TAG, "PowerSavingReceiver mIsWFDConnected = " + mIsWFDConnected);
            }
        }
    }


    class PowerSavingEnableObserver extends ContentObserver {
        PowerSavingEnableObserver(Handler handler) {
            super(handler);
            observe();

        }

        void observe() {
            ContentResolver resolver = mContext.getContentResolver();
            resolver.registerContentObserver(Settings.System.getUriFor(MtkSettingsExt.System.BG_POWER_SAVING_ENABLE),
                false,
                this,
                UserHandle.USER_ALL);
            setPowerSavingEnable();
        }

        public void onChange(boolean selfChange) {
            setPowerSavingEnable();
        }
    }
}
