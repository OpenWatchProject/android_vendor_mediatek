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

package com.android.mms.util;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Telephony;
import android.provider.Telephony.Sms;
import android.util.Log;

import com.android.mms.transaction.SmsReceiver;
import com.mediatek.internal.telephony.IMtkTelephonyEx;

import mediatek.telephony.MtkSmsManager;

/// M:
public class ThreadCountManager {
    private static final int THREAD_MAX_SIZE = 100;

    private static final String TAG = "ThreadCountManager";

    private static ThreadCountManager sInstance;

    public static final int OP_FLAG_INCREASE = 0;
    public static final int OP_FLAG_DECREASE = 1;

    /**
     * constructor
     */
    private ThreadCountManager() {
    }

    public static synchronized ThreadCountManager getInstance() {
        if (sInstance == null) {
            sInstance = new ThreadCountManager();
        }
        return sInstance;
    }

    /**
     * get the thread count
     *
     * @param threadId
     * @param context
     * @return
     */
    private int get(Long threadId, Context context) {
        ContentResolver resolver = context.getContentResolver();
        String where = Telephony.Mms.THREAD_ID + "=" + threadId;
        String[] projection = new String[] { Sms.Inbox._ID };

        Cursor cursor = null;

        try {
            cursor = Sms.query(resolver, projection, where, null);
            if (cursor == null) {
                return 0;
            } else {
                int cnt = cursor.getCount();
                Log.d(TAG, "sms count is :" + cnt);
                return cnt;
            }
        } finally {
            if (cursor != null) {
                Log.d(TAG, "close cursor");
                cursor.close();
            }
        }
    }

    /**
     * while receiving one sms, check the sms count to see whether it reaches
     * the max count.
     *
     * @param threadId
     * @param context
     * @param flag indicate the operator (increase/decrease)
     * @return true if the count is less than the max size, otherwise false.
     */
    public void isFull(Long threadId, Context context, int flag) {
        try {
                IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                        ServiceManager.getService("phoneEx"));
                if (iTelEx.isTestIccCard(0) ||
                        (Integer.parseInt(SystemProperties.get("vendor.gsm.gcf.testmode", "0")) == 2)) {
                    Log.d(TAG, "Now using test icc card...");
                    int lastSubId = SmsReceiver.sLastIncomingSmsSubId;
                    if (flag == OP_FLAG_INCREASE) {
                        if (get(threadId, context) >= THREAD_MAX_SIZE) {
                            Log.d(TAG, "Storage is full. send notification...");
                        MtkSmsManager.getSmsManagerForSubscriptionId(lastSubId)
                                .setSmsMemoryStatus(false);
                        }
                    } else if (flag == OP_FLAG_DECREASE) {
                        if (get(threadId, context) < THREAD_MAX_SIZE) {
                            Log.d(TAG, "Storage is available. send notification...");
                        MtkSmsManager.getSmsManagerForSubscriptionId(lastSubId)
                                .setSmsMemoryStatus(true);
                        }
                    }
                }
        } catch (Exception ex) {
            Log.e(TAG, " " + ex.getMessage());
        }
    }

}
