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

import android.app.Activity;
import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.List;

/**
 * Configure picture size parameters on API1.
 */

public class PictureSizeParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(PictureSizeParametersConfig.class.getSimpleName());
    private PictureSize mPictureSize;
    private ISettingManager.SettingDeviceRequester mDeviceRequester;

    /**
     * Picture size parameters configure constructor.
     *
     * @param pictureSize The instance of {@link PictureSize}.
     * @param deviceRequester The instance of {@link ISettingManager.SettingDeviceRequester}.
     */
    public PictureSizeParametersConfig(PictureSize pictureSize,
                                       ISettingManager.SettingDeviceRequester deviceRequester) {
        mPictureSize = pictureSize;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        List<Camera.Size> supportedSizes = originalParameters.getSupportedPictureSizes();
        sortSizeInDescending(supportedSizes);

        List<String> supportedSizesInStr = sizeToStr(supportedSizes);
        mPictureSize.onValueInitialized(supportedSizesInStr);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        String value = mPictureSize.getValue();
        LogHelper.d(TAG, "[configParameters], value:" + value);
        if (value != null) {
            int index = value.indexOf('x');
            int width = Integer.parseInt(value.substring(0, index));
            int height = Integer.parseInt(value.substring(index + 1));
            parameters.setPictureSize(width, height);
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mPictureSize.getKey());
    }

    private List<String> sizeToStr(List<Camera.Size> sizes) {
        List<String> sizeInStr = new ArrayList<>(sizes.size());
        for (Camera.Size size : sizes) {
            sizeInStr.add(size.width + "x" + size.height);
        }
        return sizeInStr;
    }

    /**
     * Get the max size as default value of picture size.
     *
     * @return getDefaultValue.
     */
    private String getDefaultPictureSize() {
        String defaultSize = mPictureSize.getSupportedPlatformValues().get(0);
        return defaultSize;
    }

    private void sortSizeInDescending(List<Camera.Size> sizes) {
        for (int i = 0; i < sizes.size(); i++) {
            Camera.Size maxSize = sizes.get(i);
            int maxIndex = i;
            for (int j = i + 1; j < sizes.size(); j++) {
                Camera.Size tempSize = sizes.get(j);
                if (tempSize.width * tempSize.height
                        > maxSize.width * maxSize.height) {
                    maxSize = tempSize;
                    maxIndex = j;
                }
            }
            Camera.Size firstSize = sizes.get(i);
            sizes.set(i, maxSize);
            sizes.set(maxIndex, firstSize);
        }
    }
}
