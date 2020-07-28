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
 *     MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.camera.feature.setting.shutterspeed;

import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.util.Range;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.ArrayList;
import java.util.List;

/**
 * Helper class for shutter speed.
 */
class ShutterSpeedHelper {

    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(ShutterSpeedHelper.class.getSimpleName());
    /**
     * Shutter speed auto.
     */
    public static final String AUTO = "Auto";
    /**
     * Shutter speed is 1 second.
     */
    public static final String ONE_SECONDS = "1";
    /**
     * Shutter speed is 2 second.
     */
    public static final String TWO_SECONDS = "2";
    /**
     * Shutter speed is 4 second.
     */
    public static final String FOUR_SECONDS = "4";
    /**
     * Shutter speed is 8 second.
     */
    public static final String EIGHT_SECONDS = "8";
    /**
     * Shutter speed is 16 second.
     */
    public static final String THIRTY_SIX_SECONDS = "16";

    private static final long MIN_SHUTTER_SPEED_NS = 1000000000L; // 1s
    private static final long MIN_SHUTTER_SPEED_S = 1L; // 1s
    private static final int S_TO_NS = 1000000000;
    private static final int MS_TO_S = 1000;
    private static final String KEY_EXPOSURE_TIME_MAX = "max-exposure-time"; //ms

    /**
     * Get supported shutter speed list.
     *
     * @param characteristics THe Characteristics.
     * @return The supported shutter speed list.
     */
    public static List<String> getSupportedList(CameraCharacteristics characteristics) {
        Long minShutterSpeed = ShutterSpeedHelper.getMinShutterSpeed(characteristics);
        Long maxShutterSpeed = ShutterSpeedHelper.getMaxShutterSpeed(characteristics);
        LogHelper.d(TAG, "[getSupportedList]+ shutter speed range (" +
                minShutterSpeed + ", " + maxShutterSpeed + ")");
        ArrayList<String> values = new ArrayList<String>();
        values.add(AUTO);
        int minValue = Integer.parseInt(String.valueOf(minShutterSpeed / S_TO_NS));
        int maxValue = Integer.parseInt(String.valueOf(maxShutterSpeed / S_TO_NS));
        for (int i = minValue; i <= maxValue; ++i) {
            values.add(String.valueOf(i));
        }
        return getAppSupportedValues(values);
    }

    /**
     * Get supported shutter speed list.
     *
     * @param originalParameters THe original camera parameters.
     * @return The supported shutter speed list.
     */
    public static List<String> getSupportedList(Camera.Parameters originalParameters) {
        ArrayList<String> values = new ArrayList<String>();
        values.add(AUTO);
        String maxExposure = originalParameters.get(KEY_EXPOSURE_TIME_MAX);
        if (maxExposure == null) {
            return getAppSupportedValues(values);
        }
        int maxExposureTime = (Integer.parseInt(maxExposure) / MS_TO_S);
        for (int i = 1; i <= maxExposureTime; ++i) {
            values.add(String.valueOf(i));
        }
        return getAppSupportedValues(values);
    }

    /**
     * Whether shutter speed is supported or not, note that only the maximum shutter speed
     * bigger than 1 second does shutter speed setting be supported.
     *
     * @param characteristics The current CameraCharacteristics.
     * @return Whether current platform supported shutter speed.
     */
    public static boolean isShutterSpeedSupported(CameraCharacteristics characteristics) {
        if (characteristics == null) {
            LogHelper.w(TAG, "[isShutterSpeedSupported] characteristics is null");
            return false;
        }
        Range<Long> shutterSpeedRange =
                characteristics.get(CameraCharacteristics.SENSOR_INFO_EXPOSURE_TIME_RANGE);
        LogHelper.i(TAG, "[isShutterSpeedSupported] shutterSpeedRange "+shutterSpeedRange);
        return shutterSpeedRange != null && shutterSpeedRange.getUpper() >= MIN_SHUTTER_SPEED_NS;
    }

    /**
     * Whether shutter speed is supported or not, note that only the maximum shutter speed
     * bigger than 1 second does shutter speed setting be supported.
     *
     * @param originalParameters The current camera parameters.
     * @return Whether current platform supported shutter speed.
     */
    public static boolean isShutterSpeedSupported(Camera.Parameters originalParameters) {
        if (originalParameters == null) {
            LogHelper.w(TAG, "[isShutterSpeedSupported] originalParameters is null");
            return false;
        }
        String maxExposureTime = originalParameters.get(KEY_EXPOSURE_TIME_MAX);
        if (maxExposureTime == null) {
            LogHelper.w(TAG, "[isShutterSpeedSupported] maxExposureTime is null");
            return false;
        }
        LogHelper.w(TAG, "[isShutterSpeedSupported] maxExposureTime = " + maxExposureTime);
        return maxExposureTime != null
                && (Integer.parseInt(maxExposureTime) / MS_TO_S) >= MIN_SHUTTER_SPEED_S;
    }


    /**
     * Get the minimum shutter speed.
     *
     * @param characteristics The current CameraCharacteristics.
     * @return The minimum shutter speed.
     */
    private static Long getMinShutterSpeed(CameraCharacteristics characteristics) {
        if (!isShutterSpeedSupported(characteristics)) {
            return -1L;
        }
        Range<Long> shutterSpeedRange =
                characteristics.get(CameraCharacteristics.SENSOR_INFO_EXPOSURE_TIME_RANGE);
        if (shutterSpeedRange == null) {
            return -1L;
        }
        Long minShutterSpeed = shutterSpeedRange.getLower();
        LogHelper.d(TAG, "[getMinShutterSpeed] " + minShutterSpeed);
        return minShutterSpeed;
    }

    /**
     * Get the maximum shutter speed.
     *
     * @param characteristics The current CameraCharacteristics.
     * @return The maximum shutter speed.
     */
    private static Long getMaxShutterSpeed(CameraCharacteristics characteristics) {
        if (!isShutterSpeedSupported(characteristics)) {
            return -1L;
        }
        Range<Long> shutterSpeedRange =
                characteristics.get(CameraCharacteristics.SENSOR_INFO_EXPOSURE_TIME_RANGE);
        if (shutterSpeedRange == null) {
            return -1L;
        }
        Long maxShutterSpeed = shutterSpeedRange.getUpper();
        LogHelper.d(TAG, "[getMaxShutterSpeed] " + maxShutterSpeed);
        return maxShutterSpeed;
    }

    private static List<String> getAppSupportedValues(List<String> platformSupportedValues) {
        List<String> appSupportedList = new ArrayList<>();
        appSupportedList.add(AUTO);
        appSupportedList.add(ONE_SECONDS);
        appSupportedList.add(TWO_SECONDS);
        appSupportedList.add(FOUR_SECONDS);
        appSupportedList.add(EIGHT_SECONDS);
        appSupportedList.add(THIRTY_SIX_SECONDS);
        platformSupportedValues.retainAll(appSupportedList);
        LogHelper.i(TAG, "[getAppSupportedValues] supported values " + platformSupportedValues);
        return platformSupportedValues;
    }

}