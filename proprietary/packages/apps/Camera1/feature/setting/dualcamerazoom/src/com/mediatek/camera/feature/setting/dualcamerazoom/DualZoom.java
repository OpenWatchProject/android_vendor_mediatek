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

package com.mediatek.camera.feature.setting.dualcamerazoom;

import android.hardware.Camera;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.widget.SeekBar;

import com.mediatek.camera.common.IAppUiListener.OnGestureListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.utils.CameraUtil;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class is for zoom performing. Receiving scale gesture and perform zoom.
 */
public class DualZoom extends SettingBase {
    private static final Tag TAG = new Tag(DualZoom.class.getSimpleName());
    private static final String KEY_DUAL_CAMERA_ID = "key_dual_camera_id";
    private static final String PHOTO_MODE = "com.mediatek.camera.common.mode.photo.PhotoMode";
    private static final String PANORAMA_MODE
            = "com.mediatek.camera.feature.mode.panorama.PanoramaMode";
    private static final String PIP_PHOTO =
            "com.mediatek.camera.feature.mode.pip.photo.PipPhotoMode";
    private static final String PIP_VIDEO =
            "com.mediatek.camera.feature.mode.pip.video.PipVideoMode";
    private static final String SLOW_MOTION =
            "com.mediatek.camera.feature.mode.slowmotion.SlowMotionMode";
    private static final String SELF_TIMER_STATE_KEY = "self_timer_key";
    private static final String STATE_START = "start";
    private static final String STATE_STOP = "stop";
    private boolean mIsSelfTimerStart = false;
    private int mLastZoomLevel;
    private static final String DEFAULT_CAMERA = "0";
    private String mDualCameraId;
    private List<Integer> mSupportedRatios;
    private String mCurrentMode = PHOTO_MODE;
    private ZoomGestureImpl mZoomGestureImpl = new ZoomGestureImpl();
    private float mPreviousSpan;
    private DualZoomViewCtrl mDualViewCtrl = new DualZoomViewCtrl();

