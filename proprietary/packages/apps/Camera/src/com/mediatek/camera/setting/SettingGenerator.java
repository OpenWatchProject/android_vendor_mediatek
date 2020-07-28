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
package com.mediatek.camera.setting;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Point;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.text.TextUtils;
import android.util.FloatMath;

import com.android.camera.R;
import com.android.camera.FeatureSwitcher;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.setting.preference.PreferenceGroup;
import com.mediatek.camera.setting.preference.PreferenceInflater;
import com.mediatek.camera.setting.preference.SharedPreferencesTransfer;
import com.mediatek.camera.util.Log;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

public class SettingGenerator {
    private static final String TAG = "SettingGenerator";
    public static final int VSDOF_QUALITY_HIGH = CamcorderProfile.QUALITY_1080P;
    public static boolean isSupport4K2K = false;
    private static final int VIDEO_2K42_WIDTH = 3840;

    public static final String QUALITY_HIGH_SPEED_480P
        = Integer.toString(CamcorderProfile.QUALITY_HIGH_SPEED_480P);
    public static final String QUALITY_HIGH_SPEED_720P
        = Integer.toString(CamcorderProfile.QUALITY_HIGH_SPEED_720P);
    public static final String QUALITY_HIGH_SPEED_1080P
        = Integer.toString(CamcorderProfile.QUALITY_HIGH_SPEED_1080P);
    public static final String QUALITY_HIGH_SPEED_2160P
        = Integer.toString(CamcorderProfile.QUALITY_HIGH_SPEED_2160P);

    private static final String QUALITY_QCIF
         = Integer.toString(CamcorderProfile.QUALITY_QCIF);
    private static final String QUALITY_CIF
        = Integer.toString(CamcorderProfile.QUALITY_CIF);
    private static final String QUALITY_480P
        = Integer.toString(CamcorderProfile.QUALITY_480P);
    private static final String QUALITY_720P
        = Integer.toString(CamcorderProfile.QUALITY_720P);
    private static final String QUALITY_1080P
        = Integer.toString(CamcorderProfile.QUALITY_1080P);
    private static final String QUALITY_QVGA
        = Integer.toString(CamcorderProfile.QUALITY_QVGA);
    private static final String QUALITY_2160P
        = Integer.toString(CamcorderProfile.QUALITY_2160P);

    private static final String VIDEO_QUALITY_VSDOF_HIGH =
            Integer.toString(VSDOF_QUALITY_HIGH);

    private static final int[] SLOW_MOTION_SUPPORT_QUALIYS = new int[] {
        CamcorderProfile.QUALITY_HIGH_SPEED_2160P,
        CamcorderProfile.QUALITY_HIGH_SPEED_1080P,
        CamcorderProfile.QUALITY_HIGH_SPEED_720P,
        CamcorderProfile.QUALITY_HIGH_SPEED_480P};

    // SLOW_MOTION_SUPPORT_QUALIY_STRING order should be match with
    // SLOW_MOTION_SUPPORT_QUALIY
    private static final String[] SLOW_MOTION_SUPPORT_QUALIYS_STRING = new String[] {
            QUALITY_HIGH_SPEED_2160P,
            QUALITY_HIGH_SPEED_1080P,
            QUALITY_HIGH_SPEED_720P,
            QUALITY_HIGH_SPEED_480P};

    private static final int[] NORMAL_SUPPORT_QUALIYS = new int[] {
        CamcorderProfile.QUALITY_2160P,
        CamcorderProfile.QUALITY_1080P,
        CamcorderProfile.QUALITY_720P,
        CamcorderProfile.QUALITY_480P,
        CamcorderProfile.QUALITY_CIF,
        CamcorderProfile.QUALITY_QVGA,
        CamcorderProfile.QUALITY_QCIF};

    private static final String[] NORMAL_SUPPORT_QUALIYS_STRING = new String[] {
        QUALITY_2160P,
        QUALITY_1080P,
        QUALITY_720P,
        QUALITY_480P,
        QUALITY_CIF,
        QUALITY_QVGA,
        QUALITY_QCIF};

    private static final CharSequence[] COLOR_EFFECT_SUPPORT_BY_3RD = new CharSequence[] {
        "none",
        "mono",
        "sepia",
        "negative",
        "solarize",
        "aqua",
        "pastel",
        "mosaic",
        "red-tint",
        "blue-tint",
        "green-tint",
        "blackboard",
        "whiteboard",
        "sepiablue",
        "sepiagreen",
    };

    private static final int NOT_FOUND = -1;
    private static final int NORMAL_RECORD_FPS = 30;
    private static final int BACK_QUALITY_NUMBER = 4;
    private static final int FRONT_QUALITY_NUMBER = 2;
    private static final int FRONT_CAMERA_ID = 1;

    private static final String DEFAULT_ON = "on";

    private int mCameraId;
    private int mPreferenceRes = 0;

    private ICameraContext mICameraContext;
    private ICameraDeviceManager mICameraDeviceManager;
    private ICameraDevice mICameraDevice;
    private IModuleCtrl mIModuleCtrl;

    private CameraInfo[] mCameraInfo;
    private List<String> mSupportedImageProperties;
    private List<String> mSupportedFaceBeautyProperties;
    private List<String> mSupportedDualCamera = new ArrayList<String>();

    private Context mContext;
    private SharedPreferencesTransfer mPrefTransfer;
    private PreferenceInflater mInflater;

    private ArrayList<SettingItem> mSettingList = new ArrayList<SettingItem>();
    private HashMap<Integer, PreferenceGroup> mPreferencesGroupMap;
    private HashMap<Integer, ArrayList<ListPreference>> mPreferencesMap;
    private HashMap<Integer, ArrayList<SettingItem>> mSettingItemsMap;

    public SettingGenerator(ICameraContext cameraContext, SharedPreferencesTransfer prefTransfer) {
        mICameraContext = cameraContext;
        mContext = cameraContext.getActivity();
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
        mIModuleCtrl = cameraContext.getModuleController();
        mPrefTransfer = prefTransfer;
        mCameraInfo = mICameraDeviceManager.getCameraInfo();
        mCameraId = mICameraDeviceManager.getCurrentCameraId();
        mICameraDevice = mICameraDeviceManager.getCameraDevice(mCameraId);

        int cameraCounts = mICameraDeviceManager.getNumberOfCameras();
        mPreferencesGroupMap = new HashMap<Integer, PreferenceGroup>(cameraCounts);
        mPreferencesMap = new HashMap<Integer, ArrayList<ListPreference>>(cameraCounts);
        mSettingItemsMap = new HashMap<Integer, ArrayList<SettingItem>>(cameraCounts);
    }

