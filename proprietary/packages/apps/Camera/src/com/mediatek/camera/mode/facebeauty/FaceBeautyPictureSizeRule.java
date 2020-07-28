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
package com.mediatek.camera.mode.facebeauty;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.List;

public class FaceBeautyPictureSizeRule implements ISettingRule {

    private static final String TAG = "FaceBeautyPictureSizeRule";
    private long PICTURE_SIZE_4M = 4000000L;

    private List<String> mConditions = new ArrayList<String>();

    private ICameraDeviceManager mICameraDeviceManager;
    private ICameraContext mICameraContext;
    private ISettingCtrl mISettingCtrl;
    private SettingItem pictureSetting;

    public FaceBeautyPictureSizeRule(ISettingCtrl settingCtrl,
            ICameraContext cameraContext) {
        mISettingCtrl = settingCtrl;
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
        mICameraContext = cameraContext;
    }
    // this rule only use for ram optimize project
    @Override
    public void execute() {
        if (!mICameraContext.getFeatureConfig().isLowRamOptSupport()) {
            return;
        }
        pictureSetting = mISettingCtrl.getSetting(SettingConstants.KEY_PICTURE_SIZE);
        String value1 = mISettingCtrl.getSettingValue(SettingConstants.KEY_FACE_BEAUTY);
        String resultValue = pictureSetting.getValue();
        int type = pictureSetting.getType();
        int index = mConditions.indexOf(value1);
        Log.i(TAG, "[execute],index = " + index);
        // run ram optimize rule to limit picture size
        if (index != -1) {

            ListPreference pref = pictureSetting.getListPreference();
            CharSequence[] entryValues = pref.getEntryValues();
            List<String> overValues = new ArrayList<String>();
            String near4MSize = null;
            long near4M = 0L;
            int indexNum;
            int width;
            int height;
            // here get entry values from listPreference and then remove size max 4M
            for (int i = 0; i < entryValues.length; i++) {
                indexNum = entryValues[i].toString().indexOf('x');
                width = Integer.parseInt(entryValues[i].toString().substring(0, indexNum));
                height = Integer.parseInt(entryValues[i].toString().substring(indexNum + 1));
                if (PICTURE_SIZE_4M >= width * height) {
                    // remember the maximum size which is not bigger 4M
                    if (near4M < width * height) {
                        near4M = width * height;
                        near4MSize = "" + width + "x" + height;
                    }
                    overValues.add("" + width + "x" + height);
                }
            }
            // if resultValue is not bigger than 4M use it or use near4MSize
            if (0 > overValues.indexOf(resultValue)) {
                resultValue = near4MSize;
            }

            String[] values = new String[overValues.size()];
            String overrideValue = SettingUtils.buildEnableList(overValues.toArray(values),
                    resultValue);

            if (pictureSetting.isEnable()) {
                setResultSettingValue(type, resultValue, overrideValue, true, pictureSetting);
            }

            Record record = pictureSetting.new Record(resultValue, overrideValue);
            pictureSetting.addOverrideRecord(SettingConstants.KEY_FACE_BEAUTY, record);

        } else if (index == -1) {
            // restore picture size after set face beauty off
            int overrideCount = pictureSetting.getOverrideCount();
            Record record = pictureSetting.getOverrideRecord(SettingConstants.KEY_FACE_BEAUTY);
            if (record == null) {
                return;
            }
            Log.i(TAG, "overrideCount:" + overrideCount);
            pictureSetting.removeOverrideRecord(SettingConstants.KEY_FACE_BEAUTY);
            overrideCount--;

            if (overrideCount > 0) {
                Record topRecord = pictureSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    if (pictureSetting.isEnable()) {
                        String value = topRecord.getValue();
                        String overrideValue = topRecord.getOverrideValue();
                        // may be the setting's value is changed, the value in
                        // record is old.
                        ListPreference pref = pictureSetting.getListPreference();
                        if (pref != null
                                && SettingUtils.isBuiltList(overrideValue)) {
                            pref.setEnabled(true);
                            String prefValue = pref.getValue();
                            List<String> list = SettingUtils
                                    .getEnabledList(overrideValue);
                            if (list.contains(prefValue)) {
                                if (!prefValue.equals(value)) {
                                    String[] values = new String[list.size()];
                                    overrideValue = SettingUtils
                                            .buildEnableList(
                                                    list.toArray(values),
                                                    prefValue);
                                }
                                value = prefValue;
                            }
                        }
                        setResultSettingValue(type, value, overrideValue, true,
                                pictureSetting);
                    }
                }
            } else {
                mISettingCtrl.executeRule(SettingConstants.KEY_PICTURE_RATIO,
                        SettingConstants.KEY_PICTURE_SIZE);
            }
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result,
            MappingFinder mappingFinder) {
        mConditions.add(condition);
    }

    private void setResultSettingValue(int settingType, String value,
            String overrideValue, boolean restoreSupported, SettingItem item) {

        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(currentCameraId);
        Parameters parameters = cameraDevice.getParameters();
        item.setValue(value);
        ListPreference pref = item.getListPreference();

        if (SettingUtils.RESET_STATE_VALUE_DISABLE.equals(overrideValue)) {
            if (pref != null) {
                pref.setEnabled(false);
            }
        } else {
            if (pref != null) {
                pref.setOverrideValue(overrideValue, restoreSupported);
            }
            ParametersHelper.setParametersValue(parameters, currentCameraId,
                    item.getKey(), value);
        }
    }
}