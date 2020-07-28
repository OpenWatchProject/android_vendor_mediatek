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

import android.annotation.TargetApi;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.CamcorderProfile;
import android.os.Build;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;
import com.mediatek.camera.portability.CamcorderProfileEx;

import java.util.ArrayList;
import java.util.List;


/**
 * Configure video quality in capture request in camera api2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class VideoQualityCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG =
                      new LogUtil.Tag(VideoQualityCaptureRequestConfig.class.getSimpleName());
    private static final int QUALITY_INDEX_NUMBER = 2;
    private static final int QUALITY_MAX_NUMBER = 4;
    private CameraCharacteristics mCameraCharacteristics;
    private SettingDevice2Requester mDevice2Requester;
    private VideoQuality mVideoQuality;
    private List<Size> mSupportedSizes;
    /**
     * video quality capture request configure constructor.
     * @param quality The instance of {@link VideoQuality}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     */
    public VideoQualityCaptureRequestConfig(VideoQuality quality,
                                      SettingDevice2Requester device2Requester) {
        mVideoQuality = quality;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mCameraCharacteristics = characteristics;
        mSupportedSizes = getSupportedVideoSizes();
        updateSupportedValues();
        mVideoQuality.updateValue(getDefaultQuality());
        mVideoQuality.onValueInitialized();
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {

    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
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
                    && featureCharacteristics(cameraId, defMatrix[i])) {
                supported.add(Integer.toString(defMatrix[i]));
                LogHelper.d(TAG, "generateSupportedList add " + defMatrix[i]);
            }
        }
    }

    private boolean featureCharacteristics(int cameraId, int quality) {
        CamcorderProfile profile = CamcorderProfileEx.getProfile(cameraId, quality);
        Size videoSz = new Size(profile.videoFrameWidth, profile.videoFrameHeight);
        boolean support = false;
        if (mSupportedSizes.contains(videoSz)) {
            support = true;
        }
        return support;
    }

    private List<Size> getSupportedVideoSizes() {
        try {
            Size[] rawSizes = getSupportedSizeForClass();
            List<Size> sizes = new ArrayList<Size>();
            for (Size sz: rawSizes) {
                sizes.add(sz);
            }
            return sizes;
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return null;
    }

    private Size[] getSupportedSizeForClass() throws CameraAccessException {
        if (mCameraCharacteristics == null) {
            LogHelper.e(TAG, "Can't get camera characteristics!");
            return null;
        }
        StreamConfigurationMap configMap =
                mCameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] availableSizes = configMap.getOutputSizes(android.media.MediaRecorder.class);
        Size[] highResAvailableSizes = null;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            highResAvailableSizes = configMap.getHighResolutionOutputSizes(ImageFormat.PRIVATE);
        }
        if (highResAvailableSizes != null && highResAvailableSizes.length > 0) {
            Size[] allSizes = new Size[availableSizes.length + highResAvailableSizes.length];
            System.arraycopy(availableSizes, 0, allSizes, 0,
                    availableSizes.length);
            System.arraycopy(highResAvailableSizes, 0, allSizes, availableSizes.length,
                    highResAvailableSizes.length);
            availableSizes = allSizes;
        }
        return availableSizes;
    }

    /**
     * Send request when setting value is changed.
     */
    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.requestRestartSession();
    }
}
