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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.camera.common.jpeg.encoder;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;
import android.view.Surface;

import java.nio.ByteBuffer;

/**
 * Hw jpeg encode impl.
 */
class HwJpegEncodeImpl extends JpegEncoder {
    private static final String TAG = HwJpegEncodeImpl.class.getSimpleName();

    private JpegCallback mJpegCallback;
    private ImageReader mImageReader;
    private HandlerThread mImageHandlerThread;

    HwJpegEncodeImpl(Context context) {
    }

    @Override
    public int[] getSupportedInputFormats() {
        int[] supportedFormats = new int[3];
        supportedFormats[0] = PixelFormat.RGBA_8888; // 1
        supportedFormats[1] = ImageFormat.YV12; // 0x32315659
        supportedFormats[2] = PixelFormat.RGB_888; // 3
        return supportedFormats;
    }

    @Override
    public Surface configInputSurface(JpegCallback jpegCallback, int width,
            int height, int format) {
        Log.d(TAG, "[configInputSurface] jpegCallback:" + jpegCallback + ",width:"
                + width + ",height:" + height + ",format:" + format);
        mJpegCallback = jpegCallback;
        if (mImageReader != null
                && mImageReader.getWidth() == width
                && mImageReader.getHeight() == height
                && mImageReader.getImageFormat() == format) {
            Log.i(TAG, "reuse old image reader width = " + width + " height = " + height);
            return mImageReader.getSurface();
        }
        if (mImageReader != null) {
            mImageReader.close();
        }
        if (mImageHandlerThread == null) {
            mImageHandlerThread = new HandlerThread("ImageListener");
            mImageHandlerThread.start();
        }
        Looper looper = mImageHandlerThread.getLooper();
        if (looper == null) {
            throw new RuntimeException("why looper is null ?");
        }
        mImageReader = ImageReader.newInstance(width, height, format, 2);
        mImageReader.setOnImageAvailableListener(new ImageListener(), new Handler(looper));
        return mImageReader.getSurface();
    }

    @Override
    public void startEncode() {
    }

    @Override
    public void startEncodeAndReleaseWhenDown() {
    }

    @Override
    public void release() {
        Log.i(TAG, "release");
        close();
    }

    private synchronized void close() {
        Log.i(TAG, "close");
        if (mImageReader != null) {
            mImageReader.close();
            mImageReader = null;
            if (mImageHandlerThread.isAlive()) {
                mImageHandlerThread.quit();
                mImageHandlerThread = null;
            }
        }
        if (mImageHandlerThread != null && mImageHandlerThread.isAlive()) {
            mImageHandlerThread.quit();
            mImageHandlerThread = null;
        }
    }

    /**
     * Image reader listener, get image from image reader.
     *
     */
    private class ImageListener implements ImageReader.OnImageAvailableListener {

        public void onImageAvailable(ImageReader reader) {
            Log.d(TAG, "[onImageAvailable] reader:" + reader);
            Image image = reader.acquireNextImage();
            byte[] jpegData = acquireJpegBytesAndClose(image);
            mJpegCallback.onJpegAvailable(jpegData);
            Log.d(TAG, "[onImageAvailable] end");
        }
    }

    private byte[] acquireJpegBytesAndClose(Image image) {
        ByteBuffer buffer;
        Image.Plane plane0 = image.getPlanes()[0];
        buffer = plane0.getBuffer();
        Log.d(TAG, "<acquireJpegBytesAndClose> start get buffer,size:" + buffer.remaining());
        byte[] imageBytes = new byte[buffer.remaining()];
        buffer.get(imageBytes);
        buffer.rewind();
        image.close();
        return imageBytes;
    }
}