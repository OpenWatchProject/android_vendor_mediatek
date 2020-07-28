/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.v2.control.focus;

import android.app.Activity;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.Face;
import android.hardware.camera2.params.MeteringRectangle;
import android.os.Handler;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.android.camera.R;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.control.ControlHelper;
import com.mediatek.camera.v2.control.IControl.AutoFocusState;
import com.mediatek.camera.v2.control.IControl.FocusStateListener;
import com.mediatek.camera.v2.control.IControl.IAaaListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.services.ISoundPlayback;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import java.util.ArrayList;
import java.util.Map;

public class AutoFocus implements IFocus, ISettingChangedListener {
    private final static Tag TAG = new Tag(AutoFocus.class.getSimpleName());
    private final ISettingServant  mSettingServant;
    private ArrayList<String>      mCaredSettingChangedKeys = new ArrayList<String>();
    private final ISoundPlayback   mSoundPlayer;
    private IAaaListener           mAaaListener;
    private final Activity         mActivity;
    private final AppController    mAppController;
    private final Handler          mMainHandler;
    private int                    mAfMode = CameraMetadata.CONTROL_AF_MODE_CONTINUOUS_PICTURE;
    private RectF                  mPreviewArea;

    private static final int       RESET_TOUCH_FOCUS_DELAY_MILLIS = 1000;
    /** Worst case persistence of TTF target UI. */
    private static final int       FOCUS_UI_TIMEOUT_MILLIS = 3000;
    private ViewGroup              mParentViewGroup;
    private ViewGroup              mAutoFocusParentViewGroup;
    private AutoFocusRotateLayout  mAutoFocusRotateLayout;
    private View                   mFocusIndicator;

    private boolean                mTapToFocusWaitForActiveScan = false;
    private boolean                mIsAfTriggerRequestSubmitted = false;
    // make sure focus box will not be cleared by face detection during auto focusing.
    private boolean                mAfTriggerEnabled = false;
    private MeteringRectangle[]    mAFRegions = ControlHelper.ZERO_WEIGHT_3A_REGION;
    private MeteringRectangle[]    mAERegions = ControlHelper.ZERO_WEIGHT_3A_REGION;
    // Last frame for which CONTROL_AF_STATE was received.
    private long                   mLastControlAfStateFrameNumber = -1;
    private int                    mLastResultAFState = CaptureResult.CONTROL_AF_STATE_INACTIVE;
    // use to keep face detection value to reset face detection after finishing touch focus.
    private String                 mFaceDetectionValue = null;
    private RequestType mLastRequestType = RequestType.PREVIEW;
    private ArrayList<FocusStateListener> mFocusStateListener = new ArrayList<FocusStateListener>();

    public AutoFocus(ISettingServant settingServant,
            ISoundPlayback soundPlayer,
            AppController  app,
            IAaaListener aaaListener,
            ViewGroup parentViewGroup) {
        mSettingServant  = settingServant;
        mAaaListener  = aaaListener;
        mSoundPlayer     = soundPlayer;
        mAppController   = app;
        mActivity        = app.getActivity();
        mMainHandler     = new Handler(mActivity.getMainLooper());
        mParentViewGroup = parentViewGroup;
    }

    @Override
    public void open(Activity activity, ViewGroup parentView,
            boolean isCaptureIntent) {
        intializeFocusUi(mParentViewGroup);
        updateCaredSettingChangedKeys(SettingKeys.KEY_CAMERA_ID);
        mSettingServant.registerSettingChangedListener(this, mCaredSettingChangedKeys,
                ISettingChangedListener.MIDDLE_PRIORITY);
    }

    @Override
    public void resume() {
        LogHelper.i(TAG, "[resume]+");
        mIsAfTriggerRequestSubmitted = false;
        mTapToFocusWaitForActiveScan = false;
        mLastControlAfStateFrameNumber = -1;
        mAfMode = CameraMetadata.CONTROL_AF_MODE_CONTINUOUS_PICTURE;
        mLastResultAFState = CaptureResult.CONTROL_AF_STATE_INACTIVE;
        mLastRequestType = RequestType.PREVIEW;
        resetTouchFocus();
        LogHelper.i(TAG, "[resume]-");
    }

