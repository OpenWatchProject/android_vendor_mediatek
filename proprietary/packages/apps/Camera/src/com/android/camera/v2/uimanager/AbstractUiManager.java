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
package com.android.camera.v2.uimanager;

import com.android.camera.v2.ui.UiUtil;

import junit.framework.Assert;
import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;

public abstract class AbstractUiManager implements IUiManager {
    private final Activity      mActivity;
    private final ViewGroup     mParentViewGroup;

    private View                mView;
    private boolean             mShowing;
    private boolean             mEnabled = true;
    private boolean             mFilterEnable = true;

    // fade in/out animation
    private Animation           mFadeIn;
    private Animation           mFadeOut;
    private boolean             mShowAnimationEnabled = true;
    private boolean             mHideAnimationEnabled = true;

    public AbstractUiManager(Activity activity, ViewGroup parent) {
        Assert.assertNotNull(activity);
        Assert.assertNotNull(parent);
        mActivity = activity;
        mParentViewGroup = parent;
    }

    @Override
    public final View inflate(int layoutId) {
        return mActivity.getLayoutInflater().inflate(layoutId, mParentViewGroup, false);
    }

    @Override
    public void show() {
        if (mView == null) {
            mView = getView();
            mParentViewGroup.addView(mView);
//            UiUtil.setOrientation(mView, mOrientation, false);
        }
        if (mView != null && !mShowing) {
            mShowing = true;
            setEnable(mEnabled);
            refresh(); // refresh view state
            fadeIn();
            mView.setVisibility(View.VISIBLE);
        } else if (mShowing) {
            refresh();
        }
    }

    @Override
    public void refresh() {
        if (mShowing) {
            onRefresh();
        }
    }

    @Override
    public void reInflate() {
        boolean showing = mShowing;
        hide();
        if (mView != null) {
            mParentViewGroup.removeView(mView);
        }
        onRelease();
        mView = null;
        if (showing) {
            show();
        }
    }

    @Override
    public void hide() {
        if (mView != null && mShowing) {
            mShowing = false;
            fadeOut();
            mView.setVisibility(View.GONE);
        }
    }

    @Override
    public void release() {
        hide();
        if (mView != null) {
            mParentViewGroup.removeView(mView);
        }
        onRelease();
        mView = null;
    }

    @Override
    public boolean isShowing() {
        return mShowing;
    }

    @Override
    public void setEnable(boolean enable) {
        mEnabled = enable;
        if (mView != null) {
            mView.setEnabled(mEnabled);
            if (mFilterEnable) {
                UiUtil.setViewEnabledState(mView, mEnabled);
            }
        }
    }

    @Override
    public boolean isEnable() {
        return mEnabled;
    }

    @Override
    public void setFilterEnable(boolean filter) {
        mFilterEnable = filter;
    }

    protected abstract View getView();
    protected void onRefresh() {}
    protected void onRelease() {}

    protected Animation getFadeInAnimation() {
        return null;
    }

    protected Animation getFadeOutAnimation() {
        return null;
    }

    protected void fadeIn() {
        if (mShowAnimationEnabled) {
            if (mFadeIn == null) {
                mFadeIn = getFadeInAnimation();
            }
            if (mFadeIn != null) {
                mView.startAnimation(mFadeIn);
            } else {
                UiUtil.fadeIn(mView);
            }
        }
    }

    protected void fadeOut() {
        if (mHideAnimationEnabled) {
            if (mFadeOut == null) {
                mFadeOut = getFadeOutAnimation();
            }
            if (mFadeOut != null) {
                mView.startAnimation(mFadeOut);
            } else {
                UiUtil.fadeOut(mView);
            }
        }
    }

    public ViewGroup getParentView() {
        return mParentViewGroup;
    }
}
