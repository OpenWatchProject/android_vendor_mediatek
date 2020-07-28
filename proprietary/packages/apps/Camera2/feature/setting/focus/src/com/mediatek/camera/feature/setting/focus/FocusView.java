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
package com.mediatek.camera.feature.setting.focus;

import android.content.Context;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.widget.RotateLayout;

/**
 * Focus view.
 */
public class FocusView extends RotateLayout implements IFocusView {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(FocusView.class.getSimpleName());
    private ImageView mFocusRing;
    private RelativeLayout mExpandView;
    private Runnable mDisappear = new Disappear();
    private Runnable mEndAction = new EndAction();
    private static final int SCALING_UP_TIME = 1000;
    private static final int SCALING_DOWN_STAY_TIME = 2000;
    private static final int SCALING_DOWN_TIME = 200;
    private static final int DISAPPEAR_TIMEOUT = 200;
    private IFocusView.FocusViewState mState = FocusViewState.STATE_IDLE;
    private RectF mPreviewRect = new RectF();
    private int mFocusViewX;
    private int mFocusViewY;
    private boolean mIsExpandViewRightOfFocusRing = true;

    /**
     * The constructor.
     *
     * @param context The Context the view is running in, through which it can access the current
     *                theme, resources, etc.
     * @param attrs   The attributes of the XML tag that is inflating the view.
     */
    public FocusView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void onFinishInflate() {
        super.onFinishInflate();
        mFocusRing = (ImageView) findViewById(R.id.focus_ring);
        mExpandView = (RelativeLayout) findViewById(R.id.expand_view);
    }

    @Override
    public boolean isPassiveFocusRunning() {
        LogHelper.d(TAG, "[isPassiveFocusRunning] mState =  " + mState);
        return getFocusState() == FocusViewState.STATE_PASSIVE_FOCUSING;
    }

    private boolean isExpandViewOutOfDisplay() {
        int previewLeft = (int) mPreviewRect.left;
        int previewRight = (int) mPreviewRect.right;
        int previewTop = (int) mPreviewRect.top;
        int previewBottom = (int) mPreviewRect.bottom;

        switch (getOrientation()) {
            case 0:
                return mFocusViewX + (mFocusRing.getWidth() / 2 + mExpandView.getWidth()) >
                        previewRight;
            case 90:
                return mFocusViewY - (mFocusRing.getWidth() / 2 + mExpandView.getWidth()) <
                        previewTop;
            case 180:
                return mFocusViewX - (mFocusRing.getWidth() / 2 + mExpandView.getWidth()) <
                        previewLeft;
            case 270:
                return mFocusViewY + (mFocusRing.getWidth() / 2 + mExpandView.getWidth()) >
                        previewBottom;
            default:
                return false;
        }
    }

    @Override
    protected void onLayout(boolean change, int left, int top, int right, int bottom) {
        super.onLayout(change, left, top, right, bottom);
        RelativeLayout.LayoutParams layoutParams =
                new RelativeLayout.LayoutParams(mExpandView.getLayoutParams());
        //Change expand view position if it is out of display.
        if (isExpandViewOutOfDisplay()) {
            if (mIsExpandViewRightOfFocusRing) {
                layoutParams.addRule(RelativeLayout.LEFT_OF, R.id.focus_ring);
                layoutParams.addRule(RelativeLayout.CENTER_VERTICAL);
                mExpandView.setLayoutParams(layoutParams);
                mExpandView.postInvalidate();
                mIsExpandViewRightOfFocusRing = false;
                LogHelper.d(TAG, "[onLayout] set ExpandView to left");
            }
            return;
        }

        if (mIsExpandViewRightOfFocusRing == false) {
            LogHelper.d(TAG, "[onLayout] set ExpandView to right");
            layoutParams.addRule(RelativeLayout.RIGHT_OF, R.id.focus_ring);
            layoutParams.addRule(RelativeLayout.CENTER_VERTICAL);
            mExpandView.setLayoutParams(layoutParams);
            mExpandView.postInvalidate();
            mIsExpandViewRightOfFocusRing = true;
            return;
        }
    }

