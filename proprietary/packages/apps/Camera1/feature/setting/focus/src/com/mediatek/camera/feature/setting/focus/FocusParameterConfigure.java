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
package com.mediatek.camera.feature.setting.focus;

import android.hardware.Camera;
import android.hardware.Camera.Area;
import android.hardware.Camera.Parameters;
import android.os.Message;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.mode.photo.PhotoMode;
import com.mediatek.camera.common.mode.video.VideoMode;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;


/**
 * This class used to configure focus value to parameters and.
 * those settings which have restriction with focus but don't have
 * setting item in setting will configure them value in this class.
 * and just used for api1.
 */
public class FocusParameterConfigure implements ICameraSetting.IParametersConfigure, IFocus
        .Listener, IFocusController {

    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(FocusParameterConfigure.class.getSimpleName());

    private static final int MTK_CAMERA_MSG_EXT_DATA_AF = 0x00000020;
    private static final String FOCUS_MODE_AUTO = "auto";
    private static final String FOCUS_MODE_CONTINUOUS_PICTURE = "continuous-picture";
    private static final String FOCUS_MODE_CONTINUOUS_VIDEO = "continuous-video";
    private static final String FOCUS_MODE_INFINITY = "infinity";
    private static final String AUTOFOCUS = "autoFocus";
    private static final String CANCEL_AUTOFOCUS = "cancelAutoFocus";
    private final Focus mFocus;
    private String mCurrentFocusMode = FOCUS_MODE_CONTINUOUS_PICTURE;

    private boolean mIsSupportedFocus = false;
    private boolean mFocusAreaSupported;
    private boolean mMeteringAreaSupported;

    private List<Camera.Area> mFocusArea;
    private List<Area> mMeteringArea;
    private List<String> mSupportedFocusModeList = Collections.<String>emptyList();
    private List<String> mAppSupportedFocusModeList = Collections.<String>emptyList();
    private List<String> mSettingSupportedFocusModeList = Collections.<String>emptyList();
    private IFocusController.FocusStateListener mFocusStateListener;
    private ISettingManager.SettingDeviceRequester mDeviceRequester;
    private boolean mDisableUpdateFocusState = false;
    private final Object mLock = new Object();
    private ConcurrentLinkedQueue<String> mFocusQueue = new ConcurrentLinkedQueue<>();

    /**
     * The construction function.
     *
     * @param focus           the Focus class object.
     * @param deviceRequester deviceRequester The implementer of
     *                        {@link ISettingManager.SettingDeviceRequester}.
     */
    public FocusParameterConfigure(Focus focus, ISettingManager.SettingDeviceRequester
            deviceRequester) {
        mFocus = focus;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Parameters originalParameters) {
        mDisableUpdateFocusState = false;
        updateCapabilities(originalParameters);
        initPlatformSupportedValues(originalParameters);
        if (mIsSupportedFocus) {
            initAppSupportedEntryValues();
            initSettingEntryValues();
            initFocusMode(mSettingSupportedFocusModeList);
        }
    }

    @Override
    public boolean configParameters(Parameters parameters) {
        if (mIsSupportedFocus) {
            if (mCurrentFocusMode != null) {
                if (mSupportedFocusModeList.contains(mCurrentFocusMode)) {
                    if (mFocusAreaSupported) {
                        parameters.setFocusAreas(mFocusArea);
                    }
                    if (mMeteringAreaSupported) {
                        parameters.setMeteringAreas(mMeteringArea);
                    }
                    parameters.setFocusMode(mCurrentFocusMode);
                } else {
                    LogHelper.w(TAG, "[configParameters] mCurrentFocusMode is not supported in " +
                            "current platform");
                }
            }
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        LogHelper.d(TAG, "[configCommand] configCommand mCurrentFocusMode = " + mCurrentFocusMode);
        if (!mSupportedFocusModeList.contains(mCurrentFocusMode)) {
            LogHelper.w(TAG, "[configCommand] - mCurrentFocusMode does not supported with " +
                    "mSupportedFocusModeList " + mSupportedFocusModeList);
            return;
        }
        if (FOCUS_MODE_CONTINUOUS_PICTURE.equals(mCurrentFocusMode) ||
                FOCUS_MODE_CONTINUOUS_VIDEO.equals(mCurrentFocusMode)) {
            updateAfCallback(cameraProxy, true);
        } else if (FOCUS_MODE_AUTO.equals(mCurrentFocusMode)) {
            updateAfCallback(cameraProxy, false);
        }
    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mFocus.getKey());
    }

    @Override
    public void setFocusStateListener(IFocusController.FocusStateListener listener) {
        synchronized (mLock) {
            mFocusStateListener = listener;
        }
    }

    @Override
    public boolean isFocusCanDo() {
        // when devices support both continuous and infinity focus mode.
        LogHelper.d(TAG, "[isFocusCanDo] + focusMode " + mCurrentFocusMode);
        if (Parameters.FOCUS_MODE_INFINITY.equals(mCurrentFocusMode)) {
            LogHelper.w(TAG, "[isFocusCanDo] - focusMode : " + mCurrentFocusMode);
            return false;
        }
        // Check if metering area or focus area is supported.
        if (!mFocusAreaSupported) {
            LogHelper.w(TAG, "[isFocusCanDo] - mFocusAreaSupported is false");
            return false;
        }
        if (!mMeteringAreaSupported) {
            LogHelper.w(TAG, "[isFocusCanDo] - mMeteringAreaSupported is false");
            return false;
        }
        LogHelper.d(TAG, "[isFocusCanDo] - return true");
        return true;
    }

    @Override
    public void updateFocusArea(List<Camera.Area> focusArea, List<Camera.Area> meteringArea) {
        mFocusArea = focusArea;
        mMeteringArea = meteringArea;
    }

    @Override
    public void updateFocusMode(String currentValue) {
        if (mSettingSupportedFocusModeList.contains(currentValue)) {
            mCurrentFocusMode = currentValue;
            sendSettingChangeRequest();
        }
    }

    @Override
    public void overrideFocusMode(String currentValue, List<String> supportValues) {
        LogHelper.d(TAG, "[overrideFocusMode] currentValue = " + currentValue + ",supportValues = "
                + supportValues + ",mCurrentFocusMode =" + mCurrentFocusMode);
        if (mSettingSupportedFocusModeList.contains(currentValue)) {
            if (mCurrentFocusMode != currentValue) {
                mCurrentFocusMode = currentValue;
                mDeviceRequester.requestChangeCommand(mFocus.getKey());
            }
        }
    }

    @Override
    public void autoFocus() {
        LogHelper.d(TAG, "[autoFocus]");
        synchronized (mFocusQueue) {
            mFocusQueue.add(AUTOFOCUS);
        }
        mDeviceRequester.requestChangeCommand(mFocus.getKey());
    }

    @Override
    public void restoreContinue() {
        LogHelper.d(TAG, "[restoreContinue] " + mFocus.getValue());
        if (mFocus.getValue() == FOCUS_MODE_CONTINUOUS_PICTURE || mFocus.getValue() ==
                FOCUS_MODE_CONTINUOUS_VIDEO) {
            mCurrentFocusMode = mFocus.getValue();
            sendSettingChangeRequest();
            mDeviceRequester.requestChangeCommand(mFocus.getKey());
        }
    }

    @Override
    public void cancelAutoFocus() {
        LogHelper.d(TAG, "[cancelAutoFocus] mFocusQueue size is " + mFocusQueue.size() + ", peek " +
                " = " + mFocusQueue.peek());
        synchronized (mFocusQueue) {
            if (!mFocusQueue.isEmpty() && AUTOFOCUS.equals(mFocusQueue.peek())) {
                mFocusQueue.clear();
            } else {
                mFocusQueue.add(CANCEL_AUTOFOCUS);
            }
        }
        mDeviceRequester.requestChangeCommand(mFocus.getKey());
    }

    @Override
    public void updateFocusCallback() {
        mDeviceRequester.requestChangeCommand(mFocus.getKey());
    }

    @Override
    public String getCurrentFocusMode() {
        LogHelper.d(TAG, "getCurrentFocusMode " + mCurrentFocusMode);
        return mCurrentFocusMode;
    }

    @Override
    public void disableUpdateFocusState(boolean disable) {
        mDisableUpdateFocusState = disable;
    }

    @Override
    public void resetConfiguration() {
        if (!mFocusQueue.isEmpty()) {
            mFocusQueue.clear();
        }
    }

    private void updateCapabilities(Parameters originalParameters) {
        if (originalParameters == null) {
            return;
        }
        mFocusAreaSupported = (originalParameters.getMaxNumFocusAreas() > 0 && isSupported(
                Parameters.FOCUS_MODE_AUTO, originalParameters.getSupportedFocusModes()));
        mMeteringAreaSupported = (originalParameters.getMaxNumMeteringAreas() > 0);
        LogHelper.d(TAG, "[updateCapabilities] mFocusAreaSupported = " + mFocusAreaSupported + "," +
                "mMeteringAreaSupported = " + mMeteringAreaSupported);
    }

    private static boolean isSupported(String value, List<String> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    private void initPlatformSupportedValues(Parameters originalParameters) {
        mSupportedFocusModeList = originalParameters.getSupportedFocusModes();
        if (mSupportedFocusModeList != null) {
            mIsSupportedFocus = !mSupportedFocusModeList.isEmpty();
        }
        if (mIsSupportedFocus) {
            mFocus.initPlatformSupportedValues(mSupportedFocusModeList);
        }
        LogHelper.d(TAG, "[initPlatformSupportedValues] mSupportedFocusModeList " +
                mSupportedFocusModeList);
    }

    private void initAppSupportedEntryValues() {
        mAppSupportedFocusModeList = mSupportedFocusModeList;
        mFocus.initAppSupportedEntryValues(mAppSupportedFocusModeList);
    }

    private void initSettingEntryValues() {
        List<String> supportedList = new ArrayList<>();
        supportedList.addAll(mSupportedFocusModeList);
        supportedList.retainAll(mAppSupportedFocusModeList);
        mSettingSupportedFocusModeList = supportedList;
        mFocus.initSettingEntryValues(mSettingSupportedFocusModeList);
    }

    private void initFocusMode(List<String> modes) {
        LogHelper.d(TAG, "[initFocusMode] + ");
        if (modes == null || modes.isEmpty()) {
            return;
        }
        if (modes.indexOf(FOCUS_MODE_CONTINUOUS_PICTURE) > 0) {
            mCurrentFocusMode = FOCUS_MODE_CONTINUOUS_PICTURE;
        } else if (modes.indexOf(FOCUS_MODE_AUTO) > 0) {
            mCurrentFocusMode = FOCUS_MODE_AUTO;
        } else {
            mCurrentFocusMode = modes.get(0);
        }
        mFocus.setValue(mCurrentFocusMode);
        LogHelper.d(TAG, "[mCurrentFocusMode] -" + mCurrentFocusMode);
    }

    private Camera.AutoFocusMoveCallback mAutoFocusMoveCallback =
            new Camera.AutoFocusMoveCallback() {
                @Override
                public void onAutoFocusMoving(boolean start, Camera camera) {
                    long frameNumber = -1;
                    synchronized (mLock) {
                        if (mFocusStateListener == null || mDisableUpdateFocusState) {
                            LogHelper.w(TAG, "[onAutoFocusMoving] mFocusStateListener = " +
                                    mFocusStateListener + ",mDisableUpdateFocusState = " +
                                    mDisableUpdateFocusState);
                            return;
                        }
                        LogHelper.d(TAG, "[onAutoFocusMoving] start = " + start);
                        if (start) {
                            mFocusStateListener.onFocusStatusUpdate(IFocusController.AutoFocusState
                                    .PASSIVE_SCAN, frameNumber);
                        } else {
                            mFocusStateListener.onFocusStatusUpdate(IFocusController.AutoFocusState
                                    .PASSIVE_FOCUSED, frameNumber);
                        }
                    }
                }
            };

    private Camera.AutoFocusCallback mAutoFocusCallback = new Camera.AutoFocusCallback() {
        @Override
        public void onAutoFocus(boolean success, Camera camera) {
            long frameNumber = -1;
            synchronized (mLock) {
                if (mFocusStateListener == null) {
                    LogHelper.w(TAG, "[onAutoFocus] mFocusStateListener is null ");
                    return;
                }
                LogHelper.d(TAG, "[onAutoFocus] success = " + success);
                if (success) {
                    mFocusStateListener.onFocusStatusUpdate(IFocusController.AutoFocusState
                            .ACTIVE_FOCUSED, frameNumber);
                } else {
                    mFocusStateListener.onFocusStatusUpdate(IFocusController.AutoFocusState
                            .ACTIVE_UNFOCUSED, frameNumber);

                }

            }
        }
    };

    /**
     * AF data callback.
     */
    private class VendorDataCallback implements CameraProxy.VendorDataCallback {

        /**
         * Called when vendor-defined data is available.
         *
         * @param message message object from native.
         */
        @Override
        public void onDataTaken(Message message) {
            LogHelper.d(TAG, "[onDataTaken] message = " + message.what);
        }

        /**
         * Called when vendor-defined data is available.
         *
         * @param msgId The id of message which contains the callback info.
         * @param data  Callback data.
         * @param arg1  The first argument.
         * @param arg2  The second argument.
         */
        @Override
        public void onDataCallback(int msgId, byte[] data, int arg1, int arg2) {
            LogHelper.d(TAG, "[onDataCallback] data = " + data);
            if (data != null) {
                mFocus.setAfData(data);
            }
        }
    }

    private boolean isMzafEnabled() {
        return mFocus.isMultiZoneAfEnabled();
    }

    private boolean isSingleAfEnabled() {
        return mFocus.isSingleAfEnabled();
    }

    private void updateAfCallback(CameraProxy cameraProxy, boolean isContinueAf) {
        LogHelper.d(TAG, "[updateAfCallback] + isContinueAf " + isContinueAf);
        if (isContinueAf) {
            cameraProxy.setAutoFocusMoveCallback(mAutoFocusMoveCallback);
            if (isMzafEnabled()) {
                cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_DATA_AF,
                        new VendorDataCallback());
            } else if (isSingleAfEnabled()) {
                cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_DATA_AF, null);
            }
        } else {
            cameraProxy.setAutoFocusMoveCallback(null);
            if (isMzafEnabled() || isSingleAfEnabled()) {
                cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_DATA_AF, null);
            }
        }
        synchronized (mFocusQueue) {
            if (!mFocusQueue.isEmpty()) {
                String currentEvent = mFocusQueue.poll();
                if (AUTOFOCUS.equals(currentEvent)) {
                    LogHelper.d(TAG, "[updateAfCallback] call framework autoFocus");
                    cameraProxy.autoFocus(mAutoFocusCallback);
                } else if (CANCEL_AUTOFOCUS.equals(currentEvent)) {
                    LogHelper.d(TAG, "[updateAfCallback] call framework cancelAutoFocus");
                    cameraProxy.cancelAutoFocus();
                }
            }
        }
        LogHelper.d(TAG, "[updateAfCallback] -");
    }

}