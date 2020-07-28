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
package com.mediatek.powerhalservice;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemProperties;
import android.os.UEventObserver;
import android.os.UserHandle;
import android.os.Binder;
import android.util.Log;
import com.android.server.SystemService;
import com.mediatek.powerhalmgr.IPowerHalMgr;
import com.mediatek.powerhalwrapper.PowerHalWrapper;
// M: DPP @{
import android.os.IRemoteCallback;
import com.mediatek.powerhalmgr.DupLinkInfo;
// @}

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import vendor.mediatek.hardware.power.V2_0.*;
import com.mediatek.boostframework.Performance;

public class PowerHalMgrServiceImpl extends IPowerHalMgr.Stub {
    private final String TAG = "PowerHalMgrServiceImpl";
    private static PowerHalWrapper mPowerHalWrap = null;
    private static int mhandle = 0;
    private static Performance mPerformance = new Performance();

    // M: DPP @{
    private PowerHalWifiMonitor mPowerHalWifiMonitor = null;
    // @}

/*
    static {
        System.loadLibrary("powerhalmgrserv_jni");
    }
*/
    public PowerHalMgrServiceImpl(Context context){
        mPowerHalWrap = PowerHalWrapper.getInstance();
        // M: DataStall @{
        mPowerHalWifiMonitor = new PowerHalWifiMonitor(context);
        // @}
    }

    public int scnReg() {
        return mPowerHalWrap.scnReg();
    }

    public void scnConfig(int handle, int cmd, int param_1,
                                          int param_2, int param_3, int param_4) {
        mPowerHalWrap.scnConfig(handle, cmd, param_1, param_2, param_3, param_4);
    }

    public void scnUnreg(int handle) {
        mPowerHalWrap.scnUnreg(handle);
    }

    public void scnEnable(int handle, int timeout) {
        mPowerHalWrap.scnEnable(handle, timeout);
    }

    public void scnDisable(int handle) {
        mPowerHalWrap.scnDisable(handle);
    }

    public void scnUltraCfg(int handle, int ultracmd, int param_1,
                                                 int param_2, int param_3, int param_4) {
        mPowerHalWrap.scnUltraCfg(handle, ultracmd, param_1, param_2, param_3, param_4);
    }

    public void mtkCusPowerHint(int hint, int data) {
        mPowerHalWrap.mtkCusPowerHint(hint, data);
    }

    public void getCpuCap() {
        mPowerHalWrap.getCpuCap();
    }

    public void getGpuCap() {
        mPowerHalWrap.getGpuCap();
    }

    public void getGpuRTInfo() {
        mPowerHalWrap.getGpuRTInfo();
    }

    public void getCpuRTInfo() {
        mPowerHalWrap.getCpuRTInfo();
    }

    public void UpdateManagementPkt(int type, String packet) {
        mPowerHalWrap.UpdateManagementPkt(type, packet);
    }

    public void setForegroundSports() {
        String temp = "";
        mPowerHalWrap.setSysInfo(PowerHalWrapper.SETSYS_FOREGROUND_SPORTS, temp);
    }

    public void setSysInfo(int type, String data) {
        mPowerHalWrap.setSysInfo(type, data);
    }

    // M: DPP @{
    private boolean checkDppPermission() {
        int uid = Binder.getCallingUid();
        if (mPowerHalWrap.getRildCap(uid) == false) {
            logd("checkDppPermission(), no permission");
            return false;
        } else {
            return true;
        }
    }

    public boolean startDuplicatePacketPrediction() {
        if (checkDppPermission() == false)
            return false;
        mPowerHalWifiMonitor.startDuplicatePacketPrediction();
        return true;
    }

    public boolean stopDuplicatePacketPrediction() {
        if (checkDppPermission() == false)
            return false;
        mPowerHalWifiMonitor.stopDuplicatePacketPrediction();
        return true;
    }

    public boolean isDupPacketPredictionStarted() {
        return mPowerHalWifiMonitor.isDupPacketPredictionStarted();
    }

    public boolean registerDuplicatePacketPredictionEvent(IRemoteCallback listener) {
        if (checkDppPermission() == false)
            return false;
        return mPowerHalWifiMonitor.registerDuplicatePacketPredictionEvent(listener);
    }

    public boolean unregisterDuplicatePacketPredictionEvent(IRemoteCallback listener) {
        if (checkDppPermission() == false)
            return false;
        return mPowerHalWifiMonitor.unregisterDuplicatePacketPredictionEvent(listener);
    }

    public boolean updateMultiDuplicatePacketLink(DupLinkInfo[] linkList) {
        int ret = -1;
        logd("[updateMultiDuplicatePacketLink] len:" + linkList.length + " ");

        if (linkList.length > PowerHalWrapper.MAX_NETD_IP_FILTER_COUNT) {
            return false;
        }

        /* check permission */
        if (checkDppPermission() == false)
            return false;

        /* delete current ip link */
        if (linkList == null || linkList.length <= 0) {
            return false;
        }

        /* format: MULTI count IP_pair_1 IP_pair_2 IP_pair_3 */
        StringBuilder sb = new StringBuilder();
        sb.append("MULTI\t").append(String.valueOf(linkList.length));

        for (int i=0; i<linkList.length; i++) {
            DupLinkInfo info = linkList[i];

            if (PowerHalAddressUtils.isIpPairValid(info.getSrcIp(), info.getDstIp(),
                info.getSrcPort(), info.getDstPort()) == false) {
                return false;
            }

            /* -1 => null */
            String strSrcPort = (info.getSrcPort() == -1) ? "none" :
                Integer.toString(info.getSrcPort());
            String dstSrcPort = (info.getDstPort() == -1) ? "none" :
                Integer.toString(info.getDstPort());
            String linkInfo = info.getSrcIp() + " " + strSrcPort + " "
                + info.getDstIp() + " " + dstSrcPort + " ";

            if(info.getProto() == 1) {
                linkInfo += "TCP";
            } else if(info.getProto() == 2) {
                linkInfo += "UDP";
            } else {
                logd("[updateMultiDuplicatePacketLink] unknown protocol:" + info.getProto());
                return false;
            }
            sb.append("\t").append(linkInfo);
        }
        String multiLinkInfo = sb.toString();
        ret =  mPowerHalWrap.setSysInfo(PowerHalWrapper.SETSYS_NETD_DUPLICATE_PACKET_LINK,
            multiLinkInfo);

        if (ret == 0)
            return true;
        else
            return false;
    }
    // @}

    public void setPredictInfo(String pack_name, int uid) {
        String data = pack_name + " " + uid;
        logd("setPredictInfo:" + data);
        mPowerHalWrap.setSysInfo(PowerHalWrapper.SETSYS_PREDICT_INFO, data);
    }

    public int perfLockAcquire(int handle, int duration, int[] list) {
        int i;

        if (list.length % 2 != 0)
            return -1;

        logd("perfLockAcquire hdl:" + handle + " dur:" + duration + " len:" + list.length);
        for (i=0; i<list.length; i+=2) {
            logd("perfLockAcquire " + i + " id:" +
                Integer.toHexString(list[i]) + " value:" + list[i+1]);
        }
        return mPowerHalWrap.perfLockAcquire(handle, duration, list);
    }

    public void perfLockRelease(int handle) {
        mPowerHalWrap.perfLockRelease(handle);
    }

    public int querySysInfo(int cmd, int param) {
         return mPowerHalWrap.querySysInfo(cmd, param);
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
