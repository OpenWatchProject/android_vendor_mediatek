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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.camera.feature.mode.facebeauty.utils;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Size;
import android.media.Image;
import android.media.Image.Plane;
import android.media.MediaRecorder;
import android.util.Log;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;


import com.mediatek.camera.common.widget.Rotatable;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import junit.framework.Assert;

public class Util {
    private static final String TAG = "Util";
    // add for vFB begin
    public static final String KEY_FACE_BEAUTY_BIG_EYES = "pref_facebeauty_big_eyes_key";
    public static final String KEY_FACE_BEAUTY_SHARP = "pref_facebeauty_sharp_key";
    public static final String KEY_FACE_BEAUTY_SMOOTH = "pref_facebeauty_smooth_key";
    public static final String KEY_FACE_BEAUTY_SKIN_COLOR = "pref_facebeauty_skin_color_key";
    public static final String KEY_FACE_BEAUTY_SLIM = "pref_facebeauty_slim_key";
    public static final String KEY_CAMERA_FACE_BEAUTY_MULTI_MODE_KEY =
            "pref_face_beauty_multi_mode_key";
    public static final String KEY_FB_EXTREME_BEAUTY = "fb-extreme-beauty";
    public static final String FACE_BEAUTY_MODE = "face_beauty";

    public static final String KEY_VIDEO_FACE_BEAUTY = "face-beauty"; // face-beauty=true.mean's
                                                                     // start
                                                                     // vFB
                                                                     // mode
    public static final String VIDEO_FACE_BEAUTY_ENABLE = "true";
    public static final String VIDEO_FACE_BEAUTY_DISABLE = "false";
    public static final String KEY_VIDED_FACE_BEAUTY_FACE = "fb-face-pos";
    public static final String KEY_VIDED_FACE_BEAUTY_TOUCH = "fb-touch-pos";
    public static final int VIDEO_FACE_BEAUTY_MAX_SOLUTION_WIDTH = 1920;
    public static final int VIDEO_FACE_BEAUTY_MAX_SOLUTION_HEIGHT = 1088;
    public static final String KEY_INTO_VIDEO_FACE_BEAUTY_NORMAL = "face-beauty-normal";
    public static final int INTO_VIDEO_FACE_BEAUTY_NORMAL = 1;
    public static final int INTO_VIDEO_FACE_BEAUTY_NON_NORMAL = 0;

    private static final String CLASS_NAME = "android.media.MediaRecorder";
    private static final String METHOD_NAME_EXTRA = "setParameter";
    private static final Class[] METHOD_TYPES = new Class[] {String.class};

    // add for K2
    public static final String KEY_FB_EXTEME_BEAUTY_SUPPORTED = "fb-extreme-beauty-supported";
    public static final String KEY_FB_EXTEME_BEAUTY_DEFAULT = "face_beauty_multi_mode";

    // add for VFB+EIS+3DNR recording fps
    public static final String KEY_VIDEO_RECORIND_FEATURE_MAX_FPS = "feature-max-fps";
    public static final String KEY_3DNR_MODE = "3dnr-mode";
    public static final String THREE_DNR_MODE_ON = "on";
    public static final String KEY_VIDEO_STABLILIZATION = "video-stabilization";
    public static final String VIDEO_STABLILIZATION_ON = "true";
    public static final String PICTURE_RATIO_4_3 = "1.3333";

    public static final int ORIENTATION_HYSTERESIS = 5;
    public static final double ASPECT_TOLERANCE = 0.001;

    // add for vFB end

    public static void fadeIn(View... views) {
        for (View view : views) {
            fadeIn(view);
        }
    }

    public static void fadeIn(View view) {
        fadeIn(view, 0F, 1F, 400);
        // We disabled the button in fadeOut(), so enable it here.
        view.setEnabled(true);
    }

    public static void fadeIn(View view, float startAlpha, float endAlpha, long duration) {
        if (view.getVisibility() == View.VISIBLE) {
            return;
        }

        view.setVisibility(View.VISIBLE);
        Animation animation = new AlphaAnimation(startAlpha, endAlpha);
        animation.setDuration(duration);
        view.startAnimation(animation);
    }

    public static void fadeOut(View view) {
        if (view.getVisibility() != View.VISIBLE) {
            return;
        }

        // Since the button is still clickable before fade-out animation
        // ends, we disable the button first to block click.
        view.setEnabled(false);
        Animation animation = new AlphaAnimation(1F, 0F);
        animation.setDuration(400);
        view.startAnimation(animation);
        view.setVisibility(View.GONE);
    }

