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

package com.mediatek.camera.feature.setting.zoom;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

import com.mediatek.camera.common.IAppUiListener.OnGestureListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.utils.CameraUtil;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class is for zoom performing. Receiving scale gesture and perform zoom.
 */
public class Zoom extends SettingBase {
    private Tag mTag;
    private ZoomGestureImpl mZoomGestureImpl = new ZoomGestureImpl();
    private ZoomViewCtrl mZoomViewCtrl = new ZoomViewCtrl();

    //Device Control
    private IZoomConfig mZoomConfig;
    private ZoomParameterConfig mParametersConfig;
    private ZoomCaptureRequestConfig mCaptureRequestConfig;
    private ISettingChangeRequester mSettingChangeRequester;
    private String mOverrideValue = IZoomConfig.ZOOM_ON;
    private List<String> mSupportValues = new ArrayList<>();
    private Handler mModeHandler;
    // [Add for bit true test] Receive KEYCODE_ZOOM_IN and KEYCODE_ZOOM_OUT @{
    private static final int RATIO_INDEX_NULL = 0;
    private static final int RATIO_INDEX_EMPTY = 1;
    private static final int RATIO_INDEX_X10 = 2;
    private static final int RATIO_INDEX_X20 = 3;
    private static final int RATIO_INDEX_X30 = 4;
    private static final int RATIO_INDEX_X40 = 5;
    private static final String[] ZOOM_IN_TARGET_RATIO =
            new String[]{"x2.", "x2.", "x2.", "x3.", "x4.", "x4."};
    private static final String[] ZOOM_OUT_Target_RATIO =
            new String[]{"x1.0", "x1.0", "x1.0", "x1.0", "x2.0", "x3.0"};
    private static final float DISTANCE_RATIO_STEP = 0.01f;
    private static final int MSG_ZOOM_IN = 0;
    private static final int MSG_ZOOM_OUT = 1;
    private static final int MSG_DELAY = 50;
    private ZoomKeyEventListener mZoomKeyEventListener = new ZoomKeyEventListener();
    private MainHandler mMainHandler;
    private float mLastDistanceRatio = 0.0f;
    private String mCurrentRatioMsg;
    // @}

