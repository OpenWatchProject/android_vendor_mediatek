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
package com.mediatek.camera.portability;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.media.CamcorderProfile;
import android.os.Process;
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
public class CamcorderProfileEx {
    private static final String TAG = "CamcorderProfileEx";
    /**
     * Video quality for low.
     */
    public static final int QUALITY_LOW = 108;
    /**
     * Video quality for medium.
     */
    public static final int QUALITY_MEDIUM = 109;
    /**
     * Video quality for high.
     */
    public static final int QUALITY_HIGH = 110;
    /**
     * Video quality for fine.
     */
    public static final int QUALITY_FINE = 111;
    /**
     * Video qulity for h264 high.
     */
    public static final int QUALITY_H264_HIGH   = 117;
    /**
     * Video qulity for 4k2k.
     */
    public static final int QUALITY_FINE_4K2K       = 123;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_VGA_120FPS = 2231;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_HD_60FPS = 2240;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_HD_120FPS = 2241;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_HD_180FPS = 2242;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_HD_240FPS = 2243;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_FHD_60FPS = 2250;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_FHD_120FPS = 2251;
    /**
     * Video qulity for slow motion.
     */
    public static final int SLOW_MOTION_FHD_240FPS = 2252;

    private static final int SLOW_MOTION_LIST_START = SLOW_MOTION_VGA_120FPS;

    private static final int SLOW_MOTION_LIST_END = SLOW_MOTION_FHD_240FPS;

    private static final int QUALITY_LIST_END = QUALITY_FINE_4K2K;

    private static final int QUALITY_LIST_START = QUALITY_LOW;

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
     * Integer value:108 - the low quality level.<br>
     * Integer value:109 - the medium quality level.<br>
     * Integer value:110 - the high quality level.<br>
     * Integer value:111 - the fine quality level.<br>
     * @return CamcorderProlile the profile get from query
     */
    public static CamcorderProfile getProfile(int quality) {
        try {
            int numberOfCameras = Camera.getNumberOfCameras();
            CameraInfo cameraInfo = new CameraInfo();
            for (int i = 0; i < numberOfCameras; i++) {
                Camera.getCameraInfo(i, cameraInfo);
                if (cameraInfo.facing == CameraInfo.CAMERA_FACING_BACK) {
                    return getProfile(i, quality);
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "camera process killed due to getCameraInfo() error");
            Process.killProcess(Process.myPid());
        }
        return null;
    }

    /**
     * Returns the MediaTek specific camcorder profile for the given camera at the given.
     * quality level.
     * @param cameraId ID of the camera
     * @param quality Target quality level for the camcorder profile.<br>
     * @return CamcorderProfile the profile get from query
     */
    public static CamcorderProfile getProfile(int cameraId, int quality) {
        return getCamcorderProfileNative(cameraId, quality);
    }

    // Methods implemented by JNI in CamcorderProfile
    private static final CamcorderProfile getCamcorderProfileNative(
            int cameraId, int quality) {
        try {
            Method m = CamcorderProfile.class.getDeclaredMethod(
                    "native_get_camcorder_profile", int.class, int.class);
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
