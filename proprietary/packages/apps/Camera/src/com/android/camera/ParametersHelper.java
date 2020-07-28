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

import java.util.ArrayList;
import java.util.List;

import android.hardware.Camera.Parameters;
import android.text.TextUtils;

public class ParametersHelper {
    private static final String TAG = "ParametersHelper";

    public static final int FACEBEAUTY_SMOOTH = 0;
    public static final int FACEBEAUTY_SKIN_COLOR = 1;
    public static final int FACEBEAUTY_SHARP = 2;
    public static final int FACEBEAUTY_BIG_EYES = 3;
    public static final int FACEBEAUTY_SLIM = 4; // for vFB
    public static final int FACEBEAUTY_INTO_NORMAL = 5;

    public static final String KEY_RECORDING_HINT = "recording-hint";

    public static final String KEY_FACEBEAUTY_SMOOTH = "fb-smooth-level";
    public static final String KEY_FACEBEAUTY_SMOOTH_MAX = "fb-smooth-level-max";
    public static final String KEY_FACEBEAUTY_SMOOTH_MIN = "fb-smooth-level-min";

    public static final String KEY_FACEBEAUTY_SKIN_COLOR = "fb-skin-color";
    public static final String KEY_FACEBEAUTY_SKIN_COLOR_MAX = "fb-skin-color-max";
    public static final String KEY_FACEBEAUTY_SKIN_COLOR_MIN = "fb-skin-color-min";

    public static final String KEY_FACEBEAUTY_SHARP = "fb-sharp";
    public static final String KEY_FACEBEAUTY_SHARP_MAX = "fb-sharp-max";
    public static final String KEY_FACEBEAUTY_SHARP_MIN = "fb-sharp-min";

    public static final String KEY_FACEBEAUTY_SLIM = "fb-slim-face";
    public static final String KEY_FACEBEAUTY_SLIM_MAX = "fb-slim-face-max";
    public static final String KEY_FACEBEAUTY_SLIM_MIN = "fb-slim-face-min";

    // follow string should be meet with the prarmeters [Need Check With Daniel]
    // the string is in
    // :mediatek/frameworks-ext/av/camera/MtkCameraParameters.cpp
    public static final String KEY_FACEBEAUTY_BIG_EYES = "fb-enlarge-eye";
    public static final String KEY_FACEBEAUTY_BIG_EYES_MAX = "fb-slim-face-max";
    public static final String KEY_FACEBEAUTY_BIG_EYES_MIN = "fb-slim-face-min";
    // Special case for private key in android.hardware.Camera.
    // Here we defined it for supplying same behavior for get/set/isSupported.
    public static final String KEY_VIDEO_HDR = "video-hdr";
    public static final String KEY_SLOW_MOTION = "slow-motion";
    public static final String KEY_AIS_MFLL = "mfb";
    public static final String KEY_3DNR_MODE = "3dnr-mode";
    public static final String KEY_FIRST_PREVIEW_FRAME = "first-preview-frame-black";
    private static final String SUPPORTED_VALUES_SUFFIX = "-values";
    private static final String KEY_NATIVE_PIP_SUPPORTED = "native-pip-supported";
    private static final String KEY_MAX_FRAME_RATE_ZSD_ON = "pip-fps-zsd-on";
    private static final String KEY_MAX_FRAME_RATE_ZSD_OFF = "pip-fps-zsd-off";
    private static final String KEY_DYNAMIC_FRAME_RATE_SUPPORTED = "dynamic-frame-rate-supported";
    private static final String KEY_DYNAMIC_FRAME_RATE = "dynamic-frame-rate";
    private static final String KEY_MUTE_RECORDING_SOUND = "rec-mute-ogg";
    private static final String TRUE = "true";
    private static final String FALSE = "false";
    private static final String KEY_IMAGE_REFOCUS_SUPPORTED = "stereo-image-refocus-values";
    private static final String KEY_VSDOF_MODE_SUPPORTED = "stereo-vsdof-mode-values";
    private static final String KEY_DENOISE_MODE_SUPPORTED = "stereo-denoise-mode-values";
    private static final String KEY_STEREO_CAPTURE_MODULE = "stereo-capture-supported-module";
    private static final String KEY_STEREO_VSDOF_MODULE = "stereo-vsdof-supported-module";
    private static final String KEY_STEREO_DENOISE_MODULE = "stereo-denoise-supported-module";
    private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
    private static final String KEY_PANEL_SIZE = "panel-size";

