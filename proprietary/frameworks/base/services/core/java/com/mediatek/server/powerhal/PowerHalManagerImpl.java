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

package com.mediatek.server.powerhal;

//import android.content.Context;
//import android.os.IBinder;
//import android.os.RemoteException;
//import android.os.ServiceManager;
import android.util.Log;
/*
import com.mediatek.am.AMEventHookData;
import com.mediatek.am.AMEventHookData.AfterActivityDestroyed;
import com.mediatek.am.AMEventHookData.AfterActivityPaused;
import com.mediatek.am.AMEventHookData.AfterActivityResumed;
import com.mediatek.am.AMEventHookData.AfterActivityStopped;
import com.mediatek.am.AMEventHookData.BeforeActivitySwitch;
import com.mediatek.am.AMEventHookData.StartProcess;
*/
import com.mediatek.powerhalwrapper.PowerHalWrapper;
import com.mediatek.boostframework.Performance;

import java.lang.reflect.Method;

/// MTK power
import vendor.mediatek.hardware.power.V2_0.*;

public class PowerHalManagerImpl extends PowerHalManager {
    private static final String TAG = "PowerHalManagerImpl";

    public boolean mIsRotationBoostEnable = false;

    private static PowerHalWrapper mPowerHalWrap = null;
    private static Performance mPerformance = new Performance();

    public PowerHalManagerImpl() {
        mPowerHalWrap = PowerHalWrapper.getInstance();
    }

    @Override
    public void setRotationBoost(boolean enable) {
        int boostTime = 0;
        // boostTime > 0 means enable boost
        if (enable && !mIsRotationBoostEnable) {
            boostTime = PowerHalManager.ROTATE_BOOST_TIME;
        } else if (!enable && mIsRotationBoostEnable) {
            boostTime = 0;
        }

        //Log.e(TAG, "<setRotationBoost> do boost with " + enable);
        mPowerHalWrap.setRotationBoost(boostTime);
        mIsRotationBoostEnable = enable;
    }

    @Override
    public void setWFD(boolean enable) {
        //Log.e(TAG, "<setWFD> enable:" + enable);
        mPowerHalWrap.setWFD(enable);
    }

    public void perfLockAcquire(int duration, int... list) {
        //Log.e(TAG, "<perfLockAcquire> duration:" + duration);
        /*for (int i = 0; i < list.length; i++) {
            Log.e(TAG, "<perfLockAcquire> list:" + list[i]);
        }*/
        mPerformance.perfLockAcquire(duration, list);
    }

    public void perfLockRelease() {
        //Log.e(TAG, "<perfLockRelease>");
        mPerformance.perfLockRelease();
    }

    public void perfLockRelease(int handle) {
        //Log.e(TAG, "<perfLockRelease> handle:" + handle);
        mPerformance.perfLockRelease(handle);
    }

    @Override
    public void NotifyAppCrash(int pid, int uid, String packageName) {
        //Log.e(TAG, "<NotifyAppCrash> pid:" + pid + "packageName:" + packageName);
        mPowerHalWrap.NotifyAppCrash(pid, uid, packageName);
    }

    @Override
    public void setInstallationBoost(boolean enable) {
        //Log.e(TAG, "<setInstallationBoost> enable:" + enable);
        mPowerHalWrap.setInstallationBoost(enable);
    }

    @Override
    public void setSpeedDownload(int timeoutMs) {
        //Log.e(TAG, "setSpeedDownload");
        mPowerHalWrap.setSpeedDownload(timeoutMs);
    }

    public void amsBoostResume(String lastResumedPackageName, String nextResumedPackageName) {
        //Log.e(TAG, "<amsBoostResume> last:" + lastResumedPackageName +
        //           ", next:" + nextResumedPackageName);
        mPowerHalWrap.amsBoostResume(
                            lastResumedPackageName, nextResumedPackageName);
    }

    public void amsBoostNotify(int pid, String activityName, String packageName, int uid) {
        //Log.e(TAG, "amsBoostNotify pid:" + pid +
        //           ", activity:" + activityName + ", package:" + packageName);
        mPowerHalWrap.amsBoostNotify(pid, activityName, packageName, uid);
    }

    public void amsBoostProcessCreate(String hostingType, String packageName) {
        //Log.e(TAG, "amsBoostProcessCreate hostingType:" + hostingType +
        //           ", package:" + packageName);
        mPowerHalWrap.amsBoostProcessCreate(hostingType, packageName);
    }

    public void amsBoostStop() {
        //Log.e(TAG, "amsBoostStop");
        mPowerHalWrap.amsBoostStop();
    }
}
