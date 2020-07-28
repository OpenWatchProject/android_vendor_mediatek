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

package com.mediatek.camera.v2.control.focus;

import android.content.Context;
import android.graphics.Region;
import android.util.AttributeSet;

import com.android.camera.R;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.control.focus.IFocus.AutoFocusUI;
import com.mediatek.camera.v2.ui.UIRotateLayout;

/**
 *
 */
public class AutoFocusRotateLayout extends UIRotateLayout implements AutoFocusUI {
    private static final Tag TAG = new Tag(AutoFocusRotateLayout.class.getSimpleName());
    private static final int       STATE_IDLE = 0;
    private static final int       STATE_FOCUSING = 1;
    private static final int       STATE_FINISHING = 2;
    private int                    mState;

    private static final int       SCALING_UP_TIME = 1000;
    private static final int       SCALING_DOWN_TIME = 200;
    private static final int       DISAPPEAR_TIMEOUT = 200;
    private Runnable               mDisappear = new Disappear();
    private Runnable               mEndAction = new EndAction();


    public AutoFocusRotateLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void setFocusPosition(int x, int y, boolean isPassiveScan,
            int aFsize, int aEsize) {

    }

    @Override
    public void setFocusPosition(int x, int y, boolean isPassiveScan) {

    }

    @Override
    public void onFocusStarted() {
        if (mState == STATE_IDLE) {
            setDrawable(R.drawable.ic_focus_focusing);
            animate().withLayer().setDuration(SCALING_UP_TIME).scaleX(1.5f).scaleY(1.5f);
            mState = STATE_FOCUSING;
        }
    }

    @Override
    public void onFocusSucceeded() {
        if (mState == STATE_FOCUSING) {
            setDrawable(R.drawable.ic_focus_focused);
            animate().withLayer().setDuration(SCALING_DOWN_TIME).scaleX(1f).scaleY(1f)
                    .withEndAction(mEndAction);
            mState = STATE_FINISHING;
        }
    }

    @Override
    public void onFocusFailed() {
        if (mState == STATE_FOCUSING) {
            setDrawable(R.drawable.ic_focus_focused);
            animate().withLayer().setDuration(SCALING_DOWN_TIME).scaleX(1f).scaleY(1f)
                    .withEndAction(mEndAction);
            mState = STATE_FINISHING;
        }
    }

    @Override
    public void setPassiveFocusSuccess(boolean success) {
        if (mState == STATE_FOCUSING) {
            setDrawable(R.drawable.ic_focus_focused);
            animate().withLayer().setDuration(SCALING_DOWN_TIME).scaleX(1f).scaleY(1f)
                    .withEndAction(mEndAction);
            mState = STATE_FINISHING;
        }
    }

    @Override
    public boolean gatherTransparentRegion(Region region) {
        if (region != null) {
            final int[] location = new int[2];
            int width = getWidth();
            int height = getHeight();
            getLocationInWindow(location);
            LogHelper.i(TAG, "location[0]:" + location[0] + " location[1]:" + location[1]);
            int l = location[0] + width / 2 - width;
            int t = location[1] + height / 2 - height;
            int r = l + width * 2;
            int b = t + height * 2;
            LogHelper.i(TAG, "gatherTransparentRegion l:" + l +
                    " t:" + t + " r:" + r + " b:" + b);
            region.op(l, t, r, b, Region.Op.DIFFERENCE);
        }
        return true;
    }

    public void clear() {
        LogHelper.d(TAG, "clear mState = " + mState);
        animate().cancel();
        removeCallbacks(mDisappear);
        mDisappear.run();
        setScaleX(1f);
        setScaleY(1f);
    }

    private void setDrawable(int resid) {
        mChild.setBackgroundDrawable(getResources().getDrawable(resid));
    }

    private class EndAction implements Runnable {
        @Override
        public void run() {
            // Keep the focus indicator for some time.
            postDelayed(mDisappear, DISAPPEAR_TIMEOUT);
        }
    }

    private class Disappear implements Runnable {
        @Override
        public void run() {
            LogHelper.i(TAG, "Disappear run mState = " + mState);
            mChild.setBackgroundDrawable(null);
            mState = STATE_IDLE;
        }
    }
}