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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.android.soundrecorder;

import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

/**
 * A on-screen hint is a view containing a little message for the user and will
 * be shown on the screen continuously. This class helps you create and show
 * those.
 *
 * <p>
 * When the view is shown to the user, appears as a floating view over the
 * application.
 * <p>
 * The easiest way to use this class is to call one of the static methods that
 * constructs everything you need and returns a new OnScreenHint object.
 */

public class OnScreenHint {
    private final int mY;
    private int mX;
    private float mHorizontalMargin;
    private float mVerticalMargin;
    private View mView;
    private View mNextView;
    private final WindowManager.LayoutParams mParams = new WindowManager.LayoutParams();
    private final WindowManager mWM;
    private final Handler mHandler = new Handler();
    private final Runnable mShow = new Runnable() {
        public void run() {
            handleShow();
        }
    };

    private final Runnable mHide = new Runnable() {
        public void run() {
            handleHide();
        }
    };

    /**
     * Construct an empty OnScreenHint object. You must call {@link #setView}
     * before you can call {@link #show}.
     *
     * @param context
     *            The context to use. Usually your
     *            {@link android.app.Application} or
     *            {@link android.app.Activity} object.
     */
    public OnScreenHint(Context context) {
        mWM = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        mY = context.getResources().getDimensionPixelSize(R.dimen.hint_y_offset);

        mParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
        mParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
        mParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        mParams.format = PixelFormat.TRANSLUCENT;
        mParams.windowAnimations = R.style.Animation_OnScreenHint;
        mParams.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
        mParams.setTitle("OnScreenHint");
    }

    /**
     * Show the view on the screen.
     */
    public void show() {
        if (null == mNextView) {
            throw new RuntimeException("setView must have been called");
        }
        mHandler.post(mShow);
    }

    /**
     * Close the view if it's showing.
     */
    public void cancel() {
        mHandler.post(mHide);
    }

    /**
     * Make a standard hint that just contains a text view.
     *
     * @param context
     *            The context to use. Usually your
     *            {@link android.app.Application} or
     *            {@link android.app.Activity} object.
     * @param text
     *            The text to show. Can be formatted text.
     * @return OnScreenHint an OnScreenHint which has given text
     */
    public static OnScreenHint makeText(Context context, CharSequence text) {
        OnScreenHint result = new OnScreenHint(context);
        LayoutInflater inflate = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View v = inflate.inflate(R.layout.on_screen_hint, null);
        TextView tv = (TextView) v.findViewById(R.id.message);
        tv.setText(text);
        result.mNextView = v;
        return result;
    }

    /**
     * Update the text in a OnScreenHint that was previously created using one
     * of the makeText() methods.
     *
     * @param s
     *            The new text for the OnScreenHint.
     */
    public void setText(CharSequence s) {
        if (null == mNextView) {
            throw new RuntimeException("This OnScreenHint was not "
                    + "created with OnScreenHint.makeText()");
        }
        TextView tv = (TextView) mNextView.findViewById(R.id.message);
        if (null == tv) {
            throw new RuntimeException("This OnScreenHint was not "
                    + "created with OnScreenHint.makeText()");
        }
        tv.setText(s);
    }

    /**
     * show the next view when the mView != mNextView.
     */
    private void handleShow() {
        if (!mNextView.equals(mView)) {
            // remove the old view if necessary
            handleHide();
            mView = mNextView;
            final int gravity = Gravity.CENTER_HORIZONTAL | Gravity.BOTTOM;
            mParams.gravity = gravity;
            mParams.x = mX;
            mParams.y = mY;
            mParams.verticalMargin = mVerticalMargin;
            mParams.horizontalMargin = mHorizontalMargin;
            if (null != mView.getParent()) {
                mWM.removeView(mView);
            }
            mWM.addView(mView, mParams);
        }
    }

    /**
     * hide the current view.
     */
    private void handleHide() {
        if (null != mView) {
            // note: checking parent() just to make sure the view has
            // been added... i have seen cases where we get here when
            // the view isn't yet added, so let's try not to crash.
            if (null != mView.getParent()) {
                mWM.removeView(mView);
            }
            mView = null;
        }
    }

}
