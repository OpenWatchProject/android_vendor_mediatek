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

package com.mediatek.camera.ui;

import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.utils.CameraUtil;

import junit.framework.Assert;

/**
 * An abstract view manager, partly implements {@link IViewManager} common function.
 */
public abstract class AbstractViewManager implements IViewManager {
    protected final IApp mApp;
    protected final ViewGroup mParentView;
    private View mView;
    private final OnOrientationChangeListenerImpl mOrientationChangeListener;

    /**
     * Constructor of abstractViewManager.
     * @param app The {@link IApp} implementer.
     * @param parentView the root view of ui.
     */
    public AbstractViewManager(IApp app, ViewGroup parentView) {
        Assert.assertNotNull(app);
        mApp = app;
        mParentView = parentView;
        mOrientationChangeListener = new OnOrientationChangeListenerImpl();
    }

    protected abstract View getView();

    @Override
    public void setVisibility(int visibility) {
        if (visibility == View.VISIBLE) {
            show();
        } else if (visibility == View.INVISIBLE) {
            hide(View.INVISIBLE);
        } else if (visibility == View.GONE) {
            hide(View.GONE);
        }
    }

    @Override
    public int getVisibility() {
        if (mView != null) {
            return mView.getVisibility();
        }
        return -1;
    }

    @Override
    public void setEnabled(boolean enabled) {
        if (mView != null) {
            mView.setEnabled(enabled);
            mView.setClickable(enabled);
        }
    }

    @Override
    public boolean isEnabled() {
        if (mView != null) {
            return mView.isEnabled();
        }
        return false;
    }

    @Override
    public void onOrientationChanged(int newOrientation) {

    }

    @Override
    public void onLayoutOrientationChanged(boolean isLandscape) {

    }

    @Override
    public void onCreate() {
        mApp.registerOnOrientationChangeListener(mOrientationChangeListener);
    }

    @Override
    public void onResume() {

    }

    @Override
    public void onPause() {

    }

    @Override
    public void onDestroy() {
        mApp.unregisterOnOrientationChangeListener(mOrientationChangeListener);
    }

    /**
     * Update current device orientation to view.
     */
    public void updateViewOrientation() {
        int orientation = mApp.getGSensorOrientation();
        CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mView, orientation, false);
    }

    private void show() {
        if (mView == null) {
            mView = getView();
        }
        if (mView != null) {
            mView.setVisibility(View.VISIBLE);
            mView.setClickable(true);
        }
    }

    private void hide(int visibility) {
        if (mView == null) {
            mView = getView();
        }
        if (mView != null) {
            mView.setVisibility(visibility);
        }
    }

    /**
     * Implementer of OnOrientationChangeListener.
     */
    private class OnOrientationChangeListenerImpl implements IApp.OnOrientationChangeListener {

        @Override
        public void onOrientationChanged(int orientation) {
            if (mView != null) {
                CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mView,
                        orientation, true);
            }
        }
    }
}
