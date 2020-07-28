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
package com.mediatek.contacts.widget;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.mediatek.contacts.util.Log;

public class WaitCursorView {
    private static final String TAG = "WaitCursorView";

    private static final int WAIT_CURSOR_START = 1230;
    private static final long WAIT_CURSOR_DELAY_TIME = 500;
    private TextView mLoadingText;

    private ProgressBar mProgress;

    private View mLoadingContainer;
    private boolean mIsFinished = false;
    private Context mContext;

    public WaitCursorView(Context context, View loadingContainer, ProgressBar progress,
            TextView loadingText) {
        mContext = context;
        mLoadingContainer = loadingContainer;
        mProgress = progress;
        mLoadingText = loadingText;
    }

    public void startWaitCursor() {
        mIsFinished = false;
        mHandler.sendMessageDelayed(mHandler.obtainMessage(WAIT_CURSOR_START),
                WAIT_CURSOR_DELAY_TIME);
    }

    public void stopWaitCursor() {
        mIsFinished = true;
        mLoadingContainer.startAnimation(AnimationUtils.loadAnimation(mContext,
                android.R.anim.fade_out));
        mLoadingContainer.setVisibility(View.GONE);
        mLoadingText.setVisibility(View.GONE);
        mProgress.setVisibility(View.GONE);
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "[handleMessage] msg==== " + msg.what);

            switch (msg.what) {

            case WAIT_CURSOR_START:
                Log.i(TAG, "[handleMessage] !isFinished : " + !mIsFinished);
                if (!mIsFinished) {
                    mLoadingContainer.setVisibility(View.VISIBLE);
                    mLoadingText.setVisibility(View.VISIBLE);
                    mProgress.setVisibility(View.VISIBLE);
                }
                break;

            default:
                break;
            }
        }
    };
}