    private static final int BACK_CAMERA_ID = 0;
    private static final int FRONT_CAMERA_ID = 1;
    //Stereo feature support with sensor, -1 ->not support
    // 0-->only back support, 1-->only front support, 2-->all support
    private static int sStereoCaptureSupportBySensor = -1;
    private static int sStereoDenoiseSupportBySensor = -1;

    public static List<String> getSupportedVideoHdr(Parameters parameters) {
        return getSupportedValues(parameters, KEY_VIDEO_HDR);
    }

    public static boolean isNativePIPSupported(Parameters parameters) {
        if (parameters == null) {
            return false;
        }
        String str = parameters.get(KEY_NATIVE_PIP_SUPPORTED);
        return TRUE.equals(str);
    }

    public static List<Integer> getPIPFrameRateZSDOn(Parameters parameters) {
        String str = parameters.get(KEY_MAX_FRAME_RATE_ZSD_ON);
        return splitInt(str);
    }

    public static List<Integer> getPIPFrameRateZSDOff(Parameters parameters) {
        String str = parameters.get(KEY_MAX_FRAME_RATE_ZSD_OFF);
        return splitInt(str);
    }
    public static boolean isDynamicFrameRateSupported(Parameters parameters) {
        String str = parameters.get(KEY_DYNAMIC_FRAME_RATE_SUPPORTED);
        return TRUE.equals(str);
    }

    public static void setDynamicFrameRate(Parameters parameters, boolean toggle) {
        parameters.set(KEY_DYNAMIC_FRAME_RATE, toggle ? TRUE : FALSE);
    }

    public static List<String> getSupported3Dnr(Parameters parameters) {
        return getSupportedValues(parameters, KEY_3DNR_MODE);
    }

    public static void enableRecordingSound(Parameters parameters, String value) {
        if (value.equals("1") || value.equals("0")) {
            parameters.set(KEY_MUTE_RECORDING_SOUND, value);
        }
    }

    public static List<String> getSupportAisMfll(Parameters parameters) {
        return getSupportedValues(parameters, KEY_AIS_MFLL);
    }

    public static boolean isFaceBeautySupported(Parameters parameters) {
        return FeatureSwitcher.isCfbEnable()
                && isSupporteFBProperties(parameters, FACEBEAUTY_SMOOTH);
    }

    public static List<String> getSupportedSlowMotion(Parameters parameters) {
        return getSupportedValues(parameters, KEY_SLOW_MOTION); // parameters.get();
    }

    public static String getSlowMotion(Parameters parameters) {
        return parameters.get(KEY_SLOW_MOTION);
    }

    public static int getCurrentValue(Parameters parameters, int index) {
        switch (index) {
        case FACEBEAUTY_SMOOTH:
            return getInt(parameters, KEY_FACEBEAUTY_SMOOTH, 0);
        case FACEBEAUTY_SKIN_COLOR:
            return getInt(parameters, KEY_FACEBEAUTY_SKIN_COLOR, 0);
        case FACEBEAUTY_SHARP:
            return getInt(parameters, KEY_FACEBEAUTY_SHARP, 0);
        case FACEBEAUTY_BIG_EYES:
            return getInt(parameters, KEY_FACEBEAUTY_BIG_EYES, 0);
        case FACEBEAUTY_SLIM:
            return getInt(parameters, KEY_FACEBEAUTY_SLIM, 0);
        default:
            return 0;
        }
    }

    public static boolean isSupporteFBProperties(Parameters parameters, int type) {
        int max = getMaxLevel(parameters, type);
        int min = getMinLevel(parameters, type);
        return max != 0 && min != 0;
    }

