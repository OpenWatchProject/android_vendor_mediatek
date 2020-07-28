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

package com.mediatek.emcamera;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.admin.DevicePolicyManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Point;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.os.Build;
import android.os.SystemProperties;
import android.provider.Settings;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.emcamera.R;

import java.lang.reflect.Method;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

/**
 * Collection of utility functions used in this package.
 */
public class Util {
    // Orientation hysteresis amount used in rounding, in degrees
    public static final int ORIENTATION_HYSTERESIS = 5;
    private static final String TAG = "Util";
    // The brightness setting used when it is set to automatic in the system.
    // The reason why it is set to 0.7 is just because 1.0 is too bright.
    // Use the same setting among the Camera, VideoCamera and Panorama modes.
    private static final float DEFAULT_CAMERA_BRIGHTNESS = 0.7f;
    private static final int OPEN_RETRY_COUNT = 2;
    private static final double[] RATIOS = new double[]{1.3333, 1.5, 1.6667, 1.7778};
    private static final String TRUE = "true";

    private static final String PROP_MONKEY = "ro.monkey";

    /** Video qualities sorted by size. */
    public static int[] sVideoQualities = new int[] {
            CamcorderProfile.QUALITY_2160P,
            CamcorderProfile.QUALITY_1080P,
            CamcorderProfile.QUALITY_720P,
            CamcorderProfile.QUALITY_480P,
            CamcorderProfile.QUALITY_CIF,
            CamcorderProfile.QUALITY_QVGA,
            CamcorderProfile.QUALITY_QCIF
    };

    public static void aAssert(boolean cond) {
        if (!cond) {
            throw new AssertionError();
        }
    }

    public static android.hardware.Camera openCamera(Activity activity, int cameraId) throws CameraHardwareException,
            CameraDisabledException {
        // Check if device policy has disabled the camera.
        DevicePolicyManager dpm = (DevicePolicyManager) activity.getSystemService(Context.DEVICE_POLICY_SERVICE);
        if (dpm.getCameraDisabled(null)) {
            throw new CameraDisabledException();
        }
        for (int i = 0; i < OPEN_RETRY_COUNT; i++) {
            try {
                return CameraHolder.instance().open(cameraId);
            } catch (CameraHardwareException e) {
                if (i == 0) {
                    try {
                        // wait some time, and try another time
                        // Camera device may be using by VT or atv.
                        Thread.sleep(1000);
                    } catch (InterruptedException ie) {
                        Elog.w(TAG, ie.getMessage());
                    }
                    continue;
                } else {
                    // In eng build, we throw the exception so that test tool
                    // can detect it and report it
                    if ("eng".equals(Build.TYPE)) {
                        Elog.e(TAG, "Open Camera fail" + e.getMessage());
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

    public static void showErrorAndFinish(final Activity activity, int msgId) {
        new AlertDialog.Builder(activity).setCancelable(false).setTitle("EM Camera")
        .setMessage(msgId).setNeutralButton(android.R.string.ok, new OnClickListener() {

                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        activity.finish();
                    } 
                    
                }).show();
    }

    public static void showMsgDialog(final Activity activity, String title, String msg) {
        AlertDialog dialog = new AlertDialog.Builder(activity).setTitle(title).setMessage(msg)
                .setPositiveButton(android.R.string.ok, null).create();
        dialog.show();
    }

    public static void showConfirmDialog(final Activity activity, int titleId, int msgId,
            OnClickListener listener) {
        AlertDialog dialog = new AlertDialog.Builder(activity).setTitle(titleId).setMessage(msgId)
                .setPositiveButton(android.R.string.ok, listener)
                .setNegativeButton(android.R.string.cancel, listener).create();
        dialog.show();
    }

    public static void toastOnUiThread(final Activity activity, final int msgId) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(activity, msgId, Toast.LENGTH_LONG).show();
            }

        });
    }

    public static int clamp(int x, int min, int max) {
        if (x > max) {
            return max;
        }
        if (x < min) {
            return min;
        }
        return x;
    }

    public static void dumpRect(RectF rect, String msg) {
        Elog.v(TAG, msg + "=(" + rect.left + "," + rect.top + "," + rect.right + "," + rect.bottom
                + ")");
    }

    public static int getDisplayRotation(Activity activity) {
        int r = activity.getWindowManager().getDefaultDisplay().getRotation();
        int result = 0;
        switch (r) {
            case Surface.ROTATION_0:
                result = 0;
                break;
            case Surface.ROTATION_90:
                result = 90;
                break;
            case Surface.ROTATION_180:
                result = 180;
                break;
            case Surface.ROTATION_270:
                result = 270;
                break;
            default:
                break;
        }
        return result;
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

    public static Size getOptimalPreviewSize(Activity currentActivity, List<Size> sizes, double targetRatio) {
        // Use a very small tolerance because we want an exact match.
        final double aspectTolerance = 0.001;
        if (sizes == null) {
            return null;
        }
        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        // Because of bugs of overlay and layout, we sometimes will try to
        // layout the viewfinder in the portrait orientation and thus get the
        // wrong size of mSurfaceView. When we change the preview size, the
        // new overlay will be created before the old one closed, which causes
        // an exception. For now, just get the screen size

        Display display = currentActivity.getWindowManager().getDefaultDisplay();
        int targetHeight = Math.min(display.getHeight(), display.getWidth());

        if (targetHeight <= 0) {
            // We don't know the size of SurfaceView, use screen height
            targetHeight = display.getHeight();
        }

        // Try to find an size match aspect ratio and size
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > aspectTolerance) {
                continue;
            }
            if (Math.abs(size.height - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - targetHeight);
            }
        }

