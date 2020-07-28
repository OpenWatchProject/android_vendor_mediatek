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
package com.mediatek.camera.setting.rule;

import android.hardware.Camera.Size;

import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingGenerator;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class CommonRule implements ISettingRule {
    private static final String TAG = "CommonRule";

    private boolean mRestoreSupported = true;

    private String mConditionKey;
    private String mResultKey;

    private SettingItem mConditionSetting;
    private SettingItem mResultSetting;

    private List<String> mConditions = new ArrayList<String>();
    private List<List<String>> mResults = new ArrayList<List<String>>();
    private List<MappingFinder> mMappingFinder = new ArrayList<MappingFinder>();
    private SettingGenerator mSettingGenerator;

    private ICameraDeviceManager mICameraDeviceManager;

    public CommonRule(String conditionKey, String resultKey,
            ICameraDeviceManager cameraDeviceManager, SettingGenerator settingGenerator) {
        mConditionKey = conditionKey;
        mResultKey = resultKey;
        mICameraDeviceManager = cameraDeviceManager;
        mSettingGenerator = settingGenerator;
    }

    @Override
    public void execute() {
        mConditionSetting = mSettingGenerator.getSettingItem(mConditionKey);
        mResultSetting = mSettingGenerator.getSettingItem(mResultKey);

        String conditionSettingValue = mConditionSetting.getValue();
        int index = conditionSatisfied(conditionSettingValue);
        String resultValue = mResultSetting.getValue();
        int type = mResultSetting.getType();
        // if index is equal with -1, means no condition is satisfied.
//        Log.i(TAG, "[execute], conditionSetting:" + mConditionKey + ", conditionValue:" +
//                "" + conditionSettingValue + ", resultSetting:" + mResultKey + ", " +
//                "resultSettingValue:" + resultValue + ", index = " + index);
        if (index == -1) {
            int overrideCount = mResultSetting.getOverrideCount();
            Record record = mResultSetting.getOverrideRecord(mConditionKey);
            if (record == null) {
                return;
            }
//            Log.i(TAG, "overrideCount:" + overrideCount);
            mResultSetting.removeOverrideRecord(mConditionKey);
            overrideCount--;
            if (overrideCount > 0) {
                Record topRecord = mResultSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    if (mResultSetting.isEnable()) {
                        String value = topRecord.getValue();
                        String overrideValue = topRecord.getOverrideValue();
                        // may be the setting's value is changed, the value in record is old.
                        ListPreference pref = mResultSetting.getListPreference();
                        if (pref != null && SettingUtils.isBuiltList(overrideValue)) {
                            pref.setEnabled(true);

                            String prefValue =  pref.getValue();
                            List<String> list = SettingUtils.getEnabledList(overrideValue);
                            if (list.contains(prefValue)) {
                                if (!prefValue.equals(value)) {
                                    String[] values = new String[list.size()];
                                    overrideValue = SettingUtils.buildEnableList(
                                            list.toArray(values), prefValue);
                                }
                                value = prefValue;
                            }
                        }
                        setResultSettingValue(type, value, overrideValue, mRestoreSupported);
                    }
                }
            } else {
                ListPreference pref = null;
                switch (type) {
                case SettingConstants.NEITHER_IN_PARAMETER_NOR_IN_PREFERENCE:
                case SettingConstants.ONLY_IN_PARAMETER:
                    resultValue = mResultSetting.getDefaultValue();
                    break;

                case SettingConstants.ONLY_IN_PEFERENCE:
                case SettingConstants.BOTH_IN_PARAMETER_AND_PREFERENCE:
                    pref = mResultSetting.getListPreference();
                    if (pref != null) {
                        resultValue = pref.getValue();
                    }
                    break;

                default:
                    break;
                }

                String overrideValue = null;
                if (mResultSetting.isEnable()) {
                    if (mRestoreSupported == false) {
                        if (pref != null) {
                            pref.setEnabled(true);
                        }
                        mRestoreSupported = true;
                    } else {
                        setResultSettingValue(type, resultValue, overrideValue, mRestoreSupported);
                    }
                }
            }
        } else {
            List<String> resultValues = mResults.get(index);
            List<String> resultValuesAfterFilter = filterUnsupportedValue(resultValues, mResultKey);
            // just get the first value temporary
            resultValue = getResultSettingValue(resultValuesAfterFilter, index);
            String overrideValue = null;
            if (SettingUtils.RESET_STATE_VALUE_DISABLE.equals(resultValue)) {
                overrideValue = SettingUtils.RESET_STATE_VALUE_DISABLE;
                mRestoreSupported = false;
                resultValue = mResultSetting.getValue();
            } else if (resultValues.size() <= 1) {
                overrideValue = resultValue;
                mRestoreSupported = true;
            } else {
                String[] values = new String[resultValuesAfterFilter.size()];
                overrideValue = SettingUtils.buildEnableList(
                        resultValuesAfterFilter.toArray(values), resultValue);
                mRestoreSupported = true;
            }
            if (mResultSetting.isEnable()) {
                setResultSettingValue(type, resultValue, overrideValue, true);
            }

            Record record = mResultSetting.new Record(resultValue, overrideValue);
            mResultSetting.addOverrideRecord(mConditionKey, record);
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result, MappingFinder mappingFinder) {
        mConditions.add(condition);
        mResults.add(result);
        mMappingFinder.add(mappingFinder);
    }

    private int conditionSatisfied(String conditionValue) {
        int index = mConditions.indexOf(conditionValue);
        return index;
    }

    /**
     * Set result setting value.
     *
     * @param settingType
     *            setting type.
     * @param value
     *            setting value.
     * @param overrideValue
     *            setting override value.
     */
    private void setResultSettingValue(int settingType, String value,
            String overrideValue, boolean restoreSupported) {
//        Log.d(TAG, "[setResultSettingValue]settingType:" + settingType + ", value:" + value
//                + ",overrideValue = " + overrideValue + ", restoreSupported:" + restoreSupported);
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(currentCameraId);
        Parameters parameters = cameraDevice.getParameters();
        mResultSetting.setValue(value);
        switch (settingType) {
        case SettingConstants.NEITHER_IN_PARAMETER_NOR_IN_PREFERENCE:
            break;

        case SettingConstants.ONLY_IN_PARAMETER:
            ParametersHelper.setParametersValue(parameters, currentCameraId,
                    mResultSetting.getKey(), value);
            break;

        case SettingConstants.ONLY_IN_PEFERENCE:
            if (SettingUtils.RESET_STATE_VALUE_DISABLE.equals(overrideValue)) {
                mResultSetting.getListPreference().setEnabled(false);
                } else {
                    mResultSetting.getListPreference().setOverrideValue(overrideValue,
                            restoreSupported);
                }

            break;

        case SettingConstants.BOTH_IN_PARAMETER_AND_PREFERENCE:
            ListPreference pref = mResultSetting.getListPreference();
            if (SettingUtils.RESET_STATE_VALUE_DISABLE.equals(overrideValue)) {
                if (pref != null) {
                    pref.setEnabled(false);
                }
            } else {
                if (pref != null) {
                    if (mResultKey.equals(SettingConstants.KEY_FLASH)) {
                        pref.setOverrideValue(overrideValue, false);
                    } else {
                        pref.setOverrideValue(overrideValue, restoreSupported);
                    }

                }
                ParametersHelper.setParametersValue(parameters, currentCameraId,
                        mResultSetting.getKey(), value);
            }
            break;

        default:
            break;
        }
    }

    /**
     * Get result setting value.
     *
     * @param resultValues
     *            provides values for result setting to select.
     * @param index
     *            use to get MappingFinder instance.
     * @return result setting value.
     */
    private String getResultSettingValue(List<String> resultValues, int index) {
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(mICameraDeviceManager
                .getCurrentCameraId());
        String value = mResultSetting.getValue();
        if (cameraDevice == null) {
            return value;
        }
        Parameters parameters = cameraDevice.getParameters();
        if (value == null) {
            value = ParametersHelper.getParametersValue(parameters, mResultSetting.getKey());
        }
        MappingFinder mappingFinder = mMappingFinder.get(index);
        if (mappingFinder != null) {
            value = mappingFinder.find(value, resultValues);
        } else {
            // if result values contain current value of result setting. If it
            // is not contained, set
            // result setting value as first element of result values.
            if (!resultValues.contains(value)) {
                value = resultValues.get(0);
            }
        }
        return value;
    }

    private List<String> filterUnsupportedValue(List<String> resultValues, String key) {
        if (resultValues.size() == 1
                && SettingUtils.RESET_STATE_VALUE_DISABLE.equals(resultValues.get(0))) {
            return resultValues;
        }

        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(mICameraDeviceManager
                .getCurrentCameraId());
        Parameters parameters = cameraDevice.getParameters();
        List<String> supportedValues = new ArrayList<String>();
        if (key.equals(SettingConstants.KEY_PICTURE_SIZE)) {
            List<String> supportedPictureSizes = sizeListToStringList(parameters
                    .getSupportedPictureSizes());
            int limitedResolution = SettingUtils.getLimitResolution();
            if (limitedResolution > 0) {
                SettingUtils.filterLimitResolution(supportedPictureSizes);
            }
            int len = resultValues.size();
            for (int i = 0; i < len; i++) {
                if (supportedPictureSizes.contains(resultValues.get(i))) {
                    supportedValues.add(resultValues.get(i));
                }
            }
        } else {
            supportedValues = resultValues;
        }
       return supportedValues;
    }

    private List<String> sizeListToStringList(List<Size> sizes) {
        ArrayList<String> list = new ArrayList<String>();
        for (Size size : sizes) {
            list.add(String.format(Locale.ENGLISH, "%dx%d", size.width, size.height));
        }
        return list;
    }
}
