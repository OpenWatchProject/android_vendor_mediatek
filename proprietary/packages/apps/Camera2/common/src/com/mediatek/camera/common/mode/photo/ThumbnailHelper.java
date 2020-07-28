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
 *     MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.camera.common.mode.photo;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.util.Size;

import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Helper to provide thumbnail info for postview callback.
 */

public class ThumbnailHelper {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ThumbnailHelper.class.getSimpleName());
    public static final int IMAGE_BUFFER_FORMAT = ImageFormat.YUV_420_888;
    public static final int POST_VIEW_FORMAT = ImageFormat.NV21;
    public static final String FORMAT_TAG = "thumbnail";
    private static CaptureRequest.Key<int[]> mKeyRequestSize;
    private static int mViewWidth;
    private static int mWidth;
    private static int mHeight;
    private static ArrayList<Size> mThumbnailSizes;
    private static volatile boolean mIsSupport = false;
    private static volatile boolean mOverrideSupportValue = true;
    private static boolean mFindSize = false;
    private static CameraCharacteristics mCameraCharacteristics;
    private static boolean VERBOSE = false;

    /**
     * setApp to get thumbnail view width.
     * @param app Iapp.
     */
    public static void setApp(IApp app) {
        mWidth = app.getAppUi().getThumbnailViewWidth();
        mViewWidth = mWidth;
    }

    /**
     * when open camera, check whether it supports postview callback.
     * @param cs CameraCharacteristics.
     * @param context The camera context.
     * @param cameraId The current camera id.
     */
    public static void setCameraCharacteristics(CameraCharacteristics cs,
                                                Context context, int cameraId) {
        mCameraCharacteristics = cs;
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(context)
                .getDeviceDescriptionMap().get(String.valueOf(cameraId));
        if (deviceDescription != null) {
            mIsSupport = deviceDescription.isThumbnailPostViewSupport();
        }
        LogHelper.d(TAG, "[setCameraCharacteristics], mIsSupport = " + mIsSupport);

        if (deviceDescription != null) {
            mKeyRequestSize = deviceDescription.getKeyPostViewRequestSizeMode();
            LogHelper.d(TAG, "[setCameraCharacteristics], mKeyRequestSize = " + mKeyRequestSize);

            mThumbnailSizes = deviceDescription.getAvailableThumbnailSizes();
        }
    }

    /**
     * get thumbnail surface width.
     * @return the width.
     */
    public static int getThumbnailWidth() {
        return mWidth;
    }

    /**
     * get thumbnail surface height.
     * @return the height.
     */
    public static int getThumbnailHeight() {
        return mHeight;
    }

    /**
     * only support YUV_420_888 to NV21.
     * @param image the image form capture surface
     * @return the byte array.
     */
    public static byte[] getYUVBuffer(Image image) {
        if (image.getFormat() != IMAGE_BUFFER_FORMAT) {
            throw new IllegalArgumentException("Format not support!");
        }
        Rect crop = image.getCropRect();
        int format = image.getFormat();
        int width = crop.width();
        int height = crop.height();
        Image.Plane[] planes = image.getPlanes();
        byte[] data = new byte[width * height * ImageFormat.getBitsPerPixel(format) / 8];
        byte[] rowData = new byte[planes[0].getRowStride()];
        if (VERBOSE) {
            LogHelper.v(TAG, "[getYUVBuffer] get data from " + planes.length + " planes");
        }
        int channelOffset = 0;
        int outputStride = 1;
        for (int i = 0; i < planes.length; i++) {
            switch (i) {
                case 0:
                    channelOffset = 0;
                    outputStride = 1;
                    break;
                case 1:
                    channelOffset = width * height + 1;
                    outputStride = 2;
                    break;
                case 2:
                    channelOffset = width * height;
                    outputStride = 2;
                    break;
            }
            ByteBuffer buffer = planes[i].getBuffer();
            int rowStride = planes[i].getRowStride();
            int pixelStride = planes[i].getPixelStride();
            if (VERBOSE) {
                LogHelper.v(TAG, "[getYUVBuffer] pixelStride " + pixelStride);
                LogHelper.v(TAG, "[getYUVBuffer] rowStride " + rowStride);
                LogHelper.v(TAG, "[getYUVBuffer] width " + width);
                LogHelper.v(TAG, "[getYUVBuffer] height " + height);
                LogHelper.v(TAG, "[getYUVBuffer] buffer size " + buffer.remaining());
            }
            int shift = (i == 0) ? 0 : 1;
            int w = width >> shift;
            int h = height >> shift;
            buffer.position(rowStride * (crop.top >> shift) + pixelStride * (crop.left >> shift));
            for (int row = 0; row < h; row++) {
                int length;
                if (pixelStride == 1 && outputStride == 1) {
                    length = w;
                    buffer.get(data, channelOffset, length);
                    channelOffset += length;
                } else {
                    length = (w - 1) * pixelStride + 1;
                    buffer.get(rowData, 0, length);
                    for (int col = 0; col < w; col++) {
                        data[channelOffset] = rowData[col * pixelStride];
                        channelOffset += outputStride;
                    }
                }
                if (row < h - 1) {
                    buffer.position(buffer.position() + rowStride - length);
                }
            }
            if (VERBOSE) LogHelper.v(TAG, "[getYUVBuffer] Finished reading data from plane " + i);
        }
        return data;
    }

    /**
     * check whether it supports postview callback to update thumbnail.
     * @return support or not
     */
    public static boolean isPostViewSupported() {
        return mIsSupport;
    }


    /**
     * check whether override value.
     * @return support or not
     */
    public static boolean isPostViewOverrideSupported() {
        return mOverrideSupportValue && mIsSupport;
    }

    /**
     * maybe update by settings and modes.
     * @param key who override the value.
     * @param value to the value.
     */
    public static void overrideSupportedValue(String key, boolean value) {
        LogHelper.d(TAG, "[setPostViewSupportedValue] key: " + key + ", value: " + value);
        mOverrideSupportValue = value;
    }

    /**
     * check whether need set default jpeg thumbnail size.
     * if default jpeg thumbnail size is same with postview size,
     * it is goog for performance.
     * @param builder the capture request builder.
     * @return true for fond the thumbnail size for default jpeg thumbnail.
     */
    public static void setDefaultJpegThumbnailSize(CaptureRequest.Builder builder) {
        Size viewSize = new Size(mWidth, mHeight);
        builder.set(CaptureRequest.JPEG_THUMBNAIL_SIZE, viewSize);
    }

    /**
     * config capture request for postview callback.
     * @param builder the capture request builder.
     */
    public static void configPostViewRequest(CaptureRequest.Builder builder) {
        if (!mIsSupport || builder == null) {
            return;
        }
        int [] viewSize = new int[2];
        viewSize[0] = mWidth;
        viewSize[1] = mHeight;
        builder.set(mKeyRequestSize, viewSize);
    }

    /**
     * update thumbnail surface after camera is opened.
     * Be care, mViewWidth should not be zero.
     * @param targetRatio the size ratio.
     */
    public static void updateThumbnailSize(double targetRatio) {
        int thumbnailSize = mViewWidth * mViewWidth;
        int dlSize = Integer.MAX_VALUE;
        double dlRatio = Double.MAX_VALUE;
        mFindSize = false;
        mWidth = mHeight = mViewWidth;
        LogHelper.d(TAG, "[updateThumbnailSize], original, mWidth = " + mViewWidth);
        StreamConfigurationMap config =
                mCameraCharacteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] yuvSizes = config.getOutputSizes(ImageFormat.PRIVATE);
        if (yuvSizes == null) {
            return;
        }
        if (mThumbnailSizes != null) {
            for (Size size : mThumbnailSizes) {
                double thRatio = (double) size.getWidth() / size.getHeight();
                if (Math.abs(targetRatio - thRatio) <= dlRatio) {
                    dlRatio = Math.abs(targetRatio - thRatio);
                    mWidth = size.getWidth();
                    mHeight = size.getHeight();
                }
            }

            for (Size size : yuvSizes) {
                if (size.getWidth() == mWidth && size.getHeight() == mHeight) {
                    LogHelper.d(TAG, "[updateThumbnailSize 1], mWidth = " + mWidth
                            + ", mHeight = " + mHeight);
                    mFindSize = true;
                }
            }
        }

        if (mFindSize || !mIsSupport) {
            return;
        }

        for (Size size : yuvSizes) {
            int deltaSize = size.getWidth() * size.getHeight() - thumbnailSize;
            if (deltaSize > 0 && deltaSize <= dlSize) {
                dlSize = deltaSize;
                mWidth = size.getWidth();
                mHeight = size.getHeight();
            }
        }
        LogHelper.d(TAG, "[updateThumbnailSize 2], mWidth = " + mWidth + ", mHeight = " + mHeight);
    }
}