    @Override
    public void pause() {

    }

    @Override
    public void close() {
        LogHelper.i(TAG, "[close]+");
        unIntializeFocusUi(mParentViewGroup);
        mSettingServant.unRegisterSettingChangedListener(this);
        LogHelper.i(TAG, "[close]-");
    }

    @Override
    public void onOrientationCompensationChanged(int orientationCompensation) {
        mAutoFocusRotateLayout.setOrientation(orientationCompensation, true);
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        LogHelper.i(TAG, "onPreviewAreaChanged width = " + previewArea.width() +
                " height = " + previewArea.height());
        mPreviewArea = previewArea;
        // Set the length of focus indicator according to preview frame size.
        int len = Math.min((int) mPreviewArea.width(), (int) mPreviewArea.height()) / 4;
        ViewGroup.LayoutParams layout = mFocusIndicator.getLayoutParams();
        layout.width = len;
        layout.height = len;
        mFocusIndicator.requestLayout();
    }

    @Override
    public void onSingleTapUp(float x, float y) {
        LogHelper.i(TAG, "onSingleTapUp x = " + x + " y = " + y + " preview area = " + mPreviewArea
                + " mIsAfTriggered : " + mIsAfTriggerRequestSubmitted);
        if (mPreviewArea == null) {
            return;
        }
        String cameraId = mSettingServant.getCameraId();
        CameraCharacteristics characteristics = Utils.getCameraCharacteristics(mActivity, cameraId);
        if (!hasFocuser(characteristics)) {
            return;
        }
        if (!mAfTriggerEnabled) {
            mFaceDetectionValue = mSettingServant.getSettingValue(
                    SettingKeys.KEY_CAMERA_FACE_DETECT);
        }
        mAfTriggerEnabled = true;
        // Cancel any scheduled auto focus target UI actions.
        mMainHandler.removeCallbacks(mReturnToContinuousAFRunnable);
        mAppController.getCameraAppUi().setAllCommonViewEnable(false);
        resetTouchFocus();
        LogHelper.i(TAG, "[onSingleTapUp] mIsAfTriggerRequestSubmitted="
                + mIsAfTriggerRequestSubmitted);
        if (mIsAfTriggerRequestSubmitted) {
            sendAutoFocusCancelCaptureRequest();
        }
        mTapToFocusWaitForActiveScan = true;
        mLastResultAFState = CaptureResult.CONTROL_AF_STATE_INACTIVE;
        // stop face detection
        mSettingServant.doSettingChange(SettingKeys.KEY_CAMERA_FACE_DETECT, "off", false);
        // Show UI immediately even though scan has not started yet.
        float minEdge = Math.min(mPreviewArea.width(), mPreviewArea.height());

        // Use margin to set the focus indicator to the touched area.
        RelativeLayout.LayoutParams p = (RelativeLayout.LayoutParams) mAutoFocusRotateLayout
                .getLayoutParams();
        int left = 0;
        int top = 0;
        int focusWidth = mAutoFocusRotateLayout.getWidth();
        int focusHeight = mAutoFocusRotateLayout.getHeight();
        LogHelper.i(TAG, "focus area width = " + focusWidth + " height = " + focusHeight);
        left = clamp((int) x - focusWidth / 2, 0, (int) mPreviewArea.width() - focusWidth);
        top =  clamp((int) y - focusHeight / 2, 0, (int) mPreviewArea.height() - focusHeight);
        left += mPreviewArea.left;
        top  += mPreviewArea.top;
        LogHelper.i(TAG, "left = " + left + " top = " + top +
                " focus area width = " + focusWidth + " height = " + focusHeight);
        if (p.getLayoutDirection() != View.LAYOUT_DIRECTION_RTL) {
            p.setMargins(left, top, 0, 0);
        } else {
            // since in RTL language, framework will use marginRight as standard.
            int right = (int) mPreviewArea.width() - (left + focusWidth);
            p.setMargins(0, top, right, 0);
        }
        // Disable "center" rule because we no longer want to put it in the center.
        int[] rules = p.getRules();
        rules[RelativeLayout.CENTER_IN_PARENT] = 0;
        mMainHandler.removeCallbacks(mClearAutoFocusUIRunnable);
        mAutoFocusRotateLayout.requestLayout();
        mAutoFocusRotateLayout.onFocusStarted();

        // Normalize coordinates to [0,1] .
        float points[] = new float[2];
        points[0] = (x - mPreviewArea.left) / mPreviewArea.width();
        points[1] = (y - mPreviewArea.top) / mPreviewArea.height();
        // Rotate coordinates to portrait orientation .
        Matrix rotationMatrix = new Matrix();
        rotationMatrix.setRotate(Utils.getDisplayRotation(mActivity), 0.5f, 0.5f);
        rotationMatrix.mapPoints(points);

        LogHelper.i(TAG, "onSingleTapUp points[0]:" + points[0] + " points[1]:" +  points[1]);
        int sensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        Rect cropRegion = Utils.cropRegionForZoom(mActivity, cameraId, 1f);
        mAERegions = ControlHelper.aeRegionsForNormalizedCoord(points[0], points[1],
                cropRegion, sensorOrientation);
        mAFRegions = ControlHelper.afRegionsForNormalizedCoord(points[0], points[1],
                cropRegion, sensorOrientation);
        sendAutoFocusTriggerCaptureRequest();
    }