    public static int getMaxLevel(Parameters parameters, int key) {
        switch (key) {
        case FACEBEAUTY_SMOOTH:
            return getInt(parameters, KEY_FACEBEAUTY_SMOOTH_MAX, 0);
        case FACEBEAUTY_SKIN_COLOR:
            return getInt(parameters, KEY_FACEBEAUTY_SKIN_COLOR_MAX, 0);
        case FACEBEAUTY_SHARP:
            return getInt(parameters, KEY_FACEBEAUTY_SHARP_MAX, 0);
        case FACEBEAUTY_BIG_EYES:
            return getInt(parameters, KEY_FACEBEAUTY_BIG_EYES_MAX, 0);
        case FACEBEAUTY_SLIM:
            return getInt(parameters, KEY_FACEBEAUTY_SLIM_MAX, 0);
        default:
            return 0;
        }
    }

    public static int getMinLevel(Parameters parameters, int key) {
        switch (key) {
        case FACEBEAUTY_SMOOTH:
            return getInt(parameters, KEY_FACEBEAUTY_SMOOTH_MIN, 0);
        case FACEBEAUTY_SKIN_COLOR:
            return getInt(parameters, KEY_FACEBEAUTY_SKIN_COLOR_MIN, 0);
        case FACEBEAUTY_SHARP:
            return getInt(parameters, KEY_FACEBEAUTY_SHARP_MIN, 0);
        case FACEBEAUTY_BIG_EYES:
            return getInt(parameters, KEY_FACEBEAUTY_BIG_EYES_MIN, 0);
        case FACEBEAUTY_SLIM:
            return getInt(parameters, KEY_FACEBEAUTY_SLIM_MIN, 0);
        default:
            return 0;
        }
    }

    /*
     * MR1 put HDR in scene mode. So, here we don't put it into user list. In
     * apply logic, HDR will set in scene mode and show auto scene to final
     * user. If scene mode not find in ListPreference, first one(auto) will be
     * choose. I don't think this is a good design.
     */
    public static final String KEY_SCENE_MODE_HDR = "hdr";
    // special scene mode for operator, like auto.
    public static final String KEY_SCENE_MODE_NORMAL = "normal";
    public static final String ZSD_MODE_ON = "on";
    public static final String ZSD_MODE_OFF = "off";

