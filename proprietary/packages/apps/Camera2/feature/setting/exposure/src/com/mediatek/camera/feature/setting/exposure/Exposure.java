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
package com.mediatek.camera.feature.setting.exposure;

import android.app.Activity;
import android.content.Intent;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.provider.Settings;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.WindowManager;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.pq.PictureQuality;
import com.mediatek.camera.portability.IWindowManagerExt;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class use to handle exposure feature flow.
 */
public class Exposure extends SettingBase implements ExposureView.ExposureViewChangedListener,
        IExposure, IAppUiListener.OnGestureListener, IAppUiListener.OnShutterButtonListener {
    private LogUtil.Tag mTag;
    private ICameraMode.ModeType mCurrentModeType = ICameraMode.ModeType.PHOTO;

    private static final String EXPOSURE_VIEW_KEY = "key_exposure_view";
    private static final String EXPOSURE_KEY = "key_exposure";
    private static final String EXPOSURE_LOCK = "exposure-lock";
    private static final String VIDEO_STATUS_KEY = "key_video_status";
    private static final String VIDEO_STATUS_RECORDING = "recording";
    private static final String VIDEO_STATUS_PREVIEW = "preview";
    private static final String CSHOT_KEY = "key_continuous_shot";
    private static final String VALUE_CSHOT_START = "start";
    private static final String VALUE_CSHOT_STOP = "stop";
    private static final String KEY_SHUTTER_SPEED = "key_shutter_speed";
    private static final String FOCUS_STATE_KEY = "key_focus_state";
    private static final String FLASH_KEY = "key_flash";
    private static final String ZSD_KEY = "key_zsd";
    private static final String FLASH_VAULE_ON = "on";
    private static final String FLASH_VAULE_AUTO = "auto";
    private static final int EXPOSURE_VIEW_PRIORITY = 9;
    private static final int EXPOSURE_SHUTTER_PRIORITY = 30;
    private static final int LED_BRIGHTNESS = 4096;
    private static final int ANIMATOR_DURATION_SCALE_SELECTOR = 2;
    private static final int DEFAULT_VALUE = 0;

    private ISettingChangeRequester mSettingChangeRequester;
    private ExposureViewController mExposureViewController;
    private IExposure.Listener mExposureListener;
    private StatusMonitor.StatusResponder mViewStatusResponder;
    private String mLastModeState = ICameraMode.MODE_DEVICE_STATE_UNKNOWN;
    private boolean mPreviewStarted;
    private int mCompensationOrientation = 0;
    private int mDefaultBrightNess = 0;
    private static final String FOCUS_STATE_PASSIVE_SCAN = "PASSIVE_SCAN";
    private boolean mIsPanelOn = false;
    private int mCurrESSLEDMinStep = 0;
    private int mCurrESSOLEDMinStep = 0;

    private float mPrevBright;

    @Override
    public void init(IApp app, ICameraContext cameraContext, ISettingManager.SettingController
            settingController) {
        super.init(app, cameraContext, settingController);
        mTag = new LogUtil.Tag(Exposure.class.getSimpleName() + "-" +
                settingController.getCameraId());
        LogHelper.d(mTag, "[init] + ");
        mExposureViewController = new ExposureViewController(mApp, this);
        mAppUi.registerGestureListener(this, EXPOSURE_VIEW_PRIORITY);
        mApp.registerOnOrientationChangeListener(mOrientationListener);
        mApp.getAppUi().registerOnShutterButtonListener(this, EXPOSURE_SHUTTER_PRIORITY);
        mViewStatusResponder = mStatusMonitor.getStatusResponder(EXPOSURE_VIEW_KEY);
        mStatusMonitor.registerValueChangedListener(FOCUS_STATE_KEY, mStatusChangeListener);
        mStatusMonitor.registerValueChangedListener(VIDEO_STATUS_KEY, mStatusChangeListener);
        mStatusMonitor.registerValueChangedListener(CSHOT_KEY, mStatusChangeListener);
        int orientation = mApp.getGSensorOrientation();
        int compensation = CameraUtil.getDisplayRotation(mActivity);
        mCompensationOrientation = orientation + compensation;
        mDefaultBrightNess = getScreenBrightness();
        mPrevBright =
                IWindowManagerExt
                        .getWindowManagerService()
                        .getAnimationScale(ANIMATOR_DURATION_SCALE_SELECTOR);
        LogHelper.d(mTag, "[init] - mCompensationOrientation " + mCompensationOrientation);
    }

    @Override
    public void unInit() {
        LogHelper.d(mTag, "[unInit] + ");
        mLastModeState = ICameraMode.MODE_DEVICE_STATE_UNKNOWN;
        mAppUi.unregisterGestureListener(this);
        mApp.getAppUi().unregisterOnShutterButtonListener(this);
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mStatusMonitor.unregisterValueChangedListener(VIDEO_STATUS_KEY, mStatusChangeListener);
        mStatusMonitor.unregisterValueChangedListener(FOCUS_STATE_KEY, mStatusChangeListener);
        mStatusMonitor.unregisterValueChangedListener(CSHOT_KEY, mStatusChangeListener);
        mCurrentModeType = ICameraMode.ModeType.PHOTO;
        LogHelper.d(mTag, "[unInit] - ");
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(mTag, "[onModeOpened] modeKey " + modeKey + ",modeType " + modeType);
        mCurrentModeType = modeType;
    }

    @Override
    public void onModeClosed(String modeKey) {
        if (mExposureListener != null) {
            mExposureListener.updateEv(0);
            mExposureListener.setAeLock(false);
        }
        if (mIsPanelOn) {
            setPanel(false, -1);
        }
        mLastModeState = ICameraMode.MODE_DEVICE_STATE_UNKNOWN;
    }

    @Override
    public void postRestrictionAfterInitialized() {
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue, List<String>
            supportValues) {
        LogHelper.d(mTag, "[overrideValues] + headerKey = " + headerKey + " ,currentValue = " +
                currentValue + ",supportValues " + supportValues);
        if (EXPOSURE_LOCK.equals(currentValue)) {
            boolean needAeLock = Boolean.parseBoolean(supportValues.get(0));
            if (needAeLock) {
                doAeLock();
            } else {
                doAeUnLock();
                if (mExposureListener != null && getEntryValues().size() > 1
                        && getValue() != null && !getValue().equals("0")) {
                    mExposureListener.updateEv(0);
                    if (mSettingChangeRequester != null) {
                        mSettingChangeRequester.sendSettingChangeRequest();
                    }
                }
                if (mExposureViewController != null) {
                    mExposureViewController.resetExposureView();
                }
            }
            return;
        }
        super.overrideValues(headerKey, currentValue, supportValues);
        mExposureViewController.resetExposureView();
        if (getValue() != null) {
            mExposureListener.overrideExposureValue(getValue(), getEntryValues());
        }
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return EXPOSURE_KEY;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester = new ExposureParameterConfigure(this,
                    mSettingDeviceRequester);
            mExposureListener = (IExposure.Listener) mSettingChangeRequester;
        }
        return (ExposureParameterConfigure) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester = new ExposureCaptureRequestConfigure(this,
                    mSettingDevice2Requester, mActivity.getApplicationContext());
            mExposureListener = (IExposure.Listener) mSettingChangeRequester;
        }
        mPreviewStarted = true;
        return (ExposureCaptureRequestConfigure) mSettingChangeRequester;
    }

    @Override
    public PreviewStateCallback getPreviewStateCallback() {
        return mPreviewStateCallback;
    }

    @Override
    public boolean onDown(MotionEvent event) {
        return false;
    }

    @Override
    public boolean onUp(MotionEvent event) {
        if (!isEnvironmentReady()) {
            return false;
        }
        if (!mExposureViewController.needUpdateExposureView()) {
            return false;
        }
        mExposureViewController.onTrackingTouch(false);
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        return false;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
        if (!isEnvironmentReady()) {
            return false;
        }
        if (!mExposureViewController.needUpdateExposureView()) {
            return false;
        }
        float deltaY = e2.getY() - e1.getY();
        float deltaX = e2.getX() - e1.getX();
        if (Math.abs(deltaX) > Math.abs(deltaY)) {
            if (mCompensationOrientation == 90 || mCompensationOrientation == 270) {
                mExposureViewController.onVerticalScroll(e2, distanceX);
                return true;
            }
        } else {
            if (mCompensationOrientation == 0 || mCompensationOrientation == 180) {
                mExposureViewController.onVerticalScroll(e2, distanceY);
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        if (!isEnvironmentReady()) {
            return false;
        }
        return false;
    }

    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        return false;
    }

    @Override
    public boolean onDoubleTap(float x, float y) {
        return false;
    }

    @Override
    public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
        return false;
    }

    @Override
    public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
        return false;
    }

    @Override
    public boolean onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
        return false;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        LogHelper.d(mTag, "[onLongPress]");
        if (!isEnvironmentReady()) {
            return false;
        }
        return false;
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return false;
    }

    @Override
    public boolean onShutterButtonClick() {
        if (!isEnvironmentReady()) {
            return false;
        }
        // no need to do Ae preTrigger when video shutter button click
        if (ICameraMode.ModeType.VIDEO.equals(mCurrentModeType)) {
            return false;
        }
        if (mLastModeState != ICameraMode.MODE_DEVICE_STATE_PREVIEWING) {
            return false;
        }
        // no need Ae preTrigger in API1
        if (mExposureListener != null && !mExposureListener.needConsiderAePretrigger()) {
            return false;
        }
        if (mExposureListener != null && mExposureListener.checkTodoCapturAfterAeConverted()) {
            LogHelper.d(mTag, "[onShutterButtonClick] need do capture after AE converted");
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        return false;
    }

    @Override
    public void onExposureViewChanged(int value) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                LogHelper.d(mTag, "[onExposureViewChanged]+ value " + value);
                if (mExposureListener != null) {
                    mExposureListener.updateEv(value);
                }
                if (!isEnvironmentReady()) {
                    return;
                }
                if (mSettingChangeRequester != null) {
                    mSettingChangeRequester.sendSettingChangeRequest();
                }
                LogHelper.d(mTag, "[onExposureViewChanged] - ");
            }
        });
    }

    @Override
    public void onTrackingTouchStatusChanged(boolean start) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (!isEnvironmentReady()) {
                    return;
                }
                mViewStatusResponder.statusChanged(EXPOSURE_VIEW_KEY, String.valueOf(start));
            }
        });
    }

    @Override
    public void initExposureCompensation(int[] values) {
        mExposureViewController.initExposureValues(values);
    }

    @Override
    public void updateModeDeviceState(String newState) {
        LogHelper.d(mTag, "[updateModeDeviceState] + newState = " +
                newState + ",mLastModeState = " + mLastModeState);
        if (newState.equals(mLastModeState)) {
            return;
        } else {
            mLastModeState = newState;
        }
        switch (newState) {
            case ICameraMode.MODE_DEVICE_STATE_PREVIEWING:
                if (!hasDisableEvReset() && getValue() != null && !getValue().equals("0")) {
                    mExposureListener.updateEv(0);
                    if (mSettingChangeRequester != null) {
                        mSettingChangeRequester.sendSettingChangeRequest();
                    }
                }
                break;
            default:
                break;
        }
    }

    /**
     * Get current mode type.
     *
     * @return mModeType current mode type.
     */
    protected ICameraMode.ModeType getCurrentModeType() {
        return mCurrentModeType;
    }

    protected String getCurrentFlashValue() {
        String flashValue = mSettingController.queryValue(FLASH_KEY);
        LogHelper.d(mTag, "[getCurrentFlashValue] flashValue " + flashValue);
        return flashValue;
    }

    protected String getCurrentShutterValue() {
        String shutterValue = mSettingController.queryValue(KEY_SHUTTER_SPEED);
        return shutterValue;
    }

    protected void capture() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAppUi.triggerShutterButtonClick(EXPOSURE_SHUTTER_PRIORITY);
            }
        });
    }

    protected void setPanel(boolean on, int brightness) {
        LogHelper.d(mTag, "[setPanel] to " + on + ",brightness = " + brightness);
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {

                mIsPanelOn = on;
                IWindowManagerExt wm = IWindowManagerExt.getWindowManagerService();
                if (on) {
                    mCurrESSLEDMinStep = PictureQuality.getMinStepOfESSLED();
                    mCurrESSOLEDMinStep = PictureQuality.getMinStepOfESSOLED();
                    LogHelper.d(mTag, "[setPanel] mCurrESSLEDMinStep " + mCurrESSLEDMinStep +
                            ",mCurrESSOLEDMinStep " + mCurrESSOLEDMinStep);
                    wm.setAnimationScale(ANIMATOR_DURATION_SCALE_SELECTOR, DEFAULT_VALUE);
                    PictureQuality.setMinStepOfESSLED(LED_BRIGHTNESS);
                    PictureQuality.setMinStepOfESSOLED(LED_BRIGHTNESS);
                    mAppUi.updateBrightnessBackGround(true);
                    WindowManager.LayoutParams lp = mApp.getActivity().getWindow().getAttributes();
                    lp.screenBrightness = brightness * 1.0f / 255;
                    mApp.getActivity().getWindow().setAttributes(lp);
                } else {
                    wm.setAnimationScale(ANIMATOR_DURATION_SCALE_SELECTOR, mPrevBright);
                    PictureQuality.setMinStepOfESSLED(mCurrESSLEDMinStep);
                    PictureQuality.setMinStepOfESSOLED(mCurrESSOLEDMinStep);
                    WindowManager.LayoutParams lp = mApp.getActivity().getWindow().getAttributes();
                    lp.screenBrightness = mDefaultBrightNess * 1.0f / 255;
                    mApp.getActivity().getWindow().setAttributes(lp);
                    mAppUi.updateBrightnessBackGround(false);
                }
            }
        });
    }

    protected boolean isPanelOn() {
        return mIsPanelOn;
    }

    protected boolean isThirdPartyIntent() {
        Activity activity = mApp.getActivity();
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        boolean value = MediaStore.ACTION_IMAGE_CAPTURE.equals(action) ||
                MediaStore.ACTION_VIDEO_CAPTURE.equals(action);
        return value;
    }

    private IApp.OnOrientationChangeListener mOrientationListener =
            new IApp.OnOrientationChangeListener() {
                @Override
                public void onOrientationChanged(int orientation) {
                    int compensation = CameraUtil.getDisplayRotation(mActivity);
                    mCompensationOrientation = orientation + compensation;
                    LogHelper.d(mTag, "[onOrientationChanged] mCompensationOrientation " +
                            mCompensationOrientation);
                    mExposureViewController.setOrientation(mCompensationOrientation);
                }
            };

    private PreviewStateCallback mPreviewStateCallback =
            new PreviewStateCallback() {
                @Override
                public void onPreviewStopped() {
                    LogHelper.d(mTag, "[onPreviewStopped]");
                    mPreviewStarted = false;
                    mExposureViewController.setViewEnabled(false);
                }

                @Override
                public void onPreviewStarted() {
                    LogHelper.d(mTag, "[onPreviewStarted]");
                    mPreviewStarted = true;
                    mExposureViewController.setViewEnabled(true);
                    if (!hasDisableEvReset()) {
                        mExposureListener.updateEv(0);
                    }
                }
            };

    private StatusMonitor.StatusChangeListener mStatusChangeListener = new StatusMonitor
            .StatusChangeListener() {

        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.d(mTag, "[onStatusChanged] + key " + key + "," +
                    "value " + value);
            switch (key) {
                case FOCUS_STATE_KEY:
                    if (hasDisableEvReset()) {
                        return;
                    }
                    if (value.equals(FOCUS_STATE_PASSIVE_SCAN) && getValue() != String.valueOf(0)) {
                        if (mExposureListener != null) {
                            mExposureListener.updateEv(0);
                        }
                        if (mSettingChangeRequester != null) {
                            mSettingChangeRequester.sendSettingChangeRequest();
                        }
                    }
                    break;
                case VIDEO_STATUS_KEY:
                    if (VIDEO_STATUS_RECORDING.equals(value)) {
                        ((ExposureCaptureRequestConfigure) mSettingChangeRequester)
                                .changeFlashToTorchByAeState(true);
                    } else if (VIDEO_STATUS_PREVIEW.equals(value)) {
                        ((ExposureCaptureRequestConfigure) mSettingChangeRequester)
                                .changeFlashToTorchByAeState(false);
                    }
                    break;
                case CSHOT_KEY:
                    if (VALUE_CSHOT_START.equals(value)) {
                        ((ExposureCaptureRequestConfigure) mSettingChangeRequester)
                                .changeFlashToTorchByAeState(true);
                    } else if (VALUE_CSHOT_STOP.equals(value)) {
                        ((ExposureCaptureRequestConfigure) mSettingChangeRequester)
                                .changeFlashToTorchByAeState(false);
                    }
                    break;
                default:
                    break;

            }

        }
    };

    private boolean isEnvironmentReady() {
        if (!mPreviewStarted) {
            LogHelper.w(mTag, "[isEnvironmentReady] preview not started ");
            return false;
        }
        return true;
    }

    /**
     * Check EV reset to 0 is disable or not.
     *
     * @return True if EV reset to 0 is disabled.
     */
    private boolean hasDisableEvReset() {
        //Disable EV reset to 0 when AE/AF has been Locked.
        if (mExposureListener != null && mExposureListener.getAeLock()) {
            return true;
        }
        ///Disable EV reset to 0 when EV has been restricted to a specific value only.
        if (getEntryValues().size() <= 1) {
            return true;
        }
        return false;
    }

      void doAeLock() {
        mExposureListener.setAeLock(true);
    }

    //to do unlock before lock if necessary
    private void doAeUnLock() {
        if (mExposureListener != null && mExposureListener.getAeLock()) {
            mExposureListener.setAeLock(false);
            if (mSettingChangeRequester != null) {
                mSettingChangeRequester.sendSettingChangeRequest();
            }
        }
    }

    private Integer getScreenBrightness() {
        int brightness = 0;
        try {
            brightness =
                    android.provider.Settings.System.getInt(mApp.getActivity().getContentResolver(),
                            android.provider.Settings.System.SCREEN_BRIGHTNESS);
        } catch (Settings.SettingNotFoundException e) {
            LogHelper.d(mTag, "[getScreenBrightness] SettingNotFoundException");
        }
        LogHelper.d(mTag, "[getScreenBrightness] brightness " + brightness);
        return brightness;
    }

    /**
     * Get current camera id.
     * @return The current camera id.
     */
    protected int getCameraId() {
        int cameraId = Integer.parseInt(mSettingController.getCameraId());
        return cameraId;
    }
}
