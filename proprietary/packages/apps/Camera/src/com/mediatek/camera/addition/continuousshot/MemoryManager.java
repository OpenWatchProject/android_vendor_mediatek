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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.camera.addition.continuousshot;

import android.app.ActivityManager;
import android.content.ComponentCallbacks2;
import android.content.Context;
import android.content.res.Configuration;
import android.os.Debug;

import com.android.internal.util.MemInfoReader;
import com.mediatek.camera.util.Log;

/**
 * MemoryManager is designed for continuous shot,
 * who monitor the system and DVM memory status,
 * and provide the action to slow down or stop continuous shot.
 */
public class MemoryManager implements ComponentCallbacks2 {
    private static final String TAG = "MemoryManager";

    // Let's signal only 40% of max memory is allowed to be used by normal
    // continuous shots.
    private static final float DVM_SLOWDOWN_THRESHOLD = 0.4f;
    private static final float DVM_STOP_THRESHOLD = 0.1f;
    private static final long SYSTEM_SLOWDOWN_THRESHOLD = 100;
    private static final int LOW_SUITABLE_SPEED_FPS = 1;
    private static final long BYTES_IN_KILOBYTE = 1024;
    private static final int LOW_MEMORY_DEVICE = 512;
    private static final long LOW_MEMORY_DIVISOR = 2;
    private static final long SYSTEM_STOP_DIVISOR = 2;

    private final long mMaxDvmMemory;
    private final long mDvmSlowdownThreshold;
    private final long mDvmStopThreshold;
    private final long mSystemSlowdownThreshold;
    private final long mSystemStopThreshold;
    private final long mMiniMemFreeMb;
    private final MemInfoReader mMemInfoReader;
    private long mLeftStorage;
    private long mUsedStorage;
    private long mPengdingSize;
    private long mStartTime;
    private int mCount;
    private int mSuitableSpeed;

    private MemoryAction mMemoryActon = MemoryAction.NORMAL;

    private Runtime mRuntime = Runtime.getRuntime();

    /**
     * The recommend action for continuous shot,
     * module calls {@link #getMemoryAction} to get the recommend action.
     */
    public enum MemoryAction {
        NORMAL, ADJSUT_SPEED, STOP,
    }

    /**
     * The constructor will fetch the max DVM heap size and min free memory,
     * initial threshold value of slow down or stop continuous shot.
     * @param context the context of the application or activity get activity service.
     */
    public MemoryManager(Context context) {
        // initial the low memory callback.
        context.registerComponentCallbacks(this);
        // initial the threshold of camera process DVM memory check.
        mMaxDvmMemory = mRuntime.maxMemory();
        mDvmSlowdownThreshold = (long) (DVM_SLOWDOWN_THRESHOLD * mMaxDvmMemory);
        mDvmStopThreshold = (long) (DVM_STOP_THRESHOLD * mMaxDvmMemory);
        // initial the threshold of low memory killer and
        // the the threshold of system memory check.
        ActivityManager.MemoryInfo mi = new ActivityManager.MemoryInfo();
        ((ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE)).getMemoryInfo(mi);
        mMiniMemFreeMb = mi.foregroundAppThreshold / BYTES_IN_KILOBYTE / BYTES_IN_KILOBYTE;
        long divisor = 1;
        if (toMb(mMaxDvmMemory) <= LOW_MEMORY_DEVICE) {
            divisor = LOW_MEMORY_DIVISOR;
        }
        mSystemSlowdownThreshold = SYSTEM_SLOWDOWN_THRESHOLD / divisor;
        mSystemStopThreshold = mSystemSlowdownThreshold / SYSTEM_STOP_DIVISOR;
        mMemInfoReader = new MemInfoReader();
    }

    @Override
    public void onLowMemory() {
        Log.d(TAG, "[onLowMemory]...");
        mMemoryActon = MemoryAction.STOP;
    }

