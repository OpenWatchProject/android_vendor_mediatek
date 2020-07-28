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
package com.mediatek.server.anr;

import android.app.IActivityController;
import android.app.IApplicationThread;
import android.content.pm.ApplicationInfo;
import android.os.Build;
import android.os.FileUtils;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.StrictMode;
import android.os.SELinux;
import android.server.ServerProtoEnums;
import android.util.EventLog;
import android.util.Slog;
import android.util.SparseArray;
import android.util.StatsLog;

import com.android.internal.os.ProcessCpuTracker;
import com.android.server.am.ActivityManagerService;
import com.android.server.wm.ActivityRecord;
import com.android.server.am.AppNotRespondingDialog;
import com.android.server.am.AppNotRespondingDialog.Data;
import com.android.server.am.ProcessRecord;
import com.android.server.am.ProcessList;
import com.android.server.Watchdog;

import com.mediatek.aee.ExceptionLog;
import com.mediatek.anr.AnrAppManagerImpl;
import com.mediatek.anr.AnrManagerNative;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicLong;
import java.util.Date;
import java.util.Formatter;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Be used for ANR debug mechanism.
 *
 * @hide
 */
public final class AnrManagerService extends AnrManagerNative {
    private static final String TAG = "AnrManager";
    private static boolean sEnhanceEnable = true;

    private static final String PROCESS_RECORD = "com.android.server.am.ProcessRecord";
    private static final String ACTIVITY_RECORD = "com.android.server.am.ActivityRecord";
    private static final String ACTIVITY_MANAGER = "com.android.server.am.ActivityManagerService";
    private static final String APP_ERRORS = "com.android.server.am.AppErrors";
    private static final String ACTIVE_SERVICES = "com.android.server.am.ActiveServices";
    private static final String BATTERY_STATS = "com.android.server.am.BatteryStatsService";
    private static final String ATM_SERVICE = "com.android.server.wm.ActivityTaskManagerService";
    private static final String PROCESS_LIST = "com.android.server.am.ProcessList";

    private static final int START_MONITOR_BROADCAST_TIMEOUT_MSG = 1001;
    private static final int START_MONITOR_SERVICE_TIMEOUT_MSG = 1002;
    private static final int START_ANR_DUMP_MSG = 1003;
    private static final int START_MONITOR_KEYDISPATCHING_TIMEOUT_MSG = 1004;
    private static final int REMOVE_KEYDISPATCHING_TIMEOUT_MSG = 1005;
    private static final int SERVICE_TIMEOUT = 20 * 1000;

    private static final boolean IS_USER_BUILD = "user".equals(Build.TYPE)
            || "userdebug".equals(Build.TYPE);

    private static final boolean IS_USER_LOAD = "user".equals(Build.TYPE);
    private static int[] mZygotePids = null;

    private static final int MAX_MTK_TRACE_COUNT = 10;
    private static final Object mDumpStackTraces = new Object();
    private static String[] NATIVE_STACKS_OF_INTEREST = new String[] {
            "/system/bin/netd", "/system/bin/audioserver",
            "/system/bin/cameraserver", "/system/bin/drmserver",
            "/system/bin/mediadrmserver", "/system/bin/mediaserver",
            "/system/bin/sdcard", "/system/bin/surfaceflinger",
            "vendor/bin/hw/camerahalserver",
            "media.extractor", // system/bin/mediaextractor
            "media.codec", // vendor/bin/hw/android.hardware.media.omx@1.0-service
            "com.android.bluetooth", // Bluetooth service
    };

    private static final ProcessCpuTracker mAnrProcessStats = new ProcessCpuTracker(
            false);
    private final AtomicLong mLastCpuUpdateTime = new AtomicLong(0);
    private static final long MONITOR_CPU_MIN_TIME = 2500;

    private static ConcurrentHashMap<Integer, String> mMessageMap =
            new ConcurrentHashMap<Integer, String>();
    private static final int EVENT_BOOT_COMPLETED = 9001;

    private static final int MESSAGE_MAP_BUFFER_COUNT_MAX = 5;
    private static final int MESSAGE_MAP_BUFFER_SIZE_MAX = 50 * 1000;

    private static final long ANR_BOOT_DEFER_TIME = 30 * 1000;
    private static final long ANR_CPU_DEFER_TIME = 8 * 1000;
    private static final float ANR_CPU_THRESHOLD = 90.0F;

    private static final int INVALID_ANR_FLOW = -1;
    private static final int NORMAL_ANR_FLOW = 0;
    private static final int SKIP_ANR_FLOW = 1;
    private static final int SKIP_ANR_FLOW_AND_KILL = 2;

    private static final int ENABLE_ANR_DUMP_FOR_3RD_APP = 1;
    private static final int DISABLE_ANR_DUMP_FOR_3RD_APP = 0;

    private static final int INVALID_ANR_OPTION = -1;
    private static final int DISABLE_ALL_ANR_MECHANISM = 0;
    private static final int DISABLE_PARTIAL_ANR_MECHANISM = 1;
    private static final int ENABLE_ALL_ANR_MECHANISM = 2;

    private static Object lock = new Object();
    private static AnrManagerService sInstance = null;
    private AnrMonitorHandler mAnrHandler;
    private AnrDumpManager mAnrDumpManager;
    private ActivityManagerService mService;
    private int mAmsPid;
    private long mEventBootCompleted = 0;
    private long mCpuDeferred = 0;
    private int mAnrFlow = INVALID_ANR_FLOW;
    private int mAnrOption = INVALID_ANR_OPTION;
    private ExceptionLog exceptionLog = null;
    private File mTracesFile = null;

    private Class<?> mProcessRecord = getProcessRecord();
    private Class<?> mAMS = getActivityManagerService();

    private Method mKill = getProcessRecordMethod("kill", new Class[] {
            String.class, boolean.class });
    private Method mUpdateCpuStatsNow = getAMSMethod("updateCpuStatsNow");
    private Method mNoteProcessANR = getBatteryStatsServiceMethod(
            "noteProcessAnr", new Class[] { String.class, int.class });
    private Method mScheduleServiceTimeoutLocked = getActiveServicesMethod(
            "scheduleServiceTimeoutLocked", new Class[] { ProcessRecord.class });
    private Method mMakeAppNotRespondingLocked = getProcessRecordMethod(
            "makeAppNotRespondingLocked", new Class[] { String.class,
                    String.class, String.class });

    private Field mPidField = getProcessRecordField("pid");
    private Field mProcessNameField = getProcessRecordField("processName");
    private Field mThreadField = getProcessRecordField("thread");
    private Field mNotRespondingField = getProcessRecordField("notResponding");
    private Field mCrashingField = getProcessRecordField("crashing");
    private Field mUserIdField = getProcessRecordField("userId");
    private Field mUidField = getProcessRecordField("uid");
    private Field mInfoField = getProcessRecordField("info");
    private Field mPersistentField = getProcessRecordField("mPersistent");
    private Field mParentPidField = getProcessRecordField("pid");

