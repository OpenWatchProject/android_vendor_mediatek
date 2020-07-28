/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.memory;

import android.app.ActivityManager;
import android.content.ComponentCallbacks2;
import android.content.Context;
import android.content.res.Configuration;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.portability.memory.MemoryInfoManager;

/**
 * Default implementation of the {@link IMemoryManager}.
 */

public class MemoryManagerImpl implements IMemoryManager, ComponentCallbacks2 {
    // only 40% of max memory is allowed to be used by normal continuous shots.
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            MemoryManagerImpl.class.getSimpleName());
    private static final float DVM_SLOWDOWN_THRESHOLD = 0.4f;
    private static final float DVM_STOP_THRESHOLD = 0.1f;
    private static final long CSHOT_ADD_SLOWDOWN_THRESHOLD = 100;
    private static final long SYSTEM_MINI_SLOWDOWN_THRESHOLD = 36;
    private static final int LOW_SUITABLE_SPEED_FPS = 1;
    private static final long BYTES_KILOBYTE = 1024;
    private static final int LOW_MEMORY_DEVICE = 512;
    private static final long LOW_MEMORY_DIVISOR = 2;
    private static final long SYSTEM_STOP_DIVISOR = 2;

    private final long mMaxDvmMemory;
    private final long mDvmSlowdownThreshold;
    private final long mDvmStopThreshold;
    private final long mSystemSlowdownThreshold;
    private final long mSystemStopThreshold;
    private final long mMiniMemFreeMb;
    private long mLeftStorage;
    private long mUsedStorage;
    private long mPengdingSize;
    private long mStartTime;
    private int mCount;
    private int mSuitableSpeed;
    private MemoryInfoManager mMemoryInfoManager;
    private IMemoryListener mListener;
    private Context mContext;
    private Runtime mRuntime = Runtime.getRuntime();

    /**
     * init memory manager.
     * @param context the activity context.
     */
    public MemoryManagerImpl(Context context) {
        mContext = context;
        mMemoryInfoManager = new MemoryInfoManager();
        // initial the threshold of camera process DVM memory check.
        mMaxDvmMemory = mRuntime.maxMemory();
        mDvmSlowdownThreshold = (long) (DVM_SLOWDOWN_THRESHOLD * mMaxDvmMemory);
        mDvmStopThreshold = (long) (DVM_STOP_THRESHOLD * mMaxDvmMemory);

        //the the threshold of system memory check.
        MemoryInfoManager.MemoryDetailInfo sysMemoryInfo = new MemoryInfoManager.MemoryDetailInfo();
        ((ActivityManager) context.getSystemService(
                Context.ACTIVITY_SERVICE)).getMemoryInfo(sysMemoryInfo);
        mMiniMemFreeMb = sysMemoryInfo
                .getForgroundAppThreshold() / BYTES_KILOBYTE / BYTES_KILOBYTE;
        LogHelper.d(TAG, "mMiniMemFreeMb = " + mMiniMemFreeMb);

        // System memory check;
        // mMiniMemFreeMb < 36M, mimMemFree + 25M
        // system memory > 512MB project
        // stop condition: (memFree + Cached) < mimMemFree + 50M;
        // Slow down condition:(memFree + Cached) < mimMemFree + 100M;
        // system memory <= 512MB project
        // stop condition: (memFree + Cached) < mimMemFree + 25M;
        // Slow down condition: (memFree + Cached) < mimMemFree + 50M;
        long divisor = 1;
        if (mMiniMemFreeMb <= SYSTEM_MINI_SLOWDOWN_THRESHOLD) {
            divisor = LOW_MEMORY_DIVISOR * LOW_MEMORY_DIVISOR;
        } else if (mMaxDvmMemory >= LOW_MEMORY_DEVICE) {
            divisor = LOW_MEMORY_DIVISOR / LOW_MEMORY_DIVISOR;
        } else {
            divisor = LOW_MEMORY_DIVISOR;
        }
        mSystemSlowdownThreshold = CSHOT_ADD_SLOWDOWN_THRESHOLD / divisor;
        mSystemStopThreshold = mSystemSlowdownThreshold / SYSTEM_STOP_DIVISOR;
        LogHelper.d(TAG, "MemoryManagerImpl, mDvmSlowdownThreshold: " + mDvmSlowdownThreshold
                + ", mDvmStopThreshold: " + mDvmStopThreshold + ", mSystemSlowdownThreshold: "
                + mSystemSlowdownThreshold + ", mSystemStopThreshold: " + mSystemStopThreshold);
    }

    @Override
    public void onConfigurationChanged(Configuration configuration) {

    }

    @Override
    public void onLowMemory() {
        LogHelper.i(TAG, "onLowMemory");
        onMemoryStateChanged(IMemoryManager.MemoryAction.STOP);
    }

    @Override
    public void onTrimMemory(int i) {
        switch (i) {
            case ComponentCallbacks2.TRIM_MEMORY_RUNNING_CRITICAL:
            case ComponentCallbacks2.TRIM_MEMORY_BACKGROUND:
                //whether it need to adjust speed.
                LogHelper.i(TAG, "onTrimMemory, info: " + i);
                doSystemMemoryCheckAction(
                        mSystemSlowdownThreshold, IMemoryManager.MemoryAction.ADJUST_SPEED);
                break;

            case ComponentCallbacks2.TRIM_MEMORY_COMPLETE:
            case ComponentCallbacks2.TRIM_MEMORY_MODERATE:
                //whether it need to stop.
                LogHelper.i(TAG, "onTrimMemory, info: " + i);
                doSystemMemoryCheckAction(
                        mSystemStopThreshold, IMemoryManager.MemoryAction.STOP);
                break;

            default:
                break;
        }
    }

    @Override
    public void addListener(IMemoryListener listener) {
        // initial the low memory callback.
        if (mContext != null) {
            mContext.registerComponentCallbacks(this);
        }
        mListener = listener;
    }

    @Override
    public void removeListener(IMemoryListener listener) {
        if (mContext != null) {
            mContext.unregisterComponentCallbacks(this);
        }
        mListener = null;
    }

    /**
     * Initial the status of memory manager.
     * @param leftStorage The left size of storage.
     */
    public void initStateForCapture(long leftStorage) {
        onMemoryStateChanged(IMemoryManager.MemoryAction.NORMAL);
        mLeftStorage = leftStorage;
        mUsedStorage = 0;
        mPengdingSize = 0;
        mCount = 0;
    }

    /**
     * it will be called when continuous shot started.
     */
    public void initStartTime() {
        mStartTime = System.currentTimeMillis();
    }

    /**
     * get the memory action that will check DVM memory and system memory,
     * and calculate the suitable capture speed in advance.
     * @param pictureSize the last captured image size;
     * @param pendingSize the size of all pending images;
     */
    public void checkContinuousShotMemoryAction(long pictureSize, long pendingSize) {
        if (mListener == null) {
            return;
        }
        mCount++;
        mUsedStorage += pictureSize;
        mPengdingSize = pendingSize;
        long timeDuration = System.currentTimeMillis() - mStartTime;
        long captureSpeed = mCount * BYTES_KILOBYTE / timeDuration;
        long saveSpeed = (mUsedStorage - mPengdingSize) / timeDuration / BYTES_KILOBYTE;
        LogHelper.d(TAG, "[checkContinuousShotMemoryAction]Capture speed=" + captureSpeed +
                " fps, Save speed=" + saveSpeed + " MB/s");

        // remaining storage check.
        if (mUsedStorage >= mLeftStorage) {
            LogHelper.d(TAG, "checkContinuousShotMemoryAction, usedMemory > " +
                    "availableMemory,stop! used: " + mUsedStorage + ", available: " + mLeftStorage);
            onMemoryStateChanged(IMemoryManager.MemoryAction.STOP);
            return;
        }
        mSuitableSpeed = (int) ((mUsedStorage - mPengdingSize) * mCount * BYTES_KILOBYTE
                / timeDuration / mUsedStorage);

        if (doSystemMemoryCheckAction(
                mSystemStopThreshold, IMemoryManager.MemoryAction.STOP)) {
            //already stop, no need go on check.
            return;
        } else if (doSystemMemoryCheckAction(
                mSystemSlowdownThreshold, IMemoryManager.MemoryAction.ADJUST_SPEED)) {
            return;
        }

        // application pending Jpeg data size check;
        if (mPengdingSize >= mDvmSlowdownThreshold) {
            LogHelper.i(TAG, "checkContinuousShotMemoryAction, DvmSlowdownThreshold reached, " +
                    "mPengdingSize = " + mPengdingSize);
            onMemoryStateChanged(IMemoryManager.MemoryAction.ADJUST_SPEED);
            return;
        }
        // Camera process DVM memory check;
        long usedMemory = mRuntime.totalMemory() - mRuntime.freeMemory();
        LogHelper.d(TAG, "checkContinuousShotMemoryAction, process total memory: " +
                mRuntime.totalMemory()
                + ", real used memory: " + usedMemory);
        long realfree = mMaxDvmMemory - usedMemory;
        if (realfree <= mDvmStopThreshold) {
            LogHelper.i(TAG, "checkContinuousShotMemoryAction, DvmStopThreshold reached ");
            onMemoryStateChanged(IMemoryManager.MemoryAction.STOP);
            return;
        }
    }

    /**
     * get the memory action that will check DVM memory and system memory,
     * and calculate the suitable capture speed in advance.
     * @param pictureSize the last captured image size (Byte);
     */
    public void checkOneShotMemoryAction(long pictureSize) {
        if (mListener == null) {
            return;
        }
        LogHelper.d(TAG, "checkOneShotMemoryAction, pictureSize: " + pictureSize);
        //system memory check;
        if (doSystemMemoryCheckAction(
                mSystemStopThreshold + toMb(pictureSize), IMemoryManager.MemoryAction.STOP)) {
            return;
        }

        // Camera process DVM memory check;
        long usedMemory = mRuntime.totalMemory() - mRuntime.freeMemory();
        LogHelper.d(TAG, "checkOneShotMemoryAction, process total memory: " + mRuntime.totalMemory()
                + ", real used memory: " + usedMemory);
        long realfree = mMaxDvmMemory - usedMemory;
        if (realfree <= mDvmStopThreshold + pictureSize) {
            LogHelper.i(TAG, "checkOneShotMemoryAction, DvmStopThreshold reached ");
            onMemoryStateChanged(IMemoryManager.MemoryAction.STOP);
            return;
        }
        onMemoryStateChanged(IMemoryManager.MemoryAction.NORMAL);
    }

    /**
     * get suitable speed for continuous shot.
     * @return the suitable speed.
     */
    public int getSuitableSpeed() {
        if (mSuitableSpeed < LOW_SUITABLE_SPEED_FPS) {
            mSuitableSpeed = LOW_SUITABLE_SPEED_FPS;
        }
        return mSuitableSpeed;
    }

    private boolean doSystemMemoryCheckAction(long threshold, MemoryAction state) {
        long memFreeDiffMb = getSystemFreeMemory() - mMiniMemFreeMb;
        if (memFreeDiffMb < threshold) {
            LogHelper.d(TAG, "doSystemMemoryCheckAction, info: " + memFreeDiffMb +
                    " < " + threshold + ", " + state);
            onMemoryStateChanged(state);
            return true;
        }
        return false;
    }

    private long getSystemFreeMemory() {
        mMemoryInfoManager.readMemInfo();
        long cached =  mMemoryInfoManager.getCachedSizeKb() / BYTES_KILOBYTE;
        long free = mMemoryInfoManager.getFreeSizeKb() / BYTES_KILOBYTE;
        //LogHelper.d(TAG, "getSystemFreeMemory, cached: " + cached + ", free: " + free);
        long memFreeMb = cached + free;
        return memFreeMb;
    }

    private void onMemoryStateChanged(MemoryAction state) {
        if (mListener != null) {
            mListener.onMemoryStateChanged(state);
        }
    }

    private long toMb(long in) {
        return in / BYTES_KILOBYTE / BYTES_KILOBYTE;
    }
}
