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

import java.util.ArrayList;
import java.util.List;

import android.graphics.Point;
import android.util.Log;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingCtrl;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;

/**
 * This class used for Stereo picture size rule.
 */
public class StereoPictureSizeRule extends StereoSettingRule {
    private static final String TAG = "StereoPictureSizeRule";
    private static final String KEY_REFOCUS_PICTURE_SIZE_VALUES = "refocus-picture-size-values";
    private SettingItem mCurrentSettingItem;
    private SettingItem mPictureSize;

    /**
     * Creator for Stereo picture size rule.
     * @param cameraContext
     *            camera context
     * @param featureType
     *            stereo feature type
     */
    public StereoPictureSizeRule(ICameraContext cameraContext, int featureType) {
        super(cameraContext, featureType);
    }

    @Override
    public void execute() {
        super.execute();
        mCurrentSettingItem = mISettingCtrl
                .getSetting(SettingConstants.KEY_REFOCUS);
        mPictureSize = mISettingCtrl
                .getSetting(SettingConstants.KEY_PICTURE_SIZE);
        String resultValue = mPictureSize.getValue();
        String currentValue = mCurrentSettingItem.getValue();
        Log.i(TAG, "currentValue = " + currentValue);
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager
                .getCameraDevice(currentCameraId);
        Parameters parameters = cameraDevice.getParameters();
        ListPreference pref = mPictureSize.getListPreference();
        if ("on".equals(currentValue)) {
            List<Point> supportedRefocusPictureSize = SettingUtils
                    .splitSize(parameters.get(KEY_REFOCUS_PICTURE_SIZE_VALUES));
            if (supportedRefocusPictureSize == null) {
                Log.e(TAG, "there is no picture size supported by refocus");
                return;
            }
            String overrideValue = null;
            String pictureRatio = mISettingCtrl.getSettingValue(
                    SettingConstants.KEY_PICTURE_RATIO);
            List<String> optinalSize = new ArrayList<String>();
            for (Point size : supportedRefocusPictureSize) {
                double ratio = (double) size.x / size.y;
                if (Math.abs(ratio - Double.parseDouble(pictureRatio))
                        <= SettingUtils.ASPECT_TOLERANCE) {
                    optinalSize.add(SettingUtils.pointToStr(size));
                }
            }
            if (optinalSize.size() == 0) {
                Log.e(TAG, "there is no picture size meeted to current ratio:"
                        + pictureRatio + " supported by refocus");
                return;
            }
            if (!optinalSize.contains(resultValue)) {
                resultValue = optinalSize.get(optinalSize.size() - 1);
            }
            if (optinalSize.size() == 1) {
                overrideValue = optinalSize.get(0);
            } else {
                String[] values = new String[optinalSize.size()];
                overrideValue = SettingUtils.buildEnableList(
                        optinalSize.toArray(values), resultValue);
            }
            if (mPictureSize.isEnable()) {
                mPictureSize.setValue(resultValue);
                if (pref != null) {
                     pref.setOverrideValue(overrideValue, true);
                }
            }
            ParametersHelper.setParametersValue(parameters, currentCameraId,
                    SettingConstants.KEY_PICTURE_SIZE, resultValue);
            Record record = mPictureSize.new Record(resultValue, overrideValue);
            mPictureSize.addOverrideRecord(SettingConstants.KEY_REFOCUS, record);
        } else {
            mPictureSize.removeOverrideRecord(SettingConstants.KEY_REFOCUS);
            int count = mPictureSize.getOverrideCount();
            if (count > 0) {
                Record topRecord = mPictureSize.getTopOverrideRecord();

                if (topRecord != null) {
                    String value = topRecord.getValue();
                    String overrideValue = topRecord.getOverrideValue();
                    mPictureSize.setValue(value);
                    pref = mPictureSize.getListPreference();
                    if (pref != null) {
                         pref.setOverrideValue(overrideValue);
                    }
                }
            } else {
                pref = mPictureSize.getListPreference();
                if (pref != null) {
                    pref.setOverrideValue(null);
               }
                mPictureSize.setValue(resultValue);
            }
        }
    }
}