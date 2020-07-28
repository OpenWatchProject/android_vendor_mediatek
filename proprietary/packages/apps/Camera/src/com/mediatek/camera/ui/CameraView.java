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

package com.mediatek.camera.ui;

import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.Animation;
import android.widget.RelativeLayout;

import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

public abstract class CameraView implements ICameraView {
    private static final String TAG = "CameraView";

    protected Activity mActivity;

    private View mView;

    private Animation mFadeIn;
    private Animation mFadeOut;

    private int mOrientation;

    private boolean mIsShowing = false;
    private boolean mIsEnabled = true;

    public CameraView(Activity activity) {
        mActivity = activity;
        //mView = getView();
       // addView(mView);
        //Util.setOrientation(mView, mOrientation, false);
    }

    @Override
    public void init(Activity activity, ICameraAppUi cameraAppUi, IModuleCtrl moduleCtrl) {
    }

    @Override
    public void uninit() {
        hide();
        removeView();
    }

    @Override
    public void show() {
        Log.d(TAG, "[show]mShowing = " + mIsShowing);
        if (mView == null) {
            mView = getView();
            if (mView != null) {
                addView(mView);
                Util.setOrientation(mView, mOrientation, false);
            }
        }
        if (!mIsShowing) {
            mIsShowing = true;
            setEnabled(mIsEnabled);
            refresh(); // refresh view state
            if (mView != null) {
                fadeIn();
                mView.setVisibility(View.VISIBLE);
            }
        } else if (mIsShowing) {
            refresh();
        }
    }

    @Override
    public void hide() {
        Log.d(TAG, "[hide]mShowing = " + mIsShowing);
        if (mView != null && mIsShowing) {
            mIsShowing = false;
            fadeOut();
            mView.setVisibility(View.GONE);
        }
    }

    @Override
    public void refresh() {
    }

    @Override
    public void reset() {
    }

    @Override
    public void reInflate() {
        boolean showing = mIsShowing;
        hide();
        removeView();
        mView = null;
        if (showing) {
            show();
        }
    }

    @Override
    public boolean update(int type, Object... args) {
        return true;
    }

    @Override
    public boolean isShowing() {
        return mIsShowing;
    }

    @Override
    public boolean isEnabled() {
        return mIsEnabled;
    }

    @Override
    public void setEnabled(boolean enabled) {
        mIsEnabled = enabled;
        if (mView != null) {
            mView.setEnabled(mIsEnabled);
        }
    }

    @Override
    public int getViewHeight() {
        int height = mView == null ? 0 : mView.getHeight();
        return height;
    }

    @Override
    public int getViewWidth() {
        int width = mView == null ? 0 : mView.getWidth();
        return width;
    }

    @Override
    public void setListener(Object obj) {

    }

    @Override
    public void onOrientationChanged(int orientation) {
        if (mOrientation != orientation) {
            mOrientation = orientation;
            Util.setOrientation(mView, mOrientation, true);
        }
    }

    public final Activity getContext() {
        return mActivity;
    }

    public final int getOrientation() {
        return mOrientation;
    }

    public final void setOrientation(int orientation) {
        mOrientation = orientation;
    }

    protected void addView(View view) {
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        params.topMargin = 0;
        if (mView != null) {
            getContext().addContentView(mView, params);
        }
    }

    protected void removeView() {
        ViewGroup parent = mView != null ? (ViewGroup) mView.getParent() : null;
        if (parent != null) {
            parent.removeView(mView);
        }
        mView = null;
    }

    abstract protected View getView();

    protected Animation getFadeInAnimation() {
        return null;
    }

    protected Animation getFadeOutAnimation() {
        return null;
    }

    protected View inflate(int layoutId) {
        return getContext().getLayoutInflater().inflate(layoutId, null);
    }

    private void fadeIn() {
        if (mFadeIn == null) {
            mFadeIn = getFadeInAnimation();
        }
        if (mFadeIn != null) {
            mView.startAnimation(mFadeIn);
        }
    }

    private void fadeOut() {
        if (mFadeOut == null) {
            mFadeOut = getFadeOutAnimation();
        }
        if (mFadeOut != null) {
            mView.startAnimation(mFadeOut);
        }
    }
}
