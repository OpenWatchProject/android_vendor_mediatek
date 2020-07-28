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
package com.mediatek.camera.common;

import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.RelationGroup;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.List;

/**
 * The class use to create a mock setting.
 */

public class SettingCreator {

    /**
     * Create a mock setting.
     *
     * @param info The mock setting basic info. See {@link SettingInfo}.
     * @param dataStore The data store object. See {@link DataStore}. If the unit test must use
     *                  this object, it should be not null.
     * @param settingController See {@link DataStore}. If the unit test must use this object, it
     *                          should be not null.
     * @param relationGroup The mock setting's relation. If the unit test need test relation, it
     *                      should be not null.
     * @param parametersConfigure See {@link ICameraSetting.IParametersConfigure}.
     * @param callback See {@link ICameraSetting.PreviewStateCallback}.
     * @return A mock setting.
     */
    public ICameraSetting createSetting(SettingInfo info,
                                     DataStore dataStore,
                                     ISettingManager.SettingController settingController,
                                     RelationGroup relationGroup,
                                     ICameraSetting.IParametersConfigure parametersConfigure,
                                     ICameraSetting.PreviewStateCallback callback) {
        return new MockSetting(info, dataStore, settingController,
                relationGroup, parametersConfigure, callback);
    }

    /**
     * The basic info of setting.
     */
    public class SettingInfo {
        /**
         * The key of mock setting, it can not be null.
         */
        public String key;

        /**
         * The type of mock setting, it can not be null.
         */
        public ICameraSetting.SettingType settingType;

        /**
         * The default value of setting.
         */
        public String defaultValue;

        /**
         * The setting's value store in data store.
         */
        public String valueInDataStore;

        /**
         * The values supported by current platform, it can not be null.
         */
        public List<String> supportedPlatformValues;

        /**
         * The values supported defined by app, it can not be null.
         */
        public List<String> supportedEntryValues;

        /**
         * The current can be used values, it can not be null.
         */
        public List<String> entryValues;
    }

    /**
     * Mock setting class.
     */
    private class MockSetting extends SettingBase {
        private SettingInfo mInfo;
        private RelationGroup mRelationGroup;
        private IParametersConfigure mParametersConfigure;
        private PreviewStateCallback mCallback;

        /**
         * Constructor.
         *
         * @param info The basic info of setting.
         * @param dataStore The data store object. See {@link DataStore}.
         * @param settingController See {@link DataStore}.
         * @param relationGroup The mock setting's relation.
         * @param parametersConfigure See {@link IParametersConfigure}.
         * @param callback See{@link PreviewStateCallback}.
         */
        public MockSetting(SettingInfo info,
                             DataStore dataStore,
                             ISettingManager.SettingController settingController,
                             RelationGroup relationGroup,
                             IParametersConfigure parametersConfigure,
                             PreviewStateCallback callback) {
            mInfo = info;
            mDataStore = dataStore;
            mSettingController = settingController;
            mRelationGroup = relationGroup;
            mParametersConfigure = parametersConfigure;
            mCallback = callback;
            setSupportedEntryValues(info.supportedEntryValues);
            setSupportedPlatformValues(info.supportedPlatformValues);
            setEntryValues(info.entryValues);
            setValue(info.valueInDataStore != null ? info.valueInDataStore : info.defaultValue);
        }

        @Override
        public void init(IApp app, ICameraContext cameraContext,
                         ISettingManager.SettingController settingController) {
        }

        @Override
        public void unInit() {

        }

        @Override
        public void postRestrictionAfterInitialized() {

        }

        @Override
        public void overrideValues(String headerKey, String currentValue,
                                   List<String> supportValues) {
            if (mSettingController != null) {
                String beforeOverride = getValue();
                super.overrideValues(headerKey, currentValue, supportValues);
                if (!beforeOverride.equals(getValue())) {
                    mSettingController.postRestriction(
                            mRelationGroup.getRelation(getValue(), true));
                }
            } else {
                setValue(currentValue);
                setEntryValues(supportValues);
            }
        }

        @Override
        public SettingType getSettingType() {
            return mInfo.settingType;
        }

        @Override
        public String getKey() {
            return mInfo.key;
        }

        @Override
        public IParametersConfigure getParametersConfigure() {
            return mParametersConfigure;
        }

        @Override
        public ICaptureRequestConfigure getCaptureRequestConfigure() {
            return null;
        }

        @Override
        public PreviewStateCallback getPreviewStateCallback() {
            return mCallback;
        }
    }
}
