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

package com.mediatek.camera.feature.setting;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * This class is usd for continuous shot state manager.
 */

public class CsState {

    private static final LogUtil.Tag TAG = new LogUtil.Tag(CsState.class.getSimpleName());
    private State mCShotState = State.STATE_INIT;
    private final Object mStateSync = new Object();

    /**
     * This is usd for tag continuous shot state.
     */
    enum State {
        STATE_INIT,
        STATE_CAPTURE_STARTED,
        STATE_CAPTURING,
        STATE_STOPPED,
        STATE_ERROR
    }

    /**
     * Set a new CS state.
     * @param newState new CS state.
     */
    void updateState(State newState) {
        checkState(getCShotState(), newState);
        setCShotState(newState);
    }

    /**
     * Get current CS state.
     * @return current state.
     */
    State getCShotState() {
        synchronized (mStateSync) {
            return mCShotState;
        }
    }

    private void setCShotState(State state) {
        synchronized (mStateSync) {
            mCShotState = state;
        }
    }

    private void checkState(State oldState, State newState) {
        if (oldState == newState) {
            return;
        }
        switch (newState) {
            case STATE_CAPTURE_STARTED:
                // OPENED --> CAPTURE_STARTED when start continuous shot.
                if (oldState != State.STATE_INIT) {
                    LogHelper.e(TAG, "[checkState]Error!");
                }
                break;
            case STATE_CAPTURING:
                // CAPTURE_STARTED --> CAPTURING when takePicture.
                if (oldState != State.STATE_CAPTURE_STARTED) {
                    LogHelper.e(TAG, "[checkState]Error!");
                }
                break;
            case STATE_STOPPED:
                // CAPTURING --> STOPPED when stop continuous shot.
                if (oldState != State.STATE_CAPTURING) {
                    LogHelper.e(TAG, "[checkState]Error!");
                }
                break;
            default:
                break;
        }
    }
}
