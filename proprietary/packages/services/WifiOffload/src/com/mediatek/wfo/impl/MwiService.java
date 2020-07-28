/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.wfo.impl;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.SystemProperties;
import android.os.Message;

import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import com.mediatek.wfo.IMwiService;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.ril.MwiRIL;

/**
 * Mobile Wi-Fi Interaction Service.
 */
public class MwiService extends IMwiService.Stub {
    static final String MWIS_LOG_TAG = "MWIS";
    private static final boolean VDBG = true;  // STOPSHIP if true

    private static MwiService mInstance = null;
    private Context mContext;

    private int mSimCount;

    private static WifiPdnHandler mWifiPdnHandler;
    private WfcHandler mWfcHandler;
    private WfcLocationHandler mWfcLocationHandler;
    private MwiRIL[] mMwiRil;

    HandlerThread mHandlerThread = new HandlerThread("MwiServiceHandlerThread");

    public static MwiService getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new MwiService(context);
        }
        return mInstance;
    }

    public static MwiService getInstance() {
        return mInstance;
    }

    public MwiService(Context context) {
        logd("Construct MwiService");
        mContext = context;

        TelephonyManager telephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager != null) {
            mSimCount = telephonyManager.getSimCount();
        } else {
            logd("telephonyManager = null");
        }
        logd("mSimCount: " + mSimCount);



        mHandlerThread.start();

        // Initialize Mwi RIL
        mMwiRil = new MwiRIL[mSimCount];
        for(int i = 0; i < mSimCount; i++) {
            mMwiRil[i] = new MwiRIL(context, i, mHandlerThread.getLooper());
        }

        // mWifiPdnHandler = new WifiPdnHandler(mContext, mSimCount, mHandlerThread.getLooper(), mMwiRil);
        // mWfcHandler = new WfcHandler(mContext, mWifiPdnHandler, mSimCount, mHandlerThread.getLooper(), mMwiRil);
        mWifiPdnHandler = new WifiPdnHandler(
                mContext, mSimCount, mHandlerThread.getLooper(), mMwiRil);
        mWfcHandler = WfcHandler.getInstance(
                mContext, mWifiPdnHandler, mSimCount, mHandlerThread.getLooper(), mMwiRil);
        mWfcLocationHandler = new WfcLocationHandler(mContext, mWfcHandler, mWifiPdnHandler,
                mSimCount, mHandlerThread.getLooper(), mMwiRil);
    }

    public MwiRIL getMwiRIL(int slotId) {
        if (slotId >= mSimCount) {
            Rlog.e(MWIS_LOG_TAG, "Out of the bounds, slotId is: " + slotId);
            return null;
        }
        return mMwiRil[slotId];
    }

    private boolean is93RilMode() {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return true;
        }
        return false;
    }

    @Override
    public IWifiOffloadService getWfcHandlerInterface() {
        logd("getWfcHandlerInterface");
        return mWfcHandler.getWfoInterface();
    }

    public void dispose() {
        logd("dispose()");
        mHandlerThread.quit();
        mHandlerThread.interrupt();
    }

    private static void logd(String l) {
        Rlog.d(MWIS_LOG_TAG, "[MwiService] " + l);
    }
}
