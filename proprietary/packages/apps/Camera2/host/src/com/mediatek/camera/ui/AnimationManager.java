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

import android.animation.Animator;
import android.animation.AnimatorInflater;
import android.animation.AnimatorListenerAdapter;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.os.AsyncTask;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AccelerateInterpolator;
import android.widget.ImageView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUi.AnimationType;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.ui.preview.PreviewSurfaceView;

/**
 * View Animation manager.
 */
class AnimationManager {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(AnimationManager.class.getSimpleName());
    private static final int YUV_SCALE_RATIO = 2;
    private final IApp mApp;
    private final CameraAppUI mAppUI;
    private final ImageView mAnimationView;
    private final ViewGroup mAnimationRoot;
    private final View mCoverView;

    private AnimatorSet mFlipAnimation;
    private AnimatorSet mSwitchCameraAnimator;

    /**
     * Animation async task data structure, used to pass the animation type and data to
     * the async thread.
     */
    private static final class AsyncData {
        public AnimationType mType;
        public IAppUi.AnimationData mData;
    }

    /**
     * View Animation constructor.
     *
     * @param app        The {@link IApp} implementer.
     * @param appui      The {@link CameraAppUI} instance.
     */
    public AnimationManager(IApp app, CameraAppUI appui) {
        mApp = app;
        mAppUI = appui;
        mAnimationRoot = (ViewGroup) mApp.getActivity().findViewById(R.id.animation_root);
        mAnimationView = (ImageView) mApp.getActivity().findViewById(R.id.animation_view);

        mCoverView = mApp.getActivity().findViewById(R.id.camera_cover);
    }

    /**
     * Start the animation.
     *
     * @param type Animation type. {@link AnimationType}
     * @param data Animation data. {@link IAppUi.AnimationData}
     */
    public void animationStart(AnimationType type, IAppUi.AnimationData data) {
        LogHelper.d(TAG, "Start animation type: " + type);
        switch (type) {
            case TYPE_CAPTURE:
                mCoverView.setVisibility(View.VISIBLE);
                playCaptureAnimation();
                break;
            default:
                break;
        }
    }

    /**
     * Stop the animation.
     *
     * @param type Animation type. {@link AnimationType}
     */
    public void animationEnd(AnimationType type) {
        LogHelper.d(TAG, "End animation type: " + type);
        switch (type) {
            case TYPE_CAPTURE:
                mCoverView.setVisibility(View.GONE);
                break;
            default:
                break;
        }
    }

    private void playCaptureAnimation() {
        LogHelper.d(TAG, "playCaptureAnimation +");
        AnimatorSet captureAnimation =
                (AnimatorSet) AnimatorInflater.loadAnimator(mApp.getActivity(),
                R.animator.cature_anim);
        captureAnimation.setTarget(mCoverView);

        captureAnimation.addListener(new AnimatorListenerAdapter() {

            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                mCoverView.setVisibility(View.GONE);
            }
        });

        captureAnimation.start();
        LogHelper.d(TAG, "playCaptureAnimation -");
    }

}
