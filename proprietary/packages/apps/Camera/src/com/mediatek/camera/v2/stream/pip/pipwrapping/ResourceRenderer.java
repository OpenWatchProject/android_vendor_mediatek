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

import android.app.Activity;
import android.graphics.RectF;
import android.opengl.GLES20;
import android.opengl.Matrix;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.io.IOException;
import java.nio.FloatBuffer;

public class ResourceRenderer extends Renderer {
    private static final Tag TAG = new Tag(ResourceRenderer.class.getSimpleName());
    // position
    private FloatBuffer         mVtxBuf;
    private FloatBuffer         mTexCoordBuf;
    // matrix
    private float[]             mPosMtx = GLUtil.createIdentityMtx();
    private float[]             mMMtx = GLUtil.createIdentityMtx();
    private float[]             mVMtx = GLUtil.createIdentityMtx();
    private float[]             mPMtx = GLUtil.createIdentityMtx();
    private int                 mProgram = -1;
    private int                 maPositionHandle = -1;
    private int                 maTexCoordHandle = -1;
    private int                 muPosMtxHandle = -1;
    private int                 muResourceSamplerHandle = -1;
    private int                 muTexRotateMtxHandle = -1;

    private RectF               mResourceRect;

    private int                 mResourceId    = -1;
    private int                 mResourceTexId = -12345;

    final String vertexShader =
            "attribute vec4 aPosition;\n" +
            "attribute vec4 aTexCoord;\n" +
            "uniform   mat4 uPosMtx;\n" +
            "uniform   mat4 uTexRotateMtx;\n" +
            "varying   vec2 vTexCoord;\n" +
            "void main() {\n" +
            "  gl_Position = uPosMtx * aPosition;\n" +
            "  vTexCoord   = (uTexRotateMtx * aTexCoord).xy;\n" +
            "}\n";
    final String fragmentShader =
            "precision mediump float;\n" +
            "uniform sampler2D uResourceSampler;\n" +
            "varying vec2               vTexCoord;\n" +
            "void main() {\n" +
            "        gl_FragColor = texture2D(uResourceSampler, vTexCoord);\n" +
            "}\n";

    public ResourceRenderer(Activity activity) {
        super(activity);
        mTexCoordBuf = createFloatBuffer(mTexCoordBuf, GLUtil.createTexCoord());
        mResourceRect = new RectF();
    }

    public void init() {
        LogHelper.d(TAG, "initResource");
        initProgram();
    }

    public void updateTemplate(int resourceId) {
        LogHelper.d(TAG, "updateTemplate resourceId = " + resourceId);
        if (resourceId == mResourceId) {
            return;
        }
        releaseResource();
        try {
            mResourceTexId = initBitmapTexture(resourceId, false);
        } catch (IOException e) {
            LogHelper.e(TAG, "initBitmapTexture faile + " + e);
        }
    }

    public void releaseResource() {
        if (mResourceTexId > 0) {
            releaseBitmapTexture(mResourceTexId);
            mResourceTexId = -12345;
        }
    }

    public RectF getResourceRect() {
        return mResourceRect;
    }

    @Override
    public void setRendererSize(int width, int height, boolean needReverse) {
        LogHelper.d(TAG, "setRendererSize width = " + width + " height = " + height);
        if (width == getRendererWidth() && height == getRendererHeight()) {
            return;
        }
        resetMatrix();
        super.setRendererSize(width, height);
        Matrix.orthoM(mPMtx, 0, 0, width, 0, height, -1, 1);
        if (needReverse || width < height) {
            // portrait should reverse coordinate
            Matrix.translateM(mMMtx, 0, 0, height, 0);
            Matrix.scaleM(mMMtx, 0, mMMtx, 0, 1, -1, 1);
        }
        Matrix.multiplyMM(mPosMtx, 0, mMMtx, 0, mVMtx, 0);
        Matrix.multiplyMM(mPosMtx, 0, mPMtx, 0, mPosMtx, 0);
    }

    private void initVertexData(float rCenterX, float rCenterY, float edge) {
        LogHelper.d(TAG, "initVertexData rCenterX = " + rCenterX + " rCenterY = " +
                rCenterY + " edge = " + edge);
        mVtxBuf = createFloatBuffer(mVtxBuf,
                GLUtil.createSquareVtxByCenterEdge(rCenterX, rCenterY, edge));
        mResourceRect.set(rCenterX - edge / 2, rCenterY - edge / 2, rCenterX +
                edge / 2, rCenterY + edge / 2);
    }

    public void draw(float rCenterX, float rCenterY, float edge, FloatBuffer vtxBuf) {
        LogHelper.d(TAG, "ResourceRendrer draw start rCenterX = "
                + rCenterX + " rCenterY = " + rCenterY
                + " edge = " + edge + " vtxBuf = " + vtxBuf);
        if (getRendererWidth() <= 0 || getRendererHeight() <= 0) {
            return;
        }
        GLES20.glEnable(GLES20.GL_BLEND);
        GLES20.glBlendFunc(GLES20.GL_ONE, GLES20.GL_ONE_MINUS_SRC_ALPHA);
        GLUtil.checkGlError("ResourceRendrer draw start");
        // use program
        GLES20.glUseProgram(mProgram);
        // position
        if (vtxBuf == null) {
            initVertexData(rCenterX, rCenterY, edge);
            mVtxBuf.position(0);
            GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false, 4 * 3,
                    mVtxBuf);
        } else {
            vtxBuf.position(0);
            GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT,
                    false, 4 * 3, vtxBuf);
        }

        mTexCoordBuf.position(0);
        GLES20.glVertexAttribPointer(maTexCoordHandle, 2, GLES20.GL_FLOAT, false, 4 * 2,
                mTexCoordBuf);
        GLES20.glEnableVertexAttribArray(maPositionHandle);
        GLES20.glEnableVertexAttribArray(maTexCoordHandle);
        // matrix
        GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mPosMtx, 0);
        GLES20.glUniformMatrix4fv(muTexRotateMtxHandle, 1, false, GLUtil.createIdentityMtx(), 0);
        // sampler
        GLES20.glUniform1i(muResourceSamplerHandle, 0);
        // texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mResourceTexId);
        // draw
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 6);
        GLUtil.checkGlError("ResourceRendrer draw end");
        LogHelper.d(TAG, "ResourceRendrer draw end");
    }

    private void initProgram() {
        mProgram = createProgram(vertexShader, fragmentShader);
        // position
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        maTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        // matrix
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        muTexRotateMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexRotateMtx");
        // sampler
        muResourceSamplerHandle = GLES20.glGetUniformLocation(mProgram, "uResourceSampler");
    }

    private void resetMatrix() {
        mPosMtx = GLUtil.createIdentityMtx();
        mPMtx = GLUtil.createIdentityMtx();
        mVMtx = GLUtil.createIdentityMtx();
        mMMtx = GLUtil.createIdentityMtx();
    }
}
