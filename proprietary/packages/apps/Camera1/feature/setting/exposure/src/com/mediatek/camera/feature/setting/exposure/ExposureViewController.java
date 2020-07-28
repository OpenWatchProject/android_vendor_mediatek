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
package com.mediatek.camera.feature.setting.exposure;

import android.app.Activity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * The focus controller interacts with the focus UI.
 */
public class ExposureViewController {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(ExposureViewController.class.getSimpleName());
    private static final int INVALID_SIZE = 1;
    private Exposure mExposure;
    private ViewGroup mFeatureRootView;
    private ExposureView mExposureView;
    private Activity mActivity;
    private RelativeLayout mExpandView;

    /**
     * Constructor of exposure view.
     *
     * @param app   The application app level controller
     * @param exposure Exposure instance.
     */
    public ExposureViewController(final IApp app, final Exposure exposure) {
        mActivity = app.getActivity();
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mExposure = exposure;
                mFeatureRootView = app.getAppUi().getPreviewFrameLayout();
                if (mFeatureRootView.findViewById(R.id.focus_view) == null) {
                    app.getActivity().getLayoutInflater().inflate(R.layout.focus_view,
                            mFeatureRootView, true);
                }
                mExpandView = (RelativeLayout) mFeatureRootView.findViewById(R.id.expand_view);
                LogHelper.d(TAG, "ExposureViewController current EV = " + mFeatureRootView
                        .findViewById(R.id.exposure_view));
                if (mFeatureRootView.findViewById(R.id.exposure_view) == null) {
                    app.getActivity().getLayoutInflater().inflate(R
                                    .layout.exposure_view,
                            mExpandView, true);
                }
                mExposureView = (ExposureView) mFeatureRootView.findViewById(R.id.exposure_view);
                LogHelper.d(TAG, "ExposureViewController mExposureView EV = " + mExposureView);
                mExposureView.setListener(mExposure);
            }
        });
    }

    protected void resetExposureView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mExposureView == null) {
                    LogHelper.w(TAG, "[resetExposureView] mExposureView is null");
                    return;
                }
                int size = mExposure.getEntryValues().size();
                LogHelper.d(TAG, "[resetExposureView] size " + size);
                if (size <= INVALID_SIZE) {
                    mExposureView.setListener(null);
                    mExposureView.setVisibility(View.GONE);
                } else {
                    mExposureView.setListener(mExposure);
                    mExposureView.setVisibility(View.VISIBLE);
                    mExposureView.resetExposureView();
                }
            }
        });
    }

    protected void initExposureValues(final int[] values) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mExposureView == null) {
                    LogHelper.w(TAG, "[initExposureValues] mExposureView is null");
                    return;
                }
                mExposureView.initExposureView(values);
            }
        });
    }

    protected void setOrientation(final int orientation) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mExposureView == null) {
                    return;
                }
                mExposureView.setOrientation(orientation);
            }
        });
    }

    /**
     * If exposure view should be updated when there is scroll on the screen.
     *
     * @return True if expand view is shown on the screen.
     */
    protected boolean needUpdateExposureView() {
        return (mExposureView != null && mExpandView.getVisibility() == View.VISIBLE
                && mExposureView.getVisibility() == View.VISIBLE);
    }

    /**
     * Called when the vertical scroll position of screen changes.
     *
     * @param event The move motion event that triggered the current onScroll.
     * @param delta The distance along the vertical axis that has been scrolled since the last
     *              call to onScroll.
     */
    protected void onVerticalScroll(MotionEvent event, float delta) {
        mExposureView.onVerticalScroll(event, delta);
    }

    /**
     * Called when the vertical scroll position of screen changes begin and end.
     *
     * @param start True when start to scroll,false otherwise.
     */
    protected void onTrackingTouch(boolean start) {
        mExposureView.onTrackingTouch(start);
    }

    /**
     * Config exposure UI enable state.
     *
     * @param enabled Whether the view can be enable or not.
     */
    protected void setViewEnabled(final boolean enabled) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mExposureView != null) {
                    mExposureView.setViewEnabled(enabled);
                }
            }
        });
    }

}
