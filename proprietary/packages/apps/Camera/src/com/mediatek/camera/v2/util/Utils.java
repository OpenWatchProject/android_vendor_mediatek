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
package com.mediatek.camera.v2.util;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.Rect;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.media.CamcorderProfile;
import android.media.ExifInterface;
import android.media.Image;
import android.media.Image.Plane;
import android.media.MediaMetadataRetriever;
import android.util.Size;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import java.io.Closeable;
import java.io.FileDescriptor;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;

import junit.framework.Assert;

import com.android.camera.R;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.stream.pip.pipwrapping.PipEGLConfigWrapper;
import com.mediatek.camera.v2.ui.Rotatable;

public class Utils {
    public static final int ROTATION_0 = 0;
    public static final int ROTATION_90 = 90;
    public static final int ROTATION_180 = 180;
    public static final int ROTATION_270 = 270;
    private static final Tag TAG = new Tag(Utils.class.getSimpleName());
    private static final int                 UNKNOWN = -1;
    private static final String              ENABLE_LIST_HEAD = "[L];";
    private static final String              ENABLE_LIST_SEPERATOR = ";";
    public static final String               RESET_STATE_VALUE_DISABLE = "disable-value";
    /** Orientation hysteresis amount used in rounding, in degrees. */
    public static final int                  ORIENTATION_HYSTERESIS = 5;
    public static final double[]             RATIOS = new double[] { 1.3333, 1.5, 1.6667, 1.7778 };
    public static final double               ASPECT_TOLERANCE = 0.03;
    private static ImageFileNamer            sImageFileNamer;

    private static final String FORMAT_JPEG = "-superfine";
    private static final String FORMAT_AUTORAMA = "autorama";
    private static final double REF_JPEG_RESOLUTION = 320 * 240;
    private static final int BYTE_PER_REF_JPEG_RESOLUTION = 13312;
    private static final int BYTE_PER_AUTORAMA = 163840;
    private static final int BYER_DEFAULT = 1500000;

    private Utils() {

    }

    /**
     * Return the bytes is taken by one image.
     * @param format The string contains of image format and resolution.
     * @return The bytes is taken by one image.
     */
    public static int getImageSize(String format) {
        if (format == null) {
            return BYER_DEFAULT;
        }
        if (format.indexOf(FORMAT_JPEG) > 0) {
            int indexF = format.indexOf(FORMAT_JPEG);
            int indexX = format.indexOf('x');
            if (indexX > 0) {
                int width = Integer.parseInt(format.substring(0, indexX));
                int height = Integer.parseInt(format.substring(indexX + 1, indexF));
                double scale = (double) (width * height) / REF_JPEG_RESOLUTION;
                return (int) (BYTE_PER_REF_JPEG_RESOLUTION * scale);
            } else {
                return BYER_DEFAULT;
            }
        } else if (format.indexOf(FORMAT_AUTORAMA) > 0) {
            return BYTE_PER_AUTORAMA;
        } else {
            return BYER_DEFAULT;
        }
    }

    public static void initialize(Context context) {
        sImageFileNamer = new ImageFileNamer(
                context.getString(R.string.image_file_name_format));
    }

    public static double findFullscreenRatio(Context context) {
        double find = 1.3333;
        if (context != null) {
            WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            Display display = wm.getDefaultDisplay();
            Point point = new Point();
            display.getRealSize(point);

            double fullscreen;
            if (point.x > point.y) {
                fullscreen = (double) point.x / point.y;
            } else {
                fullscreen = (double) point.y / point.x;
            }
            LogHelper.i(TAG, "fullscreen = " + fullscreen + " x = " + point.x + " y = " + point.y);
            for (int i = 0; i < RATIOS.length; i++) {
                if (Math.abs(RATIOS[i] - fullscreen) < Math.abs(fullscreen - find)) {
                    find = RATIOS[i];
                }
            }
        }
        return find;
    }

