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
package com.mediatek.camera.ui.photo;


import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.IReviewUI;
import com.mediatek.camera.common.mode.IReviewUI.ReviewSpec;
import com.mediatek.camera.common.mode.photo.intent.IIntentPhotoUi;

/**
 * An implementation of Intent photo ui.
 */
public class IntentPhotoUi implements IIntentPhotoUi {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(IntentPhotoUi.class.getSimpleName());
    private Activity mActivity;
    private ViewGroup mViewGroup;
    private OkButtonClickListener mOkButtonClickListener;
    private RetakeButtonClickListener mRetakeButtonClickListener;
    private IReviewUI mIReviewUI;
    private ReviewSpec mReviewSpec;
    private IAppUi mIAppUi;
    private boolean mIsShown;

    /**
     * create a intent photo ui instance.
     * @param activity current activity.
     * @param parentView the root view.
     * @param appUi the app ui.
     */
    public IntentPhotoUi(Activity activity, ViewGroup parentView, IAppUi appUi) {
        LogHelper.i(TAG, "[IntentPhotoUi] Construct");
        mActivity = activity;
        mViewGroup = parentView;
        mIAppUi = appUi;
        mIReviewUI = appUi.getReviewUI();
        mReviewSpec = new ReviewSpec();
    }

    @Override
    public void onPictureReceived(byte[] data) {
        LogHelper.d(TAG, "[onPictureReceived]");
        //hide the camera preview ui, will show all the ui when retake pressed.
        mIAppUi.applyAllUIVisibility(View.INVISIBLE);
        //update the listener
        mIReviewUI.initReviewUI(mReviewSpec);
        //when received this message, need show the UI
        mIsShown = true;
        mIReviewUI.showReviewUI(null);
    }

    @Override
    public void setOkButtonClickListener(OkButtonClickListener okButtonClickListener) {
        mOkButtonClickListener = okButtonClickListener;
        mReviewSpec.saveListener = mOkButtonListener;
    }

    @Override
    public void setRetakeButtonClickListener(RetakeButtonClickListener retakeButtonClickListener) {
        mRetakeButtonClickListener = retakeButtonClickListener;
        mReviewSpec.retakeListener = mRetakeListener;
    }

    private View.OnClickListener mOkButtonListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            LogHelper.d(TAG, "[mOkButtonListener]");
            if (mOkButtonClickListener != null) {
                mOkButtonClickListener.onOkClickClicked();
            }
        }
    };

    private View.OnClickListener mRetakeListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            LogHelper.d(TAG, "[mRetakeListener]");
            mIsShown = false;
            mIReviewUI.hideReviewUI();
            mIAppUi.applyAllUIVisibility(View.VISIBLE);
            if (mRetakeButtonClickListener != null) {
                mRetakeButtonClickListener.onRetakeClicked();
            }
        }
    };

    @Override
    public void show() {
        mIsShown = true;
        mIReviewUI.initReviewUI(mReviewSpec);
        mIReviewUI.showReviewUI(null);
    }

    @Override
    public void hide() {
        mIsShown = false;
        mIReviewUI.hideReviewUI();
    }

    @Override
    public boolean isShown() {
        return mIsShown;
    }

    /**
     * update orientation.
     *
     * @param orientation current orientation.
     */
    @Override
    public void onOrientationChanged(int orientation) {
        mIReviewUI.updateOrientation(orientation);
    }
}