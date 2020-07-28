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

import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera;
import android.hardware.Camera.Area;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.IAppUiListener.OnGestureListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.CoordinatesTransform;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.setting.focus.IFocusController.AutoFocusState;
import com.mediatek.camera.feature.setting.focus.IFocusView.FocusViewState;
import com.mediatek.camera.portability.SystemProperties;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class use to handle Focus feature flow.
 */
public class Focus extends SettingBase implements
                                        IFocus,
                                        OnGestureListener,
                                        IAppUiListener.OnShutterButtonListener {
    private LogUtil.Tag mTag;
    private static final String ROI_DEBUG_PROPERTY = "vendor.mtk.camera.app.3a.debug";
    private static final String FOCUS_KEY = "key_focus";
    private FocusViewController mFocusViewController;
    private String mLockIndicator = "";
    private ICameraMode.ModeType mCurrentModeType = ICameraMode.ModeType.PHOTO;
    private String mCurrentMode = "com.mediatek.camera.common.mode.photo.PhotoMode";
    private FocusParameterConfigure mFocusParameterConfigure;
    private FocusCaptureRequestConfigure mFocusRequestConfigure;
    private ISettingChangeRequester mSettingChangeRequester;
    private IFocusController mFocusController;
    private IFocus.Listener mFocusListener = null;
    private MainHandler mMainHandler;
    private StatusMonitor.StatusResponder mFocusStateStatusResponder;

    private String mModeDeviceState = ICameraMode.MODE_DEVICE_STATE_UNKNOWN;
    private static final String EXPOSURE_VIEW_KEY = "key_exposure_view";
    private static final String FACE_EXIST_KEY = "key_face_exist";
    private static final String FOCUS_STATE_KEY = "key_focus_state";
    private static final String FOCUS_MODE_KEY = "key_focus_mode";
    private static final String FOCUS_MODE_SINGLE = "single";
    private static final String FOCUS_MODE_MULTI = "multi";
    private static final String FOCUS_SOUND = "focus-sound";
    private static final String FOCUS_UI = "focus-ui";
    private static final String FOCUS_LOCK = "focus-lock";

    private final static int FOCUS_VIEW_PRIORITY = 10;
    private static final int FOCUS_SHUTTER_PRIORITY = 20;

    private final RectF mPreviewRect = new RectF(0, 0, 0, 0);

    private static final int RESET_TOUCH_FOCUS = 1;
    private static final int FOCUS_STATE_UPDATE = 2;

    private boolean mInitialized;
    private boolean mPreviewStarted;
    private List<Camera.Area> mFocusArea;
    private List<Area> mMeteringArea;
    private boolean mMirror = false;
    private int mDisplayOrientation;
    private boolean mIsEvChanging = false;
    private boolean mIsFaceExist = false;
    private boolean mNeedResetTouchFocus = false;
    private boolean mNeedTriggerShutterButton;
    private boolean mNeedPlayFocusSound = true;
    private boolean mNeedShowFocusUi = true;
    private boolean mIsAutoFocusTriggered;
    private boolean mFocusStateUpdateDisabled = false;

    private Point mLockPoint = new Point();
    private IFocus.LockState mLockState = LockState.STATE_UNLOCKED;
    private IFocus.AfModeState mAfModeState = AfModeState.STATE_INVALID;

    private static final String FOCUS_MODE_CONTINUOUS_PICTURE = "continuous-picture";
    private static final String FOCUS_MODE_CONTINUOUS_VIDEO = "continuous-video";
    /**
     * Width of touch AF region in [0,1] relative to shorter edge of the current
     * crop region. Multiply this number by the number of pixels along the
     * shorter edge of the current crop region's width to get a value in pixels.
     * <p>
     * This value has been tested on Nexus 5 and Shamu, but will need to be
     * tuned per device depending on how its ISP interprets the metering box and
     * weight.
     * <p>
     * Values prior to L release:
     * Normal mode: 0.125 * longest edge
     * Gcam: Fixed at 300px x 300px.
     */
    private static final float AF_REGION_BOX = 0.2f;

    /**
     * Width of touch metering region in [0,1] relative to shorter edge of the
     * current crop region. Multiply this number by the number of pixels along
     * shorter edge of the current crop region's width to get a value in pixels.
     * <p>
     * This value has been tested on Nexus 5 and Shamu, but will need to be
     * tuned per device depending on how its ISP interprets the metering box and
     * weight.
     * <p>
     * Values prior to L release:
     * Normal mode: 0.1875 * longest edge
     * Gcam: Fixed  ffat 300px x 300px.
     */
    private static final float AE_REGION_BOX = 0.3f;
    /**
     * Duration to hold after manual tap to focus.
     */
    private static int sFocusHoldMills = 3000;

    private IAppUi.HintInfo mLockIndicatorHint;
    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mTag = new LogUtil.Tag(Focus.class.getSimpleName() + "-" + settingController.getCameraId());
        LogHelper.d(mTag, "[init]+");
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
        mAppUi.registerGestureListener(this, FOCUS_VIEW_PRIORITY);
        mAppUi.registerOnShutterButtonListener(this, FOCUS_SHUTTER_PRIORITY);
        setMirror(Integer.valueOf(mSettingController.getCameraId()));
        setDisplayOrientation();
        mFocusStateStatusResponder = mStatusMonitor.getStatusResponder(FOCUS_STATE_KEY);
        mStatusMonitor.registerValueChangedListener(FOCUS_MODE_KEY, mFocusStatusChangeListener);
        mStatusMonitor.registerValueChangedListener(FACE_EXIST_KEY, mFocusStatusChangeListener);
        mFocusViewController = new FocusViewController(mApp, this);
        mLockIndicator = mActivity.getString(R.string.aeaf_lock_indicator);
        LogHelper.d(mTag, "[init]-");

        mLockIndicatorHint = new IAppUi.HintInfo();
        mLockIndicatorHint.mBackground = mActivity.getDrawable(R.drawable.focus_hint_background);
        mLockIndicatorHint.mType = IAppUi.HintType.TYPE_ALWAYS_TOP;
        mLockIndicatorHint.mHintText = mLockIndicator;

        if (SystemProperties.getInt(ROI_DEBUG_PROPERTY, 0) == 1) {
            LogHelper.d(mTag, "[init] in roi debug mode, set sFocusHoldMills = 5000");
            sFocusHoldMills = 5000;
        }
    }

    @Override
    public void unInit() {
        LogHelper.d(mTag, "[unInit]+");
        mModeDeviceState = ICameraMode.MODE_DEVICE_STATE_UNKNOWN;
        if (mFocusController != null) {
            mFocusController.setFocusStateListener(null);
        }
        mMainHandler.removeMessages(RESET_TOUCH_FOCUS);
        mMainHandler.removeMessages(FOCUS_STATE_UPDATE);
        mAppUi.hideScreenHint(mLockIndicatorHint);
        mLockState = LockState.STATE_UNLOCKED;
        mAppUi.unregisterGestureListener(this);
        mAppUi.unregisterOnShutterButtonListener(this);
        mStatusMonitor.unregisterValueChangedListener(FOCUS_MODE_KEY, mFocusStatusChangeListener);
        mStatusMonitor.unregisterValueChangedListener(FACE_EXIST_KEY, mFocusStatusChangeListener);
        mCurrentModeType = ICameraMode.ModeType.PHOTO;
        mIsEvChanging = false;
        mIsFaceExist = false;
        mNeedResetTouchFocus = false;
        mNeedPlayFocusSound = true;
        mFocusStateUpdateDisabled = false;
        LogHelper.d(mTag, "[unInit]-");
    }

    @Override
    public void addViewEntry() {
        LogHelper.d(mTag, "[addViewEntry]");
        mFocusViewController.addFocusView();
        mStatusMonitor.registerValueChangedListener(EXPOSURE_VIEW_KEY, mFocusStatusChangeListener);
        mApp.registerOnOrientationChangeListener(mOrientationListener);
        mAppUi.registerOnPreviewAreaChangedListener(mPreviewAreaChangedListener);
    }

    @Override
    public void removeViewEntry() {
        LogHelper.d(mTag, "[removeViewEntry]");
        mFocusViewController.clearFocusUi();
        mFocusViewController.removeFocusView();
        mStatusMonitor.unregisterValueChangedListener(EXPOSURE_VIEW_KEY,
                mFocusStatusChangeListener);
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mAppUi.unregisterOnPreviewAreaChangedListener(mPreviewAreaChangedListener);
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(mTag, "[onModeOpened] modeKey " + modeKey + ",modeType " + modeType);
        mCurrentMode = modeKey;
        mCurrentModeType = modeType;
    }

    @Override
    public void onModeClosed(String modeKey) {
        LogHelper.d(mTag, "[onModeClosed]");
        super.onModeClosed(modeKey);
        mAppUi.hideScreenHint(mLockIndicatorHint);
        mLockState = LockState.STATE_UNLOCKED;
        if (mFocusViewController != null) {
            mFocusViewController.clearFocusUi();
        }
        if (mFocusListener != null) {
            mFocusListener.resetConfiguration();
            resetFocusArea();
        }
        mIsEvChanging = false;
        mNeedResetTouchFocus = false;
        mNeedPlayFocusSound = true;
        mIsFaceExist = false;
        mFocusStateUpdateDisabled = false;
    }

    @Override
    public void updateModeDeviceState(String newState) {
        mModeDeviceState = newState;
        LogHelper.d(mTag, "[updateModeDeviceState] + newState = " + newState);
        switch (newState) {
            case ICameraMode.MODE_DEVICE_STATE_CAPTURING:
                if (!isLockActive()) {
                    mFocusViewController.clearFocusUi();
                    mFocusViewController.clearAfData();
                }
                mNeedPlayFocusSound = false;
                mNeedShowFocusUi = false;
                mFocusListener.disableUpdateFocusState(true);
                mFocusStateUpdateDisabled = true;
                break;
            case ICameraMode.MODE_DEVICE_STATE_RECORDING:
                if (!isLockActive()) {
                    mFocusViewController.clearFocusUi();
                    mFocusViewController.clearAfData();
                }
                mNeedPlayFocusSound = false;
                mNeedShowFocusUi = true;
                if (getEntryValues() != null && getEntryValues().size() == 1 &&
                        getEntryValues().get(0).equals("auto")) {
                    mFocusListener.disableUpdateFocusState(true);
                    mFocusStateUpdateDisabled = true;
                }
                break;
            case ICameraMode.MODE_DEVICE_STATE_PREVIEWING:
                mNeedPlayFocusSound = true;
                mNeedShowFocusUi = true;
                mFocusListener.disableUpdateFocusState(false);
                mFocusStateUpdateDisabled = false;
                break;
            default:
                break;
        }
        LogHelper.d(mTag, "[updateModeDeviceState] - mNeedPlayFocusSound = " +
                mNeedPlayFocusSound + ",mNeedShowFocusUi = " + mNeedShowFocusUi);
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return FOCUS_KEY;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mFocusParameterConfigure == null) {
            mFocusParameterConfigure = new FocusParameterConfigure(this, mSettingDeviceRequester);
            mSettingChangeRequester = mFocusParameterConfigure;
            mFocusListener = (IFocus.Listener) mFocusParameterConfigure;
            mFocusController = mFocusParameterConfigure;
            mFocusController.setFocusStateListener(mFocusStateListener);
        }
        return mFocusParameterConfigure;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mFocusRequestConfigure == null) {
            mFocusRequestConfigure = new FocusCaptureRequestConfigure(this,
                    mSettingDevice2Requester);
            mSettingChangeRequester = mFocusRequestConfigure;
            mFocusListener = (IFocus.Listener) mFocusRequestConfigure;
            mFocusController = mFocusRequestConfigure;
            mFocusController.setFocusStateListener(mFocusStateListener);
        }
        mPreviewStarted = true;
        return mFocusRequestConfigure;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        LogHelper.d(mTag, "[overrideValues] + headerKey = " + headerKey + ",currentValue = " +
                currentValue + ",supportValues " + supportValues);
        if (FOCUS_SOUND.equals(currentValue)) {
            mNeedPlayFocusSound = Boolean.parseBoolean(supportValues.get(0));
            return;
        }
        if (FOCUS_UI.equals(currentValue)) {
            mNeedShowFocusUi = Boolean.parseBoolean(supportValues.get(0));
            return;
        }
        if (FOCUS_LOCK.equals(currentValue)) {
            return;
        }
        super.overrideValues(headerKey, currentValue, supportValues);
        if (mFocusListener != null && getValue() != null && !isLockActive()) {
            mFocusListener.overrideFocusMode(getValue(), getEntryValues());
        }
    }

    @Override
    public PreviewStateCallback getPreviewStateCallback() {
        return mPreviewStateCallback;
    }

    @Override
    public void postRestrictionAfterInitialized() {
        mSettingController.postRestriction(FocusRestriction.getRestriction()
                .getRelation("continuous-picture",
                        true));
    }

    @Override
    public boolean onDown(MotionEvent event) {
        return false;
    }

    @Override
    public boolean onUp(MotionEvent event) {
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        return false;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float dx, float dy) {
        return false;
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        LogHelper.d(mTag, "[onSingleTapUp] + x " + x + ",y = " + y);
        //step1:Check whether auto focus can do
        if (!checkAfEnv()) {
            return false;
        }
        mIsAutoFocusTriggered = true;
        if (needCancelAutoFocus()) {
            mFocusListener.cancelAutoFocus();
        }
        //step2:Check to restore af lock state and UI when needed.
        handleAfLockRestore();
        //step3:Clear any focus UI before show touch focus UI
        mFocusViewController.clearFocusUi();
        //step4:stop face detection
        mSettingController.postRestriction(FocusRestriction.getRestriction().getRelation("auto",
                true));
        //API2 face detection need to config face detect mode
        if (mSettingChangeRequester != null) {
            mSettingChangeRequester.sendSettingChangeRequest();
        }
        //step5:init focus and metering area and show focus UI
        try {
            initializeFocusAreas(x, y);
            initializeMeteringArea(x, y);
        } catch (IllegalArgumentException e) {
            LogHelper.e(mTag, "onSingleTapUp IllegalArgumentException");
            return false;
        }

        if (mNeedShowFocusUi) {
            mFocusViewController.showActiveFocusAt((int) x, (int) y);
        }
        //step6:notify focus area and parameter
        mFocusListener.updateFocusArea(mFocusArea, mMeteringArea);
        //step7:do auto focus
        mMainHandler.removeMessages(RESET_TOUCH_FOCUS);
        mFocusListener.updateFocusMode("auto");
        mFocusListener.autoFocus();
        //exposure need onSingleTapUp to reset exposure progress,so return false.
        LogHelper.d(mTag, "[onSingleTapUp]-");
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
        if (mFocusViewController == null) {
            return false;
        }
        mIsAutoFocusTriggered = true;
        // do not do AE/AF lock in panorama mode and pip mode.
        if (mCurrentMode.equals("com.mediatek.camera.feature.mode.panorama.PanoramaMode")) {
            return false;
        }
        if (mCurrentMode.contains("com.mediatek.camera.feature.mode.pip")) {
            return false;
        }
        mLockPoint.set((int) x, (int) y);
        triggerAfLock();
        return false;
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return false;
    }

    @Override
    public boolean onShutterButtonClick() {
        if (mFocusViewController == null) {
            return false;
        }
        FocusViewState state = mFocusViewController.getFocusState();
        LogHelper.d(mTag, "[onShutterButtonClick] state " + state);
        if (FocusViewState.STATE_ACTIVE_FOCUSING == state &&
                ICameraMode.ModeType.PHOTO.equals(mCurrentModeType)) {
            mNeedTriggerShutterButton = true;
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        if (!isLockActive() && mFocusViewController != null) {
            mFocusViewController.clearFocusUi();
        }
        return false;
    }

    @Override
    public void initPlatformSupportedValues(List<String> platformSupportedValues) {
        setSupportedPlatformValues(platformSupportedValues);
    }

    @Override
    public void initAppSupportedEntryValues(List<String> appSupportedEntryValues) {
        setSupportedEntryValues(appSupportedEntryValues);
    }

    @Override
    public void initSettingEntryValues(List<String> settingEntryValues) {
        setEntryValues(settingEntryValues);
    }

    /**
     * Set multi-zone af data to focus manager.
     * @param data The data of multi-zone af.
     */
    protected void setAfData(byte[] data) {
        mFocusViewController.setAfData(data);
    }

    protected boolean isMultiZoneAfEnabled() {
        return mAfModeState == AfModeState.STATE_MULTI;
    }

    protected boolean isSingleAfEnabled() {
        return mAfModeState == AfModeState.STATE_SINGLE;
    }

    private AfModeState updateAfModeState() {
        String afModeValue = mSettingController.queryValue(FOCUS_MODE_KEY);
        LogHelper.d(mTag, "[updateAfModeState]+ currentAfMode = " + afModeValue);
        if (FOCUS_MODE_SINGLE.equals(afModeValue)) {
            mAfModeState = AfModeState.STATE_SINGLE;
        } else if (FOCUS_MODE_MULTI.equals(afModeValue)) {
            mAfModeState = AfModeState.STATE_MULTI;
        } else {
            mAfModeState = AfModeState.STATE_INVALID;
        }
        LogHelper.d(mTag, "[updateAfModeState]- mAfModeState = " + mAfModeState);
        return mAfModeState;
    }

    /**
     * Handler let some task execute in main thread.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(mTag, "[handleMessage] msg.what = " + msg.what);
            if (mFocusViewController == null) {
                LogHelper.w(mTag, "[handleMessage] mFocusViewController is null ");
                return;
            }
            switch (msg.what) {
                case RESET_TOUCH_FOCUS:
                    mFocusViewController.stopFocusAnimations();
                    if (getEntryValues() != null && getEntryValues().size() == 1 &&
                            getEntryValues().get(0).equals("auto")) {
                        mFocusViewController.clearFocusUi();
                    }
                    mFocusListener.restoreContinue();
                    mNeedResetTouchFocus = false;
                    //start face detection
                    mSettingController.postRestriction(FocusRestriction.getRestriction()
                            .getRelation("continuous-picture",
                                    true));
                    if (mSettingChangeRequester != null) {
                        mSettingChangeRequester.sendSettingChangeRequest();
                    }
                    break;
                case FOCUS_STATE_UPDATE:
                    onFocusStateUpdate((AutoFocusState) msg.obj);
                    break;
                default:
                    break;
            }
        }
    }

    private PreviewStateCallback mPreviewStateCallback =
            new PreviewStateCallback() {
                @Override
                public void onPreviewStopped() {
                    LogHelper.d(mTag, "[onPreviewStopped]");
                    mFocusListener.resetConfiguration();
                    if (mFocusViewController != null && !isLockActive()) {
                        mFocusViewController.clearFocusUi();
                    }
                    mPreviewStarted = false;
                }

                @Override
                public void onPreviewStarted() {
                    LogHelper.d(mTag, "[onPreviewStarted]");
                    mFocusListener.resetConfiguration();
                    mPreviewStarted = true;
                    updateAfModeState();
                    if (!isLockActive()) {
                        mFocusListener.restoreContinue();
                    }
                }
            };

    private final IFocusController.FocusStateListener mFocusStateListener =
            new IFocusController.FocusStateListener() {

                @Override
                public void onFocusStatusUpdate(AutoFocusState state,
                                                long frameNumber) {
                    mMainHandler.obtainMessage(FOCUS_STATE_UPDATE, state).sendToTarget();
                }
            };

    private IAppUiListener.OnPreviewAreaChangedListener mPreviewAreaChangedListener
            = new IAppUiListener.OnPreviewAreaChangedListener() {
        @Override
        public void onPreviewAreaChanged(RectF newPreviewArea, Size previewSize) {
            setPreviewRect(newPreviewArea);
        }
    };

    private IApp.OnOrientationChangeListener mOrientationListener =
            new IApp.OnOrientationChangeListener() {
                @Override
                public void onOrientationChanged(int orientation) {
                    mFocusViewController.setOrientation(orientation);
                    setDisplayOrientation();
                }
            };

    private StatusMonitor.StatusChangeListener mFocusStatusChangeListener = new StatusMonitor
            .StatusChangeListener() {

        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.d(mTag, "[onStatusChanged]+ key: " + key + "," +
                    "value: " + value + ",mLockState = " + mLockState);
            switch (key) {
                case EXPOSURE_VIEW_KEY:
                    boolean isEvChanging = Boolean.parseBoolean(value);
                    if (!LockState.STATE_LOCKING.equals(mLockState)) {
                        onExposureViewStatusChanged(isEvChanging);
                    }
                    break;
                case FOCUS_MODE_KEY:
                    mFocusViewController.clearFocusUi();
                    updateAfModeState();
                    mFocusListener.updateFocusCallback();
                    break;
                case FACE_EXIST_KEY:
                    mIsFaceExist = Boolean.parseBoolean(value);
                    break;
                default:
                    break;
            }
            LogHelper.d(mTag, "[onStatusChanged]- mNeedShowFocusUi " + mNeedShowFocusUi);
        }
    };

    private void onFocusStateUpdate(AutoFocusState state) {
        mFocusStateStatusResponder.statusChanged(FOCUS_STATE_KEY, state.toString());

        if (mFocusViewController == null) {
            LogHelper.w(mTag, "[onFocusStateUpdate] mFocusViewController is null");
            return;
        }
        if (!mIsAutoFocusTriggered && mIsEvChanging) {
            LogHelper.w(mTag, "[onFocusStateUpdate] mIsEvChanging when not touch");
            return;
        }
        switch (state) {
            case PASSIVE_SCAN:
                LogHelper.d(mTag, "[onFocusStateUpdate] passive focus start with state " +
                        state + " , mNeedShowFocusUi " + mNeedShowFocusUi);
                if (isLockActive() || mFocusViewController.isActiveFocusRunning()) {
                    LogHelper.w(mTag, "[onFocusStateUpdate] ignore the state " + state);
                    return;
                }
                if (mFocusStateUpdateDisabled) {
                    LogHelper.w(mTag, "[onFocusStateUpdate] disable update passive focus state ");
                    return;
                }

                mIsEvChanging = false;
                if (mFocusArea != null || mMeteringArea != null) {
                    resetFocusArea();
                    if (mSettingChangeRequester != null) {
                        mSettingChangeRequester.sendSettingChangeRequest();
                    }
                }

                if (mNeedShowFocusUi) {
                    mFocusViewController.clearFocusUi();
                    if (!mIsFaceExist) {
                        mFocusViewController.showPassiveFocusAtCenter();
                    }
                }
                break;
            case ACTIVE_SCAN:
                // Unused, manual scans are triggered via the UI
                break;
            case PASSIVE_FOCUSED:
            case PASSIVE_UNFOCUSED:
                LogHelper.d(mTag, "[onFocusStateUpdate] passive focus done with state " + state);
                if (isLockActive() || mFocusViewController.isActiveFocusRunning()) {
                    LogHelper.w(mTag, "[onFocusStateUpdate] ignore the state " + state);
                    return;
                }
                mFocusViewController.stopFocusAnimations();
                break;
            case ACTIVE_FOCUSED:
            case ACTIVE_UNFOCUSED:
                LogHelper.d(mTag, "[onFocusStateUpdate] active focus done with state " + state +
                        " , mNeedTriggerShutterButton " + mNeedTriggerShutterButton + " , " +
                        "mNeedPlayFocusSound " + mNeedPlayFocusSound + " , mLockState " +
                        mLockState);
                mIsAutoFocusTriggered = false;
                if (mNeedTriggerShutterButton) {
                    mActivity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (!isLockActive()) {
                                mFocusViewController.clearFocusUi();
                            } else {
                                mFocusViewController.stopFocusAnimations();
                            }
                            mAppUi.triggerShutterButtonClick(FOCUS_SHUTTER_PRIORITY);
                        }
                    });
                    mNeedTriggerShutterButton = false;
                } else {
                    if (mNeedPlayFocusSound) {
                        mCameraContext.getSoundPlayback().play(ISoundPlayback
                                .FOCUS_COMPLETE);
                    }
                    //the focus indicator should be dark 2s later
                    mFocusViewController.stopFocusAnimations();
                    //change to continue focus 3s later ,note that focus indicator position
                    //will not change when continue focus move callback has been received.
                    //no need to clear focus UI when AE/AF lock done
                    if (LockState.STATE_LOCKING.equals(mLockState)) {
                        mLockState = LockState.STATE_LOCKED;
                    } else {
                        resetTouchFocus();
                    }

                }
                break;
            default:
                break;
        }
        LogHelper.d(mTag, "[onFocusStateUpdate]-");
    }

    private void onExposureViewStatusChanged(boolean isEvChanging) {
        if (!mPreviewStarted) {
            LogHelper.w(mTag, "[onExposureViewStatusChanged] mPreviewStarted not started");
            return;
        }
        if (mIsEvChanging != isEvChanging) {
            mIsEvChanging = isEvChanging;
            if (mIsEvChanging) {
                mFocusViewController.highlightFocusView();
            } else {
                mFocusViewController.lowlightFocusView();
            }
            if (FOCUS_MODE_CONTINUOUS_PICTURE.equals(mFocusListener.getCurrentFocusMode()) ||
                    FOCUS_MODE_CONTINUOUS_VIDEO.equals(mFocusListener.getCurrentFocusMode())) {
                if (mIsEvChanging) {
                    mFocusListener.autoFocus();
                } else {
                    mFocusListener.cancelAutoFocus();
                }
            }
            LogHelper.d(mTag, "[onExposureViewStatusChanged] mNeedResetTouchFocus " +
                    mNeedResetTouchFocus);
            if (mNeedResetTouchFocus) {
                if (mIsEvChanging) {
                    mMainHandler.removeMessages(RESET_TOUCH_FOCUS);
                } else {
                    mMainHandler.sendEmptyMessageDelayed(RESET_TOUCH_FOCUS,
                            sFocusHoldMills);
                }
            }
        }
    }

    /**
     * This setter should be the only way to mutate mPreviewRect.
     */
    private void setPreviewRect(RectF previewRect) {
        LogHelper.d(mTag, "[setPreviewRect] ");
        if (!mPreviewRect.equals(previewRect)) {
            mPreviewRect.set(previewRect);
            mFocusViewController.onPreviewChanged(previewRect);
            mInitialized = true;
        }
    }

    private void setDisplayOrientation() {
        int cameraId = Integer.valueOf(mSettingController.getCameraId());
        int displayRotation = CameraUtil.getDisplayRotation(mActivity);
        int displayOrientation;
        // FIXME: work around,setting feature no need to check API1 or API2
        if (mFocusListener instanceof FocusParameterConfigure) {
            displayOrientation = CameraUtil.getDisplayOrientationFromDeviceSpec(
                    displayRotation, cameraId, mApp.getActivity());
        } else {
            displayOrientation = CameraUtil.getV2DisplayOrientation(displayRotation,
                    cameraId, mApp.getActivity());
        }
        if (mDisplayOrientation != displayOrientation) {
            mDisplayOrientation = displayOrientation;
            LogHelper.d(mTag, "[setDisplayOrientation] : mDisplayOrientation = " +
                    mDisplayOrientation +
                    ", cameraId = " + cameraId + ",mMirror = " + mMirror);
        }
    }

    private void initializeFocusAreas(float x, float y) {
        LogHelper.d(mTag, "[initializeFocusAreas]");
        if (mFocusArea == null) {
            mFocusArea = new ArrayList<Area>();
            mFocusArea.add(new Area(new Rect(), 1));
        }
        Rect rect = new Rect();
        CameraUtil.rectFToRect(mPreviewRect, rect);
        // FIXME: work around,will dispatch mFocusArea to device
        if (mFocusListener instanceof FocusParameterConfigure) {
            mFocusArea.get(0).rect = CoordinatesTransform.uiToNormalizedPreview(new Point((int) x,
                            (int) y), rect, AF_REGION_BOX, mMirror,
                    mDisplayOrientation);
        } else {
            int displayRotation = CameraUtil.getDisplayRotation(mActivity);
            mFocusArea.get(0).rect = CoordinatesTransform.uiToSensor(new Point((int) x,
                    (int) y), rect, displayRotation, AE_REGION_BOX,
                    mFocusRequestConfigure.getCropRegion(),
                    mFocusRequestConfigure.getCameraCharacteristics());
        }
    }

    private void initializeMeteringArea(float x, float y) {
        LogHelper.d(mTag, "[initializeMeteringArea]");
        if (mMeteringArea == null) {
            mMeteringArea = new ArrayList<Area>();
            mMeteringArea.add(new Area(new Rect(), 1));
        }
        Rect rect = new Rect();
        CameraUtil.rectFToRect(mPreviewRect, rect);
        // FIXME: work around,will dispatch mFocusArea to device
        if (mFocusListener instanceof FocusParameterConfigure) {
            mMeteringArea.get(0).rect = CoordinatesTransform.uiToNormalizedPreview(new Point(
                    (int) x,
                    (int) y), rect, AE_REGION_BOX, mMirror, mDisplayOrientation);
        } else {
            int displayRotation = CameraUtil.getDisplayRotation(mActivity);
            mMeteringArea.get(0).rect = CoordinatesTransform.uiToSensor(new Point((int) x,
                            (int) y), rect, displayRotation, AE_REGION_BOX,
                    mFocusRequestConfigure.getCropRegion(),
                    mFocusRequestConfigure.getCameraCharacteristics());
        }
    }

    private void resetFocusArea() {
        mFocusArea = null;
        mMeteringArea = null;
        mFocusListener.updateFocusArea(mFocusArea, mMeteringArea);
    }

    private void setMirror(int cameraId) {
        if (cameraId == 0) {
            mMirror = false;
        } else {
            mMirror = true;
        }
    }

    private boolean checkAfEnv() {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M && mActivity.isInMultiWindowMode()) {
            LogHelper.w(mTag, "[checkAfEnv] ignore focus event in MultiWindowMode");
            return false;
        }
        if (ICameraMode.MODE_DEVICE_STATE_CAPTURING == mModeDeviceState) {
            LogHelper.w(mTag, "[checkAfEnv] touch focus has been disabled mModeDeviceState = " +
                    mModeDeviceState);
            return false;
        }
        if (mNeedTriggerShutterButton) {
            LogHelper.w(mTag, "[checkAfEnv] need trigger shutter button");
            return false;
        }
        //ignore touch focus if preview not initialized.
        if (!mPreviewStarted) {
            LogHelper.w(mTag, "[checkAfEnv] preview not started ");
            return false;
        }
        if (!mInitialized) {
            LogHelper.w(mTag, "[checkAfEnv] preview not initialized " + mInitialized);
            return false;
        }
        // If focus view is not ready,do nothing.
        if (!mFocusViewController.isReadyTodoFocus()) {
            return false;
        }
        if (!mFocusListener.isFocusCanDo()) {
            return false;
        }
        // do not do touch focus if some one restriction focus to continue only
        List<String> entryValues = getEntryValues();
        if (entryValues != null && entryValues.size() == 1 &&
                (entryValues.contains("continuous-picture")
                        || entryValues.contains("continuous-video"))) {
            return false;
        }
        return true;
    }

    private void triggerAfLock() {
        LogHelper.d(mTag, "[triggerAfLock]+ ");
        //step1:Check whether auto focus can do
        if (!checkAfEnv()) {
            return;
        }
        if (needCancelAutoFocus()) {
            mFocusListener.cancelAutoFocus();
        }
        mNeedResetTouchFocus = false;
        //step2:Clear any focus UI before show touch focus UI
        mFocusViewController.clearFocusUi();
        //step3:stop face detection
        mSettingController.postRestriction(FocusRestriction.getRestriction().getRelation("auto",
                true));
        //API2 face detection need to config face detect mode
        if(mSettingChangeRequester!=null){
            mSettingChangeRequester.sendSettingChangeRequest();
        }
        //step4:init focus and metering area and show focus UI
        try {
            initializeFocusAreas(mLockPoint.x, mLockPoint.y);
            initializeMeteringArea(mLockPoint.x, mLockPoint.y);
        } catch (IllegalArgumentException e) {
            LogHelper.e(mTag, "triggerAfLock IllegalArgumentException");
            return;
        }
        if (mNeedShowFocusUi) {
            mFocusViewController.showActiveFocusAt(mLockPoint.x, mLockPoint.y);
            mAppUi.hideScreenHint(mLockIndicatorHint);
            mAppUi.showScreenHint(mLockIndicatorHint);
        }
        //step5:notify focus area and parameter
        mFocusListener.updateFocusArea(mFocusArea, mMeteringArea);
        mMainHandler.removeMessages(RESET_TOUCH_FOCUS);
        //step7:post to to ae lock
        mSettingController.postRestriction(FocusRestriction.getAfLockRestriction()
                .getRelation(FocusRestriction.FOCUS_LOCK, true));
        //step6:fix focus mode to auto and do auto focus
        mFocusListener.updateFocusMode("auto");
        mFocusListener.autoFocus();
        mLockState = LockState.STATE_LOCKING;
        LogHelper.d(mTag, "[triggerAfLock]-");
    }


    private void handleAfLockRestore(){
        LogHelper.d(mTag, "[handleAfLockRestore] mLockState " + mLockState);
        if (isLockActive()) {
            mSettingController.postRestriction(FocusRestriction.getAfLockRestriction()
                    .getRelation(FocusRestriction.FOCUS_UNLOCK, true));
            if (mSettingChangeRequester != null) {
                mSettingChangeRequester.sendSettingChangeRequest();
            }
            mLockState = LockState.STATE_UNLOCKED;
            mAppUi.hideScreenHint(mLockIndicatorHint);
        }
    }

    /**
     * Whether lock is active or not.
     *
     * @return True means it is going to lock or has been locked.False means in unlock state.
     */
    private boolean isLockActive() {
        boolean isLockActive = LockState.STATE_LOCKING.equals(mLockState) || LockState
                .STATE_LOCKED.equals(mLockState);
        LogHelper.d(mTag, "[isLockActive] isLockActive =  " + isLockActive);
        return isLockActive;
    }

    private boolean needCancelAutoFocus() {
        if (mFocusViewController == null) {
            LogHelper.w(mTag, "[needCancelAutoFocus] mFocusViewController is null");
            return false;
        }
        FocusViewState state = mFocusViewController.getFocusState();
        boolean result = mFocusArea != null && (FocusViewState.STATE_ACTIVE_FOCUSING == state);
        if (result == false) {
            LogHelper.d(mTag, "[needCancelAutoFocus] no need cancelAutoFocus mFocusArea = " +
                    mFocusArea + ",state=  " + state);
        }
        return result;
    }

    /**
     * Reset touch focus 3s later if ev is not changing.
     * Otherwise,touch focus will be reset 3s later after ev stop changing.
     * Note:No need to reset touch focus in LockState.STATE_LOCKING state.
     */
    private void resetTouchFocus() {
        LogHelper.d(mTag, "[resetTouchFocus] mIsEvChanging = " + mIsEvChanging);
        mNeedResetTouchFocus = true;
        if (!mIsEvChanging) {
            mMainHandler.sendEmptyMessageDelayed(RESET_TOUCH_FOCUS,
                    sFocusHoldMills);
        }
    }

    /**
     * get current mode.
     * @return current mode
     */
    public String getCurrentMode() {
        return mCurrentMode;
    }
}
