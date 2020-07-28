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

package com.mediatek.camera.mode.facebeauty;

import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.WindowManager.BadTokenException;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.camera.R;

import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

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
 * constructs everything you need and returns a new {@code OnScreenHint} object.
 */
public class FaceBeautyInfo {
    static final String TAG = "FaceBeautyInfo";

    private View mView;
    private TextView mText;
    private RelativeLayout mLayout;

    private final WindowManager.LayoutParams mParams = new WindowManager.LayoutParams();
    private final WindowManager mWM;
    private final Handler mHandler = new Handler();

    private IModuleCtrl mIModuleCtrl;

    // / for orientation function.
    private static final int TOAST_DURATION = 3000; // milliseconds
    private static final int EFFECTS_BUTTON_WIDTH = 52;
    private static final int WIDTH_SEEK_BAR = 180;
    private static final int DEFUALT_LAYOUT_PDDING_RIGHT = 92;
    private static final int COMPONENT_VALUE = 12;

    private int mOrientation = -1;
    private int mWidthPixel = -1;
    private int mHeightPixel = -1;
    private int targetId = -1;
    private int mDefaultIconNumber = -1;

    private float mDensity = -1;

    /**
     * Construct an empty OnScreenHint object.
     *
     * @param context
     *            The context to use. Usually your
     *            {@link android.app.Application} or
     *            {@link android.app.Activity} object.
     */
    public FaceBeautyInfo(Context context, IModuleCtrl moduleCtrl) {
        mWM = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);

        mParams.height = WindowManager.LayoutParams.MATCH_PARENT;
        mParams.width = WindowManager.LayoutParams.MATCH_PARENT;
        mParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                | WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
        mParams.format = PixelFormat.TRANSLUCENT;
        // mParams.windowAnimations = R.style.Animation_OnScreenHint;
        mParams.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
        mParams.setTitle("OnScreenHint");

        mIModuleCtrl = moduleCtrl;

        DisplayMetrics mDisplayMetrics = new DisplayMetrics();
        mDisplayMetrics = context.getResources().getDisplayMetrics();
        mDensity = mDisplayMetrics.density;
        mWidthPixel = mDisplayMetrics.widthPixels;
        mHeightPixel = mDisplayMetrics.heightPixels;

        LayoutInflater inflate = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mView = inflate.inflate(R.layout.onscreen_effect_mode_toast, null);
        mText = (TextView) mView.findViewById(R.id.message);

        mLayout = (RelativeLayout) mView.findViewById(R.id.onscreen_toast_layout);
    }

    /**
     * Update the text in a OnScreenHint that was previously created using one
     * of the makeText() methods.
     *
     * @param s
     *            The new text for the OnScreenHint.
     */
    public void setText(CharSequence s) {

        if (mText == null) {
            throw new RuntimeException("This FaceBeautyInfo was null,please check");
        }
        mText.setText(s);
    }

    /**
     * Show the view on the screen.
     */
    public void show() {
        mHandler.post(mShow);
    }

    /**
     * Close the view if it's showing.
     */
    public void cancel() {
        mHandler.post(mHide);
    }

    public void showToast() {
        mHandler.removeCallbacks(mShow);
        mHandler.removeCallbacks(mHide);
        mHandler.post(mShow);
        mHandler.postDelayed(mHide, TOAST_DURATION);
    }

    public void hideToast() {
        mHandler.removeCallbacks(mShow);
        mHandler.removeCallbacks(mHide);
        mHandler.post(mHide);
    }

    public void setTargetId(int target, int number) {
        targetId = target;
        mDefaultIconNumber = number;
    }

    public void onOrientationChanged(int orientation) {
        Log.i(TAG, "[onOrientationChanged] orientation = " + orientation + ",mOrientation = "
                + mOrientation);
        if (mOrientation != orientation) {
            mOrientation = orientation;
            Util.setOrientation(mView, mOrientation, true);
        }
        setInfoView();
    }

    private synchronized void handleShow() {
        if (mView != null) {
            handleHide();
            setInfoView();
            try {
                if (mView.getParent() != null) {
                    mWM.removeView(mView);
                }
                //should update the view's orientation
                Util.setOrientation(mView, mIModuleCtrl.getOrientationCompensation(), true);
                mWM.addView(mView, mParams);
            } catch (BadTokenException ex) {
                ex.printStackTrace();
            }
            Util.fadeIn(mView);
        }
    }

    private synchronized void handleHide() {
        if (mView != null) {
            // note: checking parent() just to make sure the view has
            // been added... i have seen cases where we get here when
            // the view isn't yet added, so let's try not to crash.
            Util.fadeOut(mView);
        }
    }

    private final Runnable mShow = new Runnable() {
        @Override
        public void run() {
            handleShow();
        }
    };

    private final Runnable mHide = new Runnable() {
        @Override
        public void run() {
            handleHide();
        }
    };

    private int calcualateStaticMargin() {
        float value = 0;
        value = (EFFECTS_BUTTON_WIDTH) * mDensity;
        Log.d(TAG, "calcualateStaticMargin,value =  " + value);

        return (int) value;
    }

    private int calcualteDefualtMargin() {
        float defaultMargin = 0;
        defaultMargin = Math.max(mWidthPixel, mHeightPixel)
                - (EFFECTS_BUTTON_WIDTH * mDefaultIconNumber + WIDTH_SEEK_BAR
                        + DEFUALT_LAYOUT_PDDING_RIGHT /*+ COMPONENT_VALUE*/) * mDensity;
        Log.d(TAG, "[calcualteDefualtMargin] defaultMargin = " + defaultMargin + ",mWidthPixel = "
                + mWidthPixel + ",mHeightPixel = " + mHeightPixel + ",mDensity = " + mDensity);

        return (int) defaultMargin;
    }

    private void setInfoView() {
        if (mView != null) {
            if (isLandcape()) {
                setLandScapeView();
            } else {
                setPortraitView();
            }
            mView.requestLayout();
        }
    }

    private boolean isLandcape() {
        int orientation = mIModuleCtrl.getOrientation();
        // should be checked for sensor setup orientation
        boolean land = orientation == 90 || orientation == 270;
        Log.d(TAG, "[isLandcape] orientation=" + orientation + ", land = " + land);

        return land;
    }

    private void setLandScapeView() {
        ViewGroup.LayoutParams vp = mLayout.getLayoutParams();
        vp.height = ViewGroup.LayoutParams.MATCH_PARENT;
        vp.width = ViewGroup.LayoutParams.WRAP_CONTENT;
        mLayout.setLayoutParams(vp);
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        int defalutLeftMargin = calcualteDefualtMargin()
                + (int) (EFFECTS_BUTTON_WIDTH * targetId * mDensity);
        if (defalutLeftMargin < 0) {
            defalutLeftMargin = 0;
        }
        params.leftMargin = defalutLeftMargin;
        params.bottomMargin = calcualateStaticMargin();
        mText.setLayoutParams(params);
    }

    private void setPortraitView() {

        ViewGroup.LayoutParams vp = mLayout.getLayoutParams();
        vp.width = ViewGroup.LayoutParams.MATCH_PARENT;
        vp.height = ViewGroup.LayoutParams.WRAP_CONTENT;
        mLayout.setLayoutParams(vp);
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        params.leftMargin = calcualateStaticMargin();
        params.topMargin = calcualteDefualtMargin()
                + (int) (EFFECTS_BUTTON_WIDTH * targetId * mDensity);
        mText.setLayoutParams(params);
    }
}