    /**
     * Create all the setting objects
     *
     * @param group
     *            the group contain setting listPreference.
     */
    public void createSettings(int preferenceRes) {
        mPreferenceRes = preferenceRes;
        mInflater = new PreferenceInflater(mContext, mPrefTransfer);
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        PreferenceGroup group = (PreferenceGroup) mInflater.inflate(preferenceRes);
        mPreferencesGroupMap.put(currentCameraId, group);
        createSettingItems();
        createPreferences(group, currentCameraId);
    }

    public void updatePreferences() {
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        mICameraDevice = mICameraDeviceManager.getCameraDevice(currentCameraId);
        mCameraId = currentCameraId;
        // get all list preference which current camera supports.
        ArrayList<ListPreference> preferences = mPreferencesMap.get(currentCameraId);
//        Log.i(TAG, "[updatePreferences], currentCameraId:" + currentCameraId);
        if (preferences == null) {
            PreferenceGroup group = (PreferenceGroup) mInflater.inflate(mPreferenceRes);
            mPreferencesGroupMap.put(currentCameraId, group);
            createPreferences(group, currentCameraId);
        } else {
            ArrayList<SettingItem> settingItems = mSettingItemsMap.get(currentCameraId);
            for (int i = 0; i < preferences.size(); i++) {
                SettingItem settingItem = settingItems.get(i);
                ListPreference preference = preferences.get(i);
                updateSettingItem(settingItem, preference);
                settingItem.clearAllOverrideRecord();
                settingItem.setLastValue(settingItem.getDefaultValue());
                if (preference != null) {
                    preference.setOverrideValue(null);
                }
            }

            SettingItem picRatioSetting =
                    getSettingItem(SettingConstants.ROW_SETTING_PICTURE_RATIO, mCameraId);
            if (picRatioSetting != null) {
                picRatioSetting.setLastValue(null);
            }

            SettingItem fbSetting =
                    getSettingItem(SettingConstants.ROW_SETTING_MULTI_FACE_MODE, mCameraId);
            if (fbSetting != null) {
                fbSetting.setLastValue(null);
            }

            SettingItem zsdSetting =
                    getSettingItem(SettingConstants.ROW_SETTING_ZSD, mCameraId);
            if (zsdSetting != null) {
                zsdSetting.setLastValue(null);
            }

            SettingItem multiAFSetting =
                    getSettingItem(SettingConstants.ROW_SETTING_AF, mCameraId);
            if (multiAFSetting != null) {
                multiAFSetting.setLastValue(null);
            }

            SettingItem captureModeSetting =
                    getSettingItem(SettingConstants.ROW_SETTING_CAPTURE_MODE, mCameraId);
            if (captureModeSetting != null) {
                captureModeSetting.setLastValue(null);
            }

            SettingItem recordingHintSetting =
                    getSettingItem(SettingConstants.ROW_SETTING_RECORDING_HINT, mCameraId);
            if (recordingHintSetting != null) {
                recordingHintSetting.setLastValue(null);
            }

            SettingItem antibandingSetting =
                    getSettingItem(SettingConstants.ROW_SETTING_ANTI_FLICKER, mCameraId);
            if (antibandingSetting != null) {
                antibandingSetting.setLastValue(null);
            }

            overrideSettingByIntent();
        }
    }

    public SettingItem getSettingItem(String key) {
        int settingId = SettingConstants.getSettingId(key);
        return getSettingItem(settingId);
    }

    public SettingItem getSettingItem(int settingId) {
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        return getSettingItem(settingId, currentCameraId);
    }

    public SettingItem getSettingItem(int settingId, int cameraId) {
        ArrayList<SettingItem> settingItems = mSettingItemsMap.get(cameraId);
        if (settingItems == null) {
            return null;
        }
        return settingItems.get(settingId);
    }

    public PreferenceGroup getPreferenceGroup() {
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        return mPreferencesGroupMap.get(currentCameraId);
    }

    public ListPreference getListPreference(int row) {
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        ArrayList<ListPreference> preferences = mPreferencesMap.get(currentCameraId);
        if (preferences == null) {
            Log.e(TAG, "Call setting before setting updated, return null");
            return null;
        }
        return preferences.get(row);
    }

    public ListPreference getListPreference(String key) {
        int settingId = SettingConstants.getSettingId(key);
        return getListPreference(settingId);
    }

    public void restoreSetting(int cameraId) {
        ArrayList<SettingItem> settingItems = mSettingItemsMap.get(cameraId);
        if (settingItems != null) {
            for (int i = 0; i < settingItems.size(); i++) {
                SettingItem settingItem = settingItems.get(i);
                settingItem.setValue(settingItem.getDefaultValue());
            }
        }

        ArrayList<ListPreference> preferences = mPreferencesMap.get(cameraId);
        if (preferences != null) {
            for (int i = 0; i < preferences.size(); i++) {
                ListPreference pref = preferences.get(i);
                if (pref != null) {
                    pref.setOverrideValue(null, false);
                }
            }
        }

        SettingItem videoQualityItem = getSettingItem(SettingConstants.KEY_VIDEO_QUALITY);
        ListPreference videoPref = videoQualityItem.getListPreference();
        if (videoPref != null) {
            List<String> videoQualitys = getMTKSupportedVideoQuality();
            if (videoQualitys != null && videoQualitys.size() > 0) {
                videoPref.filterUnsupported(videoQualitys);
                videoQualityItem.setDefaultValue(videoQualitys.get(0));
                videoQualityItem.setLastValue(null);
                videoPref.setValue(videoQualitys.get(0));
            }
        }

        // need update picture ratio setting as full ratio.
        SharedPreferences sharePreferences = mPrefTransfer
                .getSharedPreferences(SettingConstants.KEY_PICTURE_RATIO);
        SettingItem pictureRatioSetting = getSettingItem(SettingConstants.KEY_PICTURE_RATIO);
        ListPreference pictureRatioPref = pictureRatioSetting.getListPreference();
        if (pictureRatioPref != null) {
            String ratio = sharePreferences.getString(SettingConstants.KEY_PICTURE_RATIO,
                    String.valueOf(4d / 3));
            pictureRatioPref.setValue(ratio);
            pictureRatioSetting.setValue(ratio);
            pictureRatioSetting.setDefaultValue(ratio);
            pictureRatioSetting.setLastValue(null);
        }

        List<String> supportedFBMode = getSupportedFaceBeautyMode();
        if (supportedFBMode != null && supportedFBMode.size() > 0) {
            String defaultFBMode = supportedFBMode.get(0);
            SettingItem fbModeSetting = getSettingItem(SettingConstants.KEY_MULTI_FACE_BEAUTY);
            fbModeSetting.setValue(defaultFBMode);
            fbModeSetting.setDefaultValue(defaultFBMode);
            ListPreference pref = fbModeSetting.getListPreference();
            if (pref != null) {
                pref.setValue(defaultFBMode);
            }
        }

        SettingItem zsdSetting = getSettingItem(SettingConstants.KEY_CAMERA_ZSD);
        ListPreference zsdPref = zsdSetting.getListPreference();
        if (zsdSetting.isEnable()) {
            zsdPref.setValue(zsdSetting.getDefaultValue());
        }

        SettingItem antiBandingSetting = getSettingItem(SettingConstants.KEY_ANTI_BANDING);
        ListPreference antiBandingPref = antiBandingSetting.getListPreference();
        if (antiBandingSetting.isEnable()) {
            antiBandingPref.setValue(antiBandingSetting.getDefaultValue());
        }

        SettingItem captureModeSetting = getSettingItem(SettingConstants.KEY_CAPTURE_MODE);
        if (captureModeSetting != null) {
            captureModeSetting.setLastValue(null);
        }

        overrideSettingByIntent();
    }

