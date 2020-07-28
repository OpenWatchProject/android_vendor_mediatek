/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.powerhalwrapper;

import android.util.Log;
import android.util.Printer;
import android.os.Build;
import android.os.Looper;
import android.os.Process;
import android.os.RemoteException;
import android.os.Binder;
import android.os.SystemProperties;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.PrintStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;


import java.util.HashMap;
import java.util.Iterator;
import android.util.Log;
import android.os.Trace;
import android.os.HwBinder;
import android.os.IHwBinder;
import android.os.IHwInterface;
import android.os.SystemProperties;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/// MTK power
import vendor.mediatek.hardware.power.V2_0.*;

public class PowerHalWrapper {

    private static final String TAG = "PowerHalWrapper";

    private static final int AMS_BOOST_TIME = 10000;

    public static final int SETSYS_MANAGEMENT_PREDICT  = 1;
    public static final int SETSYS_SPORTS_APK          = 2;
    public static final int SETSYS_FOREGROUND_SPORTS   = 3;
    public static final int SETSYS_MANAGEMENT_PERIODIC = 4;
    public static final int SETSYS_INTERNET_STATUS     = 5;
    public static final int SETSYS_NETD_STATUS         = 6;
    public static final int SETSYS_PREDICT_INFO        = 7;
    public static final int SETSYS_NETD_DUPLICATE_PACKET_LINK = 8;

    public static final int CMD_SET_SCREEN_OFF_STATE       = 11;
    public static final int CMD_SET_NETD_BOOST_UID         = 101;
    public static final int CMD_SET_MD_LOW_LATENCY         = 120;
    public static final int CMD_SET_GPU_LOW_LATENCY        = 126;
    public static final int CMD_SET_WIFI_SMART_PREDICT     = 171;
    public static final int CMD_SET_UX_PREDICT_LOW_LATENCY = 172;
    public static final int CMD_SET_APP_CRASH              = 174;
    public static final int CMD_SET_CERT_PID               = 175;

    public static final int PERF_RES_NET_WIFI_SMART_PREDICT    = 0x02804100;
    public static final int PERF_RES_NET_MD_CRASH_PID          = 0x0280c300;
    public static final int PERF_RES_POWERHAL_SCREEN_OFF_STATE = 0x03400000;

    public static final int MTKPOWER_CMD_GET_RILD_CAP       = 40;
    public static final int MTKPOWER_CMD_GET_POWER_SCN_TYPE = 105;

    public static final int POWER_HIDL_SET_SYS_INFO    = 0;

    public static final int SCREEN_OFF_DISABLE      = 0;
    public static final int SCREEN_OFF_ENABLE       = 1;
    public static final int SCREEN_OFF_WAIT_RESTORE = 2;

    public static final int SCN_USER_HINT       = 2;
    public static final int SCN_PERF_LOCK_HINT  = 3;

    public static final int MAX_NETD_IP_FILTER_COUNT   = 3;

    private static boolean AMS_BOOST_PROCESS_CREATE = true;
    private static boolean AMS_BOOST_PROCESS_CREATE_BOOST = true;
    private static boolean AMS_BOOST_PACK_SWITCH = true;
    private static boolean AMS_BOOST_ACT_SWITCH = true;
    private static boolean EXT_PEAK_PERF_MODE = false;
    private static int graphic_low_lat_now = 0;
    private static int graphic_user_pid = 0;
    private static int graphic_user_hdl = 0;
    private static PowerHalWrapper sInstance = null;
    private static Object lock = new Object();
    public List<ScnList> scnlist = new ArrayList<ScnList>();
    private static String mProcessCreatePack = null;

