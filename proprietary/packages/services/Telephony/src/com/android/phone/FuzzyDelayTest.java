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

package com.android.phone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.telephony.Rlog;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import static com.android.internal.util.Preconditions.checkNotNull;

public class FuzzyDelayTest {
    static final String LOG_TAG = "FuzzyDelayTest";
    private static FuzzyDelayTest sFuzzyDelayTest;
    private static int sFuzzyMax;

    public static FuzzyDelayTest init() {
        synchronized (FuzzyDelayTest.class) {
            // Only create this obj when property is 1
            if (sFuzzyDelayTest == null) {
                //Get Max Fuzzy time default is 2 second.
                sFuzzyMax = SystemProperties.getInt("persist.vendor.radio.phonefuzzy.max", 2);

                Rlog.d(LOG_TAG, "new FuzzyDelayTest()");
                sFuzzyDelayTest = new FuzzyDelayTest();
            }
            return sFuzzyDelayTest;
        }
    }

    private FuzzyDelayTest() {

        Phone phone = null;
        Context context;
        Rlog.d(LOG_TAG, "create FuzzyDelayTest()");
        try {
            phone = PhoneFactory.getDefaultPhone();
        } catch (IllegalStateException e) {
            Rlog.e(LOG_TAG, "failed to get default phone from PhoneFactory: "
                    + e.toString());
        }
        checkNotNull(phone, "default phone is null");
        context = phone.getContext();
        checkNotNull(context, "missing Context");

        IntentFilter dealyTestIntentFilter = new IntentFilter();
        dealyTestIntentFilter
                .addAction("com.mediatek.phone.ACTION_PHONE_DELAY_START");
        dealyTestIntentFilter
                .addAction("com.mediatek.phone.ACTION_PHONE_DELAY_SLEEP");
        dealyTestIntentFilter
                .addAction("com.mediatek.phone.ACTION_PHONE_DELAY_STOP");
        context.registerReceiver(mDealyTestReceiver, dealyTestIntentFilter);
    }

    private final BroadcastReceiver mDealyTestReceiver = new BroadcastReceiver() {
        boolean running = false;

        private void trigerNextSleep(Context context, Intent intent) {
            Intent delayIntent = new Intent(intent);
            delayIntent
                    .setAction("com.mediatek.phone.ACTION_PHONE_DELAY_SLEEP");
            context.sendBroadcast(delayIntent);
            Rlog.d(LOG_TAG, "ACTION_PHONE trigerNextSleep Done");
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            Rlog.d(LOG_TAG, "mDealyTestReceiver.onReceive, action=" + action);

            if ("com.mediatek.phone.ACTION_PHONE_DELAY_START".equals(action)) {
                Rlog.d(LOG_TAG, "ACTION_PHONE_DELAY_START Received");
                running = true;
                trigerNextSleep(context, intent);
            } else if ("com.mediatek.phone.ACTION_PHONE_DELAY_SLEEP"
                    .equals(action)) {
                int sleepDuration = (int) (Math.random() * 1000 * sFuzzyMax);
                Rlog.d(LOG_TAG, "ACTION_PHONE_DELAY_SLEEP Received"
                        + sleepDuration + "/" + running);
                if (running == true) {
                    try {
                        Thread.sleep(sleepDuration);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    Rlog.d(LOG_TAG, "ACTION_PHONE_DELAY_SLEEP Done");
                    trigerNextSleep(context, intent);
                }
            } else if ("com.mediatek.phone.ACTION_PHONE_DELAY_STOP"
                    .equals(action)) {
                Rlog.d(LOG_TAG, "ACTION_PHONE_DELAY_STOP Received");
                running = false;
            }
        }
    };
}
