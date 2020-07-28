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
/*
 * Copyright (C) 2016 The Android Open Source Project
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

package com.mediatek.services.telephony;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import com.android.services.telephony.Log;

import com.android.internal.telephony.Phone;

/**
 * Helper class that listens to a Phone's radio state and sends a callback when the radio state of
 * that Phone is either "in service".
 */
public class EcbmCallStateListener {

    /**
     * Receives the result of the EcbmCallStateListener's attempt to turn on the radio.
     */
    public interface Callback {
        void onComplete(EcbmCallStateListener listener);
    }

    private static long TIME_WAIT_IN_SERVICE_MILLIS = 1000;  // msec
    private static int MAX_NUM_WAITING = 10;
    private int timeoutCount = 0;

    // Handler message codes; see handleMessage()
    public static final int MSG_SERVICE_STATE_CHANGED = 1;
    public static final int MSG_TIMEOUT = 2;

    private Callback mCallback;  // The callback to notify upon completion.
    private Phone mPhone;  // The phone that will attempt to place the call.


    private final Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SERVICE_STATE_CHANGED:
                    onServiceStateChanged((ServiceState) ((AsyncResult) msg.obj).result);
                    break;
                case MSG_TIMEOUT:
                    onTimeout();
                    break;
                default:
                    Log.wtf(this, "handleMessage: unexpected message: %d.", msg.what);
                    break;
            }
        }
    };

    /**
     * Wait for IN_SERVICE
     */
    public void waitForInService(Phone phone, Callback callback) {
        Log.d(this, "waitForInService: Phone " + phone.getPhoneId());

        // First of all, clean up any state left over from a prior emergency call sequence. This
        // ensures that we'll behave sanely if another startTurnOnRadioSequence() comes in while
        // we're already in the middle of the sequence.
        cleanup();

        mPhone = phone;
        mCallback = callback;

        if (isOkToCall(mPhone.getServiceState())) {
            Log.d(this, "Ok to make call");
            onComplete();
            cleanup();
        } else {
            registerForServiceStateChanged();
            // Discard waiting after waiting for 30 seconds.
            startTimer();
        }
    }

    /**
     * Handles the SERVICE_STATE_CHANGED event. Normally this event tells us that the radio has
     * finally come up. In that case, it's now safe to actually place the emergency call.
     */
    private void onServiceStateChanged(ServiceState ss) {
        Log.i(this, "onServiceStateChanged(), voice reg state = %d, data reg state = %d, Phone(%d)",
                ss.getVoiceRegState(),
                ss.getDataRegState(),
                mPhone.getPhoneId());

        if (isOkToCall(ss)) {
            onComplete();
            cleanup();
        } else {
            // The service state changed, but we're still not ready to call yet.
            Log.i(this, "onServiceStateChanged: not ready to call yet, keep waiting.");
        }
    }

    /**
     * We currently only look to make sure that the radio is on before dialing. We should be able to
     * make emergency calls at any time after the radio has been powered on and isn't in the
     * UNAVAILABLE state, even if it is reporting the OUT_OF_SERVICE state.
     */
    private boolean isOkToCall(ServiceState ss) {
        if (mPhone.isWifiCallingEnabled() || mPhone.isVolteEnabled()) {
            return true;
        }
 
        int voiceSS = ss.getVoiceRegState();  //i.e. state.getState()
        int dataSS = ss.getDataRegState();
        int voiceNetType = ss.getVoiceNetworkType();
        int dataNetType = ss.getDataNetworkType();

        if (voiceSS == ServiceState.STATE_IN_SERVICE &&
            (voiceNetType != TelephonyManager.NETWORK_TYPE_LTE
             || voiceNetType != TelephonyManager.NETWORK_TYPE_LTE_CA)) {
            return true;
        }

        return false;
    }

    /**
     * Handles timer expiring.
     */
    protected void onTimeout() {
        ServiceState ss = mPhone.getServiceState();
        Log.i(this, "onTimeout():  voice reg state = %d, data reg state = %d",
                ss.getVoiceRegState(), ss.getDataRegState());

        if (isOkToCall(ss)) {
            Log.d(this, "[onTimeout] Ok to make call");
            onComplete();
            cleanup();
        } else {
            timeoutCount++;
            if (timeoutCount < MAX_NUM_WAITING) {
                startTimer();
            } else {
                Log.d(this, "[onTimeout] Not in service and time out. Try to make call");
                onComplete();
                cleanup();
            }
        }
    }

    public void cleanup() {
        Log.d(this, "cleanup(), "
                + (mPhone != null ? "phoneId=" + mPhone.getPhoneId() : "(mPhone null)"));

        // This will send a failure call back if callback has yet to be invoked.  If the callback
        // was already invoked, it's a no-op.
        onComplete();

        unregisterForServiceStateChanged();
        cancelTimer();

        // Used for unregisterForServiceStateChanged() so we null it out here instead.
        mPhone = null;
    }

    private void startTimer() {
        cancelTimer();
        mHandler.sendEmptyMessageDelayed(MSG_TIMEOUT, TIME_WAIT_IN_SERVICE_MILLIS);
    }

    private void cancelTimer() {
        mHandler.removeMessages(MSG_TIMEOUT);
    }

    private void registerForServiceStateChanged() {
        // Unregister first, just to make sure we never register ourselves twice.  (We need this
        // because Phone.registerForServiceStateChanged() does not prevent multiple registration of
        // the same handler.)
        unregisterForServiceStateChanged();
        mPhone.registerForServiceStateChanged(mHandler, MSG_SERVICE_STATE_CHANGED, null);
    }

    private void unregisterForServiceStateChanged() {
        // This method is safe to call even if we haven't set mPhone yet.
        if (mPhone != null) {
            mPhone.unregisterForServiceStateChanged(mHandler);  // Safe even if unnecessary
        }
        mHandler.removeMessages(MSG_SERVICE_STATE_CHANGED);  // Clean up any pending messages too
    }

    private void onComplete() {
        if (mCallback != null) {
            Callback tempCallback = mCallback;
            mCallback = null;
            tempCallback.onComplete(this);
        }
    }
}
