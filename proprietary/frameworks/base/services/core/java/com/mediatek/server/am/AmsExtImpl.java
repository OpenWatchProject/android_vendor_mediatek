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

package com.mediatek.server.am;

import static com.android.server.am.ActivityManagerDebugConfig.APPEND_CATEGORY_NAME;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_ALL;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_ANR;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_BACKGROUND_CHECK;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_BACKUP;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_BROADCAST;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_BROADCAST_BACKGROUND;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_BROADCAST_LIGHT;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_BROADCAST_DEFERRAL;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_LRU;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_MU;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_NETWORK;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_OOM_ADJ;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_OOM_ADJ_REASON;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_POWER;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_POWER_QUICK;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_PROCESS_OBSERVERS;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_PROCESSES;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_PROVIDER;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_PSS;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_SERVICE;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_FOREGROUND_SERVICE;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_SERVICE_EXECUTING;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_UID_OBSERVERS;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_USAGE_STATS;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_PERMISSIONS_REVIEW;
import static com.android.server.am.ActivityManagerDebugConfig.DEBUG_WHITELISTS;

import static com.android.server.wm.ActivityTaskManagerDebugConfig.APPEND_CATEGORY_NAME;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_ALL;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_ALL_ACTIVITIES;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_ADD_REMOVE;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_CONFIGURATION;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_CONTAINERS;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_FOCUS;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_IMMERSIVE;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_LOCKTASK;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_PAUSE;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_RECENTS;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_RECENTS_TRIM_TASKS;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_SAVED_STATE;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_STACK;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_STATES;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_SWITCH;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_TASKS;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_TRANSITION;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_VISIBILITY;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_APP;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_IDLE;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_RELEASE;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_USER_LEAVING;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_PERMISSIONS_REVIEW;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_RESULTS;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_CLEANUP;
import static com.android.server.wm.ActivityTaskManagerDebugConfig.DEBUG_METRICS;

import android.app.ActivityManagerInternal;
import android.app.ActivityManager;
import android.app.AppGlobals;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageManager;
import android.content.pm.PackageManager;
import android.content.pm.PackageManagerInternal;
import android.content.pm.ResolveInfo;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.util.ArrayMap;
import android.util.Slog;
import android.util.SparseArray;
import com.android.internal.app.procstats.ProcessStats;
import com.android.internal.os.ProcessCpuTracker;
import com.android.server.LocalServices;
import com.android.server.am.ActivityManagerService;
import com.android.server.wm.ActivityRecord;
import com.android.server.am.ProcessList;
import com.android.server.am.ProcessRecord;
import com.android.server.am.ActivityManagerDebugConfig;
import com.android.server.wm.ActivityTaskManagerDebugConfig;
import android.content.pm.ApplicationInfo;

/// M: App-based AAL @{
import com.mediatek.amsAal.AalUtils;
/// @}

///M: CTA permission control
import com.mediatek.cta.CtaManagerFactory;
import com.mediatek.cta.CtaManager;

/// M: DuraSpeed @{
import com.mediatek.duraspeed.manager.IDuraSpeedNative;
import com.mediatek.duraspeed.suppress.ISuppressAction;
/// @}

import com.mediatek.server.powerhal.PowerHalManager;
import com.mediatek.server.powerhal.PowerHalManagerImpl;

