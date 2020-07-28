/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.gles.egl;

import android.annotation.TargetApi;
import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLExt;
import android.opengl.EGLSurface;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

/**
 * This class maintain core EGL states: EGLDisplay, EGLContext, EGLConfig.
 * The EGLContext must only be attached to one thread at a time.
 * Note: this class is not thread-safe.
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
public final class EglCore {
    private static final Tag TAG = new Tag(EglCore.class.getSimpleName());
    /**
     * This flag used in constructor to indicate that surface must be recordable.
     * This discourages EGL from using a pixel format that cannot be converted
     * efficiently to something usable by the video encoder.
     */
    public static final int CONSTRUCTOR_FLAG_RECORDABLE = 0x01;
    /**
     * This flag used in constructor to indicate that ask for OpenGL ES 3,
     * fall back to OpenGL ES 2 if not available, without this flag, OpenGL ES 2.0
     * is used.
     */
    public static final int CONSTRUCTOR_FLAG_TRY_GLES3 = 0x02;
    private EGLDisplay mEGLDisplay = EGL14.EGL_NO_DISPLAY;
    private EGLContext mEGLContext = EGL14.EGL_NO_CONTEXT;
    private EGLConfig mEGLConfig = null;
    private int mGlVersion = -1;
    private int mOutputPixelFormat = -1;

    /**
     * Prepares EGLDisplay, EGLContext and EGLConfig.
     *
     * Equivalent to EglCore(null, 0);
     */
    public EglCore() {
        init(null, 0, null);
    }

    /**
     * Prepares EGLDisplay, EGLContext and EGLConfig.
     *
     * @param sharedContext The context to share, or null if sharing is not desired.
     * @param flags Configuration bit flags, e.g. CONSTRUCTOR_FLAG_RECORDABLE.
     */
    public EglCore(EGLContext sharedContext, int flags) {
        init(sharedContext, flags, null);
    }

    /**
     * Prepares EGLDisplay, EGLContext and EGLConfig.
     *
     * @param sharedContext The context to share, or null if sharing is not desired.
     * @param flags Configuration bit flags, e.g. CONSTRUCTOR_FLAG_RECORDABLE.
     * @param supportedOutputFormats wanted gpu output formats list.
     */
    public EglCore(EGLContext sharedContext, int flags, int[] supportedOutputFormats) {
        init(sharedContext, flags, supportedOutputFormats);
    }

    /**
     * Get the GLES output pixel format.
     *
     * @return Returns the GLES output pixel format,
     *         Ex.ImageFormat.YV12 or PixelFormat.RGBA_8888 etc.
     */
    public int getOutPutPixelFormat() {
        return mOutputPixelFormat;
    }

    /**
     * Discard all resources held by this class, notably the EGL context.
     * This must be called from the thread where the context was created.
     *
     * On completion, no context will be current.
     */
    public void release() {
        if (mEGLDisplay != EGL14.EGL_NO_DISPLAY) {
            // Android is unusual in that it uses a reference-counted
            // EGLDisplay. So for every eglInitialize() we need an eglTerminate().
            EGL14.eglMakeCurrent(
                    mEGLDisplay,
                    EGL14.EGL_NO_SURFACE,
                    EGL14.EGL_NO_SURFACE,
                    EGL14.EGL_NO_CONTEXT);
            EGL14.eglDestroyContext(mEGLDisplay, mEGLContext);
            EGL14.eglReleaseThread();
            EGL14.eglTerminate(mEGLDisplay);
        }

        mEGLDisplay = EGL14.EGL_NO_DISPLAY;
        mEGLContext = EGL14.EGL_NO_CONTEXT;
        mEGLConfig = null;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            if (mEGLDisplay != EGL14.EGL_NO_DISPLAY) {
                // we're limited here -- finalizers don't run on the thread that holds
                // the EGL state, so if a surface or context is still current on another
                // thread we can't fully release it here.
                // Exceptions thrown from here are quietly discarded, complain in the log.
                LogHelper.w(TAG, "EglCore was not explicitly released -- state my be leaked!!!");
                release();
            }
        } finally {
            super.finalize();
        }
    }

    /**
     * Destroys the specified surface.
     * Note the surface won't actually be destroyed if it's still current in a context.
     *
     * @param eglSurface the EGLSurface to be released.
     */
    public void releaseEglSurface(EGLSurface eglSurface) {
        if (eglSurface != null) {
            EGL14.eglDestroySurface(mEGLDisplay, eglSurface);
        }
    }

    /**
     * Creates an EGL surface associated with a Surface.
     * <p>
     * If this is destined for MediaCodec, the EGLConfig should have the
     * "recordable" attribute.
     *
     * @param surface the surface want to create EGLSurface.
     * @return EGLSurface an created EGLSurface.
     */
    public EGLSurface createWindowSurface(Object surface) {
        if (!(surface instanceof Surface) && !(surface instanceof SurfaceTexture)) {
            throw new RuntimeException("invalid surface: " + surface);
        }

        // Create a window surface, and attach it to the Surface we received.
        int[] surfaceAttribs = {
                EGL14.EGL_NONE
        };
        EGLSurface eglSurface = EGL14.eglCreateWindowSurface(
                mEGLDisplay,
                mEGLConfig,
                surface,
                surfaceAttribs,
                0);
        EglUtil.checkEglError("[createWindowSurface] eglCreateWindowSurface");
        if (eglSurface == null) {
            throw new RuntimeException("[createWindowSurface] surface was null");
        }
        return eglSurface;
    }

    /**
     * Creates an EGL surface associated with an offscreen buffer.
     *
     * @param width the off screen surface's width.
     * @param height the off screen surface's height.
     * @return an created off screen EGLSurface.
     */
    public EGLSurface createOffscreenSurface(int width, int height) {
        int[] surfaceAttributes = {
                EGL14.EGL_WIDTH, width,
                EGL14.EGL_HEIGHT, height,
                EGL14.EGL_NONE
        };
        EGLSurface eglSurface = EGL14.eglCreatePbufferSurface(
                mEGLDisplay,
                mEGLConfig,
                surfaceAttributes,
                0);
        EglUtil.checkEglError("[createOffscreenSurface] eglCreatePbufferSurface");
        if (eglSurface == null) {
            throw new RuntimeException("[createOffscreenSurface] surface was null");
        }
        return eglSurface;
    }

    /**
     * Makes our EGL context current, using the supplied surface for both "draw" and "read".
     *
     * @param eglSurface the EGLSurface to be make current.
     */
    public void makeCurrent(EGLSurface eglSurface) {
        if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
            // called makeCurrent() before create?
            LogHelper.w(TAG, "[makeCurrent] NOTE: makeCurrent w/o display");
        }
        if (!EGL14.eglMakeCurrent(
                mEGLDisplay,
                eglSurface,
                eglSurface,
                mEGLContext)) {
            throw new RuntimeException("[makeCurrent] eglMakeCurrent failed");
        }
    }

    /**
     * Makes our EGL context current, using the supplied "draw" and "read" surfaces.
     *
     * @param drawSurface the EGLSurface to be make current for draw.
     * @param readSurface the EGLSurface to be make current for read.
     */
    public void makeCurrent(EGLSurface drawSurface, EGLSurface readSurface) {
        if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
            // called makeCurrent() before create?
            LogHelper.w(TAG, "[makeCurrent] NOTE: makeCurrent w/o display");
        }
        if (!EGL14.eglMakeCurrent(
                mEGLDisplay,
                drawSurface,
                readSurface,
                mEGLContext)) {
            throw new RuntimeException("[makeCurrent] eglMakeCurrent failed");
        }
    }

    /**
     * Makes no context current.
     */
    public void makeNothingCurrent() {
        if (!EGL14.eglMakeCurrent(
                mEGLDisplay,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_CONTEXT)) {
            throw new RuntimeException("eglMakeCurrent failed");
        }
    }

    /**
     * Calls eglSwapBuffers. Use this to "publish" the current frame.
     *
     * @param eglSurface the surface receive current frame.
     * @return false on failure
     */
    public boolean swapBuffers(EGLSurface eglSurface) {
        return EGL14.eglSwapBuffers(mEGLDisplay, eglSurface);
    }

    /**
     * Sends the presentation time stamp to EGL. Time is expressed in nanoseconds.
     *
     * @param eglSurface the EGLSurface to be set presentation time.
     * @param nsecs presentation time.
     */
    public void setPresentationTime(EGLSurface eglSurface, long nsecs) {
        EGLExt.eglPresentationTimeANDROID(mEGLDisplay, eglSurface, nsecs);
    }

    /**
     * Check whether our context and the specified surface are current.
     *
     * @param eglSurface the specified EGLSurface.
     * @return Returns true if our context and the specified surface are current.
     */
    public boolean isCurrent(EGLSurface eglSurface) {
        return mEGLContext.equals(EGL14.eglGetCurrentContext())
                && eglSurface.equals(EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
    }

    /**
     * Performs a simple surface query.
     *
     * @param eglSurface specifies the EGLSurface to query.
     * @param what Specifies the EGL surface attribute to be return.
     * @return the requested value.
     */
    public int querySurface(EGLSurface eglSurface, int what) {
        int[] value = new int[1];
        EGL14.eglQuerySurface(
                mEGLDisplay,
                eglSurface,
                what,
                value,
                0);
        return value[0];
    }

    /**
     * Get the GLES version this context is configured for (2 or 3).
     *
     * @return Returns the GLES version this context is configured for (2 or 3).
     */
    public int getGlVersion() {
        return mGlVersion;
    }

    /**
     * Writes the current display, context, and surface to the log.
     * @param msg message.
     */
    public static void logCurrent(String msg) {
        EGLDisplay display;
        EGLContext context;
        EGLSurface surface;
        display = EGL14.eglGetCurrentDisplay();
        context = EGL14.eglGetCurrentContext();
        surface = EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW);
        LogHelper.i(TAG, "Current EGL (" + msg +
                        "): display=" + display +
                        ", context=" + context +
                        ", surface=" + surface);
    }

    private void init(EGLContext sharedContext, int flags, int[] outputFormats) {
        LogHelper.d(TAG, "[init]+");
        if (mEGLDisplay != EGL14.EGL_NO_DISPLAY) {
            throw new RuntimeException("[init] EGL already set up");
        }
        if (sharedContext == null) {
            sharedContext = EGL14.EGL_NO_CONTEXT;
        }

        mEGLDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
        if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
            throw new RuntimeException("[init] unable to get EGL14 display");
        }

        int[] version = new int[2];
        if (!EGL14.eglInitialize(mEGLDisplay, version, 0, version, 1)) {
            mEGLDisplay = null;
            throw new RuntimeException("[init] unable to initialize EGL14");
        }

        EglConfigSelector eglConfigSelector = new EglConfigSelector();
        if (outputFormats != null) {
            eglConfigSelector.setSupportedFormats(outputFormats);
        }
        mEGLConfig = eglConfigSelector.chooseConfigEGL14(
                mEGLDisplay,
                (flags & CONSTRUCTOR_FLAG_RECORDABLE) != 0);
        mOutputPixelFormat = eglConfigSelector.getSelectedPixelFormat();

        mEGLContext = null;
        if ((flags & CONSTRUCTOR_FLAG_TRY_GLES3) != 0) {
            // If requested, try to configure context for OpenGL ES 3.x. Note that
            // contexts with different client versions can't share state.
            mGlVersion = 3;
            int[] attribute3List = {
                    EGL14.EGL_CONTEXT_CLIENT_VERSION,
                    3,
                    EGL14.EGL_NONE
            };
            mEGLContext = EGL14.eglCreateContext(
                    mEGLDisplay,
                    mEGLConfig,
                    sharedContext,
                    attribute3List,
                    0);
            if (EGL14.eglGetError() != EGL14.EGL_SUCCESS) {
                LogHelper.i(TAG, "[init] GLES 3.x not available");
                mEGLContext = null;
            }
        }
        if (mEGLContext == null) {
            mGlVersion = 2;
            int[] attribute2List = {
                    EGL14.EGL_CONTEXT_CLIENT_VERSION,
                    2,
                    EGL14.EGL_NONE};
            mEGLContext = EGL14.eglCreateContext(
                    mEGLDisplay,
                    mEGLConfig,
                    sharedContext,
                    attribute2List,
                    0);
        }
        EglUtil.checkEglError("[init] eglCreateContext");
        if (mEGLContext == null) {
            throw new RuntimeException("[init] null context");
        }
        LogHelper.d(TAG, "[init]-");
    }
}