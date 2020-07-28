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

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.ICameraMode.ModeType;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nonnull;

/**
 * The factory used to create and recycle SettingManager instance.
 */
public class SettingManagerFactory {
    private final IApp mApp;
    private final ICameraContext mCameraContext;
    private final Map<String, SettingManagerWrapper> mSettingManagerWrappers
            = new ConcurrentHashMap<>();

    /**
     * SettingManagerFactory constructor.
     *
     * @param app The instance of IApp.
     * @param cameraContext The instance of {@link ICameraContext};
     */
    public SettingManagerFactory(IApp app, ICameraContext cameraContext) {
        mApp = app;
        mCameraContext = cameraContext;
    }

    /**
     * Get an instance of SettingManager. It will return original instance
     * if the specified camera id's SettingManger exists.
     *
     * @param cameraId the camera id of setting manager.
     * @param modeKey The key to indicator mode.
     * @param modeType The type of mode.
     * @param currentCameraApi current camera api
     * @return an instance of SettingManager.
     */
    @Nonnull
    public ISettingManager getInstance(@Nonnull String cameraId,
                                       @Nonnull String modeKey,
                                       @Nonnull ModeType modeType,
                                       @Nonnull CameraApi currentCameraApi) {
        SettingManagerWrapper settingManagerWrapper = mSettingManagerWrappers.get(cameraId);
        if (settingManagerWrapper == null) {
            settingManagerWrapper = new SettingManagerWrapper(
                    new SettingManager(),
                    modeKey,
                    modeType);
            settingManagerWrapper.getSettingManager().init(cameraId, mApp,
                    mCameraContext, currentCameraApi);
            settingManagerWrapper.getSettingManager().bindMode(modeKey, modeType);

            mSettingManagerWrappers.put(cameraId, settingManagerWrapper);
        }

        SettingManager settingManager = settingManagerWrapper.getSettingManager();
        if (!modeKey.equals(settingManagerWrapper.getModeKey())) {
            String oldModeKey = settingManagerWrapper.getModeKey();
            settingManagerWrapper.updateModeKey(modeKey);
            settingManagerWrapper.updateModeType(modeType);
            settingManager.unbindMode(oldModeKey);
            settingManager.bindMode(modeKey, modeType);
        }

        return settingManager;
    }

    /**
     * Recycle SettingManger instance by camera id.
     *
     * @param cameraId been recycled SettingManager's id.
     */
    public void recycle(@Nonnull String cameraId) {
        SettingManagerWrapper settingManagerWrapper = mSettingManagerWrappers.get(cameraId);
        if (settingManagerWrapper != null) {
            settingManagerWrapper.getSettingManager().unbindMode(
                    settingManagerWrapper.getModeKey());
            settingManagerWrapper.getSettingManager().unInit();
            mSettingManagerWrappers.remove(cameraId);
        }
    }

    /**
     * Recycle all SettingManager instances.
     */
    public void recycleAll() {
        Set<String> cameraIds = mSettingManagerWrappers.keySet();
        for (String cameraId : cameraIds) {
            recycle(cameraId);
        }
    }

    /**
     * A wrapper class for SettingManager, used to restore mode key and mode type.
     */
    private class SettingManagerWrapper {
        private final SettingManager mSettingManager;
        private String mModeKey;
        private ModeType mModeType;

        /**
         * Construct the SettingManagerWrapper.
         *
         * @param settingManager SettingManager created by mode.
         * @param modeKey Mode's key.
         * @param modeType Mode's type.
         */
        SettingManagerWrapper(SettingManager settingManager,
                              String modeKey,
                              ModeType modeType) {
            mSettingManager = settingManager;
            mModeKey = modeKey;
            mModeType = modeType;
        }

        /**
         * Get an instance of SettingManager.
         *
         * @return the instance of SettingManager.
         */
        SettingManager getSettingManager() {
            return mSettingManager;
        }

        /**
         * Update the mode key.
         *
         * @param modeKey update mode's key.
         */
        void updateModeKey(String modeKey) {
            mModeKey = modeKey;
        }

        /**
         * Update the mode type.
         *
         * @param modeType update mode's type.
         */
        void updateModeType(ModeType modeType) {
            mModeType = modeType;
        }

        /**
         * Get the mode's type.
         * @return mode's type.
         */
        ModeType getModeType() {
            return mModeType;
        }

        /**
         * Get the mode's key.
         *
         * @return the mode's key.
         */
        String getModeKey() {
            return mModeKey;
        }
    }
}
