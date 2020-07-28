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

import android.content.ContentValues;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.location.Location;
import android.provider.MediaStore;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;

import java.io.Closeable;
import java.io.IOException;
import java.sql.Date;
import java.text.SimpleDateFormat;

/**
 * this class used for assistant photo mode.
 * such as a tool class.
 */

public class SdofPhotoHelper {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SdofPhotoHelper.class.getSimpleName());
    private static final String IMAGE_FORMAT = "'IMG'_yyyyMMdd_HHmmss_S";
    private static final String KEY_STEREO_REFOCUS_PICTURE = "camera_refocus";
    private static final int IS_STEREO_PICTURE = 1;
    private static final int IS_STEREO_THUMBNAIL = 2;


    private ImageFileName mImageFileName;
    private ICameraContext mICameraContext;

    /**
     * The constructor of SdofPhotoHelper.
     * @param cameraContext current camera context.
     */
    public SdofPhotoHelper(ICameraContext cameraContext) {
        mICameraContext = cameraContext;
        mImageFileName = new ImageFileName(IMAGE_FORMAT);
    }

    /**
     * create a content values from data.
     * @param data the resource file.
     * @param pictureWidth  the width of content values.
     * @param pictureHeight the height of content value.
     * @param time          the picture capture time.
     * @param isStereo true if it is stereo picture.
     * @return the content values from the data.
     */
    public ContentValues createContentValues(byte[] data, int
            pictureWidth, int pictureHeight, long time, boolean isStereo) {
        ContentValues values = new ContentValues();
        String fileDirectory = mICameraContext.getStorageService().getFileDirectory();
        String title = mImageFileName.generateTitle(time);
        String fileName = title + ".jpg";
        int orientation = CameraUtil.getOrientationFromExif(data);

        String mime = "image/jpeg";
        String path = fileDirectory + '/' + fileName;

        values.put(MediaStore.Images.ImageColumns.DATE_TAKEN, time);
        values.put(MediaStore.Images.ImageColumns.TITLE, title);
        values.put(MediaStore.Images.ImageColumns.DISPLAY_NAME, fileName);
        values.put(MediaStore.Images.ImageColumns.MIME_TYPE, mime);
        values.put(MediaStore.Images.ImageColumns.WIDTH, pictureWidth);
        values.put(MediaStore.Images.ImageColumns.HEIGHT, pictureHeight);
        values.put(MediaStore.Images.ImageColumns.ORIENTATION, orientation);
        values.put(MediaStore.Images.ImageColumns.DATA, path);
        values.put(MediaStore.Images.ImageColumns.SIZE, data.length);

        values.put(KEY_STEREO_REFOCUS_PICTURE, IS_STEREO_PICTURE);
        Location location = mICameraContext.getLocation();
        if (location != null) {
            values.put(MediaStore.Images.ImageColumns.LATITUDE, location.getLatitude());
            values.put(MediaStore.Images.ImageColumns.LONGITUDE, location.getLongitude());
        }
        LogHelper.d(TAG, "createContentValues, width : " + pictureWidth + ",height = " +
                pictureHeight + ",orientation = " + orientation);
        return values;
    }

    /**
     * create a content values from thumbnail data.
     *
     * @param pictureWidth  the width of content values.
     * @param pictureHeight the height of content value.
     * @param time          the picture capture time.
     * @return the content values from the data.
     */
    public ContentValues createThumbnailContentValues(int pictureWidth,
                                                      int pictureHeight, long time) {
        ContentValues values = new ContentValues();
        String fileDirectory = mICameraContext.getStorageService().getFileDirectory();
        String title = mImageFileName.generateTitle(time);
        String fileName = "." + title + ".jpg";

        String mime = "image/jpeg";
        // Add suffix .stereothumb with thumb jpeg path
        // for avoiding media scanner scanning when reboot device.
        String path = fileDirectory + '/' + fileName + ".stereothumb";

        values.put(MediaStore.Images.ImageColumns.DATE_TAKEN, time);
        values.put(MediaStore.Images.ImageColumns.TITLE, title);
        values.put(MediaStore.Images.ImageColumns.DISPLAY_NAME, fileName);
        values.put(MediaStore.Images.ImageColumns.MIME_TYPE, mime);
        values.put(MediaStore.Images.ImageColumns.WIDTH, pictureWidth);
        values.put(MediaStore.Images.ImageColumns.HEIGHT, pictureHeight);
        values.put(MediaStore.Images.ImageColumns.DATA, path);

        values.put(KEY_STEREO_REFOCUS_PICTURE, IS_STEREO_THUMBNAIL);
        Location location = mICameraContext.getLocation();
        if (location != null) {
            values.put(MediaStore.Images.ImageColumns.LATITUDE, location.getLatitude());
            values.put(MediaStore.Images.ImageColumns.LONGITUDE, location.getLongitude());
        }
        LogHelper.d(TAG, "createThumbnailContentValues, width : " + pictureWidth + ",height = " +
                pictureHeight + ", path = " + path);
        return values;
    }

    /**
     * Get the file name by time.
     *
     * @param time current time.
     * @return the name by time.
     */
    public static String getFileName(long time) {
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat(IMAGE_FORMAT);
        Date date = new Date(time);
        String result = simpleDateFormat.format(date);
        return result + ".jpg";
    }

    /**
     * Used for create image file name.
     */
    private class ImageFileName {
        private SimpleDateFormat mSimpleDateFormat;

        public ImageFileName(String format) {
            mSimpleDateFormat = new SimpleDateFormat(format);
        }

        public synchronized String generateTitle(long dateTaken) {
            Date date = new Date(dateTaken);
            String result = mSimpleDateFormat.format(date);
            return result;
        }
    }
}
