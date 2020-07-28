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
package com.mediatek.camera.feature.setting.matrixdisplay;

import android.graphics.ImageFormat;
import android.graphics.Point;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.view.Surface;

import com.mediatek.camera.common.debug.CameraSysTrace;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.matrixdisplay.ext.MatrixDisplayExt;
import com.mediatek.camera.matrixdisplay.ext.MatrixDisplayExt.EffectAvailableCallback;

import java.util.ArrayList;

/**
 * This class is used to process preview frame with native.
 */

public class MatrixDisplayHandler implements IPreviewFrameCallback,
        MatrixDisplayViewManager.SurfaceAvailableListener,
        MatrixDisplayViewManager.EffectUpdateListener {
    private final static LogUtil.Tag TAG = new
            LogUtil.Tag(MatrixDisplayHandler.class.getSimpleName());

    private final static int MSG_INIT_EFFECT = 100;
    private final static int MSG_REGISTER_BUFFERS = 101;
    private final static int MSG_SET_SURFACE_NATIVE = 102;
    private final static int MSG_PROCESS_EFFECT = 103;
    private final static int MSG_RELEASE_EFFECT = 104;

    private final static int MATRIX_FORMAT = 11;

    private final static int EFFECT_NUM_OF_PAGE = 12;
    private final static int PAGE_NUM = 3;
    private final static int CACHE_BUFFER_NUM = 3;
    private final static int BIT_NUM_PER_BYTE = 8;
    private static final int NUM_OF_DROP = 6;
    private static final int MAX_NUM_OF_PROCESSING = 2;
    private static final int PRINT_FPS_EVERY_FEW_FRAMES = 20;
    private static final int WIDTH_BE_DIVIDED_BY = 32;
    private static final int HEIGHT_BE_DIVIDED_BY = 16;

    private int mMaxSurfaceBufferWidth;
    private int mMaxSurfaceBufferHeight;

    private int[] mEffectIndexs = new int[EFFECT_NUM_OF_PAGE];
    private Surface[] mSurfaceArray = new Surface[EFFECT_NUM_OF_PAGE];
    private byte[][] mEffectsBuffers = new byte[EFFECT_NUM_OF_PAGE * PAGE_NUM][];
    private ArrayList<byte[]> mCacheBuffer = new ArrayList<byte[]>();

    private ConditionVariable mReleaseCondition = new ConditionVariable();
    private EffectAvailableListener mEffectAvailableListener;
    private MatrixDisplayExt mMatrixDisplayExt;
    private volatile boolean mIsReleased = false;
    private boolean mHasRegisterBuffer = false;
    private int mNumOfCurrentProcess = 0;
    private int mCacheIndex = 0;
    private int mInputFrames = 0;
    private long mInputStartTime;
    private int mNumOfDropFrame = NUM_OF_DROP;
    private Handler mHandler;

    /**
     * Used to notify effect available event.
     */
    public interface EffectAvailableListener {
        /**
         * Notify effect available event.
         */
        public void onEffectAvailable();
    }

    @Override
    public void onPreviewFrameAvailable(byte[] data) {
        LogHelper.d(TAG, "[onPreviewFrameAvailable] pv callback data:" + data.length
                + ", mNumOfCurrentProcess:" + mNumOfCurrentProcess);
        if (mInputFrames == 0) {
            mInputStartTime = System.currentTimeMillis();
        }
        mInputFrames++;
        if (mInputFrames % PRINT_FPS_EVERY_FEW_FRAMES == 0) {
            long duration = System.currentTimeMillis() - mInputStartTime;
            LogHelper.d(TAG, "[onPreviewFrameAvailable] pv callback fps:" +
                    (PRINT_FPS_EVERY_FEW_FRAMES * 1000) / duration);
            mInputStartTime = System.currentTimeMillis();
        }

        if (mNumOfCurrentProcess == MAX_NUM_OF_PROCESSING) {
            if (mHandler != null) {
                mHandler.removeMessages(MSG_PROCESS_EFFECT);
                mNumOfCurrentProcess = 1;
            }
        }

        if (mNumOfCurrentProcess < MAX_NUM_OF_PROCESSING
                && mNumOfDropFrame >= NUM_OF_DROP) {
            if (mHasRegisterBuffer) {
                processEffect(data);
            }
        } else if (mNumOfDropFrame < NUM_OF_DROP) {
            mNumOfDropFrame++;
        }
    }

    @Override
    public void onEffectUpdated(int position, int effectIndex) {
        mEffectIndexs[position] = effectIndex;
    }

    @Override
    public void onSurfaceAvailable(Surface surface, int width, int height, int position) {
        mSurfaceArray[position] = surface;
        if (!mHasRegisterBuffer) {
            Point point = updateSurfaceSize(width, height);
            int bufferWidth = point.x;
            int bufferHeight = point.y;
            int bufferSize = bufferWidth * bufferHeight * 3 / 2;
            for (int i = 0; i < EFFECT_NUM_OF_PAGE * PAGE_NUM; i++) {
                if (mEffectsBuffers[i] == null) {
                    mEffectsBuffers[i] = new byte[bufferSize];
                }
            }
            LogHelper.d(TAG, "[onSurfaceAvailable] register buffer size, bufferWidth:" +
                    bufferWidth + ",bufferHeight:" + bufferHeight);
            if (mHandler != null) {
                mHandler.obtainMessage(MSG_REGISTER_BUFFERS, bufferWidth, bufferHeight)
                        .sendToTarget();
                mHasRegisterBuffer = true;
            }
        }
        if (mHandler != null) {
            mHandler.obtainMessage(MSG_SET_SURFACE_NATIVE, position, 0, surface).sendToTarget();
        }
    }

    /**
     * Initialize matrix display.
     *
     * @param previewWidth  The width of preview buffer.
     * @param previewHeight The height of preview buffer.
     * @param imageFormat   image format.
     * @param layoutWidth   The width of layout
     * @param layoutHeight  The height of layout
     */
    public void initialize(int previewWidth, int previewHeight, int imageFormat,
                           int layoutWidth, int layoutHeight) {
        LogHelper.d(TAG, "[initialize]previewWidth:" + previewWidth
                + ", previewHeight" + previewHeight + ",imageFormat:" + imageFormat);
        // using layoutWidth/4 and layoutHeight/4 as the max width and height
        mMaxSurfaceBufferWidth = Math.max(layoutWidth, layoutHeight) / 4;
        mMaxSurfaceBufferHeight = Math.min(layoutWidth, layoutHeight) / 4;
        mMatrixDisplayExt = MatrixDisplayExt.getInstance();
        if (mHandler == null) {
            HandlerThread ht = new HandlerThread("draw buffer handler thread",
                    Process.THREAD_PRIORITY_DISPLAY);
            ht.start();
            mHandler = new EffectHandler(ht.getLooper());
        }

        for (int i = 0; i < mEffectIndexs.length; i++) {
            mEffectIndexs[i] = -1;
        }

        int bufferSize = previewWidth * previewHeight *
                ImageFormat.getBitsPerPixel(imageFormat) / BIT_NUM_PER_BYTE;
        if (mCacheBuffer.size() == 0) {
            for (int i = 0; i < CACHE_BUFFER_NUM; i++) {
                byte[] cacheBuffer = new byte[bufferSize];
                mCacheBuffer.add(cacheBuffer);
            }
        }
        mCacheIndex = 0;
        mNumOfDropFrame = 0;
        mNumOfCurrentProcess = 0;
        mMatrixDisplayExt.setCallback(mEffectsCallback);
        mHandler.obtainMessage(MSG_INIT_EFFECT, previewWidth, previewHeight).sendToTarget();
    }

    /**
     * Release matrix display.
     */
    public void release() {
        if (mIsReleased) {
            return;
        }
        if (mMatrixDisplayExt != null) {
            mMatrixDisplayExt.setCallback(null);
        }
        LogHelper.d(TAG, "[release] mHandler:" + mHandler);
        if (mHandler != null) {
            mHandler.sendEmptyMessage(MSG_RELEASE_EFFECT);
            mReleaseCondition.block();
            mReleaseCondition.close();
            mHandler.getLooper().quit();
            mHandler = null;
        }
        mMatrixDisplayExt = null;
        LogHelper.d(TAG, "[release] end");
    }

    /**
     * Set effect available listener.
     *
     * @param listener effect available listener.
     */
    public void setEffectAvailableListener(EffectAvailableListener listener) {
        mEffectAvailableListener = listener;
    }

    private Point updateSurfaceSize(final int width, final int height) {
        LogHelper.d(TAG, "[updateSurfaceSize] input size, width = " + width
                + ", height = " + height
                + ", mMaxSurfaceBufferWidth = " + mMaxSurfaceBufferWidth
                + ", mMaxSurfaceBufferHeight = " + mMaxSurfaceBufferHeight);
        int newWidth = width;
        int newHeight = height;
        final float step = 0.1f; // The step to reduce width/height gradually
        float count = 1.f + step;
        while (newWidth > mMaxSurfaceBufferWidth
                || newHeight > mMaxSurfaceBufferHeight) {
            newWidth = (int) ((float) width / count);
            newHeight = (int) ((float) height / count);
            count += step;
        }
        newWidth = newWidth / WIDTH_BE_DIVIDED_BY * WIDTH_BE_DIVIDED_BY;
        newHeight = newHeight / HEIGHT_BE_DIVIDED_BY * HEIGHT_BE_DIVIDED_BY;
        Point point = new Point(newWidth, newHeight);
        LogHelper.d(TAG, "[updateSurfaceSize] output size,newWidth:" + newWidth
                + ",newHeight:" + newHeight);
        return point;
    }

    private void processEffect(byte[] data) {
        if (data == null) {
            LogHelper.w(TAG, "[processEffect] data is null, return");
            return;
        }

        byte[] cacheBuf = mCacheBuffer.get(mCacheIndex);
        if (data.length != cacheBuf.length) {
            LogHelper.d(TAG, "[processEffect]preview buffer size is larger,return!");
            return;
        }
        System.arraycopy(data, 0, cacheBuf, 0, data.length);
        mCacheIndex = (mCacheIndex + 1) % CACHE_BUFFER_NUM;
        mNumOfCurrentProcess++;
        if (mHandler != null) {
            Message msg = mHandler.obtainMessage(MSG_PROCESS_EFFECT, mCacheIndex, 0, cacheBuf);
            mHandler.sendMessage(msg);
        }
    }

    private void releaseMatrixDisplay() {
        LogHelper.d(TAG, "<releaseMatrixDisplay>");

        if (!mIsReleased) {
            mIsReleased = true;
            mHasRegisterBuffer = false;
            mMatrixDisplayExt.release();

            for (int i = 0; i < mEffectsBuffers.length; i++) {
                mEffectsBuffers[i] = null;
            }
            for (int i = 0; i < mSurfaceArray.length; i++) {
                mSurfaceArray[i] = null;
            }
            mCacheBuffer.clear();
        }
        mReleaseCondition.open();
    }

    /**
     * Interact with JNI with different thread.
     */
    private class EffectHandler extends Handler {

        /**
         * Constructor, initialize looper to handler.
         *
         * @param looper handler looper
         */
        public EffectHandler(Looper looper) {
            super(looper);
        }


        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "<handleMessage> msg.what:" + msg.what
                    + ",mIsReleased:" + mIsReleased);

            switch (msg.what) {
                case MSG_INIT_EFFECT:
                    LogHelper.d(TAG, "<handleMessage> previewWidth:" + msg.arg1
                            + ",previewHeight:" + msg.arg2);
                    mMatrixDisplayExt.initialize((int) msg.arg1, (int) msg.arg2,
                            EFFECT_NUM_OF_PAGE, MATRIX_FORMAT);
                    mIsReleased = false;
                    break;

                case MSG_REGISTER_BUFFERS:
                    if (mIsReleased) {
                        return;
                    }
                    mMatrixDisplayExt.setBuffers(msg.arg1, msg.arg2, mEffectsBuffers);
                    break;

                case MSG_SET_SURFACE_NATIVE:
                    if (mIsReleased) {
                        return;
                    }
                    mMatrixDisplayExt.setSurface((Surface) msg.obj, msg.arg1);
                    break;

                case MSG_PROCESS_EFFECT:
                    if (mIsReleased) {
                        return;
                    }
                    long beginTime = System.currentTimeMillis();
                    CameraSysTrace.onEventSystrace("process frame", true);
                    mMatrixDisplayExt.process((byte[]) msg.obj, mEffectIndexs);
                    mNumOfCurrentProcess--;
                    CameraSysTrace.onEventSystrace("process frame", false);
                    long endTime = System.currentTimeMillis();
                    LogHelper.d(TAG, "process time:" + (endTime - beginTime));
                    break;

                case MSG_RELEASE_EFFECT:
                    releaseMatrixDisplay();
                    break;

                default:
                    LogHelper.d(TAG, "<handleMessage>unrecognized message!");
                    break;
            }
        }

    }

    private EffectAvailableCallback mEffectsCallback = new EffectAvailableCallback() {
        @Override
        public void onEffectAvailable() {
            LogHelper.d(TAG, "[onEffectAvailable]");
            mEffectAvailableListener.onEffectAvailable();
        }
    };

}
