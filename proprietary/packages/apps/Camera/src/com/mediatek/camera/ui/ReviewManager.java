/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.ui;

import android.app.Activity;
import android.graphics.Bitmap;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;

import com.android.camera.R;
import com.android.camera.Thumbnail;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.util.Log;

import java.io.FileDescriptor;

public class ReviewManager extends CameraView implements View.OnClickListener {
    private static final String TAG = "ReviewManager";

    public static final int UPDATE_SHOW_BITMAP = 0;
    private int mOrientationCompensation;

    private ImageView mReviewImage;
    private RotateImageView mRetakeView;
    private RotateImageView mPlayView;
    private FileDescriptor mFileDescriptor;
    private String mFilePath;
    private Bitmap mReviewBitmap;
    private OnClickListener mRetakeListener;
    private OnClickListener mPlayListener;

    private ICameraAppUi mICameraAppUi;
    private IModuleCtrl mIMoudleCtrl;

    public ReviewManager(Activity context) {
        super(context);
        Log.i(TAG, "[ReviewManager]constructor...");
    }

    @Override
    public void init(Activity activity, ICameraAppUi cameraAppUi, IModuleCtrl moduleCtrl) {
        Log.i(TAG, "[init]...");
        mICameraAppUi = cameraAppUi;
        mIMoudleCtrl = moduleCtrl;
    }

    @Override
    public void uninit() {
        Log.i(TAG, "[uninit]...");
        super.uninit();
        if (mReviewImage != null) {
            mReviewImage.setImageBitmap(null);
        }
    }

    @Override
    public void refresh() {
        Log.i(TAG, "[refresh]...");
        if (mReviewBitmap == null) {
            if (mFileDescriptor != null) {
                mReviewBitmap = Thumbnail.createVideoThumbnailBitmap(mFileDescriptor,
                        mICameraAppUi.getPreviewFrameWidth());
            } else if (mFilePath != null) {
                mReviewBitmap = Thumbnail.createVideoThumbnailBitmap(mFilePath,
                        mICameraAppUi.getPreviewFrameWidth());
            }
        }
    }

    @Override
    public boolean update(int type, Object... args) {
        int info = -1;
        if (args[0] != null) {
            info = Integer.parseInt(args[0].toString());
        }
        Log.i(TAG, "[update] info = " + info);
        switch (info) {
        case UPDATE_SHOW_BITMAP:
            if (args[1] != null) {
                Bitmap bitmap = (Bitmap) args[1];
                show(bitmap);
            }
            break;

        default:
            break;
        }

        return true;
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.review_layout);
        mPlayView = (RotateImageView) view.findViewById(R.id.btn_play);
        mRetakeView = (RotateImageView) view.findViewById(R.id.btn_retake);
        mReviewImage = (ImageView) view.findViewById(R.id.review_image);
        if (mReviewImage != null && mIMoudleCtrl.isVideoCaptureIntent()) {
            mReviewImage.setVisibility(View.GONE);
        }
        if (mPlayView != null) {
            mPlayView.setVisibility(View.VISIBLE);
            mPlayView.setOnClickListener(this);
        }
        if (mRetakeView != null) {
            mRetakeView.setOnClickListener(this);
        }
        return view;
    }

    @Override
    public void onClick(View view) {
        OnClickListener listener = null;
        if (mRetakeView == view) {
            listener = mRetakeListener;
        } else {
            listener = mPlayListener;
        }
        // press cancel button will delete the file
        // press ok button will send intent to review the file
        // if press cancel button and ok button quickly, the error will occurs
        if (listener != null && view.isShown()) {
            listener.onClick(view);
        }
    }

    public void setListener(OnClickListener retakeListener, OnClickListener playListener) {
        mRetakeListener = retakeListener;
        mPlayListener = playListener;
    }

    private void show(Bitmap bitmap) {
        mReviewBitmap = bitmap;
        super.show();
    }
}