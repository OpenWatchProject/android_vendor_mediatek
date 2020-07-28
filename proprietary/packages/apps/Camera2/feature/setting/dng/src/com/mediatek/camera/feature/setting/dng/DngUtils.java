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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.feature.setting.dng;

import android.annotation.TargetApi;
import android.content.ContentValues;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.DngCreator;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.location.Location;
import android.media.ExifInterface;
import android.media.Image;
import android.os.Build;
import android.provider.MediaStore;
import android.util.Size;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import junit.framework.Assert;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

/**
 * Utils class of dng tool.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class DngUtils {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(DngUtils.class.getSimpleName());
    private static final String IMAGE_FORMAT = "'IMG'_yyyyMMdd_HHmmss";
    /**
     * Get the raw image data from creator by dng format.
     * @param imageData picture data.
     * @param cs characteristics.
     * @param captureResult capture result.
     * @param rawSize raw size.
     * @param dngOrientation dng orientation.
     * @return dng data.
     */
    public static byte[] getDngDataFromCreator(byte[] imageData,
                                         CameraCharacteristics cs, CaptureResult captureResult,
                                                        Size rawSize, int dngOrientation) {
        ByteArrayOutputStream outputStream;
        byte[] dngData = null;
        try {
            DngCreator dngCreator = new DngCreator(cs, captureResult);
            dngCreator.setOrientation(dngOrientation);
            outputStream = new ByteArrayOutputStream();
            ByteBuffer rawBuffer = ByteBuffer.wrap(imageData);
            dngCreator.writeByteBuffer(outputStream, rawSize, rawBuffer, 0);
            dngData = outputStream.toByteArray();
            outputStream.close();
            rawBuffer = null;
        } catch (IOException e) {
            LogHelper.e(TAG, "[convertRawToDng], dng write error");
        }
        return dngData;
    }

    /**
     * Get the dng orientation.
     *
     * @param imageOrientation the picture orientation.
     * @return the dng orientation.
     */
    public static int getDngOrientation(int imageOrientation) {
        int orientation;
        if (imageOrientation == 0) {
            orientation = ExifInterface.ORIENTATION_NORMAL;
        } else if (imageOrientation == 90) {
            orientation = ExifInterface.ORIENTATION_ROTATE_90;
        } else if (imageOrientation == 180) {
            orientation = ExifInterface.ORIENTATION_ROTATE_180;
        } else {
            orientation = ExifInterface.ORIENTATION_ROTATE_270;
        }
        return orientation;
    }

    /**
     * Get the dng raw size.
     * @param cs the picture orientation.
     * @return the raw size.
     */
    public static Size getRawSize(CameraCharacteristics cs) {
        Rect activeArray = null;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            activeArray = getValueFromKey(cs,
                    CameraCharacteristics.SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE);
        }
        if (activeArray == null) {
            LogHelper.e(TAG, "[getRawSize], get raw size error");
            return null;
        }
        int rawWidth = activeArray.width();
        int rawHeight = activeArray.height();
        LogHelper.d(TAG, "[getRawSize], rawWidth = " + rawWidth + ", rawHeight = " + rawHeight);
        return new Size(rawWidth, rawHeight);
    }

    /**
     * Get the value from characteristics.
     * @param characteristics the camera characteristics.
     * @param key the key needed.
     * @param <T> the generic type returned.
     * @return the value of key.
     */
    public static <T> T getValueFromKey(CameraCharacteristics characteristics,
                                  CameraCharacteristics.Key<T> key) {
        T value = null;
        try {
            value = characteristics.get(key);
            if (value == null) {
                LogHelper.e(TAG, key.getName() + "was null");
            }
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, key.getName() + " was not supported by this device");
        }

        return value;
    }

    /**
     * Get the list available from characteristics.
     * @param characteristics the camera characteristics.
     * @return the list of available capablities.
     */
    public static List<Integer> getAvailableCapablities(CameraCharacteristics characteristics) {
        CameraCharacteristics.Key<int[]> key = CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES;
        int[] availableCaps = DngUtils.getValueFromKey(characteristics, key);
        if (availableCaps == null) {
            LogHelper.i(TAG, "The camera available capabilities is null");
            return new ArrayList<Integer>();
        }

        List<Integer> capList = new ArrayList<Integer>(availableCaps.length);
        String capString = "";
        for (int cap : availableCaps) {
            capList.add(cap);
            capString += (cap + ", ");
        }
        LogHelper.d(TAG, "The camera available capabilities are:" + capString);
        return capList;
    }

    /**
     * Get the content value.
     * @param shutterTime the image.
     * @param filePath the file path.
     * @param width the raw image width.
     * @param height the raw image height.
     * @param orientation the raw image orientation.
     * @return the contentValues.
     */
    public static ContentValues getContentValue(long shutterTime, String filePath, int width,
                                                int height, int orientation) {
        String title = generateTitle(shutterTime);
        String filename = title + ".dng";
        String mime = "image/x-adobe-dng";
        String path = filePath + '/' + filename;

        ContentValues rawContentValues = new ContentValues();
        rawContentValues.put(MediaStore.Images.ImageColumns.DATE_TAKEN, shutterTime);
        rawContentValues.put(MediaStore.Images.ImageColumns.TITLE, title);
        rawContentValues.put(MediaStore.Images.ImageColumns.DISPLAY_NAME, filename);
        rawContentValues.put(MediaStore.Images.ImageColumns.DATA, path);
        rawContentValues.put(MediaStore.Images.ImageColumns.MIME_TYPE, mime);

        rawContentValues.put(MediaStore.Images.ImageColumns.WIDTH, width);
        rawContentValues.put(MediaStore.Images.ImageColumns.HEIGHT, height);
        rawContentValues.put(MediaStore.Images.ImageColumns.ORIENTATION, orientation);
        return rawContentValues;
    }

    /**
     * Check dng capture size is available or not.
     * @param characteristics the camera characteristics.
     * @return capture size is available.
     */
    public static boolean isDngCaptureSizeAvailable(CameraCharacteristics characteristics) {
        StreamConfigurationMap config = characteristics
                .get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] rawSizes = config.getOutputSizes(ImageFormat.RAW_SENSOR);
        if (rawSizes == null) {
            LogHelper.e(TAG, "[isDngCaptureSizeAvailable]" +
                    " No capture sizes available for raw format");
            return false;
        }
        for (int i = 0; i < rawSizes.length; i++) {
            LogHelper.d(TAG, "[isDngSupported] raw supported size:" + rawSizes[i]);
        }

        Rect activeArray = getValueFromKey(characteristics,
                CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
        if (activeArray == null) {
            LogHelper.e(TAG, "[isDngSupported] Active array is null");
            return false;
        } else {
            LogHelper.d(TAG, "[isDngSupported] Active array is:" + activeArray);
            Size activeArraySize = new Size(activeArray.width(), activeArray.height());
            boolean contain = false;
            for (Size size : rawSizes) {
                if (size.getWidth() == activeArraySize.getWidth()
                        && size.getHeight() == activeArraySize.getHeight()) {
                    contain = true;
                    break;
                }
            }
            if (!contain) {
                LogHelper.e(TAG, "[isDngSupported]"
                        + " Aavailable sizes for RAW format do not include active array size");
                return false;
            }
        }
        return true;
    }

    private static String generateTitle(long dateTaken) {
        Date date = new Date(dateTaken);
        String result = new SimpleDateFormat(IMAGE_FORMAT).format(date);
        return result;
    }
}
