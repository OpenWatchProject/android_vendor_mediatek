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
package com.mediatek.camera.mode;

import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule.MappingFinder;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.List;

    public class VideoHdrRule implements ISettingRule {

        private String TAG = "VideoHdrRule";
        private ISettingCtrl mISettingCtrl;
        private ICameraDeviceManager mICameraDeviceManager;
        private IModuleCtrl mIMoudleCtrl;
        private ICameraDevice mICameraDevice;
        private ICameraContext mICameraContext;

        private List<String> mConditions = new ArrayList<String>();
        private List<List<String>> mResults = new ArrayList<List<String>>();
        private List<MappingFinder> mMappingFinder = new ArrayList<MappingFinder>();
        private Parameters mParameters;
        private String mLastHdrValue;
        private boolean mHasOverride = false;

        public VideoHdrRule(ICameraContext cameraContext) {
            Log.i(TAG, "[VideoPreviewRule]constructor...");
            mICameraContext = cameraContext;
            mISettingCtrl = cameraContext.getSettingController();
            mICameraDeviceManager = cameraContext.getCameraDeviceManager();
            mIMoudleCtrl = cameraContext.getModuleController();

        }

        @Override
        public void execute() {
            String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO);
        SettingItem zsdSetting = mISettingCtrl.getSetting(SettingConstants.KEY_CAMERA_ZSD);
        boolean isZsdOpened = "on".equals(zsdSetting.getValue());
            getCameraDevice();
            int index = conditionSatisfied(value);
            if (index == -1) {
                if (mHasOverride) {
                    restoreHdrRule();
                    mHasOverride = false;
                }
            if (mICameraContext.getFeatureConfig().isZSDHDRSupported() && isZsdOpened) {
                int cameraId = mICameraDeviceManager.getCurrentCameraId();
                ParametersHelper.setParametersValue(mParameters, cameraId,
                        SettingConstants.KEY_HDR, "off");
            }
            } else {
                mLastHdrValue = mISettingCtrl.getSettingValue(SettingConstants.KEY_HDR);
                setHdrRule();
            }

        }

        @Override
    public void addLimitation(String condition, List<String> result, MappingFinder mappingFinder) {
        mConditions.add(condition);
        mResults.add(result);
        mMappingFinder.add(mappingFinder);
    }

        private void setHdrRule() {
        mParameters = mICameraDevice.getParameters();
        List<String> supportedValues = ParametersHelper.getParametersSupportedValues(mParameters,
                SettingConstants.KEY_HDR);
        if (supportedValues == null || supportedValues.indexOf("on") < 0) {
                mISettingCtrl.setSettingValue(SettingConstants.KEY_HDR, "off",
                        mICameraDeviceManager.getCurrentCameraId());
                SettingItem setting = mISettingCtrl.getSetting(SettingConstants.KEY_HDR);
                if (!mHasOverride) {
                    int overrideCount = setting.getOverrideCount();
                    setting.setOverrideCount(overrideCount + 1);
                    mHasOverride = true;
                }
                Record record = setting.new Record("off", "off");
                setting.addOverrideRecord(SettingConstants.KEY_VIDEO, record);
        } else {
            int cameraId = mICameraDeviceManager.getCurrentCameraId();
            SettingItem setting = mISettingCtrl.getSetting(SettingConstants.KEY_HDR);
            ParametersHelper.setParametersValue(mParameters, cameraId, SettingConstants.KEY_HDR,
                    setting.getValue());
        }
        }

        private void restoreHdrRule() {
            Log.i(TAG, "[restoreHdrRule], mLastHdrValue:" + mLastHdrValue + ", and mHasOverride:"
                + mHasOverride);
            if (!mHasOverride) {
                return;
            }
            String overrideValue = null;
            SettingItem setting = mISettingCtrl.getSetting(SettingConstants.KEY_HDR);
            int overrideCount = setting.getOverrideCount();
            Record overrideRecord = setting.getOverrideRecord(SettingConstants.KEY_VIDEO);
            if (overrideCount > 0 && overrideRecord != null) {
            overrideCount--;
                setting.setOverrideCount(overrideCount);
                setting.removeOverrideRecord(SettingConstants.KEY_VIDEO);
                mHasOverride = false;
            }

            Log.i(TAG, "overrideCount:" + overrideCount);
            if (overrideCount > 0) {
                Record record = setting.getTopOverrideRecord();
                if (record != null) {
                    mISettingCtrl.setSettingValue(SettingConstants.KEY_HDR, record.getValue(),
                            mICameraDeviceManager.getCurrentCameraId());
                    overrideValue = record.getOverrideValue();
                }
            } else {
                mISettingCtrl.setSettingValue(SettingConstants.KEY_HDR, mLastHdrValue,
                        mICameraDeviceManager.getCurrentCameraId());
            }

            ListPreference pref = mISettingCtrl.getListPreference(SettingConstants.KEY_HDR);
            if (pref != null) {
                pref.setOverrideValue(overrideValue);
            }
        }

        private int conditionSatisfied(String conditionValue) {
            int index = mConditions.indexOf(conditionValue);
            Log.i(TAG, "[conditionSatisfied]limitation index:" + index);
            return index;
        }

        private void getCameraDevice() {
            if (mICameraDeviceManager != null) {
                int camerId = mICameraDeviceManager.getCurrentCameraId();
                mICameraDevice = mICameraDeviceManager.getCameraDevice(camerId);
            }

        }
    }
