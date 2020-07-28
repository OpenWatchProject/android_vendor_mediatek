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

import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

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

import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class use to handle exposure feature flow.
 */
public class Exposure extends SettingBase implements ExposureView.ExposureViewChangedListener,
        IExposure, IAppUiListener.OnGestureListener ,IAppUiListener.OnShutterButtonListener{
    private LogUtil.Tag mTag;
    private ICameraMode.ModeType mCurrentModeType = ICameraMode.ModeType.PHOTO;

    private static final String EXPOSURE_VIEW_KEY = "key_exposure_view";
    private static final String EXPOSURE_KEY = "key_exposure";
    private static final String EXPOSURE_LOCK = "exposure-lock";

    private static final String FOCUS_STATE_KEY = "key_focus_state";
    private static final String FLASH_KEY = "key_flash";
    private static final String FLASH_VAULE_ON = "on";
    private static final String FLASH_VAULE_AUTO = "auto";
    private final static int EXPOSURE_VIEW_PRIORITY =  9;
    private static final int EXPOSURE_SHUTTER_PRIORITY = 30;

    private ISettingChangeRequester mSettingChangeRequester;
    private ExposureViewController mExposureViewController;
    private IExposure.Listener mExposureListener;
    private StatusMonitor.StatusResponder mViewStatusResponder;
    private boolean mPreviewStarted;
    private int mCompensationOrientation = 0;

    private static final String FOCUS_STATE_PASSIVE_SCAN = "PASSIVE_SCAN";

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
        int orientation = mApp.getGSensorOrientation();
        int compensation = CameraUtil.getDisplayRotation(mActivity);
        mCompensationOrientation = orientation + compensation;
        LogHelper.d(mTag, "[init] - mCompensationOrientation " + mCompensationOrientation);
    }

    @Override
    public void unInit() {
        LogHelper.d(mTag, "[unInit] + ");
        mAppUi.unregisterGestureListener(this);
        mApp.getAppUi().unregisterOnShutterButtonListener(this);
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mStatusMonitor.unregisterValueChangedListener(FOCUS_STATE_KEY, mStatusChangeListener);
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
            mExposureListener.setAeLock(Boolean.parseBoolean(supportValues.get(0)));
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
                    mSettingDevice2Requester);
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
        mExposureViewController.resetExposureView();
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
        if (!hasDisableEvReset()) {
            mExposureListener.updateEv(0);
        }
        if (mExposureListener != null) {
            mExposureListener.setAeLock(false);
        }
        if (mSettingChangeRequester != null) {
            mSettingChangeRequester.sendSettingChangeRequest();
        }
        if (mExposureViewController != null) {
            mExposureViewController.resetExposureView();
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
        // no need Ae preTrigger in API1
        if (mExposureListener != null && !mExposureListener.needConsiderAePretrigger()) {
            return false;
        }
        if (mExposureListener != null && needAePreTriggerAndCapture()) {
            mExposureListener.doAePreTrigger();
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

    @Override
    public void onTrackingTouchStatusChanged(boolean start) {
        if (!isEnvironmentReady()) {
            return;
        }
        mViewStatusResponder.statusChanged(EXPOSURE_VIEW_KEY, String.valueOf(start));
    }

    @Override
    public void initExposureCompensation(int[] values) {
        mExposureViewController.initExposureValues(values);
    }

    protected void capture() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAppUi.triggerShutterButtonClick(EXPOSURE_SHUTTER_PRIORITY);
            }
        });
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
            if (!key.equals(FOCUS_STATE_KEY)) {
                return;
            }
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
        }
    };

    private boolean needAePreTriggerAndCapture() {
        boolean needAePretrigger = false;
        String flashValue = mSettingController.queryValue(FLASH_KEY);
        if (FLASH_VAULE_ON.equals(flashValue) || FLASH_VAULE_AUTO.equals(flashValue)) {
            needAePretrigger = true;
        } else {
            needAePretrigger = false;
        }
        return needAePretrigger;
    }

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

}
