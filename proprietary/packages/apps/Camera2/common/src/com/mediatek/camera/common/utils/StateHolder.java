/*
  * Copyright Statement:
  *
  *   This software/firmware and related documentation ("MediaTek Software") are
  *   protected under relevant copyright laws. The information contained herein is
  *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
  *   the prior written permission of MediaTek inc. and/or its licensors, any
  *   reproduction, modification, use or disclosure of MediaTek Software, and
  *   information contained herein, in whole or in part, shall be strictly
  *   prohibited.
  *
  *   MediaTek Inc. (C) 2016. All rights reserved.
  *
  *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
  *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
  *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
  *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
  *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
  *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
  *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
  *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
  *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
  *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
  *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
  *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
  *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
  *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
  *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
  *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
  *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
  *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
  *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
  *
  *   The following software/firmware and/or related documentation ("MediaTek
  *   Software") have been modified by MediaTek Inc. All revisions are subject to
  *   any receiver's applicable license agreements with MediaTek Inc.


 */

package com.mediatek.camera.common.utils;

import android.os.SystemClock;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

/**
 * An thread-safe state holder.
 */
public abstract class StateHolder {
    private static final Tag TAG = new Tag(StateHolder.class.getSimpleName());
    private static final long OPERATION_TIMEOUT_MS = 3500;
    private int mState;
    private boolean mInvalid;

    /**
     * Construct a new instance of @{link CameraStateHolder} with an initial state.
     *
     * @param state The initial state.
     */
    public StateHolder(int state) {
        setState(state);
        mInvalid = false;
    }

    /**
     * Change to a new state.
     *
     * @param state The new state.
     */
    public synchronized void setState(int state) {
        if (mState != state) {
            LogHelper.v(TAG, "[setState] - state = " + Integer.toBinaryString(state));
        }
        mState = state;
        this.notifyAll();
    }

    /**
     * Obtain the current state.
     *
     * @return The current state.
     */
    public synchronized int getState() {
        return mState;
    }

    /**
     * Change the state to be invalid. Once invalidated, the state will be invalid forever.
     */
    public synchronized void invalidate() {
        mInvalid = true;
    }

    /**
     * Whether the state is invalid.
     *
     * @return True if the state is invalid.
     */
    public synchronized boolean isInvalid() {
        return mInvalid;
    }

    /**
     * A condition checker class.
     */
    private interface ConditionChecker {
        /**
         * @return Whether the condition holds.
         */
        boolean success();
    }

    /**
     * A helper method used by {@link #waitToAvoidStates(int)} and
     * {@link #waitForStates(int)}. This method will wait until the
     * condition is successful.
     *
     * @param stateChecker The state checker to be used.
     * @param timeoutMs The timeout limit in milliseconds.
     * @return {@code false} if the wait is interrupted or timeout limit is
     *         reached.
     */
    private boolean waitForCondition(ConditionChecker stateChecker,
                                     long timeoutMs) {
        long timeBound = SystemClock.uptimeMillis() + timeoutMs;
        synchronized (this) {
            while (!stateChecker.success()) {
                try {
                    this.wait(timeoutMs);
                } catch (InterruptedException ex) {
                    if (SystemClock.uptimeMillis() > timeBound) {
                        // Timeout.
                        LogHelper.w(TAG, "Timeout waiting.");
                    }
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * Block the current thread until the state becomes one of the
     * specified.
     *
     * @param states Expected states.
     * @return {@code false} if the wait is interrupted or timeout limit is
     *         reached.
     */
    public boolean waitForStates(final int states) {
        LogHelper.v(TAG, "waitForStates - states = " + Integer.toBinaryString(states));
        return waitForCondition(new ConditionChecker() {
            @Override
            public boolean success() {
                return (states | getState()) == states;
            }
        }, OPERATION_TIMEOUT_MS);
    }

    /**
     * Block the current thread until the state becomes NOT one of the
     * specified.
     *
     * @param states States to avoid.
     * @return {@code false} if the wait is interrupted or timeout limit is
     *         reached.
     */
    public boolean waitToAvoidStates(final int states) {
        LogHelper.v(TAG, "waitToAvoidStates - states = " + Integer.toBinaryString(states));
        return waitForCondition(new ConditionChecker() {
            @Override
            public boolean success() {
                return (states & getState()) == 0;
            }
        }, OPERATION_TIMEOUT_MS);
    }
}