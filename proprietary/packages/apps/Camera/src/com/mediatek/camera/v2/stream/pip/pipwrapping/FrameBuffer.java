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

import android.graphics.PixelFormat;
import android.opengl.GLES20;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

/**
 * An off-screen window frame buffer binded to a texture.
 */
public class FrameBuffer {
    private static final Tag TAG = new Tag(FrameBuffer.class.getSimpleName());
    private final int[] mFboId = new int[] { 0 };
    private final int[] mFboTexId = new int[] { 0 };

    public FrameBuffer() {
    }

    public void init() {
        generateFbo();
    }

    public void setRendererSize(int width, int height) {
        LogHelper.d(TAG, "initFBO width = " + width + " height = " + height);
        if (width <= 0 || height <= 0) {
            return;
        }
        GLUtil.checkGlError("initFBO start");
        // bind fbo
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFboId[0]);
        // bind texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mFboTexId[0]);
        // set parameters to texture
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        // This is necessary to work with user-generated frame buffers with
        // dimensions that are NOT powers of 2.
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_CLAMP_TO_EDGE);
        // texImage
        if (PipEGLConfigWrapper.getInstance().getPixelFormat() == PixelFormat.RGBA_8888) {
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, width, height, 0,
                    GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
        } else if (PipEGLConfigWrapper.getInstance().getPixelFormat() == PixelFormat.RGB_565) {
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGB, width, height, 0,
                    GLES20.GL_RGB, GLES20.GL_UNSIGNED_SHORT_5_6_5, null);
        }
        // bind texture to fbo
        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0,
                GLES20.GL_TEXTURE_2D, mFboTexId[0], 0);

        if (GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER)
                != GLES20.GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("glCheckFramebufferStatus() "
                    + GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER));
        }
        GLUtil.checkGlError("initFBO end");
    }

    public int getFboTexId() {
        return mFboTexId[0];
    }

    public void setupFrameBufferGraphics(int width, int height) {
        GLUtil.checkGlError("[setupFrameBufferGraphics]+");
        LogHelper.v(TAG, "setupFrameBufferGraphics width = " + width + " height = " + height);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFboId[0]);
        GLUtil.checkGlError("[setupFrameBufferGraphics]+ glBindFramebuffer");
        GLES20.glViewport(0, 0, width, height);
        GLUtil.checkGlError("[setupFrameBufferGraphics]+ glViewport");
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        GLUtil.checkGlError("[setupFrameBufferGraphics] glClearColor");
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
        GLUtil.checkGlError("[setupFrameBufferGraphics]" +
                " glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT)");
        // enable blend, in order to get a transparent background
        GLES20.glEnable(GLES20.GL_BLEND);
        GLUtil.checkGlError("[setupFrameBufferGraphics] glEnable");
        GLES20.glBlendFunc(GLES20.GL_ONE, GLES20.GL_ONE_MINUS_SRC_ALPHA);
        GLUtil.checkGlError("[setupFrameBufferGraphics]-");
        GLUtil.checkEglError("setupFrameBufferGraphics");
    }

    public void setScreenBufferGraphics() {
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
    }

    public void unInit() {
        LogHelper.d(TAG, "[unInit]+");
        deleteFBO();
        LogHelper.d(TAG, "[unInit]-");
    }

    private void generateFbo() {
        // generate fbo,rbo,texture id
        GLES20.glGenFramebuffers(1, mFboId, 0);
        GLES20.glGenTextures(1, mFboTexId, 0);
        LogHelper.d(TAG, "FrameBuffer glGenTextures texture num = 1");
    }

    private void deleteFBO() {
        GLES20.glDeleteFramebuffers(1, mFboId, 0);
        GLES20.glDeleteTextures(1, mFboTexId, 0);
    }
}
