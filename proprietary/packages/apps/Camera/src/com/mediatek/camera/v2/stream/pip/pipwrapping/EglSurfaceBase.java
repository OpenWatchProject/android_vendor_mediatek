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
package com.mediatek.camera.v2.stream.pip.pipwrapping;

import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.opengl.EGL14;
import android.opengl.EGLSurface;
import android.opengl.GLES20;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Common base class for EGL surfaces.
 * <p>
 * There can be multiple surfaces associated with a single context.
 */
public class EglSurfaceBase {
    // EglBase object we're associated with. It may be associated with multiple
    // surfaces.
    protected EglCore mEglBase;

    private EGLSurface mEGLSurface = EGL14.EGL_NO_SURFACE;
    private int mWidth = -1;
    private int mHeight = -1;

    protected EglSurfaceBase(EglCore eglBase) {
        mEglBase = eglBase;
    }

    /**
     * Creates a window surface.
     * <p>
     *
     * @param surface
     *            May be a Surface or SurfaceTexture.
     */
    public void createWindowSurface(Object surface) {
        if (mEGLSurface != EGL14.EGL_NO_SURFACE) {
            throw new IllegalStateException("surface already created");
        }
        mEGLSurface = mEglBase.createWindowSurface(surface);
        mWidth = mEglBase.querySurface(mEGLSurface, EGL14.EGL_WIDTH);
        mHeight = mEglBase.querySurface(mEGLSurface, EGL14.EGL_HEIGHT);
    }

    /**
     * Creates an off-screen surface.
     */
    public void createOffscreenSurface(int width, int height) {
        if (mEGLSurface != EGL14.EGL_NO_SURFACE) {
            throw new IllegalStateException("surface already created");
        }
        mEGLSurface = mEglBase.createOffscreenSurface(width, height);
        mWidth = width;
        mHeight = height;
    }

    /**
     * Returns the surface's width, in pixels.
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Returns the surface's height, in pixels.
     */
    public int getHeight() {
        return mHeight;
    }

    /**
     * Release the EGL surface.
     */
    public void releaseEglSurface() {
        mEglBase.releaseSurface(mEGLSurface);
        mEGLSurface = EGL14.EGL_NO_SURFACE;
        mWidth = mHeight = -1;
    }

    /**
     * Makes our EGL context and surface current.
     */
    public void makeCurrent() {
        mEglBase.makeCurrent(mEGLSurface);
    }

    /**
     * Makes no context current.
     */
    public void makeNothingCurrent() {
        mEglBase.makeNothingCurrent();
    }

    /**
     * Calls eglSwapBuffers. Use this to "publish" the current frame.
     *
     * @return false on failure
     */
    public boolean swapBuffers() {
        return mEglBase.swapBuffers(mEGLSurface);
    }

    /**
     * Sends the presentation time stamp to EGL.
     *
     * @param nsecs
     *            Timestamp, in nanoseconds.
     */
    public void setPresentationTime(long nsecs) {
        mEglBase.setPresentationTime(mEGLSurface, nsecs);
    }

    /**
     * Saves the EGL surface to a file.
     * <p>
     * Expects that this object's EGL surface is current.
     */
    public void saveFrame(File file) throws IOException {
        if (!mEglBase.isCurrent(mEGLSurface)) {
            throw new RuntimeException("Expected EGL context/surface is not current");
        }
        // glReadPixels gives us a ByteBuffer filled with what is essentially
        // big-endian RGBA
        // data (i.e. a byte of red, followed by a byte of green...). We need an
        // int[] filled
        // with little-endian ARGB data to feed to Bitmap.
        //
        // If we implement this as a series of buf.get() calls, we can spend 2.5
        // seconds just
        // copying data around for a 720p frame. It's better to do a bulk get()
        // and then
        // rearrange the data in memory. (For comparison, the PNG compress takes
        // about 500ms
        // for a trivial frame.)
        //
        // So... we set the ByteBuffer to little-endian, which should turn the
        // bulk IntBuffer
        // get() into a straight memcpy on most Android devices. Our ints will
        // hold ABGR data.
        // Swapping B and R gives us ARGB.
        //
        // Making this even more interesting is the upside-down nature of GL,
        // which means
        // our output will look upside-down relative to what appears on screen
        // if the
        // typical GL conventions are used.

        String filename = file.toString();

        ByteBuffer buf = ByteBuffer.allocateDirect(mWidth * mHeight * 4);
        buf.order(ByteOrder.LITTLE_ENDIAN);
        if (PipEGLConfigWrapper.getInstance().getPixelFormat() == PixelFormat.RGBA_8888) {
            GLES20.glReadPixels(0, 0, mWidth, mHeight,
                    GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, buf);
        } else if (PipEGLConfigWrapper.getInstance().getPixelFormat()
                == PixelFormat.RGB_565) {
            GLES20.glReadPixels(0, 0, mWidth, mHeight,
                    GLES20.GL_RGB, GLES20.GL_UNSIGNED_SHORT_5_6_5, buf);
        }
        buf.rewind();

        int pixelCount = mWidth * mHeight;
        int[] colors = new int[pixelCount];
        buf.asIntBuffer().get(colors);
        for (int i = 0; i < pixelCount; i++) {
            int c = colors[i];
            colors[i] = (c & 0xff00ff00) | ((c & 0x00ff0000) >> 16) | ((c & 0x000000ff) << 16);
        }

        BufferedOutputStream bos = null;
        try {
            bos = new BufferedOutputStream(new FileOutputStream(filename));
            Bitmap bmp = Bitmap.createBitmap(colors, mWidth, mHeight, Bitmap.Config.ARGB_8888);
            bmp.compress(Bitmap.CompressFormat.PNG, 90, bos);
            bmp.recycle();
        } finally {
            if (bos != null)
                bos.close();
        }
    }
}
