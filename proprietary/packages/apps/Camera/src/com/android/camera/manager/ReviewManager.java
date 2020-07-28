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
package com.android.camera.manager;

import android.graphics.Bitmap;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;

import com.android.camera.CameraActivity;
import com.android.camera.R;
import com.android.camera.Thumbnail;
import com.android.camera.Util;
import com.android.camera.ui.RotateImageView;

import com.mediatek.camera.util.Log;

import java.io.FileDescriptor;

public class ReviewManager extends ViewManager implements View.OnClickListener {
    private static final String TAG = "ReviewManager";

    private static final boolean LOG = true;

    private ImageView mReviewImage;
    private RotateImageView mRetakeView;
    private RotateImageView mPlayView;
    private FileDescriptor mFileDescriptor;
    private String mFilePath;
    private int mOrientationCompensation;
    private Bitmap mReviewBitmap;
    private OnClickListener mRetakeLisenter;
    private OnClickListener mPlayListener;

    public ReviewManager(CameraActivity context) {
        super(context, VIEW_LAYER_BOTTOM);
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.review_layout_orig);
        mPlayView = (RotateImageView) view.findViewById(R.id.btn_play);
        mRetakeView = (RotateImageView) view.findViewById(R.id.btn_retake);
        mReviewImage = (ImageView) view.findViewById(R.id.review_image);
        if (mReviewImage != null && getContext().isImageCaptureIntent()) {
            mReviewImage.setVisibility(View.GONE);
        }
        if (mPlayView != null) {
            if (getContext().isImageCaptureIntent()) {
                mPlayView.setVisibility(View.GONE);
            } else {
                mPlayView.setVisibility(View.VISIBLE);
            }
        }
        mRetakeView.setOnClickListener(this);
        mPlayView.setOnClickListener(this);
        return view;
    }

    @Override
    public void onClick(View view) {
        Log.i(TAG, "onClick, view = " + view);
        OnClickListener listener = null;
        if (mRetakeView == view) {
            // listener = getContext().getCameraActor().getRetakeListener();
            listener = mRetakeLisenter;
        } else {
            // listener = getContext().getCameraActor().getPlayListener();
            listener = mPlayListener;
        }
        // press cancel button will delete the file
        // press ok button will send intent to review the file
        // if press cancel button and ok button quickly, the error will occurs
        if (listener != null && view.isShown()) {
            listener.onClick(view);
        }
        if (LOG) {
            Log.d(TAG, "onClick(" + view + ") listener=" + listener);
        }
    }

    @Override
    protected void onRefresh() {
        if (LOG) {
            Log.v(TAG, "onRefresh() mFileDescriptor=" + mFileDescriptor + ", mFilePath="
                    + mFilePath + ", OrientationCompensation=" + mOrientationCompensation
                    + ", mReviewBitmap=" + mReviewBitmap);
        }
        if (mReviewBitmap == null) {
            if (mFileDescriptor != null) {
                mReviewBitmap = Thumbnail.createVideoThumbnailBitmap(mFileDescriptor, getContext()
                        .getPreviewFrameWidth());
            } else if (mFilePath != null) {
                mReviewBitmap = Thumbnail.createVideoThumbnailBitmap(mFilePath, getContext()
                        .getPreviewFrameWidth());
            }
        }
        if (mReviewBitmap != null && mReviewImage != null) {
            // MetadataRetriever already rotates the thumbnail. We should rotate
            // it to match the UI orientation
            mReviewBitmap = Util.rotateAndMirror(mReviewBitmap, -mOrientationCompensation, false);
            mReviewImage.setImageBitmap(mReviewBitmap);
            mReviewImage.setVisibility(View.VISIBLE);
        } else {
            if (mReviewImage != null) {
                mReviewImage.setImageBitmap(null);
            }
        }
    }

    @Override
    protected void onRelease() {
        super.onRelease();
        if (mReviewImage != null) {
            mReviewImage.setImageBitmap(null);
        }
    }

    public void setReviewListener(OnClickListener retatekListener, OnClickListener playListener) {
        mRetakeLisenter = retatekListener;
        mPlayListener = playListener;
    }
    public void show(FileDescriptor fd) {
        if (LOG) {
            Log.v(TAG, "show(" + fd + ") mReviewBitmap=" + mReviewBitmap);
        }
        mFileDescriptor = fd;
        mReviewBitmap = null;
        show();
    }

    public void show(String filePath) {
        if (LOG) {
            Log.v(TAG, "show(" + filePath + ") mReviewBitmap=" + mReviewBitmap);
        }
        mFilePath = filePath;
        mReviewBitmap = null;
        show();
    }

    public void setOrientationCompensation(int orientationCompensation) {
        if (LOG) {
            Log.v(TAG, "setOrientationCompensation(" + orientationCompensation + ")");
        }
        mOrientationCompensation = orientationCompensation;
    }
}
