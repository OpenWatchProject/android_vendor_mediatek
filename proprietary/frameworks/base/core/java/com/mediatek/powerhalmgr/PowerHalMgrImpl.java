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

package com.mediatek.powerhalmgr;

import android.content.Context;
import android.os.IBinder;
import android.os.IRemoteCallback;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.os.Trace;

import com.mediatek.powerhalmgr.DupLinkInfo;
import java.util.List;

public class PowerHalMgrImpl extends PowerHalMgr {

    private static final String TAG = "PowerHalMgrImpl";

    private static PowerHalMgrImpl sInstance = null;
    private IPowerHalMgr sService = null;
    private static Object lock = new Object();
    private Context mContext;

    private int inited = 0;

    private int setTid = 0;
    private long mPreviousTime = 0;

    public static native int nativeGetPid();
    public static native int nativeGetTid();

    private void init() {
        if (inited == 0) {
            IBinder b = ServiceManager.checkService("power_hal_mgr_service");
            if (b != null) {
                sService = IPowerHalMgr.Stub.asInterface(b);
                if (sService != null)
                    inited = 1;
                else
                    loge("ERR: getService() sService is still null..");
            }
        }
    }

    public static PowerHalMgrImpl getInstance() {
        if (null == sInstance) {
            synchronized (lock) {
                if (null == sInstance) {
                    sInstance = new PowerHalMgrImpl();
                }
            }
        }
        return sInstance;
    }

    public PowerHalMgrImpl() {

    }

    public int scnReg() {
        int handle = -1;
        try {
            init();
            //int pid = nativeGetPid();
            //int tid = nativeGetTid();
            if (sService != null)
                handle = sService.scnReg();
        } catch (RemoteException e) {
            loge("ERR: RemoteException in scnReg:" + e);
        }
        //log("[scnReg] - handle:"+handle);
        return handle;
    }

    public void scnConfig(int handle, int cmd, int param_1, int param_2, int param_3, int param_4) {
        //log("[scnConfig] - "+handle+", "+cmd+", "+param_1+", "+param_2+", "+param_3+", "+param_4);
        try {
            init();
            if (sService != null)
                sService.scnConfig(handle, cmd, param_1, param_2, param_3, param_4);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in scnConfig:" + e);
        }
    }

    public void scnUnreg(int handle) {
        //log("[scnUnreg] - "+handle);
        try {
            init();
            if (sService != null)
                sService.scnUnreg(handle);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in scnUnreg:" + e);
        }
    }

    public void scnEnable(int handle, int timeout) {
        //log("[scnEnable] - "+handle+", "+timeout);
        try {
            init();
            if (sService != null)
                sService.scnEnable(handle, timeout);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in scnEnable:" + e);
        }
    }

    public void scnDisable(int handle) {

        try {
            init();
            if (sService != null)
                sService.scnDisable(handle);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in scnDisable:" + e);
        }
    }

    public void scnUltraCfg(int handle, int ultracmd, int param_1,
                                                   int param_2, int param_3, int param_4) {

        try {
            init();
            if (sService != null)
                sService.scnUltraCfg(handle, ultracmd, param_1, param_2, param_3, param_4);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in scnConfig:" + e);
        }
    }

    public void mtkCusPowerHint(int hint, int data) {

        try {
            init();
            if (sService != null)
                sService.mtkCusPowerHint(hint, data);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in mtkCusPowerHint:" + e);
        }
    }

    public void getCpuCap() {
        log("getCpuCap");
    }

    public void getGpuCap() {
        log("getGpuCap");
    }

    public void getGpuRTInfo() {
        log("getGpuRTInfo");
    }

    public void getCpuRTInfo() {
        log("getCpuRTInfo");
    }

    public void UpdateManagementPkt(int type, String packet) {
        try {
            init();
            if (sService != null)
                sService.UpdateManagementPkt(type, packet);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in UpdateManagementPkt:" + e);
        }
    }

    public void setForegroundSports() {
        try {
            init();
            if (sService != null)
                sService.setForegroundSports();
        } catch (RemoteException e) {
            loge("ERR: RemoteException in setForegroundSports:" + e);
        }
    }

    public void setSysInfo(int type, String data) {
        try {
            init();
            if (sService != null)
                sService.setSysInfo(type, data);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in setSysInfo:" + e);
        }
    }

    // M: DPP @{
    public boolean startDuplicatePacketPrediction() {
        boolean status = false;
        logd("startDuplicatePacketPrediction()");
        try {
            init();
            if (sService != null)
                status = sService.startDuplicatePacketPrediction();
        } catch (RemoteException e) {
            loge("ERR: RemoteException in startDuplicatePacketPrediction:" + e);
        }
        return status;
    }

    public boolean stopDuplicatePacketPrediction() {
        boolean status = false;
        logd("stopDuplicatePacketPrediction()");
        try {
            init();
            if (sService != null)
                status = sService.stopDuplicatePacketPrediction();
        } catch (RemoteException e) {
            loge("ERR: RemoteException in stopDuplicatePacketPrediction:" + e);
        }
        return status;
    }

    public boolean isDupPacketPredictionStarted() {
        try {
            init();
            if (sService != null) {
                boolean enable = sService.isDupPacketPredictionStarted();
                logd("isDupPacketPredictionStarted() enable:" + enable);
                return enable;
            }
        } catch (RemoteException e) {
            loge("ERR: RemoteException in isDupPacketPredictionStarted:" + e);
        }
        return false;
    }

    public boolean registerDuplicatePacketPredictionEvent(IRemoteCallback listener) {
        boolean status = false;
        logd("registerDuplicatePacketPredictionEvent() " + listener.getClass().toString());
        try {
            init();
            if (sService != null)
                status = sService.registerDuplicatePacketPredictionEvent(listener);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in registerDuplicatePacketPredictionEvent:" + e);
        }
        return status;
    }

    public boolean unregisterDuplicatePacketPredictionEvent(IRemoteCallback listener) {
        boolean status = false;
        logd("unregisterDuplicatePacketPredictionEvent() " + listener.getClass().toString());
        try {
            init();
            if (sService != null)
                status = sService.unregisterDuplicatePacketPredictionEvent(listener);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in unregisterDuplicatePacketPredictionEvent:" + e);
        }
        return status;
    }

    public boolean updateMultiDuplicatePacketLink(DupLinkInfo[] linkList) {
        boolean status = false;

        try {
            init();
            if (sService != null)
                status = sService.updateMultiDuplicatePacketLink(linkList);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in updateMultiDuplicatePacketLink:" + e);
        }
        return status;
    }
    // @}

    public void setPredictInfo(String pack_name, int uid) {
        try {
            init();
            if (sService != null)
                sService.setPredictInfo(pack_name, uid);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in setPredictInfo:" + e);
        }
    }

    public int perfLockAcquire(int handle, int duration, int[] list) {
        try {
            init();
            if (sService != null)
                handle = sService.perfLockAcquire(handle, duration, list);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in perfLockAcquire:" + e);
        }
        return handle;
    }

    public void perfLockRelease(int handle) {
        try {
            init();
            if (sService != null)
                sService.perfLockRelease(handle);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in perfLockRelease:" + e);
        }
    }

    public int querySysInfo(int cmd, int param) {
        int value = -1;

        try {
            init();
            if (sService != null)
                value = sService.querySysInfo(cmd, param);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in perfLockAcquire:" + e);
        }
        return value;
    }

    private void log(String info) {
        Log.i(TAG, info + " ");
    }

    private void logd(String info) {
        Log.d(TAG, info + " ");
    }

    private void loge(String info) {
        Log.e(TAG, "ERR: " + info + " ");
    }
}

