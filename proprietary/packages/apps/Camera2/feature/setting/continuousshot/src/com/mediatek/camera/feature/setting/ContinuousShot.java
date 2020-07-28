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

package com.mediatek.camera.feature.setting;

import android.content.ContentValues;
import android.content.Intent;
import android.graphics.Bitmap;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener.OnShutterButtonListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.memory.IMemoryManager;
import com.mediatek.camera.common.memory.MemoryManagerImpl;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ICameraSetting.IParametersConfigure;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.storage.IStorageService;
import com.mediatek.camera.common.storage.MediaSaver.MediaSaverListener;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.setting.CsState.State;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import javax.annotation.Nonnull;


/**
 * The implement class for IContinuousShot.
 */
public class ContinuousShot extends ContinuousShotBase implements IParametersConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            ContinuousShot.class.getSimpleName());
    private static final String KEY_CAPTURE_MODE = "cap-mode";
    private static final String MTK_CAMERA_MODE = "mtk-cam-mode";
    private static final String CAPTURE_MODE_CONTINUOUS = "continuousshot";
    private static final String CAPTURE_MODE_NORMAL = "normal";
    private static final String KEY_BURST_SHOT_NUM = "burst-num";
    private static final String KEY_CSHOT_INDICATOR = "cshot-indicator";
    private static final String KEY_SUPPORTED_CAPTURE_MODE = "cap-mode-values";

    private static final int CAMERA_MODE_MTK_PRV = 1;

    // Command
    private static final int MTK_CAMERA_COMMAND_CANCEL_CONTINUOUS_SHOT = 0x1000000D;
    private static final int MTK_CAMERA_COMMAND_CONTINUOUS_SHOT_SPEED = 0x1000000E;

    // Message
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END = 0x00000006;
    private final Object mStopLock = new Object();

    private CsState mState;

    private boolean mIsContinuousShotSupport;
    private boolean mIsShutterCallbackReceived;

    @Override
    public void init(IApp app, ICameraContext cameraContext,
            ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mState = new CsState();
        mState.updateState(State.STATE_INIT);
    }

    @Override
    public void unInit() {
        super.unInit();
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        return this;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        int cameraId = Integer.valueOf(mSettingController.getCameraId());
        if (cameraId > 0) {
            mIsContinuousShotSupport = false;
        } else {
            List<String> captureMode = split(originalParameters.get(KEY_SUPPORTED_CAPTURE_MODE));
            mIsContinuousShotSupport = captureMode != null &&
                    captureMode.indexOf(CAPTURE_MODE_CONTINUOUS) >= 0;
        }
        initializeValue(mIsContinuousShotSupport);
        LogHelper.d(TAG, "[setOriginalParameters] Support CS: = " + mIsContinuousShotSupport);
        if (!mIsContinuousShotSupport) {
            return;
        }
        if (!("true".equalsIgnoreCase(originalParameters.get(KEY_CSHOT_INDICATOR)))) {
            disableIndicator();
        }
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        State state = mState.getCShotState();
        LogHelper.d(TAG, "[configParameters],current state : " + state);
        if (parameters.get(MTK_CAMERA_MODE) != null) {
            parameters.set(MTK_CAMERA_MODE, CAMERA_MODE_MTK_PRV);
        }
        switch (state) {
            case STATE_CAPTURE_STARTED:
            case STATE_CAPTURING:
                configureStartCaptureParameters(parameters);
                break;

            default:
                configureStopCaptureParameters(parameters);
                break;
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        synchronized (mStopLock) {
            State state = mState.getCShotState();
            LogHelper.i(TAG, "[configCommand],current state : " + state);
            switch (state) {
                case STATE_CAPTURE_STARTED:
                    LogHelper.d(TAG, "[configCommand] do capture");
                    mState.updateState(State.STATE_CAPTURING);
                    cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END,
                            new VendorDataCallback());
                    cameraProxy.takePicture(mShutterCallback, null, null, mJpegCallback);
                    break;
                case STATE_INIT:
                    LogHelper.d(TAG, "[configCommand] start preview");
                    cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END,
                            null);
                    cameraProxy.startPreviewAsync();
                    break;
                case STATE_CAPTURING:
                    cameraProxy.sendCommand(MTK_CAMERA_COMMAND_CONTINUOUS_SHOT_SPEED,
                            getSuitableSpeed(), 0);
                    break;
                default:
                    LogHelper.d(TAG, "[configCommand] cancel continuous shot");
                    cameraProxy.sendCommand(MTK_CAMERA_COMMAND_CANCEL_CONTINUOUS_SHOT, 0, 0);
                    break;
            }
        }
    }

    @Override
    public void sendSettingChangeRequest() {

    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        super.overrideValues(headerKey, currentValue, supportValues);
        LogHelper.d(TAG, "[overrideValues] getValue() = " + getValue() + ", headerKey = "
                + headerKey + ", currentValue = " + currentValue + ", supportValues  = "
                + supportValues);
        mIsContinuousShotSupport = CONTINUOUSSHOT_ON.equals(getValue());
    }

    @Override
    public void onModeClosed(String modeKey) {
        onContinuousShotStopped();
        onContinuousShotDone(0);
        super.onModeClosed(modeKey);
    }

    protected void slowDownContinuousShot() {
        if (mState.getCShotState() == State.STATE_CAPTURING) {
            mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
        }
    }

    protected boolean startContinuousShot() {
        if (!mIsContinuousShotSupport) {
            return false;
        }

        LogHelper.i(TAG, "[startContinuousShot]");
        // must set capturing state before capture to distinguish with normal capture.
        mState.updateState(State.STATE_CAPTURE_STARTED);
        onContinuousShotStarted();
        mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
        return true;
    }

    protected boolean stopContinuousShot() {
        State state = mState.getCShotState();
        if (State.STATE_CAPTURING != state && State.STATE_CAPTURE_STARTED != state) {
            LogHelper.w(TAG, "[stopContinuousShot], state is error,don't do anything");
            return false;
        }
        LogHelper.i(TAG, "[stopContinuousShot] state = " + state);
        onContinuousShotStopped();
        if (state == State.STATE_CAPTURE_STARTED) {
            onContinuousShotDone(0);
        } else {
            stopSound();
            mState.updateState(State.STATE_STOPPED);
            mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
        }
        return true;
    }

    protected void onContinuousShotDone(int captureNum) {
        super.onContinuousShotDone(captureNum);
        mIsShutterCallbackReceived = false;
        mState.updateState(State.STATE_INIT);
        mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);

    }

    protected void requestChangeOverrideValues() {
        mSettingDeviceRequester.requestChangeSettingValue(KEY_CSHOT);
    }

    private void configureStartCaptureParameters(Camera.Parameters parameters) {
        int cameraId = Integer.parseInt(mSettingController.getCameraId());
        int rotation = CameraUtil.getJpegRotationFromDeviceSpec(cameraId, mApp
                .getGSensorOrientation(), mActivity);
        LogHelper.d(TAG, "[configureStartCaptureParameters] rotation = " + rotation);
        parameters.set(KEY_CAPTURE_MODE, CAPTURE_MODE_CONTINUOUS);
        parameters.set(KEY_BURST_SHOT_NUM, MAX_CAPTURE_NUMBER);
        parameters.setRotation(rotation);
    }

    private void configureStopCaptureParameters(Camera.Parameters parameters) {
        parameters.set(KEY_CAPTURE_MODE, CAPTURE_MODE_NORMAL);
        parameters.set(KEY_BURST_SHOT_NUM, 0);
    }

    private ArrayList<String> split(String str) {
        if (str == null) {
            return null;
        }

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<String> substrings = new ArrayList<>();
        for (String s : splitter) {
            substrings.add(s);
        }
        LogHelper.d(TAG, "[split] substrings = " + substrings);
        return substrings;
    }

    private ShutterCallback mShutterCallback = new ShutterCallback() {
        @Override
        public void onShutter() {
            synchronized (mStopLock) {
                LogHelper.d(TAG, "[onShutter]");
                mIsShutterCallbackReceived = true;
                // start play sound only at first time when continuous shot
                if (State.STATE_CAPTURING == mState.getCShotState()) {
                    playSound();
                }
            }
        }
    };

    private PictureCallback mJpegCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] bytes, Camera camera) {
            if (!mIsShutterCallbackReceived) {
                LogHelper.w(TAG, "[onPictureTaken] current CShot shutter callback not return," +
                        " this is not CShot jpegCallback, dismiss the data.");
                return;
            }
            // jpeg data check.
            if (bytes == null) {
                stopContinuousShot();
                return;
            }
            saveJpeg(bytes);
        }
    };

    /**
     * Continuous shot data callback.
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
            LogHelper.d(TAG, "[onDataCallback] msgID = " + msgId + ", number = " + arg1
                    + ", mIsShutterCallbackReceived = " + mIsShutterCallbackReceived);
            if (msgId == MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END) {
                if (mIsShutterCallbackReceived) {
                    onContinuousShotDone(arg1);
                } else {
                    //shutter callback maybe had been canceled by stop command.
                    mState.updateState(State.STATE_INIT);
                    onContinuousShotStopped();
                }
            }
        }
    }
}
