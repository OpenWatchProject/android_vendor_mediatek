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
public class ContinuousShot extends SettingBase implements IParametersConfigure,
        OnShutterButtonListener, IMemoryManager.IMemoryListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            ContinuousShot.class.getSimpleName());
    private static final String KEY_CAPTURE_MODE = "cap-mode";
    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String CAPTURE_MODE_CONTINUOUS = "continuousshot";
    private static final String CAPTURE_MODE_NORMAL = "normal";
    private static final String KEY_BURST_SHOT_NUM = "burst-num";
    private static final String CONTINUOUSSHOT_ON = "on";
    private static final String CONTINUOUSSHOT_OFF = "off";
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final String KEY_CSHOT_INDICATOR = "cshot-indicator";
    private static final String KEY_SUPPORTED_CAPTURE_MODE = "cap-mode-values";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String VALUE_OF_AUTO_SCENE_DETECTION = "auto-scene-detection";

    private static final int SHUTTER_BUTTON_PRIORITY = 50;
    private static final int MAX_CAPTURE_NUMBER = 100;
    private static final int INIT_CONTINUOUS_SHOT_VIEW = 1000;
    private static final int UNINIT_CONTINUOUS_SHOT_VIEW = 1001;
    private static final int SHOW_CONTINUOUS_SHOT_VIEW = 1002;
    private static final int HIDE_CONTINUOUS_SHOT_VIEW = 1003;

    // Command
    private static final int MTK_CAMERA_COMMAND_CANCLE_CONTINUOUS_SHOT = 0x1000000D;
    private static final int MTK_CAMERA_COMMAND_CONTINUOUS_SHOT_SPEED = 0x1000000E;

    // Message
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END = 0x00000006;
    private long mShutterTime = 0;
    private int mCurrentShotsNum;
    private int mSavedNum;
    private long mFreeStorageForCapture;
    private boolean mIsSupportIndicator;
    private String mFileDirectory;

    private CsState mState;
    private CsNamingRule mCsNamingRule;
    private CaptureSound mCaptureSound;
    private MemoryManagerImpl mMemoryManager;
    private IContinuousShotView mIContinuousShotView;
    private ICameraContext mCameraContext;
    private IMemoryManager.MemoryAction mMemoryState;

    private Handler mIndicatorViewHandler;
    private Object mNumlock = new Object();
    private Object mStopLock = new Object();

    private boolean mIsContinuousShotSupport = false;
    private boolean mIsShutterCallbackReceived = false;

    @Override
    public void init(IApp app, ICameraContext cameraContext,
            ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        LogHelper.d(TAG, "[init]");
        mCameraContext = cameraContext;
        open(app);
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "[unInit]");
        close();
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO;
    }

    @Override
    public String getKey() {
        return KEY_CSHOT;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        return this;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        return null;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        int cameraId = Integer.valueOf(mSettingController.getCameraId());
        if (cameraId > 0) {
            mIsContinuousShotSupport = false;
        } else {
            List<String> captureMode = split(originalParameters.get(KEY_SUPPORTED_CAPTURE_MODE));
            mIsContinuousShotSupport = captureMode == null ? false
                : captureMode.indexOf(CAPTURE_MODE_CONTINUOUS) >= 0;
        }
        initializeValue();
        LogHelper.d(TAG, "[setOriginalParameters] Support CS: = " + mIsContinuousShotSupport);
        if (!mIsContinuousShotSupport) {
            return;
        }
        initIndicatorView(originalParameters);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (!mIsContinuousShotSupport) {
            return false;
        }
        State state = mState.getCShotState();
        LogHelper.d(TAG, "[configParameters],current state : " + state);
        switch (state) {
            case STATE_CAPTURE_STARTED:
                configureStartCaptureParameters(parameters);
                break;

            case STATE_STOPPED:
            case STATE_OPENED:
                configureStopCaptureParameters(parameters);
                break;

            default:
                break;
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        if (!mIsContinuousShotSupport) {
            return;
        }
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
                case STATE_SAVING:
                    LogHelper.d(TAG, "[configCommand] clear the callback");
                    cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END,
                            null);
                    break;
                case STATE_OPENED:
                    LogHelper.d(TAG, "[configCommand] start preview");
                    cameraProxy.startPreviewAsync();
                    break;
                case STATE_CAPTURING:
                    cameraProxy.sendCommand(MTK_CAMERA_COMMAND_CONTINUOUS_SHOT_SPEED,
                            mMemoryManager.getSuitableSpeed(), 0);
                    break;
                default:
                    LogHelper.d(TAG, "[configCommand] cancel continuous shot");
                    cameraProxy.sendCommand(MTK_CAMERA_COMMAND_CANCLE_CONTINUOUS_SHOT, 0, 0);
                    break;
            }
        }
    }

    @Override
    public void sendSettingChangeRequest() {

    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        State state = mState.getCShotState();
        LogHelper.d(TAG, "[onShutterButtonFocus] pressed = " + pressed + ",cs supported = " +
                mIsContinuousShotSupport + " state = " + state);
        if (mIsContinuousShotSupport && !pressed &&
                (State.STATE_CAPTURING == state || State.STATE_CAPTURE_STARTED == state)) {
            mAppUi.applyAllUIEnabledImmediately(false);
            new Thread() {
                public void run() {
                    synchronized (mStopLock) {
                        stopContinuousShot();
                    }
                }
            }.start();
            return true;
        }
        return false;
    }

    @Override
    public boolean onShutterButtonClick() {
        if (State.STATE_INIT == mState.getCShotState()
                || State.STATE_OPENED == mState.getCShotState()) {
            return false;
        }
        return true;
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        LogHelper.d(TAG, "[onShutterButtonLongPressed], CShot supported = "
                + mIsContinuousShotSupport);
        if (mIsContinuousShotSupport) {
            startContinuousShot();
            return true;
        }
        return false;
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        super.onModeOpened(modeKey, modeType);
        LogHelper.d(TAG, "[onModeOpened] modeType = " + modeType);
        if (ICameraMode.ModeType.PHOTO == modeType) {
            mAppUi.registerOnShutterButtonListener(this, SHUTTER_BUTTON_PRIORITY);
        }
    }

    @Override
    public void onModeClosed(String modeKey) {
        super.onModeClosed(modeKey);
        LogHelper.d(TAG, "[onModeClosed]");
        if (mIndicatorViewHandler != null) {
            mIndicatorViewHandler.removeMessages(UNINIT_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.sendEmptyMessage(UNINIT_CONTINUOUS_SHOT_VIEW);
        }
        mAppUi.unregisterOnShutterButtonListener(this);
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        LogHelper.d(TAG, "[overrideValues1] getValue() = " + getValue() + ", headerKey = "
                + headerKey + ", currentValue = " + currentValue + ", supportValues  = "
                + supportValues);
        super.overrideValues(headerKey, currentValue, supportValues);
        mIsContinuousShotSupport = CONTINUOUSSHOT_ON.equals(getValue());
    }

    @Override
    public void onMemoryStateChanged(IMemoryManager.MemoryAction state) {
        LogHelper.d(TAG, "[onMemoryStateChanged] memory state = " + state);
        switch (state) {
            case ADJUST_SPEED:
                if (mState.getCShotState() == State.STATE_CAPTURING
                        && mMemoryState != IMemoryManager.MemoryAction.ADJUST_SPEED) {
                    mMemoryState = IMemoryManager.MemoryAction.ADJUST_SPEED;
                    mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
                }
                break;
            case STOP:
                mMemoryState = IMemoryManager.MemoryAction.STOP;
                stopContinuousShot();
                break;
            default:
                mMemoryState = IMemoryManager.MemoryAction.NORMAL;
                break;
        }
    }

    private void open(IApp app) {
        prepareOpen();
        State state = mState.getCShotState();
        LogHelper.d(TAG, "[open]+ state = " + state);
        if (State.STATE_OPENED == state) {
            return;
        }
        mCaptureSound = new CaptureSound(app.getActivity());
        mCaptureSound.load();
        mMemoryManager = new MemoryManagerImpl(app.getActivity());
        mState.updateState(State.STATE_OPENED);
        LogHelper.d(TAG, "[open]-");
    }

    private void prepareOpen() {
        mFileDirectory = mCameraContext.getStorageService().getFileDirectory();
        mState = new CsState();
        mCsNamingRule = new CsNamingRule();
    }

    private void startContinuousShot() {
        if (!checkStorageMemory()) {
            return;
        }
        if (!isReadyForCapture()) {
            LogHelper.e(TAG, "[startContinuousShot] current state is not ready, can't do CS");
            return;
        }
        LogHelper.i(TAG, "[startContinuousShot]+");
        //init the key value in beginning, and the num should be updated with
        //the capture state, make sure the capture had been ended before next
        //capture begin.
        mCurrentShotsNum = 0;
        mSavedNum = 0;
        if (mIContinuousShotView != null) {
            mIContinuousShotView.setOrientation(mApp.getGSensorOrientation());
        }
        mFileDirectory = mCameraContext.getStorageService().getFileDirectory();
        mMemoryManager.addListener(this);
        mMemoryManager.initStateForCapture(mFreeStorageForCapture);
        mApp.disableGSensorOrientation();
        disableAllUIExceptionShutter();
        // must set capturing state before capture to distinguish with normal capture.
        mState.updateState(State.STATE_CAPTURE_STARTED);
        postRestriction(mState);
        mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
        mCameraContext.getStorageService().registerStorageStateListener(mStorageStateListener);
        LogHelper.i(TAG, "[startContinuousShot]-");
    }

    private boolean stopContinuousShot() {
        State state = mState.getCShotState();
        LogHelper.i(TAG, "[stopContinuousShot]+ state = " + state);
        if (State.STATE_CAPTURING != state && State.STATE_CAPTURE_STARTED != state) {
            LogHelper.w(TAG, "[stopContinuousShot], state is error,don't do anything");
            return false;
        }
        // if no image receive, don't show saving process
        if (mCurrentShotsNum != 0) {
            showSavingProgress(false);
        }
        if (state == State.STATE_CAPTURE_STARTED) {
            mState.updateState(State.STATE_OPENED);
            postRestriction(mState);
        } else {
            mCaptureSound.stop();
            mState.updateState(State.STATE_STOPPED);
        }

        if (mIndicatorViewHandler != null) {
            mIndicatorViewHandler.removeMessages(HIDE_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.sendEmptyMessage(HIDE_CONTINUOUS_SHOT_VIEW);
        }
        mMemoryManager.removeListener(this);
        mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
        mCameraContext.getStorageService().unRegisterStorageStateListener(mStorageStateListener);
        mApp.enableGSensorOrientation();
        mAppUi.applyAllUIEnabled(true);
        LogHelper.i(TAG, "[stopContinuousShot]-");
        return true;
    }

    private void onContinuousShotDone(int capNum) {
        LogHelper.i(TAG, "[onContinuousShotDone]+ Capture number = " + capNum + ",state = " +
                mState.getCShotState());
        mState.updateState(State.STATE_SAVING);
        showSavingProgress(true);
        if (mIndicatorViewHandler != null) {
            mIndicatorViewHandler.removeMessages(HIDE_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.sendEmptyMessage(HIDE_CONTINUOUS_SHOT_VIEW);
        }
        WaitSavingDoneThread waitSavingDoneThread = new WaitSavingDoneThread();
        waitSavingDoneThread.start();
        //need after current is saving state to requestChangeCommand.
        //because configureCommand will check whether is saving state to clear the capture callback.
        //this time will clear the callback.
        mIsShutterCallbackReceived = false;
        LogHelper.i(TAG, "[onContinuousShotDone]-");
    }

    private void initIndicatorView(Camera.Parameters parameters) {
        mIsSupportIndicator = "true".equalsIgnoreCase(parameters.get(KEY_CSHOT_INDICATOR));
        LogHelper.d(TAG, "[initIndicatorView] mIsSupportIndicator = " + mIsSupportIndicator);
        //Create the indicator view and handler if current project support indicator view.
        if (mIsSupportIndicator) {
            mIndicatorViewHandler = new MainHandler(mActivity.getMainLooper());
            mIContinuousShotView = new ContinuousShotView();
            mIndicatorViewHandler.removeMessages(INIT_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.sendEmptyMessage(INIT_CONTINUOUS_SHOT_VIEW);
        }
    }

    private void disableAllUIExceptionShutter() {
        mAppUi.applyAllUIEnabledImmediately(false);
        mAppUi.setUIEnabledImmediately(IAppUi.SHUTTER_BUTTON, true);
        mAppUi.setUIEnabledImmediately(IAppUi.SHUTTER_TEXT, false);
    }

    private void close() {
        mAppUi.unregisterOnShutterButtonListener(this);
        State state = mState.getCShotState();
        LogHelper.d(TAG, "[close]+ state = " + state);
        if (State.STATE_INIT == state) {
            return;
        } else if (State.STATE_CAPTURING == state) {
            // if camera is closed before focus up, we will call stop and done.
            stopContinuousShot();
            dismissSavingProcess();
        } else if (State.STATE_STOPPED == state) {
            dismissSavingProcess();
        }
        mCaptureSound.release();
        mState.updateState(State.STATE_INIT);

        if (mIndicatorViewHandler != null) {
            mIndicatorViewHandler.removeMessages(INIT_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.removeMessages(SHOW_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.removeMessages(HIDE_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.removeMessages(UNINIT_CONTINUOUS_SHOT_VIEW);
            mIndicatorViewHandler.sendEmptyMessage(UNINIT_CONTINUOUS_SHOT_VIEW);
        }

        LogHelper.d(TAG, "[close]-");
    }

    private boolean checkStorageMemory() {
        mFreeStorageForCapture = mCameraContext.getStorageService().getCaptureStorageSpace();
        if (mFreeStorageForCapture <= 0) {
            LogHelper.d(TAG, "storage is full");
            return false;
        } else {
            return true;
        }
    }

    private IStorageService.IStorageStateListener mStorageStateListener =
            new IStorageService.IStorageStateListener() {
        @Override
        public void onStateChanged(int storageState, Intent intent) {
            if (Intent.ACTION_MEDIA_EJECT.equals(intent.getAction())) {
                LogHelper.i(TAG, "[onStateChanged] storage out service Intent.ACTION_MEDIA_EJECT");
                stopContinuousShot();
            }
        }
    };

    private void showSavingProgress(boolean isShotDone) {
        String message = null;
        if (isShotDone) {
            //show the message such as:saving 99 ....
            message = String.format(Locale.ENGLISH, mActivity.getString(R.string
                    .continuous_saving_pictures), mCurrentShotsNum);
        }
        LogHelper.d(TAG, "[showSavingProgress],isShotDone = " + isShotDone + ",msg = " + message);
        mAppUi.showSavingDialog(message, true);
        mAppUi.applyAllUIVisibility(View.INVISIBLE);
    }

    private void dismissSavingProcess() {
        LogHelper.d(TAG, "[dismissSavingProcess]");
        mAppUi.hideSavingDialog();
        mAppUi.applyAllUIVisibility(View.VISIBLE);
    }

    private boolean isReadyForCapture() {
        boolean value = true;
        State state = mState.getCShotState();
        if (State.STATE_OPENED != state) {
            value = false;
            LogHelper.w(TAG, "[isReadyForCapture] wrong state = " + state);
        }
        // check storage space.
        if (mCameraContext.getStorageService().getCaptureStorageSpace() <= 0) {
            LogHelper.w(TAG, "[isReadyForCapture] there is not enough storage space!");
            value = false;
        }
        //check picture size
        String pictureSize = mSettingController.queryValue(KEY_PICTURE_SIZE);
        if (pictureSize == null) {
            value = false;
            LogHelper.w(TAG, "[isReadyForCapture] there is no picture size,need check");
        }

        if (!mIsContinuousShotSupport) {
            value = false;
            LogHelper.w(TAG, "[isReadyForCapture] don't support cs");
        }
        return value;
    }

    private void postRestriction(CsState state) {
        String value = CONTINUOUSSHOT_OFF;
        if (State.STATE_CAPTURE_STARTED == state.getCShotState()) {
            value = CONTINUOUSSHOT_ON;
        }

        mSettingController.postRestriction(
                ContinuousShotRestriction.getRestriction().getRelation(value, true));
        mSettingController.postRestriction(ContinuousShotRestriction.getFocusUiRestriction()
                .getRelation(value, true));
        mSettingController.postRestriction(ContinuousShotRestriction.getFocusSoundRestriction()
                .getRelation(value, true));

        if (State.STATE_CAPTURE_STARTED == state.getCShotState()) {
            if (VALUE_OF_AUTO_SCENE_DETECTION
                    .equals(mSettingController.queryValue(KEY_SCENE_MODE))) {
                mSettingController.postRestriction(
                        ContinuousShotRestriction.getAsdRestriction()
                                .getRelation(CONTINUOUSSHOT_ON, true));
            }
        } else {
            mSettingController.postRestriction(
                    ContinuousShotRestriction
                            .getAsdRestriction()
                            .getRelation(CONTINUOUSSHOT_OFF, true));
        }
        mSettingController.refreshViewEntry();
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

    /**
     * Wait saving done.
     */
    private class WaitSavingDoneThread extends Thread {
        @Override
        public void run() {
            LogHelper.i(TAG, "[WaitSavingDoneThread]wait");
            synchronized (ContinuousShot.this) {
                if (mSavedNum != mCurrentShotsNum) {
                    try {
                        ContinuousShot.this.wait();
                    } catch (InterruptedException e) {
                        LogHelper.e(TAG, "WaitSavingDoneThread exception: " + e);
                    }
                }
            }
            mSavedNum = 0;
            mCurrentShotsNum = 0;
            mState.updateState(CsState.State.STATE_OPENED);
            mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
            postRestriction(mState);
            dismissSavingProcess();
        }
    }

    private ShutterCallback mShutterCallback = new ShutterCallback() {
        @Override
        public void onShutter() {
            synchronized (mStopLock) {
                mShutterTime = System.currentTimeMillis();
                mMemoryManager.initStartTime();
                LogHelper.d(TAG, "[onShutter]");
                mIsShutterCallbackReceived = true;
                // start play sound only at first time when continuous shot
                if (State.STATE_CAPTURING == mState.getCShotState()) {
                    mCaptureSound.play();
                }
            }
        }
    };

    private PictureCallback mJpegCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] bytes, Camera camera) {
            State state = mState.getCShotState();
            LogHelper.d(TAG, "[onPictureTaken]+ bytes = " + bytes + ", currentState = " + state);
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
            //update thumbnail
            updateThumbnail(bytes);
            synchronized (mNumlock) {
                ++mCurrentShotsNum;
            }
            //show continuous shot speed view.
            if (mIndicatorViewHandler != null) {
                mIndicatorViewHandler.removeMessages(SHOW_CONTINUOUS_SHOT_VIEW);
                mIndicatorViewHandler.sendEmptyMessage(SHOW_CONTINUOUS_SHOT_VIEW);
            }

            //Save image.
            Size jpegSize = CameraUtil.getSizeFromExif(bytes);
            ContentValues contentValues = mCsNamingRule.createContentValues(bytes, mFileDirectory,
                    jpegSize.getWidth(), jpegSize.getHeight(), mShutterTime, mCurrentShotsNum);

            mCameraContext.getMediaSaver().addSaveRequest(bytes, contentValues, null,
                    mMediaSaverListener);
            if (mCurrentShotsNum == MAX_CAPTURE_NUMBER) {
                stopContinuousShot();
                return;
            }
            //check DVM and system memory
            mMemoryManager.checkContinuousShotMemoryAction(bytes.length,
                    mCameraContext.getMediaSaver().getBytesWaitingToSave());
            LogHelper.d(TAG, "[onPictureTaken]- mCurrentShotsNum = " + mCurrentShotsNum);
        }
    };

    private void updateThumbnail(byte[] bytes) {
        Bitmap bitmap = BitmapCreator.createBitmapFromJpeg(bytes, mApp.getAppUi()
                .getThumbnailViewWidth());
        mApp.getAppUi().updateThumbnail(bitmap);
    }

    MediaSaverListener mMediaSaverListener = new MediaSaverListener() {
        @Override
        public void onFileSaved(Uri uri) {
            mSavedNum++;
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri + ", savedNum = " +
                    mSavedNum);
            synchronized (mNumlock) {
                if (mSavedNum == mCurrentShotsNum) {
                    synchronized (ContinuousShot.this) {
                        ContinuousShot.this.notifyAll();
                    }
                }
            }
            mApp.notifyNewMedia(uri, true);
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
                    mState.updateState(CsState.State.STATE_OPENED);
                    mSettingDeviceRequester.requestChangeCommand(KEY_CSHOT);
                    postRestriction(mState);
                }
            }
        }
    }

    /**
     * Handler let some task execute in main thread.
     */
    private class MainHandler extends Handler {
        MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage]msg.what = " + msg.what);
            if (mIContinuousShotView == null) {
                return;
            }
            switch (msg.what) {
                case INIT_CONTINUOUS_SHOT_VIEW:
                    mIContinuousShotView.init(mActivity, mAppUi);
                    break;
                case UNINIT_CONTINUOUS_SHOT_VIEW:
                    mIContinuousShotView.unInit();
                    break;
                case SHOW_CONTINUOUS_SHOT_VIEW:
                    mIContinuousShotView.show(String.valueOf(mCurrentShotsNum));
                    break;
                case HIDE_CONTINUOUS_SHOT_VIEW:
                    mIContinuousShotView.hide();
                    break;
                default:
                    break;
            }
        }
    }

    private void initializeValue() {
        List<String> supportedList = new ArrayList<>();
        String defaultValue = CONTINUOUSSHOT_OFF;
        if (mIsContinuousShotSupport) {
            defaultValue = CONTINUOUSSHOT_ON;
        }
        supportedList.add(CONTINUOUSSHOT_ON);
        supportedList.add(CONTINUOUSSHOT_OFF);
        setSupportedPlatformValues(supportedList);
        setSupportedEntryValues(supportedList);
        setEntryValues(supportedList);
        mDataStore.setValue(getKey(), defaultValue, getStoreScope(), false, true);
        setValue(defaultValue);
    }
}