    public static void setOrientation(View view, int orientation, boolean animation) {
        if (view == null) {
            Log.w(TAG, "[setOrientation]view is null,return.");
            return;
        }
        if (view instanceof Rotatable) {
            ((Rotatable) view).setOrientation(orientation, animation);
        } else if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0, count = group.getChildCount(); i < count; i++) {
                setOrientation(group.getChildAt(i), orientation, animation);
            }
        }
    }

    public static boolean equals(Object a, Object b) {
        return (a == b) || (a == null ? false : a.equals(b));
    }

    // object -> byte
    public static byte[] toByteArray(Object object) {
        byte[] bytes = null;
        ByteArrayOutputStream bs = new ByteArrayOutputStream();
        try {
            ObjectOutputStream os = new ObjectOutputStream(bs);
            os.writeObject(object);
            os.flush();
            bytes = bs.toByteArray();
            os.close();
            bs.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
        return bytes;
    }

    public static <T> T checkNotNull(T object) {
        if (object == null)
            throw new NullPointerException();
        return object;
    }

    /**
     * use for get sensor orientation.
     * @param info the camera info.
     * @return the orientation.
     */
    public static int getSensorOrientation(CameraInfo info) {
        return info.orientation;
    }

    public static int getRecordingRotation(int orientation, int cameraId, CameraInfo info) {
        int rotation = -1;
        if (orientation != OrientationEventListener.ORIENTATION_UNKNOWN) {
            if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
                rotation = (info.orientation - orientation + 360) % 360;
            } else { // back-facing camera
                rotation = (info.orientation + orientation) % 360;
            }
        } else {
            // Get the right original orientation
            rotation = info.orientation;
        }
        return rotation;
    }

    public static String createNameFormat(long dateTaken, String format) {
        Date date = new Date(dateTaken);
        SimpleDateFormat dateFormat = new SimpleDateFormat(format);
        return dateFormat.format(date);
    }

    public static boolean isSupported(Object value, List<?> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    public static int getDisplayRotation(Activity activity) {
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

    public static int getDisplayOrientation(int degrees, int cameraId) {
        CameraInfo info = new CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        int result;
        if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        return result;
    }

    public static int computeRotation(Context context, int orientation, int compensation) {
        int rotation = orientation;
        int activityOrientation = context.getResources().getConfiguration().orientation;
        if (activityOrientation == Configuration.ORIENTATION_PORTRAIT) {
            // since portrait mode use ModePickerRotateLayout rotate 270, here
            // need to compensation,
            // compensation should be 270.
            rotation = (orientation - compensation + 360) % 360;
        }
        return rotation;
    }

    // Returns the largest picture size which matches the given aspect ratio.
    public static Size getOptimalVideoSnapshotPictureSize(List<Size> sizes, double targetRatio) {
        // Use a very small tolerance because we want an exact match.
        // final double ASPECT_TOLERANCE = 0.003;
        if (sizes == null)
            return null;

        Size optimalSize = null;

        // Try to find a size matches aspect ratio and has the largest width
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE)
                continue;
            if (optimalSize == null || size.width > optimalSize.width) {
                optimalSize = size;
            }
        }

        // Cannot find one that matches the aspect ratio. This should not
        // happen.
        // Ignore the requirement.
        if (optimalSize == null) {
            for (Size size : sizes) {
                if (optimalSize == null || size.width > optimalSize.width) {
                    optimalSize = size;
                }
            }
        }
        return optimalSize;
    }

    public static Size getOptimalPreviewSize(Activity currentActivity, List<Size> sizes,
            double targetRatio, boolean findMinalRatio, boolean needStandardPreview) {
        // Use a very small tolerance because we want an exact match.
        // final double EXACTLY_EQUAL = 0.001;
        if (sizes == null) {
            return null;
        }

        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;
        double minDiffWidth = Double.MAX_VALUE;

        // Because of bugs of overlay and layout, we sometimes will try to
        // layout the viewfinder in the portrait orientation and thus get the
        // wrong size of preview surface. When we change the preview size, the
        // new overlay will be created before the old one closed, which causes
        // an exception. For now, just get the screen size.
        Display display = currentActivity.getWindowManager().getDefaultDisplay();
        Point point = new Point();
        display.getSize(point);
        int targetHeight = Math.min(point.x, point.y);
        int targetWidth = Math.max(point.x, point.y);
        if (findMinalRatio) {
            // Find minimal aspect ratio for that: special video size maybe not
            // have the mapping preview size.
            double minAspectio = Double.MAX_VALUE;
            for (Size size : sizes) {
                double aspectRatio = (double) size.width / size.height;
                if (Math.abs(aspectRatio - targetRatio) <= Math.abs(minAspectio - targetRatio)) {
                    minAspectio = aspectRatio;
                }
            }
            targetRatio = minAspectio;
        }

        // Try to find an size match aspect ratio and size
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }

            if (Math.abs(size.height - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - targetHeight);
                minDiffWidth = Math.abs(size.width - targetWidth);
            } else if ((Math.abs(size.height - targetHeight) == minDiff)
                    && Math.abs(size.width - targetWidth) < minDiffWidth) {
                optimalSize = size;
                minDiffWidth = Math.abs(size.width - targetWidth);
            }
        }

        // Cannot find the one match the aspect ratio. This should not happen.
        // Ignore the requirement.
        // / M: This will happen when native return video size and wallpaper
        // want to get specified ratio.
        if (optimalSize == null && needStandardPreview) {
            minDiff = Double.MAX_VALUE;
            targetRatio = Double.parseDouble(PICTURE_RATIO_4_3);
            for (Size size : sizes) {
                double ratio = (double) size.width / size.height;
                if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                    continue;
                }
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
        }
        return optimalSize;
    }

    public static int[] pointFToPoint(float[] point) {
        int[] pointF = new int[2];
        pointF[0] = Math.round(point[0]);
        pointF[1] = Math.round(point[1]);
        return pointF;
    }

    public static void prepareMatrix(Matrix matrix, boolean mirror, int displayOrientation,
            int viewWidth, int viewHeight) {
        // Need mirror for front camera.
        matrix.setScale(mirror ? -1 : 1, 1);
        // This is the value for android.hardware.Camera.setDisplayOrientation.
        matrix.postRotate(displayOrientation);
        // Camera driver coordinates range from (-1000, -1000) to (1000, 1000).
        // UI coordinates range from (0, 0) to (width, height).
        matrix.postScale(viewWidth / 2000f, viewHeight / 2000f);
        matrix.postTranslate(viewWidth / 2f, viewHeight / 2f);
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
            Log.e(TAG, "[makeBitmap]Got oom exception:", ex);
            return null;
        }
    }

    public static void closeSilently(Closeable c) {
        if (c == null) {
            Log.w(TAG, "[closeSilently]c is null,return.");
            return;
        }
        try {
            c.close();
        } catch (Throwable t) {
            // do nothing
            t.printStackTrace();
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
    public static long getDeviceRam() {
        long mTotal = 0;
        String path = "/proc/meminfo";
        String content = null;
        BufferedReader br = null;
        try {
            br = new BufferedReader(new FileReader(path), 8);
            String line;
            if ((line = br.readLine()) != null) {
                content = line;
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (br != null) {
                try {
                    br.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        if (content != null) {
            int begin = content.indexOf(':');
            int end = content.indexOf('k');
            content = content.substring(begin + 1, end).trim();
            mTotal = Integer.parseInt(content);
            Log.d(TAG, "getDeviceRam = " + mTotal);
        }
        return mTotal;
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
            Plane plane0 = image.getPlanes()[0];
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
     * @return bytebuffer from image.
     */
    public static ByteBuffer acquireRgbBufferAndClose(Image image) {
        ByteBuffer imageBuffer = null;
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
    public static byte[] getContinuousRgbDataFromImage(Image image) {
        if (image.getFormat() != PixelFormat.RGBA_8888 &&
                image.getFormat() != PixelFormat.RGB_888) {
            Log.i(TAG, "error format = " + image.getFormat());
            return null;
        }
        int format = image.getFormat();
        int width = image.getWidth();
        int height = image.getHeight();
        int rowStride, pixelStride;
        byte[] data = null;
        Plane[] planes = image.getPlanes();
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
        return data;
    }

    /**
     * Set the mediatek-add parameter.
     * @param recorder The media recorder object.
     * @param info The parameter string.
     * @throws Exception The parameter not support.
     */
    /*public static void setParametersExtra(MediaRecorder recorder, String info) throws Exception {
        try {
            Class cls = Class.forName(CLASS_NAME);
            Method setParameterExtra = ReflectUtil.getMethod(cls, METHOD_NAME_EXTRA, METHOD_TYPES);
            Object objectValue = ReflectUtil.callMethodOnObjectWithExp(
                                              recorder, setParameterExtra, info);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e2) {
            throw new Exception(e2.getCause());
        }
    }*/
}
