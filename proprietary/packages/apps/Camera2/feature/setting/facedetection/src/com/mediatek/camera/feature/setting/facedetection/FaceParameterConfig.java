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

package com.mediatek.camera.feature.setting.facedetection;

import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.List;

/**
 * It is the face implement for API1.
 */
@SuppressWarnings("deprecation")
public class FaceParameterConfig implements ICameraSetting.IParametersConfigure,
        IFaceConfig {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            FaceParameterConfig.class.getSimpleName());
    private FaceDeviceCtrl.IFacePerformerMonitor mFaceMonitor;
    private OnDetectedFaceUpdateListener mOnDetectedFaceUpdateListener;
    private OnFaceValueUpdateListener mOnFaceValueUpdateListener;
    private boolean mIsFaceDetectionStarted;
    private boolean mIsSupported;
    private List<String> mSupportValueList = new ArrayList<String>();
    private ISettingManager.SettingDeviceRequester mSettingDeviceRequester;

    /**
     * Constructor of face parameter config in api1.
     * @param settingDeviceRequester device requester.
     */
    public FaceParameterConfig(ISettingManager.SettingDeviceRequester settingDeviceRequester) {
        mSettingDeviceRequester = settingDeviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        mIsSupported = originalParameters.getMaxNumDetectedFaces() > 0;
        mFaceMonitor.setSupportedStatus(mIsSupported);
        if (mIsSupported) {
            mSupportValueList.clear();
            mSupportValueList.add(IFaceConfig.FACE_DETECTION_ON);
            mSupportValueList.add(IFaceConfig.FACE_DETECTION_OFF);
        }
        mOnFaceValueUpdateListener.onFaceSettingValueUpdate(mIsSupported, mSupportValueList);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        int orientation = mOnFaceValueUpdateListener.onUpdateImageOrientation();
        parameters.setRotation(orientation);
        LogHelper.d(TAG, "[configParameters] setRotation as " + orientation);
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        if (mFaceMonitor.isNeedToStart()) {
            if (mIsFaceDetectionStarted) {
                LogHelper.d(TAG, "[configCommand] already started, return");
                return;
            }
            int orientation = mOnFaceValueUpdateListener.onUpdateImageOrientation();
            LogHelper.i(TAG, "[configCommand] start face detection, orientation = " + orientation
                + ", this = " + this);
            Camera.Parameters parameters = cameraProxy.getParameters();
            if (parameters != null) {
                parameters.setRotation(orientation);
                cameraProxy.setParameters(parameters);
            }
            cameraProxy.setFaceDetectionListener(mFaceDetectionListener);
            cameraProxy.startFaceDetection();
            mIsFaceDetectionStarted = true;
        }
        if (mFaceMonitor.isNeedToStop()) {
            if (!mIsFaceDetectionStarted) {
                LogHelper.i(TAG, "[configCommand] already stopped, return");
                return;
            }
            LogHelper.i(TAG, "[configCommand] stop face detection, this = " + this);
            cameraProxy.setFaceDetectionListener(null);
            mFaceDetectionListener.onFaceDetection(null, null);
            cameraProxy.stopFaceDetection();
            mIsFaceDetectionStarted = false;
        }
    }

    @Override
    public void sendSettingChangeRequest() {
        mSettingDeviceRequester.requestChangeCommand(IFaceConfig.KEY_FACE_DETECTION);
    }

    @Override
    public void setFaceMonitor(FaceDeviceCtrl.IFacePerformerMonitor monitor) {
        mFaceMonitor = monitor;
    }

    @Override
    public void updateImageOrientation() {
        LogHelper.d(TAG, "[updateImageOrientation]");
        mSettingDeviceRequester.requestChangeSettingValue(IFaceConfig.KEY_FACE_DETECTION);
    }

    @Override
    public void resetFaceDetectionState() {
        LogHelper.i(TAG, "[resetFaceDetectionState]");
        mIsFaceDetectionStarted = false;
    }

    @Override
    public void setFaceDetectionUpdateListener(OnDetectedFaceUpdateListener listener) {
        mOnDetectedFaceUpdateListener = listener;
    }

    @Override
    public void setFaceValueUpdateListener(OnFaceValueUpdateListener listener) {
        mOnFaceValueUpdateListener = listener;
    }

    private Camera.FaceDetectionListener mFaceDetectionListener
            = new Camera.FaceDetectionListener() {

        @Override
        public void onFaceDetection(Camera.Face[] faces, Camera camera) {
            if (mOnDetectedFaceUpdateListener != null) {
                mOnDetectedFaceUpdateListener.onDetectedFaceUpdate(getFaces(faces));
            }
        }
    };

    private Face[] getFaces(Camera.Face[] faces) {
        if (faces == null || (faces != null && faces.length == 0)) {
            LogHelper.e(TAG, "[getFaces] no faces");
            return null;
        }
        Face[] faceStructures = new Face[faces.length];
        for (int i = 0; i < faces.length; i++) {
            Face faceTemp = new Face();
            faceTemp.id = faces[i].id;
            faceTemp.score = faces[i].score;
            faceTemp.rect = faces[i].rect;
            faceStructures[i] = faceTemp;
        }
        return faceStructures;
    }
}