    /* IMtkPower::hint. It shell be sync with mtkpower_hint.h */
    private static final int MTKPOWER_HINT_BASE                  = 30;
    private static final int MTKPOWER_HINT_PROCESS_CREATE        = 31;
    private static final int MTKPOWER_HINT_PACK_SWITCH           = 32;
    private static final int MTKPOWER_HINT_ACT_SWITCH            = 33;
    private static final int MTKPOWER_HINT_GAME_LAUNCH           = 34;
    private static final int MTKPOWER_HINT_APP_ROTATE            = 35;
    private static final int MTKPOWER_HINT_APP_TOUCH             = 36;
    //private static final int MTKPOWER_HINT_FRAME_UPDATE          = 37;  // no user
    private static final int MTKPOWER_HINT_GAMING                = 38;
    private static final int MTKPOWER_HINT_GALLERY_BOOST         = 39;
    private static final int MTKPOWER_HINT_GALLERY_STEREO_BOOST  = 40;
    private static final int MTKPOWER_HINT_SPORTS                = 41;
    private static final int MTKPOWER_HINT_TEST_MODE             = 42;
    private static final int MTKPOWER_HINT_WFD                   = 43;
    private static final int MTKPOWER_HINT_PMS_INSTALL           = 44;
    private static final int MTKPOWER_HINT_EXT_LAUNCH            = 45;
    private static final int MTKPOWER_HINT_WHITELIST_LAUNCH      = 46;
    private static final int MTKPOWER_HINT_WIPHY_SPEED_DL        = 47;
    private static final int MTKPOWER_HINT_SDN                   = 48;
    private static final int MTKPOWER_HINT_NUM                   = 49;

    private static final int MTKPOWER_HINT_ALWAYS_ENABLE         = 0x0FFFFFFF;

    /* It shell be sync with mtkpower_types.h */
    private static final int MTKPOWER_STATE_PAUSED    = 0;
    private static final int MTKPOWER_STATE_RESUMED   = 1;
    private static final int MTKPOWER_STATE_DESTORYED = 2;
    private static final int MTKPOWER_STATE_DEAD      = 3;
    private static final int MTKPOWER_STATE_STOPPED   = 4;

    public static native int nativeMtkPowerHint(int hint, int data);
    public static native int nativeMtkCusPowerHint(int hint, int data);
    public static native int nativeQuerySysInfo(int cmd, int param);
    public static native int nativeNotifyAppState(String packname, String actname,
                                                          int pid, int status, int uid);
    public static native int nativeScnReg();
    public static native int nativeScnConfig(int hdl, int cmd,
                                             int param_1, int param_2, int param_3, int param_4);
    public static native int nativeScnUnreg(int hdl);
    public static native int nativeScnEnable(int hdl, int timeout);
    public static native int nativeScnDisable(int hdl);
    public static native int nativeScnUltraCfg(int hdl, int ultracmd,
                                             int param_1, int param_2, int param_3, int param_4);
    public static native int nativeSetSysInfo(String data, int type);
    public static native int nativePerfLockAcq(int hdl, int duration, int... list);
    public static native int nativePerfLockRel(int hdl);

    private Lock mLock = new ReentrantLock();

    //private static IMtkPower mMtkPower = null;

    static {
        System.loadLibrary("powerhalwrap_jni");
    }

    public static PowerHalWrapper getInstance() {
        log("PowerHalWrapper.getInstance");
        if (null == sInstance) {
            synchronized (lock) {
                if (null == sInstance) {
                    sInstance = new PowerHalWrapper();
                }
            }
        }
        return sInstance;
    }

    private PowerHalWrapper() {

    }

    private void mtkPowerHint(int hint, int data) {
        nativeMtkPowerHint(hint, data);
    }

    public void mtkCusPowerHint(int hint, int data) {
        nativeMtkCusPowerHint(hint, data);
    }

    public int perfLockAcquire(int handle, int duration, int... list) {
        int new_hdl;
        int pid = Binder.getCallingPid();
        int uid = Binder.getCallingUid();
        new_hdl = nativePerfLockAcq(handle, duration, list);
        if (new_hdl > 0 && new_hdl != handle) {
            mLock.lock();
            ScnList user = new ScnList(new_hdl, pid, uid);
            scnlist.add(user);
            mLock.unlock();
        }
        return new_hdl;
    }

    public void perfLockRelease(int handle) {
        nativePerfLockRel(handle);
    }

