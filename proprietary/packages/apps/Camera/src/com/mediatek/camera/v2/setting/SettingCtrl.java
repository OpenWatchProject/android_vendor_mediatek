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
package com.mediatek.camera.v2.setting;

import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.os.Handler;
import android.os.Looper;
import android.preference.PreferenceManager;
import android.util.Size;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.setting.rule.CommonRule;
import com.mediatek.camera.v2.setting.rule.ExtraRules;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class SettingCtrl {
    private static final Tag TAG = new Tag(SettingCtrl.class.getSimpleName());
    private static final int                    CURRENT_VERSION = 5;

    public static final String                  BACK_CAMERA = "0";
    public static final String                  FRONT_CAMERA = "1";

    private ISettingRule[][] mRuleMatrix = new
            ISettingRule[SettingKeys.SETTING_COUNT][SettingKeys.SETTING_COUNT];
    private SharedPreferences mGlobalPreferences;
    private Map<String, SharedPreferences>      mPreferencesMap
                                    = new HashMap<String, SharedPreferences>();
    private Map<String, SettingCharacteristics> mCharacteristicsMap
                                    = new HashMap<String, SettingCharacteristics>();
    private List<ISettingFilterListener>        mISettingFilterListeners
                                    = new ArrayList<ISettingFilterListener>();
    private Map<ISettingFilterListener, Handler> mSettingFilterHandler =
            new HashMap<ISettingFilterListener, Handler>();
    private SettingGenerator                    mSettingGenerator = null;
    private Context                             mContext;
    private String[]                            mCameraIds;
    private String                              mCurrentCameraId = BACK_CAMERA;
    private boolean                             mConfigurationCompleted = false;
    private long                                mConfigurateThreadId = -1;
    private SettingServant                      mSettingServantForAll;
    private SettingServant                      mSettingServantForBack;
    private SettingServant                      mSettingServantForFront;

    public interface ISettingFilterListener {
        public void onFilterResult(Map<String, String> values, Map<String, String> overrideValues);
    }

    public SettingCtrl(Context context) {
        LogHelper.i(TAG, "[SettingCtrl], constructor...");
        mContext = context;
        CameraManager cameraManager = (CameraManager) context
                .getSystemService(Context.CAMERA_SERVICE);

        if (cameraManager == null) {
            LogHelper.i(TAG, "cameraManager is null");
            return;
        }

        String packagesName = context.getPackageName();
        mGlobalPreferences = PreferenceManager.getDefaultSharedPreferences(context);
        upgradeOldVersion(mGlobalPreferences);
        try {
            // initialize MTK sharePreference.
            mCameraIds = cameraManager.getCameraIdList();
            for (String cameraId : mCameraIds) {
                SharedPreferences preferences = context.getSharedPreferences(
                        packagesName + "_preferences_" + cameraId, Context.MODE_PRIVATE);
                mPreferencesMap.put(cameraId, preferences);
            }

            // get feature supported values from native.
            for (String cameraId : mCameraIds) {
                CameraCharacteristics characteristics = cameraManager
                        .getCameraCharacteristics(cameraId);
                SettingCharacteristics settingCharacteristics =
                        new SettingCharacteristics(characteristics, cameraId, context);
                mCharacteristicsMap.put(cameraId, settingCharacteristics);
            }
        } catch (CameraAccessException e) {
            // TODO: handle exception
            LogHelper.e(TAG, "camera access exception" + e.getMessage());
        }
        initializeSettings();
        mSettingServantForAll = new SettingServant(this);
        mSettingServantForBack = new SettingServant(this, BACK_CAMERA);
        mSettingServantForFront = new SettingServant(this, FRONT_CAMERA);
    }

    private void initializeSettings() {
        LogHelper.i(TAG, "[initializeSettings], begin...");
        mSettingGenerator = new SettingGenerator();
        mSettingGenerator.initializeSettingItem(SettingKeys.KEYS_FOR_SETTING,
                mCameraIds, mCharacteristicsMap);
        createRules();
        LogHelper.i(TAG, "[initializeSettings], end");
    }

    /**
     * Register listener to monitor the filter result after setting changing finished.
     * @param listener The listener to monitor the changed result.
     * @param handler The handler to deal with the result callback. This handler should not
     *     be null.
     */
    public void registerSettingFilterListener(ISettingFilterListener listener,
            Handler handler) {
        LogHelper.i(TAG, "[registerSettingFilterListener], listener:" +
                "" + listener + ", handler:" + handler);

        if (listener != null && !mISettingFilterListeners.contains(listener)) {
            mISettingFilterListeners.add(listener);
            mSettingFilterHandler.put(listener, handler);
        }
    }

    /**
     * Remove the listener to monitor the filter result after executing setting rules.
     * @param listener
     */
    public void unRegisterSettingFilterListener(ISettingFilterListener listener) {
        LogHelper.i(TAG, "[unRegisterSettingFilterListener], listener:" + listener);
        mSettingFilterHandler.remove(listener);
        mISettingFilterListeners.remove(listener);
    }

    /**
     * Configure settings' values when setting module initialized.
     * @param configureSettings The settings' key and values going to configure.
     */
    public void configurateSetting(Map<String, String> configureSettings) {
        LogHelper.d(TAG, "[configurateSetting], configureSettings:" + configureSettings.toString());
        mConfigurateThreadId = Thread.currentThread().getId();
        mConfigurationCompleted = false;
        // Configure setting items before do setting change.
        mSettingGenerator.configureSettingItems(mCurrentCameraId);
        // Every time configuration setting, it forces to execute capture mode
        // setting's relative rules. This practice needs to improve.
        SettingItem captureModeItem = mSettingGenerator
                .getSettingItem(SettingKeys.KEY_CAPTURE_MODE);
        captureModeItem.setLastValue(null);
        configureSettings.put(SettingKeys.KEY_CAPTURE_MODE, captureModeItem.getValue());

        writeInIfSharedPreferenceValueIsNull(configureSettings, mCurrentCameraId);
        // update setting sync to shared preference values.
        doSettingChange(configureSettings, false);
        mConfigurationCompleted = true;
        synchronized (this) {
            try {
                LogHelper.i(TAG, "[configurateSetting], notify all thread");
                this.notifyAll();
            } catch (Exception e) {
                LogHelper.e(TAG, "[configurateSetting], exception");
            }
        }
        LogHelper.d(TAG, "[configurateSetting], done");
    }

    /**
     * This method will be called when setting changed.
     * @param key the key value of current setting, refer to SettingKeys.
     * @param value the value of current setting will be set.
     */
    public void doSettingChange(String key, String value) {
        doSettingChange(key, value, true);
    }

    /**
     * This method will be called when setting changed.
     * @param key the key value of current setting, refer to SettingKeys.
     * @param value the value of current setting will be set.
     * @param saved whether save value to shared preference or not.
     */
    public void doSettingChange(String key, String value, boolean saved) {
        if (key == null) {
            LogHelper.i(TAG, "[doSettingChange] key is null, return.");
        }
        if (SettingKeys.KEY_CAMERA_ID.equals(key)) {
            // Configurate camera id
            String cameraId = mGlobalPreferences.getString(SettingKeys.KEY_CAMERA_ID, null);
            LogHelper.i(TAG, "[doSettingChange], do camera switch, value:" + value + "," +
                    " cameaId:" + cameraId);
            if (value.equals(cameraId)) {
                return;
            }
            SharedPreferences.Editor editor = mGlobalPreferences.edit();
            editor.putString(SettingKeys.KEY_CAMERA_ID, value);
            editor.apply();

            mCurrentCameraId = value;
            mSettingGenerator.updateCameraId(value);
            mConfigurateThreadId = Thread.currentThread().getId();
            SettingItem item = mSettingGenerator.getSettingItem(SettingKeys.KEY_CAMERA_ID);
            item.setValue(value);
            mConfigurationCompleted = false;

            Map<String, String> changedSetting = new HashMap<String, String>();
            changedSetting.put(key, value);
            mSettingServantForBack.postResultToListeners(changedSetting);
            mSettingServantForFront.postResultToListeners(changedSetting);
            mSettingServantForAll.postResultToListeners(changedSetting);
            return;
        }

        SettingItem item = mSettingGenerator.getSettingItem(key);
        if (item == null) {
            LogHelper.i(TAG, "[doSettingChange], item:" + item + ", key:" + key);
            return;
        }
        String lastValue = item.getLastValue();
        boolean isEnabled = item.isEnable();
        LogHelper.i(TAG, "[doSettingChange], key:" + key + ", value:" + value + ", lastValue:" +
                "" + lastValue + ", isEnabled:" + isEnabled + ", saved:" + saved);
        if (value == null || value.equals(lastValue) || !isEnabled) {
            LogHelper.i(TAG, "[doSettingChange], do not need to change, return.");
            return;
        }

        // Write value to SharePreferences.
        if (saved) {
            setSharedPreferencesValue(key, value, mCurrentCameraId);
        }

        // Gather the affected setting items.
        List<SettingItem> affectedSettings = new ArrayList<SettingItem>();
        List<String> newKeys = new ArrayList<String>();
        affectedSettings.add(item);
        newKeys.add(key);
        gatherAffectedItems(affectedSettings, newKeys);

        // Record values of setting items which may be changed before changing.
        Map<String, String> beforeChanged = new HashMap<String, String>();
        for (int i = 0; i < affectedSettings.size(); i++) {
            SettingItem affectedSetting = affectedSettings.get(i);
            beforeChanged.put(affectedSetting.getKey(), affectedSetting.getValue());
        }

        // The switcher setting changed means user actively open or close it.
        // In this situation, the override record about it should clear before changing.
        // This is workaround.
        if (SettingKeys.KEY_HDR.equals(key)) {
            item.setOverrideValue(null);
            item.clearAllOverrideRecord();
        }
        // To do setting changed
        doSettingChanged2(key, value);

        // Gather the result of settings changed and post to other module.
        postResultToModule(affectedSettings, beforeChanged);

        // Callback result of setting changed and post to UI module.
        postResultToUI(affectedSettings);
    }

    /**
     * Execute setting changing with the changed setting.
     * @param changedSettings The setting which changed.
     * @param saved True means write the changed values to shared preferences.
     */
    public void doSettingChange(Map<String, String> changedSettings, boolean saved) {
        LogHelper.d(TAG, "[doSettingChange], changedSettings:" + changedSettings.toString());
        if (changedSettings.size() == 0) {
            return;
        }
        Set<String> keySet = changedSettings.keySet();
        Iterator<String> keyIterator = keySet.iterator();

        //Gather settings which are going to do setting change and settings which will be affected.
        List<SettingItem> doChangeSettings = new ArrayList<SettingItem>();
        List<SettingItem> affectedSettings = new ArrayList<SettingItem>();
        List<String> newKeys = new ArrayList<String>();
        while (keyIterator.hasNext()) {
            String key = keyIterator.next();
            SettingItem item = mSettingGenerator.getSettingItem(key);
            if (!item.isEnable()) {
                continue;
            }
            affectedSettings.add(item);
            newKeys.add(key);
            doChangeSettings.add(item);
        }
        gatherAffectedItems(affectedSettings, newKeys);

        // Record values of setting items which may be changed before changing.
        Map<String, String> beforeChanged = new HashMap<String, String>();
        for (int i = 0; i < affectedSettings.size(); i++) {
            SettingItem item = affectedSettings.get(i);
            String key = item.getKey();
            String value = item.getValue();
            beforeChanged.put(key, value);
        }

        // Update values of setting items going to change before changing.
        for (int i = 0; i < doChangeSettings.size(); i++) {
            SettingItem item = doChangeSettings.get(i);
            String key = item.getKey();
            item.setValue(changedSettings.get(key));
        }

        // May be current picture size ratio is not same with the picture ratio in changedSettings.
        if (changedSettings.containsKey(SettingKeys.KEY_PICTURE_RATIO)) {
            SettingItem item = mSettingGenerator.getSettingItem(SettingKeys.KEY_PICTURE_RATIO);
            item.setLastValue(null);
        }

        // Write value to SharePreferences.
        if (saved) {
            setSharedPreferencesValue(changedSettings, mCurrentCameraId);
        }

        // To do setting change and execute rules.
        for (int i = 0; i < doChangeSettings.size(); i++) {
            SettingItem item = doChangeSettings.get(i);
            String key = item.getKey();
            String value = item.getValue();
            doSettingChanged2(key, value);
        }

        // Gather the result of settings changed and post to other module.
        postResultToModule(affectedSettings, beforeChanged);

        // Callback result of setting changed and post to UI module.
        postResultToUI(affectedSettings);
    }

    /**
     * Execute setting changing with the changed setting.
     * @param changedSettings The setting which changed.
     */
    public void doSettingChange(Map<String, String> changedSettings) {
        doSettingChange(changedSettings, true);
    }

    public ISettingServant getSettingServant(String cameraId) {
        if (cameraId == null) {
            return getAllSettingServant();
        } else if (BACK_CAMERA.equals(cameraId)) {
            return getBackSettingServant();
        } else if (FRONT_CAMERA.equals(cameraId)) {
            return getFrontSettingServant();
        }
        return getAllSettingServant();
    }

    /**
     * Add rule to rule matrix
     * @param conditionKey the key of condition setting.
     * @param resultKey the key of result setting.
     * @param rule the rule between condition setting and result setting.
     */
    public void addRule(String conditionKey, String resultKey, ISettingRule rule) {
        LogHelper.i(TAG, "[addRule], conditionKey:" + conditionKey + ", " + "resultKey:" + resultKey
                + ", rule:" + rule);
        int condtionSettingId = SettingKeys.getSettingId(conditionKey);
        int resultSettingId = SettingKeys.getSettingId(resultKey);
        mRuleMatrix[condtionSettingId][resultSettingId] = rule;
    }

    /**
     * Execute rule between condition setting and result setting.
     * @param conditionKey the key of condition setting.
     * @param resultKey the key of result setting.
     */
    public void executeRule(String conditionKey, String resultKey) {
        //TODO: execute rule
    }

    public String getCurrentCameraId() {
        return mCurrentCameraId;
    }

    /**
     * Get supported values.
     * @param key the key of setting.
     * @return supported values.
     */
    public List<String> getSupportedValues(String key) {
        return getSupportedValues(key, mCurrentCameraId);
    }

    /**
     * Get supported values.
     * @param key the key of setting.
     * @return supported values.
     */
    public List<String> getSupportedValues(String key, String cameraId) {
        if (SettingKeys.KEY_CAMERA_ID.equals(key)) {
            List<String> cameraIds = new ArrayList<String>();
            for (String camera : mCameraIds) {
                cameraIds.add(camera);
            }
            return cameraIds;
        }
        SettingCharacteristics characteristics = mCharacteristicsMap.get(cameraId);
        return characteristics.getSupportedValues(key);
    }
    /**
     * Get the setting current value.
     * @param key the key value of setting, refer to SettingKeys.
     * @return the current value of setting.
     */
    public String getSettingValue(String key) {
        return getSettingValue(key, mCurrentCameraId);
    }

    /**
     * Get the setting current value.
     * @param key the key value of setting, refer to SettingKeys.
     * @param cameraId the id of camera
     * @return the current value of setting.
     */
    public String getSettingValue(String key, String cameraId) {

        synchronized (this) {
            long threadId = Thread.currentThread().getId();
            // may be this method will be call in configuration setting thread, avoid to dead lock
            // judge the thread id is equal configuration setting thread id.
            if (!mConfigurationCompleted && threadId != mConfigurateThreadId) {
                try {
                    LogHelper.i(TAG, "[getSettingValue]," +
                            " waiting..., thread:" + Thread.currentThread());
                    this.wait();
                } catch (Exception e) {
                    LogHelper.e(TAG, "[getSettingValue], exception");
                }
            }
        }

        SettingItem item = mSettingGenerator.getSettingItem(key, cameraId);
        return item.getValue();
    }


    /**
     * Get the SettingItem object.
     * @param key the key value of setting
     * @return return setting object
     */
    public SettingItem getSettingItem(String key) {
        return getSettingItem(key, mCurrentCameraId);
    }


    /**
     * Get the setting object.
     * @param key the key value of setting
     * @param cameraId the camera id
     * @return return setting object
     */
    public SettingItem getSettingItem(String key, String cameraId) {
        synchronized (this) {
            long threadId = Thread.currentThread().getId();
            // may be this method will be call in configuration setting thread, avoid to dead lock
            // judge the thread id is equal configuration setting thread id.
            if (!mConfigurationCompleted && threadId != mConfigurateThreadId) {
                try {
                    LogHelper.i(TAG, "[getSettingItem]," +
                            " waiting..., thread:" + Thread.currentThread());
                    this.wait();
                } catch (Exception e) {
                    LogHelper.e(TAG, "[getSettingValue], exception");
                }
            }
        }

        return mSettingGenerator.getSettingItem(key, cameraId);
    }


    /**
     * Get sharePreference value about current camera id.
     * @param key the key value of setting
     * @return the value of this key in sharePreference.
     */
    public String getSharePreferenceValue(String key) {
        return getSharePreferenceValue(key, mCurrentCameraId);
    }

    /**
     * Get sharePreference value about the given camera id
     * @param key the key value of setting
     * @param cameraId the camera id
     * @return the value of this key in sharePreference.
     */
    public String getSharePreferenceValue(String key, String cameraId) {
        if (isGlobalPref(key)) {
            String value = mGlobalPreferences.getString(key, null);
            return value;
        }
        SharedPreferences sharedPreferences = mPreferencesMap.get(cameraId);
        String value = sharedPreferences.getString(key, null);
        return value;
    }

    public void setSharedPreferencesValue(String key, String value) {
        setSharedPreferencesValue(key, value, mCurrentCameraId);
    }

    public void setSharedPreferencesValue(String key, String value, String cameraId) {
        SharedPreferences.Editor editor = null;
        if (isGlobalPref(key)) {
            editor = mGlobalPreferences.edit();
        } else {
            SharedPreferences preferences = mPreferencesMap.get(cameraId);
            editor = preferences.edit();
        }
        editor.putString(key, value);
        editor.apply();
    }

    public void setSharedPreferencesValue(final Map<String, String> keyValues,
            final String cameraId) {
        Set<String> keySet = keyValues.keySet();
        SharedPreferences sharedPreferences = mPreferencesMap.get(mCurrentCameraId);
        SharedPreferences.Editor localEditor = sharedPreferences.edit();
        SharedPreferences.Editor globalEditor = mGlobalPreferences.edit();

        String[] keys = keySet.toArray(new String[keySet.size()]);
        for (String key : keys) {
            String value = keyValues.get(key);
            if (isGlobalPref(key)) {
                globalEditor.putString(key, value);
            } else {
                localEditor.putString(key, value);
            }
        }
        localEditor.apply();
        globalEditor.apply();
    }

    public void clearSharedPreferencesValue(String[] keys, String cameraId) {
        SharedPreferences sharedPreferences = mPreferencesMap.get(cameraId);
        SharedPreferences.Editor localEditor = sharedPreferences.edit();
        SharedPreferences.Editor globalEditor = mGlobalPreferences.edit();
        for (String key : keys) {
            if (isGlobalPref(key)) {
                globalEditor.remove(key);
            } else {
                localEditor.remove(key);
            }
        }
        globalEditor.apply();
        localEditor.apply();
    }

    /**
     * Get preview size.
     * @return return the preview size.
     */
    public Size getPreviewSize() {
        return getPreviewSize(mCurrentCameraId);
    }

    public Size getPreviewSize(String cameraId) {
        cameraId = (cameraId == null) ? mCurrentCameraId : cameraId;
        SettingItem pictureRatioItem = mSettingGenerator.getSettingItem(
                SettingKeys.KEY_PICTURE_RATIO, cameraId);
        String ratioStr = pictureRatioItem.getValue();
        if (ratioStr == null) {
            // can not find picture ratio from setting item
            SharedPreferences sharedPreferences = mPreferencesMap.get(cameraId);
            ratioStr = sharedPreferences.getString(SettingKeys.KEY_PICTURE_RATIO, null);
        }

        double ratio;
        if (ratioStr != null) {
            ratio = Double.parseDouble(ratioStr);
        } else {
            ratio = Utils.findFullscreenRatio(mContext);
        }

        SettingCharacteristics characteristic = mCharacteristicsMap.get(cameraId);
        List<Size> supportedSizes = characteristic.getSupportedPreviewSize();
        Size size = Utils.getOptimalPreviewSize(mContext, supportedSizes, ratio);
        return size;
    }

    private void writeInIfSharedPreferenceValueIsNull(final Map<String, String> keyValues,
            final String cameraId) {
        Set<String> keySet = keyValues.keySet();
        SharedPreferences sharedPreferences = mPreferencesMap.get(cameraId);
        SharedPreferences.Editor localEditor = sharedPreferences.edit();
        SharedPreferences.Editor globalEditor = mGlobalPreferences.edit();
        String[] keys = keySet.toArray(new String[keySet.size()]);
        for (String key : keys) {
            String value = keyValues.get(key);
            if (isGlobalPref(key) && mGlobalPreferences.getString(key, null) == null) {
                globalEditor.putString(key, value);
            } else if (sharedPreferences.getString(key, null) == null) {
                localEditor.putString(key, value);
            }
        }
        localEditor.apply();
        globalEditor.apply();
    }

    private ISettingServant getAllSettingServant() {
        return mSettingServantForAll;
    }

    private ISettingServant getBackSettingServant() {
        return mSettingServantForBack;
    }

    private ISettingServant getFrontSettingServant() {
        return mSettingServantForFront;
    }

    private void createRules() {
        createRuleFromResctrictionMatrix();
        createRuleFromRestrictions();
        createRuleFromScene();
        createExtraRule();
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
            String conditionKey = SettingKeys.getSettingKey(conditionSettingIndex);
            SettingItem conditionItem = mSettingGenerator.getSettingItem(conditionKey);
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
                String resultKey = SettingKeys.getSettingKey(row);
                SettingItem resultItem = mSettingGenerator.getSettingItem(resultKey);
                conditionItem.addEffectdSetting(resultItem);
                rule = new CommonRule(conditionKey, resultKey, this);
                List<String> values = new ArrayList<String>();
                values.add(value);
                rule.addLimitation("on", values);

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
            String conditionKey = SettingKeys.getSettingKey(conditionSettingId);
            SettingItem conditionItem = mSettingGenerator.getSettingItem(conditionKey);
            // get condition values
            List<String> conditionValues = restriction.getValues();
            // get limited setting informations
            List<Restriction> limitedRestrictions = restriction.getRestrictioins();
            for (int j = 0; j < limitedRestrictions.size(); j++) {
                Restriction limitedRestriction = limitedRestrictions.get(j);
                int resultSettingId = limitedRestriction.getIndex();
                String resultKey = SettingKeys.getSettingKey(resultSettingId);
                SettingItem resultItem = mSettingGenerator.getSettingItem(resultKey);
                conditionItem.addEffectdSetting(resultItem);
                List<String> resultValues = limitedRestriction.getValues();
                ISettingRule rule = null;
                if (mRuleMatrix[conditionSettingId][resultSettingId] == null) {
                    rule = new CommonRule(conditionKey, resultKey, this);
                    mRuleMatrix[conditionSettingId][resultSettingId] = rule;
                } else {
                    rule = mRuleMatrix[conditionSettingId][resultSettingId];
                }
                // add limitation
                for (int k = 0; k < conditionValues.size(); k++) {
                    rule.addLimitation(conditionValues.get(k), resultValues);
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

        int conditionSettingId = SettingKeys.ROW_SETTING_SCENCE_MODE;
        String conditionKey = SettingKeys.getSettingKey(conditionSettingId);
        SettingItem conditionItem = mSettingGenerator.getSettingItem(conditionKey);
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
                String resultKey = SettingKeys.getSettingKey(row);
                SettingItem resultItem = mSettingGenerator.getSettingItem(resultKey);
                conditionItem.addEffectdSetting(resultItem);
                ISettingRule rule = null;
                if (mRuleMatrix[conditionSettingId][row] == null) {
                    rule = new CommonRule(conditionKey, resultKey, this);
                    mRuleMatrix[conditionSettingId][row] = rule;
                } else {
                    rule = mRuleMatrix[conditionSettingId][row];
                }
                List<String> values = new ArrayList<String>();
                values.add(value);
                rule.addLimitation(conditionValue, values);
            }
        }
    }

    private void createExtraRule() {
        ExtraRules rules = new ExtraRules(this);
        rules.createRules();
    }

    private void doSettingChanged2(String key, String value) {
        SettingItem settingItem = getSettingItem(key);
        String lastValue = settingItem.getLastValue();
        boolean isEnabled = settingItem.isEnable();
        LogHelper.d(TAG, "[doSettingChanged2], key:" + key + ", value:" + value + ", lastValue:" +
                "" + lastValue + ", isEnabled:" + isEnabled);
        if (value == null || value.equals(lastValue) || !isEnabled) {
            LogHelper.i(TAG, "[doSettingChanged2], key:" + key + ", do not need to change, return");
            return;
        }

        settingItem.setValue(value);
        settingItem.setLastValue(value);
        int settingId = settingItem.getSettingId();

        boolean isExecutedByRule = false;
        if (isNeedQueryByYAxis(key)) {
            LogHelper.d(TAG, "[doSettingChanged2], query rule by Y axis. key:" + key);
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
                if (cValue != null && !cValue.equals(cDefalutValue)) {
                    rule.execute();
                    isExecutedByRule = true;
                    break;
                }
            }
        }

        LogHelper.d(TAG, "[doSettingChanged2], isExecutedByRule:" + isExecutedByRule);
        executeRule(key);
    }

    private void executeRule(String conditionKey) {
        List<SettingItem> settingItems = null;
        SettingItem conditionSettingItem = getSettingItem(conditionKey);
        settingItems = queryResultSettings(conditionKey);
        int conditionSettingId = conditionSettingItem.getSettingId();
        for (int i = 0; i < settingItems.size(); i++) {
            SettingItem ruseltSettingItem = settingItems.get(i);
            int resultSettingId = ruseltSettingItem.getSettingId();
            ISettingRule rule = mRuleMatrix[conditionSettingId][resultSettingId];
            rule.execute();
            doSettingChanged2(ruseltSettingItem.getKey(), ruseltSettingItem.getValue());
        }
    }

    private boolean isNeedQueryByYAxis(String key) {
        boolean isNeedQueryByYAxis = SettingKeys.KEY_PICTURE_RATIO.equals(key)
                || SettingKeys.KEY_CAMERA_ZSD.equals(key)
                || SettingKeys.KEY_PICTURE_SIZE.equals(key)
                || SettingKeys.KEY_ANTI_BANDING.equals(key);
        return isNeedQueryByYAxis;
    }

    private List<SettingItem> queryConditionSettings(String resultKey) {
        List<SettingItem> settingItems = new ArrayList<SettingItem>();
        int column = SettingKeys.getSettingId(resultKey);
        int rowLength = mRuleMatrix.length;
        for (int row = 0; row < rowLength; row++) {
            ISettingRule rule = mRuleMatrix[row][column];
            if (rule != null) {
                SettingItem settingItem = getSettingItem(row);
                settingItems.add(settingItem);
            }
        }

        return settingItems;
    }

    private List<SettingItem> queryResultSettings(String conditionKey) {
        List<SettingItem> settingItems = new ArrayList<SettingItem>();
        int row = SettingKeys.getSettingId(conditionKey);
        int columnLen = mRuleMatrix[row].length;
        for (int column = 0; column < columnLen; column++) {
            ISettingRule rule = mRuleMatrix[row][column];
            if (rule != null) {
                SettingItem settingItem = getSettingItem(column);
                settingItems.add(settingItem);
            }
        }
        return settingItems;
    }

    private void gatherAffectedItems(List<SettingItem> affectItems, List<String> newAddKeys) {
        if (newAddKeys.size() == 0) {
            return;
        }

        List<String> newKeys = new ArrayList<String>();
        for (int i = 0; i < newAddKeys.size(); i++) {
            String key = newAddKeys.get(i);
            List<SettingItem> resultItems = queryResultSettings(key);
            for (int j = 0; j < resultItems.size(); j++) {
                SettingItem affectedItem = resultItems.get(j);
                if (!affectItems.contains(affectedItem)) {
                    affectItems.add(affectedItem);
                    newKeys.add(affectedItem.getKey());
                }
            }
        }
        gatherAffectedItems(affectItems, newKeys);
    }

    private void postResultToModule(List<SettingItem> affectedSettings,
            Map<String, String> beforeChanged) {
        final Map<String, String> afterChanged = new HashMap<String, String>();
        for (int i = 0; i < affectedSettings.size(); i++) {
            SettingItem affectedSetting = affectedSettings.get(i);
            String key = affectedSetting.getKey();
            String currentValue = affectedSetting.getValue();
            if (currentValue == null || currentValue.equals(beforeChanged.get(key))
                    || affectedSetting.getType() != SettingKeys.APPLY_TO_NATIVE) {
                continue;
            }
            afterChanged.put(key, currentValue);
        }
        LogHelper.d(TAG, "[postResultToModule], changedSettings:" + afterChanged.toString());
        // Callback result to the setting changed listeners.
        mSettingServantForBack.postResultToListeners(afterChanged);
        mSettingServantForFront.postResultToListeners(afterChanged);
        mSettingServantForAll.postResultToListeners(afterChanged);
    }

    private void postResultToUI(List<SettingItem> affectedSettings) {
        final Map<String, String> values = new HashMap<String, String>();
        final Map<String, String> overrideValues = new HashMap<String, String>();
        Map<String, String> resultToPrint = new HashMap<String, String>();
        for (int i = 0; i < affectedSettings.size(); i++) {
            SettingItem effectSetting = affectedSettings.get(i);
            if (!effectSetting.isEnable()) {
                continue;
            }
            String key = effectSetting.getKey();
            String value = effectSetting.getValue();
            String overrideValue = effectSetting.getOverrideValue();
            values.put(key, value);
            overrideValues.put(key, overrideValue);
            resultToPrint.put(key, value + "/" + overrideValue);
        }
        LogHelper.d(TAG, "[postResultToUI], override value:" + resultToPrint.toString());
        for (int i = 0; i < mISettingFilterListeners.size(); i++) {
            final ISettingFilterListener listener = mISettingFilterListeners.get(i);
            Handler handler = mSettingFilterHandler.get(listener);
            handler.post(new Runnable() {
                @Override
                public void run() {
                    LogHelper.d(TAG, "[postResultToUI], onFilterResult");
                    listener.onFilterResult(values, overrideValues);
                }
            });
        }
    }

    private SettingItem getSettingItem(int settingId) {
        String key = SettingKeys.getSettingKey(settingId);
        return getSettingItem(key);
    }

    private void upgradeOldVersion(SharedPreferences pref) {
        int version;
        try {
            version = pref.getInt(SettingKeys.KEY_VERSION, 0);
        } catch (Exception ex) {
            version = 0;
        }
        if (version == CURRENT_VERSION) return;

        SharedPreferences.Editor editor = pref.edit();
        if (version == 0) {
            // We won't use the preference which change in version 1.
            // So, just upgrade to version 1 directly
            version = 1;
        }
        if (version == 1) {
            // Change jpeg quality {65,75,85} to {normal,fine,superfine}
            String quality = pref.getString(SettingKeys.KEY_JPEG_QUALITY, "85");
            if (quality.equals("65")) {
                quality = "normal";
            } else if (quality.equals("75")) {
                quality = "fine";
            } else {
                quality = "superfine";
            }
            editor.putString(SettingKeys.KEY_JPEG_QUALITY, quality);
            version = 2;
        }
        if (version == 2) {
            editor.putString(SettingKeys.KEY_RECORD_LOCATION,
                    pref.getBoolean(SettingKeys.KEY_RECORD_LOCATION, false)
                    ? "on" : "none");
            version = 3;
        }
        if (version == 3) {
            // Just use video quality to replace it and
            // ignore the current settings.
            editor.remove("pref_camera_videoquality_key");
            editor.remove("pref_camera_video_duration_key");
        }

        editor.putInt(SettingKeys.KEY_VERSION, CURRENT_VERSION);
        editor.apply();
    }

    private boolean isGlobalPref(String key) {
        return SettingKeys.KEY_CAMERA_ID.equals(key)
                || SettingKeys.KEY_RECORD_LOCATION.equals(key)
                || SettingKeys.KEY_PHOTO_PIP.equals(key)
                || SettingKeys.KEY_VIDEO_PIP.equals(key);
    }

    private class SettingServant implements ISettingServant {
        private String                                                     mConcernedCamera = null;
        private SettingCtrl                                                mSettingCtrl;
        private LinkedList<ISettingServant.ISettingChangedListener>        mListeners =
                new LinkedList<ISettingServant.ISettingChangedListener>();
        private Map<ISettingServant.ISettingChangedListener, List<String>> mListenerConcern =
                new HashMap<ISettingServant.ISettingChangedListener, List<String>>();
        private Map<ISettingServant.ISettingChangedListener, Integer>      mListenerPriority =
                new HashMap<ISettingServant.ISettingChangedListener, Integer>();
        private Map<ISettingServant.ISettingChangedListener, Handler>      mListenerHandler =
                new HashMap<ISettingServant.ISettingChangedListener, Handler>();

        /**
         * Construct new instance, with concern camera id is null, means to concern all camera.
         */
        public SettingServant(SettingCtrl settingCtrl) {
            mSettingCtrl = settingCtrl;
        }

        /**
         * Construct new instance.
         * @param concernedCamera set the concerned camera id, only concern the camera that is set.
         */
        public SettingServant(SettingCtrl settingCtrl, String concernedCamera) {
            mSettingCtrl = settingCtrl;
            mConcernedCamera = concernedCamera;
        }

        @Override
        public void registerSettingChangedListener(
                ISettingServant.ISettingChangedListener listener,
                List<String> concernedSettings, int priority) {
            Looper looper = Looper.myLooper();
            Handler handler = null;
            if (looper != null) {
                handler = new Handler(looper);
            } else if ((looper = Looper.getMainLooper()) != null) {
                handler = new Handler(looper);
            } else {
                LogHelper.e(TAG, "[registerSettingChangedListener], the caller's looper is null. " +
                        "listener:" + listener);
            }

            registerSettingChangedListener(listener, concernedSettings, handler, priority);
        }


        @Override
        public void registerSettingChangedListener(
                ISettingServant.ISettingChangedListener listener,
                List<String> concernedSettings, Handler handler, int priority) {
            LogHelper.i(TAG, "[registerSettingChangedListener], listener:" + listener +
                    ", priority:" + priority);
            mListenerConcern.put(listener, concernedSettings);
            mListenerPriority.put(listener, priority);
            mListenerHandler.put(listener, handler);

            if (mListeners.contains(listener)) {
                return;
            }
            // rank the listener by descending order.
            if (mListeners.isEmpty()) {
                mListeners.add(listener);
            } else {
                int cursor = 0;
                ISettingServant.ISettingChangedListener cursorListener =
                        (ISettingServant.ISettingChangedListener) mListeners.getFirst();
                int size = mListeners.size();
                while (cursor < size &&  mListenerPriority.get(cursorListener) >= priority) {
                    cursor ++;
                    if (cursor < size) {
                        cursorListener = mListeners.get(cursor);
                    }
                }
                mListeners.add(cursor, listener);
            }
        }

        @Override
        public void unRegisterSettingChangedListener(
                ISettingServant.ISettingChangedListener listener) {
            if (mListeners.contains(listener)) {
                mListeners.remove(listener);
                mListenerConcern.remove(listener);
                mListenerPriority.remove(listener);
                mListenerHandler.remove(listener);
            }

        }

        @Override
        public String getCameraId() {
            if (mConcernedCamera == null) {
                return mCurrentCameraId;
            }
            return mConcernedCamera;
        }

        @Override
        public void doSettingChange(String key, String value, boolean saved) {
            mSettingCtrl.doSettingChange(key, value, saved);
        }

        @Override
        public String getSettingValue(String key) {
            String value = null;
            if (mConcernedCamera == null) {
                value = mSettingCtrl.getSettingValue(key);
            } else {
                value = mSettingCtrl.getSettingValue(key, mConcernedCamera);
            }
            return value;
        }

        @Override
        public SettingItem getSettingItem(String key) {
            SettingItem item = null;
            if (mConcernedCamera == null) {
                item = mSettingCtrl.getSettingItem(key);
            } else {
                item = mSettingCtrl.getSettingItem(key, mConcernedCamera);
            }
            return item;
        }

        @Override
        public Size getPreviewSize() {
            return mSettingCtrl.getPreviewSize(mConcernedCamera);
        }

        @Override
        public List<Size> getSupportedPreviewSizes() {
            SettingCharacteristics characteristic = mCharacteristicsMap.get(mConcernedCamera);
            List<Size> supportedSizes = characteristic.getSupportedPreviewSize();
            return supportedSizes;
        }

        @Override
        public String getSharedPreferencesValue(String key) {
            String value = null;
            if (mConcernedCamera == null) {
                value = mSettingCtrl.getSharePreferenceValue(key);
            } else {
                value = mSettingCtrl.getSharePreferenceValue(key, mConcernedCamera);
            }
            return value;
        }


        @Override
        public void setSharedPreferencesValue(String key, String value) {
            if (mConcernedCamera == null) {
                mSettingCtrl.setSharedPreferencesValue(key, value);
            } else {
                mSettingCtrl.setSharedPreferencesValue(key, value, mConcernedCamera);
            }
        }

        @Override
        public List<String> getSupportedValues(String key) {
            return mSettingCtrl.getSupportedValues(key, getCameraId());
        }

        public void postResultToListeners(final Map<String, String> changedSetting) {
            if (mConcernedCamera != null &&
                    !mConcernedCamera.equals(mCurrentCameraId) &&
                    !isIncludeCameraId(changedSetting)) {
                // Do not need to post result to the listeners which do not concern current camera
                // except result contains camera id setting.
                LogHelper.i(TAG, "do not need post result to listeners, mConcernedCamera:" +
                        "" + mConcernedCamera + ", mCurrentCameraId:" + mCurrentCameraId);
                return;
            }

            List<ISettingServant.ISettingChangedListener> listeners =
                    new ArrayList<ISettingServant.ISettingChangedListener>();
            for (int i = 0; i < mListeners.size(); i++) {
                listeners.add(mListeners.get(i));
            }

            for (int i = 0; i < listeners.size(); i++) {
                final ISettingServant.ISettingChangedListener listener = listeners.get(i);
                Handler handler = mListenerHandler.get(listener);
                if (handler != null) {
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            postResultToListener(listener, changedSetting);
                        }
                    });
                } else {
                    postResultToListener(listener, changedSetting);
                }
            }
        }

        private boolean isIncludeCameraId(Map<String, String> changedSetting) {
            if (changedSetting == null) {
                return false;
            }
            return changedSetting.containsKey(SettingKeys.KEY_CAMERA_ID);
        }

        private void postResultToListener(ISettingServant.ISettingChangedListener listener,
                Map<String, String> changedSetting) {
            Map<String, String> result = new HashMap<String, String>();
            List<String> concernedSettings = mListenerConcern.get(listener);
            if (concernedSettings == null && !changedSetting.isEmpty()) {
                LogHelper.i(TAG, "[postResultToListener], all result:" +
                        changedSetting.toString() + ", " +
                        "listener:" + listener);
                listener.onSettingChanged(changedSetting);
                return;
            }

            Set<String> set = changedSetting.keySet();
            Iterator<String> iterator = set.iterator();
            while (iterator.hasNext()) {
                String key = iterator.next();
                if (concernedSettings != null && concernedSettings.contains(key)) {
                    result.put(key, changedSetting.get(key));
                }
            }
            if (!result.isEmpty()) {
                LogHelper.i(TAG, "[postResultToListener], part result:" + result.toString() + ", " +
                        "listener:" + listener);
                listener.onSettingChanged(result);
            }
        }
    }
}
