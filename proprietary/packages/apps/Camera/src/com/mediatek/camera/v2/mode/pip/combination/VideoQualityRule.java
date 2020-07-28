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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.camera.v2.mode.pip.combination;

import android.media.CamcorderProfile;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.setting.ISettingRule;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.setting.SettingItem;
import com.mediatek.camera.v2.setting.SettingItem.Record;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import java.util.ArrayList;
import java.util.List;

public class VideoQualityRule implements ISettingRule {
    private static final Tag TAG = new Tag(VideoQualityRule.class.getSimpleName());
    private static final int QUALITY_NUMBER = 3;
    private final SettingCtrl        mSettingCtrl;
    private ISettingServant          mSettingServant;
    private String                   mCurrentCameraId = null;

    private static final String QUALITY_QCIF
        = Integer.toString(CamcorderProfile.QUALITY_QCIF);
    private static final String QUALITY_CIF
        = Integer.toString(CamcorderProfile.QUALITY_CIF);
    private static final String QUALITY_480P
        = Integer.toString(CamcorderProfile.QUALITY_480P);
    private static final String QUALITY_720P
        = Integer.toString(CamcorderProfile.QUALITY_720P);
    private static final String QUALITY_1080P
        = Integer.toString(CamcorderProfile.QUALITY_1080P);
    private static final String QUALITY_QVGA
        = Integer.toString(CamcorderProfile.QUALITY_QVGA);
    private static final String QUALITY_2160P
        = Integer.toString(CamcorderProfile.QUALITY_2160P);

    private static final int[] NORMAL_SUPPORT_QUALIYS = new int[] {
        CamcorderProfile.QUALITY_2160P,
        CamcorderProfile.QUALITY_1080P,
        CamcorderProfile.QUALITY_720P,
        CamcorderProfile.QUALITY_480P,
        CamcorderProfile.QUALITY_CIF,
        CamcorderProfile.QUALITY_QVGA,
        CamcorderProfile.QUALITY_QCIF};

    private static final String[] NORMAL_SUPPORT_QUALIYS_STRING = new String[] {
        QUALITY_2160P,
        QUALITY_1080P,
        QUALITY_720P,
        QUALITY_480P,
        QUALITY_CIF,
        QUALITY_QVGA,
        QUALITY_QCIF};

    public VideoQualityRule(SettingCtrl settingCtrl) {
        mSettingCtrl = settingCtrl;
        mCurrentCameraId = mSettingCtrl.getCurrentCameraId();
        mSettingServant = mSettingCtrl.getSettingServant(mSettingCtrl.getCurrentCameraId());
    }

    @Override
    public void execute() {
        String cameraId = mSettingCtrl.getCurrentCameraId();
        if (!mCurrentCameraId.equals(cameraId)) {
            mSettingServant = mSettingCtrl.getSettingServant(cameraId);
            mCurrentCameraId = cameraId;
        }
        String pipKeyValue = mSettingServant.getSettingValue(SettingKeys.KEY_PHOTO_PIP);
        SettingItem videoQualitySettingItem =
                mSettingServant.getSettingItem(SettingKeys.KEY_VIDEO_QUALITY);
        if ("on".equalsIgnoreCase(pipKeyValue)) {
            // override video quality and write value to setting.
            List<String> supportedValues = getSupportedPIPVideoQualities();
            String currentQuality = mSettingServant.getSettingValue(SettingKeys.KEY_VIDEO_QUALITY);
            String quality = getQuality(currentQuality, supportedValues);
            videoQualitySettingItem.setValue(quality);
            LogHelper.i(TAG, "enter pip set quality:" + quality);
            // update video quality setting ui.
            String overrideValue = null;
            if (supportedValues != null) {
                String[] values = new String[supportedValues.size()];
                overrideValue = Utils.buildEnableList(supportedValues.toArray(values));
                videoQualitySettingItem.setOverrideValue(overrideValue);
            }
            Record record = videoQualitySettingItem.new Record(quality, overrideValue);
            videoQualitySettingItem.addOverrideRecord(SettingKeys.KEY_PHOTO_PIP, record);
        } else if ("off".equalsIgnoreCase(pipKeyValue)) {
            int overrideCount = videoQualitySettingItem.getOverrideCount();
            Record record = videoQualitySettingItem.getOverrideRecord(SettingKeys.KEY_PHOTO_PIP);
            if (record == null) {
                return;
            }
            videoQualitySettingItem.removeOverrideRecord(SettingKeys.KEY_PHOTO_PIP);
            overrideCount--;
            String quality = null;
            if (overrideCount > 0) {
                Record topRecord = videoQualitySettingItem.getTopOverrideRecord();
                if (topRecord != null) {
                    quality = topRecord.getValue();
                    videoQualitySettingItem.setValue(quality);
                    String overrideValue = topRecord.getOverrideValue();
                    videoQualitySettingItem.setValue(quality);
                    videoQualitySettingItem.setOverrideValue(overrideValue);
                }
            } else {
                quality = mSettingServant.getSharedPreferencesValue(SettingKeys.KEY_VIDEO_QUALITY);
                videoQualitySettingItem.setOverrideValue(null);
                videoQualitySettingItem.setValue(quality);
            }
            LogHelper.i(TAG, "exit pip set quality:" + quality + " overrideCount " + overrideCount);
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result) {
    }

    private List<String> getSupportedPIPVideoQualities() {
        LogHelper.i(TAG, "getSupportedPIPVideoQualities");
        ArrayList<String> supported = new ArrayList<String>();

        int backCameraId = Integer.valueOf(SettingCtrl.BACK_CAMERA);
        int qualitys = NORMAL_SUPPORT_QUALIYS_STRING.length;
        int qualitySize = QUALITY_NUMBER;

        for (int i = 0, supportSize = 0; i < qualitys && supportSize < qualitySize; i++) {
            if (CamcorderProfile.hasProfile(backCameraId, NORMAL_SUPPORT_QUALIYS[i])) {
                CamcorderProfile profile
                    = Utils.getVideoProfile(backCameraId, NORMAL_SUPPORT_QUALIYS[i]);
                supportSize ++;
                if (profile.videoFrameWidth <= 1920) {
                    supported.add(NORMAL_SUPPORT_QUALIYS_STRING[i]);
                }
            }
        }
        int size = supported.size();
        if (size > 0) {
            return supported;
        }
        return null;
    }

    private String getQuality(String current, List<String> supportedList) {
        String supported = current;
        if (supportedList != null && !supportedList.contains(current)) {
            if (Integer.toString(CamcorderProfile.QUALITY_1080P).equals(current)) {
                // match normal fine quality to high in pip mode
                supported = Integer.toString(CamcorderProfile.QUALITY_720P);
            }
        }
        if (!supportedList.contains(supported)) {
            supported = supportedList.get(0);
        }
        return supported;
    }
}