    private void createSettingItems() {
        int cameraCounts = mICameraDeviceManager.getNumberOfCameras();
        for (int i = 0; i < cameraCounts; i++) {
            ArrayList<SettingItem> settingItems = new ArrayList<SettingItem>();
            for (int settingId = 0; settingId < SettingConstants.SETTING_COUNT; settingId++) {
                SettingItem settingItem = new SettingItem(settingId);
                String key = SettingConstants.getSettingKey(settingId);
                int type = SettingConstants.getSettingType(settingId);
                settingItem.setKey(key);
                settingItem.setType(type);
                settingItems.add(settingItem);
            }
            mSettingItemsMap.put(i, settingItems);
        }

    }

    private void createPreferences(PreferenceGroup group, int cameraId) {
//        Log.d(TAG, "[createPreferences], cameraId:" + cameraId + ", group:" + group);
        ArrayList<ListPreference> preferences = mPreferencesMap.get(cameraId);
        mSupportedImageProperties = new ArrayList<String>();
        mSupportedFaceBeautyProperties = new ArrayList<String>();
        if (preferences == null) {
            preferences = new ArrayList<ListPreference>();
            ArrayList<SettingItem> settingItems = mSettingItemsMap.get(cameraId);
            for (int settingId = 0; settingId < SettingConstants.SETTING_COUNT; settingId++) {
                String key = SettingConstants.getSettingKey(settingId);
                ListPreference preference = group.findPreference(key);

                preferences.add(preference);

                SettingItem settingItem = settingItems.get(settingId);
                settingItem.setListPreference(preference);
            }
            mPreferencesMap.put(cameraId, preferences);
        }
        // every camera maintain one setting item list.
        filterPreferences(preferences, cameraId);
    }

    private void filterPreferences(ArrayList<ListPreference> preferences, int cameraId) {
        ArrayList<SettingItem> settingItems = mSettingItemsMap.get(cameraId);
        limitPreferencesByIntent();
        for (int i = 0; i < preferences.size(); i++) {
            // filter list preference.
            ListPreference preference = preferences.get(i);
            boolean isRemove = filterPreference(preference);
            if (isRemove) {
                preference = null;
                preferences.set(i, null);
            }
            // update setting's value and default value.
            SettingItem settingItem = settingItems.get(i);
            updateSettingItem(settingItem, preference);
        }

        overrideSettingByIntent();
    }

    /**
     * Update setting's value and default value.
     *
     * @param settingItem
     *            setting instance{@link SettingItem}.
     * @param preference
     *            setting's preference {@link ListPreference}.
     */
    private void updateSettingItem(SettingItem settingItem, ListPreference preference) {
        int settingId = settingItem.getSettingId();
        int type = SettingConstants.getSettingType(settingId);
        String defaultValue = settingItem.getDefaultValue();
        switch (type) {
        case SettingConstants.NEITHER_IN_PARAMETER_NOR_IN_PREFERENCE:
        case SettingConstants.ONLY_IN_PARAMETER:
            // set setting default value and value, the value is initialized to
            // default value.
            defaultValue = SettingDataBase.getDefaultValue(settingId);
            if (!mIModuleCtrl.isNonePickIntent()) {
                if (SettingConstants.ROW_SETTING_CAMERA_MODE == settingId) {
                    defaultValue = Integer.toString(Parameters.CAMERA_MODE_NORMAL);
                }
            }
            settingItem.setDefaultValue(defaultValue);
            settingItem.setValue(defaultValue);
            break;

        case SettingConstants.BOTH_IN_PARAMETER_AND_PREFERENCE:
        case SettingConstants.ONLY_IN_PEFERENCE:
            Parameters parameters = mICameraDevice.getParameters();
            // if setting has preferences, its default value and value get from
            // preference.
            if (preference != null) {
                preference.reloadValue();
                if (defaultValue == null) {
                    defaultValue = generateDefaultValue(settingItem.getKey(),
                            mICameraDevice.getParameters(), preference);
                }
                settingItem.setDefaultValue(defaultValue);
                settingItem.setValue(preference.getValue());
            } else {
                if(settingItem.getKey().equals(SettingConstants.KEY_PICTURE_RATIO)){
                    settingItem.setEnable(true);
                }else{
                    settingItem.setEnable(false);
                }
            }
            break;
        default:
            break;
        }
    }

