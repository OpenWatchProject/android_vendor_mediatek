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
package com.mediatek.camera.mode.pip.pipwrapping;

import android.app.Activity;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.view.Surface;

import com.mediatek.camera.util.Log;

import java.nio.FloatBuffer;

public class CaptureRenderer extends Renderer {
    private static final String TAG = CaptureRenderer.class.getSimpleName();
    private FloatBuffer mVtxBuf;
    private FloatBuffer mTexCoordBuf;
    private float[] mPosMtx = GLUtil.createIdentityMtx();
    private float[] mPMtx = GLUtil.createIdentityMtx();
    private float[] mVMtx = GLUtil.createIdentityMtx();
    private float[] mMMtx = GLUtil.createIdentityMtx();
    private float[] mRotateMtx = GLUtil.createIdentityMtx();

    private EglCore mEglCore;
    private Surface mCaptureSurface;
    private WindowSurface mCaptureEGLSurface;

    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTexCoordHandle = -1;
    private int muTexRotateMtxHandle = -1;
    private int muPosMtxHandle = -1;
    private int muSamplerHandle = -1;

    public CaptureRenderer(Activity activity) {
        super(activity);
    }

    public void init() {
        Log.d(TAG, "initScreenSurface");
        mEglCore = new EglCore(EGL14.eglGetCurrentContext(), EglCore.FLAG_TRY_GLES3);
        initGL();
        mTexCoordBuf = createFloatBuffer(mTexCoordBuf, GLUtil.createTexCoord());
    }

    public void setCaptureSize(int width, int height, int orientation) {
        Log.d(TAG, "setCaptureSize width = " + width + " height = " + height
                + " orientation:" + orientation);
        int minValue = Math.min(width, height);
        int maxValue = Math.max(width, height);
        if (orientation % 180 != 0) {
            width = maxValue;
            height = minValue;
            mRotateMtx = GLUtil.createIdentityMtx();
            Matrix.translateM(mRotateMtx, 0, mRotateMtx, 0, .5f, .5f, 0);
            Matrix.rotateM(mRotateMtx, 0, orientation, 0, 0, 1);
            Matrix.translateM(mRotateMtx, 0, -.5f, -.5f, 0);
        } else {
            width = minValue;
            height = maxValue;
        }
        setRendererSize(width, height);
    }

    @Override
    public void setRendererSize(int width, int height) {
        if (width == getRendererWidth() && height == getRendererHeight()) {
            return;
        }
        resetMatrix();
        super.setRendererSize(width, height);
        Matrix.orthoM(mPMtx, 0, 0, width, 0, height, -1, 1);
        Matrix.multiplyMM(mPosMtx, 0, mMMtx, 0, mVMtx, 0);
        Matrix.multiplyMM(mPosMtx, 0, mPMtx, 0, mPosMtx, 0);
        initVertexData(width, height);
    }

    public void setCaptureSurface(Surface surface) {
        if (surface == null) {
            throw new RuntimeException("setCaptureSurface capture surface is null!!!!!");
        }
        mCaptureSurface = surface;
    }

    public void draw(int texId) {
        Log.d(TAG, "draw texId = " + texId + " mCaptureSurface = " + mCaptureSurface);
        if (mCaptureSurface != null) {
            mCaptureEGLSurface = new WindowSurface(mEglCore, mCaptureSurface);
            mCaptureEGLSurface.makeCurrent();
            Log.d(TAG, "Surface width = " + mCaptureEGLSurface.getWidth() + " height = "
                    + mCaptureEGLSurface.getHeight());
        }
        if (getRendererWidth() <= 0 || getRendererHeight() <= 0 || mCaptureEGLSurface == null) {
            return;
        }
        GLUtil.checkGlError("CaptureDraw_Start");
        GLES20.glViewport(0, 0, getRendererWidth(), getRendererHeight());
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
        // use program
        GLES20.glUseProgram(mProgram);
        // vertex
        mVtxBuf.position(0);
        GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false, 4 * 3, mVtxBuf);
        mTexCoordBuf.position(0);
        GLES20.glVertexAttribPointer(maTexCoordHandle, 2, GLES20.GL_FLOAT, false, 4 * 2,
                mTexCoordBuf);
        GLES20.glEnableVertexAttribArray(maPositionHandle);
        GLES20.glEnableVertexAttribArray(maTexCoordHandle);
        // matrix
        GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mPosMtx, 0);
        GLES20.glUniformMatrix4fv(muTexRotateMtxHandle, 1, false, mRotateMtx, 0);
        // sampler
        GLES20.glUniform1i(muSamplerHandle, 0);
        // texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texId);
        // draw
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 6);
        mCaptureEGLSurface.swapBuffers();
        GLUtil.checkGlError("CaptureDraw_End");
        mCaptureEGLSurface.makeNothingCurrent();
        mCaptureEGLSurface.releaseEglSurface();
        mCaptureEGLSurface = null;
        Log.d(TAG, "draw end");
    }

    @Override
    public void release() {
        if (mCaptureEGLSurface != null) {
            mCaptureEGLSurface.makeNothingCurrent();
            mCaptureEGLSurface.release();
            mCaptureEGLSurface = null;
        }
        if (mEglCore != null) {
            mEglCore.release();
            mEglCore = null;
        }
    }

    private void initVertexData(float width, float height) {
        mVtxBuf = createFloatBuffer(mVtxBuf, GLUtil.createFullSquareVtx(width, height));
    }

    private void initGL() {
        Log.d(TAG, "ScreenRenderer");
        GLUtil.checkGlError("initGL_Start");
        final String vertexShader =
                "attribute vec4 aPosition;\n"
              + "attribute vec4 aTexCoord;\n"
              + "uniform   mat4 uPosMtx;\n"
              + "uniform   mat4 uTexRotateMtx;\n"
              + "varying   vec2 vTexCoord;\n"
              + "void main() {\n"
              + "  gl_Position = uPosMtx * aPosition;\n"
              + "  vTexCoord     = (uTexRotateMtx * aTexCoord).xy;\n"
              + "}\n";
        final String fragmentShader =
                "precision mediump float;\n"
              + "uniform sampler2D uSampler;\n"
              + "varying vec2      vTexCoord;\n"
              + "void main() {\n"
              + "  gl_FragColor = texture2D(uSampler, vTexCoord);\n"
              + "}\n";
        mProgram = GLUtil.createProgram(vertexShader, fragmentShader);
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        maTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        // matrix
        muTexRotateMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexRotateMtx");
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        // sampler
        muSamplerHandle = GLES20.glGetUniformLocation(mProgram, "uSampler");

        GLES20.glDisable(GLES20.GL_DEPTH_TEST);
        GLES20.glDisable(GLES20.GL_CULL_FACE);
        GLES20.glDisable(GLES20.GL_BLEND);
        GLUtil.checkGlError("initGL_E");
    }

    private void resetMatrix() {
        mPosMtx = GLUtil.createIdentityMtx();
        mPMtx = GLUtil.createIdentityMtx();
        mVMtx = GLUtil.createIdentityMtx();
        mMMtx = GLUtil.createIdentityMtx();
    }
}
