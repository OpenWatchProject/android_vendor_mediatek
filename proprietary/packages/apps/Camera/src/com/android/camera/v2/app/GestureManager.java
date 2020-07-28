/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.android.camera.v2.app;

import java.util.ArrayList;

import android.view.MotionEvent;

import com.android.camera.v2.ui.PreviewStatusListener.OnGestureListener;
import com.android.camera.v2.ui.PreviewStatusListener.OnPreviewTouchedListener;

/**
 *
 */
public abstract class GestureManager {
    protected final GestureNotifier                    mGestureNotifier = new GestureNotifier();
    private final ArrayList<OnPreviewTouchedListener>  mPreviewTouchListeners =
            new ArrayList<OnPreviewTouchedListener>();
    private OnGestureListener                          mPreviewGestureListener;
    //TODO consider a best way intercept other touch events
    private boolean                                    mTouchEventsNeedintercept;
    private boolean                                    mScrollEnable = true;
    protected int                                      mGsensorOrientation = 0;

    public void registerPreviewTouchListener(OnPreviewTouchedListener listener) {
        if (listener != null && !mPreviewTouchListeners.contains(listener)) {
            mPreviewTouchListeners.add(listener);
        }
    }

    public void unRegisterPreviewTouchListener(OnPreviewTouchedListener listener) {
        if (listener != null && mPreviewTouchListeners.contains(listener)) {
            mPreviewTouchListeners.remove(listener);
        }
    }

    public void setPreviewGestureListener(OnGestureListener gestureListener) {
        mPreviewGestureListener = gestureListener;
    }

    public void setScrollEnable(boolean enabled) {
        mScrollEnable = enabled;
    }

    public void onOrientationChanged(int newOrientation) {
        mGsensorOrientation = newOrientation;
    }

    protected class GestureNotifier {
        public GestureNotifier() {
        }

        public boolean onDown(float x, float y) {
            boolean interceptEvent = false;
            for (OnPreviewTouchedListener touchListener : mPreviewTouchListeners) {
                mTouchEventsNeedintercept = touchListener.onPreviewTouched() ||
                        mTouchEventsNeedintercept;
            }
            if (mTouchEventsNeedintercept) {
                return true;
            }
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onDown(x, y) || interceptEvent;
            }
            return interceptEvent;
        }

        public boolean onUp() {
            // restore touch event status
            mTouchEventsNeedintercept = false;
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onUp();
            }
            return interceptEvent;
        }

        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onFling(e1, e2, velocityX, velocityY);
            }
            return interceptEvent || !mScrollEnable;
        }

        public boolean onScroll(float dx, float dy, float totalX, float totalY) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onScroll(dx, dy, totalX, totalY);
            }
            return interceptEvent || !mScrollEnable;
        }

        public boolean onSingleTapUp(float x, float y) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onSingleTapUp(x, y);
            }
            return interceptEvent;
        }

        public boolean onSingleTapConfirmed(float x, float y) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onSingleTapConfirmed(x, y);
            }
            return interceptEvent;
        }

        public boolean onDoubleTap(float x, float y) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onDoubleTap(x, y);
            }
            return interceptEvent;
        }

        public boolean onScale(float focusX, float focusY, float scale) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onScale(focusX, focusY, scale);
            }
            return interceptEvent;
        }

        public boolean onScaleBegin(float focusX, float focusY) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onScaleBegin(focusX, focusY);
            }
            return interceptEvent;
        }

        public boolean onLongPress(float x, float y) {
            if (mTouchEventsNeedintercept) {
                return true;
            }
            boolean interceptEvent = false;
            if (mPreviewGestureListener != null) {
                interceptEvent = mPreviewGestureListener.onLongPress(x, y);
            }
            return interceptEvent;
        }
    }
}