/// M: DuraSpeed @{
import dalvik.system.PathClassLoader;
/// @}

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.PrintWriter;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AmsExtImpl extends AmsExt {
    private static final String TAG = "AmsExtImpl";
    private static final String INKERNEL_MINFREE_PATH =
            "/sys/module/lowmemorykiller/parameters/minfree";
    private boolean isDebug = false;

    private boolean isDuraSpeedSupport = "1".equals
            (SystemProperties.get("persist.vendor.duraspeed.support"));

    /// M: AGO heavy loading performance improve
    private boolean isHeavyLoadingSupport = "1".equals(
            SystemProperties.get("persist.vendor.heavy.loading.support"));
    private ArrayList<String> mSuppressList = new ArrayList<String>();
    private String mCurrentPackage;
    private static final String SANITY_CASE_PREF = "sanity";
    private static final String CASE_PREF = "case";
    private static final String[] WHITE_LIST = {};

    public PowerHalManagerImpl mPowerHalManagerImpl = null;

    /// M: App-based AAL @{
    private AalUtils mAalUtils = null;
    /// @}

    /// M: DuraSpeed @{
    public static PathClassLoader sClassLoader;
    private IDuraSpeedNative mDuraSpeedService;
    private ISuppressAction mSuppressAction;
    private Context mContext;
    /// @}

    private ActivityManagerInternal mActivityManagerInternal;
    private PackageManagerInternal mPackageManagerInternal;
    private Method mStartProcessMethod;
    private Field mProcessNamesField;

    private final String amsLogProp = "persist.vendor.sys.activitylog";

    ///M: CTA permission control
    private CtaManager mCtaManager = null;
    private ActivityManager mAm = null;

    public AmsExtImpl() {
        mPowerHalManagerImpl = new PowerHalManagerImpl();
        /// M: DuraSpeed @{
        if (isDuraSpeedSupport) {
            String className1 = "com.mediatek.duraspeed.manager.DuraSpeedService";
            String className2 = "com.mediatek.duraspeed.suppress.SuppressAction";
            String classPackage = "/system/framework/duraspeed.jar";
            Class<?> clazz = null;
            try {
                sClassLoader = new PathClassLoader(classPackage, AmsExtImpl.class.getClassLoader());
                clazz = Class.forName(className1, false, sClassLoader);
                mDuraSpeedService = (IDuraSpeedNative) clazz.getConstructor().newInstance();

                clazz = Class.forName(className2, false, sClassLoader);
                mSuppressAction = (ISuppressAction) clazz.getConstructor().newInstance();
            } catch (Exception e) {
                Slog.e("AmsExtImpl", e.toString());
            }
        }
        /// @}

        /// M: App-based AAL @{
        if (mAalUtils == null && AalUtils.isSupported()) {
            mAalUtils = AalUtils.getInstance();
        }
        ///@}
    }

    @Override
    public void onAddErrorToDropBox(String dropboxTag, String info, int pid) {
        if (isDebug) {
            Slog.d(TAG, "onAddErrorToDropBox, dropboxTag=" + dropboxTag
                    + ", info=" + info
                    + ", pid=" + pid);
        }
    }

    @Override
    public void onSystemReady(Context context) {
        Slog.d(TAG, "onSystemReady");

        /// M: DuraSpeed @{
        if (isDuraSpeedSupport && mDuraSpeedService != null) {
            mDuraSpeedService.onSystemReady();
        }
        mContext = context;
        /// @}
    }

    @Override
    public void onBeforeActivitySwitch(ActivityRecord lastResumedActivity,
                                       ActivityRecord nextResumedActivity,
                                       boolean pausing,
                                       int nextResumedActivityType) {
        if (nextResumedActivity == null || nextResumedActivity.info == null
                || lastResumedActivity == null) {
            return;
        } else if (nextResumedActivity.packageName == lastResumedActivity.packageName
                && nextResumedActivity.info.name == lastResumedActivity.info.name) {
            // same activity
            return;
        }

        String lastResumedPackageName = lastResumedActivity.packageName;
        String nextResumedPackageName = nextResumedActivity.packageName;
        if (isDebug) {
            Slog.d(TAG, "onBeforeActivitySwitch, lastResumedPackageName=" + lastResumedPackageName
                    + ", nextResumedPackageName=" + nextResumedPackageName);
        }

        if (mPowerHalManagerImpl != null) {
            mPowerHalManagerImpl.amsBoostResume(lastResumedPackageName, nextResumedPackageName);
        }

        /// M: DuraSpeed @{
        if (isDuraSpeedSupport && mDuraSpeedService != null) {
            mDuraSpeedService.onBeforeActivitySwitch(lastResumedActivity,
                    nextResumedActivity, pausing, nextResumedActivityType);
        }
        /// @}

        /// M: For ago performance
        checkSuppressInfo(lastResumedPackageName, nextResumedPackageName);
    }

    @Override
    public void onAfterActivityResumed(ActivityRecord resumedActivity) {
        if (resumedActivity.app == null) {
            return;
        }

        int pid = resumedActivity.app.mPid;
        int uid = resumedActivity.app.mUid;
        String activityName = resumedActivity.info.name;
        String packageName = resumedActivity.info.packageName;
        if (isDebug) {
            Slog.d(TAG, "onAfterActivityResumed, pid=" + pid
                    + ", activityName=" + activityName
                    + ", packageName=" + packageName);
        }

        if (mPowerHalManagerImpl != null) {
            mPowerHalManagerImpl.amsBoostNotify(pid, activityName, packageName, uid);
        }

        /// M: App-based AAL @{
        if (mAalUtils != null) {
            mAalUtils.onAfterActivityResumed(packageName, activityName);
        }
        /// @}
    }

    @Override
    public void onUpdateSleep(boolean wasSleeping, boolean isSleepingAfterUpdate) {
        if (isDebug) {
            Slog.d(TAG, "onUpdateSleep, wasSleeping=" + wasSleeping
                    + ", isSleepingAfterUpdate=" + isSleepingAfterUpdate);
        }
        /// M: App-based AAL @{
        if (mAalUtils != null) {
            mAalUtils.onUpdateSleep(wasSleeping, isSleepingAfterUpdate);
        }
        /// @}
    }

    /// M: App-based AAL @{
    @Override
    public void setAalMode(int mode) {
        if (mAalUtils != null) {
            mAalUtils.setAalMode(mode);
        }
    }

    @Override
    public void setAalEnabled(boolean enabled) {
        if (mAalUtils != null) {
            mAalUtils.setEnabled(enabled);
        }
    }

    @Override
    public int amsAalDump(PrintWriter pw, String[] args, int opti) {
        if (mAalUtils != null) {
            return mAalUtils.dump(pw, args, opti);
        } else {
            return opti;
        }
    }
    /// @}

    @Override
    public void onStartProcess(String hostingType, String packageName) {
        if (isDebug) {
            Slog.d(TAG, "onStartProcess, hostingType=" + hostingType
                    + ", packageName=" + packageName);
        }

        if (mPowerHalManagerImpl != null) {
            mPowerHalManagerImpl.amsBoostProcessCreate(hostingType, packageName);
        }
    }

    @Override
    public void onNotifyAppCrash(int pid, int uid, String packageName) {
        if (isDebug) {
            Slog.d(TAG, "onNotifyAppCrash, packageName=" + packageName
                    + ", pid=" + pid);
        }

        if (mPowerHalManagerImpl != null) {
            mPowerHalManagerImpl.NotifyAppCrash(pid, uid, packageName);
        }
    }

    @Override
    public void onEndOfActivityIdle(Context context, Intent idleIntent) {
        if (isDebug) {
            Slog.d(TAG, "onEndOfActivityIdle, idleIntent=" + idleIntent);
        }

        if (mPowerHalManagerImpl != null) {
            mPowerHalManagerImpl.amsBoostStop();
        }
        /// M: DuraSpeed @{
        if (isDuraSpeedSupport && mDuraSpeedService != null) {
            mDuraSpeedService.onActivityIdle(context, idleIntent);
        }
        /// @}
    }

    @Override
    public void enableAmsLog(ArrayList<ProcessRecord> lruProcesses) {
        String activitylog = SystemProperties.get(amsLogProp, null);
        if (activitylog != null && !activitylog.equals("")) {
            if (activitylog.indexOf(" ") != -1
                    && activitylog.indexOf(" ") + 1 <= activitylog.length()) {
                String[] args = new String[2];
                args[0] = activitylog.substring(0, activitylog.indexOf(" "));
                args[1] = activitylog.substring(activitylog.indexOf(" ") + 1, activitylog.length());
                enableAmsLog(null, args, 0, lruProcesses);
            } else {
                SystemProperties.set(amsLogProp, "");
            }
        }
    }

    @Override
    public void enableAmsLog(PrintWriter pw, String[] args,
            int opti, ArrayList<ProcessRecord> lruProcesses) {
        String option = null;
        boolean isEnable = false;
        int indexLast = opti + 1;

        if (indexLast >= args.length) {
            if (pw != null) {
                pw.println("  Invalid argument!");
            }
            SystemProperties.set(amsLogProp, "");
        } else {
            option = args[opti];
            isEnable = "on".equals(args[indexLast]) ? true : false;
            SystemProperties.set(amsLogProp, args[opti] + " " + args[indexLast]);

            if (option.equals("x")) {
                enableAmsLog(isEnable, lruProcesses);
            } else {
                if (pw != null) {
                    pw.println("  Invalid argument!");
                }
                SystemProperties.set(amsLogProp, "");
            }
        }
    }

    private void enableAmsLog(boolean isEnable, ArrayList<ProcessRecord> lruProcesses) {
        isDebug = isEnable;
        // From AMS debug config
        ActivityManagerDebugConfig.APPEND_CATEGORY_NAME = isEnable;
        ActivityManagerDebugConfig.DEBUG_ALL = isEnable;
        DEBUG_ANR = isEnable;
        DEBUG_BACKGROUND_CHECK = isEnable;
        DEBUG_BACKUP = isEnable;
        DEBUG_BROADCAST = isEnable;
        DEBUG_BROADCAST_BACKGROUND = isEnable;
        DEBUG_BROADCAST_LIGHT = isEnable;
        DEBUG_BROADCAST_DEFERRAL = isEnable;
        DEBUG_LRU = isEnable;
        DEBUG_MU = isEnable;
        DEBUG_NETWORK = isEnable;
        // DEBUG_OOM_ADJ = isEnable;
        // DEBUG_OOM_ADJ_REASON = isEnable;
        DEBUG_POWER = isEnable;
        DEBUG_POWER_QUICK = isEnable;
        DEBUG_PROCESS_OBSERVERS = isEnable;
        DEBUG_PROCESSES = isEnable;
        DEBUG_PROVIDER = isEnable;
        DEBUG_PSS = isEnable;
        DEBUG_SERVICE = isEnable;
        DEBUG_FOREGROUND_SERVICE = isEnable;
        DEBUG_SERVICE_EXECUTING = isEnable;
        DEBUG_UID_OBSERVERS = isEnable;
        DEBUG_USAGE_STATS = isEnable;
        ActivityManagerDebugConfig.DEBUG_PERMISSIONS_REVIEW = isEnable;
        DEBUG_WHITELISTS = isEnable;

        // From Task Manager debug config
        ActivityTaskManagerDebugConfig.APPEND_CATEGORY_NAME = isEnable;
        ActivityTaskManagerDebugConfig.DEBUG_ALL = isEnable;
        DEBUG_ALL_ACTIVITIES = isEnable;
        DEBUG_ADD_REMOVE = isEnable;
        DEBUG_CONFIGURATION = isEnable;
        DEBUG_CONTAINERS = isEnable;
        DEBUG_FOCUS = isEnable;
        DEBUG_IMMERSIVE = isEnable;
        DEBUG_LOCKTASK = isEnable;
        DEBUG_PAUSE = isEnable;
        DEBUG_RECENTS = isEnable;
        DEBUG_RECENTS_TRIM_TASKS = isEnable;
        DEBUG_SAVED_STATE = isEnable;
        DEBUG_STACK = isEnable;
        DEBUG_STATES = isEnable;
        DEBUG_SWITCH = isEnable;
        DEBUG_TASKS = isEnable;
        DEBUG_TRANSITION = isEnable;
        DEBUG_VISIBILITY = isEnable;
        DEBUG_APP = isEnable;
        DEBUG_IDLE = isEnable;
        DEBUG_RELEASE = isEnable;
        DEBUG_USER_LEAVING = isEnable;
        ActivityTaskManagerDebugConfig.DEBUG_PERMISSIONS_REVIEW = isEnable;
        DEBUG_RESULTS = isEnable;
        DEBUG_CLEANUP = isEnable;
        DEBUG_METRICS = isEnable;

        for (int i = 0; i < lruProcesses.size(); i++) {
            ProcessRecord app = lruProcesses.get(i);
            if (app != null && app.thread != null) {
                try {
                    app.thread.enableActivityThreadLog(isEnable);
                } catch (Exception e) {
                     Slog.e(TAG, "Error happens when enableActivityThreadLog", e);
                }
            }
        }
    }

    /// M: DuraSpeed @{
    @Override
    public void onWakefulnessChanged(int wakefulness) {
        if (isDuraSpeedSupport && mDuraSpeedService != null) {
            mDuraSpeedService.onWakefulnessChanged(wakefulness);
        }
    }

    @Override
    public void addDuraSpeedService() {
        if (isDuraSpeedSupport && mDuraSpeedService != null) {
            ServiceManager.addService("duraspeed", (IBinder) mDuraSpeedService, true);
        }
    }

    @Override
    public void startDuraSpeedService(Context context) {
        if (isDuraSpeedSupport && mDuraSpeedService != null) {
            mDuraSpeedService.startDuraSpeedService(context);
            File file = new File(INKERNEL_MINFREE_PATH);
            if (!file.exists()) {
                MemoryServerThread memoryServer = new MemoryServerThread();
                memoryServer.start();
            }
        }
    }

    @Override
    public String onReadyToStartComponent(String packageName, int uid,
            String suppressReason, String className) {
        if (mDuraSpeedService != null && mDuraSpeedService.isDuraSpeedEnabled()) {
            return mSuppressAction.onReadyToStartComponent(packageName, uid,
                    suppressReason, className);
        }
        return null;
    }

    @Override
    public boolean onBeforeStartProcessForStaticReceiver(String packageName) {
        if (mDuraSpeedService != null && mDuraSpeedService.isDuraSpeedEnabled()) {
            return mSuppressAction.onBeforeStartProcessForStaticReceiver(packageName);
        }
        return false;
    }

    @Override
    public void addToSuppressRestartList(String packageName) {
        if (mDuraSpeedService != null && mDuraSpeedService.isDuraSpeedEnabled() &&
                mContext != null) {
            mSuppressAction.addToSuppressRestartList(mContext, packageName);
        }
    }

    @Override
    public boolean notRemoveAlarm(String packageName) {
        if (mDuraSpeedService != null && mDuraSpeedService.isDuraSpeedEnabled()) {
            return mSuppressAction.notRemoveAlarm(packageName);
        }
        return false;
    }
    /// @}

    /// Enhance wtf excetpion
    public boolean IsBuildInApp() {
        IPackageManager pm = AppGlobals.getPackageManager();
        try {
            String pkgName = pm.getNameForUid(Binder.getCallingUid());
            ApplicationInfo appInfo = pm.getApplicationInfo(pkgName, 0, UserHandle.getCallingUserId());
            //Build in app
            if (appInfo != null && ((appInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0
                || (appInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0)) {
                    return true;
            }
        } catch (RemoteException e) {
                Slog.e(TAG, "getCallerProcessName exception :" + e);
        }
        return false;
    }

    @Override
    public boolean preLaunchApplication(String callingPackage, Intent intent, String resolvedType,
            int startFlags) {
        if ("com.mediatek.duraspeedml".equals(callingPackage)) {
            int modifiedFlags = startFlags
                    | PackageManager.MATCH_DEFAULT_ONLY
                    | PackageManager.GET_SHARED_LIBRARY_FILES;
            if (intent.isWebIntent()
                    || (intent.getFlags() & Intent.FLAG_ACTIVITY_MATCH_EXTERNAL) != 0) {
                modifiedFlags |= PackageManager.MATCH_INSTANT;
            }
            if (mActivityManagerInternal == null) {
                mActivityManagerInternal = LocalServices.getService(ActivityManagerInternal.class);
            }
            if (mPackageManagerInternal == null) {
                mPackageManagerInternal = LocalServices.getService(PackageManagerInternal.class);
            }
            ResolveInfo rInfo = mPackageManagerInternal.resolveIntent(
                    intent,
                    resolvedType,
                    modifiedFlags,
                    UserHandle.getCallingUserId(),
                    true,
                    Binder.getCallingUid());
            if (rInfo == null) {
                return true;
            }
            ActivityInfo aInfo = rInfo.activityInfo;
            if (aInfo == null || aInfo.applicationInfo == null) {
                return true;
            }
            if (mStartProcessMethod == null) {
                try {
                    mStartProcessMethod = ActivityManagerInternal.class
                            .getDeclaredMethod("startProcess",
                                    String.class, ApplicationInfo.class, boolean.class,
                                    String.class, ComponentName.class);
                    mStartProcessMethod.setAccessible(true);
                } catch (NoSuchMethodException e) {
                    Slog.e(TAG, "preLaunchApplication, no such method" + e.getMessage());
                }
            }
            if (mStartProcessMethod != null) {
                try {
                    mStartProcessMethod.invoke(mActivityManagerInternal, aInfo.processName,
                            aInfo.applicationInfo, true, "activity", intent.getComponent());
                } catch (IllegalAccessException e) {
                    Slog.e(TAG, "preLaunchApplication, IllegalAccessException:"
                            + e.getMessage());
                } catch (IllegalArgumentException e) {
                    Slog.e(TAG, "preLaunchApplication, IllegalArgumentException:"
                            + e.getMessage());
                } catch (InvocationTargetException e) {
                    Slog.e(TAG, "preLaunchApplication, InvocationTargetException:"
                            + e.getMessage());
                }
            }
            return true;
        }
        return false;
    }

    /// M: CTA requirement
    public boolean checkAutoBootPermission(Context context,String packageName, int userId,
            ArrayList<ProcessRecord> runningProcess, int callingPid) {
        if (mCtaManager == null) {
            mCtaManager = CtaManagerFactory.getInstance().makeCtaManager();
        }

        if (!mCtaManager.isCtaSupported()) {
            return true;
        }

        for (int i = runningProcess.size() - 1; i >= 0; i--) {
            ProcessRecord processRecord = runningProcess.get(i);
            if (processRecord.pid == callingPid) {
                /*Slog.e(TAG, "check Pid."
                            + ",callingPid:" + callingPid
                            + ", calling.name:" + processRecord.processName
                            + " start process:" + packageName
                            + " processRecord.curAdj:" + processRecord.curAdj
                            + " ProcessList.PERCEPTIBLE_APP_ADJ:"
                            + ProcessList.PERCEPTIBLE_APP_ADJ);*/
                if (processRecord.curAdj > ProcessList.PERCEPTIBLE_APP_ADJ) {
                    boolean result  = mCtaManager.checkAutoBootPermission(context,
                        packageName, userId);
                    Slog.e(TAG, "check result:" + result);
                    if(!result) {
                        Slog.e(TAG, "can't start procss"
                            + " because auto boot permission."
                            + " calling package:" + processRecord.processName
                            + " start process:" + packageName);
                        if (mAm == null) {
                            mAm = (ActivityManager) context.getSystemService(
                            Context.ACTIVITY_SERVICE);
                        }
                        mAm.forceStopPackageAsUser(packageName, userId);
                        return false;
                    }
                }
            }
        }
        return true;
    }

    @Override
    public void forceStopRelatedApps(Context context, ProcessRecord app, ApplicationInfo appInfo,
            int userId, ArrayList<ProcessRecord> lruProcesses) {
        if (isDuraSpeedSupport && mDuraSpeedService != null) {
            mDuraSpeedService.onAppProcessDied(app.processName, appInfo.packageName);
            return;
        }
        if (!isHeavyLoadingSupport) {
            return;
        }
        int minAdj = app.curAdj;
        if (!appInfo.isSystemApp() && app.curAdj > 200
            && !isDefaultWhitelistAPP(appInfo.packageName)
            && !mSuppressList.contains(appInfo.packageName)) {
            for (ProcessRecord pr : lruProcesses) {
                for (String packageName : pr.getPackageList()) {
                    if (appInfo.packageName.equals(packageName)) {
                        minAdj = Math.min(pr.curAdj, minAdj);
                    }
                }
            }

            if (minAdj > 200) {
                if (mAm == null) {
                    mAm = (ActivityManager) context.getSystemService(
                    Context.ACTIVITY_SERVICE);
                }
                Slog.d(TAG, "force stop process: " + app.processName
                    + " curAdj=" + app.curAdj
                    + " packageName: " + appInfo.packageName
                    + " minAdj= " + minAdj);
                mSuppressList.add(appInfo.packageName);
                mAm.forceStopPackageAsUser(appInfo.packageName, userId);
            }
        }
    }

    @Override
    public boolean isComponentNeedsStart(String packageName, String suppressReason) {
        if (isDuraSpeedSupport || !isHeavyLoadingSupport) {
            return true;
        }
        if (mSuppressList.contains(packageName)) {
            Slog.d(TAG, "the " + suppressReason + " can't start,"
                + " related packageName: " + packageName
                + " is in the suppress list");
            return false;
        }
        return true;
    }


     /**
     * Add for AGO project heavy loading performance.
     * check the suppresslist when the activity start.
     * to void user can't start the related APP.
     */
    private void checkSuppressInfo(String lastResumedPackageName,
            String nextResumedPackageName) {
        if (isDuraSpeedSupport || !isHeavyLoadingSupport) {
            return;
        }

        if (lastResumedPackageName != null) {
            if (lastResumedPackageName.equals(nextResumedPackageName)) {
                return;
            }
        }
        if (mCurrentPackage != null) {
            if (mCurrentPackage.equals(nextResumedPackageName)) {
                return;
            }
        }

        mCurrentPackage = nextResumedPackageName;
        if (mSuppressList.contains(mCurrentPackage)) {
            Slog.d(TAG, "maybe user start the app, packageName: " + mCurrentPackage
                + ", need remove it from suppress List");
            mSuppressList.remove(mCurrentPackage);
        }
    }

     /**
     * Add for AGO project heavy loading performance.
     * Add whitelist for some app can't force stop.
     */
    private boolean isDefaultWhitelistAPP(String packageName) {
        // For test app (google, mediatek).
        if (packageName.startsWith("android") ||
                packageName.startsWith("com.android") ||
                packageName.startsWith("com.google.android") ||
                packageName.toLowerCase().contains(SANITY_CASE_PREF) ||
                packageName.toLowerCase().contains(CASE_PREF)) {
            return true;
        }

        // For some special app internal.
        for (String whitelist : WHITE_LIST) {
            if (packageName.equals(whitelist)) {
                return true;
            }
        }
        return false;
    }

    // Maintain a task which waiting for LMKD client connected, LMKD will trigger DuraSpeed.
    private class MemoryServerThread extends Thread {
        public static final String HOST_NAME = "duraspeed_memory";

        @Override
        public void run() {
            LocalServerSocket serverSocket = null;
            ExecutorService threadExecutor = Executors.newCachedThreadPool();

            try {
                Slog.d(TAG, "Crate local socket: duraspeed_memory");
                // Create server socket
                serverSocket = new LocalServerSocket(HOST_NAME);

                while (true) {
                    Slog.d(TAG, "Waiting Client connected...");
                    // Allow multiple connection connect to server.
                    LocalSocket socket = serverSocket.accept();
                    socket.setReceiveBufferSize(256);
                    socket.setSendBufferSize(256);
                    Slog.i(TAG, "There is a client is accepted: " + socket.toString());
                    threadExecutor.execute(new ConnectionHandler(socket));
                }
            } catch (Exception e) {
                Slog.w(TAG, "listenConnection catch Exception");
                e.printStackTrace();
            } finally {
                Slog.d(TAG, "listenConnection finally shutdown!!");
                if (threadExecutor != null )
                    threadExecutor.shutdown();
                if (serverSocket != null) {
                    try {
                        serverSocket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
            Slog.d(TAG, "listenConnection() - end");
        }
    }

    /*
     * level: 0\1\2 means low\medium\critical
     * pressure: value range 0~100, for low memory project:
     *     >60 means low, <40 means critical. 40~60 means medium.
     */
    public class ConnectionHandler implements Runnable {
        private LocalSocket mSocket;
        private boolean mIsContinue = true;
        private InputStreamReader mInput = null;
        private DataOutputStream mOutput = null;

        public ConnectionHandler(LocalSocket clientSocket) {
            mSocket = clientSocket;
        }

        public void terminate() {
            Slog.d(TAG, "DuraSpeed memory trigger process terminate.");
            mIsContinue = false;
        }

        @Override
        public void run() {
            Slog.i(TAG, "DuraSpeed new connection: " + mSocket.toString());

            try {
                mInput = new InputStreamReader(mSocket.getInputStream());
                mOutput = new DataOutputStream(mSocket.getOutputStream());
                try {
                    final BufferedReader bufferedReader = new BufferedReader(mInput);
                    while (mIsContinue) {
                        String lmkdData = bufferedReader.readLine();
                        String[] result = lmkdData.split(":");
                        if (result[0] == null || result[1] == null) {
                            Slog.e(TAG, "Received lmkdData error");
                            continue;
                        }
                        int minScoreAdj = Integer.parseInt(result[0].trim());
                        int minFree = Integer.parseInt(result[1].trim());
                        String level = result[2].trim();
                        String memPressrue = result[3].trim();
                        int levelResult = Integer.parseInt(level);
                        int memPressrueResult = Integer.parseInt(memPressrue);
                        if (mDuraSpeedService.isDuraSpeedEnabled()) {
                            mDuraSpeedService.triggerMemory(minFree * 4, memPressrueResult);
                        }
                    }
                } catch (Exception e) {
                    Slog.w(TAG, "duraSpeed: memory Exception.");
                    e.printStackTrace();
                    terminate();
               }
                Slog.w(TAG, "duraSpeed: New connection running ending ");

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}

