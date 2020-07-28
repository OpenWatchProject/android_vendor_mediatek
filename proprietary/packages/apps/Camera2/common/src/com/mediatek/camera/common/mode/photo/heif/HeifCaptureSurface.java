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
package com.mediatek.camera.common.mode.photo.heif;

import android.annotation.TargetApi;
import android.content.ContentValues;

import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.hardware.camera2.CaptureRequest;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.net.Uri;
import android.os.*;
import android.provider.MediaStore;
import android.view.Surface;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.photo.HeifHelper;
import com.mediatek.camera.common.mode.photo.ThumbnailHelper;
import com.mediatek.camera.common.bgservice.CaptureSurface;
import com.mediatek.camera.common.bgservice.ImageReaderManager;
import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;


/**
 * Used for capture surface.
 */
@TargetApi(Build.VERSION_CODES.KITKAT)
public class HeifCaptureSurface extends CaptureSurface implements IDeviceListener{
    private static final LogUtil.Tag TAG = new LogUtil.Tag(HeifCaptureSurface.class.getSimpleName
            ());
    private static final String FORMAT_JPEG = "jpeg";
    private static final String FORMAT_HEIF = "heif";
    private static final String FORMAT_THUMBNAIL = "thumbnail";
    private int mPictureWidth;
    private int mPictureHeight;
    @SuppressWarnings("deprecation")
    private int mFormat = PixelFormat.JPEG;
    private int mMaxImages = 12;
    private final Object mImageReaderSync = new Object();
    private ImageCallback mImageCallback;
    private String mFormatTag = new String(FORMAT_JPEG);
    private HandlerThread mHandlerThread;
    private HeifHelper mHeifHelper;
    private Handler mHeifEncodeHandler;
    private Surface mHeifSurface;
    private HeifWriter mHeifWriter = null;
    private ICompeletedCallback mListener;


    /**
     * Prepare the capture surface handler.
     */
    public HeifCaptureSurface(ICameraContext context, ICompeletedCallback listener) {
        LogHelper.d(TAG, "[HeifCaptureSurface]Construct");
        mHeifHelper = new HeifHelper(context);
        mHandlerThread = new HandlerThread("HeifCaptureSurface",
                android.os.Process.THREAD_PRIORITY_FOREGROUND);
        mHandlerThread.start();
        mHeifEncodeHandler = new Handler(mHandlerThread.getLooper());
        mImageReaderManager = new ImageReaderManager();
        mListener = listener;
    }

    public void setCompletedListener(ICompeletedCallback listener) {

    }
    @Override
    public void onTakePicture() {
        if (mFormat == ImageFormat.YUV_420_888) {
            LogHelper.i(TAG, "[onTakePicture]....2222 ");
            mHeifEncodeHandler.post(
                    new Runnable() {
                        public void run() {
                            try {
                                mHeifWriter.start();
                                long beginTime = System.currentTimeMillis();
                                mHeifWriter.stop(20000);
                                LogHelper.i(TAG, "[saveData] save heif file " +
                                        "consume time = " + (System
                                        .currentTimeMillis() - beginTime));
                                //new File(tempFile).renameTo(new File
                                // (filePath));
                                Uri uri = mHeifHelper.saveData();
                                if (uri != null && mListener != null) {
                                    mListener.onFinishSaveDataCallback(uri);
                                }
                                mHeifWriter.close();
                            } catch (Exception e) {
                                LogHelper.e(TAG, "Exception", e);
                            }

                        }
                    }
            );
        }
    }
    /**
     * Update a new picture info,such as size ,format , max image.
     *
     * @param width    the target picture width.
     * @param height   the target picture height.
     * @param format   The format of the Image that this reader will produce.
     *                 this must be one of the {@link ImageFormat} or
     *                 {@link PixelFormat} constants. Note that not
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
    @Override
    public boolean updatePictureInfo(final int width, final int height, int format, int maxImage) {
        // Check picture info whether is same as before or not.
        // if the info don't change, No need create it again.
        LogHelper.i(TAG, "[updatePictureInfo] width = " + width + ",height = " + height + "," +
                "format = " + format + ",maxImage = " + maxImage + ",mHeifSurface = " +
                mHeifSurface);
        if (mHeifSurface != null && mPictureWidth == width && mPictureHeight == height &&
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
            if (mFormat == ImageFormat.YUV_420_888) {
                final ContentValues values = mHeifHelper.getContentValues(mPictureWidth,
                        mPictureHeight);
                String filePath = values.getAsString(MediaStore.Images.ImageColumns.DATA);
                int orientation = values.getAsInteger(MediaStore.Images.ImageColumns.ORIENTATION)
                        .intValue();
                HeifWriter.Builder builder = new HeifWriter.Builder(
                        filePath, width, height, HeifWriter.INPUT_MODE_SURFACE);
                builder.setGridEnabled(true);
                builder.setRotation(orientation);
                builder.setQuality(60);
                try {
                    mHeifWriter = builder.build();
                    mHeifSurface = mHeifWriter.getInputSurface();
                    LogHelper.i(TAG, "[updatePictureInfo] new heifwriter ");
                } catch (Exception e) {
                    LogHelper.e(TAG, "Exception", e);
                }
            } else {
                ImageReader captureImageReader = mImageReaderManager.getImageReader(mPictureWidth,
                        mPictureHeight, mFormat, mMaxImages);
                captureImageReader.setOnImageAvailableListener(mCaptureImageListener,
                        mCaptureHandler);
            }

        }
        return true;
    }

    @Override
    public boolean updatePictureInfo(int format) {
        return updatePictureInfo(mPictureWidth, mPictureHeight, format, mMaxImages);

    }

    @Override
    public int getImageReaderId() {
        if (mFormat == ImageFormat.YUV_420_888) {
            return 1;
        } else {
            return super.getImageReaderId();
        }
    }
    /**
     * Get the capture surface from image reader.
     *
     * @return the surface is from image reader.
     * if don't have call the updatePictureInfo() before getSurface() will be return null.
     * such as you have calling releaseCaptureSurface(), the value is null.
     */
    @Override
    public Surface getSurface() {
        synchronized (mImageReaderSync) {
            if (mFormat == ImageFormat.YUV_420_888) {
                if (mHeifSurface != null) {
                    LogHelper.d(TAG, "[getSurface], mHeifSurface = " + mHeifSurface);
                    return mHeifSurface;
                }
            } else {
                if (mImageReaderManager != null) {
                    ImageReader captureImageReader = mImageReaderManager.getImageReader(
                            mPictureWidth, mPictureHeight, mFormat, mMaxImages);
                    return captureImageReader.getSurface();
                }
            }

            return null;
        }
    }

    /**
     * Release the capture surface when don't need again.
     */
    @Override
    public void releaseCaptureSurface() {
        LogHelper.d(TAG, "[releaseCaptureSurface], mHeifSurface = " + mHeifSurface);
        synchronized (mImageReaderSync) {
            if (mFormat == ImageFormat.YUV_420_888) {
                //mHeifSurface.close();
                mHeifSurface = null;
            } else {
                super.releaseCaptureSurface();
            }
        }
    }

    /**
     * When activity destroy, release the resource.
     */
    public void release() {
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

}
