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

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.util.SettingKeys;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SettingGenerator {
    private static final Tag TAG = new Tag(SettingGenerator.class.getSimpleName());
    private Map<String, List<SettingItem>>
    mSettingItemMap = new HashMap<String, List<SettingItem>>();
    private String mCurrentCameraId = "0";

    public SettingGenerator() {

    }

    public void initializeSettingItem(String[] keys, String[] cameraIds,
            Map<String, SettingCharacteristics> characteristics) {
        for (String cameraId : cameraIds) {
            List<SettingItem> items = new ArrayList<SettingItem>();
            SettingCharacteristics characteristic = characteristics.get(cameraId);
            for (String key : keys) {
                SettingItem item = new SettingItem(key);
                items.add(item);
                // Set SettingItem object default value and last value.
                int settingId = SettingKeys.getSettingId(key);
                item.setSettingId(settingId);
                String defaultValue = SettingDataBase.getDefaultValue(settingId);
                item.setDefaultValue(defaultValue);
                item.setLastValue(defaultValue);
                int type = SettingKeys.getSettingType(settingId);
                item.setType(type);
                List<String> supportedValues = characteristic.getSupportedValues(key);
                if (supportedValues == null && type == SettingKeys.APPLY_TO_NATIVE) {
                    item.setEnable(false);
                } else {
                    LogHelper.i(TAG, "SettingItem:" + item.toString());
                }
            }
            mSettingItemMap.put(cameraId, items);
            // capture mode setting is virtually set for setting logic, it do not used to show on
            // UI or set to native. Its default value is normal.
            SettingItem captureModeItem = getSettingItem(SettingKeys.KEY_CAPTURE_MODE, cameraId);
            captureModeItem.setValue("normal");
        }
    }

    public void updateCameraId(String cameraId) {
        LogHelper.i(TAG, "[updateCameraId], cameraId:" + cameraId + ", " +
                "mCurrentCameraId:" + mCurrentCameraId);
        if (mCurrentCameraId != null && mCurrentCameraId.equals(cameraId)) {
            return;
        }
        mCurrentCameraId = cameraId;
    }

    /**
     * Configure the setting items' last value for the special camera.
     * @param cameraId The id of camera.
     */
    public void configureSettingItems(String cameraId) {
        List<SettingItem> items = mSettingItemMap.get(cameraId);
        for (int i = 0; i < items.size(); i++) {
            SettingItem item = items.get(i);
            item.setLastValue(item.getDefaultValue());
            item.setOverrideValue(null);
            item.clearAllOverrideRecord();
        }
    }

    public SettingItem getSettingItem(String key) {
        return getSettingItem(key, mCurrentCameraId);
    }

    public SettingItem getSettingItem(String key, String cameraId) {
        if (key == null) {
            LogHelper.w(TAG, "the input key is null, return null.");
            return null;
        }
        List<SettingItem> items = mSettingItemMap.get(cameraId);
        for (int i = 0; i < items.size(); i++) {
            SettingItem item = items.get(i);
            if (key.equals(item.getKey())) {
                return item;
            }
        }
        LogHelper.w(TAG, "key:" + key + ", setting item return null");
        return null;
    }
}
