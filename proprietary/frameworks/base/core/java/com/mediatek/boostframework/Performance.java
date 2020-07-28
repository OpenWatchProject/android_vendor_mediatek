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

package com.mediatek.boostframework;

import android.util.Log;
import android.util.Printer;
import android.os.Build;
import android.os.Looper;
import android.os.Process;
import android.os.RemoteException;
import android.os.Binder;
import android.os.Trace;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.powerhalwrapper.PowerHalWrapper;

public class Performance {
    private static final String TAG = "Performance";
    private int mhandle = 0;
    private static PowerHalWrapper mPowerHalWrap = null;

    public Performance() {
        mPowerHalWrap = PowerHalWrapper.getInstance();
    }

    public int perfchecklist(int... list) {
        return (list.length % 2 == 0) ? 1 : 0;
    }

    public int perfLockAcquire(int duration, int... list) {

        //Log.e(TAG, "<perfLockAcquire> list.length: " + list.length);

        if (perfchecklist(list) == 0)
            return -1;

        mhandle = mPowerHalWrap.perfLockAcquire(mhandle, duration, list);

        //Log.e(TAG, "<perfLockAcquire> mhandle: " + mhandle);

        return mhandle;
    }

    public int perfLockRelease() {
        //Log.e(TAG, "<perfLockRelease> mhandle" + mhandle);
        mPowerHalWrap.perfLockRelease(mhandle);
        return 0;
    }

    public int perfLockRelease(int handle) {
        //Log.e(TAG, "<perfLockRelease> handle: " + handle);
        mPowerHalWrap.perfLockRelease(handle);
        return 0;
    }

    private static void log(String info) {
        Log.d("@M_" + TAG, "[Performance] " + info + " ");
    }

    private static void loge(String info) {
        Log.e("@M_" + TAG, "[Performance] ERR: " + info + " ");
    }

}
