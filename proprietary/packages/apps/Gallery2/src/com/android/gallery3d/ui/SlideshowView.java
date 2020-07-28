/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.gallery3d.ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.PointF;

import com.android.gallery3d.R;
import com.android.gallery3d.anim.CanvasAnimation;
import com.android.gallery3d.anim.FloatAnimation;
import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.glrenderer.BitmapTexture;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.android.gallery3d.glrenderer.ResourceTexture;
import com.android.gallery3d.glrenderer.Texture;

import com.mediatek.gallery3d.adapter.FeatureManager;
import com.mediatek.gallerybasic.base.ISlideshowRenderer;

import java.util.Random;

public class SlideshowView extends GLView {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/SlideshowView";

    private static final int SLIDESHOW_DURATION = 3500;
    private static final int TRANSITION_DURATION = 1000;

    private static final float SCALE_SPEED = 0.20f ;
    private static final float MOVE_SPEED = SCALE_SPEED;

    private int mCurrentRotation;
    private BitmapTexture mCurrentTexture;
    private SlideshowAnimation mCurrentAnimation;

    private int mPrevRotation;
    private BitmapTexture mPrevTexture;
    private SlideshowAnimation mPrevAnimation;

    private final FloatAnimation mTransitionAnimation =
            new FloatAnimation(0, 1, TRANSITION_DURATION);

    private Random mRandom = new Random();

    /// M: [BUG.ADD] @{
    public SlideshowView(AbstractGalleryActivity activity) {
        mContext = activity.getAndroidContext();
        mVideoPlayIcon = new ResourceTexture(mContext,
                R.drawable.ic_control_play);
        /// M: [FEATURE.ADD] @{
        mExtRenders =
                (ISlideshowRenderer[]) FeatureManager.getInstance().getImplement(
                        ISlideshowRenderer.class, mContext, mContext.getResources());
        /// @}
    }
    /// @}

    public void next(Bitmap bitmap, int rotation) {

        mTransitionAnimation.start();

        if (mPrevTexture != null) {
            mPrevTexture.getBitmap().recycle();
            mPrevTexture.recycle();
        }

        mPrevTexture = mCurrentTexture;
        mPrevAnimation = mCurrentAnimation;
        mPrevRotation = mCurrentRotation;

        mCurrentRotation = rotation;
        mCurrentTexture = new BitmapTexture(bitmap);
        if (((rotation / 90) & 0x01) == 0) {
            mCurrentAnimation = new SlideshowAnimation(
                    mCurrentTexture.getWidth(), mCurrentTexture.getHeight(),
                    mRandom);
        } else {
            mCurrentAnimation = new SlideshowAnimation(
                    mCurrentTexture.getHeight(), mCurrentTexture.getWidth(),
                    mRandom);
        }
        mCurrentAnimation.start();

        invalidate();
    }

    public void release() {
        if (mPrevTexture != null) {
            mPrevTexture.recycle();
            mPrevTexture = null;
        }
        if (mCurrentTexture != null) {
            mCurrentTexture.recycle();
            mCurrentTexture = null;
        }
    }