    /**
     * Generate default value of setting item from parameters or list preference.
     * @param key The key used to indicators setting item.
     * @param parameters The hardware parameters
     * @param pref The list preference from XML.
     * @return Return the default value.
     */
    private String generateDefaultValue(String key, Parameters parameters
            , ListPreference pref) {
        String defaultValue = null;
        if (SettingConstants.KEY_PICTURE_RATIO.equals(key)) {
            List<String> supportedRatios = SettingUtils.buildPreviewRatios(mContext, parameters);
            if (supportedRatios != null && supportedRatios.size() > 0) {
                return supportedRatios.get(supportedRatios.size() - 1);
            }
        }
        if (SettingConstants.KEY_PICTURE_SIZE.equals(key)) {
            SettingItem pictureRatioItem = getSettingItem(SettingConstants.KEY_PICTURE_RATIO);
            List<String> pictureSizes = SettingUtils
                    .buildSupportedPictureSizeByRatio(parameters, pictureRatioItem.getValue());
            if (pictureSizes != null && pictureSizes.size() > 0) {
                return pictureSizes.get(pictureSizes.size() - 1);
            }
        }

        if (SettingConstants.KEY_VIDEO_QUALITY.equals(key)) {
            List<String> videoQualitys = getMTKSupportedVideoQuality();
            if (videoQualitys != null && videoQualitys.size() > 0) {
                return videoQualitys.get(0);
            }
        }

        if (SettingConstants.KEY_MULTI_FACE_BEAUTY.equals(key)) {
            List<String> supportedFBMode = getSupportedFaceBeautyMode();
            if (supportedFBMode != null && supportedFBMode.size() > 0) {
                return supportedFBMode.get(0);
            }
        }
        if (SettingConstants.KEY_CAMERA_ZSD.equals(key)) {
            return SettingUtils.getZsdDefaultValue();
        }
        if (SettingConstants.KEY_ANTI_BANDING.equals(key)) {
            return SettingUtils.getAntiBandingDefaultValue();
        }

        defaultValue = pref.getDefaultValue();
        return defaultValue;
    }

