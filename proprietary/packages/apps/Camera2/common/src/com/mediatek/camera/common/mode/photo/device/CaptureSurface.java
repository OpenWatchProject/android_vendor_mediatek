/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.mode.photo.device;

import android.annotation.TargetApi;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.photo.HeifHelper;
import com.mediatek.camera.common.mode.photo.ThumbnailHelper;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Used for capture surface.
 */
@TargetApi(Build.VERSION_CODES.KITKAT)
public class CaptureSurface {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(CaptureSurface.class.getSimpleName());
    private static final String FORMAT_JPEG = "jpeg";
    private static final String FORMAT_HEIF = "heif";
    private static final String FORMAT_THUMBNAIL = "thumbnail";
    private int mPictureWidth;
    private int mPictureHeight;
    @SuppressWarnings("deprecation")
    private int mFormat = PixelFormat.JPEG;
    private int mMaxImages = 12;
    private ImageReader mCaptureImageReader;
    private final Handler mCaptureHandler;
    private final Object mImageReaderSync = new Object();
    private ImageCallback mImageCallback;
    private String mFormatTag = new String(FORMAT_JPEG);
    private List<Integer> mBGEventCallbackQueue = new ArrayList<>();
    private List<ImageReader> mImageReaderList = new ArrayList<>();
    private List<ImageReader> mImageReaderQueue = new ArrayList<>();
    private boolean mIsCreatedByBG = false;
    /**
     * Capture image callback.
     */
    public interface ImageCallback {
        /**
         * When picture callback will be fired this function.
         * @param data picture data.
         */
        void onPictureCallback(byte[] data, int format, String formatTag, int width, int height);
    }

    public CaptureSurface(Handler handler) {
        LogHelper.d(TAG, "[CaptureSurface]Construct with handler");
        mCaptureHandler = handler;
        mIsCreatedByBG = true;
    }

    /**
     * Prepare the capture surface handler.
     */
    public CaptureSurface() {
        LogHelper.d(TAG, "[CaptureSurface]Construct");
        mCaptureHandler = new Handler(Looper.myLooper());
    }

    /**
     * Update a new picture info,such as size ,format , max image.
     * @param width    the target picture width.
     * @param height   the target picture height.
     * @param format   The format of the Image that this reader will produce.
     *                 this must be one of the {@link android.graphics.ImageFormat} or
     *                 {@link android.graphics.PixelFormat} constants. Note that not
     *                 all formats are supported, like ImageFormat.NV21. The default value is
     *                 PixelFormat.JPEG;
     * @param maxImage The maximum number of images the user will want to
     *                 access simultaneously. This should be as small as possible to
     *                 limit memory use. Once maxImages Images are obtained by the
     *                 user, one of them has to be released before a new Image will
     *                 become available for access through onImageAvailable().
     *                 Must be greater than 0.
     * @return if surface is changed, will return true, otherwise will false.
     */
    public boolean updatePictureInfo(int width, int height, int format, int maxImage) {
        // Check picture info whether is same as before or not.
        // if the info don't change, No need create it again.
        LogHelper.i(TAG, "[updatePictureInfo] width = " + width + ",height = " + height + "," +
                "format = " + format + ",maxImage = " + maxImage + ",mCaptureImageReader = " +
                mCaptureImageReader);
        if (mCaptureImageReader != null && mPictureWidth == width && mPictureHeight == height &&
                format == mFormat && maxImage == mMaxImages) {
            LogHelper.d(TAG, "[updatePictureInfo],the info : " + mPictureWidth + " x " +
                    mPictureHeight + ",format = " + format + ",maxImage = " + maxImage + " is " +
                    "same as before");
            return false;
        }
        // Save the new picture info.
        mPictureWidth = width;
        mPictureHeight = height;
        mFormat = format;
        mMaxImages = maxImage;

        // Create a image reader for images of the desired size,format and max image.
        synchronized (mImageReaderSync) {
            mCaptureImageReader = ImageReader.newInstance(mPictureWidth, mPictureHeight, mFormat,
                    mMaxImages);
            mCaptureImageReader.setOnImageAvailableListener(mCaptureImageListener, mCaptureHandler);
            mImageReaderList.add(mCaptureImageReader);
            LogHelper.d(TAG, "[updatePictureInfo], new mCaptureImageReader = " +
                    mCaptureImageReader);
        }
        return true;
    }

    public boolean updatePictureInfo(int format) {
        return updatePictureInfo(mPictureWidth, mPictureHeight, format, mMaxImages);

    }

    /**
     * Get the capture surface from image reader.
     * @return the surface is from image reader.
     *          if don't have call the updatePictureInfo() before getSurface() will be return null.
     *          such as you have calling releaseCaptureSurface(), the value is null.
     */
    public Surface getSurface() {
        synchronized (mImageReaderSync) {
            if (mCaptureImageReader != null) {
                return mCaptureImageReader.getSurface();
            }
            return null;
        }
    }

