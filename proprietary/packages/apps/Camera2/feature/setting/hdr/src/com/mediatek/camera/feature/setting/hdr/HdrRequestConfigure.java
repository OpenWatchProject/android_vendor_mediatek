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
package com.mediatek.camera.feature.setting.hdr;

import android.annotation.TargetApi;
import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import junit.framework.Assert;

import java.util.ArrayList;
import java.util.List;

/**
 * This class used to configure hdr value to capture request.
 * and configure those settings value which have restriction with hdr but
 * without ui
 * and just used for api2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class HdrRequestConfigure implements ICameraSetting.ICaptureRequestConfigure, IHdr.Listener {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(HdrRequestConfigure.class.getSimpleName());
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_OFF = new int[]{0};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_ON = new int[]{1};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_AUTO = new int[]{2};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON = new int[]{3};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO = new int[]{4};
    private static final int[] CAM_HDR_AUTO_DETECTION_ON = new int[]{1};
    private static final String HDR_AUTO_VALUE = "auto";
    private static final String HDR_OFF_VALUE = "off";
    private static final String HDR_ON_VALUE = "on";
    private static final String HDR_KEY_AVAILABLE_HDR_MODES_PHOTO =
            "com.mediatek.hdrfeature.availableHdrModesPhoto";
    private static final String HDR_KEY_AVAILABLE_HDR_MODES_VIDEO =
            "com.mediatek.hdrfeature.availableHdrModesVideo";
    private static final String HDR_KEY_DETECTION_RESULT =
            "com.mediatek.hdrfeature.hdrDetectionResult";
    private static final String HDR_KEY_DETECTION_MODE =
            "com.mediatek.hdrfeature.hdrMode";
    private static final String HDR_KEY_SESSION_MODE =
            "com.mediatek.hdrfeature.SessionParamhdrMode";
    private static final String HDR_KEY_MSHDR_AVAILABLE_MODE =
            "com.mediatek.hdrfeature.availableMStreamHdrModes";
    private static final int MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE = 2;
    private static final int MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW = 6;
    private CameraCharacteristics.Key<int[]> mKeyHdrAvailablePhotoModes;
    private CameraCharacteristics.Key<int[]> mKeyHdrAvailableVideoModes;
    private CaptureResult.Key<int[]> mKeyHdrDetectionResult;
    private CaptureRequest.Key<int[]> mKeyHdrRequestMode;
    private CaptureRequest.Key<int[]> mKeyHdrRequsetSessionMode;
    private CameraCharacteristics.Key<int[]> mKeyMSHdrAvailableMode;
    private Context mContext;
    private Hdr mHdr;
    private int mCameraId = -1;
    private int mLastHdrDetectionValue = -1;
    private boolean mIsHdrSupported = false;
    private boolean mIsVendorHdrSupported = false;
    private boolean mIsSensorDetectionHdrOnSupported = false;
    private boolean mIsSensorDetectionHdrAutoSupported = false;
    private boolean mIsMStreamHdrSupported = false;
    private ISettingManager.SettingDevice2Requester mSettingDevice2Requester;
    private String mHdrSessionValue = "off";

    /**
     * The construction function.
     *
     * @param hdr the Hdr class object.
     * @param requester setting device request to send value changed request.
     */
    public HdrRequestConfigure(Hdr hdr,
                               ISettingManager.SettingDevice2Requester requester, Context context) {
        mContext = context;
        mHdr = hdr;
        mSettingDevice2Requester = requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mIsHdrSupported = false;
        mIsVendorHdrSupported = false;
        initHdrVendorKey(characteristics);
        if (mHdr.getCurrentModeType() == ICameraMode.ModeType.VIDEO) {
            initHdrVideoSettingValues(characteristics);
        } else {
            initHdrPhotoSettingValues(characteristics);
        }
        /*hdr need keep the state before switch mode. if the state is not supported in new mode,
          set hdr to off. if hdr is not supported in the new mode, keep the state for restore state
          when mode switch.
        */
        if (mHdr.getEntryValues().size() > 1 && !mHdr.getEntryValues().contains(mHdr.getValue())) {
            mHdr.setValue(HDR_OFF_VALUE);
        } else if (mHdr.getEntryValues().size() <= 1) {
            mHdr.resetRestriction();
        }
        mHdrSessionValue = mHdr.getValue();
        if (mKeyMSHdrAvailableMode != null) {
            int[] smhdrModes = characteristics.get(mKeyMSHdrAvailableMode);
            if (smhdrModes != null && smhdrModes.length >= 1 &&
                    (smhdrModes[0] == MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE ||
                    smhdrModes[0] == MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW)) {
                mIsMStreamHdrSupported = true;
            }
        }
        LogHelper.d(TAG, "[setCameraCharacteristics], mIsHdrSupported = " + mIsHdrSupported);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        if (mHdr.getEntryValues().size() <= 1) {
            return;
        }
        LogHelper.d(TAG, "[configCaptureRequest], value = " + mHdr.getValue());
        int controlMode;
        if (HDR_ON_VALUE.equals(mHdr.getValue()) || HDR_AUTO_VALUE.equals(mHdr.getValue())) {
            controlMode = CaptureRequest.CONTROL_MODE_USE_SCENE_MODE;
            captureBuilder.set(CaptureRequest.CONTROL_MODE, controlMode);
            captureBuilder.set(CaptureRequest.CONTROL_SCENE_MODE,
                    CameraMetadata.CONTROL_SCENE_MODE_HDR);
        }
        if (!mIsVendorHdrSupported) {
            return;
        }
        if (mHdr.getCurrentModeType() == ICameraMode.ModeType.VIDEO) {
            configVideoCaptureRequest(captureBuilder);
        } else {
            configPhotoCaptureRequest(captureBuilder);
        }
        configHdrSessionRequest(captureBuilder);
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
        return mPreviewCallback;
    }

    @Override
    public void sendSettingChangeRequest() {
        mSettingDevice2Requester.createAndChangeRepeatingRequest();
    }

    @Override
    public void onPreviewStateChanged(boolean isPreviewStarted) {
        // no need in API2
    }

    @Override
    public void onHdrValueChanged() {
        mSettingDevice2Requester.createAndChangeRepeatingRequest();
    }

    @Override
    public void updateModeDeviceState(String newState) {
    }

    @Override
    public void setCameraId(int id) {
        mCameraId = id;
    }

    @Override
    public boolean isZsdHdrSupported() {
        return false;
    }

    @Override
    public boolean isMStreamHDRSupported() {
        return mIsMStreamHdrSupported;
    }

    private CameraCaptureSession.CaptureCallback mPreviewCallback
            = new CameraCaptureSession.CaptureCallback() {

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                                       TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            Assert.assertNotNull(result);
            //auto result
            if (!HDR_AUTO_VALUE.equals(mHdr.getValue())) {
                return;
            }
            List<CaptureResult.Key<?>> keyList = result.getKeys();
            for (CaptureResult.Key<?> key : keyList) {
                if (key.getName().equals(HDR_KEY_DETECTION_RESULT)) {
                    int[] value = result.get(mKeyHdrDetectionResult);
                    if (value[0] == mLastHdrDetectionValue) {
                        // do not update result every times
                        return;
                    }
                    LogHelper.d(TAG, "onCaptureCompleted, value: " + value[0]);
                    if (value[0] == CAM_HDR_AUTO_DETECTION_ON[0]) {
                        mHdr.onAutoDetectionResult(true);
                    } else {
                        mHdr.onAutoDetectionResult(false);
                    }
                    mLastHdrDetectionValue = value[0];
                    break;
                }
            }
        }
    };

    private void configVideoCaptureRequest(CaptureRequest.Builder captureBuilder) {
        Assert.assertNotNull(mKeyHdrRequestMode);
        if (HDR_ON_VALUE.equals(mHdr.getValue())) {
            captureBuilder.set(mKeyHdrRequestMode,
                    CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON);
        } else if (HDR_AUTO_VALUE.equals(mHdr.getValue())) {
            captureBuilder.set(mKeyHdrRequestMode,
                    CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO);
        } else {
            captureBuilder.set(mKeyHdrRequestMode,
                    CAM_HDR_FEATURE_HDR_MODE_OFF);
        }
    }

    private void configPhotoCaptureRequest(CaptureRequest.Builder captureBuilder) {
        Assert.assertNotNull(mKeyHdrRequestMode);
        if (HDR_ON_VALUE.equals(mHdr.getValue())) {
            if (mIsSensorDetectionHdrOnSupported) {
                captureBuilder.set(mKeyHdrRequestMode,
                        CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON);
            } else {
                captureBuilder.set(mKeyHdrRequestMode,
                        CAM_HDR_FEATURE_HDR_MODE_ON);
            }
        } else if (HDR_AUTO_VALUE.equals(mHdr.getValue())) {
            if (mIsSensorDetectionHdrAutoSupported) {
                captureBuilder.set(mKeyHdrRequestMode,
                        CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO);
            } else {
                captureBuilder.set(mKeyHdrRequestMode,
                        CAM_HDR_FEATURE_HDR_MODE_AUTO);
            }
        } else {
            captureBuilder.set(mKeyHdrRequestMode,
                    CAM_HDR_FEATURE_HDR_MODE_OFF);
        }
    }

    private void configHdrSessionRequest(CaptureRequest.Builder captureBuilder) {
        if (mKeyHdrRequsetSessionMode == null) {
            LogHelper.w(TAG, "[configHdrSessionRequest] mKeyHdrRequsetSessionMode is null");
            return;
        }
        LogHelper.d(TAG, "[configHdrSessionRequest] currrent mode "
                + mHdr.getCurrentModeType() + ",mHdrSessionValue " + mHdrSessionValue);
        if (mHdr.getCurrentModeType() == ICameraMode.ModeType.VIDEO) {
            if (HDR_ON_VALUE.equals(mHdrSessionValue)) {
                captureBuilder.set(mKeyHdrRequsetSessionMode,
                        CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON);
            } else if (HDR_AUTO_VALUE.equals(mHdrSessionValue)) {
                captureBuilder.set(mKeyHdrRequsetSessionMode,
                        CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO);
            } else {
                captureBuilder.set(mKeyHdrRequsetSessionMode,
                        CAM_HDR_FEATURE_HDR_MODE_OFF);
            }
            return;
        }
        if (HDR_ON_VALUE.equals(mHdrSessionValue)) {
            if (mIsSensorDetectionHdrOnSupported) {
                captureBuilder.set(mKeyHdrRequsetSessionMode,
                        CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON);
            } else {
                captureBuilder.set(mKeyHdrRequsetSessionMode,
                        CAM_HDR_FEATURE_HDR_MODE_ON);
            }
        } else if (HDR_AUTO_VALUE.equals(mHdrSessionValue)) {
            if (mIsSensorDetectionHdrAutoSupported) {
                captureBuilder.set(mKeyHdrRequsetSessionMode,
                        CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO);
            } else {
                captureBuilder.set(mKeyHdrRequsetSessionMode,
                        CAM_HDR_FEATURE_HDR_MODE_AUTO);
            }
        } else {
            captureBuilder.set(mKeyHdrRequsetSessionMode,
                    CAM_HDR_FEATURE_HDR_MODE_OFF);
        }
    }

    private void initHdrVendorKey(CameraCharacteristics cs) {
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mCameraId));
        if (deviceDescription != null) {
            mKeyHdrAvailablePhotoModes = deviceDescription.getKeyHdrAvailablePhotoModes();
            mKeyHdrAvailableVideoModes = deviceDescription.getKeyHdrAvailableVideoModes();
            mKeyHdrDetectionResult = deviceDescription.getKeyHdrDetectionResult();
            mKeyHdrRequestMode = deviceDescription.getKeyHdrRequestMode();
            mKeyHdrRequsetSessionMode = deviceDescription.getKeyHdrRequsetSessionMode();
            mKeyMSHdrAvailableMode = deviceDescription.getKeyMSHDRAvailableModes();
            LogHelper.i(TAG, "initHdrVendorKey init vendor key from device spec mCameraId: "
                    + mCameraId);
            return;
        }
        List<CameraCharacteristics.Key<?>> keyList = cs.getKeys();
        for (CameraCharacteristics.Key<?> key : keyList) {
            if (key.getName().equals(HDR_KEY_AVAILABLE_HDR_MODES_PHOTO)) {
                mKeyHdrAvailablePhotoModes = (CameraCharacteristics.Key<int[]>) key;
            } else if (key.getName().equals(HDR_KEY_AVAILABLE_HDR_MODES_VIDEO)) {
                mKeyHdrAvailableVideoModes = (CameraCharacteristics.Key<int[]>) key;
            } else if (key.getName().equals(HDR_KEY_MSHDR_AVAILABLE_MODE)) {
                mKeyMSHdrAvailableMode = (CameraCharacteristics.Key<int[]>) key;
            }
        }
        List<CaptureResult.Key<?>> resultKeyList = cs.getAvailableCaptureResultKeys();
        for (CaptureResult.Key<?> resultKey : resultKeyList) {
            if (resultKey.getName().equals(HDR_KEY_DETECTION_RESULT)) {
                mKeyHdrDetectionResult = (CaptureResult.Key<int[]>) resultKey;
            }
        }
        List<CaptureRequest.Key<?>> requestKeyList = cs.getAvailableCaptureRequestKeys();
        for (CaptureRequest.Key<?> requestKey : requestKeyList) {
            if (requestKey.getName().equals(HDR_KEY_DETECTION_MODE)) {
                mKeyHdrRequestMode = (CaptureRequest.Key<int[]>) requestKey;
            } else if (requestKey.getName().equals(HDR_KEY_SESSION_MODE)) {
                mKeyHdrRequsetSessionMode = (CaptureRequest.Key<int[]>) requestKey;
            }
        }
    }

    private void initHdrPhotoSettingValues(CameraCharacteristics cs) {
        List<String> supportedList = new ArrayList<>();
        mIsSensorDetectionHdrOnSupported = false;
        mIsSensorDetectionHdrAutoSupported = false;
        supportedList.add(HDR_OFF_VALUE);

        int[] supportSceneList = cs.get(
                CameraCharacteristics.CONTROL_AVAILABLE_SCENE_MODES);
        for (int value : supportSceneList) {
            if (value == CameraMetadata.CONTROL_SCENE_MODE_HDR) {
                supportedList.add(HDR_ON_VALUE);
                mIsHdrSupported = true;
                break;
            }
        }
        if (!mIsHdrSupported) {
            return;
        }
        LogHelper.d(TAG, "initHdrPhotoSettingValues ");

        int[] supportVendorList = null;
        if (mKeyHdrAvailablePhotoModes != null) {
            supportVendorList = cs.get(mKeyHdrAvailablePhotoModes);
        }
        if (supportVendorList != null) {
            for (int value : supportVendorList) {
                LogHelper.d(TAG, "photo support value: " + value);
                if (value == CAM_HDR_FEATURE_HDR_MODE_ON[0]) {
                    mIsVendorHdrSupported = true;
                } else if (value == CAM_HDR_FEATURE_HDR_MODE_AUTO[0]) {
                    supportedList.add(HDR_AUTO_VALUE);
                } else if (value == CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON[0]) {
                    mIsSensorDetectionHdrOnSupported = true;
                } else if (value == CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO[0]) {
                    mIsSensorDetectionHdrAutoSupported = true;
                }
            }
        }
        mHdr.setSupportedPlatformValues(supportedList);
        mHdr.setSupportedEntryValues(supportedList);
        mHdr.setEntryValues(supportedList);
    }

    private void initHdrVideoSettingValues(CameraCharacteristics cs) {
        LogHelper.d(TAG, "initHdrVideoSettingValues ");
        List<String> supportedList = new ArrayList<>();
        supportedList.add(HDR_OFF_VALUE);

        int[] supportVendorList = null;
        if (mKeyHdrAvailableVideoModes != null) {
            supportVendorList = cs.get(mKeyHdrAvailableVideoModes);
        }
        if (supportVendorList != null) {
            for (int value : supportVendorList) {
                LogHelper.d(TAG, "video support value: " + value);
                if (value == CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON[0]) {
                    supportedList.add(HDR_ON_VALUE);
                    mIsVendorHdrSupported = true;
                } else if (value == CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO[0]) {
                    supportedList.add(HDR_AUTO_VALUE);
                }
            }
        }
        mHdr.setSupportedPlatformValues(supportedList);
        mHdr.setSupportedEntryValues(supportedList);
        mHdr.setEntryValues(supportedList);
    }
}
