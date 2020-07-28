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

package com.mediatek.camera.gesture;


import android.content.Context;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

import javax.annotation.Nonnull;

/**
 *  This class aggregates three gesture detectors: GestureDetector,
 *  ScaleGestureDetector, and UpDetector.
 */
public class GestureRecognizer {

    /**
     * Gesture recognizer listener, supply support gesture events to user.
     */
    public interface  Listener extends GestureDetector.OnGestureListener,
                                       GestureDetector.OnDoubleTapListener,
                                       ScaleGestureDetector.OnScaleGestureListener,
                                       DownUpDetector.DownUpListener {

    }

    private Listener mListener;
    private final GestureDetector mGestureDetector;
    private DownUpDetector mDownUpDetector;
    private final ScaleGestureDetector mScaleDetector;


    /**
     * Creates a GestureRecognizer with the supplied listener.
     *
     * @param context the application's context
     * @param listener the listener invoked for all the callbacks, this must
     * not be null.
     */
    public GestureRecognizer(@Nonnull Context context, @Nonnull Listener listener) {
        mListener = listener;
        mGestureDetector = new GestureDetector(context, new GestureListenerImpl(), null, true);
        mScaleDetector = new ScaleGestureDetector(context, new ScaleListenerImpl());
        mDownUpDetector = new DownUpDetector(new DownUpListenerImpl());
    }


    /**
     * Analyzes the given motion event and if applicable triggers the
     * appropriate callbacks on the {@link Listener} supplied.
     *
     * @param event The current motion event.
     */
    public void onTouchEvent(MotionEvent event) {
        mGestureDetector.onTouchEvent(event);
        mScaleDetector.onTouchEvent(event);
        mDownUpDetector.onTouchEvent(event);
    }

    /**
     * Gesture detector listener implementor.
     */
    private class GestureListenerImpl extends GestureDetector.SimpleOnGestureListener {

        @Override
        public boolean onSingleTapUp(MotionEvent e) {
            return mListener.onSingleTapUp(e);
        }

        @Override
        public void onLongPress(MotionEvent e) {
            mListener.onLongPress(e);
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            return mListener.onScroll(e1, e2, distanceX, distanceY);
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            return mListener.onFling(e1, e2, velocityX, velocityY);
        }

        @Override
        public void onShowPress(MotionEvent e) {
            mListener.onShowPress(e);
        }

        @Override
        public boolean onDown(MotionEvent e) {
            //Use DownUpDetector's onDownEvent() instead of the onDown() event.
            return mListener.onDown(e);
        }

        @Override
        public boolean onDoubleTap(MotionEvent e) {
            return mListener.onDoubleTap(e);
        }

        @Override
        public boolean onDoubleTapEvent(MotionEvent e) {
            return mListener.onDoubleTapEvent(e);
        }

        @Override
        public boolean onSingleTapConfirmed(MotionEvent e) {
            return mListener.onSingleTapConfirmed(e);
        }
    }

    /**
     * Scale gesture detector listener implementor.
     */
    private class ScaleListenerImpl extends ScaleGestureDetector.SimpleOnScaleGestureListener {

        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            return mListener.onScale(detector);
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector detector) {
            return mListener.onScaleBegin(detector);
        }

        @Override
        public void onScaleEnd(ScaleGestureDetector detector) {
            mListener.onScaleEnd(detector);
        }
    }

    /**
     * DownUp gesture detector listener implementor.
     */
    private class DownUpListenerImpl implements DownUpDetector.DownUpListener {

        @Override
        public void onDownEvent(MotionEvent e) {
            mListener.onDownEvent(e);
        }

        @Override
        public void onUpEvent(MotionEvent e) {
            mListener.onUpEvent(e);
        }
    }
}
