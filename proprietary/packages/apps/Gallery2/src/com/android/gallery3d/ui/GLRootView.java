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

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Process;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;

import com.android.gallery3d.R;
import com.android.gallery3d.anim.CanvasAnimation;
import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.glrenderer.BasicTexture;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.android.gallery3d.glrenderer.GLES11Canvas;
import com.android.gallery3d.glrenderer.GLES20Canvas;
import com.android.gallery3d.glrenderer.StringTexture;
import com.android.gallery3d.glrenderer.UploadedTexture;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.MotionEventHelper;
import com.android.gallery3d.util.Profile;
import com.mediatek.gallery3d.util.FeatureConfig;
import com.mediatek.gallerybasic.gl.GLIdleExecuter;
import com.mediatek.gallerybasic.gl.MBasicTexture;
import com.mediatek.gallerybasic.util.DebugUtils;
import com.mediatek.galleryportable.SystemPropertyUtils;
import com.mediatek.galleryportable.TraceHelper;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;

// The root component of all <code>GLView</code>s. The rendering is done in GL
// thread while the event handling is done in the main thread.  To synchronize
// the two threads, the entry points of this package need to synchronize on the
// <code>GLRootView</code> instance unless it can be proved that the rendering
// thread won't access the same thing as the method. The entry points include:
// (1) The public methods of HeadUpDisplay
// (2) The public methods of CameraHeadUpDisplay
// (3) The overridden methods in GLRootView.
public class GLRootView extends GLSurfaceView
        implements GLSurfaceView.Renderer, GLRoot {
    private static final String TAG = "Gallery2/GLRootView";

    private static final boolean DEBUG_FPS = false;
    private int mFrameCount = 0;
    private long mFrameCountingStart = 0;

    private static final boolean DEBUG_INVALIDATE = false;
    private int mInvalidateColor = 0;

    private static final boolean DEBUG_DRAWING_STAT = false;

    private static final boolean DEBUG_PROFILE = false;
    private static final boolean DEBUG_PROFILE_SLOW_ONLY = false;

    private static final int FLAG_INITIALIZED = 1;
    private static final int FLAG_NEED_LAYOUT = 2;

    private GL11 mGL;
    private GLCanvas mCanvas;
    private GLView mContentView;

    private OrientationSource mOrientationSource;
    // mCompensation is the difference between the UI orientation on GLCanvas
    // and the framework orientation. See OrientationManager for details.
    private int mCompensation;
    // mCompensationMatrix maps the coordinates of touch events. It is kept sync
    // with mCompensation.
    private Matrix mCompensationMatrix = new Matrix();
    private int mDisplayRotation;

    private int mFlags = FLAG_NEED_LAYOUT;
    private volatile boolean mRenderRequested = false;

    private final ArrayList<CanvasAnimation> mAnimations =
            new ArrayList<CanvasAnimation>();

    private final ArrayDeque<OnGLIdleListener> mIdleListeners =
            new ArrayDeque<OnGLIdleListener>();

    private final IdleRunner mIdleRunner = new IdleRunner();

    private final ReentrantLock mRenderLock = new ReentrantLock();
    private final Condition mFreezeCondition =
            mRenderLock.newCondition();
    private boolean mFreeze;

    private long mLastDrawFinishTime;
    private boolean mInDownState = false;
    private boolean mFirstDraw = true;

    /// M: [PERF.ADD] add for performance test case 2.@{
    private static boolean FPS_PERFORMANCE = false;
    /// @}

    /// M: [BUG.ADD] record width and height change @{
    private int mWidth;
    private int mHeight;
    /// @}

    public GLRootView(Context context) {
        this(context, null);
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>GLRootView-new1-done");
        TraceHelper.endSection();
        /// @}
    }

    public GLRootView(Context context, AttributeSet attrs) {
        super(context, attrs);
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>GLRootView-new2-super-done");
        TraceHelper.endSection();
        TraceHelper.beginSection(">>>>GLRootView-new2-other");
        /// @}
        mFlags |= FLAG_INITIALIZED;
        setBackgroundDrawable(null);
        /// M: [BUG.MODIFY] @{
        /*setEGLContextClientVersion(ApiHelper.HAS_GLES20_REQUIRED ? 2 : 1);
        */
        //this is for emulator which OpenGL version should be selected.
        //ro.kernel.qemu.gles = true, means Use host GPU enable.
        //supportSW3D = true, means SW3D support OpenGL 2.0.
        if (FeatureConfig.SUPPORT_EMULATOR) {
            EGL10 egl = (EGL10) EGLContext.getEGL();
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>GLRootView-new2-eglGetDisplay");
            /// @}
            EGLDisplay eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>GLRootView-new2-eglInitialize");
            /// @}
            egl.eglInitialize(eglDisplay, null);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
            if (eglDisplay == EGL10.EGL_NO_DISPLAY) {
                throw new RuntimeException("eglGetDisplay failed");
            }
            String version = egl.eglQueryString(eglDisplay,  egl.EGL_VERSION);
            mSupportSW3D = version.startsWith("2");
            Log.d(TAG, "ro.kernel.qemu.gles = " + "1"
                    .equals(SystemPropertyUtils.get("ro.kernel.qemu.gles"))
                        + ", supportSW3D = " + mSupportSW3D);
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>GLRootView-new2-setEGLContextClientVersion");
            /// @}
            setEGLContextClientVersion((ApiHelper.HAS_GLES20_REQUIRED && (mSupportSW3D || "1"
                    .equals(SystemPropertyUtils.get("ro.kernel.qemu.gles")))) ? 2 : 1);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
        } else {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>GLRootView-new2-setEGLContextClientVersion");
            /// @}
            setEGLContextClientVersion(ApiHelper.HAS_GLES20_REQUIRED ? 2 : 1);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
        }
        /// @}
        if (ApiHelper.USE_888_PIXEL_FORMAT) {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>GLRootView-new2-setEGLConfigChooser");
            /// @}
            setEGLConfigChooser(8, 8, 8, 0, 0, 0);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
        } else {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>GLRootView-new2-setEGLConfigChooser");
            /// @}
            setEGLConfigChooser(5, 6, 5, 0, 0, 0);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
        }
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>GLRootView-new2-setRenderer");
        /// @}
        setRenderer(this);
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
        if (ApiHelper.USE_888_PIXEL_FORMAT) {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>GLRootView-new2-setFormat");
            /// @}
            getHolder().setFormat(PixelFormat.RGB_888);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
        } else {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>GLRootView-new2-setFormat");
            /// @}
            getHolder().setFormat(PixelFormat.RGB_565);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
        }
        // Uncomment this to enable gl error check.
        // setDebugFlags(DEBUG_CHECK_GL_ERROR);
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
    }

    @Override
    public void registerLaunchedAnimation(CanvasAnimation animation) {
        // Register the newly launched animation so that we can set the start
        // time more precisely. (Usually, it takes much longer for first
        // rendering, so we set the animation start time as the time we
        // complete rendering)
        mAnimations.add(animation);
    }

    @Override
    public void addOnGLIdleListener(OnGLIdleListener listener) {
        synchronized (mIdleListeners) {
            mIdleListeners.addLast(listener);
            mIdleRunner.enable();
        }
    }

    @Override
    public void setContentPane(GLView content) {
        if (mContentView == content) return;
        if (mContentView != null) {
            if (mInDownState) {
                long now = SystemClock.uptimeMillis();
                MotionEvent cancelEvent = MotionEvent.obtain(
                        now, now, MotionEvent.ACTION_CANCEL, 0, 0, 0);
                mContentView.dispatchTouchEvent(cancelEvent);
                cancelEvent.recycle();
                mInDownState = false;
            }
            mContentView.detachFromRoot();
            BasicTexture.yieldAllTextures();
            /// M: [FEATURE.ADD] @{
            MBasicTexture.yieldAllTextures();
            /// @}
        }
        mContentView = content;
        if (content != null) {
            content.attachToRoot(this);
            requestLayoutContentPane();
        }
    }

    @Override
    public void requestRenderForced() {
        superRequestRender();
    }

    @Override
    public void requestRender() {
        if (DEBUG_INVALIDATE) {
            StackTraceElement e = Thread.currentThread().getStackTrace()[4];
            String caller = e.getFileName() + ":" + e.getLineNumber() + " ";
            Log.d(TAG, "invalidate: " + caller);
        }
        if (mRenderRequested) return;
        mRenderRequested = true;
        if (ApiHelper.HAS_POST_ON_ANIMATION) {
            postOnAnimation(mRequestRenderOnAnimationFrame);
        } else {
            super.requestRender();
        }
    }

    private Runnable mRequestRenderOnAnimationFrame = new Runnable() {
        @Override
        public void run() {
            superRequestRender();
        }
    };

    private void superRequestRender() {
        super.requestRender();
    }

    @Override
    public void requestLayoutContentPane() {
        mRenderLock.lock();
        try {
            if (mContentView == null || (mFlags & FLAG_NEED_LAYOUT) != 0) return;

            // "View" system will invoke onLayout() for initialization(bug ?), we
            // have to ignore it since the GLThread is not ready yet.
            if ((mFlags & FLAG_INITIALIZED) == 0) return;

            mFlags |= FLAG_NEED_LAYOUT;
            requestRender();
        } finally {
            mRenderLock.unlock();
        }
    }

    private void layoutContentPane() {
        mFlags &= ~FLAG_NEED_LAYOUT;

        int w = getWidth();
        int h = getHeight();
        int displayRotation = 0;
        int compensation = 0;

        // Get the new orientation values
        if (mOrientationSource != null) {
            displayRotation = mOrientationSource.getDisplayRotation();
            compensation = mOrientationSource.getCompensation();
        } else {
            displayRotation = 0;
            compensation = 0;
        }

        /// M: [BUG.MODIFY] @{
        /*if (mCompensation != compensation) {*/
        if (mCompensation != compensation || mWidth != w || mHeight != h) {
            mWidth = w;
            mHeight = h;
        /// @}
            mCompensation = compensation;
            if (mCompensation % 180 != 0) {
                mCompensationMatrix.setRotate(mCompensation);
                // move center to origin before rotation
                mCompensationMatrix.preTranslate(-w / 2, -h / 2);
                // align with the new origin after rotation
                mCompensationMatrix.postTranslate(h / 2, w / 2);
            } else {
                mCompensationMatrix.setRotate(mCompensation, w / 2, h / 2);
            }
        }
        mDisplayRotation = displayRotation;

        // Do the actual layout.
        if (mCompensation % 180 != 0) {
            int tmp = w;
            w = h;
            h = tmp;
        }
        Log.i(TAG, "layout content pane " + w + "x" + h
                + " (compensation " + mCompensation + ")");
        if (mContentView != null && w != 0 && h != 0) {
            mContentView.layout(0, 0, w, h);
        }
        // Uncomment this to dump the view hierarchy.
        //mContentView.dumpTree("");
    }

    @Override
    protected void onLayout(
            boolean changed, int left, int top, int right, int bottom) {
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>GLRootView-onLayout");
        /// @}
        if (changed) requestLayoutContentPane();
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
    }

    /**
     * Called when the context is created, possibly after automatic destruction.
     */
    // This is a GLSurfaceView.Renderer callback
    @Override
    public void onSurfaceCreated(GL10 gl1, EGLConfig config) {
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>GLRootView-onSurfaceCreated");
        Log.d(TAG, "<onSurfaceCreated> gl1 = " + gl1);
        /// @}
        GL11 gl = (GL11) gl1;
        if (mGL != null) {
            // The GL Object has changed
            Log.i(TAG, "GLObject has changed from " + mGL + " to " + gl);
        }
        mRenderLock.lock();
        try {
            mGL = gl;
            /// M: [BUG.MODIFY] this is for emulator which OpenGL version should be selected.
            // ro.kernel.qemu.gles = true, means Use host GPU enable.
            // supportSW3D = true, means SW3D support OpenGL 2.0.@{
            /*mCanvas = ApiHelper.HAS_GLES20_REQUIRED ? new GLES20Canvas() : new GLES11Canvas(gl);*/
            if (FeatureConfig.SUPPORT_EMULATOR) {
                mCanvas = (ApiHelper.HAS_GLES20_REQUIRED && (mSupportSW3D || "1"
                        .equals(SystemPropertyUtils.get("ro.kernel.qemu.gles")))) ?
                                new GLES20Canvas() : new GLES11Canvas(gl);
            } else {
                mCanvas = ApiHelper.HAS_GLES20_REQUIRED ? new GLES20Canvas()
                        : new GLES11Canvas(gl);
            }
            /// @}
            BasicTexture.invalidateAllTextures();
            /// M: [FEATURE.ADD] @{
            MBasicTexture.invalidateAllTextures();
            /// @}
        } finally {
            mRenderLock.unlock();
        }

        if (DEBUG_FPS || DEBUG_PROFILE) {
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        } else {
            setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }

        /// M: [FEATURE.ADD] @{
        if (mGLIdleExecuter != null) {
            mGLIdleExecuter.setCanvas(mCanvas.getMGLCanvas());
        }
        /// @}
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
    }

    /**
     * Called when the OpenGL surface is recreated without destroying the
     * context.
     */
    // This is a GLSurfaceView.Renderer callback
    @Override
    public void onSurfaceChanged(GL10 gl1, int width, int height) {
        Log.i(TAG, "onSurfaceChanged: " + width + "x" + height
                + ", gl10: " + gl1.toString());
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>GLRootView-onSurfaceChanged");
        /// @}
        Process.setThreadPriority(Process.THREAD_PRIORITY_DISPLAY);
        GalleryUtils.setRenderThread();
        if (DEBUG_PROFILE) {
            Log.d(TAG, "Start profiling");
            Profile.enable(20);  // take a sample every 20ms
        }
        GL11 gl = (GL11) gl1;
        Utils.assertTrue(mGL == gl);

        mCanvas.setSize(width, height);
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
    }

    private void outputFps() {
        long now = System.nanoTime();
        if (mFrameCountingStart == 0) {
            mFrameCountingStart = now;
        } else if ((now - mFrameCountingStart) > 1000000000) {
            Log.d(TAG, "fps: " + (double) mFrameCount
                    * 1000000000 / (now - mFrameCountingStart));
            mFrameCountingStart = now;
            mFrameCount = 0;
        }
        /// M: [TESTCASE.ADD] @{
        else if (FPS_PERFORMANCE && mFrameCount != 0) {
            Log.d("Gallery2PerformanceTestCase2", "[Performance Auto Test] Gallery render fps = "
                    + (int) mFrameCount * 1000000000 / (now - mFrameCountingStart));
            mFrameCountingStart = now;
            mFrameCount = 0;
        }
        /// @}
        ++mFrameCount;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        /// M: [BUG.ADD] avoid NE after surfaceDestroy @{
        if (mSurfaceDestroyed) {
            Log.d(TAG, "<onDrawFrame> mSurfaceDestroyed = true, no render, return");
            return;
        }
        /// @}

        AnimationTime.update();
        long t0;
        if (DEBUG_PROFILE_SLOW_ONLY) {
            Profile.hold();
            t0 = System.nanoTime();
        }
        mRenderLock.lock();

        /// M: [BUG.ADD] @{
        /*while (mFreeze) {
            mFreezeCondition.awaitUninterruptibly();
        }

        try {*/
        try {
            while (mFreeze) {
                mFreezeCondition.awaitUninterruptibly();
            }
        /// @}
            onDrawFrameLocked(gl);
        } finally {
            mRenderLock.unlock();
        }

        // We put a black cover View in front of the SurfaceView and hide it
        // after the first draw. This prevents the SurfaceView being transparent
        // before the first draw.
        if (mFirstDraw) {
            mFirstDraw = false;
            post(new Runnable() {
                    @Override
                    public void run() {
                        View root = getRootView();
                        View cover = root.findViewById(R.id.gl_root_cover);
                        cover.setVisibility(GONE);
                    }
                });
        }

        if (DEBUG_PROFILE_SLOW_ONLY) {
            long t = System.nanoTime();
            long durationInMs = (t - mLastDrawFinishTime) / 1000000;
            long durationDrawInMs = (t - t0) / 1000000;
            mLastDrawFinishTime = t;

            if (durationInMs > 34) {  // 34ms -> we skipped at least 2 frames
                Log.v(TAG, "----- SLOW (" + durationDrawInMs + "/" +
                        durationInMs + ") -----");
                Profile.commit();
            } else {
                Profile.drop();
            }
        }
    }

    private void onDrawFrameLocked(GL10 gl) {
        if (DEBUG_FPS) outputFps();

        /// M: [TESTCASE.ADD] @{
        if (FPS_PERFORMANCE) {
            outputFps();
        }
        /// @}

        // release the unbound textures and deleted buffers.
        mCanvas.deleteRecycledResources();

        // reset texture upload limit
        UploadedTexture.resetUploadLimit();

        mRenderRequested = false;

        if ((mOrientationSource != null
                && mDisplayRotation != mOrientationSource.getDisplayRotation())
                || (mFlags & FLAG_NEED_LAYOUT) != 0) {
            layoutContentPane();
        }

        mCanvas.save(GLCanvas.SAVE_FLAG_ALL);
        rotateCanvas(-mCompensation);
        if (mContentView != null) {
           mContentView.render(mCanvas);
        } else {
            // Make sure we always draw something to prevent displaying garbage
            mCanvas.clearBuffer();
        }
        mCanvas.restore();

        if (!mAnimations.isEmpty()) {
            long now = AnimationTime.get();
            for (int i = 0, n = mAnimations.size(); i < n; i++) {
                mAnimations.get(i).setStartTime(now);
            }
            mAnimations.clear();
        }

        if (UploadedTexture.uploadLimitReached()) {
            requestRender();
        }

        synchronized (mIdleListeners) {
            if (!mIdleListeners.isEmpty()) mIdleRunner.enable();
        }

        /// M: [FEATURE.ADD] @{
        if (mGLIdleExecuter != null) {
            mGLIdleExecuter.onRenderComplete();
        }
        /// @}

        /// M: [DEBUG.ADD] @{
        if (DebugUtils.DEBUG_RENDER) {
            renderFrameId();
        }
        /// @}

        if (DEBUG_INVALIDATE) {
            mCanvas.fillRect(10, 10, 5, 5, mInvalidateColor);
            mInvalidateColor = ~mInvalidateColor;
        }

        if (DEBUG_DRAWING_STAT) {
            mCanvas.dumpStatisticsAndClear();
        }
    }

    private void rotateCanvas(int degrees) {
        if (degrees == 0) return;
        int w = getWidth();
        int h = getHeight();
        int cx = w / 2;
        int cy = h / 2;
        mCanvas.translate(cx, cy);
        mCanvas.rotate(degrees, 0, 0, 1);
        if (degrees % 180 != 0) {
            mCanvas.translate(-cy, -cx);
        } else {
            mCanvas.translate(-cx, -cy);
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        if (!isEnabled()) return false;

        int action = event.getAction();
        if (action == MotionEvent.ACTION_CANCEL
                || action == MotionEvent.ACTION_UP) {
            mInDownState = false;
        } else if (!mInDownState && action != MotionEvent.ACTION_DOWN) {
            return false;
        }

        if (mCompensation != 0) {
            event = MotionEventHelper.transformEvent(event, mCompensationMatrix);
        }

        mRenderLock.lock();
        try {
            // If this has been detached from root, we don't need to handle event
            boolean handled = mContentView != null
                    && mContentView.dispatchTouchEvent(event);
            if (action == MotionEvent.ACTION_DOWN && handled) {
                mInDownState = true;
            }
            return handled;
        } finally {
            mRenderLock.unlock();
        }
    }

    private class IdleRunner implements Runnable {
        // true if the idle runner is in the queue
        private boolean mActive = false;

        @Override
        public void run() {
            /// M: [BUG.ADD] fix album label no displaying bug @{
            if (mSurfaceDestroyed || mGLSurfaceViewPaused) {
                Log.d(TAG, "<IdleRunner.run> mSurfaceDestroyed " + mSurfaceDestroyed
                        + ", mGLSurfaceViewPaused " + mGLSurfaceViewPaused + ", return");
                mActive = false;
                return;
            }
            /// @}
            OnGLIdleListener listener;
            synchronized (mIdleListeners) {
                mActive = false;
                if (mIdleListeners.isEmpty()) return;
                listener = mIdleListeners.removeFirst();
            }
            mRenderLock.lock();
            boolean keepInQueue;
            try {
                keepInQueue = listener.onGLIdle(mCanvas, mRenderRequested);
            } finally {
                mRenderLock.unlock();
            }
            synchronized (mIdleListeners) {
                if (keepInQueue) mIdleListeners.addLast(listener);
                if (!mRenderRequested && !mIdleListeners.isEmpty()) enable();
            }
        }

        public void enable() {
            // Who gets the flag can add it to the queue
            if (mActive) return;
            mActive = true;
            queueEvent(this);
        }
    }

    @Override
    public void lockRenderThread() {
        mRenderLock.lock();
    }

    @Override
    public void unlockRenderThread() {
        mRenderLock.unlock();
    }

    @Override
    public void onPause() {
        unfreeze();
        /// M: [BUG.ADD] @{
        // log the GLSurfaceView status to avoid some error cases
        mGLSurfaceViewPaused = true;
        Log.d(TAG, "<onPause> set mGLSurfaceViewPaused as true");
        /// @}
        super.onPause();
        if (DEBUG_PROFILE) {
            Log.d(TAG, "Stop profiling");
            Profile.disableAll();
            Profile.dumpToFile("/sdcard/gallery.prof");
            Profile.reset();
        }
    }

    @Override
    public void setOrientationSource(OrientationSource source) {
        mOrientationSource = source;
    }

    @Override
    public int getDisplayRotation() {
        return mDisplayRotation;
    }

    @Override
    public int getCompensation() {
        return mCompensation;
    }

    @Override
    public Matrix getCompensationMatrix() {
        return mCompensationMatrix;
    }

    @Override
    public void freeze() {
        mRenderLock.lock();
        mFreeze = true;
        mRenderLock.unlock();
    }

    @Override
    public void unfreeze() {
        mRenderLock.lock();
        /// M: [BUG.MODIFY] @{
        /*mFreeze = false;
        mFreezeCondition.signalAll();
        mRenderLock.unlock();*/
        try {
            mFreeze = false;
            mFreezeCondition.signalAll();
        } finally {
            mRenderLock.unlock();
        }
        /// @}
    }

    @Override
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void setLightsOutMode(boolean enabled) {
        if (!ApiHelper.HAS_SET_SYSTEM_UI_VISIBILITY) return;

        int flags = 0;
        if (enabled) {
            flags = STATUS_BAR_HIDDEN;
            if (ApiHelper.HAS_VIEW_SYSTEM_UI_FLAG_LAYOUT_STABLE) {
                flags |= (SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_LAYOUT_STABLE);
            }
        }
        setSystemUiVisibility(flags);
    }

    // We need to unfreeze in the following methods and in onPause().
    // These methods will wait on GLThread. If we have freezed the GLRootView,
    // the GLThread will wait on main thread to call unfreeze and cause dead
    // lock.
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<surfaceChanged> format = " + format + ", w = " + w + ", h = " + h);
        /// @}
        unfreeze();
        super.surfaceChanged(holder, format, w, h);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        /// M: [BUG.ADD] avoid NE after surfaceDestroy @{
        Log.d(TAG, "<surfaceCreated> set mSurfaceDestroyed = false");
        mSurfaceDestroyed = false;
        /// @}
        unfreeze();
        super.surfaceCreated(holder);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        /// M: [BUG.ADD] avoid NE after surfaceDestroy @{
        Log.d(TAG, "<surfaceDestroyed> set mSurfaceDestroyed = true");
        mSurfaceDestroyed = true;
        /// @}
        unfreeze();
        super.surfaceDestroyed(holder);
    }

    @Override
    protected void onDetachedFromWindow() {
        unfreeze();
        super.onDetachedFromWindow();
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            unfreeze();
        } finally {
            super.finalize();
        }
    }

