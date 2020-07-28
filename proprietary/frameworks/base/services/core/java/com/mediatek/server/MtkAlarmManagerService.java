/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.server;
import com.android.server.*;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.AppOpsManager;
import android.app.BroadcastOptions;
import android.app.IAlarmCompleteListener;
import android.app.IAlarmListener;
import android.app.IAlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PermissionInfo;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.Process;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.WorkSource;
import android.provider.Settings;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.text.format.Time;
import android.util.ArrayMap;
import android.util.KeyValueListParser;
import android.util.Log;
import android.util.Slog;
import android.util.SparseArray;
import android.util.SparseBooleanArray;
import android.util.SparseLongArray;
import android.util.TimeUtils;

import java.io.ByteArrayOutputStream;
import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Locale;
import java.util.Random;
import java.util.TimeZone;
import java.util.TreeSet;

import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.AlarmManager.RTC;
import static android.app.AlarmManager.ELAPSED_REALTIME_WAKEUP;
import static android.app.AlarmManager.ELAPSED_REALTIME;
/// M: added for powerOffAlarm feature @{
import static android.app.AlarmManager.PRE_SCHEDULE_POWER_OFF_ALARM;
///@}
/// M: For handling non-wakeup alarms while WFD is connected
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplayStatus;
///@}

public class MtkAlarmManagerService extends AlarmManagerService{

    /// M: added for powerOffAlarm feature @{
    private Object mWaitThreadlock = new Object();
    private Object mPowerOffAlarmLock = new Object();
    private final ArrayList<Alarm> mPoweroffAlarms = new ArrayList<Alarm>();
    private long mNativeData;
    //PowerOff alarm will be preponed by these many seconds.
    static final long POWER_OFF_ALARM_BUFFER_TIME = 150*1000; // 150 seconds
    ///@}

    /// M: For handling non-wakeup alarms while WFD is connected
    WFDStatusChangedReceiver mWFDStatusChangedReceiver;
    boolean mIsWFDConnected = false;
    ///@}

    public MtkAlarmManagerService(Context context) {
        super(context);
    }

    /// M: For handling non-wakeup alarms while WFD is connected
    @Override
    protected void registerWFDStatusChangeReciever(){
        mWFDStatusChangedReceiver = new WFDStatusChangedReceiver();
    }

    @Override
    protected boolean isWFDConnected(){
        return mIsWFDConnected;
    }
    ///@}

    @Override
    public void onStart() {
       super.onStart();
       if (mInjector != null) {
           mNativeData = mInjector.getNativeData();
       }
    }

