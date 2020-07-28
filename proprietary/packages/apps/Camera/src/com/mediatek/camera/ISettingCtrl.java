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
package com.mediatek.camera;

import android.content.SharedPreferences;

import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.setting.preference.PreferenceGroup;

public interface ISettingCtrl {

    /**
     *
     * @param preferenceRes
     */
    public void initializeSettings(int preferenceRes, SharedPreferences globalPref,
            SharedPreferences localPref);

    public boolean isSettingsInitialized();

    /**
     * Update setting
     */
    public void updateSetting(SharedPreferences localPref);

    /**
     * This method will be called when setting changed.
     * @param key the key value of current setting.
     * @param value the value of current setting will be set.
     */
    public void onSettingChanged(String key, String value);

    /**
     * Add rule to rule matrix.
     * @param conditionKey the key of condition setting.
     * @param resultKey the key of result setting.
     * @param rule the rule between condition setting and result setting.
     */
    public void addRule(String conditionKey, String resultKey, ISettingRule rule);

    /**
     * Execute rule between condition setting and result setting.
     * @param conditionKey the key of condition setting.
     * @param resultKey the key of result setting.
     */
    public void executeRule(String conditionKey, String resultKey);

    /**
     * Get the setting current value.
     * @param key the key value of setting
     * @return the current value of setting
     */
    public String getSettingValue(String key);

    /**
     * Get the setting default value.
     * @param key the key value of setting
     * @return the default value of setting
     */
    public String getDefaultValue(String key);

    /**
     * Get the setting object.
     * @param key the key value of setting
     * @return return setting object
     */
    public SettingItem getSetting(String key);

    /**
     * Get the setting object.
     * @param key the key value of setting
     * @param cameraId the camera id
     * @return return setting object
     */
    public SettingItem getSetting(String key, int cameraId);

    /**
     * Get the ListPreference field of setting
     * @param key the key value of setting
     * @return ListPreference field value of setting or null if this setting has
     *         no ListPreference
     */
    public ListPreference getListPreference(String key);

    /**
     * Set the setting value
     * @param key the key of setting
     * @param value the value of setting
     * @param cameraId the id of camera
     */
    public void setSettingValue(String key, String value, int cameraId);

    /**
     * Get preference group
     * @return preference group
     */
    public PreferenceGroup getPreferenceGroup();

    /**
     * Restore setting related to camera
     * @param cameraId the id of camera
     */
    public void restoreSetting(int cameraId);

    /**
     * Get the camera mode for capture mode
     * @param modeKey the key of capture mode
     * @return the camera mode
     */
    public String getCameraMode(String modeKey);

    /**
     * Rest the setting
     */
    public void resetSetting();
}
