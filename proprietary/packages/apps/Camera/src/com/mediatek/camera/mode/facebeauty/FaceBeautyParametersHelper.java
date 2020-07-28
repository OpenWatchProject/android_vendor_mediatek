/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.mode.facebeauty;

import android.app.Activity;
import android.content.SharedPreferences;
import android.hardware.Camera.Parameters;
import android.text.TextUtils;

import com.android.camera.R;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.List;

public class FaceBeautyParametersHelper {
    private static final String TAG = "FaceBeautyParametersHelper";
    // if the parameters not have defined this key, get the key's value will be
    // null. so need return the default value to 0;
    private static final String DEFAULT_VALUE = "0";
    public static final int FACEBEAUTY_SMOOTH = 0;
    public static final int FACEBEAUTY_SKIN_COLOR = 1;
    public static final int FACEBEAUTY_SHARP = 2;
    public static final int FACEBEAUTY_BIG_EYES = 3;
    public static final int FACEBEAUTY_SLIM = 4; // for vFB
    public static final int FACEBEAUTY_INTO_NORMAL = 5;

    public static final int VIDEO_FACE_BEAUTY_MAX_SOLUTION_WIDTH = 1920;
    public static final int VIDEO_FACE_BEAUTY_MAX_SOLUTION_HEIGHT = 1088;

    protected static final String KEY_VIDED_FACE_BEAUTY_FACE = "fb-face-pos";


    // private static final String KEY_RECORDING_HINT = "recording-hint";

    private static final String KEY_FACEBEAUTY_SMOOTH = "fb-smooth-level";
    private static final String KEY_FACEBEAUTY_SMOOTH_MAX = "fb-smooth-level-max";
    private static final String KEY_FACEBEAUTY_SMOOTH_MIN = "fb-smooth-level-min";
    private static final String KEY_FACEBEAUTY_SMOOTH_DEFAULT = "fb-smooth-level-default";

    private static final String KEY_FACEBEAUTY_SKIN_COLOR = "fb-skin-color";
    private static final String KEY_FACEBEAUTY_SKIN_COLOR_MAX = "fb-skin-color-max";
    private static final String KEY_FACEBEAUTY_SKIN_COLOR_MIN = "fb-skin-color-min";
    private static final String KEY_FACEBEAUTY_SKIN_COLOR_DEFALUT = "fb-skin-color-default";

    private static final String KEY_FACEBEAUTY_SHARP = "fb-sharp";
    private static final String KEY_FACEBEAUTY_SHARP_MAX = "fb-sharp-max";
    private static final String KEY_FACEBEAUTY_SHARP_MIN = "fb-sharp-min";
    private static final String KEY_FACEBEAUTY_SHARP_DEFAULT = "fb-sharp-default";


    private static final String KEY_FACEBEAUTY_SLIM = "fb-slim-face";
    private static final String KEY_FACEBEAUTY_SLIM_MAX = "fb-slim-face-max";
    private static final String KEY_FACEBEAUTY_SLIM_MIN = "fb-slim-face-min";
    private static final String KEY_FACEBEAUTY_SLIM_DEFAULT = "fb-slim-face-default";


    // follow string should be meet with the prarmeters [Need Check With Daniel]
    // the string is in
    // :mediatek/frameworks-ext/av/camera/MtkCameraParameters.cpp
    private static final String KEY_FACEBEAUTY_BIG_EYES = "fb-enlarge-eye";
    private static final String KEY_FACEBEAUTY_BIG_EYES_MAX = "fb-slim-face-max";
    private static final String KEY_FACEBEAUTY_BIG_EYES_MIN = "fb-slim-face-min";
    private static final String KEY_FACEBEAUTY_BIG_EYES_DEFALUT = "fb-enlarge-eye-default";

    private static final String SUPPORTED_VALUES_SUFFIX = "-values";

    private ICameraContext mICameraContext;
    private IModuleCtrl mIModuleCtrl;
    private ICameraDeviceManager mICameraDeviceManager;
    private ICameraDevice mICameraDevice;
    private ISettingCtrl mISettingCtrl;
    private Activity mActivity;

    public interface ParameterListener {

        public boolean canShowFbIcon(int length);

        public boolean isMultiFbMode();

        public void setVFBSharedPrefences(int index, String value);

        public int getvFbSharedPreferences(int index);

        public void setParameters(int index, String value);

        public int getMaxLevel(int key);

        public int getMinLevel(int key);
    }

    public FaceBeautyParametersHelper(ICameraContext cameraContext) {
        mICameraContext = cameraContext;
        mISettingCtrl = mICameraContext.getSettingController();
        mActivity = mICameraContext.getActivity();
        mIModuleCtrl = mICameraContext.getModuleController();
    }

    public void updateParameters(ICameraDevice device) {
        mICameraDevice = device;
    }

