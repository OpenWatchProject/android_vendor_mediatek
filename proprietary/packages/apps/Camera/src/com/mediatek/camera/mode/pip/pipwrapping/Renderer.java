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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.Color;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.view.Surface;

import com.mediatek.camera.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * Almost all Renderer's methods should be called in GL Thread.
 */
public class Renderer {
    private static final String      TAG = "Renderer";
    private static final int         INTERVALS = 300;
    private int                      mRendererWidth = 0;
    private int                      mRendererHeight = 0;
    private Activity                 mActivity;
    private CropBox                  mCropBox = new CropBox();
    // M: debug info for draw preview.
    private int                      mDrawFrameCount = 0;
    private long                     mDrawStartTime = 0;

    public Renderer(Activity activity) {
        mActivity = activity;
    }

    protected FloatBuffer createFloatBuffer(FloatBuffer floatBuffer, float[] texCoor) {
        if (floatBuffer == null) {
            Log.v(TAG, "ByteBuffer.allocateDirect");
            floatBuffer = ByteBuffer.allocateDirect(texCoor.length * 4)
                    .order(ByteOrder.nativeOrder()).asFloatBuffer();
        }
        floatBuffer.clear();
        floatBuffer.put(texCoor);
        floatBuffer.position(0);
        return floatBuffer;
    }

    // when do take picture, should reverse coordinate system align y
    protected void setRendererSize(int width, int height, boolean needReverse) {
        mRendererWidth = width;
        mRendererHeight = height;
    }

    // default is no need to reverse coordinate system
    protected void setRendererSize(int width, int height) {
        mRendererWidth = width;
        mRendererHeight = height;
    }

    protected void setSurface(Surface surface, boolean scaled, boolean rotated) {

    }

