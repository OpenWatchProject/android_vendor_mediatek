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
package com.mediatek.anr;

import android.util.Log;
import android.util.Printer;
import android.os.Build;
import android.os.Looper;
import android.os.Process;
import android.os.RemoteException;


import java.util.HashMap;
import java.util.Iterator;

/**
 * Use to dump message history and message queue.
 *
 * @hide
 */
public final class AnrAppManagerImpl extends AnrAppManager {
    private static final String TAG = "AnrAppManager";
    protected static HashMap<String, MessageLogger> sMap = new HashMap<String, MessageLogger>();
    private static AnrAppManagerImpl sInstance = null;
    private static MessageLogger sSingletonLogger = null;
    private static Object lock = new Object();

    public static AnrAppManagerImpl getInstance() {
        if (null == sInstance) {
            synchronized (lock) {
                if (null == sInstance) {
                    sInstance = new AnrAppManagerImpl();
                }
            }
        }
        return sInstance;
    }

    @Override
    public void setMessageLogger(Looper looper) {
        if ("eng".equals(Build.TYPE)) {
            looper.setMessageLogging(newMessageLogger(false));
        }
    }

    @Override
    public void dumpMessage(boolean dumpAll) {
        if (dumpAll) {
            dumpAllMessageHistory();
        } else {
            dumpMessageHistory();
        }
    }

    public static Printer newMessageLogger(boolean mValue) {
        sSingletonLogger = new MessageLogger(mValue);
        return sSingletonLogger;
    }

    public static Printer newMessageLogger(boolean mValue, String name) {
        if (sMap.containsKey(name)) {
            sMap.remove(name);
        }
        MessageLogger logger = new MessageLogger(mValue, name);
        sMap.put(name, logger);
        return logger;
    }

    public static void dumpMessageHistory() {
        if (null == sSingletonLogger) {
            Log.i(TAG, "!!! It is not under singleton mode, U can't use it. !!!\n");
            try {
                AnrManagerNative.getDefault().informMessageDump(
                    "!!! It is not under singleton mode, U can't use it. !!!\n", Process.myPid());
            } catch (RemoteException ex) {
                Log.i(TAG, "informMessageDump exception " + ex);
            }
        } else {
            sSingletonLogger.dumpMessageHistory();
        }
    }

    public static void dumpAllMessageHistory() {
        MessageLogger logger = null;
        String tmp_str = null;
        if (null != sSingletonLogger) {
            Log.i(TAG, "!!! It is under multiple instance mode"
                + ", but you are in singleton usage style. !!!\n");
            try {
                AnrManagerNative.getDefault().informMessageDump(
                    "!!! It is under multiple instance mode," +
                    "but you are in singleton usage style. !!!\n",
                    Process.myPid());
            } catch (RemoteException ex) {
                Log.i(TAG, "informMessageDump exception " + ex);
            }
            return;
        }
        if (sMap == null) {
            Log.i(TAG, String.format("!!! DumpAll, sMap is null\n"));
            try {
                AnrManagerNative.getDefault().informMessageDump(
                    "!!! DumpAll, sMap is null\n", Process.myPid());
            } catch (RemoteException ex) {
                Log.i(TAG, "informMessageDump exception " + ex);
            }
            return ;
        }
        Iterator<String> name_iter = sMap.keySet().iterator();
        while (name_iter.hasNext()) {
            tmp_str = name_iter.next();
            Log.i(TAG, String.format(
                ">>> DumpByName, Thread name: %s dump is starting <<<\n", tmp_str));
            sMap.get(tmp_str).setInitStr(String.format(
                ">>> DumpByName, Thread name: %s dump is starting <<<\n", tmp_str));
            sMap.get(tmp_str).dumpMessageHistory();
        }
    }
}