    /**
     * Release the capture surface when don't need again.
     */
    public void releaseCaptureSurface() {
        LogHelper.d(TAG, "[releaseCaptureSurface], release mCaptureImageReader = " +
                mCaptureImageReader);
        synchronized (mImageReaderSync) {
            if (mCaptureImageReader != null) {
                mCaptureImageReader.close();
                mCaptureImageReader = null;
            }
        }
    }

    /**
     * When activity destroy, release the resource.
     */
    public void release() {
    }

    public void setCaptureCallback(ImageCallback captureCallback) {
        mImageCallback = captureCallback;
    }


    public void setFormat(String format) {
        if (format == null) {
            LogHelper.e(TAG, "[setFormat], null pointer! mFormatTag: " + format);
            return;
        }
        mFormatTag = format;
        LogHelper.d(TAG, "[setFormat], mFormatTag: " + mFormatTag);
    }

    public String getFormat() {
        return mFormatTag;
    }

    public void addBGEventCallbackQueue(int frameNumber) {
        synchronized (mBGEventCallbackQueue) {
            mBGEventCallbackQueue.add(frameNumber);
        }
    }

    public int getImageReaderId() {
        return mImageReaderList.indexOf(mCaptureImageReader);
    }

    public void notifyImageReader() {
        if (mIsCreatedByBG) {
            synchronized (mImageReaderQueue) {
                LogHelper.d(TAG, "notifyImageReader mImageReaderQueue.size = "
                        + mImageReaderQueue.size());
                if (!mImageReaderQueue.isEmpty() && mImageCallback != null) {
                    Image image = mImageReaderQueue.get(0).acquireNextImage();
                    mImageReaderQueue.remove(0);
                    mBGEventCallbackQueue.remove(0);
                    if (image == null) {
                        LogHelper.w(TAG, "[notifyImageReader] image buffer is null");
                        return;
                    }
                    int format = image.getFormat();
                    int width = image.getWidth();
                    int height = image.getHeight();
                    mImageCallback.onPictureCallback(getBuffer(image),
                            format, mFormatTag, width, height);
                }
            }
        }
    }
    private final OnImageAvailableListener mCaptureImageListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader imageReader) {
            LogHelper.d(TAG, "[onImageAvailable] mIsCreatedByBG = " + mIsCreatedByBG
                    + ", mBGEventCallbackQueue.size = " + mBGEventCallbackQueue.size());
            if (mIsCreatedByBG) {
                if (mBGEventCallbackQueue.isEmpty()) {
                    synchronized (mImageReaderQueue) {
                        mImageReaderQueue.add(imageReader);
                    }
                } else {
                    synchronized (mBGEventCallbackQueue) {
                        if (mImageCallback != null) {
                            Image image = imageReader.acquireNextImage();
                            mBGEventCallbackQueue.remove(0);
                            if (image == null) {
                                LogHelper.w(TAG, "[onImageAvailable] image buffer is null");
                                return;
                            }
                            int format = image.getFormat();
                            int width = image.getWidth();
                            int height = image.getHeight();
                            mImageCallback.onPictureCallback(getBuffer(image),
                                    format, mFormatTag, width, height);
                        }
                    }
                }
            } else {
                if (mImageCallback != null) {
                    Image image = imageReader.acquireNextImage();
                    if (image == null) {
                        LogHelper.w(TAG, "[onImageAvailable] image buffer is null");
                        return;
                    }
                    int format = image.getFormat();
                    int width = image.getWidth();
                    int height = image.getHeight();
                    mImageCallback.onPictureCallback(getBuffer(image),
                            format, mFormatTag, width, height);
                }
            }
        }
    };

    private byte[] getBuffer(Image image) {
        synchronized (mImageReaderSync) {
            byte[] imageBuffer = null;
            LogHelper.i(TAG, "[getBuffer] image.getFormat = " + image.getFormat()
                    + " image.getWidth = " + image.getWidth() + " image.getHeight = "
                    + image.getHeight() + " format = " + image.getFormat()
                    + " formatTag = " + mFormatTag);

            if (FORMAT_HEIF.equalsIgnoreCase(mFormatTag)) {
                imageBuffer = HeifHelper.getYUVBuffer(image);
            } else if (FORMAT_JPEG.equalsIgnoreCase(mFormatTag)) {
                Image.Plane plane = image.getPlanes()[0];
                ByteBuffer buffer = plane.getBuffer();
                imageBuffer = new byte[buffer.remaining()];
                buffer.get(imageBuffer);
                buffer.rewind();
            } else if (FORMAT_THUMBNAIL.equalsIgnoreCase(mFormatTag)) {
                imageBuffer = ThumbnailHelper.getYUVBuffer(image);
            }
            else {
                throw new RuntimeException("Unsupported image format.");
            }
            image.close();
            return imageBuffer;
        }
    }

}
