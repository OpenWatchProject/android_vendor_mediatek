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
import android.content.SharedPreferences.Editor;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.ISettingRule.MappingFinder;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.setting.preference.PreferenceGroup;
import com.mediatek.camera.setting.preference.SharedPreferencesTransfer;
import com.mediatek.camera.setting.rule.CommonRule;
import com.mediatek.camera.setting.rule.RuleContainer;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class SettingCtrl implements ISettingCtrl {
    private static final String TAG = "SettingCtrl";

    private static final int QUERY_BY_X_AXIS = 0;
    private static final int QUERY_BY_Y_AXIS = 1;

    private boolean mIsInitializedSettings = false;

    private ISettingRule[][] mRuleMatrix
        = new ISettingRule[SettingConstants.SETTING_COUNT][SettingConstants.SETTING_COUNT];

    private ICameraContext mICameraContext;
    private ICameraDeviceManager mICameraDeviceManager;
    private ICameraDevice mICameraDevice;

    private Context mContext;
    private SharedPreferencesTransfer mPrefTransfer;
    private SettingGenerator mSettingGenerator;
    private HashMap<Integer, SharedPreferences> mLocalPrefs;
    private SharedPreferences mGlobalPref;

    public SettingCtrl(ICameraContext cameraContext) {
        mICameraContext = cameraContext;
        mContext = cameraContext.getActivity();
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
        mLocalPrefs = new HashMap<Integer, SharedPreferences>(
                mICameraDeviceManager.getNumberOfCameras());
    }

    @Override
    public void initializeSettings(int preferenceRes, SharedPreferences globalPref,
            SharedPreferences localPref) {
//        Log.d(TAG, "[initializeSettings]...");
        mGlobalPref = globalPref;
        mLocalPrefs.put(mICameraDeviceManager.getCurrentCameraId(), localPref);
        mPrefTransfer = new SharedPreferencesTransfer(globalPref, localPref);
        mSettingGenerator = new SettingGenerator(mICameraContext, mPrefTransfer);
        mSettingGenerator.createSettings(preferenceRes);
        createRules();
        mIsInitializedSettings = true;
    }

    @Override
    public boolean isSettingsInitialized() {
        return mIsInitializedSettings;
    }

    @Override
    public void onSettingChanged(String settingKey, String value) {
//        Log.i(TAG, "[onSettingChanged], settingKey = " + settingKey + ",value = " + value);
        if (!mIsInitializedSettings) {
            Log.w(TAG, "[onSettingChanged] mIsInitializedSettings is false, return.");
            return;
        }

        if (SettingConstants.KEY_NORMAL.equals(settingKey)) {
//            Log.i(TAG, "[onSettingChanged] settingKey is KEY_NORMAL,return.");
            return;
        }

        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        mICameraDevice = mICameraDeviceManager.getCameraDevice(currentCameraId);
        if (mICameraDevice == null) {
            return;
        }

        Parameters parameters = mICameraDevice.getParameters();
        // The switcher setting changed means user actively open or close it.
        // In this situation, the override record about it should clear before changing.
        // This is workaround.
        if (SettingConstants.KEY_HDR.equals(settingKey)) {
            int settingId = SettingConstants.getSettingId(settingKey);
            SettingItem setting = getSetting(settingId);
            setting.clearAllOverrideRecord();
        }
        onSettingChanged(parameters, currentCameraId, settingKey, value);

        if (SettingConstants.KEY_SCENE_MODE.equals(settingKey)) {
            SettingItem videoSetting = getSetting(SettingConstants.ROW_SETTING_VIDEO);
            int conditionSettingIndex = SettingConstants.ROW_SETTING_VIDEO;
            int resultSettingIndex = SettingConstants.ROW_SETTING_PICTURE_RATIO;
            ISettingRule rule = mRuleMatrix[conditionSettingIndex][resultSettingIndex];
            if (rule != null && "on".equals(videoSetting.getValue())) {
                rule.execute();
            }
        }
    }

    private void onSettingChanged(Parameters parameters, int currentCameraId, String key,
            String value) {
        int settingId = SettingConstants.getSettingId(key);
        SettingItem setting = getSetting(settingId);
        String lastValue = setting.getLastValue();
        if (value == null
                || value.equals(lastValue) && !SettingConstants.KEY_PICTURE_SIZE.equals(key)) {
            return;
        }
        setting.setValue(value);
        setting.setLastValue(value);

        boolean isExecutedByRule = false;
        if (isNeedQueryByYAxis(key)) {
            List<SettingItem> settingItems = queryConditionSettings(key);
            for (int i = 0; i < settingItems.size(); i++) {
                SettingItem conditionSettingItem = settingItems.get(i);
                int conditionSettingId = conditionSettingItem.getSettingId();
                ISettingRule rule = mRuleMatrix[conditionSettingId][settingId];
                if (rule == null) {
                    continue;
                }

                String cDefalutValue = conditionSettingItem.getDefaultValue();
                String cValue = conditionSettingItem.getValue();
                if (cDefalutValue != null && cValue != null && !cDefalutValue.equals(cValue)) {
                    rule.execute();
                    isExecutedByRule = true;
                    break;
                }
            }
        }

        if (!isExecutedByRule) {
            if (SettingConstants.KEY_PICTURE_RATIO.equals(key)) {
                SettingUtils.setPreviewSize(mContext, parameters, value);
            } else {
                int settingType = setting.getType();
                switch (settingType) {
                case SettingConstants.ONLY_IN_PARAMETER:
                case SettingConstants.BOTH_IN_PARAMETER_AND_PREFERENCE:
                    if (setting.isEnable()) {
                        ParametersHelper
                                .setParametersValue(parameters, currentCameraId, key, value);
                    } else {
                        Log.d(TAG, "[onSettingChanged], setting is disable, key:" + key);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        executeRule(parameters, currentCameraId, key);
    }

    private void executeRule(Parameters parameters, int currentCameraId, String conditionKey) {
        List<SettingItem> settingItems = null;
        SettingItem conditionSettingItem = mSettingGenerator.getSettingItem(conditionKey);
        settingItems = queryResultSettings(conditionKey);
        int conditionSettingId = SettingConstants.getSettingId(conditionKey);
        String value = conditionSettingItem.getValue();
        if (value != null && value.equals(conditionSettingItem.getDefaultValue())) {
            // leave
            for (int i = 0; i < settingItems.size(); i++) {
                SettingItem ruseltSettingItem = settingItems.get(i);
                int resultSettingId = ruseltSettingItem.getSettingId();
                ISettingRule rule = mRuleMatrix[conditionSettingId][resultSettingId];
                rule.execute();
            }

            for (int i = 0; i < settingItems.size(); i++) {
                SettingItem ruseltSettingItem = settingItems.get(i);
                onSettingChanged(parameters, currentCameraId, ruseltSettingItem.getKey(),
                        ruseltSettingItem.getValue());
            }
        } else {
            // enter
            for (int i = 0; i < settingItems.size(); i++) {
                SettingItem ruseltSettingItem = settingItems.get(i);
                int resultSettingId = ruseltSettingItem.getSettingId();
                ISettingRule rule = mRuleMatrix[conditionSettingId][resultSettingId];
                rule.execute();
                onSettingChanged(parameters, currentCameraId, ruseltSettingItem.getKey(),
                        ruseltSettingItem.getValue());
            }
        }
    }

    @Override
    public void addRule(String conditionKey, String resultKey, ISettingRule rule) {
//        Log.d(TAG, "[addRule], conditionKey:" + conditionKey + ", " + "resultKey:" + resultKey
//                + ", rule:" + rule);
        int condtionSettingId = SettingConstants.getSettingId(conditionKey);
        int resultSettingId = SettingConstants.getSettingId(resultKey);
        mRuleMatrix[condtionSettingId][resultSettingId] = rule;
    }

    @Override
    public void executeRule(String conditionKey, String resultKey) {
        int condtionSettingId = SettingConstants.getSettingId(conditionKey);
        int resultSettingId = SettingConstants.getSettingId(resultKey);
        ISettingRule rule = mRuleMatrix[condtionSettingId][resultSettingId];
        if (rule != null) {
            rule.execute();
        }
    }

    @Override
    public String getSettingValue(String key) {
        if (!mIsInitializedSettings) {
//            Log.d(TAG, "[getSettingValue] mIsInitializedSettings:" + mIsInitializedSettings);
            return null;
        }
        String value = null;
        SettingItem settingItem = getSetting(key);
        value = settingItem.getValue();
        if (value == null) {
            value = settingItem.getDefaultValue();
        }
        return value;
    }

    @Override
    public SettingItem getSetting(String key) {
        int settingId = SettingConstants.getSettingId(key);
        return getSetting(settingId);
    }

    @Override
    public String getDefaultValue(String key) {
        if (!mIsInitializedSettings) {
//            Log.d(TAG, "[getSettingValue] mIsInitializedSettings:" + mIsInitializedSettings);
            return null;
        }
        String value = null;
        SettingItem settingItem = getSetting(key);
        value = settingItem.getDefaultValue();
        return value;
    }

    @Override
    public void setSettingValue(String key, String value, int cameraId) {
        int settingId = SettingConstants.getSettingId(key);
        SettingItem settingItem = getSetting(settingId, cameraId);
        settingItem.setValue(value);
    }

    @Override
    public ListPreference getListPreference(String key) {
        if (!mIsInitializedSettings) {
//            Log.d(TAG, "[getListPreference] mIsInitializedSettings:" + mIsInitializedSettings);
            return null;
        }
        int settingId = SettingConstants.getSettingId(key);
        return mSettingGenerator.getListPreference(settingId);
    }

    @Override
    public PreferenceGroup getPreferenceGroup() {
        return mSettingGenerator.getPreferenceGroup();
    }

    @Override
    public void updateSetting(SharedPreferences localPref) {
//        Log.d(TAG, "[updateSetting], start...");
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        mLocalPrefs.put(currentCameraId, localPref);
        mPrefTransfer.updateLocalPreferences(localPref);
        mSettingGenerator.updatePreferences();
        synchronizeSetting();

        mICameraDevice = mICameraDeviceManager.getCameraDevice(currentCameraId);
        if (mICameraDevice == null) {
            return;
        }

        Parameters parameters = mICameraDevice.getParameters();
        for (int i = 0; i < SettingConstants.SETTING_COUNT; i++) {
            SettingItem settingItem = mSettingGenerator.getSettingItem(i);
            String key = settingItem.getKey();
            String value = settingItem.getValue();
            String lastValue = settingItem.getLastValue();
            if (settingItem.isEnable() && value != null && !value.equals(lastValue)) {
                onSettingChanged(parameters, currentCameraId, key, value);
            }
        }
//        Log.d(TAG, "[updateSetting], end...");
    }

    @Override
    public void restoreSetting(int cameraId) {
//        Log.d(TAG, "[restoreSetting], cameraId:" + cameraId);
        mSettingGenerator.restoreSetting(cameraId);
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
        Parameters parameters = null;
        if (cameraDevice != null) {
            parameters = cameraDevice.getParameters();
        }

        if (parameters != null) {
            for (int i = 0; i < SettingConstants.SETTING_COUNT; i++) {
                SettingItem settingItem = mSettingGenerator.getSettingItem(i);
                String key = settingItem.getKey();
                String value = settingItem.getValue();
                String lastvalue = settingItem.getLastValue();
                String defaultValue = settingItem.getDefaultValue();
                if (defaultValue != null && value != null && !value.equals(lastvalue)) {
                    onSettingChanged(key, value);
                }
            }
        }
    }

    @Override
    public String getCameraMode(String modeKey) {
        int settingId = SettingConstants.getSettingId(modeKey);
        int index = SettingDataBase.getSettingColumn(settingId);
        int[][] modeMatrix = SettingDataBase.getRestrictionMatrix();
        int state = modeMatrix[SettingConstants.ROW_SETTING_CAMERA_MODE][index];
        return SettingDataBase.getSettingResetValue(
                SettingConstants.ROW_SETTING_CAMERA_MODE, state);
    }

    @Override
    public void resetSetting() {
        long start = System.currentTimeMillis();
        resetSettings(mGlobalPref);
        int count = mICameraDeviceManager.getNumberOfCameras();
        for (int i = 0; i < count; i++) {
             SharedPreferences sharedPreference = mLocalPrefs.get(i);
             if (sharedPreference != null) {
                 resetSettings(sharedPreference);
             }
        }
        long stop = System.currentTimeMillis();
//        Log.d(TAG, "resetSettings() consume:" + (stop - start));
    }

    @Override
    public SettingItem getSetting(String key, int cameraId) {
        int settingId = SettingConstants.getSettingId(key);
        return mSettingGenerator.getSettingItem(settingId, cameraId);
    }

    private void resetSettings(SharedPreferences sharedPreference) {
        if (sharedPreference == null) {
//            Log.e(TAG, "[resetSettings], sharedPreference is null, return");
            return;
        }
        Editor editor = sharedPreference.edit();
        boolean isNonePickIntent = mICameraContext.getModuleController().isNonePickIntent();
        if (isNonePickIntent) {
            int[] resetSettingItems = SettingConstants.RESET_SETTING_ITEMS;
            for (int i = 0, len = resetSettingItems.length; i < len; i++) {
                int settingId = resetSettingItems[i];
                String key = SettingConstants.getSettingKey(settingId);
                editor.remove(key); // remove it will use default.
            }
        } else {
            int[] resetSettingItems = SettingConstants.THIRDPART_RESET_SETTING_ITEMS;
            for (int i = 0, len = resetSettingItems.length; i < len; i++) {
                int settingId = resetSettingItems[i];
                String key = SettingConstants.getSettingKey(settingId);
                editor.remove(key); // remove it will use default.
            }
        }
        editor.apply();
    }

    private SettingItem getSetting(int settingId) {
        return mSettingGenerator.getSettingItem(settingId);
    }

    private SettingItem getSetting(int settingId, int cameraId) {
        return mSettingGenerator.getSettingItem(settingId, cameraId);
    }

    private void createRules() {
        createRuleFromResctrictionMatrix();
        createRuleFromRestrictions();
        createRuleFromScene();
        RuleContainer container = new RuleContainer(this, mICameraContext);
        container.addRule();
    }

    private void createRuleFromResctrictionMatrix() {
        int[][] restrictionMatrix = SettingDataBase.getRestrictionMatrix();
        if (restrictionMatrix == null) {
            return;
        }
        int rowLen = restrictionMatrix.length;
        int columnLen = 0;
        // get column length of matrix
        for (int row = 0; row < rowLen; row++) {
            if (restrictionMatrix[row] != null) {
                columnLen = restrictionMatrix[row].length;
                break;
            }
        }

        for (int column = 0; column < columnLen; column++) {
            int conditionSettingIndex = SettingDataBase.getSettingIndex(column);
            String conditionKey = SettingConstants.getSettingKey(conditionSettingIndex);
            for (int row = 0; row < rowLen; row++) {
                if (restrictionMatrix[row] == null) {
                    continue;
                }
                String value = SettingDataBase.getSettingResetValue(row,
                        restrictionMatrix[row][column]);
                if (value == null) {
                    continue;
                }
                ISettingRule rule = null;
                String resultKey = SettingConstants.getSettingKey(row);
                rule = new CommonRule(conditionKey, resultKey, mICameraDeviceManager,
                        mSettingGenerator);
                List<String> values = new ArrayList<String>();
                values.add(value);
                rule.addLimitation("on", values, null);

                mRuleMatrix[conditionSettingIndex][row] = rule;
            }
        }
    }

    /**
     * create rule between settings from restrictions array
     */
    private void createRuleFromRestrictions() {
        Restriction[] restrictionArray = SettingDataBase.getRestrictions();
        for (int i = 0; i < restrictionArray.length; i++) {
            Restriction restriction = restrictionArray[i];
            int conditionSettingId = restriction.getIndex();
            // when if just cfb ,not need remove the fine video quality
            if ((SettingConstants.ROW_SETTING_FACE_BEAUTY == conditionSettingId
                    && !mICameraContext.getFeatureConfig().isVfbEnable())
                    || (SettingConstants.ROW_SETTING_FACE_BEAUTY == conditionSettingId
                    && !SettingGenerator.isSupport4K2K)) {
//                Log.d(TAG, "not add the resction,index = " + conditionSettingId);
                continue;
            }

            String conditionKey = SettingConstants.getSettingKey(conditionSettingId);
            // get condition values
            List<String> conditionValues = restriction.getValues();
            // get limited setting informations
            List<Restriction> limitedRestrictions = restriction.getRestrictioins();
            for (int j = 0; j < limitedRestrictions.size(); j++) {
                Restriction limitedRestriction = limitedRestrictions.get(j);
                int resultSettingId = limitedRestriction.getIndex();
                String resultKey = SettingConstants.getSettingKey(resultSettingId);
                List<String> resultValues = limitedRestriction.getValues();
                MappingFinder mappingFinder = limitedRestriction.getMappingFinder();
                ISettingRule rule = null;
                if (mRuleMatrix[conditionSettingId][resultSettingId] == null) {
                    rule = new CommonRule(conditionKey, resultKey, mICameraDeviceManager,
                            mSettingGenerator);
                    mRuleMatrix[conditionSettingId][resultSettingId] = rule;
                } else {
                    rule = mRuleMatrix[conditionSettingId][resultSettingId];
                }
                // add limitation
                for (int k = 0; k < conditionValues.size(); k++) {
                    rule.addLimitation(conditionValues.get(k), resultValues, mappingFinder);
                }
            }
        }
    }

    /**
     * create rule between settings from scene mode matrix
     */
    private void createRuleFromScene() {
        // read relationship between setting from scene mode
        int[][] sceneMatrix = SettingDataBase.getSceneRestrictionMatrix();
        if (sceneMatrix == null) {
            return;
        }

        int rowLen = sceneMatrix.length;
        int columnLen = 0;
        // get column num of matrix
        for (int row = 0; row < rowLen; row++) {
            if (sceneMatrix[row] != null) {
                columnLen = sceneMatrix[row].length;
                break;
            }
        }

        int conditionSettingId = SettingConstants.ROW_SETTING_SCENCE_MODE;
        String conditionKey = SettingConstants.getSettingKey(conditionSettingId);
        for (int column = 0; column < columnLen; column++) {
            String conditionValue = SettingDataBase.getSceneMode(column);
            for (int row = 0; row < rowLen; row++) {
                if (sceneMatrix[row] == null) {
                    continue;
                }

                String value = SettingDataBase.getSettingResetValue(row, sceneMatrix[row][column]);
                if (value == null) {
                    continue;
                }
                String resultKey = SettingConstants.getSettingKey(row);
                ISettingRule rule = null;
                if (mRuleMatrix[conditionSettingId][row] == null) {
                    rule = new CommonRule(conditionKey, resultKey, mICameraDeviceManager,
                            mSettingGenerator);
                    mRuleMatrix[conditionSettingId][row] = rule;
                } else {
                    rule = mRuleMatrix[conditionSettingId][row];
                }
                List<String> values = new ArrayList<String>();
                values.add(value);
                rule.addLimitation(conditionValue, values, null);
            }
        }
    }

    /**
     * Query rules from rule matrix.
     *
     * @param key
     *            use to query rule in some row or column
     * @param queryAxis
     *            query orientation
     * @return list of rules.
     */
    private List<ISettingRule> queryRules(String key, int queryAxis) {
        List<ISettingRule> rules = new ArrayList<ISettingRule>();
        if (queryAxis == QUERY_BY_Y_AXIS) {
            // query rule by Y axis.
            int column = SettingConstants.getSettingId(key);
            int rowLen = mRuleMatrix.length;
            for (int row = 0; row < rowLen; row++) {
                ISettingRule rule = mRuleMatrix[row][column];
                if (rule != null) {
                    rules.add(rule);
                }
            }
        } else if (queryAxis == QUERY_BY_X_AXIS) {
            // query rule by x axis.
            int row = SettingConstants.getSettingId(key);
            int columnLen = mRuleMatrix[row].length;
            for (int column = 0; column < columnLen; column++) {
                ISettingRule rule = mRuleMatrix[row][column];
                if (rule != null) {
                    rules.add(rule);
                }
            }
        }
        return rules;
    }

    private List<SettingItem> queryConditionSettings(String resultKey) {
        List<SettingItem> settingItems = new ArrayList<SettingItem>();
        int column = SettingConstants.getSettingId(resultKey);
        int rowLength = mRuleMatrix.length;
        for (int row = 0; row < rowLength; row++) {
            ISettingRule rule = mRuleMatrix[row][column];
            if (rule != null) {
                SettingItem settingItem = getSetting(row);
                settingItems.add(settingItem);
            }
        }
        return settingItems;
    }

    private List<SettingItem> queryResultSettings(String conditionKey) {
        List<SettingItem> settingItems = new ArrayList<SettingItem>();
        int row = SettingConstants.getSettingId(conditionKey);
        int columnLen = mRuleMatrix[row].length;
        for (int column = 0; column < columnLen; column++) {
            ISettingRule rule = mRuleMatrix[row][column];
            if (rule != null) {
                SettingItem settingItem = getSetting(column);
                settingItems.add(settingItem);
            }
        }
        return settingItems;
    }

    private boolean isNeedQueryByYAxis(String key) {
        boolean isNeedQueryByYAxis = SettingConstants.KEY_PICTURE_RATIO.equals(key)
                || SettingConstants.KEY_CAMERA_ZSD.equals(key)
                || SettingConstants.KEY_PICTURE_SIZE.equals(key)
                || SettingConstants.KEY_ANTI_BANDING.equals(key);
        return isNeedQueryByYAxis;
    }

    /**
     * Some setting need keep same value when switch camera, like facebeauty.
     */
    private void synchronizeSetting() {
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        int numberOfCamera = mICameraDeviceManager.getNumberOfCameras();
        int faceBeautySettingId = SettingConstants.getSettingId(SettingConstants.KEY_FACE_BEAUTY);
        SettingItem currentSetting = getSetting(faceBeautySettingId, currentCameraId);
        SettingItem previousSetting = getSetting(faceBeautySettingId,
                (currentCameraId + 1) % numberOfCamera);
        currentSetting.setValue(previousSetting.getValue());
        previousSetting.setValue("off");

        int photoPIpSettingId = SettingConstants.getSettingId(SettingConstants.KEY_PHOTO_PIP);
        SettingItem currentPhotoPIpSetting = getSetting(photoPIpSettingId, currentCameraId);
        SettingItem previousPhotoPIpSetting = getSetting(photoPIpSettingId,
                (currentCameraId + 1) % numberOfCamera);
        currentPhotoPIpSetting.setValue(previousPhotoPIpSetting.getValue());
        previousPhotoPIpSetting.setValue("off");

        int videoPIpSettingId = SettingConstants.getSettingId(SettingConstants.KEY_VIDEO_PIP);
        SettingItem currentVideoPIpSetting = getSetting(videoPIpSettingId, currentCameraId);
        SettingItem previousVideoPIpSetting = getSetting(videoPIpSettingId,
                (currentCameraId + 1) % numberOfCamera);
        currentVideoPIpSetting.setValue(previousVideoPIpSetting.getValue());
        previousVideoPIpSetting.setValue("off");
    }
}
