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

import android.os.Handler;
import android.util.Size;

import java.util.List;
import java.util.Map;

public interface ISettingServant {

    public interface ISettingChangedListener {
        public static final int LOW_PRIORITY      = 0;
        public static final int MIDDLE_PRIORITY   = 1;
        public static final int HIGH_PRIORITY     = 2;
        public void onSettingChanged(Map<String, String> result);
    }

    /**
     * Register listener to monitor the changed settings.
     * @param listener
     * @param concernedSettings The key of setting listener wants to listen,
     *        if it is null means that all the setting the listener wants to listen.
     * @param priority The priority of this listener.
     */
    public void registerSettingChangedListener(ISettingChangedListener listener,
            List<String> concernedSettings, int priority);

    /**
     * Register listener to monitor the changed settings.
     * @param listener
     * @param concernedSettings The key of setting listener wants to listen,
     *        if it is null means that all the setting the listener wants to listen.
     * @param handler The setting result will be post to this handler.
     * @param priority The priority of this listener.
     */
    public void registerSettingChangedListener(ISettingChangedListener listener,
            List<String> concernedSettings, Handler handler, int priority);

    /**
     * Remove the listener do not to monitor the changed settings.
     * @param listener
     */
    public void unRegisterSettingChangedListener(ISettingChangedListener listener);

    public String getCameraId();
    /**
     * This method will be called when setting changed.
     * @param key the key value of current setting, refer to SettingKeys.
     * @param value the value of current setting will be set.
     * @param saved whether save value to shared preference or not.
     */
    public void doSettingChange(String key, String value, boolean saved);

    /**
     * Get setting value
     * @param Key the key value of setting, refer to SettingKeys.
     * @return Value of setting
     */
    public String getSettingValue(String Key);

    /**
     * Get the SettingItem object.
     * @param key the key value of setting
     * @return return setting object
     */
    public SettingItem getSettingItem(String key);

    public Size getPreviewSize();

    /**
     * Get supported preview sizes.
     * @return supported preview sizes.
     */
    public List<Size> getSupportedPreviewSizes();

    /**
     * Get value of setting in SharedPreferences.
     * @param key The key value of setting, refer to SettingKeys.
     * @return Value of setting in SharedPreferences.
     */
    public String getSharedPreferencesValue(String key);

    /**
     * Set value of setting in SharedPreferences.
     * @param key The key value of setting, refer to SettingKeys.
     * @param value Value of setting want to set in SharedPreferences.
     */
    public void setSharedPreferencesValue(String key, String value);

    /**
     * Get supported values.
     * @param key the key of setting.
     * @return supported values.
     */
    public List<String> getSupportedValues(String key);
}