    private Field mParentAppField = getActivityRecordField("app");

    //private Field mControllerField = getAMSField("mController");
    //private Field mLruProcessesField = getAMSField("mLruProcesses");
    private Field mLruProcessesField = getPLField("mLruProcesses");
    private Field mProcessListField = getAMSField("mProcessList");
    private Field mProcessCpuTrackerField = getAMSField("mProcessCpuTracker");
    private Field mMonitorCpuUsageField = getAMSField("MONITOR_CPU_USAGE");
    private Field mShowNotRespondingUiMsgField = getAMSField("SHOW_NOT_RESPONDING_UI_MSG");
    private Field mBatteryStatsServiceField = getAMSField("mBatteryStatsService");
    private Field mActiveServicesField = getAMSField("mServices");
    private Field mUiHandlerField = getAMSField("mUiHandler");

    private Field mControllerField = getATMField("mController");
    private Class<?> getProcessRecord() {
        try {
            return Class.forName(PROCESS_RECORD);
        } catch (Exception e) {
            return null;
        }
    }

    private Class<?> getActivityManagerService() {
        try {
            return Class.forName(ACTIVITY_MANAGER);
        } catch (Exception e) {
            return null;
        }
    }

    private Method getProcessRecordMethod(String func, Class[] cls) {
        try {
            Method method = mProcessRecord.getDeclaredMethod(func, cls);
            method.setAccessible(true);
            return method;
        } catch (Exception e) {
            Slog.w(TAG, "getProcessRecordMethod Exception: " + e);
            return null;
        }
    }

    private Method getAMSMethod(String func) {
        try {
            Method method = mAMS.getDeclaredMethod(func);
            method.setAccessible(true);
            return method;
        } catch (Exception e) {
            return null;
        }
    }

    private Method getBatteryStatsServiceMethod(String func, Class[] cls) {
        try {
            Class<?> batteryStatsService = Class.forName(BATTERY_STATS);
            Method method = batteryStatsService.getDeclaredMethod(func, cls);
            method.setAccessible(true);
            return method;
        } catch (Exception e) {
            return null;
        }
    }

    private Method getActiveServicesMethod(String func, Class[] cls) {
        try {
            Class<?> activeServices = Class.forName(ACTIVE_SERVICES);
            return activeServices.getDeclaredMethod(func, cls);
        } catch (Exception e) {
            return null;
        }
    }

    private Method getAppErrorsMethod(String func, Class[] cls) {
        try {
            Class<?> appErrors = Class.forName(APP_ERRORS);
            Method method = appErrors.getDeclaredMethod(func, cls);
            method.setAccessible(true);
            return method;
        } catch (Exception e) {
            return null;
        }
    }

    private Field getProcessRecordField(String var) {
        try {
            Field field = mProcessRecord.getDeclaredField(var);
            field.setAccessible(true);
            return field;
        } catch (Exception e) {
            return null;
        }
    }

    private Field getActivityRecordField(String var) {
        try {
            Class<?> mActivityRecord = Class.forName(ACTIVITY_RECORD);
            Field field = mActivityRecord.getDeclaredField(var);
            field.setAccessible(true);
            return field;
        } catch (Exception e) {
            return null;
        }
    }

    private Field getAMSField(String var) {
        try {
            Field field = mAMS.getDeclaredField(var);
            field.setAccessible(true);
            return field;
        } catch (Exception e) {
            return null;
        }
    }

    private Field getATMField(String var) {
        try {
            Class<?> mATM = Class.forName(ATM_SERVICE);
            Field field = mATM.getDeclaredField(var);
            field.setAccessible(true);
            return field;
        } catch (Exception e) {
            return null;
        }
    }

    private Field getPLField(String var) {
        try {
            Class<?> mATM = Class.forName(PROCESS_LIST);
            Field field = mATM.getDeclaredField(var);
            field.setAccessible(true);
            return field;
        } catch (Exception e) {
            return null;
        }
    }

    public static AnrManagerService getInstance() {
        if (null == sInstance) {
            synchronized (lock) {
                if (null == sInstance) {
                    sInstance = new AnrManagerService();
                }
            }
        }
        return sInstance;
    }

    // If you need to initial something, please put it here.
    public void startAnrManagerService(int pid) {
        Slog.i(TAG, "startAnrManagerService");
        mAmsPid = pid;
        HandlerThread handlerThread = new HandlerThread("AnrMonitorThread");
        handlerThread.start();
        mAnrHandler = new AnrMonitorHandler(handlerThread.getLooper());
        mAnrDumpManager = new AnrDumpManager();
        mAnrProcessStats.init();
        //prepareStackTraceFile(SystemProperties.get(
        //        "dalvik.vm.mtk-stack-trace-file", null));
        prepareStackTraceFile(SystemProperties.get(
                "dalvik.vm.stack-trace-file", null));
        File traceFile = new File(SystemProperties.get(
                "dalvik.vm.stack-trace-file", null));
        File traceDir = traceFile.getParentFile();
        if (traceDir != null && !SELinux.restoreconRecursive(traceDir)) {
            Slog.i(TAG,
                    "startAnrManagerService SELinux.restoreconRecursive fail dir = "
                            + traceDir.toString());
        }
        if (SystemProperties.get("ro.vendor.have_aee_feature").equals("1")) {
            exceptionLog = ExceptionLog.getInstance();
        }
//        if (!IS_USER_BUILD) {
//            Looper.myLooper().setMessageLogging(
//                    AnrAppManagerImpl.getInstance().newMessageLogger(false,
//                            Thread.currentThread().getName()));
//        }
        mKill.setAccessible(true);
        mUpdateCpuStatsNow.setAccessible(true);
        mNoteProcessANR.setAccessible(true);
        mScheduleServiceTimeoutLocked.setAccessible(true);
        mMakeAppNotRespondingLocked.setAccessible(true);
    }

    public void sendBroadcastMonitorMessage(long timeoutTime,
            long mTimeoutPeriod) {
        if (ENABLE_ALL_ANR_MECHANISM == checkAnrDebugMechanism()) {
            Message broadcastMonitor = mAnrHandler
                    .obtainMessage(START_MONITOR_BROADCAST_TIMEOUT_MSG);
            mAnrHandler.sendMessageAtTime(broadcastMonitor, timeoutTime
                    - mTimeoutPeriod / 2);
        }
    }

    public void removeBroadcastMonitorMessage() {
        if (ENABLE_ALL_ANR_MECHANISM == checkAnrDebugMechanism()) {
            mAnrHandler.removeMessages(START_MONITOR_BROADCAST_TIMEOUT_MSG);
        }
    }

    public void sendServiceMonitorMessage() {
        long now = SystemClock.uptimeMillis();
        if (ENABLE_ALL_ANR_MECHANISM == checkAnrDebugMechanism()) {
            Message serviceMonitor = mAnrHandler
                    .obtainMessage(START_MONITOR_SERVICE_TIMEOUT_MSG);
            mAnrHandler.sendMessageAtTime(serviceMonitor, now + SERVICE_TIMEOUT
                    * 2 / 3);
        }
    }

