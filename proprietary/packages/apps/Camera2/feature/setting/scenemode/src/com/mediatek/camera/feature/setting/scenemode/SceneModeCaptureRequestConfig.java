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
package com.mediatek.camera.feature.setting.scenemode;

import android.app.Activity;
import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * Scene mode capture request configure.
 */

public class SceneModeCaptureRequestConfig implements
        ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            SceneModeCaptureRequestConfig.class.getSimpleName());

    private static final String OFF_VALUE = "off";
    private static final String ASD_AVAILABLE_MODES_KEY_NAME
            = "com.mediatek.facefeature.availableasdmodes";
    private static final String ASD_REQUEST_MODE_KEY_NAME
            = "com.mediatek.facefeature.asdmode";
    private static final String ASD_RESULT_MODE_KEY_NAME
            = "com.mediatek.facefeature.asdresult";

    private static final int MTK_FACE_FEATURE_ASD_MODE_OFF = 0;
    private static final int MTK_FACE_FEATURE_ASD_MODE_SIMPLE = 1;
    private static final int MTK_FACE_FEATURE_ASD_MODE_FULL = 2;

    private static final int MSG_ON_COMPLETED_CALLBACK = 0;

    private SceneMode mSceneMode;
    private SettingDevice2Requester mDevice2Requester;
    private CameraCharacteristics.Key<int[]> mAsdAvailableModesKey = null;
    private CaptureRequest.Key<int[]> mAsdRequestModeKey = null;
    private CaptureResult.Key<int[]> mAsdResultModeKey = null;
    private CameraCaptureSession.CaptureCallback mCaptureCallback;
    private String mDetectedSceneMode;
    private Handler mMainHandler;
    private Context mContext;

    /**
     * Scene mode enum value.
     */
    enum ModeEnum {
        OFF(0),
        FACE_PORTRAIT(1),
        ACTION(2),
        PORTRAIT(3),
        LANDSCAPE(4),
        NIGHT(5),
        NIGHT_PORTRAIT(6),
        THEATRE(7),
        BEACH(8),
        SNOW(9),
        SUNSET(10),
        STEADYPHOTO(11),
        FIREWORKS(12),
        SPORTS(13),
        PARTY(14),
        CANDLELIGHT(15),
        BARCODE(16),
        HIGH_SPEED_VIDEO(17),
        HDR(18),
        BACKLIGHT_PORTRAIT(32);

        private int mValue = 0;
        ModeEnum(int value) {
            this.mValue = value;
        }

        /**
         * Get enum value which is in integer.
         *
         * @return The enum value.
         */
        public int getValue() {
            return this.mValue;
        }

        /**
         * Get enum name which is in string.
         *
         * @return The enum name.
         */
        public String getName() {
            return this.toString();
        }
    }

    /**
     * Scene mode capture quest configure constructor.
     *
     * @param activity The camera activity.
     * @param sceneMode The instance of {@link SceneMode}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     * @param context The camera context.
     */
    public SceneModeCaptureRequestConfig(Activity activity,
                                         SceneMode sceneMode,
                                         SettingDevice2Requester device2Requester,
                                         Context context) {
        mSceneMode = sceneMode;
        mDevice2Requester = device2Requester;
        mMainHandler = new MainHandler(activity.getMainLooper());
        mContext = context;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        int[] scenes = characteristics.get(CameraCharacteristics.CONTROL_AVAILABLE_SCENE_MODES);

        List<String> sceneNames = convertEnumToString(scenes);
        if (!sceneNames.contains(OFF_VALUE)) {
            sceneNames.add(OFF_VALUE);
        }

        boolean isAsdSupported = true;
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                .getDeviceDescriptionMap().get(String.valueOf(mSceneMode.getCameraId()));
        if (deviceDescription != null) {
            mAsdAvailableModesKey = deviceDescription.getKeyAsdAvailableModes();
        }

        if (mAsdAvailableModesKey == null) {
            LogHelper.d(TAG, "available asd modes key isn't existed");
            isAsdSupported = false;
        }

        if (deviceDescription != null) {
            mAsdRequestModeKey = deviceDescription.getKeyAsdRequestMode();
        }

        if (mAsdRequestModeKey == null) {
            LogHelper.d(TAG, "asd request key isn't existed");
            isAsdSupported = false;
        }

        if (deviceDescription != null) {
            mAsdResultModeKey = deviceDescription.getKeyAsdResult();
        }

        if (mAsdResultModeKey == null) {
            LogHelper.d(TAG, "asd result key isn't existed");
            isAsdSupported = false;
        }

        if (isAsdSupported) {
            sceneNames.add(SceneMode.VALUE_OF_AUTO_SCENE_DETECTION);
        }
        mSceneMode.initializeValue(sceneNames, OFF_VALUE);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        String value = mSceneMode.getValue();
        LogHelper.d(TAG, "[configCaptureRequest], scene mode:" + value
                + ", mDetectedSceneMode:" + mDetectedSceneMode);

        if (SceneMode.VALUE_OF_AUTO_SCENE_DETECTION.equals(value)) {
            int[] asdMode = {MTK_FACE_FEATURE_ASD_MODE_SIMPLE};
            captureBuilder.set(mAsdRequestModeKey, asdMode);
            if (ModeEnum.HDR.getName().toLowerCase(Locale.ENGLISH).equals(mDetectedSceneMode)
                    || ModeEnum.BACKLIGHT_PORTRAIT.getName().replace('_', '-')
                    .toLowerCase(Locale.ENGLISH).equals(mDetectedSceneMode)
                    || ModeEnum.OFF.getName().toLowerCase(Locale.ENGLISH)
                    .equals(mDetectedSceneMode)) {
                LogHelper.d(TAG, "Special detected scene mode, actually set is: "
                        + ModeEnum.OFF.getValue());
                captureBuilder.set(CaptureRequest.CONTROL_SCENE_MODE,
                        ModeEnum.OFF.getValue());
                captureBuilder.set(CaptureRequest.CONTROL_MODE,
                        CaptureRequest.CONTROL_MODE_AUTO);
            } else {
                captureBuilder.set(CaptureRequest.CONTROL_SCENE_MODE,
                        convertStringToEnum(mDetectedSceneMode));
                captureBuilder.set(CaptureRequest.CONTROL_MODE,
                        CaptureRequest.CONTROL_MODE_USE_SCENE_MODE);
            }
        } else if (value != null && !ModeEnum.OFF.toString()
                .equalsIgnoreCase(value)) {
            captureBuilder.set(CaptureRequest.CONTROL_SCENE_MODE,
                    convertStringToEnum(value));
            captureBuilder.set(CaptureRequest.CONTROL_MODE,
                    CaptureRequest.CONTROL_MODE_USE_SCENE_MODE);
        }
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
        if (!SceneMode.VALUE_OF_AUTO_SCENE_DETECTION.equals(mSceneMode.getValue())) {
            return null;
        }
        if (mCaptureCallback == null) {
            mCaptureCallback =  new CameraCaptureSession.CaptureCallback() {
                @Override
                public void onCaptureCompleted(CameraCaptureSession session,
                                               CaptureRequest request,
                                               TotalCaptureResult result) {

                    int[] detectedScene = result.get(mAsdResultModeKey);
                    if (detectedScene == null) {
                        return;
                    }
                    mDetectedSceneMode = convertEnumToString(detectedScene[0]);
                    mMainHandler.removeMessages(MSG_ON_COMPLETED_CALLBACK);
                    mMainHandler.obtainMessage(MSG_ON_COMPLETED_CALLBACK,
                            mDetectedSceneMode).sendToTarget();
                }
            };
        }
        return mCaptureCallback;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.createAndChangeRepeatingRequest();
    }

    private List<String> convertEnumToString(int[] enumIndexs) {
        ModeEnum[] modes = ModeEnum.values();
        List<String> names = new ArrayList<>(enumIndexs.length);
        for (int i = 0; i < enumIndexs.length; i++) {
            int enumIndex = enumIndexs[i];
            for (ModeEnum mode : modes) {
                if (mode.getValue() == enumIndex) {
                    String name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                    names.add(name);
                    break;
                }
            }
        }
        return names;
    }

    private String convertEnumToString(int enumIndex) {
        String name = null;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            if (mode.getValue() == enumIndex) {
                name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                break;
            }
        }
        return name;
    }

    private int convertStringToEnum(String value) {
        int enumIndex = 0;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            String modeName = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
            if (modeName.equalsIgnoreCase(value)) {
                enumIndex = mode.getValue();
                break;
            }
        }
        return enumIndex;
    }

    /**
     * Handler to handle scene mode callback.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            String detectedSceneMode = (String) msg.obj;
            switch (msg.what) {
                case MSG_ON_COMPLETED_CALLBACK:
                    if (ModeEnum.HDR.getName().toLowerCase(Locale.ENGLISH)
                        .equals(detectedSceneMode)) {
                        mSceneMode.onSceneDetected("hdr-detection");
                    } else {
                        mSceneMode.onSceneDetected(detectedSceneMode);
                    }
                    break;

                default:
                    break;
            }

        }
    }
}
