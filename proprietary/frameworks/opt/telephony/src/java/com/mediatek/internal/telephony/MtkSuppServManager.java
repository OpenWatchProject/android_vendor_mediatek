/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.content.Context;

import android.os.HandlerThread;
import android.os.Looper;
import android.os.SystemProperties;

import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;

public class MtkSuppServManager {
    public static final String LOG_TAG = "SuppServManager";
    private static final boolean DBG = true;

    /* Singleton instance for MtkSuppServQueueHelper */
    private static MtkSuppServManager instance = null;

    private Phone[] mPhones;           // store all the Phone objects for different slots
    private Context mContext;
    private HandlerThread mHandlerThread;

    private static MtkSuppServQueueHelper mSuppServQueueHelper = null;
    private static MtkSuppServHelper[] mSuppServHelper = null;
    private static MtkSuppServConf[] mSuppServConf = null;

    private MtkSuppServManager(Context context, Phone[] phones) {
        mContext = context;
        mPhones = phones;
    }

    public static MtkSuppServManager getInstance() {
        return instance;
    }

    public static MtkSuppServManager makeSuppServManager(Context context, Phone[] phones) {
        if (context == null || phones == null) {
            return null;
        }

        if (instance == null) {
            Rlog.d(LOG_TAG, "Create MtkSuppServManager singleton instance, phones.length = " +
                    phones.length);
            instance = new MtkSuppServManager(context, phones);
        } else {
            instance.mContext = context;
            instance.mPhones = phones;
        }
        return instance;
    }

    /* Get MtkSuppServQueueHelper instance */
    public static MtkSuppServQueueHelper getSuppServQueueHelper() {
        if (mSuppServQueueHelper != null) {
            return mSuppServQueueHelper;
        } else {
            Rlog.e(LOG_TAG, "Get MtkSuppServQueueHelper instance failed!");
            return null;
        }
    }

    /* Get MtkSuppServHelper instance */
    public static MtkSuppServHelper getSuppServHelper(int phoneId) {
        if ((mSuppServHelper != null) && (mSuppServHelper.length > phoneId) && (phoneId >= 0)) {
            return mSuppServHelper[phoneId];
        } else {
            Rlog.e(LOG_TAG, "Get MtkSuppServHelper instance failed!");
            return null;
        }
    }

    /* Get MtkSuppServConf instance */
    public static MtkSuppServConf getSuppServConf(int phoneId) {
        if ((mSuppServConf != null) && (mSuppServConf.length > phoneId) && (phoneId >= 0)) {
            return mSuppServConf[phoneId];
        } else {
            Rlog.e(LOG_TAG, "Get MtkSuppServConf instance failed!");
            return null;
        }
    }

    public void init() {
        Rlog.d(LOG_TAG, "Initialize MtkSuppServManager!");

        // All Supplementary service helper module would use this sub thread.
        mHandlerThread = new HandlerThread("MtkSuppServManager");
        mHandlerThread.start();
        Looper looper = mHandlerThread.getLooper();

        // Fusion RIL supports SS queue mechanism, no need to create the instance
        if (!supportMdAutoSetupIms()) {
            mSuppServQueueHelper = MtkSuppServQueueHelper.makeSuppServQueueHelper(
                    mContext, mPhones);
            mSuppServQueueHelper.init(looper);
        }

        int numPhones = TelephonyManager.getDefault().getPhoneCount();

        mSuppServHelper = new MtkSuppServHelper[numPhones];
        for (int i = 0; i < numPhones; i++) {
            mSuppServHelper[i] =
                    new MtkSuppServHelper(mContext, mPhones[i]);
            mSuppServHelper[i].init(looper);
        }

        mSuppServConf = new MtkSuppServConf[numPhones];
        for (int i = 0; i < numPhones; i++) {
            mSuppServConf[i] = new MtkSuppServConf(mContext, mPhones[i]);
            mSuppServConf[i].init(looper);
        }
    }

    public void dispose() {
        Rlog.d(LOG_TAG, "dispose.");
        Looper looper = mHandlerThread.getLooper();
        looper.quit();
    }

    private boolean supportMdAutoSetupIms() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }
}