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

package com.mediatek.camera.feature.mode.vsdof.photo;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CameraManager;
import android.os.Process;
import android.provider.MediaStore;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;

import java.io.Closeable;
import java.io.IOException;
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.List;

/**
 * this class used for assistant photo mode.
 * such as a tool class.
 */

public class SdofPhotoHelper {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SdofPhotoHelper.class.getSimpleName());
    private static final String IMAGE_FORMAT = "'IMG'_yyyyMMdd_HHmmss_SSS";


    private ImageFileName mImageFileName;
    private ICameraContext mICameraContext;

    /**
     * The constructor of PhotoModeHelper.
     * @param cameraContext current camera context.
     */
    public SdofPhotoHelper(ICameraContext cameraContext) {
        mICameraContext = cameraContext;
        mImageFileName = new ImageFileName(IMAGE_FORMAT);
    }

    /**
     * create a content values from data.
     * @param data the resource file.
     * @param fileDirectory file directory.
     * @param pictureWidth the width of content values.
     * @param pictureHeight the height of content valuse.
     * @return the content values from the data.
     */
    public ContentValues createContentValues(byte[] data, String fileDirectory, int
            pictureWidth, int pictureHeight) {
        ContentValues values = new ContentValues();
        long dateTaken = System.currentTimeMillis();
        String title = mImageFileName.generateTitle(dateTaken);
        String fileName = title + ".jpg";
        int orientation = CameraUtil.getOrientationFromExif(data);

        String mime = "image/jpeg";
        String path = fileDirectory + '/' + fileName;

        values.put(MediaStore.Images.ImageColumns.DATE_TAKEN, dateTaken);
        values.put(MediaStore.Images.ImageColumns.TITLE, title);
        values.put(MediaStore.Images.ImageColumns.DISPLAY_NAME, fileName);
        values.put(MediaStore.Images.ImageColumns.MIME_TYPE, mime);
        values.put(MediaStore.Images.ImageColumns.WIDTH, pictureWidth);
        values.put(MediaStore.Images.ImageColumns.HEIGHT, pictureHeight);

        values.put(MediaStore.Images.ImageColumns.ORIENTATION, orientation);
        values.put(MediaStore.Images.ImageColumns.DATA, path);

        LogHelper.d(TAG, "createContentValues, width : " + pictureWidth + ",height = " +
                pictureHeight + ",orientation = " + orientation);
        return values;
    }

    /**
     * make a bitmap form the data.
     * @param jpegData the resource of data.
     * @param maxNumOfPixels which max number of pixels will be samples.
     * @return the bitmap from the data.maybe null if decode fail.
     */
    public static Bitmap makeBitmap(byte[] jpegData, int maxNumOfPixels) {
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeByteArray(jpegData, 0, jpegData.length, options);
            if (options.mCancel || options.outWidth == -1 || options.outHeight == -1) {
                return null;
            }
            options.inSampleSize = computeSampleSize(options, -1, maxNumOfPixels);
            options.inJustDecodeBounds = false;

            options.inDither = false;
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            return BitmapFactory.decodeByteArray(jpegData, 0, jpegData.length, options);
        } catch (OutOfMemoryError ex) {
            LogHelper.e(TAG, "[makeBitmap] Got oom exception:", ex);
            return null;
        }
    }

    /**
     * Rotates and/or mirrors the bitmap. If a new bitmap is created, the
     * original bitmap is recycled.
     * @param b which bitmap will be rotate.
     * @param degrees which degree need to rotate.
     * @param mirror true means need mirror; otherwise don't need.
     * @return the rotated bitmap.
     */
    public static Bitmap rotateAndMirror(Bitmap b, int degrees, boolean mirror) {
        if ((degrees != 0 || mirror) && b != null) {
            Matrix m = new Matrix();
            // Mirror first.
            // horizontal flip + rotation = -rotation + horizontal flip
            if (mirror) {
                m.postScale(-1, 1);
                degrees = (degrees + 360) % 360;
                if (degrees == 0 || degrees == 180) {
                    m.postTranslate(b.getWidth(), 0);
                } else if (degrees == 90 || degrees == 270) {
                    m.postTranslate(b.getHeight(), 0);
                } else {
                    throw new IllegalArgumentException("Invalid degrees=" + degrees);
                }
            }
            if (degrees != 0) {
                // clockwise
                m.postRotate(degrees, (float) b.getWidth() / 2, (float) b.getHeight() / 2);
            }

            try {
                Bitmap b2 = Bitmap.createBitmap(b, 0, 0, b.getWidth(), b.getHeight(), m, true);
                if (b != b2) {
                    b.recycle();
                    b = b2;
                }
            } catch (OutOfMemoryError ex) {
                // We have no memory to rotate. Return the original bitmap.
                ex.printStackTrace();
            }
        }
        return b;
    }


    /**
     * close the resource.
     * @param closeable which resource need close.
     */
    public static void closeSilently(Closeable closeable) {
        if (closeable == null) {
            LogHelper.w(TAG, "[closeSilently] closeable is null ,return");
            return;
        }
        try {
            closeable.close();
        } catch (IOException e) {
            //do noting.
            e.printStackTrace();
        }
    }

    /**
     * Get the camera orientation from camera info.
     * @param cameraId the target camera id.
     * @param activity current activity.
     * @return orientation value.
     */
    public int getCameraInfoOrientation(String cameraId, Activity activity) {
        try {
            CameraManager cameraManager = (CameraManager) activity
                    .getSystemService(Context.CAMERA_SERVICE);
            CameraCharacteristics characteristics =
                    cameraManager.getCameraCharacteristics(cameraId);
            if (characteristics == null) {
                LogHelper.e(TAG, "[getCameraInfoOrientation] characteristics is null");
                return 0;
            }
            int orientation = characteristics
                    .get(CameraCharacteristics.SENSOR_ORIENTATION);
            return orientation;
        } catch (Exception e) {
            e.printStackTrace();
            LogHelper.e(TAG, "[getCameraInfoOrientation] camera process killed due to" +
                    " getCameraCharacteristics() error");
            Process.killProcess(Process.myPid());
        }
        return 0;
    }

    /**
     * Check the camera is need mirror or not.
     * @param cameraId current camera id.
     * @param activity current activity.
     * @return true means need mirror.
     */
    public boolean isMirror(String cameraId, Activity activity) {
        try {
            CameraManager cameraManager = (CameraManager) activity
                    .getSystemService(Context.CAMERA_SERVICE);
            CameraCharacteristics characteristics =
                    cameraManager.getCameraCharacteristics(cameraId);
            if (characteristics == null) {
                LogHelper.e(TAG, "[isMirror] characteristics is null");
                return false;
            }
            int facing = characteristics.get(CameraCharacteristics.LENS_FACING);
            int sensorOrientation = characteristics
                    .get(CameraCharacteristics.SENSOR_ORIENTATION);
            return (facing == CameraMetadata.LENS_FACING_FRONT);
        } catch (Exception e) {
            e.printStackTrace();
            LogHelper.e(TAG, "[isMirror] camera process killed due to" +
                    " getCameraCharacteristics() error");
            Process.killProcess(Process.myPid());
        }
        return false;
    }

    private static int computeSampleSize(BitmapFactory.Options options, int minSideLength,
            int maxNumOfPixels) {
        int initialSize = computeInitialSampleSize(options, minSideLength, maxNumOfPixels);

        int roundedSize;
        if (initialSize <= 8) {
            roundedSize = 1;
            while (roundedSize < initialSize) {
                roundedSize <<= 1;
            }
        } else {
            roundedSize = (initialSize + 7) / 8 * 8;
        }

        return roundedSize;
    }

    private static int computeInitialSampleSize(BitmapFactory.Options options, int minSideLength,
            int maxNumOfPixels) {
        double w = options.outWidth;
        double h = options.outHeight;

        int lowerBound = (maxNumOfPixels < 0) ? 1 : (int) Math.ceil(Math.sqrt(w * h
                / maxNumOfPixels));
        int upperBound = (minSideLength < 0) ? 128 : (int) Math.min(Math.floor(w / minSideLength),
                Math.floor(h / minSideLength));

        if (upperBound < lowerBound) {
            // return the larger one when there is no overlapping zone.
            return lowerBound;
        }

        if (maxNumOfPixels < 0 && minSideLength < 0) {
            return 1;
        } else if (minSideLength < 0) {
            return lowerBound;
        } else {
            return upperBound;
        }
    }
    /**
     * Used for create image file name.
     */
    private class ImageFileName {
        private SimpleDateFormat mSimpleDateFormat;
        public ImageFileName(String format) {
            mSimpleDateFormat = new SimpleDateFormat(format);
        }

        public String generateTitle(long dateTaken) {
            Date date = new Date(dateTaken);
            String result = mSimpleDateFormat.format(date);
            return result;
        }
    }

}
