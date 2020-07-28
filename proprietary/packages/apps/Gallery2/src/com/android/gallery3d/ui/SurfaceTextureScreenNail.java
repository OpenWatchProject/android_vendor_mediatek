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

import android.annotation.TargetApi;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.os.HandlerThread;
import android.os.Process;

import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.glrenderer.ExtTexture;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.mediatek.gallery3d.util.FeatureConfig;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.SystemPropertyUtils;

@TargetApi(ApiHelper.VERSION_CODES.HONEYCOMB)
public abstract class SurfaceTextureScreenNail implements ScreenNail,
        SurfaceTexture.OnFrameAvailableListener {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/SurfaceTextureScreenNail";
    // This constant is not available in API level before 15, but it was just an
    // oversight.
    private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;

    protected ExtTexture mExtTexture;
    private SurfaceTexture mSurfaceTexture;
    private int mWidth, mHeight;
    private float[] mTransform = new float[16];
    private boolean mHasTexture = false;

    /// M: [FEATURE.ADD] debug info for draw preview. @{
    protected static final int INTERVALS = 60;
    protected int mDebugFlag = SystemPropertyUtils.getInt("cam.debug", 0);
    protected boolean mDebug = false;
    protected boolean mDebugLevel2 = false;
    protected int mDrawFrameCount = 0;
    protected int mRequestCount = 0;
    protected long mRequestStartTime = 0;
    protected long mDrawStartTime = 0;
    private static int sMaxHightProrityFrameCount = 8;
    private int mCurrentFrameCount = 0;
    private static HandlerThread sFrameListener = new HandlerThread("FrameListener");
    /// @}

    public SurfaceTextureScreenNail() {
        /// M: [FEATURE.ADD] @{
        mDebug = mDebugFlag > 0;
        mDebugLevel2 = mDebugFlag > 1;
        /// @}
    }

    public void acquireSurfaceTexture(GLCanvas canvas) {
        mExtTexture = new ExtTexture(canvas, GL_TEXTURE_EXTERNAL_OES);
        mExtTexture.setSize(mWidth, mHeight);

        /// M: [FEATURE.MODIFY] @{
        /*mSurfaceTexture = new SurfaceTexture(mExtTexture.getId());*/
        if (!sFrameListener.isAlive()) {
            sFrameListener.start();
        }
        Log.e(TAG,
                "<acquireSurfaceTexture> stscrnail construct surfacetexture with id "
                        + mExtTexture.getId());
        mSurfaceTexture = new SurfaceTexture(mExtTexture.getId());
        initializePriority();
        /// @}

        setDefaultBufferSize(mSurfaceTexture, mWidth, mHeight);
        mSurfaceTexture.setOnFrameAvailableListener(this);
        synchronized (this) {
            mHasTexture = true;
        }
    }

    /// M: [FEATURE.ADD] add for camera launch performance @{
    public void acquireSurfaceTexture() {
        if (mExtTexture == null) {
            mExtTexture = new ExtTexture(GL_TEXTURE_EXTERNAL_OES);
        }
        mExtTexture.setSize(mWidth, mHeight);
        if (!sFrameListener.isAlive()) {
            sFrameListener.start();
        }
        mSurfaceTexture = new SurfaceTexture(mExtTexture.getId());
        initializePriority();
        setDefaultBufferSize(mSurfaceTexture, mWidth, mHeight);
        mSurfaceTexture.setOnFrameAvailableListener(this);
        synchronized (this) {
            mHasTexture = true;
        }
    }
    /// @}

    @TargetApi(ApiHelper.VERSION_CODES.ICE_CREAM_SANDWICH_MR1)
    private static void setDefaultBufferSize(SurfaceTexture st, int width, int height) {
        if (ApiHelper.HAS_SET_DEFALT_BUFFER_SIZE) {
            st.setDefaultBufferSize(width, height);
        }
    }

    @TargetApi(ApiHelper.VERSION_CODES.ICE_CREAM_SANDWICH)
    private static void releaseSurfaceTexture(SurfaceTexture st) {
        st.setOnFrameAvailableListener(null);
        if (ApiHelper.HAS_RELEASE_SURFACE_TEXTURE) {
            st.release();
        }
    }

    public SurfaceTexture getSurfaceTexture() {
        return mSurfaceTexture;
    }

/// M: [FEATURE.MODIFY] @{
/*    public void releaseSurfaceTexture() {
        synchronized (this) {
            mHasTexture = false;
        }
        mExtTexture.recycle();
        mExtTexture = null;
        releaseSurfaceTexture(mSurfaceTexture);
        mSurfaceTexture = null;
    }*/
    public void releaseSurfaceTexture(boolean needReleaseExtTexture) {
        Log.d(TAG,
                "<releaseSurfaceTexture> releaseSurfaceTexture needReleaseExtTexture = "
                        + needReleaseExtTexture);
        if (needReleaseExtTexture) {
            synchronized (this) {
                mHasTexture = false;
            }
            mExtTexture.recycle();
            mExtTexture = null;
        }
        releaseSurfaceTexture(mSurfaceTexture);
        mSurfaceTexture = null;
    }
/// @}


    /// M: [FEATURE.ADD] @{
    public void fullHandlerCapacity() {
        if (!FeatureConfig.SUPPORT_EMULATOR) {
            Log.d(TAG, "<fullHandlerCapacity> set urgent display");
            Process.setThreadPriority(sFrameListener.getThreadId(),
                    Process.THREAD_PRIORITY_URGENT_DISPLAY);
        }
    }

    public void normalHandlerCapacity() {
        if (!FeatureConfig.SUPPORT_EMULATOR) {
            Log.d(TAG, "<normalHandlerCapacity> set normal");
            Process.setThreadPriority(sFrameListener.getThreadId(),
                    Process.THREAD_PRIORITY_DEFAULT);
        }
    }

    private void initializePriority() {
        fullHandlerCapacity();
        mCurrentFrameCount = 0;
    }

    private void checkThreadPriority() {
        if (mCurrentFrameCount == sMaxHightProrityFrameCount) {
            normalHandlerCapacity();
            mCurrentFrameCount++;
        } else if (mCurrentFrameCount < sMaxHightProrityFrameCount) {
            mCurrentFrameCount++;
        }
    }
    /// @}

    public void setSize(int width, int height) {
        mWidth = width;
        mHeight = height;
    }

    public void resizeTexture() {
        if (mExtTexture != null) {
            mExtTexture.setSize(mWidth, mHeight);
            setDefaultBufferSize(mSurfaceTexture, mWidth, mHeight);
        }
    }

    @Override
    public int getWidth() {
        return mWidth;
    }

    @Override
    public int getHeight() {
        return mHeight;
    }

    @Override
    public void draw(GLCanvas canvas, int x, int y, int width, int height) {
        synchronized (this) {
            if (!mHasTexture) return;
            /// M: [FEATURE.ADD] @{
            checkThreadPriority();
            /// @}
            mSurfaceTexture.updateTexImage();
            mSurfaceTexture.getTransformMatrix(mTransform);

            // Flip vertically.
            canvas.save(GLCanvas.SAVE_FLAG_MATRIX);
            int cx = x + width / 2;
            int cy = y + height / 2;
            canvas.translate(cx, cy);
            canvas.scale(1, -1, 1);
            canvas.translate(-cx, -cy);
            updateTransformMatrix(mTransform);
            canvas.drawTexture(mExtTexture, mTransform, x, y, width, height);
            canvas.restore();

            /// M: [FEATURE.ADD] @{
            if (mDebug) {
                if (mDebugLevel2) {
                    Log.d(TAG, "<draw> GLCanvas drawing Frame");
                }
                mDrawFrameCount++;
                if (mDrawFrameCount % INTERVALS == 0) {
                    long currentTime = System.currentTimeMillis();
                    int intervals = (int) (currentTime - mDrawStartTime);
                    Log.d(TAG, "<draw> Drawing frame, fps = "
                            + (mDrawFrameCount * 1000.0f) / intervals
                            + " in last " + intervals + " millisecond.");
                    mDrawStartTime = currentTime;
                    mDrawFrameCount = 0;
                }
            }
            /// @}
        }
    }

    @Override
    public void draw(GLCanvas canvas, RectF source, RectF dest) {
        /// M: [BUG.MODIFY] @{
        //throw new UnsupportedOperationException();
        Log.d(TAG, "<draw> draw fails!!!");
        return;
        /// @}
    }

    protected void updateTransformMatrix(float[] matrix) {}

    @Override
    abstract public void noDraw();

    @Override
    abstract public void recycle();

    @Override
    abstract public void onFrameAvailable(SurfaceTexture surfaceTexture);

    /// M: [FEATURE.ADD] plugin @{
    @Override
    public MediaItem getMediaItem() {
        return null;
    }
    /// @}

    /// M: [BUG.ADD] Update animation for this texture. @{
    public boolean isAnimating() {
        return false;
    }
    /// @}
}
