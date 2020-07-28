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

package com.mediatek.camera.v2.stream;

import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.stream.pip.pipwrapping.PipEGLConfigWrapper;
import com.mediatek.camera.v2.util.Utils;

import junit.framework.Assert;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

/**
 * A stream used for capture.
 */
public class CaptureStream implements ICaptureStream {
    private static final Tag TAG = new Tag(CaptureStream.class.getSimpleName());
    private static int                            MAX_CAPTURE_IMAGES = 2;
    // Receives the normal captured images.
    private ImageReader                           mImageReader;
    // Handler thread for camera-related operations.
    private HandlerThread                         mCaptureHandlerThread;
    private Handler                               mCaptureHandler;
    private Surface                               mCaptureSurface;
    private int                                   mCaptureWidth;
    private int                                   mCaptureHeight;
    // Receives the normal captured images.
    private ImageReader.OnImageAvailableListener mCaptureImageListener =
            new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (mCallback != null) {
                byte[] data = null;
                int width = 0;
                int height = 0;
                int imageFormat = -1;
                synchronized (mImageReader) {
                    Image image = reader.acquireLatestImage();
                    imageFormat = image.getFormat();
                    width = image.getWidth();
                    height = image.getHeight();
                    if (PixelFormat.RGBA_8888 == imageFormat) {
                        data = compressRGBA888ToJpeg(image);
                    } else if (PixelFormat.JPEG == imageFormat) {
                        data = Utils.acquireJpegBytesAndClose(image);
                    }
                }
                mCallback.onCaptureCompleted(new ImageInfo(data, width, height, imageFormat));
            }
        }
    };

    private CaptureStreamCallback                 mCallback;

    public CaptureStream() {

    }

    @Override
    public void setCaptureStreamCallback(CaptureStreamCallback callback) {
        mCallback = callback;
    }

    @Override
    public synchronized boolean updateCaptureSize(Size pictureSize, int pictureFormat) {
        LogHelper.i(TAG, "[updateCaptureSize]+ size:" + pictureSize.getWidth() + "x"
                    + pictureSize.getHeight());
        Assert.assertNotNull(pictureSize);
        checkSupportedFormat(pictureFormat);
        // if image reader thread not started, start it
        if (mCaptureHandler == null) {
            mCaptureHandlerThread = new HandlerThread("ImageReaderStream.CaptureThread");
            mCaptureHandlerThread.start();
            mCaptureHandler = new Handler(mCaptureHandlerThread.getLooper());
        }
        // check size, if same size skip
        int width = pictureSize.getWidth();
        int height = pictureSize.getHeight();
        if (mImageReader != null && mImageReader.getWidth() == width
                                 && mImageReader.getHeight() == height
                                 && mImageReader.getImageFormat() == pictureFormat) {
            LogHelper.i(TAG, "[updateCaptureSize]- configure the same size, skip : " + "" +
                    " width  = " + width +
                    " height = " + height +
                    " format = " + pictureFormat);
            return false;
        }
        mCaptureWidth = width;
        mCaptureHeight = height;
        // if previous ImageReader exists, close it for create a new one.
        // FIXME consider capture state, may can not close directly
//        if (mImageReader != null) {
//            mImageReader.close();
//            mImageReader = null;
//        }
        mImageReader = ImageReader.newInstance(mCaptureWidth, mCaptureHeight, pictureFormat,
                MAX_CAPTURE_IMAGES);
        mImageReader.setOnImageAvailableListener(mCaptureImageListener, mCaptureHandler);
        mCaptureSurface = mImageReader.getSurface();
        LogHelper.i(TAG, "[updateCaptureSize]- mCaptureSurface:" + mCaptureSurface);
        return true;
    }

    @Override
    public Map<String, Surface> getCaptureInputSurface() {
        LogHelper.i(TAG, "[getCaptureInputSurface]+");
        Map<String, Surface> surfaceMap = new HashMap<String, Surface>();
        if (mCaptureSurface == null) {
            throw new IllegalStateException("You should call " +
                    "CaptureStream.updateCaptureSize firstly, " +
                    "when get input capture surface");
        }
        surfaceMap.put(CAPUTRE_SURFACE_KEY, mCaptureSurface);
        LogHelper.i(TAG, "[getCaptureInputSurface]- mCaptureSurface:" + mCaptureSurface);
        return surfaceMap;
    }

    private void checkSupportedFormat(int format) {
        boolean supported = false;
        switch (format) {
        case PixelFormat.RGBA_8888:
        case ImageFormat.JPEG:
            supported = true;
            break;
        default:
            break;
        }
        if (!supported) {
            throw new IllegalArgumentException("ImageReaderStream unsupported format : " + format);
        }
    }

    private byte[] compressRGBA888ToJpeg(Image image) {
        byte[] jpegData = null;
        try {
            Bitmap bitmap = Bitmap.createBitmap(image.getWidth(), image.getHeight(),
                    PipEGLConfigWrapper.getInstance().getBitmapConfig());
            ByteBuffer imageBuffer = null;
            // get byte buffer from image
            if ((image.getPlanes()[0].getPixelStride() * image.getWidth()) != image.getPlanes()[0]
                    .getRowStride()) {
                LogHelper.i(TAG, "getPixelStride = " + image.getPlanes()[0].getPixelStride()
                        + " getRowStride = " + image.getPlanes()[0].getRowStride());
                // buffer is not placed continuously, should remove buffer
                // position again
                byte[] bytes = Utils.getContinuousRGBADataFromImage(image);
                imageBuffer = ByteBuffer.allocateDirect(bytes.length);
                imageBuffer.put(bytes);
                imageBuffer.rewind();
                bytes = null;
            } else {
                // continuous buffer, read directly
                imageBuffer = image.getPlanes()[0].getBuffer();
            }
            System.gc();
            // copy buffer to bitmap
            bitmap.copyPixelsFromBuffer(imageBuffer);
            imageBuffer.clear();
            imageBuffer = null;
            // compress and save it
            ByteArrayOutputStream bos = null;
            try {
                bos = new ByteArrayOutputStream();
                bitmap.compress(Bitmap.CompressFormat.JPEG, 90, bos);
                bitmap.recycle();
                bitmap = null;
            } finally {
                jpegData = bos.toByteArray();
                if (bos != null) {
                    try {
                        bos.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
            System.gc();
            image.close();
            image = null;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        return jpegData;
    }

    @Override
    public void releaseCaptureStream() {
        LogHelper.i(TAG, "releaseCaptureStream");
        if (mImageReader != null) {
            mImageReader.close();
            mImageReader = null;
        }

        if (mCaptureHandlerThread != null) {
            mCaptureHandlerThread.quitSafely();
            mCaptureHandler = null;
        }
    }
}