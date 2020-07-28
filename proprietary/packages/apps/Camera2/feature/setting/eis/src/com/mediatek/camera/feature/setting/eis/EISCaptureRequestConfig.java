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

package com.mediatek.camera.feature.setting.eis;

import android.annotation.TargetApi;

import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;


/**
 * This is for EIS capture flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class EISCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            EISCaptureRequestConfig.class.getSimpleName());
    private static final String EIS_OFF = "off";
    private static final String EIS_ON = "on";
    private static final int EIS_MODE_OFF = 0;
    private static final int EIS_MODE_ON = 1;
    private static final int[] CAM_EIS_SESSION_PARAMETER_OFF = new int[]{0};
    private static final int[] CAM_EIS_SESSION_PARAMETER_ON = new int[]{1};
    private static final int[] MTK_EIS_FEATURE_PREVIEW_EIS_OFF = new int[]{0};
    private static final int[] MTK_EIS_FEATURE_PREVIEW_EIS_ON = new int[]{1};
    private SettingDevice2Requester mDevice2Requester;
    private CameraCharacteristics mCameraCharacteristics;
    private EIS mEis;
    private CaptureRequest.Key<int[]> mKeyEisSessionParameter;
    private CaptureRequest.Key<int[]> mKeyPreviewEisParameter;
    private Context mContext;

    /**
     * EIS capture request configure constructor.
     * @param eis The instance of {@link EIS}.
     * @param device2Requester  The implementer of {@link SettingDevice2Requester}.
     * @param context The application context.
     */
    public EISCaptureRequestConfig(EIS eis, SettingDevice2Requester device2Requester,
                                   Context context) {
        mContext = context;
        mEis = eis;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mCameraCharacteristics = characteristics;
        initEisVendorKey(characteristics);
        updateSupportedValues();
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        String eisValue = mEis.getValue();
        LogHelper.d(TAG, "configCaptureRequest EIS to " + eisValue);
        if ("on".equals(eisValue)) {
            captureBuilder.set(CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE,
                    CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE_ON);
        } else {
            captureBuilder.set(CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE,
                    CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE_OFF);
        }
        configEisSessionRequestParameter(captureBuilder, eisValue);
        configPreviewEisParameter(captureBuilder, eisValue);
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.requestRestartSession();
    }

    private void updateSupportedValues() {
        if (mCameraCharacteristics == null) {
            return;
        }
        int[] availableEisModes = mCameraCharacteristics.get(
                CameraCharacteristics.CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES);
        if (availableEisModes == null || availableEisModes.length == 0) {
            LogHelper.i(TAG, "[updateSupportedValues] EIS is not supported with " +
                    "availableEisModes" + availableEisModes);
            return;
        }
        List<String> supportedList = new ArrayList<>();
        for (int mode : availableEisModes) {
            if (mode == EIS_MODE_OFF) {
                supportedList.add(EIS_OFF);
            }
            if (mode == EIS_MODE_ON) {
                supportedList.add(EIS_ON);
            }
        }
        LogHelper.d(TAG, "[updateSupportedValues] supportedList " + supportedList);
        mEis.setSupportedPlatformValues(supportedList);
        mEis.setEntryValues(supportedList);
        mEis.setSupportedEntryValues(supportedList);
    }

    private void initEisVendorKey(CameraCharacteristics cs) {
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mEis.getCameraId()));
        if (deviceDescription != null) {
            mKeyEisSessionParameter = deviceDescription.getKeyEisRequsetSessionParameter();
            mKeyPreviewEisParameter = deviceDescription.getKeyPreviewEisParameter();
            LogHelper.d(TAG, "mKeyEisSessionParameter = " + mKeyEisSessionParameter +
                    ",mKeyEisSessionParameter  = " + mKeyEisSessionParameter);
        }
    }

    private void configEisSessionRequestParameter(CaptureRequest.Builder captureBuilder,
                                                  String value) {
        if (mKeyEisSessionParameter == null) {
            LogHelper.w(TAG, "[configEisSessionRequestParameter] mKeyEisSessionParameter is null");
            return;
        }
        if ("on".equals(value)) {
            captureBuilder.set(mKeyEisSessionParameter, CAM_EIS_SESSION_PARAMETER_ON);
        } else {
            captureBuilder.set(mKeyEisSessionParameter, CAM_EIS_SESSION_PARAMETER_OFF);
        }
    }

    private void configPreviewEisParameter(CaptureRequest.Builder captureBuilder,
                                           String value) {
        if (mKeyPreviewEisParameter == null) {
            LogHelper.w(TAG, "[configPreviewEisParameter] mKeyPreviewEisParameter is null");
            return;
        }
        if ("on".equals(value)) {
            captureBuilder.set(mKeyPreviewEisParameter, MTK_EIS_FEATURE_PREVIEW_EIS_ON);
        } else {
            captureBuilder.set(mKeyPreviewEisParameter, MTK_EIS_FEATURE_PREVIEW_EIS_OFF);
        }
    }

}
