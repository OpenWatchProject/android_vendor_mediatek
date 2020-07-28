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
package com.android.mms.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.GestureDetector.OnGestureListener;
import android.widget.ScrollView;

import com.android.mms.MmsApp;
import com.android.mms.util.MmsLog;


/** M:
 * Dialog mode
 */
public class MsgContentSlideView extends ScrollView implements OnGestureListener {
    private GestureDetector mDetector;
    private MsgContentSlideListener mFlingListener;

    public MsgContentSlideView(Context context) {
        super(context);
        mDetector = new GestureDetector(this);
        mFlingListener = null;
    }

    public MsgContentSlideView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mDetector = new GestureDetector(this);
        mFlingListener = null;
    }

    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        int xStart;
        int xEnd;
        int flingDist;
        int myWidth;
        int yStart;
        int yEnd;
        int minDeltaX;

        MmsLog.d(MmsApp.TXN_TAG, "MsgContentSlideView.onFling");

        if (mFlingListener == null) {
            MmsLog.d(MmsApp.TXN_TAG, "MsgNumSlideView.onFling, no listener");
            return true;
        }

        if (e1 == null || e2 == null) {
            if (e1 == null) {
                MmsLog.d(MmsApp.TXN_TAG, "e1 null");
            } else {
                MmsLog.d(MmsApp.TXN_TAG, "e2 null");
            }
            return false;
        }
        xStart = (int) e1.getX();
        xEnd = (int) e2.getX();
        flingDist = xStart - xEnd;
        myWidth = getWidth();
        minDeltaX = myWidth / 10;

        MmsLog.d(MmsApp.TXN_TAG, "e1=" + xStart  + "e2=" + xEnd);

        if (Math.abs(xEnd - xStart) <= minDeltaX) {
            return false;
        }

        if (flingDist > 0 && flingDist > myWidth / 3) {
            MmsLog.d(MmsApp.TXN_TAG, "left");
            mFlingListener.onSlideToNext();
        } else if (flingDist < 0 && (-flingDist) > myWidth / 3) {
            MmsLog.d(MmsApp.TXN_TAG, "right");
            mFlingListener.onSlideToPrev();
        }

        return true;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        boolean ret = false;
        MmsLog.d(MmsApp.TXN_TAG, "MsgContentSlideView.onTouchEvent");
        super.onTouchEvent(event);
        this.mDetector.onTouchEvent(event);
        return true;
    }

    public boolean onDown(MotionEvent e) {
       MmsLog.d(MmsApp.TXN_TAG, "MsgContentSlideView.onDown");
       return true;
    }

    public void onLongPress(MotionEvent e) {
        MmsLog.d(MmsApp.TXN_TAG, "MsgContentSlideView.onLongPress");

    }

    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
            float distanceY) {
        MmsLog.d(MmsApp.TXN_TAG, "MsgContentSlideView.onScroll");
        return false;
    }

    public void onShowPress(MotionEvent e) {
        MmsLog.d(MmsApp.TXN_TAG, "MsgContentSlideView.onShowPress");
    }

    public boolean onSingleTapUp(MotionEvent e) {
        MmsLog.d(MmsApp.TXN_TAG, "MsgContentSlideView.onSingleTapUp");
        return false;
    }

    public void registerFlingListener(MsgContentSlideListener listener) {
        mFlingListener = listener;
    }

    public void unregisterFlingListener() {
        mFlingListener = null;
    }

    public interface MsgContentSlideListener {
        void onSlideToPrev();
        void onSlideToNext();
    }
}