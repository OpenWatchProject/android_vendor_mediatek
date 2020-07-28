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

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

/**
 * This is for face detection flow.
 */
@SuppressWarnings("deprecation")
public class FaceDeviceCtrl {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(FaceDeviceCtrl.class.getSimpleName());
    private boolean mIsPreviewStarted;
    private String mFaceOverrideState = IFaceConfig.FACE_DETECTION_ON;
    private boolean mIsFaceDetectionSupported;
    private IFacePerformerMonitor mFaceMonitor = new FacePerformerMonitor();
    private FaceParameterConfig mParameterConfig;
    private FaceCaptureRequestConfig mCaptureRequestConfig;
    private IFaceConfig mFaceConfig;
    private IFaceConfig.OnFaceValueUpdateListener mFaceValueUpdateListener;

    /**
     * init face device controller, used to set face config listener.
     */
    public void init() {
    }

    /**
     * Interface for face device to query controller state.
     */
    public interface IFacePerformerMonitor {

        /**
         * set whether the feature is supported.
         * @param isSupport true is supported.
         */
        public void setSupportedStatus(boolean isSupport);

        /**
         * This is to notify face detection value.
         * @return true if preview started and face detection state is stop
         * and face detection override value by restriction is on, or false.
         */
        public boolean isNeedToStart();

        /**
         * This is to query face detection state.
         * @return true It should do stop flow.
         */
        public boolean isNeedToStop();
    }

    /**
     * This is to notify preview status.
     * @param isPreviewStarted true if preview started,
     *                         or false if preview stopped.
     */
    public void onPreviewStatus(boolean isPreviewStarted) {
        LogHelper.d(TAG, "[onPreviewStatus] isPreviewStarted = " + isPreviewStarted);
        mIsPreviewStarted = isPreviewStarted;
        if (!isPreviewStarted && mFaceConfig != null) {
            //preview stop, FD had stopped.
            mFaceConfig.resetFaceDetectionState();
        }
    }

    /**
     * update image orientation to face algo.
     */
    public void updateImageOrientation() {
        if (mFaceConfig != null) {
            //no need update orientation when preview not started.
            mFaceConfig.updateImageOrientation();
        }
    }

    /**
     * This is to api1 flow.
     * @param  settingDeviceRequester the device requester.
     * @return  the Api1 interface.
     */
    public ICameraSetting.IParametersConfigure getParametersConfigure(
            ISettingManager.SettingDeviceRequester settingDeviceRequester) {
        if (mParameterConfig == null) {
            mParameterConfig = new FaceParameterConfig(settingDeviceRequester);
            mParameterConfig.setFaceMonitor(mFaceMonitor);
            mParameterConfig.setFaceValueUpdateListener(mFaceValueUpdateListener);
            mFaceConfig = mParameterConfig;
        }
        return mParameterConfig;
    }

    /**
     * This is to api2 flow.
     * @param  settingDevice2Requester the device 2 requester.
     * @return  the Api2 interface.
     */
    public ICameraSetting.ICaptureRequestConfigure getCaptureRequestConfigure(
            ISettingManager.SettingDevice2Requester settingDevice2Requester) {
        if (mCaptureRequestConfig == null) {
            mCaptureRequestConfig = new FaceCaptureRequestConfig(settingDevice2Requester);
            mCaptureRequestConfig.setFaceMonitor(mFaceMonitor);
            mCaptureRequestConfig.setFaceValueUpdateListener(mFaceValueUpdateListener);
            mFaceConfig = mCaptureRequestConfig;
        }
        mIsPreviewStarted = true;
        return mCaptureRequestConfig;
    }

    /**
     * This is to notify face detection value.
     * @param  overrideValue face detection value by other restriction.
     */
    public void updateFaceDetectionStatus(String overrideValue) {
        mFaceOverrideState = overrideValue;
    }

    /**
     * This is to check face detection status changed or not.
     * @param  curValue face detection current value.
     * @return true, if face detection status changed.
     */
    public boolean isFaceDetectionStatusChanged(String curValue) {
        return !mFaceOverrideState.equals(curValue);
    }

    /**
     * This is to set face detection update listener.
     * @param  detectedFaceUpdateListener the detected face listener.
     *
     */
    public void setDetectedFaceUpdateListener(IFaceConfig.OnDetectedFaceUpdateListener
                                                      detectedFaceUpdateListener) {
        if (mFaceConfig != null) {
            mFaceConfig.setFaceDetectionUpdateListener(detectedFaceUpdateListener);
        }
    }

    /**
     * This is to set face detection update listener.
     * @param  onFaceSettingValueUpdateListener the detected face listener.
     *
     */
    public void setFaceValueUpdateListener(IFaceConfig.OnFaceValueUpdateListener
                                                          onFaceSettingValueUpdateListener) {
        mFaceValueUpdateListener = onFaceSettingValueUpdateListener;
    }

    /**
     * Used for face device to query controller state.
     */
    private class FacePerformerMonitor implements IFacePerformerMonitor {

        public void setSupportedStatus(boolean isSupport) {
            mIsFaceDetectionSupported = isSupport;
        }

        public boolean isNeedToStart() {
            boolean overrideState = mFaceOverrideState.equals(IFaceConfig.FACE_DETECTION_ON);
            boolean needStart = overrideState && mIsPreviewStarted
                    && mIsFaceDetectionSupported;
            LogHelper.d(TAG, "[isNeedStart]  overrideState = " + overrideState
                    + ", mIsPreviewStarted = " + mIsPreviewStarted
                    + ", mIsFaceDetectionSupported = " + mIsFaceDetectionSupported
                    + ", needStart = " + needStart);
            return needStart;
        }

        public boolean isNeedToStop() {
            boolean overrideState = mFaceOverrideState.equals(IFaceConfig.FACE_DETECTION_OFF);
            boolean needStop = overrideState && mIsPreviewStarted
                    && mIsFaceDetectionSupported;
            LogHelper.d(TAG, "[isNeedStop]  overrideState = " + overrideState
                    + ", mIsPreviewStarted = " + mIsPreviewStarted
                    + ", mIsFaceDetectionSupported = " + mIsFaceDetectionSupported
                    + ", needStop = " + needStop);
            return needStop;
        }
    }
}
