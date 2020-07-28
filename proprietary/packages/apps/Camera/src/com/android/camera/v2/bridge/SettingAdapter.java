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

package com.android.camera.v2.bridge;

import android.os.Handler;

import com.android.camera.v2.app.SettingAgent;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.services.CameraServices;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.setting.SettingCtrl.ISettingFilterListener;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;


public class SettingAdapter implements SettingAgent {
    private static final Tag TAG = new Tag(SettingAdapter.class.getSimpleName());
    private final SettingCtrl                mSettingCtrl;
    private static Map<String, String>       mKeyMapping = new HashMap<String, String>();
    private Map<String, String>              mInvertedKeyMapping = new HashMap<String, String>();
    private Map<SettingChangedListener,
            ISettingFilterListener>          mListenersMapping
            = new HashMap<SettingChangedListener, ISettingFilterListener>();

    public SettingAdapter(AppControllerAdapter appControllerAdapter) {
        CameraServices service = appControllerAdapter.getServices();
        mSettingCtrl = service.getSettingController();
        invertMapping();
    }

    @Override
    public List<String> getSupportedValues(String key, String cameraId) {
        List<String> supportedValues =
                mSettingCtrl.getSupportedValues(mKeyMapping.get(key), cameraId);
        return supportedValues;
    }

    @Override
    public void configurateSetting(Map<String, String> defaultSettings) {
        Map<String, String> newChangedSettings = new LinkedHashMap<String, String>();
        Set<String> keys = defaultSettings.keySet();
        Iterator<String> iterator = keys.iterator();

        while (iterator.hasNext()) {
            String key = iterator.next();
            String value = defaultSettings.get(key);
            String newKey = mKeyMapping.get(key);
            if (newKey != null) {
                newChangedSettings.put(newKey, value);
            } else {
                LogHelper.i(TAG, "[doSettingChanged], key:" + key + ", newKey:" + newKey);
            }
        }

        mSettingCtrl.configurateSetting(newChangedSettings);
    }

    @Override
    public void doSettingChange(String key, String value) {
        String newKey = mKeyMapping.get(key);
        if (newKey == null) {
            LogHelper.i(TAG, "[doSettingChanged], key:" + key + ", newKey:" + newKey);
            return;
        }
        mSettingCtrl.doSettingChange(newKey, value);
    }

    @Override
    public void doSettingChange(Map<String, String> changedSettings) {
        Map<String, String> newChangedSettings = new HashMap<String, String>();
        Set<String> keys = changedSettings.keySet();
        Iterator<String> iterator = keys.iterator();

        while (iterator.hasNext()) {
            String key = iterator.next();
            String value = changedSettings.get(key);
            String newKey = mKeyMapping.get(key);
            if (newKey != null) {
                newChangedSettings.put(newKey, value);
            } else {
                LogHelper.i(TAG, "[doSettingChanged], key:" + key + ", newKey:" + newKey);
            }
        }

        mSettingCtrl.doSettingChange(newChangedSettings);
    }

    @Override
    public void registerSettingChangedListener(final SettingChangedListener listener,
            Handler handler) {
        ISettingFilterListener settingFilterListener = new ISettingFilterListener() {

            @Override
            public void onFilterResult(Map<String, String> values,
                    Map<String, String> overrideValues) {
                Map<String, String> newValues = new HashMap<String, String>();
                Map<String, String> newOverrideValues = new HashMap<String, String>();

                Set<String> keys = values.keySet();
                Iterator<String> iterator = keys.iterator();
                while (iterator.hasNext()) {
                    String key = iterator.next();
                    String value = values.get(key);
                    String overrideValue = overrideValues.get(key);
                    String newKey = mInvertedKeyMapping.get(key);
                    newValues.put(newKey, value);
                    if (overrideValues.containsKey(key)) {
                        newOverrideValues.put(newKey, overrideValue);
                    }
                }

                listener.onSettingResult(newValues, newOverrideValues);
            }
        };
        mListenersMapping.put(listener, settingFilterListener);
        mSettingCtrl.registerSettingFilterListener(settingFilterListener, handler);
    }

