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
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.Matrix;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.nio.FloatBuffer;

public class BottomGraphicRenderer extends Renderer {
    private static final Tag TAG = new Tag(BottomGraphicRenderer.class.getSimpleName());
    // position
    private FloatBuffer mVtxBuf;
    private FloatBuffer mTexCoordBuf;
    // matrix
    private float[] mPosMtx = GLUtil.createIdentityMtx();
    private float[] mPMtx = GLUtil.createIdentityMtx();
    private float[] mVMtx = GLUtil.createIdentityMtx();
    private float[] mMMtx = GLUtil.createIdentityMtx();
    private float[] mEditMtx = GLUtil.createIdentityMtx();

    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTexCoordHandle = -1;
    private int muPosMtxHandle = -1;
    private int muTexMtxHandle = -1;
    private int muTexRotateMtxHandle = -1;
    private int muIsPreviewHandle = -1;
    private int muPreviewSamplerHandle = -1;
    private int muPictureSampleHandle = -1;

    final String vertexShader = "" +
            "attribute vec4    aPosition;\n"
          + "attribute vec4    aTexCoord;\n"
          + "uniform   float   uIsPreview;\n"
          + "uniform   mat4    uPosMtx;\n"
          + "uniform   mat4    uTexMtx;\n"
          + "uniform   mat4    uTexRotateMtx;\n"
          + "varying   vec2    vTexCoord;\n"
          + "varying   float   vfIsPreview;\n"
          + "void main() {\n"
          + "    gl_Position   = uPosMtx * aPosition;\n"
          + "    vTexCoord     = (uTexRotateMtx * uTexMtx * aTexCoord).xy;\n"
          + "    vfIsPreview   = uIsPreview;\n"
          + "}\n";

    final String fragmentShader =
              "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "uniform   sampler2D            uPictureSampler;\n"
            + "uniform   samplerExternalOES   uPreviewSampler;\n"
            + "varying   vec2                 vTexCoord;\n"
            + "varying   float                vfIsPreview;\n"
            + "void main() {\n"
            + "    if (vfIsPreview > 0.0) {\n"
            + "        gl_FragColor = texture2D(uPreviewSampler, vTexCoord);\n"
            + "    } else { \n"
            + "        gl_FragColor = texture2D(uPictureSampler, vTexCoord);\n"
            + "    }\n"
            + "}\n";

    public BottomGraphicRenderer(Activity activity) {
        super(activity);
        initProgram();
    }

    @Override
    public void setRendererSize(int width, int height, boolean needReverse) {
        LogHelper.d(TAG, "setRendererSize width = "
                + width + " height = " + height + " needReverse = "
                + needReverse);
        resetMatrix();
        super.setRendererSize(width, height);
        Matrix.orthoM(mPMtx, 0, 0, width, 0, height, -1, 1);
        if (needReverse) {
            Matrix.translateM(mMMtx, 0, 0, height, 0);
            Matrix.scaleM(mMMtx, 0, mMMtx, 0, 1, -1, 1);
        }
        Matrix.multiplyMM(mPosMtx, 0, mEditMtx, 0, mMMtx, 0);
        Matrix.multiplyMM(mPosMtx, 0, mVMtx, 0, mPosMtx, 0);
        Matrix.multiplyMM(mPosMtx, 0, mPMtx, 0, mPosMtx, 0);
        mVtxBuf = createFloatBuffer(mVtxBuf, GLUtil.createFullSquareVtx(width, height));
    }

    public void draw(int preTexId, final float[] preTexMtx,
            final float[] texReverseRotateMtx, boolean needFlip) {
        GLUtil.checkGlError("BottomGraphicRenderer draw start");
        int sampleId = (preTexMtx == null) ? 2 : 1;
        //use program
        GLES20.glUseProgram(mProgram);
        //position
        mVtxBuf.position(0);
        GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false, 4 * 3, mVtxBuf);
        mTexCoordBuf = createFloatBuffer(mTexCoordBuf, GLUtil.createTexCoord(0, 1, 0, 1, needFlip));
        mTexCoordBuf.position(0);
        GLES20.glVertexAttribPointer(maTexCoordHandle, 2,
                GLES20.GL_FLOAT, false, 4 * 2, mTexCoordBuf);
        GLES20.glEnableVertexAttribArray(maPositionHandle);
        GLES20.glEnableVertexAttribArray(maTexCoordHandle);
        //matrix
        GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mPosMtx, 0);
        GLES20.glUniform1f(muIsPreviewHandle, (preTexMtx == null) ? 0.0f : 1.0f);
        GLES20.glUniformMatrix4fv(muTexMtxHandle, 1, false, preTexMtx, 0);
        GLES20.glUniformMatrix4fv(muTexRotateMtxHandle, 1, false, texReverseRotateMtx, 0);
        //sampler
        GLES20.glUniform1i((preTexMtx == null) ? muPictureSampleHandle : muPreviewSamplerHandle,
                sampleId);
        //texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0 + sampleId);
        GLES20.glBindTexture((preTexMtx == null) ? GLES20.GL_TEXTURE_2D :
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES, preTexId);
        //draw
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 6);
        GLUtil.checkGlError("BottomGraphicRenderer draw end");
    }

    private void initProgram() {
        LogHelper.d(TAG, "initProgram");
        mProgram = createProgram(vertexShader, fragmentShader);
        GLUtil.checkGlError("BottomGraphicRenderer after mProgram");
        // position
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        maTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        // matrix
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        muTexMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexMtx");
        muIsPreviewHandle = GLES20.glGetUniformLocation(mProgram, "uIsPreview");
        muTexRotateMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexRotateMtx");
        // sampler
        muPreviewSamplerHandle = GLES20.glGetUniformLocation(mProgram, "uPreviewSampler");
        muPictureSampleHandle = GLES20.glGetUniformLocation(mProgram, "uPictureSampler");
        GLUtil.checkGlError("BottomGraphicRenderer initProgram");
    }

    private void resetMatrix() {
        mPosMtx = GLUtil.createIdentityMtx();
        mPMtx = GLUtil.createIdentityMtx();
        mVMtx = GLUtil.createIdentityMtx();
        mMMtx = GLUtil.createIdentityMtx();
        mEditMtx = GLUtil.createIdentityMtx();
    }
}
