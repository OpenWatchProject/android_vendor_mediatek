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

package com.mediatek.camera.common.utils;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.media.Image;
import android.net.Uri;
import android.os.Build;
import android.provider.MediaStore;
import android.support.annotation.RequiresApi;
import android.text.TextUtils;
import android.view.Display;
import android.view.KeyEvent;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import com.google.common.base.Joiner;
import com.google.common.base.Preconditions;
import com.google.common.base.Splitter;
import com.google.common.base.Strings;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.exif.ExifInterface;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.DeviceSpec;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.widget.Rotatable;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.portability.WifiDisplayStatusEx;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nonnull;

/**
 * Collection of utility functions related to camera.
 */
public final class CameraUtil {
    private static final Tag TAG = new Tag(CameraUtil.class.getSimpleName());
    private static final int UNKNOWN = -1;
    private static final String DEVICE_TYPE_SPLITTER = "#";
    private static final int ORIENTATION_UNKNOWN = -1;
    private static final double ASPECT_TOLERANCE = 0.02;
    private static final String PICTURE_RATIO_4_3 = "1.3333";
    private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
    private static final String FALSE = "false";
    private static AlertDialog sAlertDialog;

    /**
     * Unspecified camera error.
     * @see Camera.ErrorCallback
     */
    public static final int CAMERA_ERROR_UNKNOWN = 1;

    /**
     * Camera was disconnected due to use by higher priority user.
     * @see Camera.ErrorCallback
     */
    public static final int CAMERA_ERROR_EVICTED = 2;

    /**
     * Media server died. In this case, the application must release the
     * Camera object and instantiate a new one.
     * @see Camera.ErrorCallback
     */
    public static final int CAMERA_ERROR_SERVER_DIED = 100;
    /**
     * If device policy has disabled the camera.
     */
    public static final int CAMERA_HARDWARE_EXCEPTION = 1000;

    /**
     * If camera open fail.
     */
    public static final int CAMERA_OPEN_FAIL = 1050;

    /**
     * TableList used for query db.
     */
    public static enum TableList {
        FILE_TABLE,
        VIDEO_TABLE,
        IMAGE_TABLE
    }
    /**
     * Format Device Type.
     * @param deviceUsage specified device usage (e.g. Single, Dual, PIP, ...)
     * @param apiType specified camera api type (e.g. API1, API2)
     * @return a formatted device type with specified splitter.
     */
    public static String formatDeviceType(@Nonnull String deviceUsage,
                                          @Nonnull String apiType) {
        return Joiner.on(DEVICE_TYPE_SPLITTER).join(deviceUsage, apiType);
    }

    /**
     * Split device type to list.
     * @param deviceType been split device type.
     * @return A list of device type.
     */
    public static List<String> splitDeviceType(String deviceType) {
        Preconditions.checkArgument(!Strings.isNullOrEmpty(deviceType),
                "splitDeviceType with device type is null or empty!");
        Splitter splitter = Splitter.on(DEVICE_TYPE_SPLITTER).omitEmptyStrings().trimResults();
        return splitter.splitToList(deviceType);
    }

    /**
     * Whether the two size has the same aspect ratio.
     *
     * @param firstSize first size.
     * @param secondSize second size.
     * @return Whether the two size has the same aspect ratio.
     */
    public static boolean isTheSameAspectRatio(@Nonnull Size firstSize,
                                               @Nonnull Size secondSize) {
        if (firstSize == null || secondSize == null) {
            return false;
        }
        double firstAspectRatio = ((double) firstSize.getWidth()) / firstSize.getHeight();
        double secondAspectRatio = ((double) secondSize.getWidth()) / secondSize.getHeight();
        return Math.abs(firstAspectRatio - secondAspectRatio) <= 0.02;
    }

    /**
     * Check whether two strings are the same.
     * @param firstStr the first string.
     * @param secondStr the second string.
     * @return whether the two strings are the same.
     */
    public static boolean isTheSameString(@Nonnull String firstStr,
                                       @Nonnull String secondStr) {
        Preconditions.checkArgument(!Strings.isNullOrEmpty(firstStr),
                "isTheSameDeviceType with first device type is null or empty!");
        Preconditions.checkArgument(!Strings.isNullOrEmpty(secondStr),
                "isTheSameDeviceType with second device type is null or empty!");
        return firstStr.equalsIgnoreCase(secondStr);
    }

    /**
     * Get recording rotation.
     * @param orientation the g-sensor's orientation.
     * @param cameraId the camera id.
     * @return the recording rotation.
     */
    public static int getRecordingRotation(int orientation, int cameraId) {
        LogHelper.d(TAG, "[getRecordingRotation]orientation = " + orientation +
                ",cameraId = " + cameraId);
        int rotation;
        if (orientation != OrientationEventListener.ORIENTATION_UNKNOWN) {
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(cameraId, info);
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                rotation = (info.orientation - orientation + 360) % 360;
            } else { // back-facing camera
                rotation = (info.orientation + orientation) % 360;
            }
        } else {
            // Get the right original orientation
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(cameraId, info);
            rotation = info.orientation;
        }

