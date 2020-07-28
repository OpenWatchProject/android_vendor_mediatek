/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.camcorder;

import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.media.CamcorderProfile;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * This class has been provided to enable camcorder profiles specific to devices
 * with MediaTek chipsets to be retrieved. This class is similar to android.media.CamcorderProfile
 * (see <a href="http://developer.android.com/reference/android/media/CamcorderProfile.html">
 * http://developer.android.com/reference/android/media/CamcorderProfile.html</a>).
 *
 * <p>Retrieves the
 * predefined MediaTek specific camcorder profile settings for camcorder applications.
 * These settings are read-only.
 *
 * <p>The compressed output from a recording session with a given
 * CamcorderProfile contains two tracks: one for audio and the other for video.
 *
 * <p>Each profile specifies the following set of parameters:
 * <ul>
 * <li> The file output format
 * <li> Video codec format
 * <li> Video bit rate in bits per second
 * <li> Video frame rate in frames per second
 * <li> Width and height of video frame
 * <li> Audio codec format
 * <li> Audio bit rate in bits per second,
 * <li> Audio sample rate
 * <li> Number of audio channels for recording.
 * </ul>
 */
public class CamcorderProfileEx
{
    private static final String TAG = "CamcorderProfileEx";
    /**
     * @hide
     * @internal
     */
    public static final int QUALITY_LOW = 108;
    /**
     * @hide
     * @internal
     */
    public static final int QUALITY_MEDIUM = 109;
    /**
     * @hide
     * @internal
     */
    public static final int QUALITY_HIGH = 110;
    /**
     * @hide
     * @internal
     */
    public static final int QUALITY_FINE = 111;
    /**
     * @hide
     */
    public static final int QUALITY_NIGHT_LOW = 112;
    /**
     * @hide
     */
    public static final int QUALITY_NIGHT_MEDIUM = 113;
    /**
     * @hide
     */
    public static final int QUALITY_NIGHT_HIGH = 114;
    /**
     * @hide
     */
    public static final int QUALITY_NIGHT_FINE = 115;
    /**
     * @hide
     */
    public static final int QUALITY_LIVE_EFFECT = 116;

    /**
     * @hide
     */
    public static final int QUALITY_H264_HIGH   = 117;
    /**
     * @hide
     * @internal
     */
    public static final int QUALITY_FINE_4K2K       = 123;
    /**
     * @hide
     * @internal
     */
    public static final int SLOW_MOTION_HD_120FPS = 2241;
    /**
     * @hide
     * @internal
     */
    public static final int SLOW_MOTION_HD_180FPS = 2242;
    /**
     * @hide
     * @internal
     */
    public static final int SLOW_MOTION_FHD_120FPS = 2251;
    /**
     * @hide
     * @internal
     */
    public static final int SLOW_MOTION_VGA_120FPS = 2231;
    /**
     * @hide
     * @internal
    */
    public static final int SLOW_MOTION_HD_60FPS = 2240;
    /**
     * @hide
     * @internal
     */
    public static final int SLOW_MOTION_FHD_60FPS = 2250;
    /**
     * @hide
     */
    private static final int SLOW_MOTION_LIST_START = SLOW_MOTION_VGA_120FPS;
    /**
     * @hide
     */
    private static final int SLOW_MOTION_LIST_END = SLOW_MOTION_FHD_120FPS;
    /**
     * @hide
     */
    public static final int QUALITY_LIST_END = QUALITY_FINE_4K2K;
    /**
     * @hide
     */
    private static final int QUALITY_LIST_START;

    /**
     * @hide
     */
    public static final int QUALITY_TIME_LAPSE_LIST_START;
    /**
     * @hide
     */
    public static final int QUALITY_TIME_LAPSE_LIST_END;

    
    static {
    QUALITY_TIME_LAPSE_LIST_START = getQualityNum("QUALITY_TIME_LAPSE_LIST_START");
    QUALITY_LIST_START = getQualityNum("QUALITY_LIST_START");
        QUALITY_TIME_LAPSE_LIST_END = QUALITY_TIME_LAPSE_LIST_START + QUALITY_LIST_END;
    }

    /**
     * @hide
     */
    private static int getQualityNum(String qualityName) {
        int qualityValue = 0;
        try {
            Field f = CamcorderProfile.class.getDeclaredField(qualityName);
            f.setAccessible(true);
            qualityValue = f.getInt(null);
        } catch (SecurityException e) {
            Log.e(TAG, "getQualityNum error");
        } catch (NoSuchFieldException e) {
            Log.e(TAG, "getQualityNum error");
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "getQualityNum error");
        } catch (IllegalAccessException e) {
            Log.e(TAG, "getQualityNum error");
        }
        return qualityValue;
    }

    /**
     * Returns the MediaTek specific camcorder profile for the back camera at the given
     * quality level.
     *
     * @param quality Target quality level for the camcorder profile.<br>
     * Integer value:8 - the low quality level.<br>
     * Integer value:9 - the medium quality level.<br>
     * Integer value:10 - the high quality level.<br>
     * Integer value:11 - the fine quality level.<br>
     */
    public static CamcorderProfile getProfile(int quality) {
        int numberOfCameras = Camera.getNumberOfCameras();
        CameraInfo cameraInfo = new CameraInfo();
        for (int i = 0; i < numberOfCameras; i++) {
            Camera.getCameraInfo(i, cameraInfo);
            if (cameraInfo.facing == CameraInfo.CAMERA_FACING_BACK) {
                return getProfile(i, quality);
            }
        }
        return null;
    }

    /**
     * Returns the MediaTek specific camcorder profile for the given camera at the given
     * quality level.
     *
     * @param cameraId ID of the camera
     * @param quality Target quality level for the camcorder profile.<br>
     * Integer value:8 - the low quality level.<br>
     * Integer value:9 - the medium quality level.<br>
     * Integer value:10 - the high quality level.<br>
     * Integer value:11 - the fine quality level.<br>
     */
    public static CamcorderProfile getProfile(int cameraId, int quality) {
        if (!((quality >= QUALITY_LIST_START &&
               quality <= QUALITY_LIST_END) ||
              (quality >= QUALITY_TIME_LAPSE_LIST_START &&
               quality <= QUALITY_TIME_LAPSE_LIST_END) ||
               (quality >= SLOW_MOTION_LIST_START &&
               quality <= SLOW_MOTION_LIST_END))) {
            String errMessage = "Unsupported quality level: " + quality;
            throw new IllegalArgumentException(errMessage);
        }
        return native_get_camcorder_profile(cameraId, quality);
    }

    // Methods implemented by JNI in CamcorderProfile
    private static final CamcorderProfile native_get_camcorder_profile(
            int cameraId, int quality) {
        try {
            Method m = CamcorderProfile.class.getDeclaredMethod("native_get_camcorder_profile", int.class, int.class);
            m.setAccessible(true);
            return (CamcorderProfile) m.invoke(null, cameraId, quality);
        } catch (SecurityException e) {
            Log.e(TAG, "native_get_camcorder_profile error");
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "native_get_camcorder_profile error");
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "native_get_camcorder_profile error");
        } catch (IllegalAccessException e) {
            Log.e(TAG, "native_get_camcorder_profile error");
        } catch (InvocationTargetException e) {
            Log.e(TAG, "native_get_camcorder_profile error");
        }
        return null;
    }
}
