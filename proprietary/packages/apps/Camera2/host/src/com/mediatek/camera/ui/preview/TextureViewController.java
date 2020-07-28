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
package com.mediatek.camera.ui.preview;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.R;

import com.mediatek.camera.common.IAppUiListener.ISurfaceStatusListener;
import com.mediatek.camera.common.app.IApp;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Camera preview controller for TextureView.
 */

class TextureViewController implements IController {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(TextureViewController.class.getSimpleName());

    private IApp mApp;
    private int mPreviewWidth = 0;
    private int mPreviewHeight = 0;
    private double mPreviewAspectRatio = 0.0d;
    private View.OnLayoutChangeListener mOnLayoutChangeListener;
    private View.OnTouchListener mOnTouchListener;
    private SurfaceChangeCallback mSurfaceChangeCallback;
    private ViewGroup mPreviewRoot;
    private ViewGroup mPreviewContainer;
    private ViewGroup mLastPreviewContainer;
    private PreviewTextureView mTextureView;
    private BlockingQueue<View> mFrameLayoutQueue =
            new LinkedBlockingQueue<View>();

    private boolean mIsSurfaceCreated = false;

    public TextureViewController(IApp app) {
        mApp = app;
        mPreviewRoot = (ViewGroup) mApp.getActivity().findViewById(R.id.preview_frame_root);
    }

    @Override
    public void updatePreviewSize(int width, int height, ISurfaceStatusListener listener) {
        LogHelper.i(TAG, "updatePreviewSize: new size (" + width + " , " + height + " )"
                + " current size (" + mPreviewWidth + " , " + mPreviewHeight + " )" + "," +
                "mIsSurfaceCreated = " + mIsSurfaceCreated +
                " listener = " + listener);
        if (mPreviewWidth == width && mPreviewHeight == height) {
            //If preview size is same, just call back surface available.
            ISurfaceStatusListener l = mSurfaceChangeCallback.getBindStatusListener();
            if (listener != null && listener != l) {
                mTextureView.setSurfaceTextureListener(null);
                mSurfaceChangeCallback = new SurfaceChangeCallback(listener);
                mTextureView.setSurfaceTextureListener(mSurfaceChangeCallback);
            }
            if (mIsSurfaceCreated) {
                if (listener != null && mTextureView.isAvailable()) {
                    mTextureView.getSurfaceTexture()
                            .setDefaultBufferSize(mPreviewWidth, mPreviewHeight);
                    listener.surfaceAvailable(mTextureView.getSurfaceTexture(),
                            mPreviewWidth, mPreviewHeight);
                }
            }
            return;
        }

        double ratio = (double) Math.max(width, height)
                / Math.min(width, height);


        if (ratio == mPreviewAspectRatio) {
            mPreviewWidth = width;
            mPreviewHeight = height;
            if (mTextureView.isAvailable()) {
                mTextureView.getSurfaceTexture().setDefaultBufferSize(mPreviewWidth,
                        mPreviewHeight);
            }

            if (listener != null) {
                listener.surfaceAvailable(mTextureView.getSurfaceTexture(),
                        mPreviewWidth, mPreviewHeight);
                ISurfaceStatusListener l = mSurfaceChangeCallback.getBindStatusListener();
                if (listener != l) {
                    mTextureView.setSurfaceTextureListener(null);
                    mSurfaceChangeCallback = new SurfaceChangeCallback(listener);
                    mTextureView.setSurfaceTextureListener(mSurfaceChangeCallback);
                }
            }
            return;
        }

        if (mPreviewAspectRatio != 0) {
            mLastPreviewContainer = mPreviewContainer;
            mTextureView = null;
        }
        mPreviewWidth = width;
        mPreviewHeight = height;
        mPreviewAspectRatio = (double) Math.max(width, height)
                / Math.min(width, height);
        if (mTextureView == null) {
            attachTextureView(listener);
        } else {
            ISurfaceStatusListener l = mSurfaceChangeCallback.getBindStatusListener();
            if (listener != null && listener != l) {
                mTextureView.setSurfaceTextureListener(null);
                mSurfaceChangeCallback = new SurfaceChangeCallback(listener);
                mTextureView.setSurfaceTextureListener(mSurfaceChangeCallback);
                listener.surfaceAvailable(mTextureView.getSurfaceTexture(),
                        mPreviewWidth, mPreviewHeight);
            }
        }
        mTextureView.setAspectRatio(mPreviewAspectRatio);
    }

    @Override
    public View getView() {
        return null;
    }