        return rotation;
    }

    /**
     * Get current camera display rotation.
     * @param activity camera activity.
     * @return the activity orientation.
     */
    public static int getDisplayRotation(Activity activity) {
        if (CameraUtil.isTablet() || WifiDisplayStatusEx.isWfdEnabled(activity)) {
            int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
            switch (rotation) {
                case Surface.ROTATION_0:
                    return 0;
                case Surface.ROTATION_90:
                    return 90;
                case Surface.ROTATION_180:
                    return 180;
                case Surface.ROTATION_270:
                    return 270;
                default:
                    return 0;
            }
        } else {
            return 0;
        }
    }

    /**
     * get activity display orientation.
     * @param degrees activity rotation.
     * @param cameraId the preview camera id.
     * @return according camera id and display rotation to calculate the orientation.
     */
    public static int getDisplayOrientation(int degrees, int cameraId) {
        // See android.hardware.Camera.setDisplayOrientation for
        // documentation.
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        return result;
    }

    /**
     * Get the jpeg orientation from the data.
     * @param data the resource data
     * @return the orientation of the data,such as 0/90/180/270;
     */
    public static int getOrientationFromExif(byte[] data) {
        int orientation = 0;
        ExifInterface exifInterface = new ExifInterface();
        try {
            exifInterface.readExif(data);
        } catch (IOException e) {
            e.printStackTrace();
        }
        Integer value = exifInterface.getTagIntValue(ExifInterface.TAG_ORIENTATION);
        if (value == null) {
            return orientation;
        }
        switch (value.shortValue()) {
            case ExifInterface.Orientation.TOP_LEFT:
                orientation = 0;
                break;
            case ExifInterface.Orientation.RIGHT_TOP:
                orientation = 90;
                break;
            case ExifInterface.Orientation.BOTTOM_LEFT:
                orientation = 180;
                break;
            case ExifInterface.Orientation.RIGHT_BOTTOM:
                orientation = 270;
                break;
            default:
                orientation = 0;
        }
        return orientation;
    }

    /**
     * Get the jpeg size form the jpegData.
     * @param jpegData the resource data
     * @return the size of the jpegData.
     */
    public static Size getSizeFromExif(byte[] jpegData) {
        if (jpegData != null) {
            ExifInterface exif = new ExifInterface();
            try {
                exif.readExif(jpegData);
            } catch (IOException e) {
                LogHelper.w(TAG, "Failed to read EXIF data", e);
            }

            if (exif != null) {
                Integer widthValue = exif.getTagIntValue(ExifInterface.TAG_IMAGE_WIDTH);
                Integer heightValue = exif.getTagIntValue(ExifInterface.TAG_IMAGE_LENGTH);
                if (widthValue != null && heightValue != null) {
                    return new Size(widthValue.intValue(), heightValue.intValue());
                }
            }
        }
        return new Size(0, 0);
    }

    /**
     * Get the exif from sdk exif interface.
     * @param path the sdcard path.
     * @return the exif.
     */
    public static Size getSizeFromSdkExif(String path) {
        int width = 0;
        int height = 0;
        try {
            android.media.ExifInterface exifInterface = new android.media.ExifInterface(path);
            width = exifInterface.getAttributeInt(android.media.ExifInterface.TAG_IMAGE_WIDTH, 0);
            height = exifInterface.getAttributeInt(android.media.ExifInterface.TAG_IMAGE_LENGTH, 0);
        } catch (IOException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[getSizeFromSdkExif] width = " + width + ",height = " + height);
        return new Size(width, height);
    }
    /**
     * get the correct rotation for jpeg.
     * @param cameraId the camera id.
     * @param sensorOrientation current g-sensor orientation.
     * @return the rotation of the jpeg.
     */
    public static int getJpegRotation(int cameraId, int sensorOrientation) {
        //if sensorOrientation unknown, set the orientation to 0, and then
        //get the sensor install orientation.
        int rotation;
        if (sensorOrientation == ORIENTATION_UNKNOWN) {
            sensorOrientation = 0;
        }
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        if (Camera.CameraInfo.CAMERA_FACING_FRONT == info.facing) {
            rotation = (info.orientation - sensorOrientation + 360) % 360;
        } else {
            rotation = (info.orientation + sensorOrientation) % 360;
        }
        return rotation;
    }

    /**
     * Rotate the view orientation.
     * @param view The view need to rotated.
     * @param orientation The rotate orientation value.
     * @param animation Is need animation when rotate.
     */
    public static void rotateViewOrientation(View view, int orientation, boolean animation) {
        if (view == null) {
            return;
        }
        if (view instanceof Rotatable) {
            ((Rotatable) view).setOrientation(orientation, animation);
        } else if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0, count = group.getChildCount(); i < count; i++) {
                rotateViewOrientation(group.getChildAt(i), orientation, animation);
            }
        }
    }

    /**
     * Once the view's parent is a rotatelayout, the rotate of the the view is not same with others.
     * @param activity App activity instance.
     * @param view The view need to rotated.
     * @param orientation The rotate orientation value.
     * @param animation Is need animation when rotate.
     */
    public static void rotateRotateLayoutChildView(Activity activity, View view, int orientation,
                                         boolean animation) {
        int displayRotation = CameraUtil.getDisplayRotation(activity);
        if (displayRotation == 270 || displayRotation == 180) {
            orientation += 180;
        }
        CameraUtil.rotateViewOrientation(view, orientation, animation);
    }

    /**
     * Calculate current device screen orientation.
     * @param activity Current app activity.
     * @return ActivityInfo's screen orientation value.
     */
    public static int calculateCurrentScreenOrientation(Activity activity) {
        int displayRotation = CameraUtil.getDisplayRotation(activity);
        LogHelper.d(TAG, "calculateCurrentScreenOrientation displayRotation = " + displayRotation);
        if (displayRotation == 0) {
            return ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
        } else if (displayRotation == 90) {
            return ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
        } else if (displayRotation == 180) {
            return ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
        } else if (displayRotation == 270) {
            return ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
        }
        return ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
    }

    /**
     * Calculate rotatelayout compensate orientation according the activity' orientation.
     * @param activity Current app activity.
     * @return Compensate orientation value, it will be one of 0, 90, 180 ,270.
     */
    public static int calculateRotateLayoutCompensate(Activity activity) {
        int displayRotation = CameraUtil.getDisplayRotation(activity);
        LogHelper.d(TAG, "calculateRotateLayoutCompensate displayRotation = " + displayRotation);
        if (displayRotation == 0) {
            return 0;
        } else if (displayRotation == 90) {
            return 90;
        } else if (displayRotation == 180) {
            return 0;
        } else if (displayRotation == 270) {
            return 90;
        }
        return ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
    }

    /**
     * Judge if current device has navigation bar or not.
     * @param activity The activity instance.
     * @return True if device has navigation bar.
     *         False if device has no navigation bar.
     */
    public static boolean isHasNavigationBar(Activity activity) {
        Point size = new Point();
        Point realSize = new Point();
        activity.getWindowManager().getDefaultDisplay().getSize(size);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            activity.getWindowManager().getDefaultDisplay().getRealSize(realSize);
        } else {
            Display display = activity.getWindowManager().getDefaultDisplay();

            Method mGetRawH = null;
            Method mGetRawW = null;

            int realWidth = 0;
            int realHeight = 0;

            try {
                mGetRawW = Display.class.getMethod("getRawWidth");
                mGetRawH = Display.class.getMethod("getRawHeight");
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            }

            try {
                realWidth = (Integer) mGetRawW.invoke(display);
                realHeight = (Integer) mGetRawH.invoke(display);
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
            realSize.set(realWidth, realHeight);
        }
        if (realSize.equals(size)) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * Get current device navigation bar height.
     * @param activity The activity instance.
     * @return If current device has a navigation bar ,return it's height.
     *         If current device has no navigation bar, return -1.
     */
    public static int getNavigationBarHeight(Activity activity) {
        if (isHasNavigationBar(activity)) {
            //get navigation bar height.
            int resourceId = activity.getResources().getIdentifier(
                    "navigation_bar_height", "dimen", "android");
            int navigationBarHeight = activity.getResources()
                    .getDimensionPixelSize(resourceId);
            return navigationBarHeight;
        } else {
            return -1;
        }
    }

    /**
     * This function used to charge whether the column exist in the table.
     * @param activity the Activity for get resolver.
     * @param table the file table
     * @param column the db column.
     * @return the find result.
     */
    public static boolean isColumnExistInDB(Activity activity, TableList table, String column) {
        LogHelper.d(TAG, "[isColumnExistInDB] + table = " + table + " column " + column);
        Uri baseUri = null;
        switch (table) {
            case FILE_TABLE:
                baseUri = MediaStore.Files.getContentUri("external");
                break;
            case VIDEO_TABLE:
                baseUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                break;
            case IMAGE_TABLE:
                baseUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                break;
            default:
                baseUri = null;
        }
        if (baseUri == null) {
            return false;
        }
        Uri query = baseUri.buildUpon().appendQueryParameter("limit", "1").build();
        ContentResolver resolver = activity.getContentResolver();
        Cursor cursor = null;
        boolean isInDB = false;
        int index = -1;
        try {
            cursor = resolver.query(query, null, null, null, null);
            if (cursor != null) {
                index = cursor.getColumnIndex(column);
            }
            isInDB = index != -1;
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        LogHelper.d(TAG, "[isColumnExistInDB] - index = " + index + " isInDB " + isInDB);
        return isInDB;
    }

    /**
     * Prepare matrix to transfer view point to native preview point.
     * @param matrix The matrix.
     * @param mirror Whether need mirror or not. For front camera,should the point be mirrored.
     * @param displayOrientation The current displayOrientation.
     * @param viewWidth The preview width.
     * @param viewHeight The preview height.
     */
    public static void prepareMatrix(Matrix matrix, boolean mirror, int displayOrientation,
                                     int viewWidth, int viewHeight) {
        LogHelper.d(TAG, "prepareMatrix mirror =" + mirror + " displayOrientation=" +
                displayOrientation  + " viewWidth=" + viewWidth + " viewHeight=" + viewHeight);
        // Need mirror for front camera.
        matrix.setScale(mirror ? -1 : 1, 1);
        // This is the value for android.hardware.Camera.setDisplayOrientation.
        matrix.postRotate(displayOrientation);
        // Camera driver coordinates range from (-1000, -1000) to (1000, 1000).
        // UI coordinates range from (0, 0) to (width, height).
        matrix.postScale(viewWidth / 2000f, viewHeight / 2000f);
        matrix.postTranslate(viewWidth / 2f, viewHeight / 2f);
    }

    /**
     *Return the clamped value.
     * @param x The value need to clamp.
     * @param min The min value to clamp.
     * @param max The max value to clamp.
     * @return The clamped value.
     */
    public static int clamp(int x, int min, int max) {
        if (x > max) {
            return max;
        }
        if (x < min) {
            return min;
        }
        return x;
    }

    /**
     * Change RectF from Rect.
     * @param rectF The source rect.
     * @param rect The destination rect.
     */
    public static void rectFToRect(RectF rectF, Rect rect) {
        rect.left = Math.round(rectF.left);
        rect.top = Math.round(rectF.top);
        rect.right = Math.round(rectF.right);
        rect.bottom = Math.round(rectF.bottom);
    }

    /**
     * Copied from android.hardware.Camera
     * Splits a comma delimited string to an ArrayList of String.
     * @param str the string to be split.
     * @return null if the passing string is null or the size is 0.
     */
    public static ArrayList<String> splitString(String str) {
        ArrayList<String> substrings = null;
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            substrings = new ArrayList<>();
            for (String s : splitter) {
                substrings.add(s);
            }
        }
        return substrings;
    }

    /**
     * Splits a comma delimited string to an ArrayList of Integer.
     * Return null if the passing string is null or the size is 0.
     * @param str the str to be split.
     * @return an array of int.
     */
    public static ArrayList<Integer> splitInt(String str) {
        if (str == null) {
            return null;
        }
        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<Integer> substrings = new ArrayList<>();
        for (String s : splitter) {
            substrings.add(Integer.parseInt(s));
        }
        if (substrings.size() == 0) {
            return null;
        }
        return substrings;
    }

    /**
     * Get the smallest size which aspect ratio is the same with target size.
     * @param supportedSizes supported sizes.
     * @param targetSize target size.
     * @param needMinSize need minimal size.
     * @return the smallest size.
     */
    public static Size getSizeByTargetSize(
            List<String> supportedSizes,
            Size targetSize,
            boolean needMinSize) {
        if (targetSize == null || supportedSizes == null || supportedSizes.size() <= 0) {
            return null;
        }
        Size resultSize = new Size(0, 0);
        long resultSizeArea = needMinSize ? Integer.MAX_VALUE : 0;

        double targetSizeRatio = ((double) targetSize.getWidth()) / targetSize.getHeight();
        for (String sizeString : supportedSizes) {
            Size size = getSize(sizeString);
            double ratio = ((double) size.getWidth()) / size.getHeight();
            if (Math.abs(ratio - targetSizeRatio) <= 0.02) {
                long tempSizeArea = size.getWidth() * size.getHeight();
                if (needMinSize && tempSizeArea < resultSizeArea) {
                    resultSize = size;
                    resultSizeArea = size.getWidth() * size.getHeight();
                }
                if (!needMinSize && tempSizeArea > resultSizeArea) {
                    resultSize = size;
                    resultSizeArea = size.getWidth() * size.getHeight();
                }
            }
        }
        return resultSize;
    }

    /**
     * Get the smallest Size which aspect ratio is the same with target size.
     * @param supportedSizes supported sizes.
     * @param targetSize target size.
     * @param needMinSize need minimal size.
     * @return the smallest size.
     */
    public static Size getSizeByTargetSize(
            List<Camera.Size> supportedSizes,
            Camera.Size targetSize,
            boolean needMinSize) {
        if (targetSize == null || supportedSizes == null || supportedSizes.size() <= 0) {
            return null;
        }
        Size resultSize = new Size(0, 0);
        long resultSizeArea = needMinSize ? Integer.MAX_VALUE : 0;
        double targetSizeRatio = ((double) targetSize.width) / targetSize.height;
        for (Camera.Size cameraSize : supportedSizes) {
            Size size = new Size(cameraSize.width, cameraSize.height);
            double ratio = (double) cameraSize.width / cameraSize.height;
            if (Math.abs(ratio - targetSizeRatio) <= 0.02) {
                long tempSizeArea = size.getWidth() * size.getHeight();
                if (needMinSize && tempSizeArea < resultSizeArea) {
                    resultSize = size;
                    resultSizeArea = size.getWidth() * size.getHeight();
                }
                if (!needMinSize && tempSizeArea > resultSizeArea) {
                    resultSize = size;
                    resultSizeArea = size.getWidth() * size.getHeight();
                }
            }
        }
        return resultSize;
    }

    /**
     * Filter supported sizes by peak size.
     * @param originalSupportedSizes original supported sizes.
     * @param peakSize the peak size.
     * @return result supported sizes.
     */
    public static ArrayList<Size> filterSupportedSizes(ArrayList<Size> originalSupportedSizes,
                                                Size peakSize) {
        ArrayList<Size> resultSizes = new ArrayList<>();
        if (peakSize == null || originalSupportedSizes == null ||
                originalSupportedSizes.size() <= 0) {
            return resultSizes;
        }
        for (Size size : originalSupportedSizes) {
            if (size.getWidth() <= peakSize.getWidth() &&
                    size.getHeight() <= peakSize.getHeight()) {
                resultSizes.add(size);
            }
        }
        return resultSizes;
    }

    /**
     * Get the best match preview size which's the ratio is closely picture ratio and screen ratio.
     *
     * @param activity                 current activity.
     * @param sizes                    all of supported preview size.
     * @param previewRatio             the picture ratio.
     * @param needMatchTargetPanelSize whether need match the panel sizes.
     * @return the best match preview size.
     */
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    public static Size getOptimalPreviewSize(Activity activity,
                                             List<Size> sizes,
                                             double previewRatio,
                                             boolean needMatchTargetPanelSize) {
        //split preview sizes to two group equal ratio and nearly ratio
        List<Size> sizeEqualRatio = new ArrayList<>();
        List<Size> sizeNearRatio = new ArrayList<>();
        for (Size size : sizes) {
            if ((double) size.getWidth() / size.getHeight() == previewRatio) {
                sizeEqualRatio.add(size);
            } else {
                sizeNearRatio.add(size);
            }
        }

        WindowManager wm = (WindowManager) activity.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        int panelWidth = Math.max(point.x, point.y);
        int panelHeight = Math.min(point.x, point.y);

        Size optimalSize = null;
        if (needMatchTargetPanelSize) {
            optimalSize = findBestMatchPanelSize(
                    sizeEqualRatio, previewRatio, panelWidth, panelHeight);
            if (optimalSize == null) {
                optimalSize = findBestMatchPanelSize(
                        sizeNearRatio, previewRatio, panelWidth, panelHeight);
            }
            if (optimalSize != null) {
                return optimalSize;
            }
        }

        optimalSize = findClosestPanelSize(
                sizeEqualRatio, previewRatio, panelWidth, panelHeight);
        if (optimalSize == null) {
            optimalSize = findClosestPanelSize(
                    sizeNearRatio, previewRatio, panelWidth, panelHeight);
        }
        if (optimalSize != null) {
            return optimalSize;
        }

        double minDiffHeight = Double.MAX_VALUE;
        if (optimalSize == null) {
            LogHelper.w(TAG, "[getPreviewSize] no preview size match the aspect ratio : " +
                    previewRatio + ", then use standard 4:3 for preview");
            previewRatio = Double.parseDouble(PICTURE_RATIO_4_3);
            for (Size size : sizes) {
                double ratio = (double) size.getWidth() / size.getHeight();
                if (Math.abs(ratio - previewRatio) > ASPECT_TOLERANCE) {
                    continue;
                }
                if (Math.abs(size.getHeight() - panelHeight) < minDiffHeight) {
                    optimalSize = size;
                    minDiffHeight = Math.abs(size.getHeight() - panelHeight);
                }
            }
        }
        return optimalSize;
    }

    //find the preview size which is closest to panel size
    private static Size findClosestPanelSize(List<Size> sizes,
                                           double targetRatio, int panelWidth, int panelHeight) {

        //find out a size match aspect ratio and size
        double minDiffHeight = Double.MAX_VALUE;
        double minDiffWidth = Double.MAX_VALUE;
        Size optimalSize = null;
        for (Size size : sizes) {
            double ratio = (double) size.getWidth() / size.getHeight();
            if (Math.abs(targetRatio - ratio) > ASPECT_TOLERANCE) {
                continue;
            }
            if (Math.abs(size.getHeight() - panelHeight) < minDiffHeight) {
                optimalSize = size;
                minDiffHeight = Math.abs(size.getHeight() - panelHeight);
                minDiffWidth = Math.abs(size.getWidth() - panelWidth);
            } else if (Math.abs(size.getHeight() - panelHeight) == minDiffHeight && Math.abs(size
                    .getWidth() - panelWidth) < minDiffWidth) {
                optimalSize = size;
                minDiffWidth = Math.abs(size.getWidth() - panelWidth);
            }
        }
        return optimalSize;
    }

    //find the preview size which is greater or equal to panel size and closest to panel size
    private static Size findBestMatchPanelSize(List<Size> sizes,
            double targetRatio, int panelWidth, int panelHeight) {
        double minDiff = Double.MAX_VALUE;
        double minDiffWidth = Double.MAX_VALUE;
        double panelAspectRatio = (double) panelWidth / panelHeight;
        Size bestMatchSize = null;
        for (Size size : sizes) {
            double ratio = (double) size.getWidth() / size.getHeight();
            // filter out the size which not tolerated by target ratio
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            // when target aspect ratio is the same with panel size aspect ratio
            // find no less than panel size's preview size
            if (Math.abs(panelAspectRatio - targetRatio) <= ASPECT_TOLERANCE &&
                    (panelHeight > size.getHeight() || panelWidth > size.getWidth())) {
                // filter out less than panel size
                continue;
            }
            // find the size closest to panel size
            if (Math.abs(size.getHeight() - panelHeight) < minDiff) {
                bestMatchSize = size;
                minDiff = Math.abs(size.getHeight() - panelHeight);
                minDiffWidth = Math.abs(size.getWidth() - panelWidth);
            } else if ((Math.abs(size.getHeight() - panelHeight) == minDiff)
                    && Math.abs(size.getWidth() - panelWidth) < minDiffWidth) {
                bestMatchSize = size;
                minDiffWidth = Math.abs(size.getWidth() - panelWidth);
            }
        }
        return bestMatchSize;
    }

    private static boolean isDisplayRotateSupported(Camera.Parameters parameters) {
        String supported = parameters.get(KEY_DISP_ROT_SUPPORTED);
        if (supported == null || FALSE.equals(supported)) {
            return false;
        }
        return true;
    }

    /**
     * Given an image reader, extracts the JPEG image bytes and then closes the
     * reader.
     *
     * @param image the reader to read the JPEG data from.
     * @return The bytes of the JPEG image. Newly allocated.
     */
    @TargetApi(Build.VERSION_CODES.KITKAT)
    public static byte[] acquireJpegBytesAndClose(Image image) {
        ByteBuffer buffer;
        if (image.getFormat() == ImageFormat.JPEG) {
            Image.Plane plane0 = image.getPlanes()[0];
            buffer = plane0.getBuffer();
        } else {
            throw new RuntimeException("Unsupported image format.");
        }
        byte[] imageBytes = new byte[buffer.remaining()];
        buffer.get(imageBytes);
        buffer.rewind();
        image.close();
        return imageBytes;
    }

    /**
     * Acquire rgb format buffer form image.
     * @param image the image where buffer from
     * @return byte buffer from image.
     */
    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    public static ByteBuffer acquireRgbBufferAndClose(Image image) {
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
    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    public static byte[] getContinuousRgbDataFromImage(Image image) {
        LogHelper.d(TAG, "getContinuousRGBADataFromImage begin");
        if (image.getFormat() != PixelFormat.RGBA_8888 &&
                image.getFormat() != PixelFormat.RGB_888) {
            LogHelper.e(TAG, "error format = " + image.getFormat());
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
        LogHelper.d(TAG, "getContinuousRGBADataFromImage end");
        return data;
    }

    /**
     * Build size to string.
     *
     * @param size the size want to express with string.
     * @return an string to express the size.
     */
    public static String buildSize(Size size) {
        if (size != null) {
            return "" + size.getWidth() + "x" + size.getHeight();
        } else {
            return "null";
        }
    }

    /**
     * Get size from string.
     *
     * @param sizeString the size use string express.
     * @return the size from string.
     */
    public static Size getSize(String sizeString) {
        Size size = null;
        if (sizeString != null) {
            int index = sizeString.indexOf('x');
            if (index != UNKNOWN) {
                int width = Integer.parseInt(sizeString.substring(0, index));
                int height = Integer.parseInt(sizeString.substring(index + 1));
                size = new Size(width, height);
            }
        }
        LogHelper.d(TAG, "getSize(" + sizeString + ") return " + size);
        return size;
    }

    /**
     * Judge if current device is tablet or not.
     * @return true: device is tablet.
     *         false: device is phone.
     */
    public static boolean isTablet() {
        boolean isTablet = "tablet"
                .equals(SystemProperties.getString("ro.build.characteristics", null));
        LogHelper.d(TAG, "isTablet = " + isTablet);
        return isTablet;
    }

    /**
     * show a dialog and finish the activity.
     * @param activity which activity need finish.
     * @param error which error.
     */
    public static void showErrorInfoAndFinish(Activity activity, int error) {
        String errorMessage = null;
        Resources resources = activity.getResources();
        switch (error) {
            case CAMERA_ERROR_UNKNOWN:
            case CAMERA_ERROR_EVICTED:
            case CAMERA_ERROR_SERVER_DIED:
            case CAMERA_OPEN_FAIL:
                errorMessage = resources.getString(
                        resources.getIdentifier("cannot_connect_camera_new", "string",
                                activity.getPackageName()));
                break;
            case CAMERA_HARDWARE_EXCEPTION:
                errorMessage = resources.getString(
                        resources.getIdentifier("camera_disabled", "string",
                                activity.getPackageName()));
                break;
            default:
                errorMessage = resources.getString(
                        resources.getIdentifier("cannot_connect_camera_new", "string",
                                activity.getPackageName()));
                break;
        }
        String name = resources.getString(
                resources.getIdentifier("dialog_ok", "string", activity.getPackageName()));
        showErrorAndFinish(activity, errorMessage, name);
    }

    /**
     * get the correct rotation for jpeg.
     * @param cameraId the camera id.
     * @param sensorOrientation current g-sensor orientation.
     * @param context current context.
     * @return the rotation of the jpeg.
     */
    public static int getJpegRotationFromDeviceSpec(int cameraId, int sensorOrientation,
            Context context) {
        //if sensorOrientation unknown, set the orientation to 0, and then
        //get the sensor install orientation.
        int rotation;
        if (sensorOrientation == ORIENTATION_UNKNOWN) {
            sensorOrientation = 0;
        }
        Camera.CameraInfo info = getCameraInfoFromDeviceSpec(context, cameraId);
        if (Camera.CameraInfo.CAMERA_FACING_FRONT == info.facing) {
            rotation = (info.orientation - sensorOrientation + 360) % 360;
        } else {
            rotation = (info.orientation + sensorOrientation) % 360;
        }
        return rotation;
    }

    /**
     * get activity display orientation.
     * @param degrees activity rotation.
     * @param cameraId the preview camera id.
     * @param context current context.
     * @return according camera id and display rotation to calculate the orientation.
     */
    public static int getDisplayOrientationFromDeviceSpec(int degrees, int cameraId,
            Context context) {
        // See android.hardware.Camera.setDisplayOrientation for
        // documentation.
        Camera.CameraInfo info = getCameraInfoFromDeviceSpec(context, cameraId);
        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        return result;
    }
    /**
     * Delete stereo camera thumb.
     * @param context the application context.
     */
    public static void delStereoThumb(Context context) {
        LogHelper.d(TAG, "[delStereoThumb]+");
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor;
        try {
            cursor = MediaStore.Images.Media.query(resolver,
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                    null,
                    "camera_refocus=?", new String[]{"2"}, null);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }
        if (null == cursor) {
            LogHelper.d(TAG, "cursor is null!");
            return;
        }
        if (cursor.getCount() == 0) {
            cursor.close();
            LogHelper.d(TAG, "cursor count is 0!");
            return;
        }
        LogHelper.d(TAG, "[delStereoThumb] cursor " + cursor.getCount());
        boolean result = false;
        while (cursor.moveToNext()) {
            long id = cursor.getLong(0);
            Uri contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
            Uri uri = ContentUris.withAppendedId(contentUri, id);
            int count = context.getContentResolver().delete(uri, null, null);
            result = count == 1;
        }
        cursor.close();
        LogHelper.i(TAG, "[delStereoThumb]-" + result);
    }

    /**
     * Get display orientation of API2;
     * @param degrees activity rotation.
     * @param cameraId the camera id.
     * @param context current context.
     * @return according camera id and display rotation to calculate the orientation.
     */
    public static int getV2DisplayOrientation(int degrees, int cameraId, Context context) {
        int result;
        CameraCharacteristics characteristics = getCameraCharacteristicsFromDeviceSpec(context,
                cameraId);
        if (characteristics == null) {
            LogHelper.e(TAG, "[getV2DisplayOrientation] characteristics is null");
            return 0;
        }
        if (degrees == OrientationEventListener.ORIENTATION_UNKNOWN) {
            LogHelper.w(TAG, "[getV2DisplayOrientation] unknown  degrees");
            return 0;
        }
        int facing = characteristics.get(CameraCharacteristics.LENS_FACING);
        int sensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        if (facing == CameraMetadata.LENS_FACING_FRONT) {
            result = (sensorOrientation + degrees) % 360;
            result = (360 - result) % 360;
        } else {
            result = (sensorOrientation - degrees + 360) % 360;
        }
        return result;
    }


    /**
     * Activity onPause will call this to do some thing that care.
     * @param activity current activity.
     */
    public static void hideAlertDialog(Activity activity) {
        if (sAlertDialog != null) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        sAlertDialog.dismiss();
                        sAlertDialog = null;
                    } catch (IllegalArgumentException e) {
                        e.printStackTrace();
                    }
                }
            });
        }
    }

    /**
     * Get camera id list by facing.
     * @param isBack true if back camera, or false.
     * @return the camera id list.
     */
    public static List<String> getCamIdsByFacing(boolean isBack) {
        int cameraNum = Camera.getNumberOfCameras();
        List<String> ids = new ArrayList<>();
        if (cameraNum > 0) {
            for (int i = 0; i < cameraNum; i++) {
                Camera.CameraInfo info = new Camera.CameraInfo();
                Camera.getCameraInfo(i, info);
                if (isBack && info.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                    ids.add(String.valueOf(i));
                }
                if (!isBack && info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    ids.add(String.valueOf(i));
                }
            }
        }
        return ids;
    }

    private static void showErrorAndFinish(final Activity activity, String message, String action) {
        LogHelper.d(TAG, "[showErrorAndFinish]");
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (activity.isFinishing() || sAlertDialog != null) {
                    return;
                }
                DialogInterface.OnClickListener buttonListener = new DialogInterface
                        .OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        LogHelper.i(TAG, "[showErrorAndFinish] on OK click, will finish activity");
                        activity.finish();
                    }
                };
                AlertDialog.Builder builder = new AlertDialog.Builder(activity).setCancelable(false)
                        .setIconAttribute(android.R.attr.alertDialogIcon)
                        .setTitle("").setMessage(message)
                        .setNeutralButton(action, buttonListener);
                if (activity.isDestroyed() || activity.isFinishing()) {
                    LogHelper.i(TAG, "[showErrorAndFinish] activity is finishing, do noting");
                    return;
                } else {
                    sAlertDialog = builder.show();
                }
            }
        });
    }

    private static Camera.CameraInfo getCameraInfoFromDeviceSpec(Context context, int cameraId) {
        DeviceSpec deviceSpec = CameraApiHelper.getDeviceSpec(context);
        ConcurrentHashMap<String, DeviceDescription> deviceDescriptionMap =
                deviceSpec.getDeviceDescriptionMap();
        DeviceDescription deviceDescription = deviceDescriptionMap.get(String.valueOf(cameraId));
        return deviceDescription.getCameraInfo();
    }

    private static CameraCharacteristics getCameraCharacteristicsFromDeviceSpec(Context context,
                                                                                int cameraId) {
        DeviceSpec deviceSpec = CameraApiHelper.getDeviceSpec(context);
        ConcurrentHashMap<String, DeviceDescription> deviceDescriptionMap =
                deviceSpec.getDeviceDescriptionMap();
        DeviceDescription deviceDescription = deviceDescriptionMap.get(String.valueOf(cameraId));
        return deviceDescription.getCameraCharacteristics();
    }

    /**
     * Check the camera id is facing front or not
     * @param context Current application context
     * @param cameraId The camera id you want to check
     * @return Return true if facing front, return false if facing back
     */
    public static boolean isCameraFacingFront(Context context, int cameraId) {
        Camera.CameraInfo info = getCameraInfoFromDeviceSpec(context, cameraId);
        return info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT;
    }

    /**
     * Judge a service is run or not.
     * @param context Current application context.
     * @param className  The service class name.
     * @return Return true if service is running, false or not.
     */
    public static boolean isServiceRun(Context context, String className) {
        boolean isRun = false;
        final int maxNum = 100;
        if (context == null || className == null) {
            LogHelper.e(TAG, "isServiceRun mContext = " + context + " className = " + className);
            return false;
        }
        try {
            ActivityManager activityManager = (ActivityManager) context
                    .getSystemService(Context.ACTIVITY_SERVICE);
            List<ActivityManager.RunningServiceInfo> serviceList = activityManager
                    .getRunningServices(maxNum);
            int size = serviceList.size();
            for (int i = 0; i < size; i++) {
                if (serviceList.get(i).service.getClassName().equals(className)) {
                    isRun = true;
                    break;
                }
            }
        } catch (Exception ex) {
            LogHelper.e(TAG, "isServiceRun " + ex);
        }
        LogHelper.d(TAG, "isServiceRun service name = " + className + " is run " + isRun);
        return isRun;
    }

    /**
     * Start service by intent.
     * @param context Current application context.
     * @param intent The intent for start service.
     */
    public static void startService(Context context, Intent intent) {
        if (context == null || intent == null) {
            LogHelper.e(TAG, "isServiceRun mContext = " + context + " intent = " + intent);
            return;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            try {
                context.startService(intent);
            } catch (Exception ex) {
                LogHelper.e(TAG, "Start service " + ex);
            }
        }
    }

    /**
     * [Add for CCT tool and bit true test] Check if enable receive keycode and do special operation
     *
     * @return true, receive keycode and do special operation, false, not receive keycode
     */
    public static boolean isSpecialKeyCodeEnabled() {
        boolean isEnable = SystemProperties.getInt("mtk.camera.app.keycode.enable", 0) == 1;
        LogHelper.d(TAG, "[isSpecialKeyCodeEnabled] isEnable = " + isEnable);
        return isEnable;
    }

    /**
     *  [Add for CCT tool and bit true test] Check if need init setting.
     * @param keyCode The key code.
     * @return True if the key code need to init setting fragement first.Otherwise false.
     */
    public static boolean isNeedInitSetting(int keyCode) {
        if (keyCode == KEYCODE_ENABLE_ZSD
                || keyCode == KEYCODE_DISABLE_ZSD) {
            return true;
        } else {
            return false;
        }
    }

    // [Add for CCT tool and bit true test] key code mapping table @{
    public static final int KEYCODE_ZOOM_IN = KeyEvent.KEYCODE_ZOOM_IN;
    public static final int KEYCODE_ZOOM_OUT = KeyEvent.KEYCODE_ZOOM_OUT;
    public static final int KEYCODE_SWITCH_TO_PHOTO = KeyEvent.KEYCODE_A;
    public static final int KEYCODE_SWITCH_TO_VIDEO = KeyEvent.KEYCODE_B;
    public static final int KEYCODE_SWITCH_CAMERA = KeyEvent.KEYCODE_C;
    public static final int KEYCODE_ENABLE_ZSD = KeyEvent.KEYCODE_D;
    public static final int KEYCODE_DISABLE_ZSD = KeyEvent.KEYCODE_E;
    // @}
}
