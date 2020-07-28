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

package com.mediatek.camera.feature.setting.facedetection;

import android.graphics.RectF;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * This is Face detection feature used to interact with other module.
 */
@SuppressWarnings("deprecation")
public class FaceDetection extends SettingBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(FaceDetection.class.getSimpleName());
    private static final String FACE_EXIST_KEY = "key_face_exist";
    private static final String FOCUS_STATE_KEY = "key_focus_state";

    private Size mPreviewSize;
    private FaceViewCtrl mFaceViewCtrl = new FaceViewCtrl();
    private FaceDeviceCtrl mFaceDeviceCtrl = new FaceDeviceCtrl();
    private ISettingChangeRequester mSettingChangeRequester;
    private List<String> mSupportValues = new ArrayList<>();
    private StatusMonitor.StatusResponder mFaceExistStatusResponder;
    private boolean mIsFaceExistLastTime = false;

    /**
     * Initialize setting. This will be called when do open camera.
     * @param app the instance of IApp.
     * @param cameraContext the CameraContext.
     * @param settingController the SettingController.
     */
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        LogHelper.d(TAG, "[init]");
        super.init(app, cameraContext, settingController);
        mFaceDeviceCtrl.init();
        mFaceViewCtrl.init(app);
        mFaceDeviceCtrl.setFaceValueUpdateListener(mOnFaceValueUpdateListener);
        //init face device and face view first.
        app.registerOnOrientationChangeListener(mOrientationListener);
        app.getAppUi().registerOnPreviewAreaChangedListener(mPreviewAreaChangedListener);
        initSettingValue();
        updateFaceDisplayOrientation();
        mFaceExistStatusResponder = mStatusMonitor.getStatusResponder(FACE_EXIST_KEY);
        mStatusMonitor.registerValueChangedListener(FOCUS_STATE_KEY, mFaceViewCtrl);
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "[unInit]");
        mIsFaceExistLastTime = false;
        mFaceViewCtrl.unInit();
        mApp.getAppUi().unregisterOnPreviewAreaChangedListener(mPreviewAreaChangedListener);
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mStatusMonitor.unregisterValueChangedListener(FOCUS_STATE_KEY, mFaceViewCtrl);
    }

    @Override
    public void postRestrictionAfterInitialized() {
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return IFaceConfig.KEY_FACE_DETECTION;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester
                    = mFaceDeviceCtrl.getParametersConfigure(mSettingDeviceRequester);
        }
        return (FaceParameterConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester
                    = mFaceDeviceCtrl.getCaptureRequestConfigure(mSettingDevice2Requester);
        }
        return (FaceCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public PreviewStateCallback getPreviewStateCallback() {
        return mPreviewStateCallback;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        super.overrideValues(headerKey, currentValue, supportValues);
        String curValue = getValue() == null ? IFaceConfig.FACE_DETECTION_OFF : getValue();
        if (mFaceDeviceCtrl.isFaceDetectionStatusChanged(curValue)) {
            LogHelper.d(TAG, "[overrideValues] curValue = " + curValue + ", headerKey = "
                    + headerKey);
            mFaceDeviceCtrl.updateFaceDetectionStatus(curValue);
            mFaceViewCtrl.enableFaceView(IFaceConfig.FACE_DETECTION_ON.equals(curValue));
            requestFaceDetection();
        }
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(TAG, "[onModeOpened] modeKey = " + modeKey);
    }

    @Override
    public void onModeClosed(String modeKey) {
        LogHelper.d(TAG, "[onModeClosed] modeKey = " + modeKey);
        mIsFaceExistLastTime = false;
        // Avoid to start FD of both front and back camera, when switch from PIP mode to other mode,
        // no need to recover FD state, because camera will be closed and then opened.
        if (!modeKey.startsWith("com.mediatek.camera.feature.mode.pip.")) {
            super.onModeClosed(modeKey);
        }
    }

    private void initSettingValue() {
        mSupportValues.add(IFaceConfig.FACE_DETECTION_OFF);
        mSupportValues.add(IFaceConfig.FACE_DETECTION_ON);
        setSupportedPlatformValues(mSupportValues);
        setSupportedEntryValues(mSupportValues);
        setEntryValues(mSupportValues);
        String value = mDataStore.getValue(
                getKey(), IFaceConfig.FACE_DETECTION_ON, getStoreScope());
        setValue(value);
    }

    private IApp.OnOrientationChangeListener mOrientationListener =
            new IApp.OnOrientationChangeListener() {
        @Override
        public void onOrientationChanged(int orientation) {
            if (IFaceConfig.FACE_DETECTION_ON.equals(getValue())) {
                updateFaceDisplayOrientation();
                updateImageOrientation();
            }
        }
    };

    private void updateFaceDisplayOrientation() {
        //orientation, g-sensor, no used
        int cameraId = Integer.valueOf(mSettingController.getCameraId());
        int displayRotation = CameraUtil.getDisplayRotation(mApp.getActivity());
        int displayOrientation = CameraUtil.getDisplayOrientationFromDeviceSpec(
                displayRotation, cameraId, mApp.getActivity());
        mFaceViewCtrl.updateFaceDisplayOrientation(displayOrientation, cameraId);
    }

    private void updateImageOrientation() {
        mFaceDeviceCtrl.updateImageOrientation();
    }

    private IAppUiListener.OnPreviewAreaChangedListener mPreviewAreaChangedListener
            = new IAppUiListener.OnPreviewAreaChangedListener() {
        @Override
        public void onPreviewAreaChanged(RectF newPreviewArea, Size previewSize) {
            mPreviewSize = previewSize;
            mFaceViewCtrl.onPreviewAreaChanged(newPreviewArea);
        }
    };

    private IFaceConfig.OnDetectedFaceUpdateListener mOnDetectedFaceUpdateListener
            = new IFaceConfig.OnDetectedFaceUpdateListener() {
        @Override
        public void onDetectedFaceUpdate(Face[] faces) {
            mFaceViewCtrl.onDetectedFaceUpdate(faces);
            boolean isFaceExist = (faces != null && faces.length > 0);
            if (isFaceExist != mIsFaceExistLastTime) {
                if (isFaceExist) {
                    mFaceExistStatusResponder.statusChanged(FACE_EXIST_KEY, String.valueOf(true));
                } else {
                    mFaceExistStatusResponder.statusChanged(FACE_EXIST_KEY, String.valueOf(false));
                }
                mIsFaceExistLastTime = isFaceExist;
            }

        }
    };

    private IFaceConfig.OnFaceValueUpdateListener mOnFaceValueUpdateListener
            = new IFaceConfig.OnFaceValueUpdateListener() {
        @Override
        public Size onFacePreviewSizeUpdate() {
            Size previewSize = new Size(mPreviewSize.getWidth(), mPreviewSize.getHeight());
            return previewSize;
        }

        @Override
        public int onUpdateImageOrientation() {
            int cameraId = Integer.valueOf(mSettingController.getCameraId());
            int rotation = CameraUtil.getJpegRotationFromDeviceSpec(
                    cameraId, mApp.getGSensorOrientation(), mApp.getActivity());
            LogHelper.d(TAG, "[onUpdateImageOrientation] camera id = " + cameraId + ", rotation = "
                    + rotation);
            return rotation;
        }

        @Override
        public void onFaceSettingValueUpdate(boolean isSupport, List<String> supportList) {
            setSupportedPlatformValues(supportList);
            setSupportedEntryValues(supportList);
            setEntryValues(supportList);
            setValue(getEntryValues().get(0));
            mFaceDeviceCtrl.setDetectedFaceUpdateListener(mOnDetectedFaceUpdateListener);
        }
    };

    private ICameraSetting.PreviewStateCallback mPreviewStateCallback =
            new ICameraSetting.PreviewStateCallback() {

        @Override
        public void onPreviewStopped() {
            mFaceViewCtrl.onPreviewStatus(false);
            mFaceDeviceCtrl.onPreviewStatus(false);
            mFaceDeviceCtrl.setDetectedFaceUpdateListener(null);
        }

        @Override
        public void onPreviewStarted() {
            mFaceDeviceCtrl.onPreviewStatus(true);
            mFaceDeviceCtrl.setDetectedFaceUpdateListener(mOnDetectedFaceUpdateListener);
            requestFaceDetection();
        }
    };

    private void requestFaceDetection() {
        if (mSettingChangeRequester != null) {
            mSettingChangeRequester.sendSettingChangeRequest();
        }
    }
}