    @Override
    protected void render(GLCanvas canvas) {
        long animTime = AnimationTime.get();
        boolean requestRender = mTransitionAnimation.calculate(animTime);
        float alpha = mPrevTexture == null ? 1f : mTransitionAnimation.get();
        /// M: [FEATURE.ADD] @{
        boolean skipStandardFlow = false;
        /// @}
        if (mPrevTexture != null && alpha != 1f) {
            requestRender |= mPrevAnimation.calculate(animTime);
            canvas.save(GLCanvas.SAVE_FLAG_ALPHA | GLCanvas.SAVE_FLAG_MATRIX);
            canvas.setAlpha(1f - alpha);
            mPrevAnimation.apply(canvas);
            canvas.rotate(mPrevRotation, 0, 0, 1);
            /// M: [FEATURE.ADD] @{
            for (ISlideshowRenderer render : mExtRenders) {
                skipStandardFlow =
                        render.renderPrevContent(canvas.getMGLCanvas(), mPrevTexture
                                .getWidth(), mPrevTexture.getHeight(), mPrevMediaItem
                                .getMediaData());
            }
            /// @}
            /// M: [FEATURE.MODIFY] @{
            /*
                mPrevTexture.draw(canvas, -mPrevTexture.getWidth() / 2,
                        -mPrevTexture.getHeight() / 2);
            */
            if (!skipStandardFlow) {
                mPrevTexture.draw(canvas, -mPrevTexture.getWidth() / 2,
                        -mPrevTexture.getHeight() / 2);
            }
            /// @}
            /// M: [FEATURE.ADD] @{
            for (ISlideshowRenderer render : mExtRenders) {
                render.renderPrevCover(canvas.getMGLCanvas(), -mPrevTexture
                                .getWidth() / 2, -mPrevTexture.getHeight() / 2,
                        mPrevTexture.getWidth(),
                        mPrevTexture.getHeight(), mPrevMediaItem.getMediaData(),
                        1.0f / mPrevAnimation.getCurrentScale());
            }
            /// @}
            canvas.restore();
            /// M: [BUG.ADD] @{
            if (mPrevMediaItem.getMediaType() == MediaObject.MEDIA_TYPE_VIDEO) {
                drawVideoOverlay(canvas);
            }
            /// @}
        }
        if (mCurrentTexture != null) {
            requestRender |= mCurrentAnimation.calculate(animTime);
            canvas.save(GLCanvas.SAVE_FLAG_ALPHA | GLCanvas.SAVE_FLAG_MATRIX);
            canvas.setAlpha(alpha);
            mCurrentAnimation.apply(canvas);
            canvas.rotate(mCurrentRotation, 0, 0, 1);
            /// M: [FEATURE.ADD] @{
            skipStandardFlow = false;
            for (ISlideshowRenderer render : mExtRenders) {
                skipStandardFlow =
                        render.renderCurrentContent(canvas.getMGLCanvas(), mCurrentTexture
                                .getWidth(), mCurrentTexture.getHeight(), mCurrentMediaItem
                                .getMediaData());
            }
            /// @}
            /// M: [FEATURE.MODIFY] @{
            /*mCurrentTexture.draw(canvas, -mCurrentTexture.getWidth() / 2,
                    -mCurrentTexture.getHeight() / 2);*/
            if (!skipStandardFlow) {
                mCurrentTexture.draw(canvas, -mCurrentTexture.getWidth() / 2,
                        -mCurrentTexture.getHeight() / 2);
            }
            /// @}
            /// M: [FEATURE.ADD] @{
            for (ISlideshowRenderer render : mExtRenders) {
                render.renderCurrentCover(canvas.getMGLCanvas(), -mCurrentTexture
                        .getWidth() / 2, -mCurrentTexture.getHeight() / 2, mCurrentTexture
                        .getWidth(), mCurrentTexture.getHeight(), mCurrentMediaItem
                        .getMediaData(), 1.0f / mCurrentAnimation.getCurrentScale());
            }
            /// @}
            canvas.restore();
            /// M: [BUG.ADD] @{
            if (mCurrentMediaItem.getMediaType() == MediaObject.MEDIA_TYPE_VIDEO) {
                drawVideoOverlay(canvas);
            }
            /// @}
        }
        if (requestRender)

            invalidate();
    }

    private class SlideshowAnimation extends CanvasAnimation {
        private final int mWidth;
        private final int mHeight;

        private final PointF mMovingVector;
        private float mProgress;
        /// M: [BUG.ADD] @{
        private float mCurrentScale = 1.0f;
        private MediaItem mMediaItem;
        /// @}
        public SlideshowAnimation(int width, int height, Random random) {
            mWidth = width;
            mHeight = height;
            mMovingVector = new PointF(
                    MOVE_SPEED * mWidth * (random.nextFloat() - 0.5f),
                    MOVE_SPEED * mHeight * (random.nextFloat() - 0.5f));
            setDuration(SLIDESHOW_DURATION);
        }

