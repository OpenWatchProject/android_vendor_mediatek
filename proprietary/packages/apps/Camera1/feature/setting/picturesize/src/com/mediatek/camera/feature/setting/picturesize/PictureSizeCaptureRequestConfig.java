/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.picturesize;

import android.graphics.ImageFormat;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;

/**
 * Configure picture size in capture request in camera api2.
 */

public class PictureSizeCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(PictureSizeCaptureRequestConfig.class.getSimpleName());

    private PictureSize mPictureSize;
    private SettingDevice2Requester mDevice2Requester;
    /**
     * Picture size capture request configure constructor.
     *
     * @param pictureSize The instance of {@link PictureSize}.
     * @param device2Requester The instance of {@link SettingDevice2Requester}.
     */
    public PictureSizeCaptureRequestConfig(PictureSize pictureSize,
                                           SettingDevice2Requester device2Requester) {
        mPictureSize = pictureSize;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        StreamConfigurationMap s = characteristics
                .get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        List<Size> supportedSizes = getSupportedPictureSize(s, ImageFormat.JPEG);
        sortSizeInDescending(supportedSizes);

        List<String> supportedSizesInStr = sizeToStr(supportedSizes);
        mPictureSize.onValueInitialized(supportedSizesInStr);
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

    @Override
    public void sendSettingChangeRequest() {

    }

    private List<Size> getSupportedPictureSize(StreamConfigurationMap s, int format) {
        if (s == null) {
            return null;
        }
        List<Size> supportedValues = new ArrayList<>();
        Size[] highSizes = s.getHighResolutionOutputSizes(format);
        if (highSizes != null) {
            for (Size size : highSizes) {
                supportedValues.add(size);
            }
        }

        Size[] sizes = s.getOutputSizes(format);
        if (sizes != null) {
            for (Size size : sizes) {
                supportedValues.add(size);
            }
        }
        return supportedValues;
    }

    private List<String> sizeToStr(List<Size> sizes) {
        List<String> sizeInStr = new ArrayList<>(sizes.size());
        for (Size size : sizes) {
            sizeInStr.add(size.getWidth() + "x" + size.getHeight());
        }
        return sizeInStr;
    }

    private void sortSizeInDescending(List<Size> sizes) {
        for (int i = 0; i < sizes.size(); i++) {
            Size maxSize = sizes.get(i);
            int maxIndex = i;
            for (int j = i + 1; j < sizes.size(); j++) {
                Size tempSize = sizes.get(j);
                if (tempSize.getWidth() * tempSize.getHeight()
                        > maxSize.getWidth() * maxSize.getHeight()) {
                    maxSize = tempSize;
                    maxIndex = j;
                }
            }
            Size firstSize = sizes.get(i);
            sizes.set(i, maxSize);
            sizes.set(maxIndex, firstSize);
        }
    }
}
