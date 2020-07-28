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

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.provider.MediaStore;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.android.camera.R;
import com.android.camera.v2.ui.RotateImageView;
import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.io.FileDescriptor;

public class ReviewManager extends AbstractUiManager implements View.OnClickListener {
    private static final Tag TAG = new Tag(ReviewManager.class.getSimpleName());
    private ImageView                           mReviewImage;
    private RotateImageView                     mRetakeView;
    private RotateImageView                     mPlayView;
    private FileDescriptor                      mFileDescriptor;
    private String                              mFilePath;
    private Bitmap                              mReviewBitmap;
    private OnClickListener                     mRetakeLisenter;
    private OnClickListener                     mPlayListener;
    private ViewGroup                           mReviewLayer;
    private Activity                            mActivity;
    private Intent                              mIntent;
    private boolean                             mShownByIntent = true;
    private OnRetakeButtonClickListener         mOnRetakeButtonClickListener;
    private OnPlayButtonClickListener           mOnPlayButtonClickListener;

    public interface OnRetakeButtonClickListener {
        public void onRetakeButtonClick();
    }

    public interface OnPlayButtonClickListener {
        public void onPlayButtonClick();
    }

    public ReviewManager(Activity activity, ViewGroup parent) {
        super(activity, parent);
        mActivity = activity;
        mReviewLayer = parent;

        mIntent = activity.getIntent();
        String action = null;
        if (mIntent != null) {
            action = mIntent.getAction();
        }
        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
                || MediaStore.ACTION_VIDEO_CAPTURE.equals(action)
                || CameraUtil.ACTION_STEREO3D.equals(action)) {
            mShownByIntent = true;
        }
    }

    @Override
    public void show() {
        LogHelper.i(TAG, "[show], mShownByIntent:" + mShownByIntent);
        if (mShownByIntent) {
            super.show();
        }
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.review_layout_v2);
        mPlayView = (RotateImageView) view.findViewById(R.id.btn_play);
        mRetakeView = (RotateImageView) view.findViewById(R.id.btn_retake);
        mReviewImage = (ImageView) view.findViewById(R.id.review_image);

        String action = mIntent.getAction();
        LogHelper.i(TAG, "intent.action:" + action);
        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)) {
            mReviewImage.setVisibility(View.GONE);
            mPlayView.setVisibility(View.GONE);
        }
        if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            mPlayView.setVisibility(View.VISIBLE);
        }

        mRetakeView.setOnClickListener(this);
        mPlayView.setOnClickListener(this);
        return view;
    }

    @Override
    public void onClick(View view) {
        if (mPlayView == view) {
            if (mOnPlayButtonClickListener != null) {
                mOnPlayButtonClickListener.onPlayButtonClick();
            }
        }

        if (mRetakeView == view) {
            if (mOnRetakeButtonClickListener != null) {
                mOnRetakeButtonClickListener.onRetakeButtonClick();
            }
        }
    }

    @Override
    protected void onRefresh() {
        LogHelper.i(TAG, "[onRefresh], mReviewImage:" + mReviewImage + "" +
                ", mReviewBitmap:" + mReviewBitmap);
        if (mReviewImage != null && mReviewBitmap != null) {
            mReviewImage.setImageBitmap(mReviewBitmap);
            mReviewImage.setVisibility(View.VISIBLE);
        }
    }

    public void setOnRetakeButtonClickListener(OnRetakeButtonClickListener listener) {
        LogHelper.i(TAG, "[setOnRetakeButtonClickListener], listener:" + listener);
        mOnRetakeButtonClickListener = listener;
    }

    public void setOnPlayButtonClickListener(OnPlayButtonClickListener listener) {
        LogHelper.i(TAG, "[setOnPlayButtonClickListener], listener:" + listener);
        mOnPlayButtonClickListener = listener;
    }

    public void setReviewImage(Bitmap bitmap) {
        mReviewBitmap = bitmap;
        super.show();
    }
}
