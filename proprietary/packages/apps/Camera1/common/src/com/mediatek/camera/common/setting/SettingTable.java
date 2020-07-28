/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.setting;

import com.google.common.collect.ArrayListMultimap;
import com.google.common.collect.Multimap;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.setting.ICameraSetting.SettingType;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

/**
 * This class is used to record using setting.
 */
public class SettingTable {
    private static final Tag TAG = new Tag(SettingTable.class.getSimpleName());
    private final ConcurrentHashMap<String, ICameraSetting> mSettingsByKey =
            new ConcurrentHashMap<>();
    private final Multimap<SettingType, ICameraSetting> mSettingListByType =
            ArrayListMultimap.create();
    private final ConcurrentHashMap<String, ICameraSetting> mConfigParametersGroup =
            new ConcurrentHashMap<>();
    private final ConcurrentHashMap<String, ICameraSetting> mCaptureRequestGroup =
            new ConcurrentHashMap<>();
    private final ConcurrentHashMap<String, ICameraSetting> mPreviewStatusGroup =
            new ConcurrentHashMap<>();

    private final Object mSettingsByTypeSync = new Object();

    /**
     * Add a setting to setting table.
     *
     * @param setting The instance
     */
    public void add(ICameraSetting setting) {
        if (setting == null) {
            LogHelper.w(TAG, "[add] why pass NULL setting!!!!!!");
            return;
        }
        if (setting.getKey() == null || setting.getSettingType() == null) {
            throw new IllegalArgumentException("[SettingTable.add] setting:" + setting +
                    ",Please check why you return NULL setting key or setting type!!!");
        }

        mSettingsByKey.put(setting.getKey(), setting);
    }

    /**
     * Appends all of the ICameraSettings in the collections.
     * @param settings collection containing ICameraSettings to be added.
     */
    public void addAll(List<ICameraSetting> settings) {
        for (ICameraSetting setting: settings) {
            add(setting);
        }
    }

    /**
     * Classify settings to different groups.
     *
     * @param cameraApi The camera api.
     */
    public void classify(CameraDeviceManagerFactory.CameraApi cameraApi) {
        for (ICameraSetting setting : mSettingsByKey.values()) {
            addSettingByTypeSync(setting);
            if (CameraDeviceManagerFactory.CameraApi.API1 == cameraApi) {
                if (setting.getParametersConfigure() != null) {
                    mConfigParametersGroup.put(setting.getKey(), setting);
                }
            } else if (CameraDeviceManagerFactory.CameraApi.API2 == cameraApi) {
                if (setting.getCaptureRequestConfigure() != null) {
                    mCaptureRequestGroup.put(setting.getKey(), setting);
                }
            }

            if (setting.getPreviewStateCallback() != null) {
                mPreviewStatusGroup.put(setting.getKey(), setting);
            }
        }
    }
    /**
     * Remove a setting from setting table.
     *
     * @param setting The instance.
     */
    public void remove(ICameraSetting setting) {
        mSettingsByKey.remove(setting.getKey());
        removeSettingByTypeSync(setting);
    }

    /**
     * Removes all ICameraSettings in key collection and type collection.
     */
    public void removeAll() {
        mSettingListByType.clear();
        mSettingsByKey.clear();
        mConfigParametersGroup.clear();
        mCaptureRequestGroup.clear();
        mPreviewStatusGroup.clear();
    }

    /**
     * Get a setting indicated by the input key from setting table.
     * @param key The string used to indicate the setting.
     * @return If the setting indicated by the input key in the setting table, then
     *     it, otherwise, return null.
     */
    public ICameraSetting get(String key) {
        return mSettingsByKey.get(key);
    }

    /**
     * Get a setting indicated by the input key from ConfigParameters setting table.
     *
     * @param key The string used to indicate the setting.
     * @return an instance of ICameraSetting indicated by the input key.
     */
    public ICameraSetting getConfigParameterSetting(String key) {
        return mConfigParametersGroup.get(key);
    }

    /**
     * Get a setting indicated by the input key from capture request setting table.
     *
     * @param key The string used to indicate the setting.
     * @return an instance of ICameraSetting indicated by the input key.
     */
    public ICameraSetting getCaptureRequestSetting(String key) {
        return mCaptureRequestGroup.get(key);
    }

    /**
     * Get all ICameraSetting instances.
     * @return all ICameraSetting instances.
     */
    public List<ICameraSetting> getAllSettings() {
        ArrayList<ICameraSetting> settings = new ArrayList<>();
        settings.addAll(mSettingsByKey.values());
        return settings;
    }

    /**
     * Get all ICameraSetting instances in config parameters setting table.
     *
     * @return all ICameraSetting instances.
     */
    public List<ICameraSetting> getAllConfigParametersSettings() {
        ArrayList<ICameraSetting> settings = new ArrayList<>();
        settings.addAll(mConfigParametersGroup.values());
        return settings;
    }

    /**
     * Get all camera settings in capture quest setting table.
     *
     * @return All the settings has capture request configure.
     */
    public List<ICameraSetting> getAllCaptureRequestSettings() {
        ArrayList<ICameraSetting> settings = new ArrayList<>();
        settings.addAll(mCaptureRequestGroup.values());
        return settings;
    }

    /**
     * Get all ICameraSetting instances in preview status setting table.
     *
     * @return all ICameraSetting instances.
     */
    public List<ICameraSetting> getAllPreviewStatusSettings() {
        ArrayList<ICameraSetting> settings = new ArrayList<>();
        settings.addAll(mPreviewStatusGroup.values());
        return settings;
    }

    /**
     * Get the list of settings by {@link SettingType}.
     *
     * @param settingType want to get which setting type's ICameraSetting.
     * @return a list of settings.
     */
    public ArrayList<ICameraSetting> getSettingListByType(SettingType settingType) {
        ArrayList<ICameraSetting> settings = new ArrayList<>();
        synchronized (mSettingsByTypeSync) {
            switch (settingType) {
                case PHOTO:
                    settings.addAll(mSettingListByType.get(SettingType.PHOTO));
                    settings.addAll(mSettingListByType.get(SettingType.PHOTO_AND_VIDEO));
                    break;

                case VIDEO:
                    settings.addAll(mSettingListByType.get(SettingType.VIDEO));
                    settings.addAll(mSettingListByType.get(SettingType.PHOTO_AND_VIDEO));
                    break;

                case PHOTO_AND_VIDEO:
                    settings.addAll(mSettingListByType.get(SettingType.PHOTO_AND_VIDEO));
                    break;

                default:
                    break;
            }
            return settings;
        }
    }

    private void addSettingByTypeSync(ICameraSetting setting) {
        synchronized (mSettingsByTypeSync) {
            if (!mSettingListByType.containsValue(setting)) {
                mSettingListByType.put(setting.getSettingType(), setting);
            }
        }
    }

    private void removeSettingByTypeSync(ICameraSetting setting) {
        synchronized (mSettingsByTypeSync) {
            mSettingListByType.remove(setting.getSettingType(), setting);
        }
    }
}