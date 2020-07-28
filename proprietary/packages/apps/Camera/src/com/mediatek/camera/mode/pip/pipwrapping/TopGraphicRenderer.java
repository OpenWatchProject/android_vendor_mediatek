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
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.Matrix;

import com.android.camera.PIPCustomization;
import com.android.camera.Util;
import com.mediatek.camera.util.Log;

import java.io.IOException;
import java.nio.FloatBuffer;

public class TopGraphicRenderer extends Renderer {
    private static final String TAG = "TopGraphicRenderer";

    // position
    private FloatBuffer mVtxBuf;
    private FloatBuffer mTexCoordBuf;
    private FloatBuffer mTempTexCoordBuf;
    // matrix
    private float[] mMVPMtx = GLUtil.createIdentityMtx();
    private float[] mMMtx = GLUtil.createIdentityMtx();
    private float[] mVMtx = GLUtil.createIdentityMtx(); // view
    private float[] mPMtx = GLUtil.createIdentityMtx(); // projection
    private int mBackTempTexId = -12345;

    private ResourceRenderer mTopTemplateRenderer;

    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTexCoordHandle = -1;
    private int maTempTexCoordHandle = -1;
    private int muPosMtxHandle = -1;
    private int muTexMtxHandle = -1;
    private int muTexRotateMtxHandle = -1;
    private int muIsPreviewHandle = -1;
    private int muPictureSampleHandle = -1;
    private int muPreviewSamplerHandle = -1;
    private int muBackTempSamplerHandle = -1;

    final String vertexShader = "attribute vec4   aPosition;\n"
            + "attribute vec4   aTexCoord;\n"
            + "attribute vec4   aTempTexCoord;\n"
            + "uniform   float  uIsPreview;\n"
            + "uniform   mat4   uPosMtx;\n"
            + "uniform   mat4   uTexMtx;\n"
            + "uniform   mat4   uTexRotateMtx;\n"
            + "varying   vec2   vTexCoord;\n"
            + "varying   vec2   vTempTexCoord;\n"
            + "varying   float  vIsPreview;\n"
            + "void main() {\n" + "    gl_Position    = uPosMtx * aPosition;\n"
            + "    vTexCoord     = (uTexRotateMtx * uTexMtx * aTexCoord).xy;\n"
            + "    vTempTexCoord  = aTempTexCoord.xy;\n"
            + "    vIsPreview     = uIsPreview;\n"
            + "}\n";

    final String fragmentShader = "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n" + "uniform   samplerExternalOES uPreviewSampler;\n"
            + "uniform   sampler2D uPictureSampler;\n" + "uniform   sampler2D uBackSampler;\n"
            + "varying   vec2               vTexCoord;\n" + "varying   vec2       vTempTexCoord;\n"
            + "varying   float  vIsPreview;\n" + "const vec3 black = vec3(0, 0, 0);  \n"
            + "void main() {\n"
            + "    vec3 texture1 = vec3(texture2D(uBackSampler,vTempTexCoord).rgb);\n"
            + "    if((equal(texture1, black)).r) {\n"
            + "        gl_FragColor = vec4(0, 0, 0, 0);\n" + "    } else {\n"
            + "        if (vIsPreview > 0.0) {\n"
            + "            gl_FragColor = texture2D(uPreviewSampler, vTexCoord);\n"
            + "        } else { \n"
            + "            gl_FragColor = texture2D(uPictureSampler, vTexCoord);\n" + "        }\n"
            + "    }\n" + "}\n";

    public TopGraphicRenderer(Activity activity) {
        super(activity);
        Log.d(TAG, "TopGraphicRenderer");
        initProgram();
        mTexCoordBuf = createFloatBuffer(mTexCoordBuf, GLUtil.createTexCoord());
        mTempTexCoordBuf = createFloatBuffer(mTempTexCoordBuf, GLUtil.createTexCoord());
        mTopTemplateRenderer = new ResourceRenderer(getActivity());
        mTopTemplateRenderer.init();
    }

    public void initTemplateTexture(int backResource, int frontResource) {
        Log.d(TAG, "initTemplateTexture");
        if (mBackTempTexId > 0) {
            releaseBitmapTexture(mBackTempTexId);
            mBackTempTexId = -12345;
        }
        try {
            if (backResource > 0) {
                mBackTempTexId = initBitmapTexture(backResource, true);
            }
        } catch (IOException e) {
            Log.e(TAG, "initBitmapTexture faile + " + e);
        }
        if (mTopTemplateRenderer != null && frontResource > 0) {
            mTopTemplateRenderer.updateTemplate(frontResource);
        }
    }

