/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.mode.panorama;

import android.os.Handler;
import android.view.ViewGroup;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * The animation controller for panorama view.
 */
public class AnimationController {
    private static final LogUtil.Tag TAG = new LogUtil.
            Tag(AnimationController.class.getSimpleName());

    private static final int ANIM_DURATION = 180;
    private int mCenterDotIndex = 0;
    private int mDirectionDotIndex = 0;

    private ViewGroup[] mDirectionIndicators;
    private ViewGroup mCenterArrow;

    private Handler mHandler = new Handler();

    /**
     * Constructor of AnimationController.
     * @param indicators The direction indicators.
     * @param arrow The arrow view group.
     */
    public AnimationController(ViewGroup[] indicators, ViewGroup arrow) {
        mDirectionIndicators = indicators;
        mCenterArrow = arrow;
    }

    /**
     * Start direction animation.
     */
    public void startDirectionAnimation() {
        LogHelper.d(TAG, "[startDirectionAnimation]...");
        mDirectionDotIndex = 0;
        mApplyDirectionAnim.run();
    }

    /**
     * Start center animation.
     */
    public void startCenterAnimation() {
        LogHelper.d(TAG, "[startCenterAnimation]...");
        mCenterDotIndex = 0;
        mApplyCenterArrowAnim.run();
    }

    /**
     * Stop center animation.
     */
    public void stopCenterAnimation() {
        LogHelper.d(TAG, "[stopCenterAnimation]...");
        if (mCenterArrow != null) {
            for (int i = 0; i < mCenterArrow.getChildCount(); i++) {
                mCenterArrow.getChildAt(i).clearAnimation();
            }
        }
    }

    private Runnable mApplyCenterArrowAnim = new Runnable() {
        private int mDotCount = 0;

        public void run() {
            if (mDotCount == 0) {
                mDotCount = mCenterArrow.getChildCount();
            }
            if (mDotCount <= mCenterDotIndex) {
                LogHelper.w(TAG, "[run]mApplyCenterArrowAnim return, mDotCount = " + mDotCount
                        + ",mCenterDotIndex =" + mCenterDotIndex);
                return;
            }
            AlphaAnimation alpha = new AlphaAnimation(1.0f, 0.0f);
            alpha.setDuration(ANIM_DURATION * 8);
            alpha.setRepeatCount(Animation.INFINITE);

            if (mCenterArrow != null) {
                mCenterArrow.getChildAt(mCenterDotIndex).startAnimation(alpha);
            }
            alpha.startNow();
            mCenterDotIndex++;
            mHandler.postDelayed(this, ANIM_DURATION * 2 / mDotCount);
        }
    };

    private Runnable mApplyDirectionAnim = new Runnable() {
        private int mDotCount = 0;

        public void run() {
            for (ViewGroup viewGroup : mDirectionIndicators) {
                if (viewGroup == null) {
                    LogHelper.w(TAG, "[run]viewGroup is null,return!");
                    return;
                }
            }
            if (mDotCount == 0) {
                mDotCount = mDirectionIndicators[0].getChildCount();
            }

            if (mDotCount <= mDirectionDotIndex) {
                LogHelper.i(TAG, "[run]mApplyDirectionAnim,return,mDotCount = " + mDotCount
                        + ",mCenterDotIndex =" + mCenterDotIndex);
                return;
            }
            AlphaAnimation alpha = new AlphaAnimation(1.0f, 0.0f);
            alpha.setDuration(ANIM_DURATION * mDotCount * 3 / 2);
            alpha.setRepeatCount(Animation.INFINITE);

            mDirectionIndicators[0].getChildAt(mDirectionDotIndex).startAnimation(alpha);
            mDirectionIndicators[1].getChildAt(mDotCount - mDirectionDotIndex - 1).startAnimation(
                    alpha);
            mDirectionIndicators[2].getChildAt(mDotCount - mDirectionDotIndex - 1).startAnimation(
                    alpha);
            mDirectionIndicators[3].getChildAt(mDirectionDotIndex).startAnimation(alpha);
            alpha.startNow();

            mDirectionDotIndex++;
            mHandler.postDelayed(this, ANIM_DURATION / 2);
        }
    };
}