    public int scnReg() {
       int handle;
       int pid = Binder.getCallingPid();
       int uid = Binder.getCallingUid();
       handle = nativeScnReg();
       if (handle > 0) {
            mLock.lock();
            ScnList user = new ScnList(handle, pid, uid);
            scnlist.add(user);
            mLock.unlock();
        }
        return handle;
    }

    public int scnConfig(int hdl, int cmd, int param_1,
                                      int param_2, int param_3, int param_4) {
        int pid = Binder.getCallingPid();

        if (cmd == CMD_SET_GPU_LOW_LATENCY) {
            graphic_user_pid = pid;
            graphic_user_hdl = hdl;
            log("<scnConfig> pid:" + pid + " hdl:" + hdl + " cmd:" + cmd);
        }
        nativeScnConfig(hdl, cmd, param_1, param_2, param_3, param_4);
        return 0;
    }

    public int scnUnreg(int hdl) {
        if (hdl == graphic_user_hdl) {
            graphic_user_pid = 0;
            graphic_user_hdl = 0;
        }
        mLock.lock();
        if (null != scnlist && scnlist.size() > 0) {
            Iterator<ScnList> iter = scnlist.iterator();
            while (iter.hasNext()) {
                ScnList item = iter.next();
                if (item.gethandle() == hdl)
                iter.remove();
            }
        }
        mLock.unlock();
        nativeScnUnreg(hdl);
        return 0;
    }

    public int scnEnable(int hdl, int timeout) {
        int pid = Binder.getCallingPid();

        if (pid == graphic_user_pid && hdl == graphic_user_hdl) {
            int propValue = -1;

            // When graphic low latency mode called
            SystemProperties.getInt("debug.graphic.lowlatencypid", propValue);
            if (propValue == -1)
            SystemProperties.set("debug.graphic.lowlatencypid", Integer.toString(pid));
            graphic_low_lat_now = 1;
            log("<scnEnable> pid:" + graphic_user_pid + " hdl:" + graphic_user_hdl + " low_lat:" +
                graphic_low_lat_now);
        }
        nativeScnEnable(hdl, timeout);
        return 0;
    }

    public int scnDisable(int hdl) {
        int pid = Binder.getCallingPid();

        if (graphic_low_lat_now == 1 && pid == graphic_user_pid && hdl == graphic_user_hdl) {
            SystemProperties.set("debug.graphic.lowlatencypid", "-1");
            graphic_low_lat_now = 0;
        }
        log("<scnDisable> pid:" + graphic_user_pid + " hdl:" + graphic_user_hdl + " low_lat" +
            graphic_low_lat_now);
        nativeScnDisable(hdl);
        return 0;
    }

    public int scnUltraCfg(int hdl, int ultracmd, int param_1,
                                             int param_2, int param_3, int param_4) {
        nativeScnUltraCfg(hdl, ultracmd, param_1, param_2, param_3, param_4);
        return 0;
    }

    public void getCpuCap() {
        log("getCpuCap");
    }

    public void getGpuCap() {
        log("mGpuCap");
    }

    public void getGpuRTInfo() {
        log("getGpuCap");
    }

    public void getCpuRTInfo() {
        log("mCpuRTInfo");
    }

    public void UpdateManagementPkt(int type, String packet) {
        logd("<UpdateManagementPkt> type:" + type + ", packet:" + packet);

        switch(type) {

        case SETSYS_MANAGEMENT_PREDICT:
            nativeSetSysInfo(packet, SETSYS_MANAGEMENT_PREDICT);
            break;

        case SETSYS_MANAGEMENT_PERIODIC:
            nativeSetSysInfo(packet, SETSYS_MANAGEMENT_PERIODIC);
            break;

        default:
            break;
        }
    }

    public int setSysInfo(int type, String data) {
        logd("<setSysInfo> type:" + type + " data:" + data);
        return nativeSetSysInfo(data, type);
    }

    public int querySysInfo(int cmd, int param) {
        logd("<querySysInfo> cmd:" + cmd + " param:" + param);
        return nativeQuerySysInfo(cmd, param);
    }

