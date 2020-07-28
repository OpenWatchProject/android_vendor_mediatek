/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.capctrl.service;

import java.util.ArrayList;
import java.util.List;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

import android.telephony.TelephonyManager;
import android.telephony.Rlog;

public class MtkCapCtrlService extends Service {
    private static final String LOG_TAG = "MtkCapService";
    private static final int BASE = 0;
    private static final int EVENT_SERVICE_CREATED = BASE + 1;
    private static final int EVENT_SERVICE_DESTROYED = BASE + 2;

    private Context mContext;
    private CapRIL[] mCapRILs;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SERVICE_CREATED:
                    mCapRILs = initCapRIL();
                    initCapCtrlInterface();
                    break;
                case EVENT_SERVICE_DESTROYED:
                    break;
                default:
                    Rlog.e(LOG_TAG, "Unhandled message with number: " + msg.what);
                    break;
            }
        }
    };

    @Override
    public void onCreate() {
        mContext = getBaseContext();
        Rlog.d(LOG_TAG, "created");
        mHandler.obtainMessage(EVENT_SERVICE_CREATED).sendToTarget();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Rlog.d(LOG_TAG, "onStartCommand");
        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        Rlog.d(LOG_TAG, "destroyed");
        mHandler.obtainMessage(EVENT_SERVICE_DESTROYED).sendToTarget();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void initCapCtrlInterface() {
        MtkCapCtrlInterfaceManager.init(mContext, mCapRILs);
    }

    private CapRIL[] initCapRIL() {
        int simCount = TelephonyManager.getDefault().getSimCount();
        Rlog.d(LOG_TAG, "initCapRIL, sim count " + simCount);
        CapRIL[] capRILs = new CapRIL[simCount];
        for (int i = 0; i < simCount; i++) {
            capRILs[i] = new CapRIL(mContext, i);
        }
        return capRILs;
    }
}