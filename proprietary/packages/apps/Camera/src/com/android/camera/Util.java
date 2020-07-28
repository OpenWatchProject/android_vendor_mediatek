/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.camera;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.admin.DevicePolicyManager;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplayStatus;
import android.location.Location;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.os.Process;
import android.os.SystemProperties;
import android.os.Debug.MemoryInfo;
import android.provider.MediaStore;
import android.util.DisplayMetrics;
import android.util.FloatMath;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;

import com.android.camera.CameraManager.CameraProxy;
import com.android.camera.ui.Rotatable;
import com.mediatek.pq.PictureQuality;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.File;
import java.io.FileFilter;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.StringTokenizer;
import java.util.regex.Pattern;

/**
 * Collection of utility functions used in this package.
 */
public class Util {
    private static final String TAG = "Util";

    // Orientation hysteresis amount used in rounding, in degrees
    public static final int ORIENTATION_HYSTERESIS = 5;

    public static final String REVIEW_ACTION = "com.android.camera.action.REVIEW";
    public static final String OFF = "off";
    public static final int FIRST_PREVIEW_BLACK_ON = 1;
    public static final int FIRST_PREVIEW_BLACK_OFF = 0;
    //launch Gallery from camera
    public static final String IS_CAMERA = "isCamera";
    public static final String IS_SECURE_CAMERA = "isSecureCamera";
    public static final String SECURE_ALBUM = "secureAlbum";
    public static final String SECURE_PATH = "securePath";

    // Private intent extras. Test only.
    private static final String EXTRAS_CAMERA_FACING = "android.intent.extras.CAMERA_FACING";

    private static float sPixelDensity = 1;
    private static ImageFileNamer sImageFileNamer;

    // / M: Mediatek modify begin @{
    private static final int OPEN_RETRY_COUNT = 2;
    private static boolean sIsMAVSupport = true;
    private static boolean sIsPANORAMASupport = true;
    private static Uri sLastUri = null;
    private static AlertDialog sAlertDialog;
    // Sync Wfd connect status because it will be changed quickly.
    private static boolean mWfdEnabled;
    public static final double ASPECT_TOLERANCE = 0.03;

    public static boolean checkLiveEffect() {
        return 1 == SystemProperties.getInt("vendor.vdo.cam.effect", 0);
    }

    public static void setLastUri(Uri uri) {
        // in videowallpaper intent we will not update thumbnail,but keep the
        // URI.
        sLastUri = uri;
    }

    // / @}
    private Util() {
    }

    public static void initialize(Context context) {
        DisplayMetrics metrics = new DisplayMetrics();
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        wm.getDefaultDisplay().getMetrics(metrics);
        sPixelDensity = metrics.density;
        sImageFileNamer = new ImageFileNamer(context.getString(R.string.image_file_name_format));
    }

    public static boolean isPANORAMASupport() {
        return sIsPANORAMASupport;
    }

    // add for debug thumbnail, can save bitmap map as jpg file keep original
    // size
    public static Bitmap dumpBitmap(Bitmap b, String path) {
        File file = new File(Storage.getMountPoint() + File.separator + path);

        FileOutputStream outputStream = null;
        try {
            if (!file.exists()) {
                file.createNewFile();
            }
            outputStream = new FileOutputStream(file);
            if (outputStream != null) {
                b.compress(Bitmap.CompressFormat.JPEG, 100, outputStream);
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        } finally {
            Util.closeSilently(outputStream);
        }
        return b;
    }

    private static boolean isMountPointValid(Uri uri, ContentResolver resolver) {
        String path = "";
        Cursor cursor = resolver.query(uri, new String[] { MediaStore.MediaColumns.DATA }, null,
                null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    path = cursor.getString(0);
                }
            } finally {
                cursor.close();
            }
        }

