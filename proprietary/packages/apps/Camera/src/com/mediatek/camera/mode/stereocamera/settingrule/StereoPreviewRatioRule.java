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
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.preference.ListPreference;

/**
 * This class used for Stereo preview ratio rule.
 */
public class StereoPreviewRatioRule extends StereoSettingRule {
    private static final String TAG = "StereoPreviewRatioRule";
    private SettingItem mCurrentSettingItem;
    private SettingItem mPictureRatioSetting;

    /**
     * Creator for Stereo ratio rule.
     * @param cameraContext
     *            camera context
     * @param featureType
     *            stereo feature type
     */
    public StereoPreviewRatioRule(ICameraContext cameraContext, int featureType) {
        super(cameraContext, featureType);
    }

    @Override
    public void execute() {
        super.execute();
        Log.i(TAG, "feature type = " + mFeatureType);
        if (mFeatureType == DENOISE) {
            mCurrentSettingItem = mISettingCtrl.getSetting(SettingConstants.KEY_PHOTO_STEREO);
        } else {
            mCurrentSettingItem = mISettingCtrl.getSetting(SettingConstants.KEY_REFOCUS);
        }
        mPictureRatioSetting = mISettingCtrl.getSetting(SettingConstants.KEY_PICTURE_RATIO);
        String resultValue = mPictureRatioSetting.getValue();
        String currentValue = mCurrentSettingItem.getValue();
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(currentCameraId);
        Parameters parameters = cameraDevice.getParameters();
        if (mFeatureType != DENOISE && ParametersHelper.isVsDofSupported(parameters)) {
            Log.i(TAG, "VsDof support 4:3");
            SettingUtils.setPreviewSize(mCameraContext.getActivity(), parameters, resultValue);
            if (ParametersHelper.isDenoiseSupported(parameters)) {
                parameters.set("preview-frame-rate", 24);
            }
            return;
        }
        ListPreference pref = mPictureRatioSetting.getListPreference();
        if ("on".equals(currentValue)) {
            String overrideValue = "1.7778";
            resultValue = "1.7778";
            if (mPictureRatioSetting.isEnable()) {
                mPictureRatioSetting.setValue(resultValue);
                if (pref != null) {
                    pref.setOverrideValue(overrideValue, true);
                }
                SettingUtils
                        .setPreviewSize(mCameraContext.getActivity(), parameters, overrideValue);
            }
            if (mFeatureType != DENOISE &&
                    ParametersHelper.isDenoiseSupported(parameters)) {
                parameters.set("preview-frame-rate", 24);
            }
            Record record = mPictureRatioSetting.new Record(resultValue, overrideValue);
            if (mFeatureType == DENOISE) {
                mPictureRatioSetting.addOverrideRecord(SettingConstants.KEY_PHOTO_STEREO, record);
            } else {
                mPictureRatioSetting.addOverrideRecord(SettingConstants.KEY_REFOCUS, record);
            }
        } else {
            if (mFeatureType == DENOISE) {
                mPictureRatioSetting.removeOverrideRecord(SettingConstants.KEY_PHOTO_STEREO);
            } else {
                mPictureRatioSetting.removeOverrideRecord(SettingConstants.KEY_REFOCUS);
            }
            int count = mPictureRatioSetting.getOverrideCount();
            if (count > 0) {
                Record topRecord = mPictureRatioSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    String value = topRecord.getValue();
                    String overrideValue = topRecord.getOverrideValue();
                    mPictureRatioSetting.setValue(value);
                    pref = mPictureRatioSetting.getListPreference();
                    if (pref != null) {
                        pref.setOverrideValue(overrideValue);
                    }
                }
            } else {
                pref = mPictureRatioSetting.getListPreference();
                if (pref != null) {
                    pref.setOverrideValue(null);
                }
                mPictureRatioSetting.setValue(resultValue);
                SettingUtils.setPreviewSize(mCameraContext.getActivity(), parameters, resultValue);
            }
        }
    }
}