    @Override
    public void configuringSessionRequest(RequestType requestType,
            Builder requestBuilder, CaptureType captureType, boolean bottomCamera) {
        LogHelper.i(TAG, "[configuringSessionRequests] + ");
        addBaselineCaptureKeysToRequest(requestBuilder);
        onRequestTypeChanged(requestType);
        if (RequestType.RECORDING == requestType || RequestType.VIDEO_SNAP_SHOT == requestType) {
            requestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CameraMetadata.CONTROL_AF_MODE_AUTO);
        } else {
            requestBuilder.set(CaptureRequest.CONTROL_AF_MODE, mAfMode);
        }
        if ((requestType == RequestType.PREVIEW || requestType == RequestType.RECORDING) &&
                captureType == CaptureType.CAPTURE &&
                mAfMode == CameraMetadata.CONTROL_AF_MODE_AUTO) {
            if (mIsAfTriggerRequestSubmitted) {
                requestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                        CaptureRequest.CONTROL_AF_TRIGGER_CANCEL);
            } else {
                requestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                        CaptureRequest.CONTROL_AF_TRIGGER_START);
            }
        }
        LogHelper.i(TAG, "[configuringSessionRequests]- requestType = " + requestType +
                " AFMode:" + mAfMode +
                " captureType:" + captureType +
                " mIsAfTriggerRequestSubmitted:" + mIsAfTriggerRequestSubmitted);
    }

    @Override
    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp,
            long frameNumber) {

    }

    @Override
    public void onPreviewCaptureProgressed(CaptureRequest request,
            CaptureResult partialResult) {
        // comment this line to avoid get faces is null which will cause focus box and
        // face box is shown at the same time.
        //autofocusStateChangeDispatcher(partialResult);
    }

    @Override
    public void onPreviewCaptureCompleted(CaptureRequest request,
            TotalCaptureResult result) {
        autofocusStateChangeDispatcher(result);
    }

    @Override
    public void clearFocusUi() {
        mIsAfTriggerRequestSubmitted = false;
        mTapToFocusWaitForActiveScan = false;
        mLastControlAfStateFrameNumber = -1;
        mLastResultAFState = CaptureResult.CONTROL_AF_STATE_INACTIVE;
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAutoFocusRotateLayout.clear();
            }
        });
    }

    @Override
    public void setFocusStateListener(FocusStateListener listener) {
        if (!mFocusStateListener.contains(listener)) {
            mFocusStateListener.add(listener);
        }
    }

    @Override
    public void clearFocusStateListener(FocusStateListener listener) {
        if (mFocusStateListener.contains(listener)) {
            mFocusStateListener.remove(listener);
        }
    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        String cameraId = result.get(SettingKeys.KEY_CAMERA_ID);
        if (cameraId != null) {
            LogHelper.i(TAG, "camera id changed new:" + cameraId);
            mIsAfTriggerRequestSubmitted = false;
            mTapToFocusWaitForActiveScan = false;
            mLastControlAfStateFrameNumber = -1;
            mLastResultAFState = CaptureResult.CONTROL_AF_STATE_INACTIVE;
            mActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    resetTouchFocus();
                }
            });
        }
    }

    private void intializeFocusUi(ViewGroup parentViewGroup) {
        mAutoFocusParentViewGroup = (ViewGroup) mActivity.getLayoutInflater().inflate(
                R.layout.focus_indicator_v2, parentViewGroup, true);
        mAutoFocusRotateLayout = (AutoFocusRotateLayout) parentViewGroup.findViewById(
                R.id.focus_indicator_rotate_layout);
        mFocusIndicator = mAutoFocusRotateLayout.findViewById(R.id.focus_indicator);
    }

    private void unIntializeFocusUi(ViewGroup parentViewGroup) {
        if (mAutoFocusParentViewGroup != null) {
            mAutoFocusParentViewGroup.removeAllViewsInLayout();
            mParentViewGroup.removeView(mAutoFocusParentViewGroup);
        }
    }

    private int clamp(int x, int min, int max) {
        if (x > max)
            return max;
        if (x < min)
            return min;
        return x;
    }

    /**
     * Request preview capture stream with auto focus trigger cycle.
     */
    private void sendAutoFocusTriggerCaptureRequest() {
        mAfMode = CameraMetadata.CONTROL_AF_MODE_AUTO;
        RequestType requiredRequestType = mAaaListener.getRepeatingRequestType();
        // make a single request to trigger auto focus
        mAaaListener.requestChangeCaptureRequets(true, requiredRequestType, CaptureType.CAPTURE);
        // change focus mode to auto, tracking focus state
        mAaaListener.requestChangeCaptureRequets(true, requiredRequestType,
                CaptureType.REPEATING_REQUEST);
        mIsAfTriggerRequestSubmitted = true;
    }

    /**
     * Request preview capture stream for canceling auto focus .
     */
    private void sendAutoFocusCancelCaptureRequest() {
        mAfMode = CameraMetadata.CONTROL_AF_MODE_AUTO;
        // make a single request to trigger auto focus
        mAaaListener.requestChangeCaptureRequets(true, mAaaListener.getRepeatingRequestType(),
                CaptureType.CAPTURE);
        mIsAfTriggerRequestSubmitted = false;
    }

    /**
     * Adds current regions to CaptureRequest and base AF mode + AF_TRIGGER_IDLE.
     *
     * @param builder Build for the CaptureRequest
     */
    private void addBaselineCaptureKeysToRequest(CaptureRequest.Builder builder) {
        builder.set(CaptureRequest.CONTROL_AF_REGIONS, mAFRegions);
        builder.set(CaptureRequest.CONTROL_AE_REGIONS, mAERegions);
        builder.set(CaptureRequest.CONTROL_AF_MODE, mAfMode);
        builder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_IDLE);
    }

    // Runnable that returns to CONTROL_AF_MODE = AF_CONTINUOUS_PICTURE.
    private final Runnable mReturnToContinuousAFRunnable = new Runnable() {
        @Override
        public void run() {
            mIsAfTriggerRequestSubmitted = false;
            resetTouchFocus();
            mAFRegions = ControlHelper.ZERO_WEIGHT_3A_REGION;
            mAfMode = CameraMetadata.CONTROL_AF_MODE_CONTINUOUS_PICTURE;
            mAaaListener.requestChangeCaptureRequets(true, mAaaListener.getRepeatingRequestType(),
                    CaptureType.REPEATING_REQUEST);
        }
    };

    private final Runnable mClearAutoFocusUIRunnable = new Runnable() {
        @Override
        public void run() {
            mAutoFocusRotateLayout.clear();
        }
    };

    /**
     * This method takes appropriate action if camera2 AF state changes.
     * <ol>
     * <li>Reports changes in camera2 AF state to OneCamera.FocusStateListener.</li>
     * <li>Take picture after AF scan if mTakePictureWhenLensIsStopped true.</li>
     * </ol>
     */
    private void autofocusStateChangeDispatcher(CaptureResult result) {
        long currentFrameNumber = result.getFrameNumber();
        if (currentFrameNumber < mLastControlAfStateFrameNumber ||
                result.get(CaptureResult.CONTROL_AF_STATE) == null) {
            LogHelper.i(TAG, "frame number, last:current " + mLastControlAfStateFrameNumber +
                    ":" + currentFrameNumber + " afState:" +
                    result.get(CaptureResult.CONTROL_AF_STATE));
            return;
        }
        Face[] faces = result.get(CaptureResult.STATISTICS_FACES);
        if (faces != null && faces.length > 0) {
            if (!mAfTriggerEnabled) {
                mMainHandler.post(mClearAutoFocusUIRunnable);
            }
            return;
        }
        mLastControlAfStateFrameNumber = result.getFrameNumber();
        int resultAFState = result.get(CaptureResult.CONTROL_AF_STATE);
        if (mLastResultAFState != resultAFState) {
            onFocusStatusUpdate(resultAFState);
            notifyFocusStateChanged(resultAFState);
        }
        mLastResultAFState = resultAFState;
    }

    private void resetTouchFocus() {
        LogHelper.i(TAG, "resetTouchFocus");
        mAutoFocusRotateLayout.clear();
        // Put focus indicator to the center.
        RelativeLayout.LayoutParams p = (RelativeLayout.LayoutParams) mAutoFocusRotateLayout
                .getLayoutParams();
        int[] rules = p.getRules();
        p.setMargins(0, 0, 0, 0);
        rules[RelativeLayout.CENTER_IN_PARENT] = RelativeLayout.TRUE;
        mAutoFocusRotateLayout.requestLayout();
    }

    private void resumeContinuousAF() {
        if (mAfTriggerEnabled) {
            // recover face detection value.
            mSettingServant.doSettingChange(SettingKeys.KEY_CAMERA_FACE_DETECT,
                    mFaceDetectionValue, false);
        }
        mAfTriggerEnabled = false;
        mTapToFocusWaitForActiveScan = true;
        mAppController.getCameraAppUi().setAllCommonViewEnable(true);
        mMainHandler.removeCallbacks(mReturnToContinuousAFRunnable);
        mMainHandler.postDelayed(mReturnToContinuousAFRunnable, RESET_TOUCH_FOCUS_DELAY_MILLIS);
    }

    private void onFocusStatusUpdate(final int resultAFState) {
        LogHelper.i(TAG, "onFocusStatusUpdate resultAFState: " + resultAFState +
                " cameraId:" + mSettingServant.getCameraId());
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (resultAFState) {
                case CaptureResult.CONTROL_AF_STATE_PASSIVE_SCAN:/**1**/
                    if (mAaaListener.getRepeatingRequestType() != RequestType.RECORDING) {
                        mAutoFocusRotateLayout.onFocusStarted();
                    }
                    if (!ControlHelper.ZERO_WEIGHT_3A_REGION.equals(mAERegions)) {
                        mAERegions = ControlHelper.ZERO_WEIGHT_3A_REGION;
                        mAaaListener.requestChangeCaptureRequets(false,
                                mAaaListener.getRepeatingRequestType(),
                                CaptureType.REPEATING_REQUEST);
                    }
                    break;
                case CaptureResult.CONTROL_AF_STATE_PASSIVE_FOCUSED:/**2**/
                    if (mAaaListener.getRepeatingRequestType() != RequestType.RECORDING) {
                        if (!mAfTriggerEnabled) {
                            mAutoFocusRotateLayout.setPassiveFocusSuccess(true);
                        }
                    }
                    break;
                case CaptureResult.CONTROL_AF_STATE_PASSIVE_UNFOCUSED:/**6**/
                    if (mAaaListener.getRepeatingRequestType() != RequestType.RECORDING) {
                        if (!mAfTriggerEnabled) {
                            mAutoFocusRotateLayout.setPassiveFocusSuccess(false);
                        }
                    }
                    break;
                case CaptureResult.CONTROL_AF_STATE_ACTIVE_SCAN:/**3**/
                    mTapToFocusWaitForActiveScan = false;
                    break;
                case CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED:/**4**/
                    if (!mTapToFocusWaitForActiveScan) {
                        mAutoFocusRotateLayout.onFocusSucceeded();
                        if (mAaaListener.getRepeatingRequestType() != RequestType.RECORDING) {
                            mSoundPlayer.play(ISoundPlayback.FOCUS_COMPLETE);
                        }
                        resumeContinuousAF();
                    }
                    break;
                case CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:/**5**/
                    if (!mTapToFocusWaitForActiveScan) {
                        mAutoFocusRotateLayout.onFocusFailed();
                        resumeContinuousAF();
                    }
                    break;
                default:
                    break;
                }
            }
        });
    }

    private boolean hasFocuser(CameraCharacteristics characteristics) {
        Float minFocusDistance = characteristics.get(
                CameraCharacteristics.LENS_INFO_MINIMUM_FOCUS_DISTANCE);
        if (minFocusDistance != null && minFocusDistance > 0) {
            return true;
        }

        // Check available AF modes
        int[] availableAfModes = characteristics.get(
                CameraCharacteristics.CONTROL_AF_AVAILABLE_MODES);

        if (availableAfModes == null) {
            return false;
        }

        // Assume that if we have an AF mode which doesn't ignore AF trigger, we have a focuser
        boolean hasFocuser = false;
        loop: for (int mode : availableAfModes) {
            switch (mode) {
                case CameraMetadata.CONTROL_AF_MODE_AUTO:
                case CameraMetadata.CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                case CameraMetadata.CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                case CameraMetadata.CONTROL_AF_MODE_MACRO:
                    hasFocuser = true;
                    break loop;
            }
        }
        return hasFocuser;
    }

    private void updateCaredSettingChangedKeys(String key) {
        if (key != null && !mCaredSettingChangedKeys.contains(key)) {
            mCaredSettingChangedKeys.add(key);
        }
    }

    /**
     * Focus UI shoudld be reset when request type change between
     * {@link com.mediatek.camera.v2.module.ModuleListener.RequestType #PREVIEW} and
     * {@link com.mediatek.camera.v2.module.ModuleListener.RequestType #RECORDING}.
     *
     * @param type The current request type.
     */
    private void onRequestTypeChanged(final RequestType type) {
        if (RequestType.RECORDING == type || RequestType.PREVIEW == type) {
            if (mLastRequestType != type) {
                LogHelper.d(TAG, "onRequestTypeChanged mLastRequestType = " + mLastRequestType
                        + " ,the new type = " + type);
                mActivity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        resetTouchFocus();
                        mLastRequestType = type;
                    }
                });
            }
        }
    }

    private void notifyFocusStateChanged(int afState) {
        AutoFocusState state = AutoFocusState.INACTIVE;
        switch (afState) {
            case CaptureResult.CONTROL_AF_STATE_PASSIVE_SCAN:/**1**/
                state = AutoFocusState.PASSIVE_SCAN;
                break;
            case CaptureResult.CONTROL_AF_STATE_PASSIVE_FOCUSED:/**2**/
                state = AutoFocusState.PASSIVE_FOCUSED;
                break;
            case CaptureResult.CONTROL_AF_STATE_PASSIVE_UNFOCUSED:/**6**/
                state = AutoFocusState.PASSIVE_UNFOCUSED;
                break;
            case CaptureResult.CONTROL_AF_STATE_ACTIVE_SCAN:/**3**/
                state = AutoFocusState.ACTIVE_SCAN;
                break;
            case CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED:/**4**/
                state = AutoFocusState.ACTIVE_FOCUSED;
                break;
            case CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:/**5**/
                state = AutoFocusState.ACTIVE_UNFOCUSED;
               break;

            default:
                break;
        }

        for (FocusStateListener listener : mFocusStateListener) {
            listener.onFocusStatusUpdate(state);
        }
    }
}