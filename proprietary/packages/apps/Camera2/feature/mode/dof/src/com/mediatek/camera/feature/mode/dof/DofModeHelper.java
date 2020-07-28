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

package com.mediatek.camera.feature.mode.dof;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.location.Location;
import android.provider.MediaStore;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.photo.PhotoModeHelper;
import com.mediatek.camera.common.utils.CameraUtil;

import java.sql.Date;
import java.text.SimpleDateFormat;



class DofModeHelper {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(PhotoModeHelper.class.getSimpleName());
    private static final String IMAGE_FORMAT = "'IMG'_yyyyMMdd_HHmmss_SSS";


    private ImageFileName mImageFileName;
    private ICameraContext mICameraContext;

    /**
     * The constructor of PhotoModeHelper.
     * @param cameraContext current camera context.
     */
    public DofModeHelper(ICameraContext cameraContext) {
        mICameraContext = cameraContext;
        mImageFileName = new DofModeHelper.ImageFileName(IMAGE_FORMAT);
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

        Location location = mICameraContext.getLocation();
        if (location != null) {
            values.put(MediaStore.Images.ImageColumns.LATITUDE, location.getLatitude());
            values.put(MediaStore.Images.ImageColumns.LONGITUDE, location.getLongitude());
        }
        LogHelper.d(TAG, "createContentValues, width : " + pictureWidth + ",height = " +
                pictureHeight + ",orientation = " + orientation + ", dataTaken = " + dateTaken
                + ", fileName = " + fileName);
        return values;
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
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        //TODO Tmp Change to true
        return true;
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
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return 0;
    }
}