    @Override
    public boolean isActiveFocusRunning() {
        LogHelper.d(TAG, "[isActiveFocusRunning] mState =  " + mState);
        boolean result = (getFocusState() == FocusViewState.STATE_ACTIVE_FOCUSING);
        return result;
    }

    @Override
    public void startPassiveFocus() {
        if (getFocusState() != FocusViewState.STATE_IDLE || getHandler() == null) {
            LogHelper.w(TAG, "[startPassiveFocus] mState " + mState + ",getHandler = " +
                    getHandler());
            return;
        }
        getHandler().removeCallbacks(mDisappear);
        setContentDescription("continue focus");
        mFocusRing.setVisibility(VISIBLE);
        mExpandView.setVisibility(INVISIBLE);
        mFocusRing.setImageDrawable(this.getResources().getDrawable(R.drawable.ic_continue_focus));
        setVisibility(VISIBLE);
        animate().withLayer().setDuration(SCALING_UP_TIME).scaleX(1.2f).scaleY(1.2f).alpha(1.0f);
        setFocusState(FocusViewState.STATE_PASSIVE_FOCUSING);
    }

    @Override
    public void startActiveFocus() {
        if (getFocusState() != FocusViewState.STATE_IDLE || getHandler() == null) {
            LogHelper.w(TAG, "[startActiveFocus] mState " + mState + ",getHandler = " +
                    getHandler());
            return;
        }
        getHandler().removeCallbacks(mDisappear);
        setContentDescription("touch focus");
        mExpandView.setVisibility(VISIBLE);
        mFocusRing.setVisibility(VISIBLE);
        mFocusRing.setImageDrawable(this.getResources().getDrawable(R.drawable.ic_touch_focus));
        setVisibility(VISIBLE);
        animate().withLayer().setDuration(SCALING_UP_TIME).scaleX(1.2f).scaleY(1.2f).alpha(1.0f);
        setFocusState(FocusViewState.STATE_ACTIVE_FOCUSING);
    }

    @Override
    public void stopFocusAnimations() {
        if (isPassiveFocusRunning()) {
            animate().withLayer().setDuration(SCALING_DOWN_TIME).scaleX(1f).scaleY(1f)
                    .withEndAction(true ? mEndAction : null);
        } else if (isActiveFocusRunning()) {
            setFocusState(FocusViewState.STATE_ACTIVE_FOCUSED);
            animate().withLayer().setDuration(SCALING_DOWN_TIME).scaleX(1f).scaleY(1f);
            postDelayed(new ActiveFocusEndAction(), SCALING_DOWN_STAY_TIME);
        }
    }

    @Override
    public void setFocusLocation(float viewX, float viewY) {
        mFocusViewX = (int) viewX;
        mFocusViewY = (int) viewY;
    }

    @Override
    public void centerFocusLocation() {

    }

    protected void setPreviewRect(RectF previewRect) {
      mPreviewRect = previewRect;
    }

    protected synchronized FocusViewState getFocusState() {
        return mState;
    }

    protected void setFocusState(FocusViewState state) {
        mState = state;
    }

    protected void clearFocusUi() {
        mFocusRing.setVisibility(INVISIBLE);
        mExpandView.setVisibility(INVISIBLE);
        setVisibility(INVISIBLE);
        setFocusState(FocusViewState.STATE_IDLE);
    }

    protected void highlightFocusView() {
        animate().withLayer().alpha(1.0f);
    }

    protected void lowlightFocusView() {
        postDelayed(new ActiveFocusEndAction(), SCALING_DOWN_STAY_TIME);
    }

    private class EndAction implements Runnable {
        @Override
        public void run() {
            // Keep the focus indicator for some time.
            postDelayed(mDisappear, DISAPPEAR_TIMEOUT);
        }
    }

    /**
     * Action of active focus.
     */
    private class ActiveFocusEndAction implements Runnable {
        @Override
        public void run() {
            LogHelper.d(TAG, "[ActiveFocusEndAction run +] mState " + mState);
            animate().withLayer().alpha(0.5f);
        }
    }

    private class Disappear implements Runnable {
        @Override
        public void run() {
            LogHelper.d(TAG, "[Disappear run +] mState " + mState);
            clearFocusUi();
        }
    }

}
