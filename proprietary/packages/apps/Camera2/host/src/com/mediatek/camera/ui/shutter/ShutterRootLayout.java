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
package com.mediatek.camera.ui.shutter;

import android.content.Context;
import android.content.res.Configuration;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.animation.DecelerateInterpolator;
import android.widget.RelativeLayout;
import android.widget.Scroller;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;

/**
 * Shutter button root layout, control the shutter ui layout and scroll animation.
 */
class ShutterRootLayout extends RelativeLayout implements ShutterView.OnShutterTextClicked {
    /**
     * Shutter type change listener.
     */
    public interface OnShutterChangeListener {
        /**
         * When current valid shutter changed, invoke the listener to notify.
         * @param newShutterName The new valid shutter name.
         */
        void onShutterChangedStart(String newShutterName);

        /**
         * When shutter change animation finish, invoke the listener to notify.
         */
        void onShutterChangedEnd();
    }
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
                                        ShutterRootLayout.class.getSimpleName());
    private static final int ANIM_DURATION_MS = 1000;
    private Scroller mScroller;

    private static final int MINI_SCROLL_LENGTH = 100;

    private int mCurrentIndex = 0;
    private int mScrollDistance = 0;

    private OnShutterChangeListener mListener;

    private boolean mResumed = false;

    public void setOnShutterChangedListener(OnShutterChangeListener listener) {
        mListener = listener;
    }

    public IAppUiListener.OnGestureListener getGestureListener() {
        return new GestureListenerImpl();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return true;
    }
    @Override
    protected void onScrollChanged(int l, int t, int oldl, int oldt) {
        super.onScrollChanged(l, t, oldl, oldt);
        View child;
        for (int i = 0; i < getChildCount(); i++) {
            child = getChildAt(i);
            ((ShutterView) child).onScrolled(l, (getWidth() + 1) / 2, mScrollDistance);
        }
    }

    @Override
    public void computeScroll() {
        if (mScroller.computeScrollOffset()) {
            scrollTo(mScroller.getCurrX(), mScroller.getCurrY());
            postInvalidate();
            if (mScroller.isFinished() && mListener != null) {
                mListener.onShutterChangedEnd();
            }
        }
    }

    public ShutterRootLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        mScroller = new Scroller(context, new DecelerateInterpolator());
    }


    @Override
    public void onShutterTextClicked(int index) {
        LogHelper.d(TAG, "onShutterTextClicked index = " + index);
        if (mScroller.isFinished() && isEnabled() && mResumed) {
            snapTOShutter(index, ANIM_DURATION_MS);
        }
    }

    public void updateCurrentShutterIndex(int shutterIndex) {
        doShutterAnimation(shutterIndex, 0);
    }

    public void onResume() {
        mResumed = true;
    }

    public void onPause() {
        mResumed = false;
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        LogHelper.d(TAG, "onLayout() = "
                + changed + "l = " + l + "  t = " + t + " r = " + r + " b = " + b);
        super.onLayout(changed, l, t, r, b);
        updateCurrentShutterIndex(mCurrentIndex);
    }

    private void doShutterAnimation(int whichShutter, int animationDuration) {
        mCurrentIndex = whichShutter;
        if (whichShutter > getChildCount() - 1) {
            mCurrentIndex = getChildCount() - 1;
        }

        int dx = 0;
        if (mCurrentIndex == 0) {
            dx = -getScrollX();
        } else {
            dx = (getChildAt(0).getMeasuredWidth() + getChildAt(1).getMeasuredWidth() + 1) / 2
                    - getScrollX();
        }
        mScroller.startScroll(getScrollX(), 0, dx, 0, animationDuration);
        mScrollDistance = Math.abs(dx);
        invalidate();
    }

    private void snapTOShutter(int whichShutter, int animationDuration) {
        if (whichShutter == mCurrentIndex) {
            return;
        }
        doShutterAnimation(whichShutter, animationDuration);
        if (mListener != null) {
            ShutterView shutter = (ShutterView) getChildAt(mCurrentIndex);
            mListener.onShutterChangedStart(shutter.getType());
        }
    }

    /**
     * Gesture listener implementer.
     */
    private class GestureListenerImpl implements IAppUiListener.OnGestureListener {

        private float mTransitionX;
        private float mTransitionY;
        private boolean mIsScale;

        @Override
        public boolean onDown(MotionEvent event) {
            mTransitionX = 0;
            mTransitionY = 0;
            return false;
        }

        @Override
        public boolean onUp(MotionEvent event) {
            mTransitionX = 0;
            mTransitionY = 0;
            return false;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            return false;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float dx, float dy) {
            if (e2.getPointerCount() > 1) {
                return false;
            }
            if (getChildCount() < 2) {
                return false;
            }
            if (mIsScale) {
                return false;
            }
            if (mScroller.isFinished() && isEnabled() && mResumed) {
                mTransitionX += dx;
                mTransitionY += dy;

                Configuration config = getResources().getConfiguration();
                if (config.orientation == Configuration.ORIENTATION_PORTRAIT) {
                    if (Math.abs(mTransitionX) > MINI_SCROLL_LENGTH
                            && Math.abs(mTransitionY) < Math.abs(mTransitionX)) {
                        if (mTransitionX > 0 && mCurrentIndex < (getChildCount() - 1)) {
                            if (getVisibility() != VISIBLE) {
                                return false;
                            }
                            if (getChildAt(mCurrentIndex + 1).getVisibility() != VISIBLE) {
                                return false;
                            }
                            snapTOShutter(mCurrentIndex + 1, ANIM_DURATION_MS);
                        } else if (mTransitionX < 0 && mCurrentIndex > 0) {
                            if (getVisibility() != VISIBLE) {
                                return false;
                            }
                            if (getChildAt(mCurrentIndex - 1).getVisibility() != VISIBLE) {
                                return false;
                            }
                            snapTOShutter(mCurrentIndex - 1, ANIM_DURATION_MS);
                        }
                        return true;
                    }
                } else if (config.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                    if (Math.abs(mTransitionY) > MINI_SCROLL_LENGTH
                            && Math.abs(mTransitionX) < Math.abs(mTransitionY)) {
                        if (mTransitionY < 0 && mCurrentIndex < (getChildCount() - 1)) {
                            if (getChildAt(mCurrentIndex + 1).getVisibility() != VISIBLE) {
                                return false;
                            }
                            snapTOShutter(mCurrentIndex + 1, ANIM_DURATION_MS);
                        } else if (mTransitionY > 0 && mCurrentIndex > 0) {
                            if (getChildAt(mCurrentIndex - 1).getVisibility() != VISIBLE) {
                                return false;
                            }
                            snapTOShutter(mCurrentIndex - 1, ANIM_DURATION_MS);
                        }
                    }
                }
                return false;
            } else {
                return true;
            }
        }

        @Override
        public boolean onSingleTapUp(float x, float y) {
            return false;
        }

        @Override
        public boolean onSingleTapConfirmed(float x, float y) {
            return false;
        }

        @Override
        public boolean onDoubleTap(float x, float y) {
            return false;
        }

        @Override
        public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
            return false;
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
            mIsScale = true;
            return false;
        }

        @Override
        public boolean onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
            mIsScale = false;
            return false;
        }

        @Override
        public boolean onLongPress(float x, float y) {
            return false;
        }
    }

    // [Add for CCT tool] Receive keycode and switch photo/video mode @{
    public IApp.KeyEventListener getKeyEventListener() {
        return new KeyEventListenerImpl();
    }

    private class KeyEventListenerImpl implements IApp.KeyEventListener {

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {
            if ((keyCode != CameraUtil.KEYCODE_SWITCH_TO_PHOTO
                    && keyCode != CameraUtil.KEYCODE_SWITCH_TO_VIDEO)
                    || !CameraUtil.isSpecialKeyCodeEnabled()) {
                return false;
            }
            return true;
        }

        @Override
        public boolean onKeyUp(int keyCode, KeyEvent event) {
            if (!CameraUtil.isSpecialKeyCodeEnabled()) {
                return false;
            }
            if ((keyCode != CameraUtil.KEYCODE_SWITCH_TO_PHOTO
                    && keyCode != CameraUtil.KEYCODE_SWITCH_TO_VIDEO)) {
                return false;
            }
            if (getChildCount() < 2) {
                LogHelper.w(TAG, "onKeyUp no need to slide betwwen photo mode and video mode," +
                        "one mode olny");
                return false;
            }
            if (keyCode == CameraUtil.KEYCODE_SWITCH_TO_PHOTO
                    && getChildCount() == 2
                    && getChildAt(0).getVisibility() == View.VISIBLE
                    && getChildAt(1).getVisibility() == View.VISIBLE) {
                onShutterTextClicked(0);
            } else if (keyCode == CameraUtil.KEYCODE_SWITCH_TO_VIDEO
                    && getChildCount() == 2
                    && getChildAt(0).getVisibility() == View.VISIBLE
                    && getChildAt(1).getVisibility() == View.VISIBLE) {
                onShutterTextClicked(1);
            }
            return true;
        }
    }
    // @}
}