    public ParameterListener getListener() {
        return mParametersListener;
    }
    private ParameterListener mParametersListener = new ParameterListener() {

        @Override
        public boolean canShowFbIcon(int length) {
            return FaceBeautyParametersHelper.this.canShowFbIcon(length);
        }

        @Override
        public void setVFBSharedPrefences(int index, String value) {
            FaceBeautyParametersHelper.this.setVFBSharedPrefences(index, value);

        }

        @Override
        public int getvFbSharedPreferences(int index) {
            return FaceBeautyParametersHelper.this.getvFbSharedPreferences(index);
        }

        @Override
        public void setParameters(int index, String value) {
            FaceBeautyParametersHelper.this.setParameters(index, value);
        }

        @Override
        public int getMaxLevel(int key) {
            return FaceBeautyParametersHelper.this.getMaxLevel(key);
        }

        @Override
        public int getMinLevel(int key) {
            return FaceBeautyParametersHelper.this.getMinLevel(key);
        }

        @Override
        public boolean isMultiFbMode() {
            return FaceBeautyParametersHelper.this.isMultiFbMode();
        }

    };

    private int getMaxLevel(int key) {
        String defalutKey = null;
        switch (key) {
        case FACEBEAUTY_SMOOTH:
            defalutKey = KEY_FACEBEAUTY_SMOOTH_DEFAULT;
            return getInt(KEY_FACEBEAUTY_SMOOTH_MAX, Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_SKIN_COLOR:
            defalutKey = KEY_FACEBEAUTY_SKIN_COLOR_DEFALUT;
            return getInt(KEY_FACEBEAUTY_SKIN_COLOR_MAX,
                    Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_SHARP:
            defalutKey = KEY_FACEBEAUTY_SHARP_DEFAULT;
            return getInt(KEY_FACEBEAUTY_SHARP_MAX, Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_BIG_EYES:
            defalutKey = KEY_FACEBEAUTY_BIG_EYES_DEFALUT;
            return getInt(KEY_FACEBEAUTY_BIG_EYES_MAX,
                    Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_SLIM:
            defalutKey = KEY_FACEBEAUTY_SLIM_DEFAULT;
            return getInt(KEY_FACEBEAUTY_SLIM_MAX, Integer.parseInt(getDefaultValue(defalutKey)));

        default:
            return Integer.parseInt(DEFAULT_VALUE);
        }
    }

    private int getMinLevel(int key) {
        String defalutKey = null;
        switch (key) {
        case FACEBEAUTY_SMOOTH:
            defalutKey = KEY_FACEBEAUTY_SMOOTH_DEFAULT;
            return getInt(KEY_FACEBEAUTY_SMOOTH_MIN, Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_SKIN_COLOR:
            defalutKey = KEY_FACEBEAUTY_SKIN_COLOR_DEFALUT;
            return getInt(KEY_FACEBEAUTY_SKIN_COLOR_MIN,
                    Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_SHARP:
            defalutKey = KEY_FACEBEAUTY_SHARP_DEFAULT;
            return getInt(KEY_FACEBEAUTY_SHARP_MIN, Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_BIG_EYES:
            defalutKey = KEY_FACEBEAUTY_BIG_EYES_DEFALUT;
            return getInt(KEY_FACEBEAUTY_BIG_EYES_MIN,
                    Integer.parseInt(getDefaultValue(defalutKey)));

        case FACEBEAUTY_SLIM:
            defalutKey = KEY_FACEBEAUTY_SLIM_DEFAULT;
            return getInt(KEY_FACEBEAUTY_SLIM_MIN, Integer.parseInt(getDefaultValue(defalutKey)));

        default:
            return Integer.parseInt(DEFAULT_VALUE);
        }
    }

    // Copied from android.hardware.Camera
    // Splits a comma delimited string to an ArrayList of String.
    // Return null if the passing string is null or the size is 0.
    private ArrayList<String> split(String str) {
        ArrayList<String> substrings = null;
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            substrings = new ArrayList<String>();
            for (String s : splitter) {
                substrings.add(s);
            }
        }

        return substrings;
    }

    private List<String> getSupportedValues(Parameters parameters, String key) {
        List<String> supportedList = null;
        if (parameters != null) {
            String str = parameters.get(key + SUPPORTED_VALUES_SUFFIX);
            supportedList = split(str);
        }
        return supportedList;
    }

    // Returns the value of a integer parameter.
    private int getInt(String key, int defaultValue) {
        if (mICameraDevice != null) {
            try {
                return Integer.parseInt(mICameraDevice.getParameter(key));
            } catch (NumberFormatException ex) {
                return defaultValue;
            }
        } else {
            return defaultValue;
        }
    }

    private void setParameters(int index, String value) {
        if (mICameraDevice == null) {
            return;
        }
        switch (index) {
        case FACEBEAUTY_SMOOTH:
            mICameraDevice.setParameter(KEY_FACEBEAUTY_SMOOTH, value);
            break;

        case FACEBEAUTY_SKIN_COLOR:
            mICameraDevice.setParameter(KEY_FACEBEAUTY_SKIN_COLOR, value);
            break;

        case FACEBEAUTY_SLIM:
            mICameraDevice.setParameter(KEY_FACEBEAUTY_SLIM, value);
            break;

        case FACEBEAUTY_BIG_EYES:
            mICameraDevice.setParameter(KEY_FACEBEAUTY_BIG_EYES, value);
            break;

        default:
            break;
        }
        mICameraDevice.applyParameters();
    }

    private int getvFbSharedPreferences(int index) {
        String key = null;
        String defalutValueKey = null;
        switch (index) {
        case FACEBEAUTY_SMOOTH:
            key = SettingConstants.KEY_FACE_BEAUTY_SMOOTH;
            defalutValueKey = KEY_FACEBEAUTY_SMOOTH_DEFAULT;
            break;

        case FACEBEAUTY_SKIN_COLOR:
            key = SettingConstants.KEY_FACE_BEAUTY_SKIN_COLOR;
            defalutValueKey = KEY_FACEBEAUTY_SKIN_COLOR_DEFALUT;
            break;

        case FACEBEAUTY_SLIM:
            key = SettingConstants.KEY_FACE_BEAUTY_SLIM;
            defalutValueKey = KEY_FACEBEAUTY_SLIM_DEFAULT;
            break;

        case FACEBEAUTY_BIG_EYES:
            key = SettingConstants.KEY_FACE_BEAUTY_BIG_EYES;
            defalutValueKey = KEY_FACEBEAUTY_BIG_EYES_DEFALUT;
            break;

        default:
            Log.i(TAG, "getvFbSharedPreferences,the key is null please check the string");
            break;
        }

        return getVFBSharedPreference(key, getDefaultValue(defalutValueKey));
    }

    private int getVFBSharedPreference(String key, String defValue) {
        String values = mIModuleCtrl.getComboPreferences().getString(key, defValue);
        Log.d(TAG, "[getVFBSharedPreference]get the effects value from sharedpreferences ,key = "
                + key + ",defalut value is :" + defValue + ",return value is :" + values);
        return Integer.parseInt(values);
    }

    private void setVFBSharedPrefences(int index, String value) {
        Log.d(TAG, "[setVFBSharedPrefences],index = " + index + ",value = " + value);
        SharedPreferences.Editor mEditor = mIModuleCtrl.getComboPreferences().getLocal().edit();
        switch (index) {
        case FaceBeautyParametersHelper.FACEBEAUTY_SMOOTH:
            mEditor.putString(SettingConstants.KEY_FACE_BEAUTY_SMOOTH, value);
            break;

        case FaceBeautyParametersHelper.FACEBEAUTY_SKIN_COLOR:
            mEditor.putString(SettingConstants.KEY_FACE_BEAUTY_SKIN_COLOR, value);
            break;

        case FaceBeautyParametersHelper.FACEBEAUTY_SLIM:
            mEditor.putString(SettingConstants.KEY_FACE_BEAUTY_SLIM, value);
            break;

        case FaceBeautyParametersHelper.FACEBEAUTY_BIG_EYES:
            mEditor.putString(SettingConstants.KEY_FACE_BEAUTY_BIG_EYES, value);
            break;

        case FaceBeautyParametersHelper.FACEBEAUTY_INTO_NORMAL:
            mEditor.putInt(SettingConstants.KEY_INTO_VIDEO_FACE_BEAUTY_NORMAL,
                    Integer.parseInt(value));
            break;

        default:
            break;
        }
        mEditor.apply();
    }

    private boolean isMultiFbMode() {
        // when setting is ready can use the interface replace
        String value = mISettingCtrl
                .getSettingValue(SettingConstants.KEY_MULTI_FACE_BEAUTY);
        Log.d(TAG, "isMultiFbMode,getCurrentFbMode: " + value);
        return mActivity.getResources().getString(R.string.face_beauty_multi_mode).equals(value);
    }

    private boolean canShowFbIcon(int length) {
        // TODO,need check not supported show FB icon when FD is detected
        // here maybe add other condition,so use this function
        boolean needshow = length > 0;
        return needshow;
    }

    private String getDefaultValue(String key) {
        String valaue = mICameraDevice.getParameter(key);
        Log.d(TAG, "[getDefaultValue] key = " + key + ",valaue = " + valaue);
        if (valaue == null) {
            valaue = DEFAULT_VALUE;
            Log.d(TAG, "[getDefaultValue] the key = " + key +
                    " not exsit,so return the value to 0");
        }
        return valaue;
    }
}
