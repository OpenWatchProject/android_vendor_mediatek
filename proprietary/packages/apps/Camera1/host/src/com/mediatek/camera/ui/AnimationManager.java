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
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicBlur;
import android.renderscript.ScriptIntrinsicYuvToRGB;
import android.renderscript.Type;
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

    private AnimationTask mAnimationTask;
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
            case TYPE_SWITCH_CAMERA:
//                flipAnimationStart(type, data);
                break;
            case TYPE_CAPTURE:
                mCoverView.setVisibility(View.VISIBLE);
                playCaptureAnimation();
                break;
            case TYPE_SWITCH_MODE:
//                slideAnimationStart(type, data);
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
            case TYPE_SWITCH_CAMERA:
                break;
            case TYPE_CAPTURE:
                mCoverView.setVisibility(View.GONE);
                break;
            case TYPE_SWITCH_MODE:
                break;
            default:
                break;
        }
    }

    /**
     * When preview started, notify animation to do clear task.
     */
    public void onPreviewStarted() {
        if (mAnimationTask == null) {
            mAppUI.removeTopSurface();
        }
    }

    private void slideAnimationStart(AnimationType type, IAppUi.AnimationData data) {
        AsyncData asyncData = new AsyncData();
        asyncData.mType = type;
        asyncData.mData = data;

        mAnimationTask = new AnimationTask();
        mAnimationTask.execute(asyncData);
    }

    /**
     * A async task used to do the animation.
     */
    private class AnimationTask extends AsyncTask<AsyncData, Void, Bitmap> {
        private AsyncData mData;


        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            PreviewSurfaceView surfaceView;
            surfaceView = (PreviewSurfaceView) mApp.getActivity()
                    .findViewById(R.id.preview_surface);
            int previewWidth = Math.max(surfaceView.getWidth(), surfaceView.getHeight());
            int previewHeight = Math.min(surfaceView.getWidth(), surfaceView.getHeight());
            LogHelper.d(TAG, "onPreExecute width " + previewWidth + " height " + previewHeight);
            ViewGroup.LayoutParams params = mAnimationView.getLayoutParams();
            params.width = previewHeight;
            params.height = previewWidth;
            mAnimationView.setLayoutParams(params);
        }

        @Override
        protected Bitmap doInBackground(AsyncData... asyncData) {
            Bitmap sourceBitmap = null;
            mData = asyncData[0];
            IAppUi.AnimationData data = mData.mData;
            LogHelper.d(TAG, "doInBackground format " + data.mFormat + " width " + data.mWidth
                                  + " height " + data.mHeight);
            if (data.mFormat == ImageFormat.NV21) {
                byte[] output = halveYUV420(data.mData, data.mWidth, data.mHeight, YUV_SCALE_RATIO);
                sourceBitmap = convertYuvToRGB(output, data.mWidth / YUV_SCALE_RATIO,
                        data.mHeight / YUV_SCALE_RATIO);
            } else {
                LogHelper.e(TAG, "Now just support NV21 format.");
                return null;
            }

            Bitmap animationBitmap =  createAnimationBitmap(sourceBitmap,
                    data.mOrientation,
                    data.mIsMirror);
            Bitmap result = blurBitmap(animationBitmap);
            return result;
        }

        @Override
        protected void onPostExecute(final Bitmap result) {
            super.onPostExecute(result);
            LogHelper.d(TAG, "onPostExecute type " + mData.mType);
            if (result == null) {
                LogHelper.e(TAG, "The result bitmap is null!");
                return;
            }
            switch(mData.mType) {
                case TYPE_SWITCH_CAMERA:
                    mAnimationRoot.setVisibility(View.VISIBLE);
                    mAnimationView.setImageBitmap(result);
                    mAnimationView.setVisibility(View.VISIBLE);
                    playSwitchCameraAnimation();
                    break;
                case TYPE_SWITCH_MODE:
                    mAnimationRoot.setVisibility(View.VISIBLE);
                    mAnimationView.setImageBitmap(result);
                    mAnimationView.setVisibility(View.VISIBLE);
                    playSlideAnimation();
                    break;
                default:
                    break;
            }
            mAnimationTask = null;
        }
    }

    private void slideAnimationEnd() {
        LogHelper.d(TAG, "slideAnimationEnd");
        mAnimationView.setVisibility(View.GONE);
        mAnimationRoot.setVisibility(View.GONE);
    }

    private void flipAnimationStart(AnimationType type, IAppUi.AnimationData data) {
        LogHelper.d(TAG, "flipAnimationStart +");
        if (data != null && data.mData != null) {
            mApp.getAppUi().applyAllUIEnabled(false);
            AsyncData asyncData = new AsyncData();
            asyncData.mType = type;
            asyncData.mData = data;

            mAnimationTask = new AnimationTask();
            mAnimationTask.execute(asyncData);
        } else {
            LogHelper.e(TAG, "The animation data is null, cannot do the animation!");
        }

        LogHelper.d(TAG, "flipAnimationStart -");
    }

    private void flipAnimationEnd() {
        boolean flag = mFlipAnimation != null && mFlipAnimation.isStarted();
        LogHelper.d(TAG, "flipAnimationEnd  " + flag);
    }

    private void playSlideAnimation() {
        ObjectAnimator fadeIn = ObjectAnimator.ofFloat(mAnimationRoot, "alpha", 0.8f, 1.0f);
        fadeIn.setInterpolator(new AccelerateInterpolator());
        fadeIn.setDuration(100);
        fadeIn.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "fade in animation end");
            }

            @Override
            public void onAnimationCancel(Animator animator) {

            }

            @Override
            public void onAnimationRepeat(Animator animator) {

            }
        });

        ObjectAnimator fadeOut = ObjectAnimator.ofFloat(mAnimationRoot, "alpha", 1.0f, 0.8f);
        fadeOut.setDuration(100);
        fadeOut.setInterpolator(new AccelerateInterpolator());
        fadeOut.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {
            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "fade out animation end");
                //reset the view to init state.
                mAnimationRoot.setVisibility(View.GONE);
                mAnimationView.setVisibility(View.GONE);
            }

            @Override
            public void onAnimationCancel(Animator animator) {
            }

            @Override
            public void onAnimationRepeat(Animator animator) {
            }
        });

        ObjectAnimator fadeOut2 = ObjectAnimator.ofFloat(mAnimationRoot, "alpha", 0.4f, 0.0f);
        fadeOut2.setDuration(200);
        fadeOut2.setInterpolator(new AccelerateInterpolator());
        fadeOut2.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "fade out animation end");
                //reset the view to init state.
                mAnimationRoot.setVisibility(View.GONE);
                mAnimationView.setVisibility(View.GONE);
            }

            @Override
            public void onAnimationCancel(Animator animator) {
            }

            @Override
            public void onAnimationRepeat(Animator animator) {
            }
        });



        AnimatorSet slideAnimation = new AnimatorSet();
        slideAnimation.playSequentially(fadeIn, fadeOut);
        slideAnimation.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "switch animation end");
                mAppUI.removeTopSurface();
                mAnimationView.setVisibility(View.GONE);
                mAnimationRoot.setVisibility(View.GONE);
                mAnimationRoot.setAlpha(0);
            }

            @Override
            public void onAnimationCancel(Animator animator) {

            }

            @Override
            public void onAnimationRepeat(Animator animator) {

            }
        });
        slideAnimation.start();
    }

    private void playSwitchCameraAnimation() {
        ObjectAnimator fadeIn = ObjectAnimator.ofFloat(mAnimationRoot, "alpha", 0.4f, 1.0f);
        fadeIn.setInterpolator(new AccelerateInterpolator());
        fadeIn.setDuration(200);
        fadeIn.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "fade in animation end");
                mAppUI.removeTopSurface();
            }

            @Override
            public void onAnimationCancel(Animator animator) {

            }

            @Override
            public void onAnimationRepeat(Animator animator) {

            }
        });

        AnimatorSet flip = (AnimatorSet) AnimatorInflater.loadAnimator(mApp.getActivity(),
                R.animator.flip_anim);
        flip.setTarget(mAnimationView);
        flip.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "flip animation end");
            }

            @Override
            public void onAnimationCancel(Animator animator) {

            }

            @Override
            public void onAnimationRepeat(Animator animator) {

            }
        });

        ObjectAnimator fadeOut = ObjectAnimator.ofFloat(mAnimationRoot, "alpha", 1.0f, 0.0f);
        fadeOut.setDuration(400);
        fadeOut.setInterpolator(new AccelerateInterpolator());
        fadeOut.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {
            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "fade out animation end");
                //reset the view to init state.
                mAnimationRoot.setVisibility(View.GONE);
                mAnimationView.setVisibility(View.GONE);
                AnimatorSet resetAnimation =
                        (AnimatorSet) AnimatorInflater.loadAnimator(mApp.getActivity(),
                                R.animator.flip_anim_reset);
                resetAnimation.setTarget(mAnimationView);
                resetAnimation.start();

            }

            @Override
            public void onAnimationCancel(Animator animator) {
            }

            @Override
            public void onAnimationRepeat(Animator animator) {
            }
        });

        mSwitchCameraAnimator = new AnimatorSet();
        mSwitchCameraAnimator.playSequentially(fadeIn, flip, fadeOut);
        mSwitchCameraAnimator.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {
                LogHelper.d(TAG, "switch animation end");
                mAppUI.removeTopSurface();
                mAnimationRoot.setAlpha(0);
                mApp.getAppUi().applyAllUIEnabled(true);
            }

            @Override
            public void onAnimationCancel(Animator animator) {

            }

            @Override
            public void onAnimationRepeat(Animator animator) {

            }
        });
        mSwitchCameraAnimator.start();
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

    private Bitmap convertYuvToRGB(byte[] yuvData, int width, int height) {
        LogHelper.d(TAG, "convertYuvToRGB +");
        RenderScript rs = RenderScript.create(mApp.getActivity().getApplicationContext());
        ScriptIntrinsicYuvToRGB convertScript =
                ScriptIntrinsicYuvToRGB.create(rs, Element.RGBA_8888(rs));

        Type.Builder tb = new Type.Builder(rs, Element.createPixel(rs, Element.DataType.UNSIGNED_8,
                Element.DataKind.PIXEL_YUV));
        tb.setX(width);
        tb.setY(height);
        tb.setMipmaps(false);
        tb.setYuvFormat(ImageFormat.NV21);

        Allocation ain = Allocation.createTyped(rs, tb.create(), Allocation.USAGE_SCRIPT);

        ain.copyFrom(yuvData);

        Type.Builder tb2 = new Type.Builder(rs, Element.RGBA_8888(rs));
        tb2.setX(width);
        tb2.setY(height);
        tb2.setMipmaps(false);

        Allocation aout = Allocation.createTyped(rs, tb2.create(), Allocation.USAGE_SCRIPT);

        convertScript.setInput(ain);
        convertScript.forEach(aout);

        Bitmap bitmap = Bitmap.createBitmap(width , height, Bitmap.Config.ARGB_8888);

        aout.copyTo(bitmap);

        //After finishing everything, we destroy the Renderscript.
        rs.destroy();
        LogHelper.d(TAG, "convertYuvToRGB -");
        return bitmap;
    }


    private Bitmap createAnimationBitmap(Bitmap srcBitmap, int rotation, boolean isMirror) {
        LogHelper.d(TAG, "createAnimationBitmap + isMirror " + isMirror + " rotation " + rotation);
        int srcWidth = srcBitmap.getWidth();
        int srcHeight = srcBitmap.getHeight();

        Matrix matrix = new Matrix();
        if (isMirror) {
            matrix.postScale(1, -1);
        }
        matrix.postRotate(rotation);
        Bitmap result = Bitmap.createBitmap(srcBitmap, 0, 0, srcWidth, srcHeight, matrix, true);
        LogHelper.d(TAG, "createAnimationBitmap -");
        return result;
    }

    private Bitmap blurBitmap(Bitmap bitmap) {
        LogHelper.d(TAG, "blurBitmap +");
        //Let's create an empty bitmap with the same size of the bitmap we want to blur
        Bitmap outBitmap = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(),
                Bitmap.Config.ARGB_8888);

        //Instantiate a new Renderscript
        RenderScript rs = RenderScript.create(mApp.getActivity().getApplicationContext());

        //Create an Intrinsic Blur Script using the Renderscript
        ScriptIntrinsicBlur blurScript = ScriptIntrinsicBlur.create(rs, Element.U8_4(rs));

        //Create the Allocations (in/out) with the Renderscript and the in/out bitmaps
        Allocation allIn = Allocation.createFromBitmap(rs, bitmap);
        Allocation allOut = Allocation.createFromBitmap(rs, outBitmap);

        //Set the radius of the blur
        blurScript.setRadius(25.f);

        //Perform the Renderscript
        blurScript.setInput(allIn);
        blurScript.forEach(allOut);

        //Copy the final bitmap created by the out Allocation to the outBitmap
        allOut.copyTo(outBitmap);

        //recycle the original bitmap
        bitmap.recycle();

        //After finishing everything, we destroy the Renderscript.
        rs.destroy();
        bitmap.recycle();
        LogHelper.d(TAG, "blurBitmap -");
        return outBitmap;
    }

    private static byte[] halveYUV420(byte[] data, int imageWidth,
                                      int imageHeight, int scaleRatio) {
        LogHelper.d(TAG, "halveYUV420 +");
        byte[] yuv = new byte[imageWidth / scaleRatio * imageHeight / scaleRatio * 3 / 2];
        // halve yuma
        int i = 0;
        for (int y = 0; y < imageHeight; y += scaleRatio) {
            for (int x = 0; x < imageWidth; x += scaleRatio) {
                yuv[i] = data[y * imageWidth + x];
                i++;
            }
        }
        // halve U and V color components
        for (int y = 0; y < imageHeight / 2; y += scaleRatio) {
            for (int x = 0; x < imageWidth; x += 2 * scaleRatio) {
                yuv[i] = data[(imageWidth * imageHeight) + (y * imageWidth) + x];
                i++;
                yuv[i] = data[(imageWidth * imageHeight) + (y * imageWidth) + (x + 1)];
                i++;
            }
        }
        LogHelper.d(TAG, "halveYUV420 -");
        return yuv;
    }
}
