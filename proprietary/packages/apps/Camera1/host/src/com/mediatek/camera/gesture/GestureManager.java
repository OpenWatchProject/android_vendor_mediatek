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
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

import com.mediatek.camera.common.IAppUiListener.OnGestureListener;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.utils.PriorityConcurrentSkipListMap;

import java.util.Iterator;
import java.util.Map;

/**
 * Gesture manager.
 */
public class GestureManager {

    private static final Tag TAG = new Tag(GestureManager.class.getSimpleName());
    private GestureRecognizer  mGestureRecognizer;
    private GestureNotifier mGestureNotifier;

    private PriorityConcurrentSkipListMap<Integer, OnGestureListener> mGestureListeners =
            new PriorityConcurrentSkipListMap<>(true);

    private View.OnTouchListener mTouchListener = new View.OnTouchListener() {

        @Override
        public boolean onTouch(View view, MotionEvent motionEvent) {
            mGestureRecognizer.onTouchEvent(motionEvent);
            return true;
        }
    };

    /**
     * Constructor for GestureManager.
     * @param context The activity context.
     */
    public GestureManager(Context context) {
        mGestureRecognizer = new GestureRecognizer(context, new GestureListenerImpl());
        mGestureNotifier = new GestureNotifier();
    }

    /**
     * Register gesture listener.
     * @param listener The gesture listener.
     * @param priority The listener priority.
     */
    public void registerGestureListener(OnGestureListener listener, int priority) {
        if (listener == null) {
            LogHelper.e(TAG, "registerGestureListener error [why null]");
        }
        mGestureListeners.put(mGestureListeners.getPriorityKey(priority, listener),
                listener);
    }
    /**
     * Unregister gesture listener.
     * @param listener The gesture listener.
     */
    public void unregisterGestureListener(OnGestureListener listener) {
        if (listener == null) {
            LogHelper.e(TAG, "unregisterGestureListener error [why null]");
        }
        if (mGestureListeners.containsValue(listener)) {
            mGestureListeners.remove(mGestureListeners.findKey(listener));
        }
    }

    /**
     * The {@link android.view.View.OnTouchListener} is used to receive gesture event.
     * @return Touch listener.
     */
    public View.OnTouchListener getOnTouchListener() {
        return mTouchListener;
    }

    /**
     * A Notifier, used to notify gesture event to the users.
     */
    private class GestureNotifier implements OnGestureListener {

        @Override
        public boolean onDown(MotionEvent event) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onDown(event)) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean onUp(MotionEvent event) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onUp(event)) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onFling(e1, e2, velocityX, velocityY)) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float dx, float dy) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onScroll(e1, e2, dx, dy)) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean onSingleTapUp(float x, float y) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onSingleTapUp(x, y)) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean onSingleTapConfirmed(float x, float y) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onSingleTapConfirmed(x, y)) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean onDoubleTap(float x, float y) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onDoubleTap(x, y)) {
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onScale(scaleGestureDetector)) {
                    return true;
                }
            }
            return true;
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onScaleBegin(scaleGestureDetector)) {
                    return true;
                }
            }
            return true;
        }

        @Override
        public boolean onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onScaleEnd(scaleGestureDetector)) {
                    return true;
                }
            }
            return true;
        }

        @Override
        public boolean onLongPress(float x, float y) {
            Iterator iterator = mGestureListeners.entrySet().iterator();
            OnGestureListener listener;
            while (iterator.hasNext()) {
                Map.Entry map = (Map.Entry) iterator.next();
                listener = (OnGestureListener) map.getValue();
                if (listener != null && listener.onLongPress(x, y)) {
                    return true;
                }
            }
            return false;
        }
    }


    /**
     * GestureRecognizer listener impl, use notifier to broadcast gesture event.
     */
    private class GestureListenerImpl implements GestureRecognizer.Listener {

        @Override
        public boolean onSingleTapConfirmed(MotionEvent motionEvent) {
            return mGestureNotifier.onSingleTapConfirmed(motionEvent.getX(), motionEvent.getY());
        }

        @Override
        public boolean onDoubleTap(MotionEvent motionEvent) {
            return mGestureNotifier.onDoubleTap(motionEvent.getX(), motionEvent.getY());
        }

        @Override
        public boolean onDoubleTapEvent(MotionEvent motionEvent) {
            return mGestureNotifier.onDoubleTap(motionEvent.getX(), motionEvent.getY());
        }

        @Override
        public boolean onDown(MotionEvent motionEvent) {
            return mGestureNotifier.onDown(motionEvent);
        }

        @Override
        public void onShowPress(MotionEvent motionEvent) {

        }

        @Override
        public boolean onSingleTapUp(MotionEvent motionEvent) {
            return mGestureNotifier.onSingleTapUp(motionEvent.getX(), motionEvent.getY());
        }

        @Override
        public boolean onScroll(MotionEvent motionEvent,
                                    MotionEvent motionEvent1, float dx, float dy) {
            return mGestureNotifier.onScroll(motionEvent, motionEvent1, dx, dy);
        }

        @Override
        public void onLongPress(MotionEvent motionEvent) {
            mGestureNotifier.onLongPress(motionEvent.getX(), motionEvent.getY());
        }

        @Override
        public boolean onFling(MotionEvent motionEvent,
                                   MotionEvent motionEvent1, float v, float v1) {
            return mGestureNotifier.onFling(motionEvent, motionEvent1, v, v1);
        }

        @Override
        public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
            return mGestureNotifier.onScale(scaleGestureDetector);
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
            return mGestureNotifier.onScaleBegin(scaleGestureDetector);
        }

        @Override
        public void onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
            mGestureNotifier.onScaleEnd(scaleGestureDetector);
        }

        /**
         * Notified when a tap occurs with the up {@link MotionEvent}
         * that triggered it. This will be triggered once for down event.
         *
         * @param event The up motion event
         */
        @Override
        public void onUpEvent(MotionEvent event) {
            mGestureNotifier.onUp(event);
        }

        /**
         * Notified when a tap occurs with the down {@link MotionEvent}
         * that triggered it. This will be triggered once for down event.
         *
         * @param event The up motion event
         */
        @Override
        public void onDownEvent(MotionEvent event) {
            mGestureNotifier.onDown(event);
        }
    }
}