        // Cannot find the one match the aspect ratio. This should not happen.
        // Ignore the requirement.
        if (optimalSize == null) {
            Elog.w(TAG, "No preview size match the aspect ratio");
            minDiff = Double.MAX_VALUE;
            for (Size size : sizes) {
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
        }
        return optimalSize;
    }

    public static void rectFToRect(RectF rectF, Rect rect) {
        rect.left = Math.round(rectF.left);
        rect.top = Math.round(rectF.top);
        rect.right = Math.round(rectF.right);
        rect.bottom = Math.round(rectF.bottom);
    }

    public static void prepareMatrix(Matrix matrix, boolean mirror, int displayOrientation, int viewWidth, int viewHeight) {
        // Need mirror for front camera.
        matrix.setScale(mirror ? -1 : 1, 1);
        // This is the value for android.hardware.Camera.setDisplayOrientation.
        matrix.postRotate(displayOrientation);
        // Camera driver coordinates range from (-1000, -1000) to (1000, 1000).
        // UI coordinates range from (0, 0) to (width, height).
        matrix.postScale(viewWidth / 2000f, viewHeight / 2000f);
        matrix.postTranslate(viewWidth / 2f, viewHeight / 2f);
    }

    public static void setRotationParameter(Parameters parameters, int cameraId, int orientation) {
        // See android.hardware.Camera.Parameters.setRotation for
        // documentation.
        int rotation = 0;
        if (orientation != OrientationEventListener.ORIENTATION_UNKNOWN) {
            CameraInfo info = CameraHolder.instance().getCameraInfo()[cameraId];
            if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
                rotation = (info.orientation - orientation + 360) % 360;
            } else { // back-facing camera
                rotation = (info.orientation + orientation) % 360;
            }
        } else {
            // Get the right original orientation
            CameraInfo info = CameraHolder.instance().getCameraInfo()[cameraId];
            rotation = info.orientation;
        }
        parameters.setRotation(rotation);
    }

    public static void enterLightsOutMode(Window window) {
        WindowManager.LayoutParams params = window.getAttributes();
        params.systemUiVisibility = View.SYSTEM_UI_FLAG_LOW_PROFILE;
        window.setAttributes(params);
    }

    public static void initializeScreenBrightness(Window win, ContentResolver resolver) {
        // Overright the brightness settings if it is automatic
        int mode = Settings.System.getInt(resolver, Settings.System.SCREEN_BRIGHTNESS_MODE,
                Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL);
        if (mode == Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC) {
            WindowManager.LayoutParams winParams = win.getAttributes();
            winParams.screenBrightness = DEFAULT_CAMERA_BRIGHTNESS;
            win.setAttributes(winParams);
        }
    }
    public static String buildSize(int width, int height) {
        return "" + width + "x" + height;
    }
    public static Point getSize(String sizeString) {
        Point size = null;
        int index = sizeString.indexOf('x');
        if (index != -1) {
            int width = Integer.parseInt(sizeString.substring(0, index));
            int height = Integer.parseInt(sizeString.substring(index + 1));
            size = new Point(width, height);
        }
        Elog.d(TAG, "getSize(" + sizeString + ") return " + size);
        return size;
    }
    public static boolean toleranceRatio(double target, double candidate) {
        boolean tolerance = true;
        if (candidate > 0) {
            tolerance = Math.abs(target - candidate) <= 0.02;
        }
        //Elog.d(TAG, "toleranceRatio(" + target + ", " + candidate + ") return " + tolerance);
        return tolerance;
    }
    public static double findFullscreenRatio(Context context, Parameters parameters) {
        double find = 4d / 3;
        if (parameters != null) {
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
            Elog.v(TAG, "fullscreen = " + fullscreen + " x = " + point.x + " y = " + point.y);
            for (int i = 0; i < RATIOS.length ; i++) {
                if (Math.abs(RATIOS[i] - fullscreen) < Math.abs(fullscreen - find)) {
                    find = RATIOS[i];
                }
            }
        }
        List<Size> sizes = parameters.getSupportedPictureSizes();
        if (sizes != null) {
            for (Size size : sizes) {
                if (toleranceRatio(find, (double) size.width / size.height)) {
                   // Elog.i(TAG, "findFullscreenRatio(" + parameters + ") return " + find);
                    return find;
                }
            }
            find = 4d / 3;
        }
       // Elog.d(TAG, "findFullscreenRatio(" + parameters + ") return " + find);
        return find;
    }
    public static List<Size> buildSupportedPictureSize(Parameters parameters, double targetRatio) {
        ArrayList<Size> list = new ArrayList<Size>();

        List<Size> sizes = parameters.getSupportedPictureSizes();
        if (sizes != null) {
            for (Size size : sizes) {
                if (toleranceRatio(targetRatio, (double) size.width / size.height)) {
                    list.add(size);
                }
            }
        }

        for (Size added : list) {
            Elog.d(TAG, "buildSupportedPictureSize() add " + added.width + "x" + added.height);
        }
        return list;
    }

    /**
     * This method returns the first supported video quality.
     */
    public static int getFirstSupportedVideoQuality(int cameraId, List<Size> supportedSizes) {
        int index = 0;
        int len = sVideoQualities.length;
        for (int i = 0; i < len; ++i) {
            if (CamcorderProfile.hasProfile(cameraId, sVideoQualities[i])
                    && featureByParameter(cameraId, sVideoQualities[i], supportedSizes)) {
                index = i;
                break;
            }
        }

        return sVideoQualities[index];
    }

    private static boolean featureByParameter(int cameraId, int quality,
                                              List<Size> supportedSizes) {
        CamcorderProfile profile = CamcorderProfile.get(cameraId, quality);
        Iterator<Size> it = supportedSizes.iterator();
        boolean support = false;
        while (it.hasNext()) {
            Size size = it.next();
            if (size.width >= profile.videoFrameWidth) {
                support = true;
                break;
            }
        }
        return support;
    }

    public static Size getOptimalPreviewCaptureSize (int width, int height, List<Size> supportedSizes) {
        double targetRatio = (double) width / height;
        int xDiff = Integer.MAX_VALUE;
        int yDiff = Integer.MAX_VALUE;
        Size previewSize = null;
        for (int i = 0; i < supportedSizes.size(); i++) {
            int w = supportedSizes.get(i).width;
            int h = supportedSizes.get(i).height;
            if (Util.toleranceRatio(targetRatio, (double) w / h)
                    && (xDiff > Math.abs(width - w) || yDiff > Math.abs(height - h))) {
                previewSize = supportedSizes.get(i);
                xDiff = Math.abs(width - w);
                yDiff = Math.abs(height - h);
            }
        }
        return previewSize;
    }

    public static Size getOptimalVideoCaptureSize (int width, int height, List<Size> supportedSizes) {
        int xDiff = Integer.MAX_VALUE;
        int yDiff = Integer.MAX_VALUE;
        Size videoSize = null;
        for (int i = 0; i < supportedSizes.size(); i++) {
            int w = supportedSizes.get(i).width;
            int h = supportedSizes.get(i).height;
            if ((w >= width && h >= height)
                    && (xDiff > Math.abs(width - w) || yDiff > Math.abs(height - h))) {
                videoSize = supportedSizes.get(i);
                xDiff = Math.abs(width - w);
                yDiff = Math.abs(height - h);
            }
        }
        return videoSize;
    }

    public static boolean getFeatureSupported(String para, Parameters parameters) {
        String str = parameters.get(para);
        Elog.d(TAG, "getFeatureSupport - " + para + " is " + str);
        return TRUE.equals(str);
    }

    public static int getIntegerFromEdit(EditText edit, int from, boolean includeFrom,
                                    int end,boolean includeEnd) {
        String editStr = edit.getText().toString();
        int value = -1;
        try {
            value = Integer.valueOf(editStr);
        } catch (NumberFormatException e) {
            Elog.w(TAG, "getIntegerFromEdit() NumberFormatException:" + e.getMessage());
        }

        if (value < from || value > end) {
            value = -1;
        } else if (!includeFrom && (value == from)) {
            value = -1;
        } else if (!includeEnd && (value == end)) {
            value = -1;
        }
        return value;
    }

    public static boolean isAutoTest() {

         // MTK method
         String monkeyProp = getProperty(PROP_MONKEY);
         Elog.v(TAG, "isAutoTest()-> Monkey running flag is " + monkeyProp);
         boolean isMonkey = ("true".equals(monkeyProp)
                 || "y".equals(monkeyProp)
                 || "yes".equals(monkeyProp)
                 || "1".equals(monkeyProp)
                 || "on".equals(monkeyProp));
//        boolean isMonkey = getBooleanProperty(PROP_MONKEY, false);
         // Andriod default API
         boolean isUserAMonkey = ActivityManager.isUserAMonkey();
         Elog.v(TAG, "isAutoTest()-> isUserAMonkey=" + isUserAMonkey);
         return (isMonkey || isUserAMonkey);
     }

    /**
     * get system.properties
     *
     * @param key
     *            The property key
     * @return property value
     */
    public static String getProperty(String key) {
        String value = "";
        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method get = c.getMethod("get", String.class, String.class);
            value = (String)(get.invoke(c, key, "" ));
        } catch (Exception e) {
            e.printStackTrace();
        }finally {
            return value;
        }
    }

    /**
     * set system.properties
     *
     * @param key
     *            The property key
     * @param value
     *            The property new value
     */
    public static void setProperty(String key, String value) {
        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method set = c.getMethod("set", String.class, String.class);
            set.invoke(c, key, value );
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
