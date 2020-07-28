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
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.opengl.EGL14;
import android.opengl.EGLContext;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

import com.mediatek.camera.util.Log;

import java.nio.FloatBuffer;


public class RecorderRenderer extends Renderer implements Runnable {
    private static final String TAG = "RecorderRenderer";

    private FloatBuffer mVtxBuf;
    private FloatBuffer mTexCoordBuf;

    private float[] mPosMtx = GLUtil.createIdentityMtx();
    private float[] mPMtx = GLUtil.createIdentityMtx();
    private float[] mVMtx = GLUtil.createIdentityMtx();
    private float[] mMMtx = GLUtil.createIdentityMtx();
    private float[] mRotateMtx = GLUtil.createIdentityMtx();

    private EglCore mEglCore;
    private Surface mRecordingSurface;
    private WindowSurface mRecordingEGLSurface;

    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTexCoordHandle = -1;
    private int muTexRotateMtxHandle = -1;
    private int muPosMtxHandle = -1;
    private int muSamplerHandle = -1;

    private EncoderHandler mEncoderHandler;
    private EGLContext mSharedEGLContext;
    private Object mReadyFence = new Object(); // guards ready/running
    private boolean mReady;
    private boolean mRunning;
    private boolean mIsCanRecordFirstFrame = false;
    private boolean mIsStopRecrodingReceived = false;
    private int mSwapVideoBufferCount = 0;
    private ConditionVariable mStartConditaionVariable = new ConditionVariable();
    private ConditionVariable mStopConditionVariable = new ConditionVariable();
    private ConditionVariable mRenderThreadBlockVar = new ConditionVariable();

    public RecorderRenderer(Activity activity) {
        super(activity);
        new Thread(this, "PIP-RecorderRenderer").start();
    }

    // should be called in GL Thread
    public void init() {
        Log.d(TAG, "init");
        initGL();
        mSharedEGLContext = EGL14.eglGetCurrentContext();
        mTexCoordBuf = createFloatBuffer(mTexCoordBuf, GLUtil.createTexCoord());
    }
    /**
     * set render size.
     * @param width render width
     * @param height reder height
     * @param orientation the recorder orientation
     */
    public void setRendererSize(int width, int height, int orientation) {
        Log.d(TAG, "setRendererSize width = " + width + " height = " + height);
        if (width == getRendererWidth() && height == getRendererHeight()) {
            return;
        }
        super.setRendererSize(width, height);
        resetMatrix();
        Matrix.orthoM(mPMtx, 0, 0, width, 0, height, -1, 1);
        initVertexData(getRendererWidth(), getRendererHeight(), orientation);
    }

    public void setRecrodingSurface(Surface surface) {
        Log.d(TAG, "setRecrodingSurface surface = " + surface);
        if (surface == null) {
            throw new NullPointerException("setRecrodingSurface, but surface is null!!!");
        }
        mRecordingSurface = surface;
        synchronized (mReadyFence) {
            if (mRunning) {
                Log.w(TAG, "Encoder thread already running");
                return;
            }
            mRunning = true;
            while (!mReady) {
                try {
                    Log.d(TAG, "wait for recording thread ready, current mReady = " + mReady);
                    mReadyFence.wait();
                } catch (InterruptedException ie) {
                    // ignore
                }
            }
        }
        mEncoderHandler.obtainMessage(EncoderHandler.MSG_UPDATE_SHARED_CONTEXT).sendToTarget();
    }

    /**
     * Tells the video recorder to start recording. (Call from UI thread.)
     * <p>
     * Returns after the recorder thread has started and is ready to accept
     * Messages. The encoder may not yet be fully configured.
     */
    public void startRecording() {
        Log.d(TAG, "startRecording begin mEncoderHandler = " + mEncoderHandler);
        synchronized (mReadyFence) {
            // delay 200ms is for not record start voice to video
            mStartConditaionVariable.block(200);
            if (mEncoderHandler != null) {
            mEncoderHandler.sendMessage(
                    mEncoderHandler.obtainMessage(EncoderHandler.MSG_START_RECORDING));
            }
        }
        Log.d(TAG, "startRecording end");
    }

