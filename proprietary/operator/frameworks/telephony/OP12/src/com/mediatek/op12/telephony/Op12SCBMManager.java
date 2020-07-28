
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.op12.telephony;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.Registrant;
import android.os.UserHandle;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.internal.telephony.scbm.*;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.opcommon.telephony.MtkRilOp;

public class Op12SCBMManager extends SCBMManagerDefault {
    private static final boolean DBG = true;
    public static final String LOG_TAG = "Op12SCBMManager";

    // Default Emergency SMS Callback Mode exit timer
    private static final int DEFAULT_SCM_EXIT_TIMER_VALUE = 300000;

    public static final String PHONE_IN_SCM_STATE = "phoneinSCMState";

    private MtkRilOp mCi;
    private int mPhoneId;
    private Context mContext;
    // mScmExitRespRegistrant is informed after the phone has been exited
    private Registrant mScmExitRespRegistrant;

    protected PowerManager.WakeLock mWakeLock;

    // Keep track of whether or not the phone is in SCBM for Phone
    protected boolean mIsPhoneInScmState = false;

    // A runnable which is used to automatically exit from Scm after a period of time.
    private Runnable mExitScmRunnable = new Runnable() {
        @Override
        public void run() {
            exitSCBM();
        }
    };

    public Op12SCBMManager(Context context , int phoneId, CommandsInterface ci) {
        super(context, phoneId, ci);
        Rlog.d(LOG_TAG, "Initialize Op12SCBMManager" + phoneId);
        mPhoneId = phoneId;
        mContext = context;
        PowerManager pm
                = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, LOG_TAG);

        mCi = (MtkRilOp)((MtkRIL)ci).getRilOp();
        mCi.setSCBM(this, EVENT_SCBM_ENTER, null);
        mCi.registerForExitSCBM(this, EVENT_EXIT_SCBM_RESPONSE,
                null);
    }


    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        int[] ints;
        String[] strings;
        Message message;

        switch (msg.what) {
            case EVENT_SCBM_ENTER:
                handleEnterSCBM(msg);
                break;
            case EVENT_EXIT_SCBM_RESPONSE:
                handleExitSCBM(msg);
                break;
            default:
                super.handleMessage(msg);
                break;
        }
    }

    private void handleEnterSCBM(Message msg) {
        if (DBG) {
            Rlog.d(LOG_TAG, "handleEnterSCBM, isInScm()="
                    + isInScm());
        }

        //if already in scm, exit it first
        if (isInScm()) {
            setIsInScm(false);
            // Remove pending exit Scm runnable, if any
            removeCallbacks(mExitScmRunnable);
            // release wakeLock
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }

            // send an Intent
            sendSCBMChange();
        }

        setIsInScm(true);

        // notify change
        sendSCBMChange();

        // Post this runnable so we will automatically exit
        // if no one invokes exitEmergencyCallbackMode() directly.
        long delayInMillis = DEFAULT_SCM_EXIT_TIMER_VALUE;
        postDelayed(mExitScmRunnable, delayInMillis);
        // We don't want to go to sleep while in Scm
        mWakeLock.acquire();
    }

    public void exitSCBM() {
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
        }
        // Send a message which will invoke handleExitEmergencyCallbackMode
        mCi.exitSCBM(null);
    }

    public boolean isInScm() {
        return mIsPhoneInScmState;
    }

    public void setIsInScm(boolean isInScm) {
        mIsPhoneInScmState = isInScm;
    }

    private void sendSCBMChange(){
        //Send an Intent
        Intent intent = new Intent(TelephonyIntents.ACTION_SCBM_CHANGED);
        intent.putExtra(PHONE_IN_SCM_STATE, isInScm());
        SubscriptionManager.putPhoneIdAndSubIdExtra(intent, mPhoneId);
        ActivityManager.broadcastStickyIntent(intent, UserHandle.USER_ALL);
        if (DBG) Rlog.d(LOG_TAG, "sendSCBMChange");
    }

    public void setOnScbModeExitResponse(Handler h, int what, Object obj) {
        mScmExitRespRegistrant = new Registrant(h, what, obj);
    }

    public void unsetOnScbModeExitResponse(Handler h) {
        mScmExitRespRegistrant.clear();
    }

    protected void handleExitSCBM(Message msg) {
        AsyncResult ar = (AsyncResult)msg.obj;
        if (DBG) {
            Rlog.d(LOG_TAG, "handleExitSCBM,ar.exception , isInScm="
                    + ar.exception + isInScm());
        }
        // Remove pending exit Scm runnable, if any
        removeCallbacks(mExitScmRunnable);

        if (mScmExitRespRegistrant != null) {
            mScmExitRespRegistrant.notifyRegistrant(ar);
        }
        // if exiting scm success
        if (ar.exception == null) {
            if (isInScm()) {
                setIsInScm(false);
            }

            // release wakeLock
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }

            // send an Intent
            sendSCBMChange();
        }
    }
}
