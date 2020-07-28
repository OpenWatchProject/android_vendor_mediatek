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

import android.animation.ObjectAnimator;
import android.animation.PropertyValuesHolder;
import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.animation.AccelerateInterpolator;

import com.mediatek.camera.common.widget.RotateImageView;


/**
 * Represent a shutter button UI module. A button designed to be used for the
 * on-screen shutter button. It's currently an {@code ImageView} that can call a
 * delegate when the pressed state changes.
 */
class ShutterButton extends RotateImageView implements View.OnLongClickListener {
    /**
     * A callback to be invoked when a ShutterButton's pressed state changes.
     */
    public interface OnShutterButtonListener {
        /**
         * Called when a ShutterButton has been pressed.
         * @param pressed
         *            The ShutterButton that was pressed.
         */
        void onShutterButtonFocused(boolean pressed);

        /**
         * Called when a shutter button clicked.
         */
        void onShutterButtonClicked();

        /**
         * Called when shutter button is held down for a long press.
         */
        void onShutterButtonLongPressed();
    }

    private OnShutterButtonListener mListener;
    private boolean mOldPressed;

    // this variable to avoid needless onClick after onLongPressed;
    private boolean mLongPressed;

    private ObjectAnimator mScaleAnimation;

    /**
     * constructor of shutterButton.
     * @param context The Context the view is running in, through which it can
     *        access the current theme, resources, etc.
     * @param attrs The attributes of the XML tag that is inflating the view.
     */
    public ShutterButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        setOnLongClickListener(this);
        PropertyValuesHolder scaleX =  PropertyValuesHolder.ofFloat("scaleX", 1.0f, 0.9f);
        PropertyValuesHolder scaleY =  PropertyValuesHolder.ofFloat("scaleY", 1.0f, 0.9f);
        mScaleAnimation = ObjectAnimator.ofPropertyValuesHolder(this, scaleX, scaleY);
        mScaleAnimation.setInterpolator(new AccelerateInterpolator());
        mScaleAnimation.setDuration(60);
    }

    public void setOnShutterButtonListener(OnShutterButtonListener listener) {
        mListener = listener;
    }

    /**
     * Hook into the drawable state changing to get changes to isPressed -- the
     * onPressed listener doesn't always get called when the pressed state
     * changes.
     */
    @Override
    protected void drawableStateChanged() {
        super.drawableStateChanged();

        final boolean pressed = isPressed();
        if (pressed != mOldPressed) {
            if (!pressed) {
                // When pressing the physical camera button the sequence of
                // events is:
                // focus pressed, optional camera pressed, focus released.
                // We want to emulate this sequence of events with the shutter
                // button. When clicking using a trackball button, the view
                // system changes the drawable state before posting click
                // notification, so the sequence of events is:
                // pressed(true), optional click, pressed(false)
                // When clicking using touch events, the view system changes the
                // drawable state after posting click notification, so the
                // sequence of events is:
                // pressed(true), pressed(false), optional click
                // Since we're emulating the physical camera button, we want to
                // have the same order of events. So we want the optional click
                // callback to be delivered before the pressed(false) callback.
                //
                // To do this, we delay the posting of the pressed(false) event
                // slightly by pushing it on the event queue. This moves it
                // after the optional click notification, so our client always
                // sees events in this sequence:
                // pressed(true), optional click, pressed(false)
                post(new Runnable() {
                    @Override
                    public void run() {
                        callShutterButtonFocus(pressed);
                    }
                });
            } else {
                callShutterButtonFocus(pressed);
            }
            mOldPressed = pressed;
        }
    }

    private void callShutterButtonFocus(boolean pressed) {
        if (mListener != null && isEnabled() && isClickable()) {
            mListener.onShutterButtonFocused(pressed);
        }
        mLongPressed = false;
        if (pressed) {
            mScaleAnimation.start();
        } else {
            mScaleAnimation.reverse();
        }
    }

    @Override
    public boolean performClick() {
        boolean result = super.performClick();
        if (mListener != null && isEnabled() && isClickable() && !mLongPressed) {
            mListener.onShutterButtonClicked();
        }
        return result;
    }

    @Override
    public void setEnabled(boolean enabled) {
        if (getAlpha() > 0.9) {
            super.setEnabled(enabled);
        } else {
            super.setEnabled(false);
        }
    }

    @Override
    public boolean onLongClick(View v) {
        if (mListener != null && isEnabled() && isClickable()) {
            mListener.onShutterButtonLongPressed();
            mLongPressed = true;
        }
        return false;
    }
}