    public void galleryBoostEnable(int timeoutMs) {
        log("<galleryBoostEnable> do boost with " + timeoutMs + "ms");
        nativeMtkPowerHint(MTKPOWER_HINT_GALLERY_BOOST, timeoutMs);
    }

    public void setRotationBoost(int boostTime) {
        log("<setRotation> do boost with " + boostTime + "ms");
        nativeMtkPowerHint(MTKPOWER_HINT_APP_ROTATE, boostTime);
    }

    public void setSpeedDownload(int timeoutMs) {
        log("<setSpeedDownload> do boost with " + timeoutMs + "ms");
        nativeMtkPowerHint(MTKPOWER_HINT_WIPHY_SPEED_DL, timeoutMs);
    }

    public void setWFD(boolean enable) {
        log("<setWFD> enable:" + enable);
        if (enable)
            nativeMtkPowerHint(MTKPOWER_HINT_WFD,
                                            MTKPOWER_HINT_ALWAYS_ENABLE);
        else
            nativeMtkPowerHint(MTKPOWER_HINT_WFD, 0);
    }

    public void setSportsApk(String pack) {
        log("<setSportsApk> pack:" + pack);
        nativeSetSysInfo(pack, SETSYS_SPORTS_APK);
    }

    public void NotifyAppCrash(int pid, int uid, String packageName) {
        int found = 0, type = -1;
        int[] list = {PERF_RES_NET_MD_CRASH_PID, -1};

        mLock.lock();

        if (null != scnlist && scnlist.size() > 0) {
            Iterator<ScnList> iter = scnlist.iterator();
            while (iter.hasNext()) {
                ScnList item = iter.next();
                if (item.getpid() == pid) {
                    if (graphic_low_lat_now == 1 && pid == graphic_user_pid
                        && item.gethandle() == graphic_user_hdl) {
                        graphic_user_pid = 0;
                        graphic_user_hdl = 0;
                        graphic_low_lat_now = 0;
                        log("<NotifyAppCrash> pid:" + graphic_user_pid + " hdl:"
                            + graphic_user_hdl + " low_lat" + graphic_low_lat_now);
                        SystemProperties.set("debug.graphic.lowlatencypid", "-1");
                    }

                    type = nativeQuerySysInfo(MTKPOWER_CMD_GET_POWER_SCN_TYPE, item.gethandle());

                    log("<NotifyAppCrash> handle:" + item.gethandle() + ", type:" + type);
                    list[1] = item.getpid();
                    nativePerfLockAcq(0, 1, list); /* 1ms */

                    if (type == SCN_USER_HINT) {
                        nativeScnDisable(item.gethandle());
                        nativeScnUnreg(item.gethandle());
                    } else if (type == SCN_PERF_LOCK_HINT) {
                        nativePerfLockRel(item.gethandle());
                    }
                    log("<NotifyAppCrash> pid:" + item.getpid() + " uid:"
                            + item.getuid() + " handle:" + item.gethandle());
                    iter.remove();
                    found++;
                    //break; // one process may register more than one handle
                }
            }
        }
        mLock.unlock();
        if (found == 0)
            nativeNotifyAppState(packageName, packageName, pid, MtkActState.STATE_DEAD, uid);
    }

    public boolean getRildCap(int uid) {
        if (nativeQuerySysInfo(MTKPOWER_CMD_GET_RILD_CAP, uid) == 1)
            return true;
        else
            return false;
    }

    public void setInstallationBoost(boolean enable) {
        log("<setInstallationBoost> enable:" + enable);
        if (enable)
            nativeMtkPowerHint(MTKPOWER_HINT_PMS_INSTALL, 15000);
        else
            nativeMtkPowerHint(MTKPOWER_HINT_PMS_INSTALL, 0);
    }


