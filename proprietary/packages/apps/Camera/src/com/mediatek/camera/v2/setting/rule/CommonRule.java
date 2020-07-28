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
package com.mediatek.camera.v2.setting.rule;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.setting.ISettingRule;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.setting.SettingItem;
import com.mediatek.camera.v2.setting.SettingItem.Record;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import java.util.ArrayList;
import java.util.List;

public class CommonRule implements ISettingRule {
    private static final Tag TAG = new Tag(CommonRule.class.getSimpleName());
    private String mConditionKey;
    private String mResultKey;

    private SettingItem mConditionSetting;
    private SettingItem mResultSetting;

    private List<String> mConditions = new ArrayList<String>();
    private List<List<String>> mResults = new ArrayList<List<String>>();
    private SettingCtrl mSettingCtrl;


    public CommonRule(String conditionKey, String resultKey, SettingCtrl settingCtrl) {
        mConditionKey = conditionKey;
        mResultKey = resultKey;
        mSettingCtrl = settingCtrl;
    }

    @Override
    public void execute() {
        mConditionSetting = mSettingCtrl.getSettingItem(mConditionKey);
        mResultSetting = mSettingCtrl.getSettingItem(mResultKey);

        String conditionSettingValue = mConditionSetting.getValue();
        int index = conditionSatisfied(conditionSettingValue);
        String resultValue = mResultSetting.getValue();
        // if index is equal with -1, means no condition is satisfied.
        LogHelper.i(TAG, "[execute], conditionSetting:" + mConditionKey + ", conditionValue:" +
                "" + conditionSettingValue + ", resultSetting:" + mResultKey + ", " +
                "resultSettingValue:" + resultValue + ", index = " + index);
        if (index == -1) {
            Record record = mResultSetting.getOverrideRecord(mConditionKey);
            if (record == null) {
                LogHelper.i(TAG, "[execute], no override record, return");
                return;
            }
            String overrideValue = null;
            mResultSetting.removeOverrideRecord(mConditionKey);
            int overrideCount = mResultSetting.getOverrideCount();
            if (overrideCount > 0) {
                Record topRecord = mResultSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    resultValue = topRecord.getValue();
                    overrideValue = topRecord.getOverrideValue();
                }
            } else {
                resultValue = mSettingCtrl.getSharePreferenceValue(mResultKey);
                if (resultValue == null) {
                    resultValue = mResultSetting.getDefaultValue();
                }
            }
            mResultSetting.setValue(resultValue);
            mResultSetting.setOverrideValue(overrideValue);
            LogHelper.i(TAG, "[execute], result: value = " + resultValue + ", " +
                    "overrideValue =" + overrideValue);
        } else {
            List<String> resultValues = mResults.get(index);
            LogHelper.i(TAG, "[execute], resultValues:" + resultValues);
            String overrideValue = null;
            if (resultValues != null && resultValues.size() == 1
                    && Utils.RESET_STATE_VALUE_DISABLE.equals(resultValues.get(0))) {
                overrideValue = Utils.RESET_STATE_VALUE_DISABLE;
                resultValue = mResultSetting.getValue();
            } else {
                // filter values with native supported values.
                List<String> resultValuesAfterFilter = filterUnsupportedValue(
                        resultValues, mResultKey);
                if (resultValuesAfterFilter.size() == 0) {
                    LogHelper.i(TAG, "[execute], resultValuesAfterFilter is null");
                    return;
                }
                // just get the first value temporary
                resultValue = getResultSettingValue(resultValuesAfterFilter, index);
                if (resultValuesAfterFilter.size() == 1) {
                    overrideValue = resultValue;
                } else {
                    String[] values = new String[resultValuesAfterFilter.size()];
                    overrideValue = Utils.buildEnableList(resultValuesAfterFilter.toArray(values));
                }
            }

            mResultSetting.setValue(resultValue);
            mResultSetting.setOverrideValue(overrideValue);

            Record record = mResultSetting.new Record(resultValue, overrideValue);
            mResultSetting.addOverrideRecord(mConditionKey, record);
            LogHelper.i(TAG, "[execute], result: value = " + resultValue + ", " +
                    "overrideValue =" + overrideValue);
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result) {
        mConditions.add(condition);
        mResults.add(result);
    }

    private int conditionSatisfied(String conditionValue) {
        int index = mConditions.indexOf(conditionValue);
        return index;
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
        String value = mResultSetting.getValue();
        if (!resultValues.contains(value)) {
            if (SettingKeys.KEY_PICTURE_RATIO.equals(mConditionKey)
                    && SettingKeys.KEY_PICTURE_SIZE.equals(mResultKey)) {
                // Always choose the max picture, the picture size is in ascending order.
                value = resultValues.get(resultValues.size() - 1);
                mSettingCtrl.setSharedPreferencesValue(SettingKeys.KEY_PICTURE_SIZE, value);
            } else {
                value = resultValues.get(0);
            }
        }
        return value;
    }

    private List<String> filterUnsupportedValue(List<String> resultValues, String key) {
        if (SettingKeys.DO_NOT_APPLY_TO_NATIVE == SettingKeys.getSettingType(key)) {
            return resultValues;
        }
        List<String> nativeSupportedValues = mSettingCtrl.getSupportedValues(key);
        List<String> afterFilterValues = new ArrayList<String>();
        if (nativeSupportedValues == null) {
            return afterFilterValues;
        }

        for (String value : resultValues) {
            if (nativeSupportedValues.contains(value)) {
                afterFilterValues.add(value);
            }
        }
        return afterFilterValues;
    }

}
