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

package com.mediatek.camera.mode.panorama;

import android.os.Handler;
import android.view.ViewGroup;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;

import com.mediatek.camera.util.Log;

public class AnimationController {
    private static final String TAG = "AnimationController";

    private static final int ANIM_DURATION = 180;
    private int mCenterDotIndex = 0;
    private int mDirectionDotIndex = 0;

    private ViewGroup[] mDirectionIndicators;
    private ViewGroup mCenterArrow;

    private Handler mHanler = new Handler();

    public AnimationController(ViewGroup[] indicators, ViewGroup arrow) {
        mDirectionIndicators = indicators;
        mCenterArrow = arrow;
    }

    public void startDirectionAnimation() {
        Log.i(TAG, "[startDirectionAnimation]...");
        mDirectionDotIndex = 0;
        mApplyDirectionAnim.run();
    }

    public void stopDirectionAnimation() {
        // do nothing
    }

    public void startCenterAnimation() {
        Log.i(TAG, "[startCenterAnimation]...");
        mCenterDotIndex = 0;
        mApplyCenterArrowAnim.run();
    }

    public void stopCenterAnimation() {
        Log.i(TAG, "[stopCenterAnimation]...");
        if (mCenterArrow != null) {
            for (int i = 0; i < mCenterArrow.getChildCount(); i++) {
                mCenterArrow.getChildAt(i).clearAnimation();
            }
        }
    }

    private Runnable mApplyCenterArrowAnim = new Runnable() {
        private int dotCount = 0;

        public void run() {
            if (dotCount == 0) {
                dotCount = mCenterArrow.getChildCount();
            }
            if (dotCount <= mCenterDotIndex) {
                Log.w(TAG, "[run]mApplyCenterArrowAnim return,dotCount = " + dotCount
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
            mHanler.postDelayed(this, ANIM_DURATION * 2 / dotCount);
        }
    };

    private Runnable mApplyDirectionAnim = new Runnable() {
        private int dotCount = 0;

        public void run() {
            for (ViewGroup viewGroup : mDirectionIndicators) {
                if (viewGroup == null) {
                    Log.w(TAG, "[run]viewGroup is null,return!");
                    return;
                }
            }
            if (dotCount == 0) {
                dotCount = mDirectionIndicators[0].getChildCount();
            }

            if (dotCount <= mDirectionDotIndex) {
                Log.i(TAG, "[run]mApplyDirectionAnim,return,dotCount = " + dotCount
                        + ",mCenterDotIndex =" + mCenterDotIndex);
                return;
            }
            AlphaAnimation alpha = new AlphaAnimation(1.0f, 0.0f);
            alpha.setDuration(ANIM_DURATION * dotCount * 3 / 2);
            alpha.setRepeatCount(Animation.INFINITE);

            mDirectionIndicators[0].getChildAt(mDirectionDotIndex).startAnimation(alpha);
            mDirectionIndicators[1].getChildAt(dotCount - mDirectionDotIndex - 1).startAnimation(
                    alpha);
            mDirectionIndicators[2].getChildAt(dotCount - mDirectionDotIndex - 1).startAnimation(
                    alpha);
            mDirectionIndicators[3].getChildAt(mDirectionDotIndex).startAnimation(alpha);
            alpha.startNow();

            mDirectionDotIndex++;
            mHanler.postDelayed(this, ANIM_DURATION / 2);
        }
    };
}