    @Override
    public void removeTopSurface() {
        int queueSize = mFrameLayoutQueue.size();
        LogHelper.d(TAG, "removeTopSurface size = " + queueSize);
        for (int i = 0; i < queueSize; i++) {
            View view = mFrameLayoutQueue.poll();
            if (view != null) {
                view.setVisibility(View.GONE);
                mPreviewRoot.removeView(view);
            }
        }
        mLastPreviewContainer = null;
    }

    @Override
    public void setOnLayoutChangeListener(View.OnLayoutChangeListener layoutChangeListener) {
        mOnLayoutChangeListener = layoutChangeListener;
    }

    @Override
    public void setOnTouchListener(View.OnTouchListener onTouchListener) {
        mOnTouchListener = onTouchListener;
    }

    @Override
    public void clearPreviewStatusListener(ISurfaceStatusListener listener) {

    }

    @Override
    public void setEnabled(boolean enabled) {
        if (mTextureView != null) {
            mTextureView.setEnabled(enabled);
        }
    }

    @Override
    public void onPause() {

    }

    @Override
    public Bitmap getPreviewBitmap(int downSample) {
        if (mTextureView == null) {
            return null;
        }
        RectF textureArea = getTextureArea();
        int width = (int) textureArea.width() / downSample;
        int height = (int) textureArea.height() / downSample;
        Bitmap preview = mTextureView.getBitmap(width, height);
        Bitmap n = Bitmap.createBitmap(
                preview, 0, 0, width, height, mTextureView.getTransform(null), true);
        return n;
    }

    private void attachTextureView(ISurfaceStatusListener listener) {
        ViewGroup container = (ViewGroup) mApp.getActivity().getLayoutInflater().inflate(
                R.layout.textureview_layout, null);
        PreviewTextureView textureView =
                (PreviewTextureView) container.findViewById(R.id.preview_surface);

        if (mLastPreviewContainer != null) {
            TextureView texture =
                    (TextureView) mLastPreviewContainer.findViewById(R.id.preview_surface);
            texture.removeOnLayoutChangeListener(mOnLayoutChangeListener);
            texture.setSurfaceTextureListener(null);
            textureView.setOnTouchListener(null);
            if (mSurfaceChangeCallback != null) {
                mSurfaceChangeCallback.onSurfaceTextureDestroyed(texture.getSurfaceTexture());
            }
            mLastPreviewContainer.bringToFront();
            if (!mFrameLayoutQueue.contains(mLastPreviewContainer)) {
                mFrameLayoutQueue.add(mLastPreviewContainer);
            }
        }

        mSurfaceChangeCallback = new SurfaceChangeCallback(listener);
        textureView.setSurfaceTextureListener(mSurfaceChangeCallback);
        textureView.addOnLayoutChangeListener(mOnLayoutChangeListener);
        textureView.setOnTouchListener(mOnTouchListener);
        mPreviewRoot.addView(container, 0);
        mTextureView = textureView;
        mPreviewContainer = container;
    }

    private RectF getTextureArea() {
        Matrix matrix = new Matrix();
        RectF area = new RectF(0, 0, mTextureView.getWidth(), mTextureView.getHeight());
        mTextureView.getTransform(matrix).mapRect(area);
        return area;
    }

    /**
     * Surface change call back receiver, it bind a status change listener.
     * When surface status change, use the listener to notify the change.
     */
    private class SurfaceChangeCallback implements TextureView.SurfaceTextureListener {
        private ISurfaceStatusListener mListener;

         SurfaceChangeCallback(ISurfaceStatusListener listener) {
            mListener = listener;
        }

         ISurfaceStatusListener getBindStatusListener() {
            return mListener;
        }

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            mIsSurfaceCreated = true;
            surface.setDefaultBufferSize(mPreviewWidth, mPreviewHeight);
            if (mListener != null) {
                mListener.surfaceChanged(surface, mPreviewWidth, mPreviewHeight);
            }
            LogHelper.d(TAG, "onSurfaceTextureAvailable surface  = " + surface +
             " width " + width + " height " + height);
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
            if (mListener != null) {
                mListener.surfaceChanged(surface, width, height);
            }
            LogHelper.d(TAG, "onSurfaceTextureSizeChanged surface  = " + surface +
                    " width " + width + " height " + height);
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
            mIsSurfaceCreated = false;
            if (mListener != null) {
                mListener.surfaceDestroyed(surface,  mPreviewWidth, mPreviewHeight);
            }
            LogHelper.d(TAG, "onSurfaceTextureDestroyed surface  = " + surface);
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surface) {

        }
    }

}