    @Override
    public void onTrimMemory(int level) {
        Log.d(TAG, "[onTrimMemory]level: " + level);
        switch (level) {
        case ComponentCallbacks2.TRIM_MEMORY_RUNNING_CRITICAL:
        case ComponentCallbacks2.TRIM_MEMORY_BACKGROUND:
        case ComponentCallbacks2.TRIM_MEMORY_UI_HIDDEN:
            mMemoryActon = MemoryAction.ADJSUT_SPEED;
            break;

        case ComponentCallbacks2.TRIM_MEMORY_COMPLETE:
        case ComponentCallbacks2.TRIM_MEMORY_MODERATE:
            mMemoryActon = MemoryAction.STOP;
            break;

        default:
            mMemoryActon = MemoryAction.NORMAL;
            break;
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // do nothing
    }

    /**
     * Initial the status of memory manager.
     * @param leftStorage The left size of storage.
     */
    public void init(long leftStorage) {
        mMemoryActon = MemoryAction.NORMAL;
        mLeftStorage = leftStorage;
        mUsedStorage = 0;
        mPengdingSize = 0;
        mCount = 0;
    }

    /**
     * it will be called when continuous shot started.
     */
    public void start() {
        mStartTime = System.currentTimeMillis();
    }

    /**
     * get the memory action that will check DVM memory and system memory,
     * and calculate the suitable capture speed in advance.
     * @param pictureSize the last captured image size;
     * @param pendingSize the size of all pending images;
     * @return MemoryAction the recommend action for continuous shot.
     */
    public MemoryAction getMemoryAction(long pictureSize, long pendingSize) {
        Log.d(TAG, "[getMemoryAction]pictureSize=" + toMb(pictureSize) + " MB, pendingSize="
                + toMb(pendingSize) + " MB");
        mCount++;
        mUsedStorage += pictureSize;
        mPengdingSize = pendingSize;
        long timeDuration = System.currentTimeMillis() - mStartTime;
        long captureSpeed = mCount * BYTES_IN_KILOBYTE / timeDuration;
        long saveSpeed = (mUsedStorage - mPengdingSize) / timeDuration / BYTES_IN_KILOBYTE;
//        Log.d(TAG, "[getMemoryAction]Capture speed=" + captureSpeed + " fps, Save speed="
//                + saveSpeed + " MB/s");
//        // remaining storage check.
//        Log.d(TAG, "[getMemoryAction]mUsedStorage=" + toMb(mUsedStorage) + " MB, mLeftStorage="
//                + toMb(mLeftStorage) + " MB");
        if (mUsedStorage >= mLeftStorage) {
            Log.d(TAG, "[getMemoryAction]Storage size check, need to stop");
            return MemoryAction.STOP;
        }
        mSuitableSpeed = (int) ((mUsedStorage - mPengdingSize) * mCount * BYTES_IN_KILOBYTE
                / timeDuration / mUsedStorage);
//        Log.d(TAG, "[getMemoryAction]Suitable speed=" + mSuitableSpeed + " fps");

        // System memory check;
        // system memory > 512MB project
        // stop condition: Max(memFree,Cached) < mimMemFree + 50M;
        // Slow down condition:Max(memFree,Cached) < mimMemFree + 100M;
        // system memory <= 512MB project
        // stop condition: Max(memFree,Cached) < mimMemFree + 25M;
        // Slow down condition:Max(memFree,Cached) < mimMemFree + 50M;
        mMemInfoReader.readMemInfo();
        long[] memInfos = mMemInfoReader.getRawInfo();
        long cached = memInfos[Debug.MEMINFO_CACHED] / BYTES_IN_KILOBYTE;
        long free = memInfos[Debug.MEMINFO_FREE] / BYTES_IN_KILOBYTE;
        long memFreeDiffMb = (cached > free) ? cached : free;
        memFreeDiffMb = memFreeDiffMb - mMiniMemFreeMb;
//        Log.d(TAG, "[getMemoryAction]cached=" + cached + " MB, free=" + free
//                + " MB, memFreeDiff=" + memFreeDiffMb + " MB");
        if (memFreeDiffMb < mSystemStopThreshold) {
            Log.d(TAG, "[getMemoryAction]System memory check, need to stop");
            return MemoryAction.STOP;
        } else if (memFreeDiffMb < mSystemSlowdownThreshold) {
            Log.d(TAG, "[getMemoryAction]System memory check,need to slowdown");
            return MemoryAction.ADJSUT_SPEED;
        }

        // application pending Jpeg data size check;
        if (mPengdingSize >= mDvmSlowdownThreshold) {
            Log.d(TAG, "[getMemoryAction]DVM memory check,need to slowdown");
            return MemoryAction.ADJSUT_SPEED;
        }
        // Camera process DVM memory check;
        long total = mRuntime.totalMemory();
        free = mRuntime.freeMemory();
        long realfree = mMaxDvmMemory - (total - free);
//        Log.d(TAG, "[getMemoryAction]total=" + toMb(total) + " MB, free=" + toMb(free)
//                + " MB, real free=" + toMb(realfree) + " MB");
        if (realfree <= mDvmStopThreshold) {
            Log.d(TAG, "[getMemoryAction]DVM memory check,need to stop");
            return MemoryAction.STOP;
        }
        return MemoryAction.NORMAL;
    }

    /**
     * get suitable speed for continuous shot which is calculated in {@link #getMemoryAction}.
     * @return the suitable speed.
     */
    public int getSuitableContinuousShotSpeed() {
        if (mSuitableSpeed < LOW_SUITABLE_SPEED_FPS) {
            mSuitableSpeed = LOW_SUITABLE_SPEED_FPS;
            Log.i(TAG, "[getSuitableContinuousShotSpeed]Current performance is very poor!");
        }
        return mSuitableSpeed;
    }

    private long toMb(long in) {
        return in / BYTES_IN_KILOBYTE / BYTES_IN_KILOBYTE;
    }
}