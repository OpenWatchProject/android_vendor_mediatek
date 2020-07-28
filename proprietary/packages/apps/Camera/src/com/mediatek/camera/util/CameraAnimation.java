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

package com.mediatek.camera.util;

import android.animation.Animator;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.view.View;

//TODO: Mediatek packages can not reference to Google packages
import com.android.camera.manager.ThumbnailViewManager;

/**
 * Class to handle animations.
 */

public class CameraAnimation {
    private static final String TAG = "CameraAnimation";

    public static final int SLIDE_DURATION = 300;
    private AnimatorSet mCaptureAnimator;

    /**
     * Starts capture animation.
     *
     * @param view
     *            a viewManager view that shows a picture captured and gets
     *            animated
     */
    public void doCaptureAnimation(final View view, Activity activity,
            final ThumbnailViewManager.AnimationEndListener thumbnailViewManager) {
        Log.d(TAG, "[doCaptureAnimation] activity.getRequestedOrientation() = "
                + activity.getRequestedOrientation());
        cancelAnimations();
        View parentView = (View) view.getParent();
        float slideDistance;
        ObjectAnimator slide;
        ObjectAnimator translateY;

        int centerX = view.getLeft() + view.getWidth() / 2;
        int centerY = view.getTop() + view.getHeight() / 2;

        int orientation = activity.getRequestedOrientation();
        if (orientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
            || orientation == ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE) {
            slideDistance = (float) view.getWidth();
            slide = ObjectAnimator.ofFloat(view, "translationX", 0f,
                    slideDistance).setDuration(CameraAnimation.SLIDE_DURATION);
            translateY = ObjectAnimator.ofFloat(view, "translationY",
                    parentView.getHeight() / 2 - centerY, 0f).setDuration(0);
        } else {
            slideDistance = (float) view.getHeight();
            slide = ObjectAnimator.ofFloat(view, "translationY", 0f,
                    slideDistance).setDuration(CameraAnimation.SLIDE_DURATION);
            translateY = ObjectAnimator.ofFloat(view, "translationX",
                    parentView.getHeight() / 2 - centerX, 0f).setDuration(0);
        }

        translateY.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {
                //Do-noting
            }

            @Override
            public void onAnimationEnd(Animator animator) {
                view.setClickable(true);
            }

            @Override
            public void onAnimationCancel(Animator animator) {
                //Do-noting
            }

            @Override
            public void onAnimationRepeat(Animator animator) {
                //Do-noting
            }
        });

        mCaptureAnimator = new AnimatorSet();
        mCaptureAnimator.playTogether(translateY, slide);
        mCaptureAnimator.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {
                view.setClickable(false);
                view.setVisibility(View.VISIBLE);
            }

            @Override
            public void onAnimationEnd(Animator animator) {
                view.setScaleX(1f);
                view.setScaleX(1f);
                view.setTranslationX(0f);
                view.setTranslationY(0f);
                view.setVisibility(View.INVISIBLE);
                mCaptureAnimator.removeAllListeners();
                mCaptureAnimator = null;
                thumbnailViewManager.onAnianmationEnd();
            }

            @Override
            public void onAnimationCancel(Animator animator) {
                view.setVisibility(View.INVISIBLE);
            }

            @Override
            public void onAnimationRepeat(Animator animator) {
                // Do nothing.
            }
        });
        mCaptureAnimator.start();
    }

    /**
     * Cancels on-going flash animation and capture animation, if any.
     */
    private void cancelAnimations() {
        // End the previous animation if the previous one is still running
        if (mCaptureAnimator != null && mCaptureAnimator.isStarted()) {
            mCaptureAnimator.cancel();
        }
    }
}