    public void removeServiceMonitorMessage() {
        if (ENABLE_ALL_ANR_MECHANISM == checkAnrDebugMechanism()) {
            mAnrHandler.removeMessages(START_MONITOR_SERVICE_TIMEOUT_MSG);
        }
    }

    public boolean startAnrDump(ActivityManagerService service,
            ProcessRecord app, String activityShortComponentName, ApplicationInfo aInfo,
            String parentShortComponentName, ProcessRecord parentProcess, boolean aboveSystem,
            String annotation, boolean showBackground, long anrTime)
            throws Exception {
        Slog.i(TAG, "startAnrDump");
        if (DISABLE_ALL_ANR_MECHANISM == checkAnrDebugMechanism()) {
            return false;
        }

        mService = service;
        boolean isSilentANR = false;
        int pid = (int) mPidField.get(app);
        String processName = (String) mProcessNameField.get(app);
        ApplicationInfo appInfo = (ApplicationInfo) mInfoField.get(app);
        IActivityController controller = (IActivityController) mControllerField
                .get(mService.mActivityTaskManager);

        int parentPid = -1;
        if (null != parentProcess) {
            parentPid = (int) mParentPidField.get(parentProcess);
        }

        if (isAnrFlowSkipped(pid, processName, annotation)) {
            return true;
        }

        if (!IS_USER_LOAD) {
            try {
                ((IApplicationThread) mThreadField.get(app))
                        .dumpMessage(pid == mAmsPid);
            } catch (Exception e) {
                Slog.e(TAG, "Error happens when dumping message history", e);
            }
        }
        AnrDumpRecord anrDumpRecord = null;

        synchronized (mService) {
            // Don't dump other PIDs if it's a background ANR
            isSilentANR = !showBackground && !app.isInterestingToUserLocked()
                    && pid != mAmsPid;
        }

        if (needAnrDump(appInfo)) {
            enableTraceLog(false);
            new BinderDumpThread(pid).start();
            if (!mAnrDumpManager.mDumpList.containsKey(app)) {
                anrDumpRecord = new AnrDumpRecord(app != null ? pid : -1,
                        false,
                        app != null ? processName : null,
                        app != null ? app.toString() : null,
                        activityShortComponentName,
                        parentProcess != null ? parentPid : -1,
                        parentShortComponentName,
                        annotation, anrTime);
                if (ENABLE_ALL_ANR_MECHANISM == checkAnrDebugMechanism()) {
                    updateProcessStats();
                    String cpuInfo = getAndroidTime() + getProcessState() + "\n";
                    anrDumpRecord.mCpuInfo = cpuInfo;
                    Slog.i(TAG, cpuInfo.toString());
                }
                mAnrDumpManager.startAsyncDump(anrDumpRecord, isSilentANR);
            }

            if (anrDumpRecord != null) {
                synchronized (anrDumpRecord) {
                    mAnrDumpManager.dumpAnrDebugInfo(anrDumpRecord, false, isSilentANR);
                }
            }
            mAnrDumpManager.removeDumpRecord(anrDumpRecord);

            anrDumpRecord.mCpuInfo = anrDumpRecord.mCpuInfo + mMessageMap.get(pid);
        }
        StatsLog.write(StatsLog.ANR_OCCURRED, (int) mUidField.get(app), processName,
                activityShortComponentName, annotation,
                (appInfo != null) ? (appInfo.isInstantApp()
                        ? StatsLog.ANROCCURRED__IS_INSTANT_APP__TRUE
                        : StatsLog.ANROCCURRED__IS_INSTANT_APP__FALSE)
                        : StatsLog.ANROCCURRED__IS_INSTANT_APP__UNAVAILABLE,
                (app != null) ? (app.isInterestingToUserLocked()
                        ? StatsLog.ANROCCURRED__FOREGROUND_STATE__FOREGROUND
                        : StatsLog.ANROCCURRED__FOREGROUND_STATE__BACKGROUND)
                        : StatsLog.ANROCCURRED__FOREGROUND_STATE__UNKNOWN,
                (app != null) ? app.getProcessClassEnum() : ServerProtoEnums.ERROR_SOURCE_UNKNOWN,
                (appInfo != null) ? appInfo.packageName : "");
        Slog.i(TAG, "addErrorToDropBox app = " + app + " processName = " + processName
                + " activityShortComponentName = "+ activityShortComponentName
                + " parentShortComponentName = " + parentShortComponentName + " parentProcess = "
                + parentProcess + " annotation = " + annotation + " mTracesFile = " + mTracesFile);
        mService.addErrorToDropBox("anr", app, processName, activityShortComponentName,
                parentShortComponentName, parentProcess, annotation,
                anrDumpRecord != null ? anrDumpRecord.mCpuInfo : "", mTracesFile, null);

        Slog.i(TAG, " controller = " + controller);
        if (controller != null) {
            try {
                // 0 == show dialog, 1 = keep waiting, -1 = kill process
                // immediately
                int res = controller.appNotResponding(processName, pid,
                        anrDumpRecord != null ? anrDumpRecord.mInfo.toString()
                                : "");
                Slog.i(TAG, " res = " + res);
                if (res != 0) {
                    if (res < 0 && pid != mAmsPid) {
                        mKill.invoke(app, new Object[] { "anr", true });
                    } else {
                        synchronized (mService) {
                            mScheduleServiceTimeoutLocked.invoke(
                                    mActiveServicesField.get(mService),
                                    new Object[] { app });
                        }
                    }
                    return true;
                }
            } catch (RemoteException e) {
                mControllerField.set(mService.mActivityTaskManager, null);
                Watchdog.getInstance().setActivityController(null);
            }
        }

        synchronized (mService) {
            mNoteProcessANR.invoke(mBatteryStatsServiceField.get(mService),
                    new Object[] { processName, (int) mUidField.get(app) });

            if (isSilentANR) {
                mKill.invoke(app, new Object[] { "bg anr", true });
                return true;
            }

            // Set the app's notResponding state, and look up the
            // errorReportReceiver
            mMakeAppNotRespondingLocked.invoke(
                    app,
                    new Object[] {
                            activityShortComponentName,
                            annotation != null ? "ANR " + annotation : "ANR",
                            anrDumpRecord != null ? anrDumpRecord.mInfo
                                    .toString() : "" });
            // Bring up the infamous App Not Responding dialog
            Message msg = Message.obtain();
            msg.what = (int) mShowNotRespondingUiMsgField.get(mAMS);
            msg.obj = new AppNotRespondingDialog.Data(app, aInfo, aboveSystem);

            Handler mUiHandler = (Handler) mUiHandlerField.get(mService);
            mUiHandler.sendMessage(msg);
        }
        return true;
    }

