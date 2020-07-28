/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.contacts.simservice;

import android.content.Intent;

import com.android.contacts.vcard.ProcessorBase;

import com.mediatek.contacts.simservice.SimProcessorManager.ProcessorCompleteListener;
import com.mediatek.contacts.util.Log;

public abstract class SimProcessorBase extends ProcessorBase {
    private final static String TAG = "SIMProcessorBase";

    private volatile boolean mCanceled;
    private volatile boolean mDone;

    protected ProcessorCompleteListener mListener;
    protected Intent mIntent;

    public SimProcessorBase(Intent intent, ProcessorCompleteListener listener) {
        mIntent = intent;
        mListener = listener;
    }

    @Override
    public int getType() {
        return 0;
    }

    @Override
    public void run() {
        try {
            doWork();
        } finally {
            Log.d(TAG, "[run]finish: type = " + getType() + ",mDone = " + mDone
                    + ",thread id = " + Thread.currentThread().getId());
            mDone = true;
            if (mListener != null && !mCanceled) {
                mListener.onProcessorCompleted(mIntent);
            }
        }
    }

    public abstract void doWork();

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        if (mDone || mCanceled) {
            return false;
        }
        mCanceled = true;

        return true;
    }

    @Override
    public boolean isCancelled() {
        return mCanceled;
    }

    @Override
    public boolean isDone() {
        return mDone;
    }

    public boolean isRunning() {
        return !isDone() && !isCancelled();
    }

    /**
     * [ALPS01224227]for race condition, we should make sure the processor to be
     * removed is the instance itself
     *
     * @param intent
     *            to check whether it is the same as mIntent
     * @return
     */
    public boolean identifyIntent(Intent intent) {
        return mIntent.equals(intent);
    }
}
