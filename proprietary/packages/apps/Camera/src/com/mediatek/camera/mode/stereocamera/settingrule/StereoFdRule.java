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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.mode.stereocamera.settingrule;

import android.util.Log;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;

/**
 * This class used for Stereo face detection rule.
 */
public class StereoFdRule extends StereoSettingRule {
    private static final String TAG = "StereoPictureSizeRule";
    private SettingItem mCurrentSettingItem;

    /**
     * Creator for Stereo face detection rule.
     * @param cameraContext
     *            camera context
     * @param featureType
     *            stereo feature type
     */
    public StereoFdRule(ICameraContext cameraContext, int featureType) {
        super(cameraContext, featureType);
    }

    @Override
    public void execute() {
        super.execute();
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(currentCameraId);
        Parameters parameters = cameraDevice.getParameters();
        if (!ParametersHelper.isVsDofSupported(parameters)) {
            Log.i(TAG, "VsDof only support zsd capture");
            return;
        }
        if (parameters == null || (parameters != null &&
                parameters.getMaxNumFocusAreas() > 0)) {
            Log.i(TAG, "VsDof not support fd when lens FF type.");
            return;
        }
        mCurrentSettingItem = mISettingCtrl
                .getSetting(SettingConstants.KEY_REFOCUS);
        String currentValue = mCurrentSettingItem.getValue();
        Log.i(TAG, "[execute] FDRule currentValue = " + currentValue);
        SettingItem fdSetting = mISettingCtrl
                .getSetting(SettingConstants.KEY_CAMERA_FACE_DETECT);
        if (!"on".equals(currentValue)) {
            int overrideCount = fdSetting.getOverrideCount();
            Record record = fdSetting
                    .getOverrideRecord(SettingConstants.KEY_REFOCUS);
            if (record == null) {
                return;
            }
            fdSetting.removeOverrideRecord(SettingConstants.KEY_REFOCUS);
            overrideCount--;
            String value = null;
            String overrideValue = null;
            ListPreference pref = mISettingCtrl
                    .getListPreference(SettingConstants.KEY_CAMERA_FACE_DETECT);
            if (overrideCount > 0) {
                Record topRecord = fdSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    value = topRecord.getValue();
                    overrideValue = topRecord.getOverrideValue();
                }
            } else {
                if (pref != null) {
                    value = pref.getValue();
                }
            }
            mISettingCtrl.setSettingValue(
                    SettingConstants.KEY_CAMERA_FACE_DETECT, value,
                    mICameraDeviceManager.getCurrentCameraId());
            if (pref != null) {
                pref.setOverrideValue(overrideValue);
            }
        } else {
            mISettingCtrl.setSettingValue(
                    SettingConstants.KEY_CAMERA_FACE_DETECT, "off",
                    mICameraDeviceManager.getCurrentCameraId());
            mISettingCtrl.getListPreference(
                    SettingConstants.KEY_CAMERA_FACE_DETECT)
                    .setOverrideValue("off");
            Record record = fdSetting.new Record("off", "off");
            fdSetting.addOverrideRecord(SettingConstants.KEY_REFOCUS,
                    record);
        }
    }
}