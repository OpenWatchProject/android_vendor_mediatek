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
package com.mediatek.camera.mode.facebeauty;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

import java.util.ArrayList;
import java.util.List;

public class VideoFaceBeautyRule implements ISettingRule {

    private String TAG = "VideoFaceBeautyVideoRule";
    private ISettingCtrl mISettingCtrl;
    private ICameraDeviceManager mICameraDeviceManager;
    private ICameraDevice mICameraDevice;

    private List<String> mConditions = new ArrayList<String>();
    private List<List<String>> mResults = new ArrayList<List<String>>();
    private List<MappingFinder> mMappingFinder = new ArrayList<MappingFinder>();
    private String mLastValue;

    public VideoFaceBeautyRule(ICameraContext cameraContext) {
        Log.i(TAG, "[VideoFaceBeautyRule]constructor...");
        mISettingCtrl = cameraContext.getSettingController();
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
    }

    @Override
    public void execute() {
        String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO);
        getCameraDevice();
        Parameters parameters = mICameraDevice.getParameters();

        int index = conditionSatisfied(value);
        if (index == -1) {
            String overrideValue = null;
            SettingItem setting = mISettingCtrl.getSetting(SettingConstants.KEY_SLOW_MOTION);
            Record record = setting.getOverrideRecord(SettingConstants.KEY_VIDEO);
            if (record == null) {
                return;
            }

            setting.removeOverrideRecord(SettingConstants.KEY_VIDEO);

            int overrideCount = setting.getOverrideCount();
            Log.i(TAG, "overrideCount:" + overrideCount);
            if (overrideCount > 0) {
                Record topRecord = setting.getTopOverrideRecord();
                if (topRecord != null) {
                    setting.setValue(topRecord.getValue());
                    overrideValue = topRecord.getOverrideValue();
                }
            } else {
                mISettingCtrl.setSettingValue(SettingConstants.KEY_SLOW_MOTION, mLastValue,
                        mICameraDeviceManager.getCurrentCameraId());
            }

            ListPreference pref = mISettingCtrl.getListPreference(SettingConstants.KEY_SLOW_MOTION);
            if (pref != null) {
                pref.setOverrideValue(overrideValue);
            }
        } else {
            if (parameters != null
                    && Util.VIDEO_FACE_BEAUTY_ENABLE.equals(parameters
                            .get(Util.KEY_VIDEO_FACE_BEAUTY))) {
                SettingItem setting = mISettingCtrl.getSetting(SettingConstants.KEY_SLOW_MOTION);
                mLastValue = setting.getValue();
                setting.setValue("off");

                ListPreference pref = mISettingCtrl
                        .getListPreference(SettingConstants.KEY_SLOW_MOTION);
                if (pref != null) {
                    pref.setOverrideValue(SettingUtils.RESET_STATE_VALUE_DISABLE);
                }

                Record record = setting.new Record("off", SettingUtils.RESET_STATE_VALUE_DISABLE);
                setting.addOverrideRecord(SettingConstants.KEY_VIDEO, record);
            }
        }

    }

    @Override
    public void addLimitation(String condition, List<String> result, MappingFinder mappingFinder) {
        mConditions.add(condition);
        mResults.add(result);
        mMappingFinder.add(mappingFinder);
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
