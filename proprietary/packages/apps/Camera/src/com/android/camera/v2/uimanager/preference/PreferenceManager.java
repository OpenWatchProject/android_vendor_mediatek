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
package com.android.camera.v2.uimanager.preference;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.provider.MediaStore;
import android.util.Size;

import com.android.camera.R;
import com.android.camera.v2.app.SettingAgent;
import com.android.camera.v2.util.CameraUtil;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class PreferenceManager {
    private static final Tag TAG = new Tag(PreferenceManager.class.getSimpleName());
    private Context                            mContext = null;
    private SettingAgent                       mSettingAgent = null;
    private Map<Integer, PreferenceGroup> mPreferenceGroupMap =
            new HashMap<Integer, PreferenceGroup>();
    private int                                mCameraId = 0;
    private Activity                           mActivity;
    private boolean                            mIsNonePickIntent = true;

    /**
     * The constructor of preference manager.
     * @param activity The activity used to get intent info.
     * @param settingAgent The agent of setting module which used to get supported values
     *     of setting, trigger setting changing, read and write setting values in shared
     *     preferences.
     */
    public PreferenceManager(Activity activity, SettingAgent settingAgent) {
        LogHelper.i(TAG, "[PreferenceManager], instructor");
        mActivity = activity;
        mContext = activity.getApplicationContext();
        mSettingAgent = settingAgent;

        Intent intent = mActivity.getIntent();
        String action = null;
        if (intent != null) {
            action = intent.getAction();
        }
        LogHelper.i(TAG, "[PreferenceManager], action:" + action);

        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
                || MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            mIsNonePickIntent = false;
        }
    }

    /**
     * Initialize list preference from XML and filter its values compared to values
     * supported by native.
     * @param preferencesRes The id of XML which used to configure list preference info.
     * @param cameraId The id of camera.
     */
    public void initializePreferences(int preferencesRes, int cameraId) {
        LogHelper.i(TAG, "[initializePreferences], start, cameraId:" + cameraId);
        // Gather keys of setting needed initialized.
        String[] intializedKeys = new String[SettingKeys.KEYS_FOR_SETTING.length];
        for (int i = 0; i < SettingKeys.KEYS_FOR_SETTING.length; i++) {
            intializedKeys[i] = SettingKeys.KEYS_FOR_SETTING[i];
        }
        if (!mIsNonePickIntent) {
            for (int i = 0; i < SettingKeys.UN_SUPPORT_BY_3RDPARTY.length; i++) {
                int settingId = SettingKeys.UN_SUPPORT_BY_3RDPARTY[i];
                intializedKeys[settingId] = null;
            }
        }

        mCameraId = cameraId;
        PreferenceGroup group = mPreferenceGroupMap.get(cameraId);
        if (group == null) {
            //constructor listPreference from xml.
            PreferenceInflater inflater = new PreferenceInflater(mContext);
            group = (PreferenceGroup) inflater.inflate(preferencesRes);
            mPreferenceGroupMap.put(cameraId, group);
            filterPreferences(intializedKeys, cameraId);
        }

        // camera id may be changed, so do camera id setting change every time.c
        mSettingAgent.doSettingChange(SettingKeys.KEY_CAMERA_ID, String.valueOf(cameraId));
        // Setting may have last setting changed value, when change to a new camera, firstly
        // reset them and make their value to default value.
        configureSettings(intializedKeys, cameraId);
        LogHelper.i(TAG, "[initializePreferences], end");
    }

    /**
     * Get list preference from list preference group.
     * @param key The key used to indicate the list preference.
     * @return The list preference indicated by the input key.
     */
    public ListPreference getListPreference(String key) {
        if (key == null) {
            return null;
        }
        if (!mIsNonePickIntent) {
            int settingId = SettingKeys.getSettingId(key);
            for (int i = 0; i < SettingKeys.UN_SUPPORT_BY_3RDPARTY.length; i++) {
                if (settingId == SettingKeys.UN_SUPPORT_BY_3RDPARTY[i]) {
                    return null;
                }
            }
        }
        PreferenceGroup group = mPreferenceGroupMap.get(mCameraId);
        return group.findPreference(key);
    }

    /**
     * Get list preference group.
     * @return The list preference group.
     */
    public PreferenceGroup getPreferenceGroup() {
        PreferenceGroup group = mPreferenceGroupMap.get(mCameraId);
        return group;
    }

    /**
     * Update list preference values after setting changed.
     * @param values The affected settings' values.
     * @param overrideValues The affected settings' override values.
     */
    public void updateSettingResult(Map<String, String> values,
            Map<String, String> overrideValues) {
        LogHelper.i(TAG, "[updateSettingResult]");
        if (values != null) {
            Set<String> set = values.keySet();
            Iterator<String> iterator = set.iterator();
            PreferenceGroup group = mPreferenceGroupMap.get(mCameraId);
            while (iterator.hasNext()) {
                String key = iterator.next();
                String value = values.get(key);
                String overrideValue = CameraUtil.buildEnabledList(overrideValues.get(key), value);
                ListPreference preference = group.findPreference(key);
                if (preference != null  && preference.isVisibled()) {
                    preference.setValue(value);
                    preference.setOverrideValue(overrideValue);
                }
            }
        }
    }

    /**
     * Set settings as its default values when setting reset.
     */
    public void restoreSetting() {
        String[] restoreKeys = new String[SettingKeys.KEYS_FOR_SETTING.length];
        for (int i = 0; i < SettingKeys.KEYS_FOR_SETTING.length; i++) {
            restoreKeys[i] = SettingKeys.KEYS_FOR_SETTING[i];
        }
        if (!mIsNonePickIntent) {
            for (int i = 0; i < SettingKeys.UN_SUPPORT_BY_3RDPARTY.length; i++) {
                int settingId = SettingKeys.UN_SUPPORT_BY_3RDPARTY[i];
                restoreKeys[settingId] = null;
            }
            for (int i = 0; i < SettingKeys.SUPPORT_BY_3RDPARTY_BUT_HIDDEN.length; i++) {
                int settingId = SettingKeys.SUPPORT_BY_3RDPARTY_BUT_HIDDEN[i];
                restoreKeys[settingId] = null;
            }
        }
        // Clear values of share preferences.
        Set<Integer> set = mPreferenceGroupMap.keySet();
        Iterator<Integer> iterator = set.iterator();
        while (iterator.hasNext()) {
            String cameraId = iterator.next().toString();
            mSettingAgent.clearSharedPreferencesValue(restoreKeys, cameraId);
        }

        // Gather setting default values.
        Map<String, String> defaultSettings = new HashMap<String, String>();
        PreferenceGroup group = mPreferenceGroupMap.get(mCameraId);
        for (int i = 0; i < restoreKeys.length; i++) {
            String key = restoreKeys[i];
            ListPreference pref = group.findPreference(key);
            if (pref != null) {
                String defaultValue = pref.getDefaultValue();
                pref.setValue(defaultValue);
                defaultSettings.put(key, defaultValue);
            }
        }
        // Do not need to reset camera id.
        defaultSettings.remove(SettingKeys.KEY_CAMERA_ID);
        // Do setting change.
        mSettingAgent.doSettingChange(defaultSettings);
    }

    /**
     * Clear some setting's values in the shared preferences when camera activity
     * destroyed.
     */
    public void clearSharedPreferencesValue() {
        Set<Integer> set = mPreferenceGroupMap.keySet();
        Iterator<Integer> iterator = set.iterator();
        int[] clearKeysId = null;
        if (mIsNonePickIntent) {
            clearKeysId = SettingKeys.RESET_SETTING_ITEMS;
        } else {
            clearKeysId = SettingKeys.THIRDPART_RESET_SETTING_ITEMS;
        }
        String[] clearKeys = new String[clearKeysId.length];
        for (int i = 0; i < clearKeysId.length; i++) {
            int settingId = clearKeysId[i];
            String settingKey = SettingKeys.getSettingKey(settingId);
            clearKeys[i] = settingKey;
        }

        while (iterator.hasNext()) {
            String cameraId = iterator.next().toString();
            mSettingAgent.clearSharedPreferencesValue(clearKeys, cameraId);
        }
    }

    private void filterPreferences(String[] prefKeys, int cameraId) {
        PreferenceGroup group = mPreferenceGroupMap.get(cameraId);
        for (int i = 0; i < prefKeys.length; i++) {
            String key = prefKeys[i];
            ListPreference preference = group.findPreference(key);
            if (preference == null) {
                continue;
            }
            int type = SettingKeys.getSettingType(key);
            // only decided by by app layer.
            if (type == SettingKeys.DECIDE_BY_APP) {
                continue;
            }
            // get the supported values of setting which is decide by native from native.
            List<String> supportedValues =
                    mSettingAgent.getSupportedValues(key, String.valueOf(mCameraId));
            String supportedValue = null;
            if (supportedValues != null) {
                for (int k = 0; k < supportedValues.size(); k++) {
                    String value = supportedValues.get(k);
                    supportedValue = supportedValue + value + ",";
                }
            }
            LogHelper.d(TAG, "key:" + key + ", supportedValue:" + supportedValue);
            //filter listPreference.
            if (SettingKeys.KEY_PICTURE_SIZE.equals(key)) {
                buildPictureSizeEntries(preference, supportedValues);
            } else {
                filterUnSupportedValues(preference, supportedValues);
            }
        }
        filterGroupListPrference(group, SettingKeys.KEY_IMAGE_PROPERTIES);
        if (!mIsNonePickIntent) {
            // Hide the some setting when camera launched from 3rd party.
            for (int i = 0; i < SettingKeys.SUPPORT_BY_3RDPARTY_BUT_HIDDEN.length; i++) {
                int settingId = SettingKeys.SUPPORT_BY_3RDPARTY_BUT_HIDDEN[i];
                String hiddenKey = SettingKeys.getSettingKey(settingId);
                ListPreference preference = group.findPreference(hiddenKey);
                if (preference != null) {
                    preference.setVisibled(false);
                }
            }
        }
    }

    private void configureSettings(String[] settingKeys, int cameraId) {
        List<String> justConfigureDefaultValues = new ArrayList<String>();
        if (!mIsNonePickIntent) {
            for (int i = 0; i < SettingKeys.SUPPORT_BY_3RDPARTY_BUT_HIDDEN.length; i++) {
                int settingId = SettingKeys.SUPPORT_BY_3RDPARTY_BUT_HIDDEN[i];
                String key = SettingKeys.getSettingKey(settingId);
                if (!SettingKeys.KEY_RECORD_LOCATION.equals(key)
                        && !SettingKeys.KEY_VIDEO_QUALITY.equals(key)) {
                    justConfigureDefaultValues.add(key);
                }
            }
        }
        PreferenceGroup group = mPreferenceGroupMap.get(cameraId);
        Map<String, String> defaultSettings = new LinkedHashMap<String, String>();
        for (int i = 0; i < settingKeys.length; i++) {
            String key = settingKeys[i];
            ListPreference preference = group.findPreference(key);
            if (preference == null) {
                continue;
            }
            preference.setOverrideValue(null);
            String defaultValue = preference.getDefaultValue();
            if (defaultValue == null) {
                List<String> supportedValues = mSettingAgent
                        .getSupportedValues(key, String.valueOf(cameraId));
                if (supportedValues != null) {
                    defaultValue = supportedValues.get(0);
                }
                preference.setDefaultValue(defaultValue);
            }
            defaultSettings.put(key, defaultValue);
            // synch listPreference value to the value in sharedpreferences.
            String sharePreferencesValue =
                    mSettingAgent.getSharedPreferencesValue(key, String.valueOf(cameraId));
            if (sharePreferencesValue != null
                    && !justConfigureDefaultValues.contains(key)) {
                defaultSettings.put(key, sharePreferencesValue);
            }
        }
        // do not need to configurate camera id.
        defaultSettings.remove(SettingKeys.KEY_CAMERA_ID);
        mSettingAgent.configurateSetting(defaultSettings);
    }

    private void buildPictureSizeEntries(ListPreference pref, List<String> supportedSizes) {
        if (supportedSizes == null || supportedSizes.size() == 0) {
            return;
        }
        sortSizesInAscending(supportedSizes);
        DecimalFormat sMegaPixelFormat = new DecimalFormat("##0");
        List<String> entryValuesList = new ArrayList<String>();
        List<String> entriesList = new ArrayList<String>();

        for (int i = 0; i < supportedSizes.size(); i++) {
            Size size = CameraUtil.getSize(supportedSizes.get(i));
            if (size == null) {
                continue;
            }
            String megaPixels = sMegaPixelFormat.format(
                    (size.getWidth() * size.getHeight()) / 1e6);
            String entry = mContext.getResources()
                    .getString(R.string.setting_summary_megapixels, megaPixels);
            if (size.getWidth() * size.getHeight() == CameraUtil.VGA_SIZE) {
                entry = "VGA";
            } else if (size.getWidth() * size.getHeight() == CameraUtil.QVGA_SIZE) {
                entry = "QVGA";
            }
            // If the entries list already has this entry and its entry value has the
            // same ratio, update the entry values in entry values list.
            boolean isEntryHaved = false;
            int index = entriesList.indexOf(entry);
            if (index > 0) {
                Size entryValue = CameraUtil.getSize(entryValuesList.get(index));
                if (CameraUtil.toleranceRatio(size, entryValue)) {
                    isEntryHaved = true;
                }
            }
            if (isEntryHaved) {
                entryValuesList.set(index, supportedSizes.get(i));
            } else {
                entryValuesList.add(supportedSizes.get(i));
                entriesList.add(entry);
            }
        }
        CharSequence[] entryValues = new CharSequence[entryValuesList.size()];
        CharSequence[] entries = new CharSequence[entryValuesList.size()];
        pref.setOriginalEntryValues(entryValuesList.toArray(entryValues));
        pref.setOriginalEntries(entriesList.toArray(entries));
        pref.filterUnsupported(supportedSizes);
    }

    private void sortSizesInAscending(List<String> supportedPictureSizes) {
        String maxSizeInString = null;
        Size maxSize = null;
        int maxIndex = 0;
        // Loop (n - 1) times.
        for (int i = 0; i < supportedPictureSizes.size() - 1; i++) {
            maxSizeInString = supportedPictureSizes.get(0);
            maxSize = CameraUtil.getSize(maxSizeInString);
            if (maxSize == null) {
                continue;
            }
            maxIndex = 0;
            String tempSizeInString = null;
            Size tempSize = null;
            // Find the max size.
            for (int j = 0; j < supportedPictureSizes.size() - i; j++) {
                tempSizeInString = supportedPictureSizes.get(j);
                tempSize = CameraUtil.getSize(tempSizeInString);
                if (tempSize == null) {
                    continue;
                }
                if (tempSize.getWidth() * tempSize.getHeight()
                        > maxSize.getWidth() * maxSize.getHeight()) {
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

    private void filterUnSupportedValues(ListPreference pref, List<String> supportedValues) {
        if (supportedValues != null) {
            pref.filterUnsupported(supportedValues);
        }

        if (supportedValues == null || supportedValues.size() <= 1) {
            pref.setVisibled(false);
            return;
        }

        if (pref.getEntries().length <= 1) {
            pref.setVisibled(false);
            return;
        }

        resetIfInvalid(pref, true);
    }

    private void filterGroupListPrference(PreferenceGroup group, String key) {
        ListPreference groupPref = group.findPreference(key);
        if (groupPref == null) {
            return;
        }
        CharSequence[] entryValues = groupPref.getOriginalEntries();
        if (entryValues == null) {
            groupPref.setVisibled(false);
            return;
        }

        List<ListPreference> mChildPrefernce = new ArrayList<ListPreference>();
        for (CharSequence value : entryValues) {
            ListPreference pref = group.findPreference(value.toString());
            if (pref != null && pref.isVisibled()) {
                mChildPrefernce.add(pref);
            }
        }
        if (mChildPrefernce.size() <= 0) {
            groupPref.setVisibled(false);
        } else {
            groupPref.setChildPreferences(mChildPrefernce.toArray(
                    new ListPreference[mChildPrefernce.size()]));
        }
    }

    private void resetIfInvalid(ListPreference pref, boolean first) {
        // Set the value to the first entry if it is invalid.
        String value = pref.getValue();
        if (pref.findIndexOfValue(value) == -1) {
            if (first) {
                pref.setValueIndex(0);
            } else if (pref.getEntryValues() != null && pref.getEntryValues().length > 0) {
                pref.setValueIndex(pref.getEntryValues().length - 1);
            }
        }
    }
}
