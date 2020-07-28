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
package com.mediatek.camera.feature.setting.videoquality;

import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;
import com.mediatek.camera.portability.CamcorderProfileEx;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;


/**
 * Configure video quality parameters on API1.
 */

public class VideoQualityParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final Tag TAG = new Tag(VideoQualityParametersConfig.class.getSimpleName());

    private static final int QUALITY_INDEX_NUMBER = 2;
    private static final int QUALITY_MAX_NUMBER = 4;
    private SettingDeviceRequester mDeviceRequester;
    private VideoQuality mVideoQuality;
    private List<Size> mSupportedSizes;

    /**
     * Video quality parameters configure constructor.
     * @param videoQuality The instance of {@link VideoQuality}.
     * @param deviceRequester The implementer of {@link SettingDeviceRequester}.
     */
    public VideoQualityParametersConfig(
                      VideoQuality videoQuality , SettingDeviceRequester deviceRequester) {
        mVideoQuality = videoQuality;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        mSupportedSizes = originalParameters.getSupportedVideoSizes();
        updateSupportedValues();
        mVideoQuality.updateValue(getDefaultQuality());
        mVideoQuality.onValueInitialized();
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    /**
     * Get the max size as default value of video quality.
     * @return getDefaultValue.
     */
    private String getDefaultQuality() {
        int defaultIndex = 0;
        if (mVideoQuality.getSupportedPlatformValues().size() > QUALITY_INDEX_NUMBER) {
            defaultIndex = 1;
        }
        String defaultSize = mVideoQuality.getSupportedPlatformValues().get(defaultIndex);
        return defaultSize;
    }

    private void updateSupportedValues() {
        List<String> supported = getSupportedListQuality(
                                      Integer.parseInt(mVideoQuality.getCameraId()));
        mVideoQuality.setSupportedPlatformValues(supported);
        mVideoQuality.setEntryValues(supported);
        mVideoQuality.setSupportedEntryValues(supported);
    }

    private List<String> getSupportedListQuality(int cameraId) {
        ArrayList<String> supported = new ArrayList<String>();
        generateSupportedList(cameraId, supported, VideoQualityHelper.sMtkVideoQualities);
        if (supported.isEmpty()) {
            generateSupportedList(cameraId, supported, VideoQualityHelper.sVideoQualities);
        }
        return supported;
    }

    private void generateSupportedList(int cameraId,
                                       ArrayList<String> supported, int[] defMatrix) {
        for (int i = 0; i < defMatrix.length && supported.size() < QUALITY_MAX_NUMBER; i++) {
            if (CamcorderProfile.hasProfile(cameraId, defMatrix[i])
                    && featureByParameter(cameraId, defMatrix[i])) {
                supported.add(Integer.toString(defMatrix[i]));
                LogHelper.d(TAG, "generateSupportedList add " + defMatrix[i]);
            }
        }
    }

    private boolean featureByParameter(int cameraId, int quality) {
        CamcorderProfile profile = CamcorderProfileEx.getProfile(cameraId, quality);
        Iterator<Size> it = mSupportedSizes.iterator();
        boolean support = false;
        while (it.hasNext()) {
            Size size = it.next();
            if (size.width >= profile.videoFrameWidth) {
                support = true;
                break;
            }
        }
        return support;
    }

    /**
     * Send request when setting value is changed.
     */
    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mVideoQuality.getKey());
    }
}
