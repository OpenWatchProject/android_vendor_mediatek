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
package com.mediatek.camera.feature.setting.iso;

import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.util.Range;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;

/**
 * Configure iso in capture request in camera api2.
 */

public class ISOCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(ISOCaptureRequestConfig.class.getSimpleName());
    private static int sIndex = 2;
    private static final String ISO_KEY_CONTROL_SPEED = "com.mediatek.3afeature.aeIsoSpeed";
    private ISO mIso;
    private ISettingManager.SettingDevice2Requester mDevice2Requester;
    private CaptureRequest.Key<int[]> mKeyIsoRequestValue;
    private Context mContext;

    /**
     * Shutter speed auto.
     */
    public static final String AUTO = "0";

    /**
     * ISO capture request configure constructor.
     *
     * @param iso The instance of {@link ISO}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     * @param context The camera context.
     */
    public ISOCaptureRequestConfig(ISO iso,
                                   SettingDevice2Requester device2Requester,
                                   Context context) {
        mContext = context;
        mIso = iso;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        initIsoVendorKey(characteristics);
        List<String> supportIsoList = getSupportedList(characteristics);
        String defaultValue = AUTO;
        mIso.onValueInitialized(supportIsoList, defaultValue);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        String value = mIso.getValue();
        LogHelper.d(TAG, "[configCaptureRequest], value:" + value);
        if (value != null && captureBuilder != null) {
            int[] mode = new int[1];
            mode[0] = Integer.parseInt(value);
            captureBuilder.set(mKeyIsoRequestValue, mode);
        }
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.createAndChangeRepeatingRequest();
    }

    private void initIsoVendorKey(CameraCharacteristics cs) {
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mIso.getCameraId()));
        if (deviceDescription != null) {
            mKeyIsoRequestValue = deviceDescription.getKeyIsoRequestMode();
        }

        LogHelper.d(TAG, "mKeyIsoRequestValue = " + mKeyIsoRequestValue);
    }

    /**
     * Get supported ISO list.
     *
     * @param characteristics THe Characteristics.
     * @return The supported ISO list.
     */
    public List<String> getSupportedList(CameraCharacteristics characteristics) {
        Integer minIso = getMinIsoValue(characteristics);
        Integer maxIso = getMaxIsoValue(characteristics);
        LogHelper.d(TAG, "[getSupportedList] ISO range (" + minIso + ", " + maxIso + ")");
        ArrayList<String> values = new ArrayList<String>();
        if (mKeyIsoRequestValue != null) {
            values.add(AUTO);
            int minValue = Integer.parseInt(String.valueOf(minIso));
            minValue = minValue % 100 == 0 ? minValue : (minValue / 100 + 1) * 100;
            int maxValue = Integer.parseInt(String.valueOf(maxIso));
            // ISO items shows in setting is mixValue power of 2.
            for (int i = minValue; i <= maxValue; i = i * sIndex) {
                values.add(String.valueOf(i));
            }
        }
        LogHelper.d(TAG, "[getSupportedList] values = " + values);
        return values;
    }
    /**
     * Get the minimum ISO.
     *
     * @param characteristics The current CameraCharacteristics.
     * @return The minimum ISO.
     */
    private static Integer getMinIsoValue(CameraCharacteristics characteristics) {
        Range<Integer> isoRange =
                characteristics.get(CameraCharacteristics.SENSOR_INFO_SENSITIVITY_RANGE);
        Integer minIso = isoRange.getLower();
        return minIso;
    }

    /**
     * Get the maximum ISO.
     *
     * @param characteristics The current CameraCharacteristics.
     * @return The maximum ISO.
     */
    private static Integer getMaxIsoValue(CameraCharacteristics characteristics) {
        Range<Integer> isoRange =
                characteristics.get(CameraCharacteristics.SENSOR_INFO_SENSITIVITY_RANGE);
        Integer maxIso = isoRange.getUpper();
        return maxIso;
    }
}