        /// M: [BUG.ADD] @{
        public SlideshowAnimation(int width, int height, Random random, MediaItem mediaItem) {
            this(width, height, random);
            mMediaItem = mediaItem;
        }
        /// @}

        @Override
        public void apply(GLCanvas canvas) {
            int viewWidth = getWidth();
            int viewHeight = getHeight();

            float initScale = Math.min((float)
                    viewWidth / mWidth, (float) viewHeight / mHeight);
            float scale = initScale * (1 + SCALE_SPEED * mProgress);
            /// M: [BUG.ADD] @{
            mCurrentScale = initScale;
            /// @}
            float centerX = viewWidth / 2 + mMovingVector.x * mProgress;
            float centerY = viewHeight / 2 + mMovingVector.y * mProgress;

            canvas.translate(centerX, centerY);
            canvas.scale(scale, scale, 0);
        }

        @Override
        public int getCanvasSaveFlags() {
            return GLCanvas.SAVE_FLAG_MATRIX;
        }

        @Override
        protected void onCalculate(float progress) {
            mProgress = progress;
        }
        /// M: [BUG.ADD] @{
        public float getCurrentScale() {
            return mCurrentScale;
        }
        /// @}
    }

    /// M: [FEATURE.ADD] plugin @{
    public void next(Bitmap bitmap, int rotation, MediaItem item) {
        mTransitionAnimation.start();
        if (mPrevTexture != null) {
            mPrevTexture.getBitmap().recycle();
            mPrevTexture.recycle();
        }
        mPrevTexture = mCurrentTexture;
        mPrevAnimation = mCurrentAnimation;
        mPrevRotation = mCurrentRotation;
        /// M: [BUG.ADD] @{
        mPrevMediaItem = mCurrentMediaItem;
        mCurrentMediaItem = item;
        /// @}

        mCurrentRotation = rotation;
        mCurrentTexture = new BitmapTexture(bitmap);
        /// M: [BUG.MODIFY] @{
        /*
        if (((rotation / 90) & 0x01) == 0) {
            mCurrentAnimation = new SlideshowAnimation(
                    mCurrentTexture.getWidth(), mCurrentTexture.getHeight(),
                    mRandom);
        } else {
            mCurrentAnimation = new SlideshowAnimation(
                    mCurrentTexture.getHeight(), mCurrentTexture.getWidth(),
                    mRandom);
        }
        */
        if (((rotation / 90) & 0x01) == 0) {
            mCurrentAnimation = new SlideshowAnimation(
                    mCurrentTexture.getWidth(), mCurrentTexture.getHeight(),
                    mRandom, mCurrentMediaItem);
        } else {
            mCurrentAnimation = new SlideshowAnimation(
                    mCurrentTexture.getHeight(), mCurrentTexture.getWidth(),
                    mRandom, mCurrentMediaItem);
        }
        /// @}
        mCurrentAnimation.start();
        invalidate();
    }
    /// @}

  //********************************************************************
  //*                              MTK                                 *
  //********************************************************************

    private MediaItem mCurrentMediaItem ;
    private MediaItem mPrevMediaItem;
    private Context mContext;
    private Texture mVideoPlayIcon;
    protected void drawVideoOverlay(GLCanvas canvas) {
        int viewWidth = this.getWidth();
        int viewHeight = this.getHeight();
        int iconSize = (int) (Math.min(viewWidth, viewHeight) + 0.5f) / 6;
        mVideoPlayIcon.draw(canvas, (viewWidth - iconSize) / 2,
                (viewHeight - iconSize) / 2, iconSize, iconSize);
    }

    private ISlideshowRenderer[] mExtRenders;
}