    /**
     * when new frame is available, swap buffer to mediacodec (Call from GL
     * thread.)
     */
    public void draw(int textureId, long timestamp) {
        synchronized (mReadyFence) {
            if (!mReady || !mIsCanRecordFirstFrame) {
                Log.w(TAG, "mReady = " + mReady + " mIsCanRecordFirstFrame = "
                        + mIsCanRecordFirstFrame);
                return;
            }
            long presentationTimeNs = timestamp;
            if (mEncoderHandler != null) {
                mRenderThreadBlockVar.close();
                mEncoderHandler.obtainMessage(EncoderHandler.MSG_FRAME_AVAILABLE,
                        (int) (presentationTimeNs >> 32), (int) presentationTimeNs, textureId)
                        .sendToTarget();
                mRenderThreadBlockVar.block();
                if (mIsStopRecrodingReceived && (mSwapVideoBufferCount > 1)) {
                    mReady = false; // when stop received, do not receive frame next time
                    mIsStopRecrodingReceived = false;
                    mEncoderHandler.sendEmptyMessage(EncoderHandler.MSG_STOP_RECORDING);
                    mEncoderHandler.sendEmptyMessage(EncoderHandler.MSG_QUIT);
                    mStopConditionVariable.open();
                }
            }
        }
    }

    public void pauseVideoRecording() {
        Log.d(TAG, "pauseVideoRecording");
        synchronized (mReadyFence) {
            mReady = false;
        }
    }

    public void resumeVideoRecording() {
        Log.d(TAG, "resumeVideoRecording has frame available message = "
                        + mEncoderHandler.hasMessages(EncoderHandler.MSG_FRAME_AVAILABLE));
        synchronized (mReadyFence) {
            mReady = true;
        }
    }

    /**
     * Tells the video recorder to stop recording. (Call from UI.) Really stop
     * signal will occur at GL Thread (see details at draw())
     */
    public void stopRecording() {
        Log.d(TAG, "stopRecording mReady = " + mReady + " mIsStopRecrodingReceived = "
                + mIsStopRecrodingReceived);
        mStopConditionVariable.close();
        synchronized (mReadyFence) {
            // make mReady to true to ensure receive a frame
            mReady = true;
            mIsStopRecrodingReceived = true;
        }
        mStopConditionVariable.block(3000);
    }

    /**
     * Encoder thread entry point. Establishes Looper/Handler and waits for
     * messages.
     */
    @Override
    public void run() {
        Log.d(TAG, "run");
        // Establish a Looper for this thread, and define a Handler for it.
        synchronized (mReadyFence) {
            Looper.prepare();
            Log.d(TAG, "new EncoderHandler()");
            mEncoderHandler = new EncoderHandler();
            mReady = true;
            mReadyFence.notify();
        }
        Looper.loop();

        Log.d(TAG, "Encoder thread exiting");
        synchronized (mReadyFence) {
            mReady = mRunning = false;
            mEncoderHandler = null;
        }
    }

    public void releaseSurface() {
        Log.d(TAG, "releaseSurface");
    }

    private void initGL() {
        GLUtil.checkGlError("initGL_Start");
        if (mProgram != -1) {
            return;
        }
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
        muTexRotateMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexRotateMtx");
        // matrix
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        // sampler
        muSamplerHandle = GLES20.glGetUniformLocation(mProgram, "uSampler");

        GLES20.glDisable(GLES20.GL_DEPTH_TEST);
        GLES20.glDisable(GLES20.GL_CULL_FACE);
        GLES20.glDisable(GLES20.GL_BLEND);

        GLUtil.checkGlError("initGL_E");
    }

