/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.internal.telephony.imsphone;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.os.SystemClock;
import android.telephony.Rlog;

import com.android.internal.telephony.imsphone.ImsPhoneConnection;

public class RttEmcGuardTimerUtil {
    private static final String TAG = "RttEmcGuardTimerUtil";

    private Context mContext;
    private static final String INTENT_RTT_EMC_GUARD_TIMER_180 =
          "com.mediatek.internal.telephony.imsphone.rtt_emc_guard_timer_180";
    private AlarmManager mAlarmManager;
    private PendingIntent mRttEmcIntent = null;
    private boolean mDuringRttGuardDuration = false;
    private boolean mIsRttEmcGuardTimerSupported = false;

    public RttEmcGuardTimerUtil(Context context) {
        mContext = context;
    }

    public void initRttEmcGuardTimer() {
        Rlog.d(TAG, "initRttEmcGuardTimer");
        if (mContext == null) {
            Rlog.d(TAG, "initRttEmcGuardTimer mContext == null");
            return;
        }
        mAlarmManager = (AlarmManager) mContext.
            getSystemService(Context.ALARM_SERVICE);
        registerRttReceiver();
    }

    private BroadcastReceiver mRttReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(INTENT_RTT_EMC_GUARD_TIMER_180)) {

                Rlog.d(TAG, "onReceive : mRttReceiver rtt guard timer 180");
                stopRttEmcGuardTimer();
                mRttEmcIntent = null;
                mDuringRttGuardDuration = false;

            }
        }
    };

    public void disposeRttEmcGuardTimer() {
        Rlog.d(TAG, "disposeRttEmcGuardTimer");
        unregisterRttReceiver();
    }

    private void registerRttReceiver() {
        Rlog.d(TAG, "registerRttReceiver");
        IntentFilter intentfilter = new IntentFilter();
        intentfilter.addAction(INTENT_RTT_EMC_GUARD_TIMER_180);
        mContext.registerReceiver(mRttReceiver, intentfilter);
    }

    private void unregisterRttReceiver() {
        Rlog.d(TAG,"unregisterRttReceiver");
        mContext.unregisterReceiver(mRttReceiver);
    }

    public void stopRttEmcGuardTimer() {

        Rlog.d(TAG, "stopRttEmcGuardTimer");

        if (mRttEmcIntent != null) {
            Rlog.d(TAG, "stopRttEmcGuardTimer, cancel timer");

            mAlarmManager.cancel(mRttEmcIntent);
            mRttEmcIntent = null;
            mDuringRttGuardDuration = false;
        }
    }

    public void checkIncomingCallInRttEmcGuardTime(ImsPhoneConnection conn) {

        Rlog.d(TAG, "checkIncomingCallInRttEmcGuardTime: " + conn);

        if (conn == null) {
            Rlog.e(TAG, "conn == null, checkIncomingCallInRttEmcGuardTime return");
            return;
        }

        if (mDuringRttGuardDuration == true) {
            ((MtkImsPhoneConnection) conn).setIncomingCallDuringRttEmcGuard(true);
        } else {
            ((MtkImsPhoneConnection) conn).setIncomingCallDuringRttEmcGuard(false);
        }
    }

    public void startRttEmcGuardTimer() {
        if (!mIsRttEmcGuardTimerSupported) {
            Rlog.d(TAG, "startRttEmcGuardTimer: Current carrier doesn't support "
                    + "RTT EMC guard timer, just return");
            return;
        }

        if (mContext == null) {
            Rlog.e(TAG, "startRttEmcGuardTimer mContext == null");
            return;
        }

        stopRttEmcGuardTimer();

        Intent intent = new Intent(INTENT_RTT_EMC_GUARD_TIMER_180);
        mRttEmcIntent = PendingIntent.getBroadcast(mContext, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        int delay = 180 * 1000; // 180s

        Rlog.d(TAG,"startRttEmcGuardTimer: delay=" + delay);

        mDuringRttGuardDuration = true;
        mAlarmManager.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP,
                SystemClock.elapsedRealtime() + delay, mRttEmcIntent);

    }

    public void setRttEmcGuardTimerSupported(boolean isSupported) {
        mIsRttEmcGuardTimerSupported = isSupported;
    }
}
