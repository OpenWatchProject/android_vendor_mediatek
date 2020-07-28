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

/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

import android.app.Activity;
import android.util.FloatMath;
import android.util.Log;
import android.view.MotionEvent;

/** M:
 * only first and second finger can pinch. if finger num > 2,
 * the gesture stop. if only one finger stay, you can
 * start with another finger down.
 *
 */
public class ScaleDetector {

    public interface OnScaleListener {

        /**
         * when the ScaleGesture start, onScaleStart will be called.
         * if return false, the onScale()and onScaleEnd will
         * not be called
         *
         * @return boolean
         */
        public boolean onScaleStart(ScaleDetector detector);

        /**
         *
         */
        public void onScaleEnd(ScaleDetector detector);

        /**
         * when two touch point the length changed,
         *
         * @param factor
         *            is the rate that the length of current two touch
         *            point and length of two touch point when
         *            MotionDown
         */
        public boolean onScale(ScaleDetector detector);
    }

    private static String LOGTAG = "ScaleDetector";

    /**
     * This value is the threshold ratio between our previous combined
     * pressure and the current combined pressure. We
     * will only fire an onScale event if the computed ratio between
     * the current and previous event pressures is greater
     * than this value. When pressure decreases rapidly between events
     * the position values can often be imprecise, as it
     * usually indicates that the user is in the process of lifting a
     * pointer off of the device. Its value was tuned
     * experimentally.
     */
    private static final float PRESSURE_THRESHOLD = 0.67f;

    // Pointer IDs currently responsible for the two fingers controlling the gesture
    private int mActiveId0;
    private int mActiveId1;

    private boolean mGestureInProgress;

    // when points > 3 and the first/second finger up, the gesture is invalid.
    private boolean mInvalidGesture;

    private MotionEvent mPrevEvent;
    private MotionEvent mCurrEvent;
    private float mPrevFingerDiffX;
    private float mPrevFingerDiffY;
    private float mCurrFingerDiffX;
    private float mCurrFingerDiffY;
    private float mCurrLen;
    private float mPrevLen;
    private float mScaleFactor;
    private float mCurrPressure;
    private float mPrevPressure;

    private final OnScaleListener mListener;
    private final Activity mActivity;

    private void log(String msg) {
        Log.e(LOGTAG, msg);
    }

    public ScaleDetector(Activity activity, OnScaleListener listener) {
        mActivity = activity;
        mListener = listener;
        reset();
    }

    public boolean onTouchEvent(MotionEvent event) {

        boolean ret = false;

        final int action = event.getActionMasked();

        if (action == MotionEvent.ACTION_DOWN) {
            reset(); // Start fresh
        }

        switch (action) {

            case MotionEvent.ACTION_DOWN:
                mActiveId0 = event.getPointerId(0);
                log("ACTION_DOWN: count = " + event.getPointerCount());
                break;

            case MotionEvent.ACTION_POINTER_DOWN: {
                int count = event.getPointerCount();
                int index = event.getActionIndex();
                int id = event.getPointerId(index);
                log("ACTION_POINTER_DOWN: count = " + count + ", actionId = " + id);

                if (count == 2) {
                    mActiveId0 = event.getPointerId(0);
                    mActiveId1 = event.getPointerId(1);

                    mPrevEvent = MotionEvent.obtain(event);
                    setContext(event);

                    if (mListener != null) {
                        mGestureInProgress = mListener.onScaleStart(this);
                        if (mGestureInProgress) {
                            // send ACTION_CANCEL to cancel previous
                            // actions before ACTION_POINTER_DOWN
                            MotionEvent cancle = MotionEvent.obtain(0, 0,
                                MotionEvent.ACTION_CANCEL, 0, 0, 0);
                            mActivity.getWindow().superDispatchTouchEvent(cancle);
                        }
                    }
                    mInvalidGesture = false;
                }

                if (count > 2 && !mInvalidGesture) {
                    mInvalidGesture = true;
                    setContext(event);
                    if (mGestureInProgress && mListener != null) {
                        mListener.onScaleEnd(this);
                    }
                }
                break;
            }

            case MotionEvent.ACTION_MOVE:
                if (mPrevEvent != null && mGestureInProgress && !mInvalidGesture) {
                    setContext(event);

                    // Only accept the event if our relative pressure is within
                    // a certain limit - this can help filter shaky data as a
                    // finger is lifted.
                    if (mCurrPressure / mPrevPressure > PRESSURE_THRESHOLD) {
                        final boolean updatePrevious = mListener.onScale(this);

                        if (updatePrevious) {
                            mPrevEvent.recycle();
                            mPrevEvent = MotionEvent.obtain(event);
                        }
                    }
                }
                break;

            case MotionEvent.ACTION_POINTER_UP: {
                int count = event.getPointerCount();
                int index_ = event.getActionIndex();
                int id_ = event.getPointerId(index_);
                log("ACTION_POINTER_UP, count = " + count + ", ActionId = " + id_);

                if (mPrevEvent != null && mGestureInProgress && count == 2 && !mInvalidGesture) {
                    setContext(event);
                    if (mListener != null) {
                        mListener.onScaleEnd(this);
                    }
                    mInvalidGesture = true;
                }
                break;
            }

            case MotionEvent.ACTION_UP:
                log("ACTION_UP");
                reset();
                break;

            case MotionEvent.ACTION_CANCEL:
                log("ACTION_CANCEL");
                reset();
                break;
        }

        if (mGestureInProgress == false) {
            log("return value is false, action = " + event.getActionMasked());
        }

        // return true;
        return mGestureInProgress;
    }