    public class AnrMonitorHandler extends Handler {
        public AnrMonitorHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case START_MONITOR_BROADCAST_TIMEOUT_MSG:
            case START_MONITOR_SERVICE_TIMEOUT_MSG:
            case START_MONITOR_KEYDISPATCHING_TIMEOUT_MSG:
                updateProcessStats();
                break;
            case START_ANR_DUMP_MSG:
                AnrDumpRecord adp = (AnrDumpRecord) msg.obj;
                boolean isSilentANR = msg.arg1 == 1;
                Slog.i(TAG, "START_ANR_DUMP_MSG: " + adp + ", isSilentANR = " + isSilentANR);
                mAnrDumpManager.dumpAnrDebugInfo(adp, true, isSilentANR);
                break;
            default:
                break;
            }
        }
    };

    protected static final class BinderWatchdog {
        private static final int MAX_TIMEOUT_PIDS = 5;
        private static final int MAX_LINES = 64;

        protected static class BinderInfo {
            /*
             * Node format Example: 2:execution 658:729 to 670:684 spends 4000
             * ms <> dex_code 4 start_at 71.726 2012-05-15 05:31:07.989
             */
            protected static final int INDEX_FROM = 1;
            protected static final int INDEX_TO = 3;

            protected int mSrcPid;
            protected int mSrcTid;
            protected int mDstPid;
            protected int mDstTid;
            protected String mText;

            protected BinderInfo(String text) {
                if (text == null || text.length() <= 0) {
                    return;
                }

                mText = new String(text);
                String[] tokens = text.split(" ");
                String[] from = tokens[INDEX_FROM].split(":");
                if (from != null && from.length == 2) {
                    mSrcPid = Integer.parseInt(from[0]);
                    mSrcTid = Integer.parseInt(from[1]);
                }

                String[] to = tokens[INDEX_TO].split(":");
                if (to != null && to.length == 2) {
                    mDstPid = Integer.parseInt(to[0]);
                    mDstTid = Integer.parseInt(to[1]);
                }
            }
        }

        public static final ArrayList<Integer> getTimeoutBinderPidList(int pid,
                int tid) {
            if (pid <= 0) {
                return null;
            }

            ArrayList<BinderInfo> binderList = readTimeoutBinderListFromFile();
            BinderInfo next = getBinderInfo(pid, tid, binderList);

            int count = 0;
            ArrayList<Integer> pidList = new ArrayList<Integer>();
            while (next != null) {
                if (next.mDstPid > 0) {
                    count++;
                    if (!pidList.contains(next.mDstPid)) {
                        Slog.i(TAG, "getTimeoutBinderPidList pid added: "
                                + next.mDstPid + " " + next.mText);
                        pidList.add(next.mDstPid);
                    } else {
                        Slog.i(TAG, "getTimeoutBinderPidList pid existed: "
                                + next.mDstPid + " " + next.mText);
                    }

                    if (count >= MAX_TIMEOUT_PIDS) {
                        break;
                    }
                }
                next = getBinderInfo(next.mDstPid, next.mDstTid, binderList);
            }

            if (pidList == null || pidList.size() == 0) {
                return getTimeoutBinderFromPid(pid, binderList);
            }
            return pidList;
        }

        public static final ArrayList<Integer> getTimeoutBinderFromPid(int pid,
                ArrayList<BinderInfo> binderList) {
            if (pid <= 0 || binderList == null) {
                return null;
            }

            Slog.i(TAG, "getTimeoutBinderFromPid " + pid + " list size: "
                    + binderList.size());
            int count = 0;
            ArrayList<Integer> pidList = new ArrayList<Integer>();
            for (BinderInfo bi : binderList) {
                if (bi != null && bi.mSrcPid == pid) {
                    count++;
                    if (!pidList.contains(bi.mDstPid)) {
                        Slog.i(TAG, "getTimeoutBinderFromPid pid added: "
                                + bi.mDstPid + " " + bi.mText);
                        pidList.add(bi.mDstPid);
                    } else {
                        Slog.i(TAG, "getTimeoutBinderFromPid pid existed: "
                                + bi.mDstPid + " " + bi.mText);
                    }

                    if (count >= MAX_TIMEOUT_PIDS) {
                        break;
                    }
                }
            }
            return pidList;
        }

        private static BinderInfo getBinderInfo(int pid, int tid,
                ArrayList<BinderInfo> binderList) {
            if (binderList == null || binderList.size() == 0 || pid == 0) {
                return null;
            }
            int size = binderList.size();
            for (BinderInfo bi : binderList) {
                if (bi.mSrcPid == pid && bi.mSrcTid == tid) {
                    Slog.i(TAG, "Timeout binder pid found: " + bi.mDstPid + " "
                            + bi.mText);
                    return bi;
                }
            }
            return null;
        }

        private static final ArrayList<BinderInfo> readTimeoutBinderListFromFile() {
            BufferedReader br = null;
            ArrayList<BinderInfo> binderList = null;
            try {
                File file = new File("/sys/kernel/debug/binder/timeout_log");
                if (file == null || !file.exists()) {
                    return null;
                }
                br = new BufferedReader(new FileReader(file));
                String line;
                binderList = new ArrayList<BinderInfo>();
                while ((line = br.readLine()) != null) {
                    BinderInfo bi = new BinderInfo(line);
                    if (bi != null && bi.mSrcPid > 0) {
                        binderList.add(bi);
                    }
                    if (binderList.size() > MAX_LINES) {
                        break;
                    }
                }
            } catch (FileNotFoundException e) {
                Slog.e(TAG, "FileNotFoundException", e);
            } catch (IOException e) {
                Slog.e(TAG, "IOException when gettting Binder. ", e);
            } finally {
                if (br != null) {
                    try {
                        br.close();
                    } catch (IOException ioe) {
                        Slog.e(TAG, "IOException when close buffer reader:",
                                ioe);
                    }
                }
                return binderList;
            }
        }

        protected static class TransactionInfo {
            protected String direction;
            protected String snd_pid;
            protected String snd_tid;
            protected String rcv_pid;
            protected String rcv_tid;
            protected String ktime;
            protected String atime;
            protected long spent_time;

            protected TransactionInfo() {
            };
        }

        private static final void readTransactionInfoFromFile(int pid,
                ArrayList<Integer> binderList) {
            String patternStr = "(\\S+.+transaction).+from\\s+(\\d+):(\\d+)\\s+to\\s+(\\d+):"
                    + "(\\d+).+start\\s+(\\d+\\.+\\d+).+android\\s+(\\d+-\\d+-\\d+\\s+\\d+:\\d+:\\d"
                    + "+\\.\\d+)";
            Pattern pattern = Pattern.compile(patternStr);

            BufferedReader br = null;
            ArrayList<TransactionInfo> transactionList = new ArrayList<TransactionInfo>();
            ArrayList<Integer> pidList = new ArrayList<Integer>();
            try {
                String filepath = "/sys/kernel/debug/binder/proc/"
                        + Integer.toString(pid);
                File file = new File(filepath);
                if (file == null || !file.exists()) {
                    Slog.d(TAG, "Filepath isn't exist");
                    return;
                }

                br = new BufferedReader(new FileReader(file));
                String line;
                Matcher matcher;
                while ((line = br.readLine()) != null) {
                    if (line.contains("transaction")) {
                        matcher = pattern.matcher(line);
                        if (matcher.find()) {
                            TransactionInfo tmpInfo = new TransactionInfo();
                            tmpInfo.direction = matcher.group(1);
                            tmpInfo.snd_pid = matcher.group(2);
                            tmpInfo.snd_tid = matcher.group(3);
                            tmpInfo.rcv_pid = matcher.group(4);
                            tmpInfo.rcv_tid = matcher.group(5);
                            tmpInfo.ktime = matcher.group(6);
                            tmpInfo.atime = matcher.group(7);
                            tmpInfo.spent_time = SystemClock.uptimeMillis()
                                    - (long) (Float.valueOf(tmpInfo.ktime) * 1000);
                            transactionList.add(tmpInfo);
                            if (tmpInfo.spent_time >= 1000) {
                                if (!binderList.contains(Integer
                                        .valueOf(tmpInfo.rcv_pid))) {
                                    binderList.add(Integer
                                            .valueOf(tmpInfo.rcv_pid));
                                    if (!pidList.contains(Integer
                                            .valueOf(tmpInfo.rcv_pid))) {
                                        pidList.add(Integer
                                                .valueOf(tmpInfo.rcv_pid));
                                        Slog.i(TAG,
                                                "Transcation binderList pid="
                                                        + tmpInfo.rcv_pid);
                                    }
                                }
                            }
                            Slog.i(TAG, tmpInfo.direction + " from "
                                    + tmpInfo.snd_pid + ":" + tmpInfo.snd_tid
                                    + " to " + tmpInfo.rcv_pid + ":"
                                    + tmpInfo.rcv_tid + " start "
                                    + tmpInfo.ktime + " android time "
                                    + tmpInfo.atime + " spent time "
                                    + tmpInfo.spent_time + " ms");
                        }
                    } else {
                        if (line.indexOf("node") != -1
                                && line.indexOf("node") < 20) {
                            break;
                        }
                    }
                }

                for (int pidnumber : pidList) {
                    readTransactionInfoFromFile(pidnumber, binderList);
                }

            } catch (FileNotFoundException e) {
                Slog.e(TAG, "FileNotFoundException", e);
            } catch (IOException e) {
                Slog.e(TAG, "IOException when gettting Binder. ", e);
            } finally {
                if (br != null) {
                    try {
                        br.close();
                    } catch (IOException ioe) {
                        Slog.e(TAG, "IOException when close buffer reader:",
                                ioe);
                    }
                }
            }
        }

        private static final void setTransactionTimeoutPids(int pid,
                ArrayList<Integer> desList, SparseArray<Boolean> lastPids) {
            ArrayList<Integer> tmpPidList = new ArrayList<Integer>();
            BinderWatchdog.readTransactionInfoFromFile(pid, tmpPidList);
            if (tmpPidList != null && tmpPidList.size() > 0) {
                for (Integer bpid : tmpPidList) {
                    if (bpid != null) {
                        int pidValue = bpid.intValue();
                        if (pidValue != pid) {
                            if (!desList.contains(pidValue)) {
                                desList.add(pidValue);
                                if (lastPids != null) {
                                    lastPids.remove(pidValue);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    public void prepareStackTraceFile(String filePath) {
        Slog.i(TAG, "prepareStackTraceFile: " + filePath);
        if (filePath == null || filePath.length() == 0) {
            return;
        }

        File traceFile = new File(filePath);
        try {
            // Create folder if not already existed.
            File traceDir = traceFile.getParentFile();
            if (traceDir != null) {
                if (!traceDir.exists()) {
                    traceDir.mkdirs();
                }
                // Create folder and set needed permission - drwxrwxr-x.
                FileUtils.setPermissions(traceDir.getPath(), 0775, -1, -1);
            }
            // Create file and set needed permission - -rw-rw-rw-.
            if (!traceFile.exists()) {
                traceFile.createNewFile();
            }
            FileUtils.setPermissions(traceFile.getPath(), 0666, -1, -1);
        } catch (IOException e) {
            Slog.e(TAG, "Unable to prepare stack trace file: " + filePath, e);
        }
    }

    public class AnrDumpRecord {
        protected int mAppPid;
        protected boolean mAppCrashing;
        protected String mProcessName;
        protected String mAppString;
        protected String mShortComponentName;
        protected int mParentAppPid;
        protected String mParentShortComponentName;
        protected String mAnnotation;
        protected long mAnrTime;
        public String mCpuInfo = null;
        public StringBuilder mInfo = new StringBuilder(256);
        protected boolean mIsCompleted;
        protected boolean mIsCancelled;

        public AnrDumpRecord(int appPid, boolean appCrashing,
                String processName, String appString,
                String shortComponentName, int parentAppPid,
                String parentShortComponentName, String annotation, long anrTime) {
            mAppPid = appPid;
            mAppCrashing = appCrashing;
            mProcessName = processName;
            mAppString = appString;
            mShortComponentName = shortComponentName;
            mParentAppPid = parentAppPid;
            mParentShortComponentName = parentShortComponentName;
            mAnnotation = annotation;
            mAnrTime = anrTime;
        }

        private boolean isValid() {
            if (mAppPid <= 0 || mIsCancelled || mIsCompleted) {
                Slog.e(TAG, "isValid! mAppPid: " + mAppPid + "mIsCancelled: "
                        + mIsCancelled + "mIsCompleted: " + mIsCompleted);
                return false;
            } else {
                return true;
            }
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("AnrDumpRecord{ ");
            sb.append(mAnnotation);
            sb.append(" ");
            sb.append(mAppString);
            sb.append(" IsCompleted:" + mIsCompleted);
            sb.append(" IsCancelled:" + mIsCancelled);
            sb.append(" }");
            return sb.toString();
        }
    }

    public class AnrDumpManager {
        public HashMap<Integer, AnrDumpRecord> mDumpList = new HashMap<Integer, AnrDumpRecord>();

        public void cancelDump(AnrDumpRecord dumpRecord) {
            if (dumpRecord == null || dumpRecord.mAppPid == -1) {
                return;
            }

            synchronized (mDumpList) {
                AnrDumpRecord value = mDumpList.remove(dumpRecord.mAppPid);
                if (value != null) {
                    value.mIsCancelled = true;
                }
            }
        }

        public void removeDumpRecord(AnrDumpRecord dumpRecord) {
            if (dumpRecord == null || dumpRecord.mAppPid == -1) {
                return;
            }

            synchronized (mDumpList) {
                AnrDumpRecord value = mDumpList.remove(dumpRecord.mAppPid);
            }
        }

        public void startAsyncDump(AnrDumpRecord dumpRecord, boolean isSilentANR) {
            Slog.i(TAG, "startAsyncDump: " + dumpRecord + ", isSilentANR = " + isSilentANR);
            if (dumpRecord == null || dumpRecord.mAppPid == -1) {
                return;
            }

            int appPid = dumpRecord.mAppPid;
            synchronized (mDumpList) {
                if (mDumpList.containsKey(appPid)) {
                    return;
                }

                mDumpList.put(appPid, dumpRecord);

                Message msg = mAnrHandler.obtainMessage(START_ANR_DUMP_MSG,
                        dumpRecord);
                msg.arg1 = isSilentANR ? 1 : 0;
                mAnrHandler.sendMessageAtTime(msg,
                        SystemClock.uptimeMillis() + 500);
            }
        }

        private boolean isDumpable(AnrDumpRecord dumpRecord) {
            synchronized (mDumpList) {
                if (dumpRecord != null
                        && mDumpList.containsKey(dumpRecord.mAppPid)
                        && dumpRecord.isValid()) {
                    return true;
                } else {
                    return false;
                }
            }
        }

        public void dumpAnrDebugInfo(AnrDumpRecord dumpRecord,
                boolean isAsyncDump, boolean isSilentANR) {
            Slog.i(TAG, "dumpAnrDebugInfo begin: " + dumpRecord
                    + ", isAsyncDump = " + isAsyncDump + ", isSilentANR = " + isSilentANR);
            if (dumpRecord == null) {
                return;
            }

            try {
                if (!isDumpable(dumpRecord)) {
                    Slog.i(TAG, "dumpAnrDebugInfo dump stopped: " + dumpRecord);
                    return;
                }
                dumpAnrDebugInfoLocked(dumpRecord, isAsyncDump, isSilentANR);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Slog.i(TAG, "dumpAnrDebugInfo end: " + dumpRecord
                    + ", isAsyncDump = " + isAsyncDump + " , isSilentANR = " + isSilentANR);
        }

        protected void dumpAnrDebugInfoLocked(AnrDumpRecord dumpRecord,
                boolean isAsyncDump, boolean isSilentANR) throws Exception {
            synchronized (dumpRecord) {
                Slog.i(TAG, "dumpAnrDebugInfoLocked: " + dumpRecord
                        + ", isAsyncDump = " + isAsyncDump + ", isSilentANR = " + isSilentANR);
                if (!isDumpable(dumpRecord)) {
                    return;
                }

                int appPid = dumpRecord.mAppPid;
                int parentAppPid = dumpRecord.mParentAppPid;
                ArrayList<Integer> firstPids = new ArrayList<Integer>();
                SparseArray<Boolean> lastPids = new SparseArray<Boolean>(20);

                /** ALPS03762884 Avoiding send sig 3 to native process
                ArrayList<Integer> binderPids = null;
                if (appPid != -1) {
                    binderPids = BinderWatchdog.getTimeoutBinderPidList(appPid,
                            appPid);
                }
                **/

                firstPids.add(appPid);

                int parentPid = appPid;
                if (parentAppPid > 0)
                    parentPid = parentAppPid;

                if (!isSilentANR) {
                    if (parentPid != appPid)
                        firstPids.add(parentPid);
                }

                if (mAmsPid != appPid && (isSilentANR || mAmsPid != parentPid)) {
                    firstPids.add(mAmsPid);
                }

                if (!isAsyncDump && !isSilentANR) {
                    synchronized (mService) {
                        ProcessList mProcessList = (ProcessList) mProcessListField.get(mService);
                        ArrayList<ProcessRecord> mLruProcesses
                           = (ArrayList<ProcessRecord>) mLruProcessesField.get(mProcessList);

                        for (int i = mLruProcesses.size() - 1; i >= 0; i--) {
                            ProcessRecord r = mLruProcesses.get(i);
                            if (r != null
                                    && (IApplicationThread) mThreadField.get(r) != null) {
                                int pid = (int) mPidField.get(r);
                                if (pid > 0 && pid != appPid
                                        && pid != parentPid && pid != mAmsPid) {
                                    if ((boolean) mPersistentField.get(r)) {
                                        firstPids.add(pid);
                                    } else {
                                        lastPids.put(pid, Boolean.TRUE);
                                    }
                                }
                            }
                        }
                    }
                }
                /** ALPS03762884 Avoiding send sig 3 to native process
                if (binderPids != null && binderPids.size() > 0) {
                    for (Integer bpid : binderPids) {
                        if (bpid != null) {
                            int pidValue = bpid.intValue();
                            if (pidValue != appPid && pidValue != parentPid
                                    && pidValue != mAmsPid) {
                                if (!firstPids.contains(pidValue)) {
                                    firstPids.add(pidValue);
                                    lastPids.remove(pidValue);
                                }
                            }
                        }
                    }
                }
                **/

                ArrayList<Integer> remotePids = new ArrayList<Integer>();
                if (appPid != -1) {
                    BinderWatchdog.setTransactionTimeoutPids(appPid, remotePids,
                            lastPids);
                }

                String annotation = dumpRecord.mAnnotation;

                // Log the ANR to the main log.
                StringBuilder info = dumpRecord.mInfo;
                info.setLength(0);
                info.append("ANR in ").append(dumpRecord.mProcessName);
                if (dumpRecord.mShortComponentName != null) {
                    info.append(" (").append(dumpRecord.mShortComponentName)
                            .append(")");
                }
                info.append(", time=").append(dumpRecord.mAnrTime);
                info.append("\n");
                if (annotation != null) {
                    info.append("Reason: ").append(annotation).append("\n");
                }
                if (dumpRecord.mParentAppPid != -1
                        && dumpRecord.mParentAppPid != dumpRecord.mAppPid) {
                    info.append("Parent: ")
                            .append(dumpRecord.mParentShortComponentName)
                            .append("\n");
                }

                final ProcessCpuTracker processStats = new ProcessCpuTracker(
                        true);

                if (!isDumpable(dumpRecord)) {
                    return;
                }

                int[] pids = Process
                        .getPidsForCommands(NATIVE_STACKS_OF_INTEREST);
                ArrayList<Integer> nativePids = null;

                if (pids != null) {
                    nativePids = new ArrayList<Integer>(pids.length);
                    for (int i : pids) {
                        nativePids.add(i);
                    }
                }

                for (Integer remotePid : remotePids) {
                    if (isJavaProcess(remotePid)) {
                        if (!firstPids.contains(remotePid)) {
                            firstPids.add(remotePid);
                        }
                    } else {
                        if (nativePids == null) {
                            nativePids = new ArrayList<Integer>();
                        }
                        if (!nativePids.contains(remotePid)) {
                            nativePids.add(remotePid);
                        }
                    }
                }

                Slog.i(TAG, "dumpStackTraces begin!");
                mTracesFile = mService.dumpStackTraces(firstPids, isSilentANR ? null : processStats,
                        isSilentANR ? null : lastPids, nativePids);
                Slog.i(TAG, "dumpStackTraces end!");

                if (!isDumpable(dumpRecord)) {
                    return;
                }

                String cpuInfo = null;
                if ((boolean) mMonitorCpuUsageField.get(mAMS)) {
                    ProcessCpuTracker mProcessCpuTracker =
                            (ProcessCpuTracker) mProcessCpuTrackerField.get(mService);
                    synchronized (mProcessCpuTracker) {
                        cpuInfo = getAndroidTime()
                                + mProcessCpuTracker
                                        .printCurrentState(dumpRecord.mAnrTime);
                        dumpRecord.mCpuInfo = dumpRecord.mCpuInfo + cpuInfo;
                    }
                    mUpdateCpuStatsNow.invoke(mService);
                    info.append(processStats.printCurrentLoad());
                    info.append(cpuInfo);
                }

                Slog.i(TAG, info.toString());

                if (!isDumpable(dumpRecord)) {
                    return;
                }
                if (mTracesFile == null) {
                    // There is no trace file
                    // so dump (only) the alleged culprit's threads to the log
                    Process.sendSignal(appPid, Process.SIGNAL_QUIT);
                }
                dumpRecord.mIsCompleted = true;
            }
        }
    }

    public boolean isJavaProcess(int pid) {
        if (pid <= 0) {
            return false;
        }

        if (mZygotePids == null) {
            String[] commands = new String[] {
               "zygote64",
               "zygote"
            };
            mZygotePids = Process.getPidsForCommands(commands);
        }

        if (mZygotePids != null) {
            int parentPid = Process.getParentPid(pid);
            for (int zygotePid : mZygotePids) {
                if (parentPid == zygotePid) {
                    return true;
                }
            }
        }
        Slog.i(TAG, "pid: " + pid + " is not a Java process");
        return false;
    }

    private Boolean isException() {
        try {
            String status = "free";
            // Two value: "free" and "dumping"
            if (status.equals(SystemProperties.get("vendor.debug.mtk.aee.status",
                    status))
                    && status.equals(SystemProperties.get(
                            "vendor.debug.mtk.aee.status64", status))
                    && status.equals(SystemProperties.get(
                            "vendor.debug.mtk.aee.vstatus", status))
                    && status.equals(SystemProperties.get(
                            "vendor.debug.mtk.aee.vstatus64", status))) {
                return false;
            }
        } catch (Exception e) {
            Slog.e(TAG, "isException: " + e.toString());
        }
        return true;
    }

    public void informMessageDump(String MessageInfo, int pid) {
        if (mMessageMap.containsKey(pid)) {
            String tmpString = mMessageMap.get(pid);
            if (tmpString.length() > MESSAGE_MAP_BUFFER_SIZE_MAX) {
                tmpString = "";
            }
            tmpString = tmpString + MessageInfo;
            mMessageMap.put(pid, tmpString);
        } else {
            if (mMessageMap.size() > MESSAGE_MAP_BUFFER_COUNT_MAX) {
                mMessageMap.clear();
            }
            mMessageMap.put(pid, MessageInfo);
        }
        Slog.i(TAG, "informMessageDump pid= " + pid);
    }

    public int checkAnrDebugMechanism() {
        if (!sEnhanceEnable) {
            return DISABLE_ALL_ANR_MECHANISM;
        }
        if (INVALID_ANR_OPTION == mAnrOption) {
            int option = ENABLE_ALL_ANR_MECHANISM;
            if (IS_USER_BUILD) {
                option = DISABLE_PARTIAL_ANR_MECHANISM;
            }
            mAnrOption = SystemProperties.getInt("persist.vendor.anr.enhancement",
                    option);
        }
        switch (mAnrOption) {
        case ENABLE_ALL_ANR_MECHANISM:
            return ENABLE_ALL_ANR_MECHANISM;
        case DISABLE_PARTIAL_ANR_MECHANISM:
            return DISABLE_PARTIAL_ANR_MECHANISM;
        case DISABLE_ALL_ANR_MECHANISM:
            return DISABLE_ALL_ANR_MECHANISM;
        default:
            return ENABLE_ALL_ANR_MECHANISM;
        }
    }

    /**
     * Inform ANR Manager about interested events.
     *
     * @param event The interested event defined at AnrManagerService
     */
    public void writeEvent(int event) {
        switch (event) {
        case EVENT_BOOT_COMPLETED:
            mEventBootCompleted = SystemClock.uptimeMillis();
            break;
        default:
            break;
        }
    }

    /**
     * Check if ANR is deferrable.
     *
     * @return true if ANR can be deferred
     */
    public boolean isAnrDeferrable() {
        if (DISABLE_ALL_ANR_MECHANISM == checkAnrDebugMechanism()) {
            return false;
        }

        if ("dexopt".equals(SystemProperties.get("vendor.anr.autotest"))) {
            Slog.i(TAG,
                    "We are doing TestDexOptSkipANR; return true in this case");
            return true;
        }
        if ("enable".equals(SystemProperties.get("vendor.anr.autotest"))) {
            Slog.i(TAG, "Do Auto Test, don't skip ANR");
            return false;
        }

        long now = SystemClock.uptimeMillis();
        if (!IS_USER_BUILD) {
            if (mEventBootCompleted == 0
                    || (now - mEventBootCompleted < ANR_BOOT_DEFER_TIME)) {
                /* ANR happened before boot completed + N seconds */
                Slog.i(TAG, "isAnrDeferrable(): true since"
                        + " mEventBootCompleted = " + mEventBootCompleted
                        + " now = " + now);
                return true;
            } else if (isException()) {
                Slog.i(TAG, "isAnrDeferrable(): true since exception");
                return true;
            } else {
                float lastCpuUsage = mAnrProcessStats.getTotalCpuPercent();
                updateProcessStats();
                float currentCpuUsage = mAnrProcessStats.getTotalCpuPercent();
                if (lastCpuUsage > ANR_CPU_THRESHOLD
                        && currentCpuUsage > ANR_CPU_THRESHOLD) {
                    if (mCpuDeferred == 0) {
                        mCpuDeferred = now;
                        Slog.i(TAG, "isAnrDeferrable(): true since CpuUsage = "
                                + currentCpuUsage + ", mCpuDeferred = "
                                + mCpuDeferred);
                        return true;
                    } else if (now - mCpuDeferred < ANR_CPU_DEFER_TIME) {
                        Slog.i(TAG, "isAnrDeferrable(): true since CpuUsage = "
                                + currentCpuUsage + ", mCpuDeferred = "
                                + mCpuDeferred + ", now = " + now);
                        return true;
                    }
                }
                mCpuDeferred = 0;
            }
        }
        return false;
    }

    /**
     * Check if ANR flow is skipped or not.
     *
     * @return true if ANR is skipped
     */
    public boolean isAnrFlowSkipped(int appPid, String appProcessName,
            String annotation) {
        if (INVALID_ANR_FLOW == mAnrFlow) {
            mAnrFlow = SystemProperties.getInt("persist.vendor.dbg.anrflow",
                    NORMAL_ANR_FLOW);
        }
        Slog.i(TAG, "isANRFlowSkipped() AnrFlow = " + mAnrFlow);

        switch (mAnrFlow) {
            case NORMAL_ANR_FLOW:
                return false;
            case SKIP_ANR_FLOW:
                Slog.i(TAG, "Skipping ANR flow: " + appPid + " " + appProcessName
                        + " " + annotation);
                return true;
            case SKIP_ANR_FLOW_AND_KILL:
                if (appPid != Process.myPid()) {
                    Slog.i(TAG, "Skipping ANR flow: " + appPid + " "
                        + appProcessName + " " + annotation);
                    Slog.w(TAG, "Kill process (" + appPid + ") due to ANR");
                    Process.killProcess(appPid);
                }
                return true;
            default:
                return false;
        }
    }

    public void updateProcessStats() {
        synchronized (mAnrProcessStats) {
            final long now = SystemClock.uptimeMillis();
            if ((now - mLastCpuUpdateTime.get()) > MONITOR_CPU_MIN_TIME) {
                mLastCpuUpdateTime.set(now);
                mAnrProcessStats.update();
            }
        }
    }

    public String getProcessState() {
        synchronized (mAnrProcessStats) {
            return mAnrProcessStats.printCurrentState(SystemClock
                    .uptimeMillis());
        }
    }

    public String getAndroidTime() {
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat(
                "yyyy-MM-dd HH:mm:ss.SS");
        Date date = new Date(System.currentTimeMillis());
        Formatter formatter = new Formatter();
        return "Android time :["
                + simpleDateFormat.format(date)
                + "] ["
                + formatter.format("%.3f",
                        (float) SystemClock.uptimeMillis() / 1000) + "]\n";
    }

    public File createFile(String filepath) {
        File file = new File(filepath);
        if (file == null || !file.exists()) {
            Slog.i(TAG, filepath + " isn't exist");
            return null;
        }
        return file;
    }

    public boolean copyFile(File srcFile, File destFile) {
        boolean result = false;
        try {
            if (!srcFile.exists()) {
                return result;
            }
            if (!destFile.exists()) {
                destFile.createNewFile();
                FileUtils.setPermissions(destFile.getPath(), 0666, -1, -1); // -rw-rw-rw-
            }

            InputStream in = new FileInputStream(srcFile);
            try {
                result = copyToFile(in, destFile);
            } finally {
                in.close();
            }
        } catch (IOException e) {
            Slog.e(TAG, "createFile fail");
            result = false;
        }
        return result;
    }

    public boolean copyToFile(InputStream inputStream, File destFile) {
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(destFile, true);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) >= 0) {
                out.write(buffer, 0, bytesRead);
            }
            out.flush();
            out.getFD().sync();
        } catch (IOException e) {
            Slog.w(TAG, "copyToFile fail" , e);
            return false;
        } finally {
            try {
                if (out != null) out.close();
            } catch (IOException e) {
                Slog.w(TAG, "close failed..");
            }
        }
        return true;
    }

    public void stringToFile(String filename, String string) throws IOException {
        FileWriter out = new FileWriter(filename, true);
        try {
            out.write(string);
        } finally {
            out.close();
        }
    }

    public class BinderDumpThread extends Thread {
        private int mPid;

        public BinderDumpThread(int pid) {
            mPid = pid;
        }

        public void run() {
            dumpBinderInfo(mPid);
        }
    }

    public void dumpBinderInfo(int pid) {
        try {
            File binderinfo = new File("/data/anr/binderinfo");
            if (binderinfo.exists()) {
                if (binderinfo.delete() == false) {
                    Slog.e(TAG,
                            "dumpBinderInfo fail due to file likely to be locked by others");
                    return;
                }
                if (binderinfo.createNewFile() == false) {
                    Slog.e(TAG,
                            "dumpBinderInfo fail due to file cannot be created");
                    return;
                }
                FileUtils.setPermissions(binderinfo.getPath(), 0666, -1, -1); // -rw-rw-rw-
            }

            File file = createFile("/sys/kernel/debug/binder/failed_transaction_log");
            if (null != file) {
                stringToFile("/data/anr/binderinfo",
                        "------ BINDER FAILED TRANSACTION LOG ------\n");
                copyFile(file, binderinfo);
            }

            file = createFile("sys/kernel/debug/binder/timeout_log");
            if (null != file) {
                stringToFile("/data/anr/binderinfo",
                        "------ BINDER TIMEOUT LOG ------\n");
                copyFile(file, binderinfo);
            }

            file = createFile("/sys/kernel/debug/binder/transaction_log");
            if (null != file) {
                stringToFile("/data/anr/binderinfo",
                        "------ BINDER TRANSACTION LOG ------\n");
                copyFile(file, binderinfo);
            }

            file = createFile("/sys/kernel/debug/binder/transactions");
            if (null != file) {
                stringToFile("/data/anr/binderinfo",
                        "------ BINDER TRANSACTIONS ------\n");
                copyFile(file, binderinfo);
            }

            file = createFile("/sys/kernel/debug/binder/stats");
            if (null != file) {
                stringToFile("/data/anr/binderinfo",
                        "------ BINDER STATS ------\n");
                copyFile(file, binderinfo);
            }

            String filepath = "/sys/kernel/debug/binder/proc/"
                    + Integer.toString(pid);
            file = new File(filepath);

            if (null != file) {
                stringToFile("/data/anr/binderinfo",
                        "------ BINDER PROCESS STATE: $i ------\n");
                copyFile(file, binderinfo);
            }

        } catch (IOException e) {
            Slog.e(TAG, "dumpBinderInfo fail");
        }
    }

    public void enableTraceLog(boolean enable) {
        Slog.i(TAG, "enableTraceLog: " + enable);
        if (null != exceptionLog) {
            exceptionLog.switchFtrace(enable ? 1 : 0);
        }
    }

    private void writeStringToFile(String filepath, String string) {
        if (filepath == null) {
            return;
        }

        File file = new File(filepath);
        FileOutputStream out = null;

        StrictMode.ThreadPolicy oldPolicy = StrictMode.allowThreadDiskReads();
        StrictMode.allowThreadDiskWrites();

        try {
            out = new FileOutputStream(file);
            out.write(string.getBytes());
            out.flush();
        } catch (IOException e) {
            Slog.e(TAG,
                    "writeStringToFile error: " + filepath + " " + e.toString());
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException ioe) {
                    Slog.e(TAG, "writeStringToFile close error: " + filepath
                            + " " + ioe.toString());
                }
            }
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    private boolean isBuiltinApp(ApplicationInfo appInfo) {
        return (appInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0
            || (appInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0;
    }

    /**
     * the 3rd party's anr dump info, it can be disabled in user load for performance
     */
    private boolean needAnrDump(ApplicationInfo appInfo) {
        return isBuiltinApp(appInfo) || (SystemProperties.getInt(
                "persist.vendor.anr.dumpthr",ENABLE_ANR_DUMP_FOR_3RD_APP) != DISABLE_ANR_DUMP_FOR_3RD_APP);
    }

}