    /**
     * Reload pip template texture.
     *
     * @param resourceId
     *            : template resource id
     * @return texture id
     * @throws IOException
     */
    protected int initBitmapTexture(int resourceId, boolean needCropSmallBox) throws IOException {
        int[] textures = new int[1];
        Log.d(TAG, "Renderer initBitmapTexture glGenTextures num = " + 1);
        GLES20.glGenTextures(1, textures, 0);
        int textureId = textures[0];
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLUtil.checkGlError("initBitmapTexture GL_TEXTURE_MAG_FILTER");
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_LINEAR_MIPMAP_NEAREST);
        GLUtil.checkGlError("initBitmapTexture GL_TEXTURE_MIN_FILTER");
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_CLAMP_TO_EDGE);
        // load bitmap begin
        InputStream is = mActivity.getResources().openRawResource(resourceId);
        Bitmap bitmapTmp;
        BitmapFactory.Options options = new Options();
        options.inScaled = false;
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        try {
            bitmapTmp = BitmapFactory.decodeStream(is, null, options);
            bitmapTmp.setHasAlpha(false);
            if (needCropSmallBox) {
                int width = bitmapTmp.getWidth();
                int heiht = bitmapTmp.getHeight();
                int minX = width;
                int maxX = 0;
                int minY = heiht;
                int maxY = 0;
                // black is big box, white is small box
                int black = Color.rgb(0, 0, 0);
                long begin = System.currentTimeMillis();
                for (int i = 0; i < width; i++) {
                    for (int j = 0; j < heiht; j++) {
                        int readRgb = bitmapTmp.getPixel(i, j);
                        if (readRgb != black) {
                            if (minX > i) {
                                minX = i;
                            }
                            if (maxX < i) {
                                maxX = i;
                            }
                            if (minY > j) {
                                minY = j;
                            }
                            if (maxY < j) {
                                maxY = j;
                            }
                        }
                    }
                }
                Log.v(TAG, "read cousume " + (System.currentTimeMillis() - begin)
                        + "ms bitmap width = " + width + " bitmap height = " + heiht + " minX = "
                        + minX + " minY = " + minY + " maxX = " + maxX + " maxY = " + maxY);
                int edge = Math.max(maxX - minX, maxY - minY);
                float xRatio = (minX + (float) edge / 2 - (float) width / 2) / (float) width;
                float yRatio = (minY + (float) edge / 2 - (float) heiht / 2) / (float) heiht;
                float scaleRatio = (float) edge / width;
                mCropBox.setTranslateXRatio(xRatio);
                mCropBox.setTranslateYRatio(yRatio);
                mCropBox.setScaleRatio(scaleRatio);
                // crop small box
                bitmapTmp = Bitmap.createBitmap(bitmapTmp, minX, minY, edge, edge);
                Log.v(TAG,
                        "new bitmap width = " + bitmapTmp.getWidth() + " height = "
                                + bitmapTmp.getHeight() + " translateXRatio = " + xRatio
                                + " translateYRatio = " + yRatio + " scaleRatio = " + scaleRatio);
            }
        } finally {
            try {
                is.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmapTmp, 0);
        bitmapTmp.recycle();
        bitmapTmp = null;
        GLES20.glGenerateMipmap(GLES20.GL_TEXTURE_2D);
        return textureId;
    }

    protected void releaseBitmapTexture(int textureId) {
        int[] textures = new int[] { textureId };
        GLES20.glDeleteTextures(1, textures, 0);
        Log.d(TAG, "Renderer releaseBitmapTexture glDeleteTextures num = " + 1);
    }

    public Activity getActivity() {
        return mActivity;
    }

    public int getRendererWidth() {
        return mRendererWidth;
    }

    public int getRendererHeight() {
        return mRendererHeight;
    }

    public int createProgram(String vertexSource, String fragmentSource) {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        int program = GLES20.glCreateProgram();
        GLES20.glAttachShader(program, vertexShader);
        GLES20.glAttachShader(program, fragmentShader);
        GLES20.glLinkProgram(program);

        int[] linkStatus = new int[1];
        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
        if (linkStatus[0] != GLES20.GL_TRUE) {
            Log.e(TAG, "Could not link program:");
            Log.e(TAG, GLES20.glGetProgramInfoLog(program));
            GLES20.glDeleteProgram(program);
            program = 0;
        }
        return program;
    }

    public int loadShader(int shaderType, String source) {
        int shader = GLES20.glCreateShader(shaderType);
        GLES20.glShaderSource(shader, source);
        GLES20.glCompileShader(shader);

        int[] compiled = new int[1];
        GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
        if (compiled[0] == 0) {
            Log.e(TAG, "Could not compile shader(TYPE=" + shaderType + "):");
            Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
            GLES20.glDeleteShader(shader);
            shader = 0;
        }

        return shader;
    }

    public void debugFrameRate(String tag) {
        mDrawFrameCount++;
        if (mDrawFrameCount % INTERVALS == 0) {
            long currentTime = System.currentTimeMillis();
            int intervals = (int) (currentTime - mDrawStartTime);
            Log.d(tag, "[Wrapping-->" + tag + "]" + "[Preview] Drawing frame, fps = "
                    + (mDrawFrameCount * 1000.0f) / intervals + " in last " + intervals
                    + " millisecond.");
            mDrawStartTime = currentTime;
            mDrawFrameCount = 0;
        }
    }

    public CropBox getCropBox() {
        return mCropBox;
    }

    class CropBox {
        private float mTranslateXRatio = .0f;
        private float mTranslateYRatio = .0f;
        private float mScaleRatio = 1.0f;

        private CropBox() {
        }

        public CropBox(float xRatio, float yRatio, float scaleRatio) {
            mTranslateXRatio = xRatio;
            mTranslateYRatio = yRatio;
            mScaleRatio = scaleRatio;
        }

        public void setTranslateXRatio(float xRatio) {
            mTranslateXRatio = xRatio;
        }

        public float getTranslateXRatio() {
            return mTranslateXRatio;
        }

        public void setTranslateYRatio(float yRatio) {
            mTranslateYRatio = yRatio;
        }

        public float getTranslateYRatio() {
            return mTranslateYRatio;
        }

        public void setScaleRatio(float scaleRatio) {
            mScaleRatio = scaleRatio;
        }

        public float getScaleRatio() {
            return mScaleRatio;
        }
    }

    public void release() {

    }
}