    private boolean filterPreference(final ListPreference preference) {
        Parameters parameters = mICameraDevice.getParameters();
        String key = null;
        int settingId = -1;
        boolean removePreference = false;
        if (preference != null) {
            key = preference.getKey();
            settingId = SettingConstants.getSettingId(key);
        }

        switch (settingId) {

        case SettingConstants.ROW_SETTING_FLASH:
        case SettingConstants.ROW_SETTING_ISO:
        case SettingConstants.ROW_SETTING_AIS:
        case SettingConstants.ROW_SETTING_3DNR:
            removePreference = filterUnsupportedOptions(preference,
                    ParametersHelper.getParametersSupportedValues(parameters, key), settingId);
            break;

        case SettingConstants.ROW_SETTING_ANTI_FLICKER:
            removePreference = filterUnsupportedOptions(preference,
                    ParametersHelper.getParametersSupportedValues(parameters, key), settingId);
            if (!removePreference) {
                preference.setValue(parameters.getAntibanding());
            }
            break;

        case SettingConstants.ROW_SETTING_COLOR_EFFECT:
            boolean isNonePickIntent = mIModuleCtrl.isNonePickIntent();
            if (!isNonePickIntent) {
                preference.setOriginalEntryValues(COLOR_EFFECT_SUPPORT_BY_3RD);
            }
            removePreference = filterUnsupportedOptions(preference,
                    ParametersHelper.getParametersSupportedValues(parameters, key), settingId);
            break;

        case SettingConstants.ROW_SETTING_DUAL_CAMERA:
            removePreference = buildCameraId(preference, settingId);
            break;

        case SettingConstants.ROW_SETTING_EXPOSURE:
            removePreference = buildExposureCompensation(preference, settingId);
            break;

        case SettingConstants.ROW_SETTING_SCENCE_MODE:
        case SettingConstants.ROW_SETTING_WHITE_BALANCE:
            updateSettingItem(SettingConstants.getSettingKey(settingId), preference);
            removePreference = filterUnsupportedOptions(preference,
                    ParametersHelper.getParametersSupportedValues(parameters, key), settingId);
            break;

        case SettingConstants.ROW_SETTING_SHARPNESS:
        case SettingConstants.ROW_SETTING_HUE:
        case SettingConstants.ROW_SETTING_SATURATION:
        case SettingConstants.ROW_SETTING_BRIGHTNESS:
        case SettingConstants.ROW_SETTING_CONTRAST:
            removePreference = filterUnsupportedOptions(preference,
                    ParametersHelper.getParametersSupportedValues(parameters, key), settingId);
            if (!removePreference) {
                buildSupportedListperference(mSupportedImageProperties, preference);
            }
            break;

        case SettingConstants.ROW_SETTING_IMAGE_PROPERTIES:
            removePreference = filterUnsupportedEntries(preference, mSupportedImageProperties,
                    true, settingId);
            break;

        case SettingConstants.ROW_SETTING_ZSD:
            if (mICameraContext.getFeatureConfig().isLowRamOptSupport()) {
                removePreference = true;
            } else {
                removePreference = filterUnsupportedOptions(preference,
                        ParametersHelper.getParametersSupportedValues(parameters, key), settingId);
                if (!removePreference) {
                    preference.setValue(parameters.getZSDMode());
                }
            }
            break;

        case SettingConstants.ROW_SETTING_FACEBEAUTY_PROPERTIES:
            if (mICameraContext.getFeatureConfig().isVfbEnable()
                    || (!mICameraContext.getFeatureConfig().isVfbEnable()
                    && !ParametersHelper.isCfbSupported(parameters, mICameraContext))) {
                removePreference = true;
            } else {
                removePreference = filterUnsupportedEntries(preference,
                        mSupportedFaceBeautyProperties, true, settingId);
            }

            break;

        case SettingConstants.ROW_SETTING_FACEBEAUTY_SMOOTH:
            removePreference = buildFaceBeautyPreference(ParametersHelper.KEY_FACEBEAUTY_SMOOTH,
                    preference, settingId);
            if (!removePreference) {
                buildSupportedListperference(mSupportedFaceBeautyProperties, preference);
            }
            break;

        case SettingConstants.ROW_SETTING_FACEBEAUTY_SKIN_COLOR:
            removePreference = buildFaceBeautyPreference(
                    ParametersHelper.KEY_FACEBEAUTY_SKIN_COLOR, preference, settingId);
            if (!removePreference) {
                buildSupportedListperference(mSupportedFaceBeautyProperties, preference);
            }
            break;
        case SettingConstants.ROW_SETTING_FACEBEAUTY_SHARP:
            removePreference = buildFaceBeautyPreference(ParametersHelper.KEY_FACEBEAUTY_SHARP,
                    preference, settingId);
            if (!removePreference) {
                buildSupportedListperference(mSupportedFaceBeautyProperties, preference);
            }
            break;
        case SettingConstants.ROW_SETTING_CAMERA_FACE_DETECT:
            break;
        case SettingConstants.ROW_SETTING_RECORD_LOCATION:
            removePreference = !FeatureSwitcher.isGpsLocationSupported();
            break;
        case SettingConstants.ROW_SETTING_MICROPHONE:
            break;
        case SettingConstants.ROW_SETTING_AUDIO_MODE:
            removePreference = true;
            break;
        case SettingConstants.ROW_SETTING_SELF_TIMER:
            break;

        // TODO need break ?
        case SettingConstants.ROW_SETTING_VIDEO_QUALITY:// video
            removePreference = filterUnsupportedOptions(preference,
                    getMTKSupportedVideoQuality(),
                    settingId);
            break;

        case SettingConstants.ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY:
            removePreference = filterUnsupportedOptions(preference,
                    getMTKSupportedSlowMotionVideoQuality(), settingId);
            break;

        case SettingConstants.ROW_SETTING_CONTINUOUS_NUM:
            if (mICameraContext.getFeatureConfig().isLowRamOptSupport() ||
                    ParametersHelper.getParametersSupportedValues(parameters,
                            SettingConstants.KEY_CAPTURE_MODE)
                            .indexOf(Parameters.CAPTURE_MODE_CONTINUOUS_SHOT) < 0) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_SLOW_MOTION:
            if (!mICameraContext.getFeatureConfig().isSlowMotionSupport()
                    || getMTKSupportedSlowMotionVideoQuality().size() < 1
                    || getMaxPreviewFrameRate() <= NORMAL_RECORD_FPS) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_PICTURE_RATIO:
            List<String> supportedRatios = SettingUtils.buildPreviewRatios(mContext, parameters);
            removePreference = filterUnsupportedOptions(preference, supportedRatios, settingId);
            break;

        case SettingConstants.ROW_SETTING_PICTURE_SIZE:// camera special case
            // M: filter supported values.
            List<String> supportedPictureSizes = sizeListToStringList(parameters
                    .getSupportedPictureSizes());
            //Add for stereo picture size
            if (parameters.get("refocus-picture-size-values") != null) {
                List<Point> supportedRefocusPictureSize = SettingUtils
                        .splitSize(parameters.get("refocus-picture-size-values"));
                if (supportedRefocusPictureSize != null) {
                    for (Point size : supportedRefocusPictureSize) {
                        supportedPictureSizes.add(SettingUtils.pointToStr(size));
                    }
                }
            }
            //may be third party will limit the picture sizes.
            int limitedResolution = SettingUtils.getLimitResolution();
            if (limitedResolution > 0) {
                SettingUtils.filterLimitResolution(supportedPictureSizes);
            }

            removePreference = buildPictureSizeEntries(preference, supportedPictureSizes);
            // M: for picture size was ordered, here we don't set it to index 0.
            ListPreference pictureRatioPref = getListPreference(
                    SettingConstants.ROW_SETTING_PICTURE_RATIO);
            String pictureRatio = SettingUtils.getRatioString(4d / 3);
            if (pictureRatioPref != null) {
                pictureRatio = pictureRatioPref.getValue();
            }
            List<String> supportedForRatio = SettingUtils
                    .buildSupportedPictureSizeByRatio(parameters, pictureRatio);
            removePreference = filterDisabledOptions(preference, supportedForRatio, false,
                    settingId);
            break;

        case SettingConstants.ROW_SETTING_VOICE:
            removePreference = true;
            break;

        case SettingConstants.ROW_SETTING_HDR:
            if (ParametersHelper.getParametersSupportedValues(parameters,
                    SettingConstants.KEY_SCENE_MODE).indexOf(Parameters.SCENE_MODE_HDR) <= 0) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_MULTI_FACE_MODE:
            // add for vFB
            removePreference = filterUnsupportedOptions(preference, getSupportedFaceBeautyMode(),
                    true, settingId);
            break;

        case SettingConstants.ROW_SETTING_STEREO_MODE:
            // if (!FeatureSwitcher.isStereo3dEnable()) {
            removePreference = true;
            // }
            break;

        case SettingConstants.ROW_SETTING_ASD:
            if (ParametersHelper.getParametersSupportedValues(parameters,
                    SettingConstants.KEY_CAPTURE_MODE).indexOf(Parameters.CAPTURE_MODE_ASD) <= 0) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_FAST_AF:
            if (ParametersHelper.isDepthAfSupported(parameters)
                    && mIModuleCtrl.isNonePickIntent()) {
                resetIfInvalid(preference, true);
                mSupportedDualCamera.add(key);
            } else {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_DISTANCE:
            if (ParametersHelper.isDistanceInfoSuppported(parameters)
                    && mIModuleCtrl.isNonePickIntent()) {
                resetIfInvalid(preference, true);
                mSupportedDualCamera.add(key);
            } else {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_VIDEO_STABLE:
            if (!"true".equals(ParametersHelper.getParametersValue(
                    parameters, SettingConstants.KEY_VIDEO_EIS))) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_DUAL_CAMERA_MODE:
            if ((!ParametersHelper.isDepthAfSupported(parameters)
                    && !ParametersHelper.isDistanceInfoSuppported(parameters))
                    || !mIModuleCtrl.isNonePickIntent()) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_HEARTBEAT_MONITOR:
            if (!ParametersHelper.isHeartbeatMonitorSupported(parameters)) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_DNG:
            if (!ParametersHelper.isDngSupported(parameters)) {
                removePreference = true;
            }
            break;

        case SettingConstants.ROW_SETTING_AF:
            if (!ParametersHelper.isMultiZoneAFSupported(parameters)) {
                removePreference = true;
            }
            break;
        case SettingConstants.ROW_SETTING_REFOCUS_VIDEO_QUALITY:
            removePreference = filterUnsupportedOptions(preference,
                    getRefocusSupportedVideoQuality(), settingId);
            break;

        default:
            break;
        }
        if (removePreference) {
            Log.d(TAG, "[filterPreference], don't support setting key:" + key);
        }
        return removePreference;
    }

    private void limitPreferencesByIntent() {
        boolean isNonePickIntent = mIModuleCtrl.isNonePickIntent();
        if (!isNonePickIntent) {
            int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
            ArrayList<ListPreference> preferences = mPreferencesMap.get(currentCameraId);
            int[] unSupportedBy3rdParty = SettingConstants.UN_SUPPORT_BY_3RDPARTY;
            for (int i = 0; i < unSupportedBy3rdParty.length; i++) {
                preferences.set(unSupportedBy3rdParty[i], null);
            }
        }

        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
//        Log.d(TAG, "currentCameraId:" + currentCameraId + ", frontCameraId:" +
//                "" + mICameraDeviceManager.getFrontCameraId());
        if (currentCameraId == mICameraDeviceManager.getFrontCameraId()) {
            ArrayList<ListPreference> preferences = mPreferencesMap.get(currentCameraId);
            int[] unSupportedByFrontCamera = SettingConstants.UN_SUPPORT_BY_FRONT_CAMERA;
            for (int i = 0; i < unSupportedByFrontCamera.length; i++) {
                preferences.set(unSupportedByFrontCamera[i], null);
            }
        }
    }

    private void overrideSettingByIntent() {
        if (!mIModuleCtrl.isNonePickIntent()) {
            int[] supportedBy3rdButHidden = SettingConstants.SUPPORT_BY_3RDPARTY_BUT_HIDDEN;
            for (int i = 0; i < supportedBy3rdButHidden.length; i++) {
                int settingIndex = supportedBy3rdButHidden[i];
                SettingItem item = getSettingItem(settingIndex);
                ListPreference pref = item.getListPreference();
                if (pref != null) {
                    pref.setVisibled(false);
                }
                //Override its value as default value except GPS and video quality.
                if (SettingConstants.ROW_SETTING_RECORD_LOCATION != settingIndex
                        && SettingConstants.ROW_SETTING_VIDEO_QUALITY != settingIndex) {
                    item.setValue(SettingDataBase.getDefaultValue(settingIndex));
                }
            }
        }
    }

    private void updateSettingItem(String key, ListPreference item) {
        if (item == null) {
            return;
        }
        CharSequence[] entries = item.getEntries();
        CharSequence[] entryValues = item.getEntryValues();
        int length = entries.length;
        ArrayList<CharSequence> newEntries = new ArrayList<CharSequence>();
        ArrayList<CharSequence> newEntryValues = new ArrayList<CharSequence>();
        for (int i = 0; i < length; i++) {
            newEntries.add(entries[i]);
            newEntryValues.add(entryValues[i]);
        }
        length = newEntryValues.size();

        item.setOriginalEntryValues(newEntryValues.toArray(new CharSequence[length]));
        item.setOriginalEntries(newEntries.toArray(new CharSequence[length]));
    }

    private boolean filterUnsupportedOptions(ListPreference pref,
            List<String> supported, int row) {
        return filterUnsupportedOptions(pref, supported, true, row);
    }

    private boolean filterUnsupportedOptions(ListPreference pref, List<String> supported,
            boolean resetFirst, int row) {
        if (supported != null) {
            pref.filterUnsupported(supported);
        }

        if (pref.getEntryValues().length == 1) {
            SettingItem settingItem = getSettingItem(row);
            CharSequence[] values = pref.getEntryValues();
            settingItem.setDefaultValue(values[0].toString());
            settingItem.setValue(values[0].toString());
        }

        // Remove the preference if the parameter is not supported or there is
        // only one options for the settings.
        if (supported == null || supported.size() <= 1) {
            return true;
        }

        if (pref.getEntries().length <= 1) {
            return true;
        }
        resetIfInvalid(pref, resetFirst);
        return false;
    }

    // add for filter unsupported image properties
    // image properties just can be filtered by entries
    private boolean filterUnsupportedEntries(ListPreference pref, List<String> supported,
            boolean resetFirst, int row) {
        if (supported == null || supported.size() <= 0) {
            return true;
        }
        pref.filterUnsupportedEntries(supported);
        if (pref.getEntries().length <= 0) {
            return true;
        }
        resetIfInvalid(pref, resetFirst);
        return false;
    }

    private void buildSupportedListperference(List<String> supportedList, ListPreference list) {
        if (list != null && supportedList != null) {
            supportedList.add(list.getKey());
        }
    }

    private boolean filterDisabledOptions(ListPreference pref, List<String> supported,
            boolean resetFirst, int row) {

        // Remove the preference if the parameter is not supported or there is
        // only one options for the settings.
        if (supported == null || supported.size() < 1) {
            return true;
        }

        pref.filterDisabled(supported);
        if (pref.getEntries().length < 1) {
            return true;
        }

        resetIfInvalid(pref, resetFirst);
        return false;
    }

    private void resetIfInvalid(ListPreference pref) {
        resetIfInvalid(pref, true);
    }

    private void resetIfInvalid(ListPreference pref, boolean first) {
        // Set the value to the first entry if it is invalid.
        String value = pref.getValue();
        if (pref.findIndexOfValue(value) == NOT_FOUND) {
            if (first) {
                pref.setValueIndex(0);
            } else if (pref.getEntryValues() != null && pref.getEntryValues().length > 0) {
                pref.setValueIndex(pref.getEntryValues().length - 1);
            }
        }
    }

    private static List<String> sizeListToStringList(List<Size> sizes) {
        ArrayList<String> list = new ArrayList<String>();
        for (Size size : sizes) {
            list.add(String.format(Locale.ENGLISH, "%dx%d", size.width, size.height));
        }
        return list;
    }

    private boolean buildCameraId(ListPreference preference, int row) {
        int numOfCameras = mCameraInfo.length;
        if (numOfCameras < 2) {
            return true;
        }

        CharSequence[] entryValues = new CharSequence[2];
        for (int i = 0; i < mCameraInfo.length; ++i) {
            int index = (mCameraInfo[i].facing == CameraInfo.CAMERA_FACING_FRONT)
                    ? CameraInfo.CAMERA_FACING_FRONT : CameraInfo.CAMERA_FACING_BACK;
            if (entryValues[index] == null) {
                entryValues[index] = "" + i;
                if (entryValues[((index == 1) ? 0 : 1)] != null) {
                    break;
                }
            }
        }
        preference.setEntryValues(entryValues);
        return false;
    }

    // whether the profile video width/height is in video sizes, if not, record will error
    private boolean isCamcorderProfileInVideoSizes(int cameraId, int camcorderProfileId) {
        if(!FeatureSwitcher.isTablet()) {   // not tablet
            return true;
        }

        boolean exist = false;
        Parameters parameters = mICameraDevice.getParameters();
        List<Size> supportedVideoSizes = parameters.getSupportedVideoSizes();

        CamcorderProfile profile = CamcorderProfile.get(cameraId, camcorderProfileId);
        int videoWidth = profile.videoFrameWidth;
        int videoHeight = profile.videoFrameHeight;

        for(Iterator it = supportedVideoSizes.iterator(); it.hasNext();){
            Size size = (Size)it.next();
                if(size.width == videoWidth && size.height == videoHeight){
//                    Log.d(TAG, "camcorder profile:" + camcorderProfileId +
//                        " exist: " + "width=" + videoWidth + ",height=" + videoHeight);
                    exist = true;
                    break;
                }
        }

        if(!exist) {
            Log.w(TAG, "CamcorderProfileId " + camcorderProfileId
                     + ", width:" + videoWidth + ", height:" + videoHeight
                     + " Not Exist In Feature Table!");
        }

        return exist;
    }

    private ArrayList<String> getMTKSupportedVideoQuality() {
        ArrayList<String> supported = new ArrayList<String>();
        // Check for supported quality, pip mode always check main camera's
        // quality
        int cameraId = mCameraId;
        int qualitys = NORMAL_SUPPORT_QUALIYS_STRING.length;
        int qualitySize = BACK_QUALITY_NUMBER;
        if (mCameraId == FRONT_CAMERA_ID) {
            qualitySize = FRONT_QUALITY_NUMBER;
        }
        for (int i = 0, supportSize = 0; i < qualitys && supportSize < qualitySize; i++) {
            if (CamcorderProfile.hasProfile(mCameraId, NORMAL_SUPPORT_QUALIYS[i])) {
                if (i == 0) {
                    List<Size> sizes = mICameraDevice.getParameters().getSupportedVideoSizes();
                    Iterator<Size> it = sizes.iterator();
                    boolean support = false;
                    while (it.hasNext()) {
                        Size size = it.next();
                        if (size.width >= VIDEO_2K42_WIDTH) {
                            support = true;
                            break;
                        }
                    }
                    if (support) {
                        supportSize ++;
                        supported.add(NORMAL_SUPPORT_QUALIYS_STRING[i]);
                    }
                } else {
                    supportSize ++;
                    Log.i(TAG, "supportSize = " + supportSize);
                    Log.i(TAG, "NORMAL_SUPPORT_QUALIYS_STRING[i] = "
                          + NORMAL_SUPPORT_QUALIYS_STRING[i]);
                    supported.add(NORMAL_SUPPORT_QUALIYS_STRING[i]);
                }
            }
        }
        return supported;
    }

    private ArrayList<String> getRefocusSupportedVideoQuality() {
        ArrayList<String> supported = new ArrayList<String>();
        int cameraId = mCameraId;
        if (CamcorderProfile.hasProfile(cameraId,
                VSDOF_QUALITY_HIGH)) {
            supported.add(VIDEO_QUALITY_VSDOF_HIGH);
        }
        return supported;
    }

    // should be refactored for icons
    private boolean buildExposureCompensation(ListPreference exposure, int row) {
        Parameters parameters = mICameraDevice.getParameters();
        int max = parameters.getMaxExposureCompensation();
        int min = parameters.getMinExposureCompensation();
        if (max == 0 && min == 0) {
            return true;
        }
        float step = parameters.getExposureCompensationStep();

        // show only integer values for exposure compensation
        int maxValue = (int) FloatMath.floor(max * step);
        int minValue = (int) FloatMath.ceil(min * step);
        ArrayList<String> entryValuesList = new ArrayList<String>();
        for (int i = minValue; i <= maxValue; ++i) {
            String value = Integer.toString(Math.round(i / step));
            entryValuesList.add(String.valueOf(value));
        }
        exposure.filterUnsupported(entryValuesList);
        return false;
    }

    // should be refactored for icons
    private boolean buildFaceBeautyPreference(String key, ListPreference fbPreference, int row) {
        Parameters parameters = mICameraDevice.getParameters();
        int max = ParametersHelper.getMaxLevel(parameters, key);
        int min = ParametersHelper.getMinLevel(parameters, key);
        if (max == 0 && min == 0) {
            // removePreference(mPreferenceGroup, fbPreference.getKey(), row);
            return true;
        }

        CharSequence[] faceBeautyValue = new CharSequence[] {
                String.valueOf(min),
                String.valueOf(0),
                String.valueOf(max),
        };
        fbPreference.setEntryValues(faceBeautyValue);
        return false;
    }

    private ArrayList<String> getMTKSupportedSlowMotionVideoQuality() {
        ArrayList<String> supported = new ArrayList<String>();
        if (!mIModuleCtrl.isNonePickIntent()
                || !mICameraContext.getFeatureConfig().isSlowMotionSupport()) {
            return supported;
        }
        List<SlowMotionParam> slowMotionParam = getSupportedPreviewSizesAndFps();
        int qualitys = SLOW_MOTION_SUPPORT_QUALIYS.length;
        for (int i = 0; i < qualitys; i++) {
            if (CamcorderProfile.hasProfile(mCameraId, SLOW_MOTION_SUPPORT_QUALIYS[i])
                    && isParametersSupport(SLOW_MOTION_SUPPORT_QUALIYS[i], slowMotionParam)) {
                supported.add(SLOW_MOTION_SUPPORT_QUALIYS_STRING[i]);
            }
        }
        if (supported.size() == 1) {
            SharedPreferences pref = mPrefTransfer
                    .getSharedPreferences(SettingConstants.KEY_SLOW_MOTION_VIDEO_QUALITY);
            SharedPreferences.Editor editor = pref.edit();
            editor.putString(SettingConstants.KEY_SLOW_MOTION_VIDEO_QUALITY, supported.get(0));
            editor.apply();

            int settingId = SettingConstants
                    .getSettingId(SettingConstants.KEY_SLOW_MOTION_VIDEO_QUALITY);
            SettingItem settingItem = getSettingItem(settingId);
            settingItem.setValue(supported.get(0));
        }

        Log.d(TAG, "supported slowMotion quality = " + supported);
        return supported;
    }

    private Integer getMaxPreviewFrameRate() {
        Parameters parameters = mICameraDevice.getParameters();
        List<Integer> frameRates = null;
        frameRates = parameters.getSupportedPreviewFrameRates();
        Integer max = Collections.max(frameRates);
        return max;
    }

    private List<SlowMotionParam> getSupportedPreviewSizesAndFps() {
        Parameters parameters = mICameraDevice.getParameters();
        String str = parameters.get(ParametersHelper.KEY_HSVR_SIZE_FPS);
        return splitSize(str);
    }

    // Splits a comma delimited string to an ArrayList of Size.
    // Return null if the passing string is null or the size is 0.
    private ArrayList<SlowMotionParam> splitSize(String str) {
        if (str == null)
            return null;
        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<SlowMotionParam> sizeList = new ArrayList<SlowMotionParam>();
        for (String s : splitter) {
            SlowMotionParam size = strToSize(s);
            if (size != null)
                sizeList.add(size);
        }
        if (sizeList.size() == 0)
            return null;
        return sizeList;
    }

    private SlowMotionParam strToSize(String str) {
        if (str == null)
            return null;
        int pos1 = str.indexOf('x');
        int pos2 = str.lastIndexOf('x');
        if (pos1 != -1 && pos2 != -1) {
            String width = str.substring(0, pos1);
            String height = str.substring(pos1 + 1, pos2);
            String fps = str.substring(pos2 + 1);
            return new SlowMotionParam(Integer.parseInt(width), Integer.parseInt(height),
                    Integer.parseInt(fps));
        }
        Log.e(TAG, "Invalid size parameter string=" + str);
        return null;
    }

    public boolean isParametersSupport(int quality, List<SlowMotionParam> slowMotionParam) {
        int currentCamera = mICameraDeviceManager.getCurrentCameraId();
        CamcorderProfile profile = CamcorderProfile.get(currentCamera,
                quality);
        if (slowMotionParam == null) {
            Log.v(TAG, "slowMotionParam = " + slowMotionParam);
            return false;
        }
        Iterator<SlowMotionParam> it = slowMotionParam.iterator();
        boolean support = false;
        if (profile != null) {
            while (it.hasNext()) {
                SlowMotionParam size = it.next();
                if (size.width == profile.videoFrameWidth
                        && size.height == profile.videoFrameHeight
                        && size.fps == profile.videoFrameRate) {
                    support = true;
                    break;
                }
            }
        }
//        Log.d(TAG, "isParametersSupport profile " + profile + ": support = " + support);
        return support;
    }

    private class SlowMotionParam {
        public SlowMotionParam(int w, int h, int f) {
            width = w;
            height = h;
            fps = f;
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof SlowMotionParam)) {
                return false;
            }
            SlowMotionParam s = (SlowMotionParam) obj;
            return width == s.width && height == s.height && fps == s.fps;
        }

        @Override
        public int hashCode() {
            return width * 32713 + height;
        }

        private int width;
        private int height;
        private int fps;
    };