    /* AMS event handler */
    public void amsBoostResume(String lastResumedPackageName, String nextResumedPackageName) {
        logd("<amsBoostResume> last:" + lastResumedPackageName + ", next:"
            + nextResumedPackageName);

        Trace.asyncTraceBegin(Trace.TRACE_TAG_ACTIVITY_MANAGER, "amPerfBoost", 0);
        /*--make sure not re-entry ext_launch--*/
        nativeMtkPowerHint(MTKPOWER_HINT_EXT_LAUNCH, 0);

        if (lastResumedPackageName == null ||
            !lastResumedPackageName.equalsIgnoreCase(nextResumedPackageName)) {
            AMS_BOOST_PACK_SWITCH = true;

            /*--main package switch--*/
            nativeMtkPowerHint(MTKPOWER_HINT_PACK_SWITCH,
                AMS_BOOST_TIME);
        }
        else {
            AMS_BOOST_ACT_SWITCH = true;

            /*--main activity switch--*/
            nativeMtkPowerHint(MTKPOWER_HINT_ACT_SWITCH,
                AMS_BOOST_TIME);
        }
    }

    public void amsBoostProcessCreate(String hostingType, String packageName) {
        if(hostingType != null && hostingType.compareTo("activity") == 0) {
            logd("amsBoostProcessCreate package:" + packageName);

            Trace.asyncTraceBegin(Trace.TRACE_TAG_ACTIVITY_MANAGER, "amPerfBoost", 0);
            AMS_BOOST_PROCESS_CREATE = true;
            AMS_BOOST_PROCESS_CREATE_BOOST = true;
            mProcessCreatePack = packageName;

            /*--make sure not re-entry ext_launch--*/
            nativeMtkPowerHint(MTKPOWER_HINT_EXT_LAUNCH, 0);
            /*--main process create--*/
            nativeMtkPowerHint(MTKPOWER_HINT_PROCESS_CREATE, AMS_BOOST_TIME);
        }
    }

    public void amsBoostStop() {
        logd("amsBoostStop AMS_BOOST_PACK_SWITCH:" + AMS_BOOST_PACK_SWITCH +
            ", AMS_BOOST_ACT_SWITCH:" + AMS_BOOST_ACT_SWITCH + ", AMS_BOOST_PROCESS_CREATE:"
            + AMS_BOOST_PROCESS_CREATE);

        if (AMS_BOOST_PACK_SWITCH) {
            AMS_BOOST_PACK_SWITCH = false;
           nativeMtkPowerHint(MTKPOWER_HINT_PACK_SWITCH, 0);
        }

        if (AMS_BOOST_ACT_SWITCH) {
            AMS_BOOST_ACT_SWITCH = false;
            nativeMtkPowerHint(MTKPOWER_HINT_ACT_SWITCH, 0);
        }

        if (AMS_BOOST_PROCESS_CREATE) {
            AMS_BOOST_PROCESS_CREATE = false;
            nativeMtkPowerHint(MTKPOWER_HINT_PROCESS_CREATE, 0);
        }

        Trace.asyncTraceEnd(Trace.TRACE_TAG_ACTIVITY_MANAGER, "amPerfBoost", 0);
    }

    public void amsBoostNotify(int pid, String activityName, String packageName, int uid) {
        logd("amsBoostNotify pid:" + pid + ",activity:" + activityName + ", package:"
            + packageName + ", mProcessCreatePack" + mProcessCreatePack);
        nativeNotifyAppState(packageName, activityName,
                                    pid, MTKPOWER_STATE_RESUMED, uid);

        if (!packageName.equalsIgnoreCase(mProcessCreatePack)) {
            logd("amsBoostNotify AMS_BOOST_PROCESS_CREATE_BOOST:"
                + AMS_BOOST_PROCESS_CREATE_BOOST);
            if(AMS_BOOST_PROCESS_CREATE_BOOST) {
                // use 1ms timeout to avoid hold time
                nativeMtkPowerHint(MTKPOWER_HINT_PROCESS_CREATE, 1);
            }
            AMS_BOOST_PROCESS_CREATE_BOOST = false;
        }
    }

    private static void log(String info) {
        Log.i(TAG, info + " ");
    }

    private static void logd(String info) {
        Log.d(TAG, info + " ");
    }

    private static void loge(String info) {
        Log.e(TAG, "ERR: " + info + " ");
    }
}

