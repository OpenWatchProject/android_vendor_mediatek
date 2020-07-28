/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.cb.cbmsg;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.PhoneConstants;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.util.MmsLog;
import com.mediatek.mms.appserviceproxy.AppServiceProxy;
import com.mediatek.mms.util.PermissionCheckUtil;

/**
 * M:
 * Handle incoming SMSes.  Just dispatches the work off to a Service.
 */
public class CbMessageReceiver extends BroadcastReceiver {
    static final Object STARTING_SERVICE_SYNC = new Object();
    static PowerManager.WakeLock sStartingService;
    private static CbMessageReceiver sInstance;

    public static CbMessageReceiver getInstance() {
        if (sInstance == null) {
            sInstance = new CbMessageReceiver();
        }
        return sInstance;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
            MmsLog.d(MmsApp.TXN_TAG,
                    "CBMessageReceiver onReceive no runtime permissions, intent: " + intent);
            return;
        }
        onReceiveWithPrivilege(context, intent, false);
    }

    protected void onReceiveWithPrivilege(Context context, Intent intent, boolean privileged) {
        MmsLog.d(MmsApp.TXN_TAG,
                "CBMessageReceiver: onReceiveWithPrivilege(). Slot Id = "
                        + Integer.toString(intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                                SubscriptionManager.INVALID_SUBSCRIPTION_ID), 10) + ", Action = "
                        + intent.getAction() + ", result = " + getResultCode());

        intent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE, MmsConfig.CB_RECEIVER_SERVICE);
        intent.putExtra("result", getResultCode());
        beginStartingService(context, intent);
    }

    // N.B.: <code>beginStartingService</code> and
    // <code>finishStartingService</code> were copied from
    // <code>com.android.calendar.AlertReceiver</code>.  We should
    // factor them out or, even better, improve the API for starting
    // services under wake locks.

    /**
     * Start the service to process the current event notifications, acquiring
     * the wake lock before returning to ensure that the service will run.
     */
    public static void beginStartingService(Context context, Intent intent) {
        synchronized (STARTING_SERVICE_SYNC) {
            if (sStartingService == null) {
                PowerManager pm =
                    (PowerManager) context.getSystemService(Context.POWER_SERVICE);
                sStartingService = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                        "StartingAlertService");
                sStartingService.setReferenceCounted(false);
            }
            sStartingService.acquire();
            /* start service*/
            context.startService(intent);
            //Context ct = context.getApplicationContext();
            //AppServiceProxy.getInstance(ct).startServiceByParam(ct, intent);
            /*
            new Thread(new Runnable() {
                public void run() {
                    MmsLog.d(MmsApp.TXN_TAG, "CbMessageReceiver: call startServiceByParam");
                    Context ct = context.getApplicationContext();
                    AppServiceProxy.getInstance(ct).startServiceByParam(ct, intent);
                }
            }).start();*/
        }
    }

    /**
     * Called back by the service when it has finished processing notifications,
     * releasing the wake lock if the service is now stopping.
     */
    public static void finishStartingService(Service service, int startId) {
        synchronized (STARTING_SERVICE_SYNC) {
            if (sStartingService != null) {
                if (service.stopSelfResult(startId)) {
                    sStartingService.release();
                }
            }
        }
    }
}