    // Copied from android.hardware.Camera
    // Splits a comma delimited string to an ArrayList of String.
    // Return null if the passing string is null or the size is 0.
    public static ArrayList<String> split(String str) {
        ArrayList<String> substrings = null;
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            substrings = new ArrayList<String>();
            for (String s : splitter) {
                substrings.add(s);
            }
        }
//        Log.d(TAG, "split(" + str + ") return " + substrings);
        return substrings;
    }

    private static ArrayList<Integer> splitInt(String str) {
        if (str == null)
            return null;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<Integer> substrings = new ArrayList<Integer>();
        for (String s : splitter) {
            substrings.add(Integer.parseInt(s));
        }
        if (substrings.size() == 0)
            return null;
        return substrings;
    }

    public static List<String> getSupportedValues(Parameters parameters, String key) {
        List<String> supportedList = null;
        if (parameters != null) {
            String str = parameters.get(key + SUPPORTED_VALUES_SUFFIX);
            supportedList = split(str);
        }
        return supportedList;
    }

    // Returns the value of a integer parameter.
    public static int getInt(Parameters parameters, String key, int defaultValue) {
        if (parameters != null) {
            try {
                return Integer.parseInt(parameters.get(key));
            } catch (NumberFormatException ex) {
                return defaultValue;
            }
        } else {
            return defaultValue;
        }
    }

    public static void setFbPropertiesParameters(Parameters parameters, int index, String value) {
//        Log.i(TAG, "setFbPropertiesParameters,index = " + index + ",value = " + value);
        switch (index) {
        case FACEBEAUTY_SMOOTH:
            parameters.set(KEY_FACEBEAUTY_SMOOTH, value);
            break;

        case FACEBEAUTY_SKIN_COLOR:
            parameters.set(KEY_FACEBEAUTY_SKIN_COLOR, value);
            break;

        case FACEBEAUTY_SLIM:
            parameters.set(KEY_FACEBEAUTY_SLIM, value);
            break;

        case FACEBEAUTY_BIG_EYES:
            parameters.set(KEY_FACEBEAUTY_BIG_EYES, value);
            break;

        default:
            break;
        }
    }

    public static boolean isImageRefocusSupported(Parameters parameters) {
        if (parameters == null) {
            return false;
        }
        String str = parameters.get(KEY_IMAGE_REFOCUS_SUPPORTED);
        if ("off".equals(str) || null == str) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * get camera id which stereo capture is supported.
     * @param parameters for check.
     * @return 0-->back camera support, 1-->front camera support
     *         2-->both camera support.
     */
    public static int getIdStereoCaptureSupt(Parameters parameters) {
        if (sStereoCaptureSupportBySensor != -1) {
            return sStereoCaptureSupportBySensor;
        }
        if (parameters != null) {
            String str = parameters.get(KEY_STEREO_CAPTURE_MODULE);
            Log.d(TAG, "getIdStereoCaptureSupt str = " + str);
            if (str != null) {
                if (str.contains("rear") && str.contains("front")) {
                    sStereoCaptureSupportBySensor = 2;
                } else if (str.contains("rear")) {
                    sStereoCaptureSupportBySensor = 0;
                } else if (str.contains("front")) {
                    sStereoCaptureSupportBySensor = 1;
                }
            }
        }
        return sStereoCaptureSupportBySensor;
    }

    /**
     * Check if native is supported.
     * Need setData callback when stereo capture
     * @param parameters for check.
     * @return true if native supported.
     */
    public static boolean isVsDofSupported(Parameters parameters) {
        if (parameters == null) {
            return false;
        }
        String str = parameters.get(KEY_VSDOF_MODE_SUPPORTED);
        if ("off".equals(str) || null == str) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * Check if native is supported.
     * Need setData callback when stereo capture
     * @param parameters for check.
     * @return true if native supported.
     */
    public static boolean isDenoiseSupported(Parameters parameters) {
        if (parameters == null) {
            return false;
        }
        String str = parameters.get(KEY_DENOISE_MODE_SUPPORTED);
        Log.d(TAG, "isDenoiseSupported str = " + str);
        if ("off".equals(str) || null == str) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * get camera id which stereo de-noise is supported.
     * @param parameters for check.
     * @return 0-->back camera support, 1-->front camera support
     *         2-->both camera support.
     */
    public static int getIdStereoDenoiseSupt(Parameters parameters) {
        if (sStereoDenoiseSupportBySensor != -1) {
            return sStereoDenoiseSupportBySensor;
        }
        if (parameters != null) {
            String str = parameters.get(KEY_STEREO_DENOISE_MODULE);
            Log.d(TAG, "getIdStereoCaptureSupt str = " + str);
            if (str != null) {
                if (str.contains("rear") && str.contains("front")) {
                    sStereoDenoiseSupportBySensor = 2;
                } else if (str.contains("rear")) {
                    sStereoDenoiseSupportBySensor = 0;
                } else if (str.contains("front")) {
                    sStereoDenoiseSupportBySensor = 1;
                }
            }
        }
        return sStereoDenoiseSupportBySensor;
    }

    /**
     * add for display 2nd bypass MDP,normal flow.
     * 1.check whether camera service supported this by check the value of KEY_DISP_ROT_SUPPORTED
     * 2.Application need to tell camera service the panel size for get new supported preview sizes
     * 3.Application find preview size by new supported preview sizes (no less than panel size)
     *
     * @param parameters camera paramter
     * @return whether camera display roate is supported.
     */
    public static boolean isDisplayRotateSupported(Parameters parameters) {
        String disp_rot_supported = parameters.get(KEY_DISP_ROT_SUPPORTED);
        if (disp_rot_supported == null || FALSE.equals(disp_rot_supported)) {
//            Log.i(TAG, "isDisplayRotateSupported: false.");
            return false;
        }
        return true;
    }

    public static void setPanelSize(Parameters parameters, String panelSize) {
//        Log.i(TAG, "setPanelSize:" + panelSize);
        if (panelSize != null) {
            parameters.set(KEY_PANEL_SIZE, panelSize);
        }
    }
}