    @Override
    public void unRegisterSettingChangedListener(SettingChangedListener listener) {
        mSettingCtrl.unRegisterSettingFilterListener(mListenersMapping.get(listener));
    }

    @Override
    public String getSharedPreferencesValue(String key, String cameraId) {
        return mSettingCtrl.getSharePreferenceValue(mKeyMapping.get(key), cameraId);
    }

    @Override
    public void clearSharedPreferencesValue(String[] keys, String cameraId) {
        String[] newKeys = new String[keys.length];
        for (int i = 0; i < keys.length; i++) {
            String key = keys[i];
            String newKey = mKeyMapping.get(key);
            newKeys[i] = newKey;
        }
        mSettingCtrl.clearSharedPreferencesValue(newKeys, cameraId);
    }

    private void invertMapping() {
        Set<String> keys = mKeyMapping.keySet();
        Iterator<String> iterator = keys.iterator();
        while (iterator.hasNext()) {
            String key = iterator.next();
            String value = mKeyMapping.get(key);
            mInvertedKeyMapping.put(value, key);
        }
    }

    static {
        mKeyMapping.put(SettingKeys.KEY_RECORD_LOCATION,
                com.mediatek.camera.v2.util.SettingKeys.KEY_RECORD_LOCATION);
        mKeyMapping.put(SettingKeys.KEY_VIDEO_QUALITY,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VIDEO_QUALITY);
        mKeyMapping.put(SettingKeys.KEY_SLOW_MOTION_VIDEO_QUALITY,
                com.mediatek.camera.v2.util.SettingKeys.KEY_SLOW_MOTION_VIDEO_QUALITY);
        mKeyMapping.put(SettingKeys.KEY_PICTURE_SIZE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_PICTURE_SIZE);
        mKeyMapping.put(SettingKeys.KEY_FLASH,
                com.mediatek.camera.v2.util.SettingKeys.KEY_FLASH);
        mKeyMapping.put(SettingKeys.KEY_WHITE_BALANCE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_WHITE_BALANCE);
        mKeyMapping.put(SettingKeys.KEY_SCENE_MODE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_SCENE_MODE);
        mKeyMapping.put(SettingKeys.KEY_EXPOSURE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_EXPOSURE);
        mKeyMapping.put(SettingKeys.KEY_ISO,
                com.mediatek.camera.v2.util.SettingKeys.KEY_ISO);
        mKeyMapping.put(SettingKeys.KEY_COLOR_EFFECT,
                com.mediatek.camera.v2.util.SettingKeys.KEY_COLOR_EFFECT);
        mKeyMapping.put(SettingKeys.KEY_CAMERA_ZSD,
                com.mediatek.camera.v2.util.SettingKeys.KEY_CAMERA_ZSD);
        mKeyMapping.put(SettingKeys.KEY_STEREO3D_PICTURE_SIZE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_STEREO3D_PICTURE_SIZE);
        mKeyMapping.put(SettingKeys.KEY_STEREO3D_MODE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_STEREO3D_MODE);
        mKeyMapping.put(SettingKeys.KEY_STEREO3D_PICTURE_FORMAT,
                com.mediatek.camera.v2.util.SettingKeys.KEY_STEREO3D_PICTURE_FORMAT);
        mKeyMapping.put(SettingKeys.KEY_VIDEO_RECORD_AUDIO,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VIDEO_RECORD_AUDIO);
        mKeyMapping.put(SettingKeys.KEY_VIDEO_HD_AUDIO_RECORDING,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VIDEO_HD_AUDIO_RECORDING);
        mKeyMapping.put(SettingKeys.KEY_IMAGE_PROPERTIES,
                com.mediatek.camera.v2.util.SettingKeys.KEY_IMAGE_PROPERTIES);
        mKeyMapping.put(SettingKeys.KEY_EDGE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_EDGE);
        mKeyMapping.put(SettingKeys.KEY_HUE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_HUE);
        mKeyMapping.put(SettingKeys.KEY_SATURATION,
                com.mediatek.camera.v2.util.SettingKeys.KEY_SATURATION);
        mKeyMapping.put(SettingKeys.KEY_BRIGHTNESS,
                com.mediatek.camera.v2.util.SettingKeys.KEY_BRIGHTNESS);
        mKeyMapping.put(SettingKeys.KEY_CONTRAST,
                com.mediatek.camera.v2.util.SettingKeys.KEY_CONTRAST);
        mKeyMapping.put(SettingKeys.KEY_SELF_TIMER,
                com.mediatek.camera.v2.util.SettingKeys.KEY_SELF_TIMER);
        mKeyMapping.put(SettingKeys.KEY_ANTI_BANDING,
                com.mediatek.camera.v2.util.SettingKeys.KEY_ANTI_BANDING);
        mKeyMapping.put(SettingKeys.KEY_VIDEO_EIS,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VIDEO_EIS);
        mKeyMapping.put(SettingKeys.KEY_VIDEO_3DNR,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VIDEO_3DNR);
        mKeyMapping.put(SettingKeys.KEY_CONTINUOUS_NUMBER,
                com.mediatek.camera.v2.util.SettingKeys.KEY_CONTINUOUS_NUMBER);
        mKeyMapping.put(SettingKeys.KEY_DUAL_CAMERA_MODE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_DUAL_CAMERA_MODE);
        mKeyMapping.put(SettingKeys.KEY_FAST_AF,
                com.mediatek.camera.v2.util.SettingKeys.KEY_FAST_AF);
        mKeyMapping.put(SettingKeys.KEY_DISTANCE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_DISTANCE);
        mKeyMapping.put(SettingKeys.KEY_PICTURE_RATIO,
                com.mediatek.camera.v2.util.SettingKeys.KEY_PICTURE_RATIO);
        mKeyMapping.put(SettingKeys.KEY_VOICE,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VOICE);
        mKeyMapping.put(SettingKeys.KEY_CAMERA_FACE_DETECT,
                com.mediatek.camera.v2.util.SettingKeys.KEY_CAMERA_FACE_DETECT);
        mKeyMapping.put(SettingKeys.KEY_PANORAMA,
                com.mediatek.camera.v2.util.SettingKeys.KEY_PANORAMA);
        mKeyMapping.put(SettingKeys.KEY_HDR,
                com.mediatek.camera.v2.util.SettingKeys.KEY_HDR);
        mKeyMapping.put(SettingKeys.KEY_ASD,
                com.mediatek.camera.v2.util.SettingKeys.KEY_ASD);
        mKeyMapping.put(SettingKeys.KEY_PHOTO_PIP,
                com.mediatek.camera.v2.util.SettingKeys.KEY_PHOTO_PIP);
        mKeyMapping.put(SettingKeys.KEY_VIDEO_PIP,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VIDEO_PIP);
        mKeyMapping.put(SettingKeys.KEY_VIDEO,
                com.mediatek.camera.v2.util.SettingKeys.KEY_VIDEO);
        mKeyMapping.put(SettingKeys.KEY_REFOCUS,
                com.mediatek.camera.v2.util.SettingKeys.KEY_REFOCUS);
        mKeyMapping.put(SettingKeys.KEY_NORMAL,
                com.mediatek.camera.v2.util.SettingKeys.KEY_NORMAL);
        mKeyMapping.put(SettingKeys.KEY_CAMERA_ID,
                com.mediatek.camera.v2.util.SettingKeys.KEY_CAMERA_ID);
        mKeyMapping.put(SettingKeys.KEY_SLOW_MOTION,
                com.mediatek.camera.v2.util.SettingKeys.KEY_SLOW_MOTION);
        mKeyMapping.put(SettingKeys.KEY_CAMERA_AIS,
                com.mediatek.camera.v2.util.SettingKeys.KEY_CAMERA_AIS);
        mKeyMapping.put(SettingKeys.KEY_DNG,
                com.mediatek.camera.v2.util.SettingKeys.KEY_DNG);
    }
}