//********************************************************************
//*                              MTK                                 *
//********************************************************************

    private GLIdleExecuter mGLIdleExecuter = null;
    private boolean mSupportSW3D;
    // Avoid NE after surfaceDestroy
    private boolean mSurfaceDestroyed = true;
    // Avoid RuntimeException of GL after EGLContext has been destroyed
    private boolean mGLSurfaceViewPaused = false;

    @Override
    public void onResume() {
        super.onResume();
        mGLSurfaceViewPaused = false;
        Log.d(TAG, "<onResume> set mGLSurfaceViewPaused as false");
    }

    public GLIdleExecuter getGLIdleExecuter() {
        if (mGLIdleExecuter == null) {
            mGLIdleExecuter = new GLIdleExecuter(this, new MyGLIdleExecuter());
        }
        if (mCanvas != null) {
            mGLIdleExecuter.setCanvas(mCanvas.getMGLCanvas());
        }
        return mGLIdleExecuter;
    }

    private class MyGLIdleExecuter implements GLIdleExecuter.IGLSurfaceViewStatusGetter {
        public boolean isRenderRequested() {
            return mRenderRequested;
        }

        public boolean isSurfaceDestroyed() {
            return mSurfaceDestroyed;
        }
    }

    public void dispatchKeyEventView(KeyEvent event) {
        mRenderLock.lock();
        try {
            if (mContentView != null) {
                mContentView.dispatchKeyEvent(event);
            }
        } finally {
            mRenderLock.unlock();
        }
    }

    /// M: [PERF.ADD] add for performance test case 2.@{
    public void setFpsPerformance(boolean open) {
        Log.d(TAG, "[Performance Auto Test] setFpsPerformance = " + open);
        FPS_PERFORMANCE = open;
        if (!open) {
            mFrameCountingStart = 0;
            mFrameCount = 0;
        }
    }
    /// @}

    /// M: [DEBUG.ADD] @{
    private int mCurrentFrameId = 0;
    private StringTexture mCurrentFrameIdTexture;
    private static final int FRAME_ID_TEXT_SIZE = 100;
    private static final int FRAME_ID_POSITION_X = 0;
    private static final int FRAME_ID_POSITION_Y = 300;

    private void renderFrameId() {
        if (mCurrentFrameIdTexture != null) {
            mCurrentFrameIdTexture.recycle();
        }
        mCurrentFrameIdTexture =
                StringTexture.newInstance(String.valueOf(mCurrentFrameId), FRAME_ID_TEXT_SIZE,
                        Color.RED);
        mCanvas.drawTexture(mCurrentFrameIdTexture, FRAME_ID_POSITION_X, FRAME_ID_POSITION_Y,
                mCurrentFrameIdTexture.getWidth(), mCurrentFrameIdTexture.getHeight());
        mCurrentFrameId++;
    }

    public class DebugThread extends Thread {
        int mTime = 0;

        public DebugThread(String name) {
            super(name);
        }

        @Override
        public void run() {
            Log.d(TAG, "<DebugThread> start");
            try {
                while (true) {
                    mTime++;
                    Log.d(TAG, "<DebugThread> time = " + mTime + ", begin #######################");
                    Log.d(TAG, "<DebugThread> mRenderLock = " + mRenderLock);
                    Log.d(TAG, "<DebugThread> mRenderLock.getHoldCount() = " +
                    mRenderLock.getHoldCount());
                    Log.d(TAG, "<DebugThread> mRenderLock.getQueueLength() = " +
                    mRenderLock.getQueueLength());
                    Log.d(TAG, "<DebugThread> mRenderLock.hasQueuedThreads() = " +
                    mRenderLock.hasQueuedThreads());
                    Log.d(TAG, "<DebugThread> mRenderLock.isFair() = " + mRenderLock.isFair());
                    Log.d(TAG, "<DebugThread> mRenderLock.isHeldByCurrentThread() = " +
                    mRenderLock.isHeldByCurrentThread());
                    Log.d(TAG, "<DebugThread> mRenderLock.isLocked() = " + mRenderLock.isLocked());
                    Log.d(TAG, "<DebugThread> time = " + mTime + ", end #########################");
                    Thread.sleep(1000);
                }
            } catch (InterruptedException e) {
                Log.d(TAG, "<DebugThread> Interrupted, " + e.getMessage());
            }
            Log.d(TAG, "<DebugThread> stop");
        }
    }

    private DebugThread mDebugThread;
    public void startDebug() {
        mDebugThread = new DebugThread("DebugThread");
        mDebugThread.start();
        Log.d(TAG, "<startDebug> new DebugThread");
    }

    public void stopDebug() {
        if (mDebugThread != null) {
            mDebugThread.interrupt();
            mDebugThread = null;
        }
        Log.d(TAG, "<stopDebug> interrupt DebugThread");
    }
    /// @}
}