    //Device Control
    private IDualZoomConfig mZoomConfig;
    private boolean mInScale = false;
    private DualZoomParameterConfig mParametersConfig;
    private DualZoomCaptureRequestConfig mCaptureRequestConfig;
    private ISettingChangeRequester mSettingChangeRequester;
    private String mOverrideValue = IDualZoomConfig.ZOOM_ON;
    private List<String> mSupportValues = new ArrayList<>();

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mDualViewCtrl.init(app);
        mZoomGestureImpl.init();
        initSettingValue();
        mStatusMonitor.registerValueChangedListener(SELF_TIMER_STATE_KEY, mStatusChangeListener);
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "uninit");
        mDataStore.setValue(KEY_DUAL_CAMERA_ID,
                DEFAULT_CAMERA, getStoreScope(), true);
        mStatusMonitor.unregisterValueChangedListener(SELF_TIMER_STATE_KEY, mStatusChangeListener);
    }

    @Override
    public void addViewEntry() {
        LogHelper.d(TAG, "[addViewEntry]");
        String curValue = getValue();
        if (IDualZoomConfig.ZOOM_OFF.equals(curValue)) {
            return;
        }
        if (isGestureSupported()) {
            mDualViewCtrl.setOnScrollListener(mOnScrollListener);
            mAppUi.registerGestureListener(mZoomGestureImpl, IApp.DEFAULT_PRIORITY);
        } else {
            mDualViewCtrl.setOnScrollListener(null);
        }
        mDualViewCtrl.setSeekBarClickListener(mSeekBarClickListener);
        mDualViewCtrl.config(isTeleSensor(), mSupportedRatios);
        mApp.registerOnOrientationChangeListener(mOrientationListener);
        if (isPipMode()) {
            mZoomConfig.onScaleStatus(false, true);
            mZoomConfig.onScalePerformed(0);
        }
    }

    @Override
    public void removeViewEntry() {
        LogHelper.d(TAG, "[removeViewEntry]");
        mDualViewCtrl.unInit();
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mDualViewCtrl.setOnScrollListener(null);
        if (isGestureSupported()) {
            mAppUi.unregisterGestureListener(mZoomGestureImpl);
        }
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
        return IDualZoomConfig.KEY_DUAL_ZOOM;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mParametersConfig == null) {
            mParametersConfig = new DualZoomParameterConfig(mSettingDeviceRequester);
            mParametersConfig.setZoomUpdateListener(mZoomLevelUpdateListener);
            mSettingChangeRequester = mParametersConfig;
            mZoomConfig = mParametersConfig;
        }
        return (DualZoomParameterConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mCaptureRequestConfig == null) {
            mCaptureRequestConfig = new DualZoomCaptureRequestConfig(mSettingDevice2Requester);
            mCaptureRequestConfig.setZoomUpdateListener(mZoomLevelUpdateListener);
            mSettingChangeRequester = mCaptureRequestConfig;
            mZoomConfig = mCaptureRequestConfig;
        }
        return (DualZoomCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        super.overrideValues(headerKey, currentValue, supportValues);
        LogHelper.i(TAG, "[overrideValues] headerKey = " + headerKey
                + ", currentValue = " + currentValue);
        String curValue = getValue();
        updateRestrictionValue(curValue);
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(TAG, "onModeOpened modeKey " + modeKey + ",modeType " + modeType);
        mCurrentMode = modeKey;
    }

    @Override
    public void onModeClosed(String modeKey) {
        LogHelper.d(TAG, "onModeClosed");
        super.onModeClosed(modeKey);
    }

    @Override
    public String getStoreScope() {
        return mDataStore.getGlobalScope();
    }

    private void updateRestrictionValue(String value) {
        mOverrideValue = value;
        if (!IDualZoomConfig.ZOOM_ON.equals(value)) {
            if (isGestureSupported()) {
                mAppUi.unregisterGestureListener(mZoomGestureImpl);
            }
            mDualViewCtrl.hideView();
        } else {
            if (isGestureSupported()) {
                mAppUi.registerGestureListener(mZoomGestureImpl, IApp.DEFAULT_PRIORITY);
            }
            if (isPipMode()) {
                mDualViewCtrl.showView(0);
            } else {
                mDualViewCtrl.showView(mLastZoomLevel);
            }
            mDualViewCtrl.resumeZoomView();
        }
    }

    private void initSettingValue() {
        mSupportValues.add(IDualZoomConfig.ZOOM_OFF);
        mSupportValues.add(IDualZoomConfig.ZOOM_ON);
        mSupportValues.add(IDualZoomConfig.ZOOM_LIMIT);
        setSupportedPlatformValues(mSupportValues);
        setSupportedEntryValues(mSupportValues);
        setEntryValues(mSupportValues);
        String value = mDataStore.getValue(getKey(), IDualZoomConfig.ZOOM_ON, getStoreScope());
        setValue(value);
    }

    private boolean isTeleSensor() {
        mDualCameraId = mDataStore.getValue(KEY_DUAL_CAMERA_ID,
                String.valueOf("0"), getStoreScope());
        return mDualCameraId.equals("2");
    }

    private boolean isPipMode() {
        return mCurrentMode.equals(PIP_PHOTO) || mCurrentMode.equals(PIP_VIDEO);
    }

    private IDualZoomConfig.OnZoomLevelUpdateListener mZoomLevelUpdateListener
            = new DualZoomParameterConfig.OnZoomLevelUpdateListener() {
        @Override
        public void onZoomLevelUpdate(int level) {
            mLastZoomLevel = level;
            if (!mIsSelfTimerStart) {
                mDualViewCtrl.showView(level);
            }
        }

        @Override
        public void onZoomRatioUpdate(float ratio) {
            mDualViewCtrl.showSlowMotionView(ratio);
        }

        @Override
        public void updateRatiosSupported(List<Integer> ratios) {
            mSupportedRatios = ratios;
        }

        @Override
        public boolean isZoomTele() {
            return isTeleSensor();
        }

        @Override
        public boolean isSingleMode() {
            return mCurrentMode.equals(PANORAMA_MODE);
        }

        public String onGetOverrideValue() {
            return mOverrideValue;
        }

        @Override
        public void updateSwitchRatioSupported(int switchRatio) {
            mDualViewCtrl.setSwitchRatio(switchRatio);
        }

        @Override
        public void updateMaxZoomSupported(float maxZoom) {
            mDualViewCtrl.setMaxZoom(maxZoom);
        }
    };

    private IApp.OnOrientationChangeListener mOrientationListener =
            new IApp.OnOrientationChangeListener() {
                @Override
                public void onOrientationChanged(int orientation) {
                    if (mDualViewCtrl != null) {
                        mDualViewCtrl.onOrientationChanged(orientation);
                    }
                }
            };

    private DualZoomViewCtrl.OnSeekBarClickListener mSeekBarClickListener
            = new DualZoomViewCtrl.OnSeekBarClickListener() {
        @Override
        public void onSingleTap() {
            if (mCurrentMode.equals(SLOW_MOTION)) {
                if (mZoomConfig != null && mZoomConfig.onScaleStatus(true, false)) {
                    requestZoom();
                    return;
                }
            }
            if (mCurrentMode.equals(PANORAMA_MODE) || mCurrentMode.equals(SLOW_MOTION)) {
                mDualCameraId = mDataStore.getValue(KEY_DUAL_CAMERA_ID,
                        "0", getStoreScope());
                String newCameraId = mDualCameraId
                        .equals(CameraUtil.getCamIdsByFacing(true).get(0)) ?
                        CameraUtil.getCamIdsByFacing(true).get(1)
                        : CameraUtil.getCamIdsByFacing(true).get(0);
                LogHelper.d(TAG, "[notifySwitch], switch camera to camera:" + newCameraId);
                boolean success = mApp.notifyCameraSelected(newCameraId);
                if (success) {
                    LogHelper.d(TAG, "[notifySwitch], switch camera success.");
                    mDualCameraId = newCameraId;
                    mDataStore.setValue(KEY_DUAL_CAMERA_ID,
                            newCameraId, getStoreScope(), true);
                }
                return;
            }
            if (mZoomConfig != null) {
                mZoomConfig.onScaleStatus(true, false);
                mZoomConfig.onScalePerformed(0);
                requestZoom();
                mDualViewCtrl.showScrollerPosition();
            }
        }
    };

    private ObservableScrollView.ScrollViewListener mOnScrollListener
            = new ObservableScrollView.ScrollViewListener() {
        @Override
        public void onScrollChanged(ObservableScrollView scrollView, int x, int y,
                                    int oldx, int oldy) {
            if (mInScale) {
                return;
            }
            LogHelper.d(TAG, "[onScrollChanged] x = " + x);
            double ratio;
            mZoomConfig.onScaleStatus(false, false);
            mZoomConfig.onScaleType(false);
            mZoomConfig.onScaleTypeName(IDualZoomConfig.TYPE_DRAG);
            int scrollerWidth = scrollView.getChildAt(0).getWidth();
            ratio = (double) x / (scrollerWidth - scrollView.getMeasuredWidth());
            if (mZoomConfig != null) {
                mZoomConfig.onScalePerformed(ratio);
                requestZoom();
            }
        }
    };

    /**
     * Class for zoom gesture listener.
     */
    private class ZoomGestureImpl implements OnGestureListener {
        private static final double MAX_DISTANCE_RATIO_WITH_SCREEN = 1.0;
        private int mScreenDistance;
        private double mLastDistance;


        /**
         * Init distance ratio.
         */
        public void init() {
            int high = mApp.getActivity().getWindowManager().getDefaultDisplay().getHeight();
            int width = mApp.getActivity().getWindowManager().getDefaultDisplay().getWidth();
            mScreenDistance = high >= width ? high : width;
            mScreenDistance *= MAX_DISTANCE_RATIO_WITH_SCREEN;
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
            //First, if it should not zoom, return false.
            String curValue = getValue();
            if (IDualZoomConfig.ZOOM_OFF.equals(curValue)) {
                return false;
            }
            double curDistance = calculateDistanceRatio(scaleGestureDetector);
            if (mZoomConfig != null && curDistance != mLastDistance) {
                mZoomConfig.onScalePerformed(curDistance);
                requestZoom();
                mLastDistance = curDistance;
            }
            return true;
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
            LogHelper.d(TAG, "[onScaleBegin]");
            if (mZoomConfig != null) {
                mInScale = true;
                mDualViewCtrl.closeZoomView();
                mDualViewCtrl.clearInvalidView(false);
                mZoomConfig.onScaleType(true);
                mZoomConfig.onScaleTypeName(IDualZoomConfig.TYPE_PINCH);
                mZoomConfig.onScaleStatus(false, false);
                mPreviousSpan = scaleGestureDetector.getCurrentSpan();
            }
            return true;
        }

        @Override
        public boolean onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
            LogHelper.d(TAG, "[onScaleEnd]");
            if (mZoomConfig != null) {
                mInScale = false;
                mDualViewCtrl.closeZoomView();
                mZoomConfig.onScaleStatus(false, false);
                mZoomConfig.onScaleType(false);
                mZoomConfig.onScaleTypeName(IDualZoomConfig.TYPE_OTHER);
                mPreviousSpan = 0;
            }
            return true;
        }

        @Override
        public boolean onLongPress(float x, float y) {
            return false;
        }

        private double calculateDistanceRatio(ScaleGestureDetector scaleGestureDetector) {
            float currentSpan = scaleGestureDetector.getCurrentSpan();
            double distanceRatio = (currentSpan - mPreviousSpan) / mScreenDistance;
            LogHelper.d(TAG, "[calculateDistanceRatio] distanceRatio = " + distanceRatio);
            return distanceRatio;
        }
    }

    private StatusMonitor.StatusChangeListener mStatusChangeListener = new StatusMonitor
            .StatusChangeListener() {

        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.d(TAG, "[onStatusChanged] + key " + key + "," +
                    "value " + value);
            switch (key) {
                case SELF_TIMER_STATE_KEY:
                    if (isGestureSupported()) {
                        mAppUi.registerGestureListener(mZoomGestureImpl, IApp.DEFAULT_PRIORITY);
                    }
                    if (STATE_START.equals(value)) {
                        mDualViewCtrl.hideView();
                        mIsSelfTimerStart = true;
                    } else if (STATE_STOP.equals(value)) {
                        mDualViewCtrl.showView(mLastZoomLevel);
                        mDualViewCtrl.resumeZoomView();
                        mIsSelfTimerStart = false;
                    }
                    break;
                default:
                    break;
            }
            LogHelper.d(TAG, "[onStatusChanged] -");
        }
    };

    private void requestZoom() {
        mSettingChangeRequester.sendSettingChangeRequest();
    }

    private boolean isGestureSupported() {
        return !mCurrentMode.equals(PANORAMA_MODE);
    }

    private boolean isSlowMotionMode() {
        return mCurrentMode.equals(SLOW_MOTION);
    }
}