        String directory =  Storage.getFileDirectory();
        File file = new File(path);
        String parent = file.getParent();
        boolean valid = directory.equals(parent);
        Log.d(TAG, "isMountPointValid(" + uri + ") path =" + path + ", Storage.MOUNT_POINT ="
                + Storage.getMountPoint() + ", return " + valid);
        return valid;
    }

    public static String createName(String format, long dateTaken) {
        Date date = new Date(dateTaken);
        SimpleDateFormat dateFormat = new SimpleDateFormat(format);
        return dateFormat.format(date);
    }

    public static void fadeIn(View... views) {
        for (View view : views) {
            fadeIn(view);
        }
    }

    public static void fadeOut(View... views) {
        for (View view : views) {
            fadeOut(view);
        }
    }

    // Decode the JPG to a bitmap to the desired size while keepping the
    // maxWidth/height
    public static Bitmap makeBitmap(String jpgFilePath, int minSideLength, int maxNumOfPixels) {
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();

            options.inJustDecodeBounds = true;

            BitmapFactory.decodeFile(jpgFilePath, options);
            if (options.mCancel || options.outWidth == -1 || options.outHeight == -1) {
                return null;
            }
            options.inSampleSize = options.outWidth / minSideLength;

            options.inJustDecodeBounds = false;
            options.inDither = false;
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            return BitmapFactory.decodeFile(jpgFilePath, options);

        } catch (OutOfMemoryError ex) {
            Log.e(TAG, "Got oom exception ", ex);
            return null;
        }
    }

    public static int getExifOrientation(ExifInterface exif) {
        int degree = 0;
        if (exif != null) {
            int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, -1);
            if (orientation != -1) {
                // We only recognize a subset of orientation tag values.
                switch (orientation) {
                case ExifInterface.ORIENTATION_ROTATE_90:
                    degree = 90;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_180:
                    degree = 180;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    degree = 270;
                    break;
                default:
                    break;
                }

            }
        }
        return degree;
    }

    public static int computeRotation(Context context, int orientation, int compensation) {
        int rotation = orientation;
        int activityOrientation = context.getResources().getConfiguration().orientation;
        if (activityOrientation == Configuration.ORIENTATION_PORTRAIT) {
            // since portrait mode use ModePickerRotateLayout rotate 270, here
            // need to compensation,
            // compensation should be 270.
            rotation = (orientation - compensation + 360) % 360;
            ;
        }
        return rotation;
    }

    // / @}
    public static int dpToPixel(int dp) {
        return Math.round(sPixelDensity * dp);
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

    /*
     * Compute the sample size as a function of minSideLength and
     * maxNumOfPixels. minSideLength is used to specify that minimal width or
     * height of a bitmap. maxNumOfPixels is used to specify the maximal size in
     * pixels that is tolerable in terms of memory usage.
     *
     * The function returns a sample size based on the constraints. Both size
     * and minSideLength can be passed in as -1 which indicates no care of the
     * corresponding constraint. The functions prefers returning a sample size
     * that generates a smaller bitmap, unless minSideLength = -1.
     *
     * Also, the function rounds up the sample size to a power of 2 or multiple
     * of 8 because BitmapFactory only honors sample size this way. For example,
     * BitmapFactory downsamples an image by 2 even though the request is 3. So
     * we round up the sample size to avoid OOM.
     */
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
            Log.e(TAG, "Got oom exception ", ex);
            return null;
        }
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

    public static void assertError(boolean cond) {
        if (!cond) {
            throw new AssertionError();
        }
    }

    public static void openCamera(Activity activity, boolean isPIP, int cameraId)
                          throws CameraHardwareException,
            CameraDisabledException {
        Log.i(TAG, "openCamera begin isPIP = " + isPIP);
        // Check if device policy has disabled the camera.
        DevicePolicyManager dpm = (DevicePolicyManager) activity
                .getSystemService(Context.DEVICE_POLICY_SERVICE);
        if (dpm.getCameraDisabled(null)) {
            throw new CameraDisabledException();
        }
        try {
            if (isPIP) {
                retryOpen(activity, OPEN_RETRY_COUNT, CameraHolder.instance().getBackCameraId());
                retryOpen(activity, OPEN_RETRY_COUNT, CameraHolder.instance().getFrontCameraId());
            } else {
                CameraProxy cameraProxy = retryOpen(activity, OPEN_RETRY_COUNT, cameraId);
                 // set panel size to camera service to update new supported preview sizes
                Parameters parameter = cameraProxy.getParameters();
                if (isSetPanelToNative(parameter, activity)) {
                    ParametersHelper.setPanelSize(parameter, getPanelSizeStr(activity));
                    cameraProxy.setParameters(parameter);
                }
            }
        } catch (CameraHardwareException e) {
            CameraHolder.instance().release(false);
            throw e;
        }
    }

    public static boolean bottomGraphicIsMainCamera(Context context) {
        boolean isMainCamera = false;
        isMainCamera = (((com.android.camera.CameraActivity)
                         context).getCameraDevice() == CameraHolder
                .instance().getCameraProxy(CameraHolder.instance().getBackCameraId()));
        Log.i(TAG, "bottomGraphicIsMainCamera = " + isMainCamera);
        return isMainCamera;
    }

    private static CameraManager.CameraProxy retryOpen(Activity activity, int count, int cameraId)
            throws CameraHardwareException {
        for (int i = 0; i < count; i++) {
            try {
                Log.i(TAG, "[retryOpen] cameraId = " + cameraId);
                CameraProxy cameraProxy = CameraHolder.instance().open(cameraId);
                // set error callback ASAP, because camera may error at any time.
                cameraProxy.setErrorCallback(new CameraErrorCallback(activity));
                return cameraProxy;
            } catch (CameraHardwareException e) {
                if (i == 0) {
                    try {
                        // wait some time, and try another time
                        // Camera device may be using by VT or atv.
                        Thread.sleep(1000);
                    } catch (InterruptedException ie) {
                        ie.printStackTrace();
                    }
                    continue;
                } else {
                    // In eng build, we throw the exception so that test tool
                    // can detect it and report it
                    if ("eng".equals(Build.TYPE)) {
                        Log.i(TAG, "Open Camera fail", e);
                        throw e;
                        // QA will always consider JE as bug, so..
                        // throw new RuntimeException("openCamera failed", e);
                    } else {
                        throw e;
                    }
                }
            }
        }
        // just for build pass
        throw new CameraHardwareException(new RuntimeException("Should never get here"));
    }

    private static String getPanelSizeStr(Context context) {
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        int panelHeight = Math.min(point.x, point.y);
        int panelWidth = Math.max(point.x, point.y);
        return "" + panelWidth + "x" + panelHeight;
    }

    public static void showErrorAndFinish(final Activity activity, int msgId) {
        DialogInterface.OnClickListener buttonListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // kill process to reset camera app status
                Process.killProcess(Process.myPid());
            }
        };
        if (activity.isFinishing() || sAlertDialog != null) {
            return;
        }
        sAlertDialog = new AlertDialog.Builder(activity).setCancelable(false)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                // .setTitle(R.string.camera_error_title)
                .setTitle("").setMessage(msgId)
                .setNeutralButton(R.string.dialog_ok, buttonListener).show();
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
                    sAlertDialog.dismiss();
                    sAlertDialog = null;
                }
            });
        }
    }

    public static <T> T checkNotNull(T object) {
        if (object == null)
            throw new NullPointerException();
        return object;
    }

    public static boolean equals(Object a, Object b) {
        return (a == b) || (a == null ? false : a.equals(b));
    }

    public static int nextPowerOf2(int n) {
        n -= 1;
        n |= n >>> 16;
        n |= n >>> 8;
        n |= n >>> 4;
        n |= n >>> 2;
        n |= n >>> 1;
        return n + 1;
    }

    public static float distance(float x, float y, float sx, float sy) {
        float dx = x - sx;
        float dy = y - sy;
        return FloatMath.sqrt(dx * dx + dy * dy);
    }

    public static int clamp(int x, int min, int max) {
        if (x > max)
            return max;
        if (x < min)
            return min;
        return x;
    }

    public static int getDisplayRotation(Activity activity) {
        if (Util.isWfdEnabled(activity) || FeatureSwitcher.isTablet()) {
            int rotation = activity.getWindowManager().getDefaultDisplay()
                    .getRotation();
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
        } else {
            return 0;
        }
    }

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
    /*
     * Judge WFD is connected or not?
     */
    public static boolean isWfdEnabled(Context mContext) {
        mWfdEnabled = false;
        int activityDisplayState = -1;
        DisplayManager mDisplayManager = (DisplayManager) mContext
                .getSystemService(Context.DISPLAY_SERVICE);
        WifiDisplayStatus mWfdStatus = mDisplayManager.getWifiDisplayStatus();
        activityDisplayState = mWfdStatus.getActiveDisplayState();
        mWfdEnabled = activityDisplayState == WifiDisplayStatus.DISPLAY_STATE_CONNECTED;
        Log.v(TAG, "isWfdEnabled() mWfdStatus=" + mWfdStatus + ", return "
                + mWfdEnabled);
        return mWfdEnabled;
    }
    public static int getCameraInfoOrientation(int cameraId) {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        Log.d(TAG, "getCameraInfoOrientation cameraId = " + cameraId + " info.orientation = "
                + info.orientation);
        return info.orientation;
    }

    public static int getGapOrientation(int displayRotation, int cameraId) {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        return displayRotation - info.orientation;
    }

    public static int getCameraOrientation(int cameraId) {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        return info.orientation;
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
            Log.d(TAG, "getOptimalPreviewSize(" + targetRatio + ") minAspectio=" + minAspectio);
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
            Log.w(TAG, "No preview size match the aspect ratio" + targetRatio + ","
                    + "then use the standard(4:3) preview size");
            minDiff = Double.MAX_VALUE;
            targetRatio = Double.parseDouble("1.3333");
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

    // Try to find a size matches aspect ratio and has the smallest size(preview
    // size & picture size)
    public static Size getMininalPIPTopSize(Activity currentActivity, List<Size> sizes,
            double targetRatio) {
        if (sizes == null || targetRatio < 0) {
            Log.d(TAG, "getMininalPIPTopSize error sizes = " + sizes + " targetRatio = "
                    + targetRatio);
            return null;
        }
        Size optimalSize = null;
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            Log.i(TAG, "getMininalPIPTopSize width = " + size.width + " height = " + size.height);
            if (Math.abs(ratio - targetRatio) > 0.02)
                continue;
            if (optimalSize == null || size.width < optimalSize.width) {
                optimalSize = size;
            }
        }
        return optimalSize;
    }

    public static boolean isBottomHasHighFrameRate(Context context) {
        boolean isHigh = false;
        Parameters bottomParameters = ((com.android.camera.CameraActivity) context).getParameters();
        Parameters topParameters = ((com.android.camera.CameraActivity) context).getTopParameters();
        int bottomFrameRate = (bottomParameters == null) ? 0 : bottomParameters
                .getPreviewFrameRate();
        int topFrameTate = (topParameters == null) ? 0 : topParameters.getPreviewFrameRate();
        Log.i(TAG, "isBottomHasHighFrameRate bottomFrameRate = " + bottomFrameRate
                + " topFrameTate = " + topFrameTate);
        return bottomFrameRate >= topFrameTate;
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
            Log.w(TAG, "No picture size match the aspect ratio");
            for (Size size : sizes) {
                if (optimalSize == null || size.width > optimalSize.width) {
                    optimalSize = size;
                }
            }
        }
        return optimalSize;
    }

    public static void dumpParameters(Parameters parameters) {
        String flattened = parameters.flatten();
        StringTokenizer tokenizer = new StringTokenizer(flattened, ";");
        Log.d(TAG, "Dump all camera parameters:");
        while (tokenizer.hasMoreElements()) {
            Log.d(TAG, tokenizer.nextToken());
        }
    }

    // This is for test only. Allow the camera to launch the specific camera.
    public static int getCameraFacingIntentExtras(Activity currentActivity) {
        int cameraId = -1;

        int intentCameraId = currentActivity.getIntent().getIntExtra(Util.EXTRAS_CAMERA_FACING, -1);

        if (isFrontCameraIntent(intentCameraId)) {
            // Check if the front camera exist
            int frontCameraId = CameraHolder.instance().getFrontCameraId();
            if (frontCameraId != -1) {
                cameraId = frontCameraId;
            }
        } else if (isBackCameraIntent(intentCameraId)) {
            // Check if the back camera exist
            int backCameraId = CameraHolder.instance().getBackCameraId();
            if (backCameraId != -1) {
                cameraId = backCameraId;
            }
        }
        return cameraId;
    }

    private static boolean isFrontCameraIntent(int intentCameraId) {
        return (intentCameraId == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT);
    }

    private static boolean isBackCameraIntent(int intentCameraId) {
        return (intentCameraId == android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK);
    }

    private static int sLocation[] = new int[2];

    // This method is not thread-safe.
    public static boolean pointInView(float x, float y, View v, int rotation) {
        if (v == null) {
            return false;
        }
        v.getLocationInWindow(sLocation);
        RectF rect = null;
        switch (rotation) {
        case 0:
            rect = new RectF(sLocation[0], sLocation[1], sLocation[0] + v.getWidth(), sLocation[1]
                    + v.getHeight());
            break;
        case 90:
            rect = new RectF(sLocation[0], sLocation[1] - v.getWidth(), sLocation[0]
                    + v.getHeight(), sLocation[1]);
            break;
        case 180:
            rect = new RectF(sLocation[0] - v.getWidth(), sLocation[1] - v.getHeight(),
                    sLocation[0], sLocation[1]);
            break;
        case 270:
            rect = new RectF(sLocation[0] - v.getHeight(), sLocation[1], sLocation[0], sLocation[1]
                    + v.getWidth());
            break;
        default:
            throw new RuntimeException("rotation=" + rotation);
        }
        boolean result = rect.contains(x, y);
        Log.v(TAG, "pointInView(" + x + ", " + y + ", " + rotation + ") rect=" + rect + " return "
                + result + " " + v);
        return result;
    }

    public static int[] getRelativeLocation(View reference, View view) {
        reference.getLocationInWindow(sLocation);
        int referenceX = sLocation[0];
        int referenceY = sLocation[1];
        view.getLocationInWindow(sLocation);
        sLocation[0] -= referenceX;
        sLocation[1] -= referenceY;
        return sLocation;
    }

    public static boolean isUriValid(Uri uri, ContentResolver resolver) {
        if (uri == null) {
            return false;
        }

        try {
            ParcelFileDescriptor pfd = resolver.openFileDescriptor(uri, "r");
            if (pfd == null) {
                Log.e(TAG, "Fail to open URI. URI=" + uri);
                return false;
            }
            pfd.close();
        } catch (IOException ex) {
            return false;
        }
        return isMountPointValid(uri, resolver);
    }

    public static void viewUri(Uri uri, Context context) {
        if (!isUriValid(uri, context.getContentResolver())) {
            Log.e(TAG, "Uri invalid. uri=" + uri);
            return;
        }

        try {
            context.startActivity(new Intent(Util.REVIEW_ACTION, uri));
        } catch (ActivityNotFoundException ex) {
            try {
                context.startActivity(new Intent(Intent.ACTION_VIEW, uri));
            } catch (ActivityNotFoundException e) {
                Log.e(TAG, "review image fail. uri=" + uri, e);
            }
        }
    }

    public static void dumpRect(RectF rect, String msg) {
        Log.v(TAG, msg + "=(" + rect.left + "," + rect.top + "," + rect.right + "," + rect.bottom
                + ")");
    }

    public static void rectFToRect(RectF rectF, Rect rect) {
        rect.left = Math.round(rectF.left);
        rect.top = Math.round(rectF.top);
        rect.right = Math.round(rectF.right);
        rect.bottom = Math.round(rectF.bottom);
    }

    public static int[] pointFToPoint(float[] point) {
        int[] pointF = new int[2];
        pointF[0] = Math.round(point[0]);
        pointF[1] = Math.round(point[1]);
        return pointF;
    }

    public static void prepareMatrix(Matrix matrix, boolean mirror, int displayOrientation,
            int viewWidth, int viewHeight) {
        Log.d(TAG, "prepareMatrix mirror =" + mirror + " displayOrientation=" + displayOrientation
                + " viewWidth=" + viewWidth + " viewHeight=" + viewHeight);
        // Need mirror for front camera.
        matrix.setScale(mirror ? -1 : 1, 1);
        // This is the value for android.hardware.Camera.setDisplayOrientation.
        matrix.postRotate(displayOrientation);
        // Camera driver coordinates range from (-1000, -1000) to (1000, 1000).
        // UI coordinates range from (0, 0) to (width, height).
        matrix.postScale(viewWidth / 2000f, viewHeight / 2000f);
        matrix.postTranslate(viewWidth / 2f, viewHeight / 2f);
    }

    public static String createJpegName(long dateTaken) {
        synchronized (sImageFileNamer) {
            return sImageFileNamer.generateName(dateTaken);
        }
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

    public static void fadeIn(View view) {
        fadeIn(view, 0F, 1F, 400);

        // We disabled the button in fadeOut(), so enable it here.
        view.setEnabled(true);
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

    public static int getJpegRotation(int cameraId, int orientation) {
        // See android.hardware.Camera.Parameters.setRotation for
        // documentation.
        int rotation = 0;
        CameraInfo info = CameraHolder.instance().getCameraInfo()[cameraId];
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

    public static void setGpsParameters(Parameters parameters, Location loc) {
        // Clear previous GPS location from the parameters.
        parameters.removeGpsData();

        // We always encode GpsTimeStamp
        parameters.setGpsTimestamp(System.currentTimeMillis() / 1000);

        // Set GPS location.
        if (loc != null) {
            double lat = loc.getLatitude();
            double lon = loc.getLongitude();
            boolean hasLatLon = (lat != 0.0d) || (lon != 0.0d);

            if (hasLatLon) {
                Log.d(TAG, "Set gps location");
                parameters.setGpsLatitude(lat);
                parameters.setGpsLongitude(lon);
                parameters.setGpsProcessingMethod(loc.getProvider().toUpperCase());
                if (loc.hasAltitude()) {
                    parameters.setGpsAltitude(loc.getAltitude());
                } else {
                    // for NETWORK_PROVIDER location provider, we may have
                    // no altitude information, but the driver needs it, so
                    // we fake one.
                    parameters.setGpsAltitude(0);
                }
                if (loc.getTime() != 0) {
                    // Location.getTime() is UTC in milliseconds.
                    // gps-timestamp is UTC in seconds.
                    long utcTimeSeconds = loc.getTime() / 1000;
                    parameters.setGpsTimestamp(utcTimeSeconds);
                }
            } else {
                loc = null;
            }
        }
    }

    public static class ImageFileNamer {
        private SimpleDateFormat mFormat;
        private String mSameDngName;
        // The date (in milliseconds) used to generate the last name.
        private long mLastDate;
        private boolean mNextIsSameName = false;
        // Number of names generated for the same second.
        private int mSameSecondCount;
        private int mRawNameFlag = 0x00;
        private int mTypeflag = 0x03;

        public ImageFileNamer(String format) {
            mFormat = new SimpleDateFormat(format, Locale.ENGLISH);
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

        /**
         * To generate the dng picture name.
         * @param dateTaken the image is named by the shot data.
         * @param pictureType the image format.
         * @return name the name for the image.
         */
        public String generateRawName(long dateTaken, int pictureType) {
            if (pictureType == Storage.PICTURE_TYPE_RAW) {
                mTypeflag = 0x02;
            } else {
                mTypeflag = 0x01;
            }
            if (mRawNameFlag != 0x00 && (mRawNameFlag & mTypeflag) != 0x00) {
                mNextIsSameName = false;
                mRawNameFlag = 0x00;
            }
            mRawNameFlag = mRawNameFlag | mTypeflag;
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
            mNextIsSameName = true;
            //the Jpeg must the same name with dng
            mSameDngName = result;
            return result;
        }

    }

    // / M: set view orientation @{
    public static void setOrientation(View view, int orientation, boolean animation) {
        if (view == null) {
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

    public static void enterCameraPQMode() {
        Log.d(TAG, "enterCameraPQMode()");
        PictureQuality.setMode(PictureQuality.MODE_CAMERA);
    }

    public static void exitCameraPQMode() {
        Log.d(TAG, "exitCameraPQMode()");
        PictureQuality.setMode(PictureQuality.MODE_NORMAL);
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

    public static int getDeviceCores() {
        int cores = 0;
        cores = initializeCoresNumber();
        Log.d(TAG, "devices cores = " + cores);
        return cores;
    }

    // / Get core number to initialize sCoreNumber;
    private static int initializeCoresNumber() {
        int sCoreNumber = 0;
        // Private Class to display only CPU devices in the directory listing
        // Get directory containing CPU info
        File dir = new File("/sys/devices/system/cpu/");
        // Filter to only list the devices we care about
        File[] files = dir.listFiles(new FileFilter() {
            public boolean accept(File file) {
                return Pattern.matches("cpu[0-9]", file.getName());
            }
        });
        if (files == null) {
            Log.d(TAG, "CPU Count: Failed.");
            // Default to return 1 core
            return 1;
        }
        sCoreNumber = files.length;
        Log.d(TAG, "CPU Count: " + sCoreNumber);
        return sCoreNumber;
    }

    private static boolean isSetPanelToNative(Parameters parameter, Activity activity) {
        boolean displayRotSupported = ParametersHelper.isDisplayRotateSupported(parameter);
        Log.d(TAG, "isSetPanelToNative displayRotSupported = "
                + displayRotSupported + ", isWfdEnable = " + mWfdEnabled);
        return displayRotSupported && !mWfdEnabled;
    }
    public static void logMemory(String title) {
        MemoryInfo mi = new MemoryInfo();
        android.os.Debug.getMemoryInfo(mi);
        String tagtitle = "logMemory() " + title;
        Log.d(TAG, tagtitle + "         PrivateDirty    Pss     SharedDirty");
        Log.d(TAG, tagtitle + " dalvik: " + mi.dalvikPrivateDirty + ", " + mi.dalvikPss + ", "
                + mi.dalvikSharedDirty + ".");
        Log.d(TAG, tagtitle + " native: " + mi.nativePrivateDirty + ", " + mi.nativePss + ", "
                + mi.nativeSharedDirty + ".");
        Log.d(TAG, tagtitle + " other: " + mi.otherPrivateDirty + ", " + mi.otherPss + ", "
                + mi.otherSharedDirty + ".");
        Log.d(TAG, tagtitle + " total: " + mi.getTotalPrivateDirty() + ", " + mi.getTotalPss()
                + ", " + mi.getTotalSharedDirty() + ".");
    }

}

// / M: Mediatek modify begin @{
// DefaultHashMap is a HashMap which returns a default value if the specified
// key is not found.
//
@SuppressWarnings("serial")
class DefaultHashMap<K, V> extends HashMap<K, V> {
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
// / @}
