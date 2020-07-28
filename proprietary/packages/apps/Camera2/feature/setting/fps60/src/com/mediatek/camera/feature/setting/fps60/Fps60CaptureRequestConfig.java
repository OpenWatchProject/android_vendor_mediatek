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

package com.mediatek.camera.feature.setting.fps60;

import android.annotation.TargetApi;
import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.media.CamcorderProfile;
import android.os.Build;
import android.util.Range;
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
 * This is for 60 fps flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class Fps60CaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            Fps60CaptureRequestConfig.class.getSimpleName());
    private static final String FPS60_OFF = "off";
    private static final String FPS60_ON = "on";
    private static final int FPS60_MODE_OFF = 0;
    private static final int FPS60_MODE_ON = 1;

    private static final String RECORD_60FPS_MODES
            = "com.mediatek.streamingfeature.availableHfpsModes";
    private static final String RECORD_60FPS_MODE
            = "com.mediatek.streamingfeature.hfpsMode";
    private static final int QUALITY_4K_WIDTH = 3840;
    private static final int QUALITY_4K_HEIGHT = 2160;
    private static final int FPS_60 = 60;
    private CameraCharacteristics.Key<int[]> mAvailableRecordModes;
    private CaptureRequest.Key<int[]> mRecordModeKey;


    private static final int[] CAM_FPS60_SESSION_PARAMETER_OFF = new int[]{0};
    private static final int[] CAM_FPS60_SESSION_PARAMETER_ON = new int[]{1};

    private SettingDevice2Requester mDevice2Requester;
    private CameraCharacteristics mCameraCharacteristics;
    private Fps60 mFps60;
    private CaptureRequest.Key<int[]> mKeyFps60SessionParameter;
    private CameraCharacteristics.Key<int[]> mKeyAvaliableHfpsEisMaxResolutions;
    private Context mContext;

    /**
     * Fps60 capture request configure constructor.
     *
     * @param fps60              The instance of {@link Fps60}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     * @param context          The application context.
     */
    public Fps60CaptureRequestConfig(Fps60 fps60, SettingDevice2Requester device2Requester,
                                     Context context) {
        mContext = context;
        mFps60 = fps60;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mCameraCharacteristics = characteristics;
        //initFps60VendorKey(characteristics); // no need, include in updateSupportedValues
        updateSupportedValues();
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        String fps60Value = mFps60.getValue();
        LogHelper.d(TAG, "configCaptureRequest fps60 to " + fps60Value);

            configFps60SessionRequestParameter(captureBuilder, fps60Value);
            configFpsParameter(captureBuilder, fps60Value);

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
        List<String> supportedList = new ArrayList<>();

        List<CameraCharacteristics.Key<?>> keys = mCameraCharacteristics.getKeys();
        for (CameraCharacteristics.Key<?> key : keys) {
            if (RECORD_60FPS_MODES.equals(key.getName())) {
                LogHelper.i(TAG, "TAG get record 60fps modes success");
                mAvailableRecordModes = (CameraCharacteristics.Key<int[]>) key;
                break;
            }
        }
        List<CaptureRequest.Key<?>> keys1 = mCameraCharacteristics.getAvailableCaptureRequestKeys();
        for (CaptureRequest.Key<?> key : keys1) {
            if (RECORD_60FPS_MODE.equals(key.getName())) {
                LogHelper.i(TAG, "TAG get record 60fps request key success");
                mKeyFps60SessionParameter = (CaptureRequest.Key<int[]>) key;
                break;
            }
        }
        if (mAvailableRecordModes != null) {
            int[] availableStates = getValueFromKey(mAvailableRecordModes);
            if (availableStates != null) {
                for (int mode : availableStates) {
                    if (mode == FPS60_MODE_OFF) {
                        supportedList.add(FPS60_OFF);
                    }
                    if (mode == FPS60_MODE_ON) {
                        supportedList.add(FPS60_ON);
                        LogHelper.i(TAG, "TAG get record 60fps support success");
                    }
                }
            }
        }
        LogHelper.d(TAG, "[updateSupportedValues] supportedList " + supportedList);
        mFps60.setSupportedPlatformValues(supportedList);
        mFps60.setEntryValues(supportedList);
        mFps60.setSupportedEntryValues(supportedList);
        mFps60.updateValue(FPS60_OFF);
    }

    private void initFps60VendorKey(CameraCharacteristics cs) {
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mFps60.getCameraId()));
        if (deviceDescription != null) {
            mKeyFps60SessionParameter = deviceDescription.getKeyFps60RequsetSessionParameter();
            LogHelper.d(TAG, "mKeyFps60SessionParameter = " + mKeyFps60SessionParameter);
        }
    }

    private void configFps60SessionRequestParameter(CaptureRequest.Builder captureBuilder,
                                                    String value) {
        if (mKeyFps60SessionParameter == null) {
            LogHelper.i(TAG,
                    "[configFps60SessionRequestParameter] mKeyFps60SessionParameter is null");
            return;
        }
        if ("on".equals(value)) {
            captureBuilder.set(mKeyFps60SessionParameter, CAM_FPS60_SESSION_PARAMETER_ON);
        } else {
            captureBuilder.set(mKeyFps60SessionParameter, CAM_FPS60_SESSION_PARAMETER_OFF);
        }
    }

    private void configFpsParameter(CaptureRequest.Builder captureBuilder,
                                    String value) {
        if ("on".equals(value)) {
            Range aeFps = new Range(60, 60);
            captureBuilder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, aeFps);
        } else {
            LogHelper.i(TAG, "[configFps60RequestParameter] don't set fps becuase 60fps off");
        }
    }

    private <T> T getValueFromKey(CameraCharacteristics.Key<T> key) {
        T value = null;
        try {
            value = mCameraCharacteristics.get(key);
            if (value == null) {
                LogHelper.e(TAG, key.getName() + "was null");
            }
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, key.getName() + " was not supported by this device");
        }
        return value;
    }
    private int [] currentVideoSize() {
        String currentValue = mFps60.getSettingController().queryValue("key_video_quality");
        LogHelper.d(TAG, "currentVideoSize = " + currentValue);
        if(currentValue == null){
            return new int[]{0,0};
        }
        CamcorderProfile cm = CamcorderProfile.get(Integer.parseInt(currentValue));
        LogHelper.d(TAG, "currentVideoSize width =  " + cm.videoFrameWidth
                + " | height = "+ cm.videoFrameHeight);
        return new int[]{cm.videoFrameWidth ,cm.videoFrameHeight };
    }
    boolean isSupportEISQuality() {
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mFps60.getCameraId()));
        if (deviceDescription != null) {
            mKeyAvaliableHfpsEisMaxResolutions = deviceDescription.getKeyAvaliableHfpsEisMaxResolutions();
            if(mKeyAvaliableHfpsEisMaxResolutions!=null) {
                LogHelper.d(TAG, "mKeyAvaliableHfpsEisMaxResolutions = " + mKeyAvaliableHfpsEisMaxResolutions);
                int[] keys = mCameraCharacteristics.get(mKeyAvaliableHfpsEisMaxResolutions);
                if (keys == null) {
                    return false;
                }
                LogHelper.d(TAG, "getMaxEISSizeInFps width =  " + keys[0]
                        + " | height = "+ keys[1]);
                if(keys[0] * keys[1] >= currentVideoSize()[0] * currentVideoSize()[1]){
                    LogHelper.d(TAG, "is SupportEISQuality in 60fps");
                    return true;
                }else {
                    LogHelper.d(TAG, "is not SupportEISQuality in 60fps");
                    return false;
                }

            }else {
                LogHelper.d(TAG, "mKeyAvaliableHfpsEisMaxResolutions = null");
            }
        }
        return false;
    }

}
