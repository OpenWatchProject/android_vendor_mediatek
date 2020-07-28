/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.setting.format;

import android.annotation.TargetApi;
import android.graphics.ImageFormat;
import android.hardware.camera2.*;
import android.hardware.camera2.params.StreamConfigurationMap;

import android.os.Build;

import android.util.Size;
import android.view.Surface;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.List;


/**
 * This is for dng capture flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class FormatCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure
         {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            FormatCaptureRequestConfig.class.getSimpleName());
    private ISettingManager.SettingDevice2Requester mSettingDevice2Requester;

    /**
     * Constructor of format set config in api2.
     * @param settingDevice2Requester device requester.
     */
    public FormatCaptureRequestConfig(ISettingManager.SettingDevice2Requester
                                           settingDevice2Requester) {
        mSettingDevice2Requester = settingDevice2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        StreamConfigurationMap s = characteristics
                .get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        List<Size> supportedSizes = getSupportedPictureSize(s, ImageFormat.YUV_420_888);
        int lengh = supportedSizes.size();
        for (int i = 0; i < lengh; i++) {
            Size size = supportedSizes.get(i);
        }
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        LogHelper.d(TAG, "[configCaptureRequest] " );
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {
/*        if (Format.FORMAT_JPEG.equalsIgnoreCase(mFormat)) {
            surfaces.add(configJpegSurface());
            releaseHeifSurface();
        } else if (Format.FORMAT_HEIF.equalsIgnoreCase(mFormat)){
            surfaces.add(configHeifSurface());
            releaseHeifSurface();
        }*/
    }
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;//mCaptureCallback;
    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        LogHelper.i(TAG, "[sendSettingChangeRequest] ");
        mSettingDevice2Requester.requestRestartSession();
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
                LogHelper.d(TAG, "[getSupportedPictureSize] high resolution supportedSizes : " +
                        size);
            }
        }

        Size[] sizes = s.getOutputSizes(format);
        if (sizes != null) {
            for (Size size : sizes) {
                supportedValues.add(size);
                LogHelper.d(TAG, "[getSupportedPictureSize] supportedSizes : " + size);
            }
        }
        return supportedValues;
    }
}