    private void initVertexData(float width, float height , int orientation) {
        mVtxBuf = createFloatBuffer(mVtxBuf, GLUtil.createFullSquareVtx(width, height));
        Matrix.multiplyMM(mPosMtx, 0, mMMtx, 0, mVMtx, 0);
        Matrix.multiplyMM(mPosMtx, 0, mPMtx, 0, mPosMtx, 0);
        // force swap landscape buffer to MediaCodec
        if (width < height) {
            if (orientation % 180 == 0) {
                orientation += 90;
            }
            android.opengl.Matrix.translateM(mRotateMtx, 0, mRotateMtx, 0, .5f, .5f, 0);
            android.opengl.Matrix.rotateM(mRotateMtx, 0, -orientation, 0, 0, 1);
            android.opengl.Matrix.translateM(mRotateMtx, 0, -.5f, -.5f, 0);
        }
    }

    private void resetMatrix() {
        mPosMtx = GLUtil.createIdentityMtx();
        mPMtx = GLUtil.createIdentityMtx();
        mVMtx = GLUtil.createIdentityMtx();
        mMMtx = GLUtil.createIdentityMtx();
        mRotateMtx = GLUtil.createIdentityMtx();
    }

    private class EncoderHandler extends Handler {
        private static final int MSG_START_RECORDING = 0;
        private static final int MSG_STOP_RECORDING = 1;
        private static final int MSG_FRAME_AVAILABLE = 2;
        private static final int MSG_SET_TEXTURE_ID = 3;
        private static final int MSG_UPDATE_SHARED_CONTEXT = 4;
        private static final int MSG_QUIT = 5;

        public EncoderHandler() {

        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage(" + msg + ")");
            switch (msg.what) {
            case MSG_START_RECORDING:
                mIsCanRecordFirstFrame = true;
                break;
            case MSG_STOP_RECORDING:
                break;
            case MSG_FRAME_AVAILABLE:
                int textureId = (Integer) msg.obj;
                long timestamp = (((long) msg.arg1) << 32) | (((long) msg.arg2) & 0xffffffffL);
                doDraw(textureId, timestamp);
                break;
            case MSG_SET_TEXTURE_ID:
                break;
            case MSG_UPDATE_SHARED_CONTEXT:
                doUpdateSharedContext();
                doStartVideoRecording(mRecordingSurface);
                break;
            case MSG_QUIT:
                if (mRecordingEGLSurface != null) {
                    mRecordingEGLSurface.makeNothingCurrent();
                    mRecordingEGLSurface.release();
                    mRecordingEGLSurface = null;
                    Log.d(TAG, "RecorderRendere swap buffer total count : "
                                + mSwapVideoBufferCount);
                }
                if (mEglCore != null) {
                    mEglCore.release();
                    mEglCore = null;
                }
                Looper looper = Looper.myLooper();
                if (looper != null) {
                    looper.quit();
                }
                break;
            }
        }

        private void doUpdateSharedContext() {
            Log.d(TAG, "doUpdateSharedContext");
            mEglCore = new EglCore(mSharedEGLContext,
                    EglCore.FLAG_TRY_GLES3 | EglCore.FLAG_RECORDABLE,
                    new int[]{PixelFormat.RGB_888, PixelFormat.RGBA_8888, ImageFormat.YV12});
        }

        private void doStartVideoRecording(Surface surface) {
            Log.d(TAG, "doStartVideoRecording");
            mRecordingEGLSurface = new WindowSurface(mEglCore, surface);
            mRecordingEGLSurface.makeCurrent();
        }

        private void doDraw(int textureId, long timestamp) {
            if (mRecordingEGLSurface == null) {
                mRenderThreadBlockVar.open();
                return;
            }
            GLUtil.checkGlError("RecordDraw_Start");
            GLES20.glClearColor(0f, 0f, 0f, 1f);
            GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
            // use program
            GLES20.glUseProgram(mProgram);
            // vertex
            mVtxBuf.position(0);
            GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false, 4 * 3,
                    mVtxBuf);
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
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
            // draw
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 6);
            // swap buffer
            mRecordingEGLSurface.setPresentationTime(timestamp);
            mSwapVideoBufferCount++;
            mRenderThreadBlockVar.open();
            mRecordingEGLSurface.swapBuffers();
            GLUtil.checkGlError("RecordDraw_End");
            debugFrameRate(TAG);
        }
    }
}