    @Override
    public void setRendererSize(int width, int height) {
        Log.d(TAG, "setRendererSize width = " + width + " height = " + height);
        if (width == getRendererWidth() && height == getRendererHeight()) {
            return;
        }
        resetMatrix();
        Matrix.orthoM(mPMtx, 0, 0, width, 0, height, -1, 1);
        // hand stands MVP matrix to match phone's coordinate system
        Matrix.translateM(mMMtx, 0, 0, height, 0);
        Matrix.scaleM(mMMtx, 0, mMMtx, 0, 1, -1, 1);

        Matrix.multiplyMM(mMVPMtx, 0, mMMtx, 0, mMVPMtx, 0);
        Matrix.multiplyMM(mMVPMtx, 0, mVMtx, 0, mMVPMtx, 0);
        Matrix.multiplyMM(mMVPMtx, 0, mPMtx, 0, mMVPMtx, 0);
        super.setRendererSize(width, height);
        mTopTemplateRenderer.setRendererSize(width, height);
    }

    public void draw(int preTex, final float[] preTexMtx, final float[] texReverseRotateMtx,
            final AnimationRect topRect, int rotation, boolean needFlip) {
        if (preTex <= 0 || topRect == null) {
            return;
        }
        // copy AnimationRect
        AnimationRect animationRect = new AnimationRect();
        animationRect.setRendererSize(topRect.getPreviewWidth(), topRect.getPreviewHeight());
        animationRect.setCurrentScaleValue(topRect.getCurrentScaleValue());
        animationRect.setOriginalDistance(topRect.getOriginalDistance());
        animationRect.initialize(topRect.getRectF().left, topRect.getRectF().top,
                topRect.getRectF().right, topRect.getRectF().bottom);
        animationRect.rotate(topRect.getCurrrentRotationValue());
        // keep original centerX and centerY
        float centerX = animationRect.getRectF().centerX();
        float centerY = animationRect.getRectF().centerY();
        // translate big box to match small box's center point
        CropBox cropBox = getCropBox();
        animationRect.translate(cropBox.getTranslateXRatio() * animationRect.getRectF().width(),
                cropBox.getTranslateYRatio() * animationRect.getRectF().height(), false);
        animationRect.rotate(animationRect.getCurrrentRotationValue());
        // scale by the same ratio to find the smallest box to wrap the small
        // box
        animationRect.scale(cropBox.getScaleRatio(), false);
        animationRect.rotate(animationRect.getCurrrentRotationValue(), centerX, centerY);
        GLUtil.checkGlError("TopGraphicRenderer draw start");
        // compute crop area
        boolean isLandScape = getRendererWidth() > getRendererHeight();
        int longer = Math.max(getRendererWidth(), getRendererHeight());
        int shorter = Math.min(getRendererWidth(), getRendererHeight());
        float lowHeight = .0f;
        float highHeight = .0f;
        float lowWidth = .0f;
        float highWidth = .0f;
        float topGraphicCropValue = PIPCustomization.TOP_GRAPHIC_CROP_RELATIVE_POSITION_VALUE;
        if (rotation < 0) {
            // when gpu coordinate not reverse, crop value must reverse
            // because the following crop value is computed by reverse coordinate.
            if (-90 == rotation || -180 == rotation) {
                topGraphicCropValue = 1 - topGraphicCropValue;
            }
            // take picture, preview is not reverse
            lowHeight = isLandScape ? 0 : (longer - shorter) * topGraphicCropValue / longer;
            highHeight = isLandScape ? 1 : ((longer - shorter) * topGraphicCropValue + shorter)
                    / longer;
            // that is top is sub camera
            boolean bottomIsMainCamera = Util.bottomGraphicIsMainCamera(getActivity());
            lowWidth = isLandScape ? (longer - shorter)
                    * (bottomIsMainCamera ? (1f - topGraphicCropValue) :
                        topGraphicCropValue) / longer
                    : 0;
            highWidth = isLandScape ? ((longer - shorter)
                    * (bottomIsMainCamera ? (1f - topGraphicCropValue) :
                        topGraphicCropValue) + shorter) / longer
                    : 1;
            mTexCoordBuf = createFloatBuffer(mTexCoordBuf,
                    GLUtil.createTexCoord(lowWidth, highWidth, lowHeight, highHeight, needFlip));
        } else {
            lowHeight = isLandScape ? 0 : (longer - shorter) * (1f - topGraphicCropValue) / longer;
            highHeight = isLandScape ? 1
                    : ((longer - shorter) * (1f - topGraphicCropValue) + shorter) / longer;
            lowWidth = isLandScape ? (longer - shorter) * (1f - topGraphicCropValue) / longer : 0;
            highWidth = isLandScape ? ((longer - shorter) * (1f - topGraphicCropValue) + shorter)
                    / longer : 1;
            switch (rotation) {
            case 0:
                mTexCoordBuf = createFloatBuffer(mTexCoordBuf,
                        GLUtil.createReverseStandTexCoord(
                                lowWidth, highWidth, lowHeight, highHeight));
                break;
            case 90:
                mTexCoordBuf = createFloatBuffer(mTexCoordBuf,
                        GLUtil.createRightTexCoord(
                                lowWidth, highWidth, lowHeight, highHeight));
                break;
            case 180:
                mTexCoordBuf = createFloatBuffer(mTexCoordBuf,
                        GLUtil.createStandTexCoord(
                                lowWidth, highWidth, lowHeight, highHeight));
                break;
            case 270:
                mTexCoordBuf = createFloatBuffer(mTexCoordBuf,
                        GLUtil.createLeftTexCoord(
                                lowWidth, highWidth, lowHeight, highHeight));
                break;
            }
        }
        GLES20.glUseProgram(mProgram);
        // position
        mVtxBuf = createFloatBuffer(mVtxBuf, GLUtil.createTopRightRect(animationRect));
        mVtxBuf.position(0);
        GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false, 4 * 3, mVtxBuf);
        mTexCoordBuf.position(0);
        GLES20.glVertexAttribPointer(maTexCoordHandle, 2, GLES20.GL_FLOAT, false, 4 * 2,
                mTexCoordBuf);
        mTempTexCoordBuf.position(0);
        GLES20.glVertexAttribPointer(maTempTexCoordHandle, 2, GLES20.GL_FLOAT, false, 4 * 2,
                mTempTexCoordBuf);
        GLES20.glEnableVertexAttribArray(maPositionHandle);
        GLES20.glEnableVertexAttribArray(maTexCoordHandle);
        GLES20.glEnableVertexAttribArray(maTempTexCoordHandle);
        // draw
        // matrix
        GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mMVPMtx, 0);
        GLES20.glUniformMatrix4fv(muTexMtxHandle, 1, false,
                (preTexMtx == null) ? GLUtil.createIdentityMtx() : preTexMtx, 0);
        GLES20.glUniformMatrix4fv(muTexRotateMtxHandle, 1, false, texReverseRotateMtx, 0);
        GLES20.glUniform1f(muIsPreviewHandle, (preTexMtx == null) ? 0.0f : 1.0f);
        // sampler
        GLES20.glUniform1i((preTexMtx == null) ? muPictureSampleHandle : muPreviewSamplerHandle, 0);
        GLES20.glUniform1i(muBackTempSamplerHandle, 1);
        // texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture((preTexMtx == null) ? GLES20.GL_TEXTURE_2D
                : GLES11Ext.GL_TEXTURE_EXTERNAL_OES, preTex);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mBackTempTexId);
        // draw
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 6);
        // set mVtxBuf back to big box
        mVtxBuf = createFloatBuffer(mVtxBuf, GLUtil.createTopRightRect(topRect));
        mTopTemplateRenderer.draw(0, 0, 0, getVtxFloatBuffer(), null);
        GLUtil.checkGlError("TopGraphicRenderer draw end");
    }

    public FloatBuffer getVtxFloatBuffer() {
        return mVtxBuf;
    }

    @Override
    public void release() {
        if (mBackTempTexId > 0) {
            releaseBitmapTexture(mBackTempTexId);
            mBackTempTexId = -12345;
        }
        if (mTopTemplateRenderer != null) {
            mTopTemplateRenderer.releaseResource();
            mTopTemplateRenderer = null;
        }
    }

    private void initProgram() {
        mProgram = createProgram(vertexShader, fragmentShader);
        // position
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        maTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        maTempTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTempTexCoord");
        // matrix
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        muTexMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexMtx");
        muTexRotateMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexRotateMtx");
        muIsPreviewHandle = GLES20.glGetUniformLocation(mProgram, "uIsPreview");
        // sampler
        muPreviewSamplerHandle = GLES20.glGetUniformLocation(mProgram, "uPreviewSampler");
        muBackTempSamplerHandle = GLES20.glGetUniformLocation(mProgram, "uBackSampler");
        muPictureSampleHandle = GLES20.glGetUniformLocation(mProgram, "uPictureSampler");
    }

    private void resetMatrix() {
        mMVPMtx = GLUtil.createIdentityMtx();
        mPMtx = GLUtil.createIdentityMtx();
        mVMtx = GLUtil.createIdentityMtx();
        mMMtx = GLUtil.createIdentityMtx();
    }
}
