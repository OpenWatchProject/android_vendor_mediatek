/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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

import android.graphics.Bitmap;
import android.graphics.RectF;

import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.glrenderer.BitmapTexture;
import com.android.gallery3d.glrenderer.GLCanvas;

public class BitmapScreenNail implements ScreenNail {
/// M: [BUG.ADD] @{
        private static final String TAG = "Gallery2/BitmapScreenNail";
    protected static final int PLACEHOLDER_COLOR = 0xFF222222;
    // The duration of the fading animation in milliseconds
    private static final int DURATION = 40;

    //private static final int MAX_SIDE = 640;
    private static int sMaxSide = 640;

    // These are special values for mAnimationStartTime
    private static final long ANIMATION_NOT_NEEDED = -1;
    private static final long ANIMATION_NEEDED = -2;
    private static final long ANIMATION_DONE = -3;

    protected int mWidth;
    protected int mHeight;
    protected Bitmap mBitmap;
    protected long mAnimationStartTime = ANIMATION_NOT_NEEDED;
    // added for performance auto test
    public static long mWaitFinishedTime = 0;
    /// @}
    /// M: [BUG.MODIFY] @{
    /*  private final BitmapTexture mBitmapTexture;*/
    protected BitmapTexture mTexture;
    /// @}

    public BitmapScreenNail(Bitmap bitmap) {
        /// M: [DEBUG.MODIFY] @{
        /*        mBitmapTexture = new BitmapTexture(bitmap);*/
        mWidth = bitmap.getWidth();
        mHeight = bitmap.getHeight();
        mBitmap = bitmap;
        /// @}
        // We create mTexture lazily, so we don't incur the cost if we don't
        // actually need it.
    }

    @Override
    public int getWidth() {
        /// M: [DEBUG.MODIFY] @{
        /*        return mBitmapTexture.getWidth();*/
        return mWidth;
        /// @}
    }

    @Override
    public int getHeight() {
        // / M: [BUG.MODIFY] @{
        // return mBitmapTexture.getHeight();
        return mHeight;
        // / @}
    }

    @Override
    public void draw(GLCanvas canvas, int x, int y, int width, int height) {
        /// M: [BUG.MODIFY] @{
        /*        mBitmapTexture.draw(canvas, x, y, width, height);
        */
        if (mBitmap == null) {
            if (mAnimationStartTime == ANIMATION_NOT_NEEDED) {
                mAnimationStartTime = ANIMATION_NEEDED;
            }
            canvas.fillRect(x, y, width, height, PLACEHOLDER_COLOR);
            /// M: added for performance auto test
            mWaitFinishedTime = System.currentTimeMillis();
            return;
        }

        if (mTexture == null) {
            // The first parameter is true, Recycle bitmap after uploaded texture.
            mTexture = new BitmapTexture(true, mBitmap);
        }

        if (mAnimationStartTime == ANIMATION_NEEDED) {
            mAnimationStartTime = now();
        }

        if (isAnimating()) {
            canvas.drawMixed(mTexture, PLACEHOLDER_COLOR, getRatio(), x, y,
                    width, height);
        } else {
            mTexture.draw(canvas, x, y, width, height);
        }

        // debug code especially for high quality screenail in PhotoPage
        if (mIsDebugEnable) {
            canvas.fillRect(x, y, width, height, 0x66660000);
        }
        /// @}
    }

    @Override
    public void noDraw() {
        // do nothing
    }

    @Override
    public void recycle() {
        /// M: [BUG.MODIFY] @{
        // mBitmapTexture.recycle();
        if (mTexture != null) {
            mTexture.recycle();
            mTexture = null;
        }
        /// @}
        /// M: [BUG.ADD] For optimize memory, bitmap is recycled after uploaded the texture.@{
        mBitmap = null;
        /// @}
    }

    @Override
    public void draw(GLCanvas canvas, RectF source, RectF dest) {
        /// M: [BUG.MODIFY] @{
        /*        canvas.drawTexture(mBitmapTexture, source, dest);*/
        if (mBitmap == null) {
            canvas.fillRect(dest.left, dest.top, dest.width(), dest.height(),
                    PLACEHOLDER_COLOR);
            return;
        }

        if (mTexture == null) {
            /// M: The first parameter is true, Recycle bitmap after uploaded texture.
            mTexture = new BitmapTexture(true, mBitmap);
        }

        canvas.drawTexture(mTexture, source, dest);

        // debug code especially for high quality screenail in PhotoPage
        if (mIsDebugEnable) {
            canvas.fillRect(dest.left, dest.top, dest.width(), dest.height(), 0x66660000);
        }
        /// @}
    }

  //********************************************************************
  //*                              MTK                                 *
  //********************************************************************

    public BitmapScreenNail(int width, int height) {
        setSize(width, height);
    }

    public void setSize(int width, int height) {
//      if (width == 0 || height == 0) {
//          width = 640;
//          height = 480;
//      }
//      float scale = Math.min(1, (float) MAX_SIDE / Math.max(width, height));
      if (width == 0 || height == 0) {
          width = sMaxSide;
          height = sMaxSide * 3 / 4;
      }
      float scale = Math.min(1, (float) sMaxSide / Math.max(width, height));
      mWidth = Math.round(scale * width);
      mHeight = Math.round(scale * height);
  }

    // Combines the two ScreenNails.
    // Returns the used one and recycle the unused one.
    public ScreenNail combine(ScreenNail other) {
        if (other == null) {
            return this;
        }

        if (!(other instanceof BitmapScreenNail)) {
            recycle();
            return other;
        }

        // Now both are BitmapScreenNail. Move over the information about width,
        // height, and Bitmap, then recycle the other.
        BitmapScreenNail newer = (BitmapScreenNail) other;
        mWidth = newer.mWidth;
        mHeight = newer.mHeight;
        if (newer.mBitmap != null) {
            mBitmap = newer.mBitmap;
            newer.mBitmap = null;

            if (mTexture != null) {
                mTexture.recycle();
                mTexture = null;
            }
        }

        newer.recycle();
        return this;
    }

    public void updatePlaceholderSize(int width, int height) {
        if (mBitmap != null) {
            return;
        }
        if (width == 0 || height == 0) {
            return;
        }
        setSize(width, height);
    }

    private static long now() {
        return AnimationTime.get();
    }

    public boolean isAnimating() {
        if (mAnimationStartTime < 0) {
            return false;
        }
        if (now() - mAnimationStartTime >= DURATION) {
            mAnimationStartTime = ANIMATION_DONE;
            return false;
        }
        return true;
    }

    private float getRatio() {
        float r = (float) (now() - mAnimationStartTime) / DURATION;
        return Utils.clamp(1.0f - r, 0.0f, 1.0f);
    }

    /// M: [FEATURE.ADD] plugin @{
    private MediaItem mMediaItem;

    public BitmapScreenNail(Bitmap bitmap, MediaItem item) {
        mWidth = bitmap.getWidth();
        mHeight = bitmap.getHeight();
        mBitmap = bitmap;
        mMediaItem = item;
    }

    public BitmapScreenNail(int width, int height, MediaItem item) {
        setSize(width, height);
        mMediaItem = item;
    }

    @Override
    public MediaItem getMediaItem() {
        return mMediaItem;
    }
    /// @}

    /// M: [DEBUG.ADD] @{
    // debug code especially for high quality screenail in PhotoPage
    private boolean mIsDebugEnable = false;
    public void setDebugEnable(boolean isEnable) {
        mIsDebugEnable = isEnable;
    }
    /// @}
}
