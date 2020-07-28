/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.ui.preview;

import android.content.Context;
import android.opengl.GLES20;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.ui.preview.GLProducerThread.GLRenderer;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

/**
 * Implementer for GLRender.
 */
class GLRendererImpl implements GLRenderer {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(GLRendererImpl.class.getSimpleName());
    private int mProgramObject;
    private int mWidth;
    private int mHeight;
    private FloatBuffer mVertices;
    private ShortBuffer mTexCoords;
    private final float[] mVerticesData = { -0.5f, -0.5f, 0, 0.5f, -0.5f, 0,
                                                         -0.5f, 0.5f, 0, 0.5f, 0.5f, 0 };
    private final short[] mTexCoordsData = {0, 1, 1, 1, 0, 0, 1, 0};

    /**
     * Constructor for GLRenderImpl.
     * @param context The application context.
     */
    public GLRendererImpl(Context context) {
        mVertices = ByteBuffer.allocateDirect(mVerticesData.length * 4)
                .order(ByteOrder.nativeOrder()).asFloatBuffer();
        mVertices.put(mVerticesData).position(0);
        mTexCoords = ByteBuffer.allocateDirect(mTexCoordsData.length * 2)
                .order(ByteOrder.nativeOrder()).asShortBuffer();
        mTexCoords.put(mTexCoordsData).position(0);
    }

    public void setViewport(int width, int height) {
        mWidth = width;
        mHeight = height;
    }

    public void initGL() {
        compileAndLinkProgram();
        GLES20.glClearColor(0,  0, 0, 0);
    }

    @Override
    public void drawFrame() {
        GLES20.glViewport(0, 0, mWidth, mHeight);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0,  0, 0, 0);
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    private int loadShader(int shaderType, String shaderSource) {
        int shader;
        int[] compiled = new int[1];
        // Create the shader object
        shader = GLES20.glCreateShader(shaderType);
        if (shader == 0) {
            return 0;
        }
        // Load the shader source
        GLES20.glShaderSource(shader, shaderSource);
        // Compile the shader
        GLES20.glCompileShader(shader);
        // Check the compile status
        GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
        if (compiled[0] == 0) {
            LogHelper.e(TAG, GLES20.glGetShaderInfoLog(shader));
            GLES20.glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    private void compileAndLinkProgram() {
        String vShaderStr = "attribute vec4 a_position;    \n"
                + "attribute vec2 a_texCoords; \n"
                + "varying vec2 v_texCoords; \n"
                + "void main()                  \n"
                + "{                            \n"
                + "   gl_Position = a_position;  \n"
                + "    v_texCoords = a_texCoords; \n"
                + "}                            \n";
        String fShaderStr = "precision mediump float;                     \n"
                + "uniform sampler2D u_Texture; \n"
                + "varying vec2 v_texCoords; \n"
                + "void main()                                  \n"
                + "{                                            \n"
                + "  gl_FragColor = texture2D(u_Texture, v_texCoords) ;\n"
                + "}                                            \n";
        int vertexShader;
        int fragmentShader;
        int programObject;
        int[] linked = new int[1];
        // Load the vertex/fragment shaders
        vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vShaderStr);
        fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fShaderStr);
        // Create the program object
        programObject = GLES20.glCreateProgram();
        if (programObject == 0) {
            return;
        }
        GLES20.glAttachShader(programObject, vertexShader);
        GLES20.glAttachShader(programObject, fragmentShader);
        // Bind vPosition to attribute 0
        GLES20.glBindAttribLocation(programObject, 0, "a_position");
        GLES20.glBindAttribLocation(programObject, 1, "a_texCoords");
        // Link the program
        GLES20.glLinkProgram(programObject);
        // Check the link status
        GLES20.glGetProgramiv(programObject, GLES20.GL_LINK_STATUS, linked, 0);
        if (linked[0] == 0) {
            LogHelper.e(TAG, "Error linking program:");
            LogHelper.e(TAG, GLES20.glGetProgramInfoLog(programObject));
            GLES20.glDeleteProgram(programObject);
            return  ;
        }
        mProgramObject = programObject;
    }
}