    private ArrayList<String> getSupportedFaceBeautyMode() {
        ArrayList<String> supported = new ArrayList<String>();
        if (isCFBSupported() && mICameraContext.getFeatureConfig().isVfbEnable()) {
            if (!isOnlyMultiFaceBeautySupported()) {
                supported.add(mContext.getResources().getString(R.string.face_beauty_single_mode));
            }
            supported.add(mContext.getResources().getString(R.string.face_beauty_multi_mode));
            supported.add(mContext.getResources().getString(R.string.pref_face_beauty_mode_off));
        }
//        Log.d(TAG, "getSupportedFaceBeautyMode : " + supported);
        return supported;
    }

    public boolean isOnlyMultiFaceBeautySupported() {
        // fb-extreme-beauty-supported is false means just supported Multi face
        // mode,
        // so need remove the single face mode in settings
        boolean isOnlySupported = "false".equals(mICameraDevice.getParameters().get(
                SettingConstants.KEY_FB_EXTEME_BEAUTY_SUPPORTED));
//        Log.d(TAG, "isOnlyMultiFaceBeautySupported = " + isOnlySupported);
        return isOnlySupported;
    }

    private boolean isCFBSupported() {
        boolean support = mICameraContext.getFeatureConfig().isCfbEnable();
//        Log.d(TAG, "isCFBSupported = " + support);
        return support;
    }

