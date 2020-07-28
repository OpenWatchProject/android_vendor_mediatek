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

package com.mediatek.camera.feature.setting;

import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.widget.RotateStrokeTextView;

/**
 * Default implement of IContinuousShotView.
 */
public class ContinuousShotView implements IContinuousShotView {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            ContinuousShotView.class.getSimpleName());
    private ViewGroup mRootViewGroup;
    private ViewGroup mContinuousShotRoot;
    private RotateStrokeTextView mRotateStrokeTextView;

    @Override
    public void init(Activity mActivity, IAppUi mAppUi) {
        LogHelper.d(TAG, "[init]");
        mRootViewGroup = mAppUi.getModeRootView();
        View viewLayout = mActivity.getLayoutInflater().inflate(R.layout.continuous_shot_view,
                mRootViewGroup, true);
        mContinuousShotRoot = (ViewGroup) viewLayout.findViewById(R.id.continuous_root);
        mRotateStrokeTextView = (RotateStrokeTextView) viewLayout.findViewById(R.id.shot_num);
    }

    @Override
    public void show(String msg) {
        if (mRotateStrokeTextView != null) {
            LogHelper.d(TAG, "[show] msg = " + msg);
            mRotateStrokeTextView.setText(msg);
            mRotateStrokeTextView.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void hide() {
        if (mRotateStrokeTextView != null) {
            LogHelper.d(TAG, "[hide]");
            mRotateStrokeTextView.setVisibility(View.INVISIBLE);
        }
    }

    @Override
    public void setOrientation(int orientation) {
        if (mRotateStrokeTextView != null) {
            mRotateStrokeTextView.setOrientation(orientation, false);
        }
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "[unInit] mRootViewGroup = " + mRootViewGroup);
        if (mRootViewGroup != null) {
            mRotateStrokeTextView.setVisibility(View.GONE);
            mRootViewGroup.removeView(mRotateStrokeTextView);
            mRootViewGroup.removeView(mContinuousShotRoot);
            mRotateStrokeTextView = null;
            mContinuousShotRoot = null;
            mRootViewGroup = null;
        }
    }
}