    private void reset() {

        if (mPrevEvent != null) {
            mPrevEvent.recycle();
            mPrevEvent = null;
        }
        if (mCurrEvent != null) {
            mCurrEvent.recycle();
            mCurrEvent = null;
        }

        mActiveId0 = -1;
        mActiveId1 = -1;
        mGestureInProgress = false;
        mInvalidGesture = false;
    }

    private void setContext(MotionEvent curr) {
        if (mCurrEvent != null) {
            mCurrEvent.recycle();
        }
        mCurrEvent = MotionEvent.obtain(curr);

        mCurrLen = -1;
        mPrevLen = -1;
        mScaleFactor = -1;

        final MotionEvent prev = mPrevEvent;

        final int prevIndex0 = prev.findPointerIndex(mActiveId0);
        final int prevIndex1 = prev.findPointerIndex(mActiveId1);
        final int currIndex0 = curr.findPointerIndex(mActiveId0);
        final int currIndex1 = curr.findPointerIndex(mActiveId1);

        if (prevIndex0 < 0 || prevIndex1 < 0 || currIndex0 < 0 || currIndex1 < 0) {
            mInvalidGesture = true;
            if (mGestureInProgress) {
                mListener.onScaleEnd(this);
            }
            return;
        }

        final float px0 = prev.getX(prevIndex0);
        final float py0 = prev.getY(prevIndex0);
        final float px1 = prev.getX(prevIndex1);
        final float py1 = prev.getY(prevIndex1);
        final float cx0 = curr.getX(currIndex0);
        final float cy0 = curr.getY(currIndex0);
        final float cx1 = curr.getX(currIndex1);
        final float cy1 = curr.getY(currIndex1);

        final float pvx = px1 - px0;
        final float pvy = py1 - py0;
        final float cvx = cx1 - cx0;
        final float cvy = cy1 - cy0;
        mPrevFingerDiffX = pvx;
        mPrevFingerDiffY = pvy;
        mCurrFingerDiffX = cvx;
        mCurrFingerDiffY = cvy;

        mCurrPressure = curr.getPressure(currIndex0) + curr.getPressure(currIndex1);
        mPrevPressure = prev.getPressure(prevIndex0) + prev.getPressure(prevIndex1);
    }

    /**
     * Return the current distance between the two pointers forming the gesture in progress.
     *
     * @return Distance between pointers in pixels.
     */
    public float getCurrentSpan() {
        if (mCurrLen == -1) {
            final float cvx = mCurrFingerDiffX;
            final float cvy = mCurrFingerDiffY;
            mCurrLen = FloatMath.sqrt(cvx * cvx + cvy * cvy);
        }
        return mCurrLen;
    }

    /**
     * Return the current x distance between the two pointers forming the gesture in progress.
     *
     * @return Distance between pointers in pixels.
     */
    public float getCurrentSpanX() {
        return mCurrFingerDiffX;
    }

    /**
     * Return the current y distance between the two pointers forming the gesture in progress.
     *
     * @return Distance between pointers in pixels.
     */
    public float getCurrentSpanY() {
        return mCurrFingerDiffY;
    }

    /**
     * Return the previous distance between the two pointers forming the gesture in progress.
     *
     * @return Previous distance between pointers in pixels.
     */
    public float getPreviousSpan() {
        if (mPrevLen == -1) {
            final float pvx = mPrevFingerDiffX;
            final float pvy = mPrevFingerDiffY;
            mPrevLen = FloatMath.sqrt(pvx * pvx + pvy * pvy);
        }
        return mPrevLen;
    }

    /**
     * Return the previous x distance between the two pointers forming the gesture in progress.
     *
     * @return Previous distance between pointers in pixels.
     */
    public float getPreviousSpanX() {
        return mPrevFingerDiffX;
    }

    /**
     * Return the previous y distance between the two pointers forming the gesture in progress.
     *
     * @return Previous distance between pointers in pixels.
     */
    public float getPreviousSpanY() {
        return mPrevFingerDiffY;
    }

    /**
     * Return the scaling factor from the previous scale event
     * to the current event. This value is defined as (
     * {@link #getCurrentSpan()} / {@link #getPreviousSpan()}).
     *
     * @return The current scaling factor.
     */
    public float getScaleFactor() {
        if (mScaleFactor == -1) {
            mScaleFactor = getCurrentSpan() / getPreviousSpan();
        }
        return mScaleFactor;
    }

}
