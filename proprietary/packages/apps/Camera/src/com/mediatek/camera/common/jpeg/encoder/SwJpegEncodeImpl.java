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

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.PixelFormat;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;
import android.view.Surface;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Sw jpeg encoder impl.
 */
@TargetApi(Build.VERSION_CODES.KITKAT)
class SwJpegEncodeImpl extends JpegEncoder {
    private static final String TAG = "CamAp_" + SwJpegEncodeImpl.class.getSimpleName();
    private ImageReader mImageReader;
    private HandlerThread mImageHandlerThread;
    private JpegCallback mJpegCallback;
    private boolean mCloseWhenEncodeDone;

    /**
     * Sw jpeg encoder implementation.
     */
    public SwJpegEncodeImpl() {

    }

    @Override
    public int[] getSupportedInputFormats() {
        int[] supportedFormats = new int[1];
        supportedFormats[0] = PixelFormat.RGBA_8888;
        return supportedFormats;
    }

    @Override
    public Surface configInputSurface(Surface outputSurface, int width,
            int height, int format) {
        throw new RuntimeException("Not implement!");
    }

    @Override
    public Surface configInputSurface(JpegCallback jpegCallback, int width,
            int height, int format) {
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
        Log.i(TAG, "startEncodeAndReleaseWhenDown");
        mCloseWhenEncodeDone = true;
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
     * Acquire rgb format buffer form image.
     * @param image the image where buffer from
     * @return byte buffer from image.
     */
    private ByteBuffer acquireRgbBufferAndClose(Image image) {
        ByteBuffer imageBuffer;
        if ((image.getPlanes()[0].getPixelStride() * image.getWidth()) != image.getPlanes()[0]
                .getRowStride()) {
            byte[] bytes = getContinuousRgbDataFromImage(image);
            imageBuffer = ByteBuffer.allocateDirect(bytes.length);
            imageBuffer.put(bytes);
            imageBuffer.rewind();
            bytes = null;
        } else {
            // continuous buffer, read directly
            imageBuffer = image.getPlanes()[0].getBuffer();
        }
        return imageBuffer;
    }

    /**
     * Read continuous byte from image when rowStride != pixelStride * width.
     * @param image the image where buffer from
     * @return byte array from image.
     */
    private byte[] getContinuousRgbDataFromImage(Image image) {
        Log.i(TAG, "getContinuousRGBADataFromImage begin");
        if (image.getFormat() != PixelFormat.RGBA_8888 &&
                image.getFormat() != PixelFormat.RGB_888) {
            Log.i(TAG, "error format = " + image.getFormat());
            return null;
        }
        int format = image.getFormat();
        int width = image.getWidth();
        int height = image.getHeight();
        int rowStride;
        int pixelStride;
        byte[] data;
        Image.Plane[] planes = image.getPlanes();
        PixelFormat pixelInfo = new PixelFormat();
        PixelFormat.getPixelFormatInfo(format, pixelInfo);
        ByteBuffer buffer = planes[0].getBuffer();
        rowStride = planes[0].getRowStride();
        pixelStride = planes[0].getPixelStride();
        data = new byte[width * height * pixelInfo.bitsPerPixel / 8];
        int offset = 0;
        int rowPadding = rowStride - pixelStride * width;
        // this format, pixelStride == bytesPerPixel, so read of the entire
        // row
        for (int y = 0; y < height; y++) {
            int length = width * pixelStride;
            buffer.get(data, offset, length);
            // Advance buffer the remainder of the row stride
            buffer.position(buffer.position() + rowPadding);
            offset += length;
        }
        Log.i(TAG, "getContinuousRGBADataFromImage end");
        return data;
    }

    /**
     *  An image available listener used to receive data from image reader.
     */
    private class ImageListener implements ImageReader.OnImageAvailableListener {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Log.i(TAG, "onImageAvailable thread name = " + Thread.currentThread().getName());
            Image image = reader.acquireNextImage();
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            ByteBuffer imageBuffer = acquireRgbBufferAndClose(image);
            Bitmap bitmap = Bitmap.createBitmap(image.getWidth(),
                    image.getHeight(), Bitmap.Config.ARGB_8888);
            bitmap.copyPixelsFromBuffer(imageBuffer);
            imageBuffer.clear();
            bitmap.compress(CompressFormat.JPEG, 95, out);
            bitmap.recycle();
            image.close();
            bitmap = null;
            imageBuffer = null;
            if (mJpegCallback != null) {
                mJpegCallback.onJpegAvailable(out.toByteArray());
            }
            if (out != null) {
                try {
                    out.close();
                    out = null;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mCloseWhenEncodeDone) {
                close();
                mCloseWhenEncodeDone = false;
            }
        }
    }
}