    public static CameraCharacteristics getCameraCharacteristics(Activity activity,
            String cameraId) {
        CameraManager camManager = (CameraManager)
                activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            return camManager.getCameraCharacteristics(cameraId);
        } catch (CameraAccessException e) {
            LogHelper.i(TAG, "CameraCharacteristics exception : ");
            e.printStackTrace();
        }
        return null;
    }

    public static Size filterSupportedSize(List<Size> supportedSizes, Size targetSize, Size bound) {
        if (supportedSizes == null || supportedSizes.size() <= 0) {
            return null;
        }
        Comparator<Size> comparator = new SizeComparator();
        Size selectedSize = targetSize;
        if (bound == null) {
            bound = targetSize;
        }
        for (Size sz : supportedSizes) {
            if (comparator.compare(sz, bound) <= 0 && checkAspectRatiosMatch(sz, selectedSize)) {
                selectedSize = sz;
            }
        }
        return selectedSize;
    }

    public static boolean isSameAspectRatio(Size sizeLeft, Size sizeRight) {
        if (sizeLeft == null || sizeRight == null) {
            return false;
        }
        float leftRatio = ((float) sizeLeft.getWidth()) / sizeLeft.getHeight();
        float rightRatio = ((float) sizeRight.getWidth()) / sizeRight.getHeight();
        return Math.abs(leftRatio - rightRatio) < 0.0001;
    }

    /**
     * Filter more than bound's size.
     * @param originalSizes been filtered original sizes.
     * @param bound max bound size
     * @return no more than bound's size list.
     */
    public static List<Size> filterSizesByBound(List<Size> originalSizes, Size bound) {
        List<Size> resultSizes = new ArrayList<Size>();
        for (Size sz : originalSizes) {
            if (compareSize(bound, sz)) {
                resultSizes.add(sz);
            }
        }
        return resultSizes;
    }

    /**
     * Shared size comparison method used by size comparators.
     *
     * <p>Compares the number of pixels it covers.If two the areas of two sizes are same, compare
     * the widths.</p>
     */
    public static boolean compareSize(Size sizeLeft, Size sizeRight) {
        Comparator<Size> comparator = new SizeComparator();
        return comparator.compare(sizeLeft, sizeRight) >= 0;
    }

    public static List<Size> getSizeList(List<String> sizeStringList) {
        if (sizeStringList == null || sizeStringList.size() <= 0) {
            return null;
        }
        List<Size> sizeList = new ArrayList<Size>(sizeStringList.size());
        for (String size:sizeStringList) {
            sizeList.add(getSize(size));
        }
        return sizeList;
    }

    public static String buildSize(Size size) {
        if (size != null) {
            return "" + size.getWidth() + "x" + size.getHeight();
        } else {
            return "null";
        }
    }

    public static Size getSize(String sizeString) {
        Size size = null;
        int index = sizeString.indexOf('x');
        if (index != UNKNOWN) {
            int width = Integer.parseInt(sizeString.substring(0, index));
            int height = Integer.parseInt(sizeString.substring(index + 1));
            size = new Size(width, height);
        }
        LogHelper.d(TAG, "getSize(" + sizeString + ") return " + size);
        return size;
    }

    public static Size getOptimalPreviewSize(Context context,
            List<Size> sizes, double targetRatio) {
        if (sizes == null) {
            return null;
        }

        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        int targetHeight = Math.min(point.x, point.y);
        int targetWidth = Math.max(point.x, point.y);

        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;
        double minDiffWidth = Double.MAX_VALUE;
        for (Size size : sizes) {
            double ratio = (double) size.getWidth() / size.getHeight();
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            if (Math.abs(size.getHeight() - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.getHeight() - targetHeight);
                minDiffWidth = Math.abs(size.getWidth() - targetWidth);
            } else if ((Math.abs(size.getHeight() - targetHeight) == minDiff)
                    && Math.abs(size.getWidth() - targetWidth) < minDiffWidth) {
                optimalSize = size;
                minDiffWidth = Math.abs(size.getWidth() - targetWidth);
            }
        }

        return optimalSize;
    }

    /**
     * Get optimal size from size list, this size is nearest width
     * and height than other sizes.
     * @param sizes size list
     * @param width comparative width
     * @param height comparative height
     * @return optimal size
     */
    public static Size getOptimalSize(List<Size> sizes, int width, int height) {
        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;
        double minDiffWidth = Double.MAX_VALUE;
        double targetRatio = (double) width / height;
        int targetHeight = Math.min(width, height);
        int targetWidth = Math.max(width, height);

        for (Size size : sizes) {
            double ratio = (double) size.getWidth() / size.getHeight();
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            if (Math.abs(size.getHeight() - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.getHeight() - targetHeight);
                minDiffWidth = Math.abs(size.getWidth() - targetWidth);
            } else if ((Math.abs(size.getHeight() - targetHeight) == minDiff)
                    && Math.abs(size.getWidth() - targetWidth) < minDiffWidth) {
                optimalSize = size;
                minDiffWidth = Math.abs(size.getWidth() - targetWidth);
            }
        }
        LogHelper.d(TAG, "[getOptimalSize]width:" + optimalSize.getWidth() +
                ",height:" + optimalSize.getHeight());
        return optimalSize;
    }

    /**
     * Get the picture sizes with ratio is very close to the given ratio from the
     * given picture sizes, tolerance is in {@link ASPECT_TOLERANCE}.
     * @param pictureSizes The given picture sizes.
     * @param targetRatio The given ratio.
     * @return Return the picture sizes with the given ratio.
     */
    public static List<String> filterPictureSizesByRatio(List<String> pictureSizes,
            double targetRatio) {
        List<String> filteredSizes = new ArrayList<String>();
        for (int i = 0; i < pictureSizes.size(); i++) {
            Size size = getSize(pictureSizes.get(i));
            double ratio = (double) size.getWidth() / size.getHeight();
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            filteredSizes.add(pictureSizes.get(i));
        }
        return filteredSizes;
    }

    /**
     * Given the device orientation and Camera2 characteristics, this returns
     * the required JPEG rotation for this camera.
     *
     * @param deviceOrientation the device orientation in degrees.
     * @return The JPEG orientation in degrees.
     */
    public static int getJpegRotation(int deviceOrientation,
            CameraCharacteristics characteristics) {
        if (deviceOrientation == android.view.OrientationEventListener.ORIENTATION_UNKNOWN) {
            return 0;
        }
        int sensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);

        // Reverse device orientation for front-facing cameras
        boolean facingFront = characteristics.get(
                CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_FRONT;
        if (facingFront) deviceOrientation = -deviceOrientation;

        // Calculate desired JPEG orientation relative to camera orientation to make
        // the image upright relative to the device orientation
        int jpegOrientation = (sensorOrientation + deviceOrientation + 360) % 360;
        LogHelper.i(TAG, "getJpegRotation : " + jpegOrientation);
        return jpegOrientation;
    }

    public static int getRecordingRotation(int orientation,
            CameraCharacteristics characteristics) {
        int rotation = -1;
        int sensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        // Reverse device orientation for front-facing cameras
        boolean facingFront = characteristics.get(CameraCharacteristics.LENS_FACING)
                == CameraCharacteristics.LENS_FACING_FRONT;

        if (orientation != OrientationEventListener.ORIENTATION_UNKNOWN) {
            if (facingFront) {
                rotation = (sensorOrientation - orientation + 360) % 360;
            } else { // back-facing camera
                rotation = (sensorOrientation + orientation) % 360;
            }
        } else {
            // Get the right original orientation
            rotation = sensorOrientation;
        }
        return rotation;
    }

    public static void setRotatableOrientation(View view, int orientation, boolean animation) {
        if (view == null) {
            return;
        }
        if (view instanceof Rotatable) {
            ((Rotatable) view).setOrientation(orientation, animation);
        } else if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0, count = group.getChildCount(); i < count; i++) {
                setRotatableOrientation(group.getChildAt(i), orientation, animation);
            }
        }
    }

    /**
     * Get the dng orientation defined with the dng spec.
     * @param imageOrientation the image preview orientation.
     * @return dng orientation.
     */
    public static int getDngOrientation(int imageOrientation) {
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

    public static int getDisplayRotation(Context context) {
        WindowManager windowManager = (WindowManager)
                context.getSystemService(Context.WINDOW_SERVICE);
        int rotation = windowManager.getDefaultDisplay().getRotation();
        switch (rotation) {
            case Surface.ROTATION_0:
                return 0;
            case Surface.ROTATION_90:
                return 90;
            case Surface.ROTATION_180:
                return 180;
            case Surface.ROTATION_270:
                return 270;
        }
        return 0;
    }

    public static int roundOrientation(int orientation, int orientationHistory) {
        boolean changeOrientation = false;
        if (orientationHistory == OrientationEventListener.ORIENTATION_UNKNOWN) {
            changeOrientation = true;
        } else {
            int dist = Math.abs(orientation - orientationHistory);
            dist = Math.min(dist, 360 - dist);
            changeOrientation = (dist >= 45 + ORIENTATION_HYSTERESIS);
        }
        if (changeOrientation) {
            return ((orientation + 45) / 90 * 90) % 360;
        }
        return orientationHistory;
    }

    /**
     * Given an image reader, extracts the JPEG image bytes and then closes the
     * reader.
     *
     * @param reader the reader to read the JPEG data from.
     * @return The bytes of the JPEG image. Newly allocated.
     */
    public static byte[] acquireJpegBytesAndClose(Image image) {
        Assert.assertNotNull(image);
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
     * get the raw data buffer from Image.
     * Given an image reader, extracts the raw image bytes
     * and then closes the reader.
     * @param image the raw image.
     * @return The bytes of the raw image. Newly allocated.
     */
    public static byte[] acquireRawBytesAndClose(Image image) {
        Assert.assertNotNull(image);
        ByteBuffer buffer;
        if (image.getFormat() == ImageFormat.RAW_SENSOR) {
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
     * Read continuous byte from image when rowStride != pixelStride * width
     */
    public static byte[] getContinuousRGBADataFromImage(Image image) {
        LogHelper.i(TAG, "getContinuousRGBADataFromImage begin");
        if (image.getFormat() != PipEGLConfigWrapper.getInstance().getPixelFormat()) {
            LogHelper.i(TAG, "error format = " + image.getFormat());
            return null;
        }
        int format = image.getFormat();
        int width = image.getWidth();
        int height = image.getHeight();
        int rowStride, pixelStride;
        byte[] data = null;
        Plane[] planes = image.getPlanes();
        if (format == PipEGLConfigWrapper.getInstance().getPixelFormat()) {
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
        }
        LogHelper.i(TAG, "getContinuousRGBADataFromImage end");
        return data;
    }

    public static CamcorderProfile getVideoProfile(int cameraId, int quality) {
        return CamcorderProfile.get(cameraId, quality);
    }

    public static String createJpegName(long dateTaken) {
        synchronized (sImageFileNamer) {
            return sImageFileNamer.generateName(dateTaken);
        }
    }

    public static String createDngName(long dateTaken) {
        synchronized (sImageFileNamer) {
            return sImageFileNamer.generateDngName(dateTaken);
        }
    }

    /**
     * Build string which is composed of elements of the list with semicolon
     * between them. For example, if the input list array is "[auto,portrait]",
     * the returned string is "auto;portrait".
     * @param list The array of values.
     * @return Return the builded string.
     */
    public static String buildEnableList(String[] list) {
        if (list == null) {
            return null;
        }
        String listStr = null;
        if (list != null) {
            listStr = "";
            List<String> uniqueList = new ArrayList<String>();
            for (int i = 0, len = list.length; i < len; i++) {
                if (uniqueList.contains(list[i])) {
                    continue;
                }
                uniqueList.add(list[i]);
                if (i == (len - 1)) {
                    listStr += list[i];
                } else {
                    listStr += (list[i] + ENABLE_LIST_SEPERATOR);
                }
            }
        }
        LogHelper.d(TAG, "buildEnableList, return " + listStr);
        return listStr;
    }

    public static String buildSize(int width, int height) {
        return "" + width + "x" + height;
    }

    public static void closeSilently(Closeable c) {
        if (c == null) {
            return;
        }
        try {
            c.close();
        } catch (Throwable t) {
            // do nothing
            t.printStackTrace();
        }
    }

    /**
     * Calculates sensor crop region for a zoom level (zoom >= 1.0).
     *
     * @return Crop region.
     */
    public static Rect cropRegionForZoom(Activity activity, String cameraId, float zoom) {
        CameraCharacteristics characteristics = getCameraCharacteristics(activity, cameraId);
        Rect sensor = characteristics.get(CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
        int xCenter = sensor.width() / 2;
        int yCenter = sensor.height() / 2;
        int xDelta = (int) (0.5f * sensor.width() / zoom);
        int yDelta = (int) (0.5f * sensor.height() / zoom);
        return new Rect(xCenter - xDelta, yCenter - yDelta, xCenter + xDelta, yCenter + yDelta);
    }

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
            LogHelper.e(TAG, "Got oom exception ", ex);
            return null;
        }
    }

    public static int computeSampleSize(BitmapFactory.Options options, int minSideLength,
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

    // Rotates the bitmap by the specified degree.
    // If a new bitmap is created, the original bitmap is recycled.
    public static Bitmap rotate(Bitmap b, int degrees) {
        return rotateAndMirror(b, degrees, false);
    }

    // Rotates and/or mirrors the bitmap. If a new bitmap is created, the
    // original bitmap is recycled.
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

    private static class ImageFileNamer {
        private final SimpleDateFormat mFormat;
        private boolean mNextIsSameName = false;
        private String mSameDngName;

        // The date (in milliseconds) used to generate the last name.
        private long mLastDate;

        // Number of names generated for the same second.
        private int mSameSecondCount;

        public ImageFileNamer(String format) {
            mFormat = new SimpleDateFormat(format);
        }

        public String generateName(long dateTaken) {
            Date date = new Date(dateTaken);
            String result = mFormat.format(date);

            // If the last name was generated for the same second,
            // we append _1, _2, etc to the name.
            if (dateTaken / 1000 == mLastDate / 1000) {
                mSameSecondCount++;
                result += "_" + mSameSecondCount;
            } else {
                mLastDate = dateTaken;
                mSameSecondCount = 0;
            }

            return result;
        }

        public String generateDngName(long dateTaken) {
            if (mNextIsSameName && mSameDngName != null) {
                mNextIsSameName = false;
                return mSameDngName;
            }

            Date date = new Date(dateTaken);
            String result = mFormat.format(date);

            // If the last name was generated for the same second,
            // we append _1, _2, etc to the name.
            if (dateTaken / 1000 == mLastDate / 1000) {
                mSameSecondCount++;
                result += "_" + mSameSecondCount;
            } else {
                mLastDate = dateTaken;
                mSameSecondCount = 0;
            }
            result += "_RAW";
            //the Jpeg must the same name with dng
            if (!mNextIsSameName) {
                mNextIsSameName = true;
                mSameDngName = result;
            }
            return result;
        }
    }
    public static Bitmap createBitmapFromVideo(String filePath,
            FileDescriptor fd, int targetWidth) {
        Bitmap bitmap = null;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        LogHelper.i(TAG, "filePath = " + filePath + " : fd = " + fd
                + " : targetWidth  = " + targetWidth);
        try {
            if (filePath != null) {
                retriever.setDataSource(filePath);
            } else {
                retriever.setDataSource(fd);
            }
            bitmap = retriever.getFrameAtTime(-1);
        } catch (IllegalArgumentException ex) {
            // Assume this is a corrupt video file
            LogHelper.i(TAG, "IllegalArgumentException");
            ex.printStackTrace();
        } catch (RuntimeException ex) {
            LogHelper.i(TAG, "RuntimeException");
            // Assume this is a corrupt video file.
            ex.printStackTrace();
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException ex) {
                // Ignore failures while cleaning up.
                ex.printStackTrace();
            }
        }
        if (bitmap == null) {
            LogHelper.i(TAG, "bitmap = null");
            return null;
        }

        // Scale down the bitmap if it is bigger than we need.
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        LogHelper.v(TAG, "bitmap = " + width + "x" + height + "   targetWidth=" + targetWidth);
        if (width > targetWidth) {
            float scale = (float) targetWidth / width;
            int w = Math.round(scale * width);
            int h = Math.round(scale * height);
            LogHelper.v(TAG, "w = " + w + "h" + h);
            bitmap = Bitmap.createScaledBitmap(bitmap, w, h, true);
        }
        return rotateAndMirror(bitmap, 0, false);
    }
    @SuppressWarnings("serial")
    private static class DefaultHashMap<K, V> extends HashMap<K, V> {
        private V mDefaultValue;

        public void putDefault(V defaultValue) {
            mDefaultValue = defaultValue;
        }

        @Override
        public V get(Object key) {
            V value = super.get(key);
            return (value == null) ? mDefaultValue : value;
        }
    }

    /**
     * Size comparator that compares the number of pixels it covers.
     *
     * <p>If two the areas of two sizes are same, compare the widths.</p>
     */
    private static class SizeComparator implements Comparator<Size> {
        @Override
        public int compare(Size lhs, Size rhs) {
            return compareSizes(lhs.getWidth(), lhs.getHeight(), rhs.getWidth(), rhs.getHeight());
        }
    }

    private static int compareSizes(int widthA, int heightA, int widthB, int heightB) {
        long left = widthA * (long) heightA;
        long right = widthB * (long) heightB;
        if (left == right) {
            left = widthA;
            right = widthB;
        }
        return (left < right) ? -1 : (left > right ? 1 : 0);
    }

    private static boolean checkAspectRatiosMatch(Size a, Size b) {
        float aAspect = a.getWidth() / (float) a.getHeight();
        float bAspect = b.getWidth() / (float) b.getHeight();
        return Math.abs(aAspect - bAspect) < ASPECT_TOLERANCE;
    }
}