    private boolean buildPictureSizeEntries(ListPreference pref, List<String> supportedSizes) {
        if (supportedSizes == null || supportedSizes.size() == 0) {
            return true;
        }
        sortSizesInAscending(supportedSizes);
        DecimalFormat sMegaPixelFormat = null;
        List<String> entryValuesList = new ArrayList<String>(); // like: 640x480, ...
        List<String> entriesList = new ArrayList<String>();     // like: VGA, 5 megapixels, ...

        if(FeatureSwitcher.isTablet()) {    // tablet
            sMegaPixelFormat = new DecimalFormat("##0.#");
        } else {                            // non tablet
            sMegaPixelFormat = new DecimalFormat("##0");
        }
        for (int i = 0; i < supportedSizes.size(); i++) {
            // Generator entry string.
            Point size = SettingUtils.getSize(supportedSizes.get(i));
            String megaPixels = sMegaPixelFormat.format((size.x * size.y) / 1e6);
            String entry = mContext.getResources()
                    .getString(R.string.setting_summary_megapixels, megaPixels);

            if (size.x * size.y == SettingUtils.VGA_SIZE) {
                entry = "VGA";
            } else if (size.x * size.y == SettingUtils.QVGA_SIZE) {
                entry = "QVGA";
            }
            // If the entries list already has this entry and its entry value has the
            // same ratio, update the entry values in entry values list.
            boolean isEntryHaved = false;
            int index = entriesList.indexOf(entry);
            if (index > 0) {
                for (int j = index; j < entriesList.size(); j++) {
                    Point entryValue = SettingUtils.getSize(entryValuesList.get(j));
                    if (SettingUtils.toleranceRatio(size, entryValue)) {
                        isEntryHaved = true;
                        index = j;
                        break;
                    }
                }
            }
            if (isEntryHaved) {
                entryValuesList.set(index, supportedSizes.get(i));
            } else {
                entryValuesList.add(supportedSizes.get(i));
                entriesList.add(entry);
            }
        }
        // Set entries and entry values to preference.
        CharSequence[] entryValues = new CharSequence[entryValuesList.size()];
        CharSequence[] entries = new CharSequence[entryValuesList.size()];
        pref.setOriginalEntryValues(entryValuesList.toArray(entryValues));
        pref.setOriginalEntries(entriesList.toArray(entries));
        pref.filterUnsupported(supportedSizes);
        return false;
    }

    private void sortSizesInAscending(List<String> supportedPictureSizes) {
        String maxSizeInString = null;
        Point maxSize = null;
        int maxIndex = 0;
        // Loop (n - 1) times.
        for (int i = 0; i < supportedPictureSizes.size() - 1; i++) {
            maxSizeInString = supportedPictureSizes.get(0);
            maxSize = SettingUtils.getSize(maxSizeInString);
            maxIndex = 0;
            String tempSizeInString = null;
            Point tempSize = null;
            // Find the max size.
            for (int j = 0; j < supportedPictureSizes.size() - i; j++) {
                tempSizeInString = supportedPictureSizes.get(j);
                tempSize = SettingUtils.getSize(tempSizeInString);
                if (tempSize.x * tempSize.y > maxSize.x * maxSize.y) {
                    maxSizeInString = tempSizeInString;
                    maxSize = tempSize;
                    maxIndex = j;
                }
            }
            // Place the max size to the end position.
            supportedPictureSizes.set(maxIndex, tempSizeInString);
            supportedPictureSizes.set(supportedPictureSizes.size() - 1 - i, maxSizeInString);
        }
    }
}