    /// M: For handling non-wakeup alarms while WFD is connected
    class WFDStatusChangedReceiver extends BroadcastReceiver {
        public WFDStatusChangedReceiver() {
            IntentFilter filter = new IntentFilter();
            // mIsWFDConnected registerReceiver
            filter.addAction(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);
            getContext().registerReceiver(this, filter);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED.equals(intent.getAction())) {

                WifiDisplayStatus wfdStatus = (WifiDisplayStatus)
                intent.getParcelableExtra(DisplayManager.EXTRA_WIFI_DISPLAY_STATUS);
                mIsWFDConnected =
                (WifiDisplayStatus.DISPLAY_STATE_CONNECTED == wfdStatus.getActiveDisplayState());
            }
        }
    }
    ///@}

    /// M: added for powerOffAlarm feature @{
    @Override
    protected boolean isPowerOffAlarmType(int type){
        if(type != PRE_SCHEDULE_POWER_OFF_ALARM)
            return false;
        else
            return true;
    }

    @Override
    protected boolean schedulePoweroffAlarm(int type,
        long triggerAtTime,long interval,PendingIntent operation,IAlarmListener directReceiver,
        String listenerTag,WorkSource workSource,AlarmManager.AlarmClockInfo alarmClock,
        String callingPackage)
    {
        /// M:add for PowerOffAlarm feature(type 7) for booting phone before actual alarm@{
        if (type == PRE_SCHEDULE_POWER_OFF_ALARM) {
            if (mNativeData == -1) {
                Slog.w(TAG, "alarm driver not open ,return!");
                return false;
            }

            Slog.d(TAG, "alarm set type 7 , package name " + operation.getTargetPackage());
            String packageName = operation.getTargetPackage();

            String setPackageName = null;
            long nowTime = System.currentTimeMillis();
            triggerAtTime = triggerAtTime - POWER_OFF_ALARM_BUFFER_TIME;

            if (triggerAtTime < nowTime) {
                Slog.w(TAG, "PowerOff alarm set time is wrong! nowTime = " + nowTime
                       + " ; triggerAtTime = " + triggerAtTime);
                return false;
            }
            Slog.d(TAG, "PowerOff alarm TriggerTime = " + triggerAtTime +" now = " + nowTime);
            synchronized (mPowerOffAlarmLock) {
                removePoweroffAlarmLocked(operation.getTargetPackage());
                final int poweroffAlarmUserId = UserHandle.getCallingUserId();
                Alarm alarm = new Alarm(type, triggerAtTime, 0, 0, 0,
                        interval, operation, directReceiver, listenerTag,
                        workSource, 0, alarmClock,
                        poweroffAlarmUserId, callingPackage);
                addPoweroffAlarmLocked(alarm);
                if (mPoweroffAlarms.size() > 0) {
                    resetPoweroffAlarm(mPoweroffAlarms.get(0));
                }
            }
            type = RTC_WAKEUP;

        }
        return true;
        ///@}
    }

    @Override
    protected void updatePoweroffAlarmtoNowRtc(){
        final long nowRTC = System.currentTimeMillis();
        updatePoweroffAlarm(nowRTC);
    }

    /**
    *For PowerOffalarm feature, to update Poweroff Alarm
    */
    private void updatePoweroffAlarm(long nowRTC) {
        synchronized (mPowerOffAlarmLock) {
            if (mPoweroffAlarms.size() == 0) {
                return;
            }

            if (mPoweroffAlarms.get(0).when > nowRTC) {
                return;
            }

            Iterator<Alarm> it = mPoweroffAlarms.iterator();

            while (it.hasNext())
            {
                Alarm alarm = it.next();

                if (alarm.when > nowRTC) {
                    // don't fire alarms in the future
                    break;
                }
                Slog.w(TAG, "power off alarm update deleted");
                // remove the alarm from the list
                it.remove();
            }

            if (mPoweroffAlarms.size() > 0) {
                resetPoweroffAlarm(mPoweroffAlarms.get(0));
            }
        }
    }

    private int addPoweroffAlarmLocked(Alarm alarm) {
        ArrayList<Alarm> alarmList = mPoweroffAlarms;

        int index = Collections.binarySearch(alarmList, alarm, sIncreasingTimeOrder);
        if (index < 0) {
            index = 0 - index - 1;
        }
        if (localLOGV) Slog.v(TAG, "Adding alarm " + alarm + " at " + index);
        alarmList.add(index, alarm);

        if (localLOGV) {
            // Display the list of alarms for this alarm type
            Slog.v(TAG, "alarms: " + alarmList.size() + " type: " + alarm.type);
            int position = 0;
            for (Alarm a : alarmList) {
                Time time = new Time();
                time.set(a.when);
                String timeStr = time.format("%b %d %I:%M:%S %p");
                Slog.v(TAG, position + ": " + timeStr
                        + " " + a.operation.getTargetPackage());
                position += 1;
            }
        }

        return index;
    }

    private void removePoweroffAlarmLocked(String packageName) {
        ArrayList<Alarm> alarmList = mPoweroffAlarms;
        if (alarmList.size() <= 0) {
            return;
        }

        // iterator over the list removing any it where the intent match
        Iterator<Alarm> it = alarmList.iterator();

        while (it.hasNext()) {
            Alarm alarm = it.next();
            if (alarm.operation.getTargetPackage().equals(packageName)) {
                it.remove();
            }
        }
    }

    /**
     *For PowerOffalarm feature, this function is used for AlarmManagerService
     * to set the latest alarm registered
     */
    private void resetPoweroffAlarm(Alarm alarm) {

        String setPackageName = alarm.operation.getTargetPackage();
        long latestTime = alarm.when;

        // Only DeskClock is allowed to set this alarm
        if (mNativeData != 0 && mNativeData != -1) {
            if (setPackageName.equals("com.android.deskclock")) {
                set(mNativeData, PRE_SCHEDULE_POWER_OFF_ALARM,
                    latestTime / 1000, (latestTime % 1000) * 1000 * 1000);
            /// M: For Poweroff alarm support for Aging app
            } else if(setPackageName.equals("com.mediatek.sqa8.aging")) {
                set(mNativeData, 7, latestTime / 1000, (latestTime % 1000) * 1000 * 1000);
            ///@}
            } else {
                Slog.w(TAG, "unknown package (" + setPackageName + ") to set power off alarm");
            }
            Slog.i(TAG, "reset power off alarm is " + setPackageName);
        } else {
            Slog.i(TAG, " do not set alarm to RTC when fd close ");
        }
    }
    /**
     * For PowerOffalarm feature, this function is used for APP to
     * cancelPoweroffAlarm
     */
    @Override
    public void cancelPoweroffAlarmImpl(String name) {
        Slog.i(TAG, "remove power off alarm pacakge name " + name);
        // not need synchronized
        synchronized (mPowerOffAlarmLock) {
            removePoweroffAlarmLocked(name);
            // AlarmPair tempAlarmPair = mPoweroffAlarms.remove(name);
            // it will always to cancel the alarm in alarm driver
            if (mNativeData != 0 && mNativeData != -1) {
                if (name.equals("com.android.deskclock")) {
                    set(mNativeData, PRE_SCHEDULE_POWER_OFF_ALARM, 0, 0);
                }
            }
            if (mPoweroffAlarms.size() > 0) {
                resetPoweroffAlarm(mPoweroffAlarms.get(0));
            }
        }
    }
    ///@}
}