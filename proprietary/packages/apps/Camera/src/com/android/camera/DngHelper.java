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

package com.android.camera;


import android.content.Context;
import android.graphics.Rect;
import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.DngCreator;
import android.location.Location;
import android.media.ExifInterface;
import android.util.Log;
import android.util.Size;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * This class is to generate dng image by raw data and metadata.
 */
public class DngHelper {
    private static final String TAG = "DngHelper";
    private static Context sContext;
    // Add for Capture DNG
    private CaptureResult mRawCaptureResult;
    private CameraCharacteristics mRawCharacteristic;
    private Size mRawSize;
    private long mCaptureStartTime = 0;
    private boolean mCameraClosed = false;
    private byte[] mRawImageData;

    /**
     * the constructor of the class.
     */
    private DngHelper() {
    }

    /**
     * To create a dnghelper handler.
     */
    private static class Singleton {
        private static final DngHelper INSTANCE = new DngHelper();
    }

    /**
     * This is for get dnghelper handler.
     * @param c the application context.
     * @return handler of DngHelper.
     */
    public static DngHelper getInstance(Context c) {
        sContext = c;
        return Singleton.INSTANCE;
    }

    /**
     * To receive metadata that contains the raw attributes.
     * @param result capture result.
     * @param characteristic camera characteristics.
     */
    public void setMetadata(CaptureResult result, CameraCharacteristics characteristic) {
        Log.i(TAG, "onMetadataReceived");
        if (result == null || characteristic == null) {
            Log.i(TAG, "onMetadataReceived, invalid callback value, return null");
            return;
        }
        mRawCaptureResult = result;
        mRawCharacteristic = characteristic;
        Rect rawSize = mRawCharacteristic.get(
                CameraCharacteristics.SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE);
        if (rawSize == null) {
            Log.i(TAG, "MetadataCallback, get raw size error");
            return;
        }
        int rawWidth = rawSize.width();
        int rawHeight = rawSize.height();
        Log.i(TAG, "mRawMetadataCallback, rawWidth = " + rawWidth + "," +
                "rawHeight = " + rawHeight);
        mRawSize = new Size(rawWidth, rawHeight);
    }

    /**
     * To receive raw data.
     * @param data the raw data.
     * @param camera the camera handler.
     */
    public void setRawdata(byte[] data) {
        long rawPictureCallbackTime = System.currentTimeMillis();
        Log.d(TAG, "rawPictureCallbackTime = " + rawPictureCallbackTime + "ms");
        mRawImageData = data;
    }

    /**
     * To create dng image with the raw data and metadata.
     * @param imageOrientation the picture orientation.
     * @param location location
     * @return the dng data
     */
    public byte[] createDngImage(int imageOrientation, Location location) {
        if (mRawImageData == null || mRawCaptureResult == null) {
            return null;
        }
        ByteArrayOutputStream outputStream = null;
        byte[] dngImage = null;
        try {
            DngCreator dngCreator = new DngCreator(mRawCharacteristic, mRawCaptureResult);
            outputStream = new ByteArrayOutputStream();
            ByteBuffer rawBuffer = ByteBuffer.wrap(mRawImageData);
            Log.d(TAG, "createDngImage = " + imageOrientation);
            dngCreator.setOrientation(getDngOrientation(imageOrientation));
            if (location != null) {
                dngCreator.setLocation(location);
            }
            dngCreator.writeByteBuffer(outputStream, mRawSize, rawBuffer, 0);
            dngImage = outputStream.toByteArray();
            outputStream.close();
            rawBuffer.clear();
        } catch (IOException ex) {
            Log.e(TAG, "createDngImage, dng write error");
            dngImage = null;
        } finally {
            //set raw data to null for speed up GC
            mRawImageData = null;
            mRawCaptureResult = null;
        }
        Log.i(TAG, "createDngImage");
        return dngImage;
    }

    /**
     * To get the dng width.
     * @return the image width in pixels.
     */
    public int getRawWidth() {
        return mRawSize.getWidth();
    }

    /**
     * To get the dng height.
     * @return the height in pixels.
     */
    public int getRawHeight() {
        return mRawSize.getHeight();
    }

    private int getDngOrientation(int imageOrientation) {
        int orientation = 0;
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
}