    /**
     * Initialize setting. This will be called when do open camera.
     *
     * @param app the instance of IApp.
     * @param cameraContext the CameraContext.
     * @param settingController the SettingController.
     */
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mTag = new Tag(Zoom.class.getSimpleName() + "-" + settingController.getCameraId());
        mModeHandler = new Handler(Looper.myLooper());
        mZoomGestureImpl.init();
        mZoomViewCtrl.init(app);
        initSettingValue();
        mApp.registerOnOrientationChangeListener(mOrientationListener);
        mAppUi.registerGestureListener(mZoomGestureImpl, IApp.DEFAULT_PRIORITY);
        LogHelper.d(mTag, "[init] zoom: " + this + ", Gesture: " + mZoomGestureImpl);
        // [Add for bit true test] Receive KEYCODE_ZOOM_IN and KEYCODE_ZOOM_OUT @{
        mMainHandler = new MainHandler(mActivity.getMainLooper());
        mApp.registerKeyEventListener(mZoomKeyEventListener, IApp.DEFAULT_PRIORITY);
        // @}
    }

    /**
     * Un-initialize setting, this will be called before close camera.
     */
    @Override
    public void unInit() {
        mZoomViewCtrl.unInit();
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mAppUi.unregisterGestureListener(mZoomGestureImpl);
        LogHelper.d(mTag, "[unInit] zoom: " + this  + ", Gesture: " + mZoomGestureImpl);
        // [Add for bit true test] Receive KEYCODE_ZOOM_IN and KEYCODE_ZOOM_OUT @{
        mApp.unRegisterKeyEventListener(mZoomKeyEventListener);
        // @}
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
        return IZoomConfig.KEY_CAMERA_ZOOM;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mParametersConfig == null) {
            mParametersConfig = new ZoomParameterConfig(mSettingDeviceRequester);
            mParametersConfig.setZoomUpdateListener(mZoomLevelUpdateListener);
            mSettingChangeRequester = mParametersConfig;
            mZoomConfig = mParametersConfig;
            LogHelper.d(mTag, "[getParametersConfigure]mZoomConfig: " + mSettingChangeRequester);
        }
        return (ZoomParameterConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mCaptureRequestConfig == null) {
            mCaptureRequestConfig = new ZoomCaptureRequestConfig(mSettingDevice2Requester);
            mCaptureRequestConfig.setZoomUpdateListener(mZoomLevelUpdateListener);
            mSettingChangeRequester = mCaptureRequestConfig;
            mZoomConfig = mCaptureRequestConfig;
            LogHelper.d(mTag, "[getCaptureRequestConfigure]mZoomConfig: "
                    + mSettingChangeRequester);
        }
        return (ZoomCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        super.overrideValues(headerKey, currentValue, supportValues);
        LogHelper.i(mTag, "[overrideValues] headerKey = " + headerKey
        + ", currentValue = " + currentValue);
        String curValue = getValue();
        updateRestrictionValue(curValue);
    }

    @Override
    public PreviewStateCallback getPreviewStateCallback() {
        return null;
    }

    private void updateRestrictionValue(String value) {
        mOverrideValue = value;
        if (IZoomConfig.ZOOM_OFF.equals(value)) {
            mZoomViewCtrl.hideView();
        }
    }

    private void initSettingValue() {
        mSupportValues.add(IZoomConfig.ZOOM_OFF);
        mSupportValues.add(IZoomConfig.ZOOM_ON);
        setSupportedPlatformValues(mSupportValues);
        setSupportedEntryValues(mSupportValues);
        setEntryValues(mSupportValues);
        String value = mDataStore.getValue(getKey(), IZoomConfig.ZOOM_ON, getStoreScope());
        setValue(value);
    }

    private IZoomConfig.OnZoomLevelUpdateListener mZoomLevelUpdateListener
            = new ZoomParameterConfig.OnZoomLevelUpdateListener() {
        @Override
        public void onZoomLevelUpdate(String ratio) {
            // [Add for bit true test] Receive KEYCODE_ZOOM_IN and KEYCODE_ZOOM_OUT @{
            mCurrentRatioMsg = ratio;
            // @}
            mZoomViewCtrl.showView(ratio);
        }

        public String onGetOverrideValue() {
            return mOverrideValue;
        }
    };

    private IApp.OnOrientationChangeListener mOrientationListener =
            new IApp.OnOrientationChangeListener() {
                @Override
                public void onOrientationChanged(int orientation) {
                    if (mZoomViewCtrl != null) {
                        mZoomViewCtrl.onOrientationChanged(orientation);
                    }
                }
            };

    /**
     * Class for zoom gesture listener.
     */
    private class ZoomGestureImpl implements OnGestureListener {
        private static final double MAX_DISTANCE_RATIO_WITH_SCREEN = 1.0 / 5.0;
        private float mPreviousSpan;
        private int mScreenDistance;
        private double mLastDistanceRatio;

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
            if (IZoomConfig.ZOOM_OFF.equals(curValue)) {
                return false;
            }
            if (mZoomConfig != null) {
                double distanceRatio = calculateDistanceRatio(scaleGestureDetector);
                mZoomConfig.onScalePerformed(distanceRatio);
                if (Math.abs(distanceRatio - mLastDistanceRatio) > 0.08) {
                    requestZoom();
                    mLastDistanceRatio = distanceRatio;
                }
            }
            return true;
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
            String curValue = getValue();
            if (IZoomConfig.ZOOM_OFF.equals(curValue)) {
                return false;
            }
            LogHelper.d(mTag, "[onScaleBegin], Gesture: " + this + ", mZoomConfig: " + mZoomConfig);
            if (mZoomConfig != null) {
                mZoomViewCtrl.clearInvalidView();
                mZoomConfig.onScaleStatus(true);
                mPreviousSpan = scaleGestureDetector.getCurrentSpan();
                mLastDistanceRatio = 0;
            }
            return true;
        }

        @Override
        public boolean onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
            String curValue = getValue();
            if (IZoomConfig.ZOOM_OFF.equals(curValue)) {
                return false;
            }
            LogHelper.d(mTag, "[onScaleEnd]");
            if (mZoomConfig != null) {
                mZoomViewCtrl.resetView();
                mZoomConfig.onScaleStatus(false);
                mPreviousSpan = 0;
                mLastDistanceRatio = 0;
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
            LogHelper.d(mTag, "[calculateDistanceRatio] distanceRatio = " + distanceRatio);
            return distanceRatio;
        }
    }

    private void requestZoom() {
        if (mModeHandler == null) {
            return;
        }
        mModeHandler.post(new Runnable() {
            @Override
            public void run() {
                mSettingChangeRequester.sendSettingChangeRequest();
            }
        });
    }

    // [Add for bit true test] Receive keycode and do zoom in/out @{
    private class ZoomKeyEventListener implements IApp.KeyEventListener {
        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {
            if ((keyCode != CameraUtil.KEYCODE_ZOOM_IN
                    && keyCode != CameraUtil.KEYCODE_ZOOM_OUT)
                    || !CameraUtil.isSpecialKeyCodeEnabled()) {
                return false;
            }
            if (IZoomConfig.ZOOM_OFF.equals(getValue())) {
                return false;
            }
            if (mZoomConfig != null) {
                mZoomViewCtrl.clearInvalidView();
                mZoomConfig.onScaleStatus(true);
            }
            return true;
        }

        @Override
        public boolean onKeyUp(int keyCode, KeyEvent event) {
            if (!CameraUtil.isSpecialKeyCodeEnabled()) {
                return false;
            }
            if (keyCode != CameraUtil.KEYCODE_ZOOM_IN
                    && keyCode != CameraUtil.KEYCODE_ZOOM_OUT) {
                return false;
            }
            if (IZoomConfig.ZOOM_OFF.equals(getValue())) {
                LogHelper.w(mTag, "onKeyUp keyCode zoom is OFF");
                return false;
            }
            if (keyCode == CameraUtil.KEYCODE_ZOOM_IN) {
                mMainHandler.obtainMessage(MSG_ZOOM_IN, getTargetRatioMsg(true)).sendToTarget();
            } else if (keyCode == CameraUtil.KEYCODE_ZOOM_OUT) {
                mMainHandler.obtainMessage(MSG_ZOOM_OUT, getTargetRatioMsg(false)).sendToTarget();
            }
            return true;
        }
    }

    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ZOOM_IN:
                    if (mZoomConfig == null) {
                        return;
                    }
                    String targetZoomInRatioMsg = (String) msg.obj;
                    if (mCurrentRatioMsg != null
                            && mCurrentRatioMsg.startsWith(targetZoomInRatioMsg)) {
                        mLastDistanceRatio = 0.0f;
                        mZoomViewCtrl.resetView();
                        mZoomConfig.onScaleStatus(false);
                        mMainHandler.removeMessages(MSG_ZOOM_IN);
                        LogHelper.d(mTag,"[handleMessage] zoom in, mCurrentRatioMsg = "
                                + mCurrentRatioMsg + ", done");
                    } else {
                        mLastDistanceRatio += DISTANCE_RATIO_STEP;
                        mZoomConfig.onScalePerformed(mLastDistanceRatio);
                        requestZoom();
                        Message message = mMainHandler.obtainMessage(MSG_ZOOM_IN,
                                targetZoomInRatioMsg);
                        mMainHandler.sendMessageDelayed(message, MSG_DELAY);
                    }
                    break;
                case MSG_ZOOM_OUT:
                    if (mZoomConfig == null) {
                        return;
                    }
                    String targetZoomOutRatioMsg = (String) msg.obj;
                    if (targetZoomOutRatioMsg.equals(mCurrentRatioMsg)) {
                        mLastDistanceRatio = 0.0f;
                        mZoomViewCtrl.resetView();
                        mZoomConfig.onScaleStatus(false);
                        mMainHandler.removeMessages(MSG_ZOOM_OUT);
                        LogHelper.d(mTag, "[handleMessage] zoom out, mCurrentRatioMsg = "
                                + mCurrentRatioMsg + ", done");
                    } else {
                        mLastDistanceRatio -= DISTANCE_RATIO_STEP;
                        mZoomConfig.onScalePerformed(mLastDistanceRatio);
                        requestZoom();
                        Message message = mMainHandler.obtainMessage(MSG_ZOOM_OUT,
                                targetZoomOutRatioMsg);
                        mMainHandler.sendMessageDelayed(message, MSG_DELAY);
                    }
                    break;
            }
        }
    }

    private String getTargetRatioMsg(boolean isZoomIn) {
        String[] targetRatio = isZoomIn ? ZOOM_IN_TARGET_RATIO : ZOOM_OUT_Target_RATIO;
        String result;
        if (mCurrentRatioMsg == null) {
            result = targetRatio[RATIO_INDEX_NULL];
        } else if (mCurrentRatioMsg.equals("")) {
            result = targetRatio[RATIO_INDEX_EMPTY];
        } else if (mCurrentRatioMsg.startsWith("x1.")) {
            result = targetRatio[RATIO_INDEX_X10];
        } else if (mCurrentRatioMsg.startsWith("x2.")) {
            result = targetRatio[RATIO_INDEX_X20];
        } else if (mCurrentRatioMsg.startsWith("x3.")) {
            result = targetRatio[RATIO_INDEX_X30];
        } else if (mCurrentRatioMsg.startsWith("x4.")) {
            result = targetRatio[RATIO_INDEX_X40];
        } else {
            result = "x1.0";
        }
        LogHelper.d(mTag, "[getTargetRatioMsg] isZoomIn = " + isZoomIn
                + ", mCurrentRatioMsg = " + mCurrentRatioMsg + ", return " + result);
        return result;
    }
    // @}
}
