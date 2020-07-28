/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.camera;

import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera.Area;
import android.hardware.Camera.Parameters;
import android.media.MediaActionSound;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.android.camera.manager.ModePicker;
import com.android.camera.ui.FocusIndicator;
import com.android.camera.ui.FocusIndicatorRotateLayout;
import com.android.camera.ui.FrameView;
import com.android.camera.ui.MultiZoneAfView;
import com.android.camera.ui.MultiZoneAfView.MultiWindow;

import com.mediatek.camera.setting.ParametersHelper;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * A class that handles everything about focus in still picture mode. This also handles the metering
 * area because it is the same as focus area.
 *
 * The test cases: (1) The camera has continuous autofocus. Move the camera. Take a picture when CAF
 * is not in progress. (2) The camera has continuous autofocus. Move the camera. Take a picture when
 * CAF is in progress. (3) The camera has face detection. Point the camera at some faces. Hold the
 * shutter. Release to take a picture. (4) The camera has face detection. Point the camera at some
 * faces. Single tap the shutter to take a picture. (5) The camera has autofocus. Single tap the
 * shutter to take a picture. (6) The camera has autofocus. Hold the shutter. Release to take a
 * picture. (7) The camera has no autofocus. Single tap the shutter and take a picture. (8) The
 * camera has autofocus and supports focus area. Touch the screen to trigger autofocus. Take a
 * picture. (9) The camera has autofocus and supports focus area. Touch the screen to trigger
 * autofocus. Wait until it times out. (10) The camera has no autofocus and supports metering area.
 * Touch the screen to change metering area.
 */
public class FocusManager implements CameraActivity.OnOrientationListener,
        CameraActivity.OnParametersReadyListener {
    private static final String TAG = "FocusManager";

    private static final int RESET_TOUCH_FOCUS = 0;

    private static final int RESET_FACE_BEAUTY_TOUCH_POSITION = 1;
    private static final int RESET_MULTIAF_FOCUS = 2;
    private static final int RESET_FACE_BEAUTY_TOUCH_POSITION_DELAY = 40;

    private static final String KEY_PDAF_SUPPORTED = "pdaf-supported";
    private static final String KEY_MZAF_SUPPORTED = "is-support-mzaf";

    private int mState = STATE_UNKNOWN;
    private static final int STATE_UNKNOWN = -1;
    private static final int STATE_IDLE = 0; // Focus is not active.
    private static final int STATE_FOCUSING = 1; // Focus is in progress.
    // Focus is in progress and the camera should take a picture after focus
    // finishes.
    private static final int STATE_FOCUSING_SNAP_ON_FINISH = 2;
    private static final int STATE_SUCCESS = 3; // Focus finishes and succeeds.
    private static final int STATE_FAIL = 4; // Focus finishes and fails.

    private boolean mInitialized;
    private boolean mFocusAreaSupported;
    private boolean mLockAeAwbNeeded;
    private boolean mLockAeNeeded = true;
    private boolean mAeLock;
    private boolean mAwbLock;
    private Matrix mMatrix;
    private Matrix mObjextMatrix;

    // The parent layout that includes only the focus indicator.
    private FocusIndicatorRotateLayout mFocusIndicatorRotateLayout;
    // The focus indicator view that holds the image resource.
    private View mFocusIndicator;
    // The multi-zone af indicator view
    private MultiZoneAfView mMultiZoneAfView;
    private MultiWindow[] mMultiAfWindows;
    private int mPreviewWidth; // The width of the preview frame layout.
    private int mPreviewHeight; // The height of the preview frame layout.
    private int mCropPreviewWidth;
    private int mCropPreviewHeight;
    private boolean mMirror; // true if the camera is front-facing.
    private int mDisplayOrientation;
    private List<Area> mFocusArea; // focus area in driver format
    private List<Area> mMeteringArea; // metering area in driver format
    private String mFocusMode;
    private String[] mDefaultFocusModes;
    private String mOverrideFocusMode;
    private Parameters mParameters;
    private ComboPreferences mPreferences;
    private Handler mHandler;
    Listener mListener;
    // The Distance info for stereo feature
    private String mDistanceInfo;

    private static final int COMMON_INFO_LENGTH = 12;
    private static final int FOCUS_FRAME_DELAY = 1000;

    private static final String[] MATRIX_FOCUS_MODE_CONTINUOUS = new String[] {
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // photo
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // hdr
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // facebeauty
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // panorama
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // asd
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // photo pip
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // photo stereo
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // stereo denoise
        Parameters.FOCUS_MODE_CONTINUOUS_VIDEO, // video
        Parameters.FOCUS_MODE_CONTINUOUS_VIDEO, // vidoe pip
        Parameters.FOCUS_MODE_CONTINUOUS_VIDEO, // video stereo
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // normal3d
        Parameters.FOCUS_MODE_CONTINUOUS_PICTURE, // panorama3d
};

    public interface Listener {
        void autoFocus();

        void cancelAutoFocus();

        boolean capture();

        void startFaceDetection();

        void stopFaceDetection();

        void setFocusParameters();

        void playSound(int soundId);

        boolean readyToCapture();
    }

    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage] msg .what = " + msg.what);

            switch (msg.what) {
            case RESET_TOUCH_FOCUS:
                cancelAutoFocus();
                break;
            case RESET_FACE_BEAUTY_TOUCH_POSITION:
                resetFaceBeautyTouchPosition();
                break;
            case RESET_MULTIAF_FOCUS:
                if (mMultiZoneAfView != null) {
                    mMultiZoneAfView.clear();
                }
                break;
            default:
                break;
            }
        }
    }

    public void setFocusAreaIndicator(View l) {
        mFocusIndicatorRotateLayout = (FocusIndicatorRotateLayout) l;
        mFocusIndicator = l.findViewById(R.id.focus_indicator);
        mMultiZoneAfView = (MultiZoneAfView) mContext.findViewById(R.id.multi_focus_indicator);

        // Put focus indicator to the center.
        RelativeLayout.LayoutParams p = (RelativeLayout.LayoutParams) mFocusIndicatorRotateLayout
                .getLayoutParams();
        int[] rules = p.getRules();
        rules[RelativeLayout.CENTER_IN_PARENT] = RelativeLayout.TRUE;
        // Set the length of focus indicator according to preview frame
        // size.
        if (mFocusIndicator != null) {
            int len = Math.min(mPreviewWidth, mPreviewHeight) / 4;
            ViewGroup.LayoutParams layout = mFocusIndicator.getLayoutParams();
            layout.width = len;
            layout.height = len;
        }
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    public void setParameters(Parameters parameters) {
        mParameters = parameters;
        mFocusAreaSupported = (mParameters.getMaxNumFocusAreas() > 0 && isSupported(
                Parameters.FOCUS_MODE_AUTO, mParameters.getSupportedFocusModes()));
        // mLockAeAwbNeeded = (mInitialParameters.isAutoExposureLockSupported()
        // ||
        // mInitialParameters.isAutoWhiteBalanceLockSupported());

        mMeteringAreaSupported = (mParameters.getMaxNumMeteringAreas() > 0);
        mAeLockSupported = mParameters.isAutoExposureLockSupported();
        mAwbLockSupported = mParameters.isAutoWhiteBalanceLockSupported();
        mContinousFocusSupported = mParameters.getSupportedFocusModes().contains(
                mContinousFocusMode);
        mLockAeAwbNeeded = mAeLockSupported || mAwbLockSupported;
    }

    public void setPreviewSize(int previewWidth, int previewHeight) {
        if (mPreviewWidth != previewWidth || mPreviewHeight != previewHeight) {
            mPreviewWidth = previewWidth;
            mPreviewHeight = previewHeight;
            setMatrix();

            // Set the length of focus indicator according to preview frame
            // size.
            if (mFocusIndicator != null) {
                int len = Math.min(mPreviewWidth, mPreviewHeight) / 4;
                ViewGroup.LayoutParams layout = mFocusIndicator.getLayoutParams();
                layout.width = len;
                layout.height = len;
            }
        }
    }

    public void setCropPreviewSize(int width, int height) {
        mCropPreviewHeight = height;
        mCropPreviewWidth = width;
    }

    public void setMirror(boolean mirror) {
        if (mMultiZoneAfView != null) {
           mMultiZoneAfView.setMirror(mirror);
        }
        mMirror = mirror;
        setMatrix();
    }

    public void setDisplayOrientation(int displayOrientation) {
        mDisplayOrientation = displayOrientation;
        if (mMultiZoneAfView != null) {
            mMultiZoneAfView.setDisplayOrientation(mDisplayOrientation);
        }
        setMatrix();
    }

    private void setMatrix() {
        if (mPreviewWidth != 0 && mPreviewHeight != 0) {
            Matrix matrix = new Matrix();
            Util.prepareMatrix(matrix, mMirror, mDisplayOrientation, mPreviewWidth, mPreviewHeight);
            // In face detection, the matrix converts the driver coordinates to
            // UI
            // coordinates. In tap focus, the inverted matrix converts the UI
            // coordinates to driver coordinates.
            matrix.invert(mMatrix);

            Matrix objectMatrix = new Matrix();
            Util.prepareMatrix(objectMatrix, false, mDisplayOrientation, mPreviewWidth,
                    mPreviewHeight);
            // In Object track, the inverted matrix converts the UI
            // coordinates to driver coordinates.
            objectMatrix.invert(mObjextMatrix);
            mInitialized = true;
        }
    }

    public void onShutterDown() {
        Log.d(TAG, "onShutterDown");
        if (!mInitialized) {
            return;
        }

        // Lock AE and AWB so users can half-press shutter and recompose.
        if (mLockAeAwbNeeded && !(mAeLock && mAwbLock)) {
            setAeLock(true);
            mAwbLock = true;
            mListener.setFocusParameters();
        }

        if (needAutoFocusCall()) {
            // Do not focus if touch focus has been triggered.
            if (mState != STATE_SUCCESS && mState != STATE_FAIL) {
                autoFocus();
            }
        }
    }

    public void onShutterUp() {
        if (!mInitialized) {
            return;
        }

        if (needAutoFocusCall()) {
            // User releases half-pressed focus key.
            if (mState == STATE_FOCUSING || mState == STATE_SUCCESS || mState == STATE_FAIL) {
                cancelAutoFocus();
            }
        }

        // Unlock AE and AWB after cancelAutoFocus. Camera API does not
        // guarantee setParameters can be called during autofocus.
        boolean isLock =  (mAeLock || mAwbLock);
        if (mLockAeAwbNeeded && isLock && (mState != STATE_FOCUSING_SNAP_ON_FINISH)) {
            mAeLock = false;
            mAwbLock = false;
            mListener.setFocusParameters();
        }
    }

    public void doSnap() {
        Log.d(TAG, "[doSnap]mInitialized =" + mInitialized + " mState=" + mState);
        if (!mInitialized) {
            return;
        }

        // M:
        if (!mListener.readyToCapture()) {
            Log.d(TAG, "[doSnap]readyToCapture is false,return.");
            return;
        }

        // If the user has half-pressed the shutter and focus is completed, we
        // can take the photo right away. If the focus mode is infinity, we can
        // also take the photo.
        if (!needAutoFocusCall() || (mState == STATE_SUCCESS || mState == STATE_FAIL)) {
            capture();
        } else if (mState == STATE_FOCUSING) {
            // Half pressing the shutter (i.e. the focus button event) will
            // already have requested AF for us, so just request capture on
            // focus here.
            mState = STATE_FOCUSING_SNAP_ON_FINISH;
        } else if (mState == STATE_IDLE) {
            // We didn't do focus. This can happen if the user press focus key
            // while the snapshot is still in progress. The user probably wants
            // the next snapshot as soon as possible, so we just do a snapshot
            // without focusing again.
            capture();

        }
    }

    public void onAutoFocus(boolean focused) {
        Log.d(TAG, "onAutoFocus focused=" + focused + " mState=" + mState + " mFocusMode="
                + mFocusMode);
        if (mState == STATE_FOCUSING_SNAP_ON_FINISH) {
            // Take the picture no matter focus succeeds or fails. No need
            // to play the AF sound if we're about to play the shutter
            // sound.
            if (focused) {
                mState = STATE_SUCCESS;
            } else {
                mState = STATE_FAIL;
            }
            updateFocusUI();
            capture();
        } else if (mState == STATE_FOCUSING) {
            // This happens when (1) user is half-pressing the focus key or
            // (2) touch focus is triggered. Play the focus tone. Do not
            // take the picture now.
            if (focused) {
                mState = STATE_SUCCESS;
                // Do not play the sound in continuous autofocus mode. It does
                // not do a full scan. The focus callback arrives before doSnap
                // so the state is always STATE_FOCUSING.
                if (!Parameters.FOCUS_MODE_CONTINUOUS_PICTURE.equals(mFocusMode)) {
                    mListener.playSound(MediaActionSound.FOCUS_COMPLETE);
                }
            } else {
                mState = STATE_FAIL;
            }
            updateFocusUI();
            // If this is triggered by touch focus, cancel focus after a
            // while.
            mHandler.sendEmptyMessageDelayed(RESET_TOUCH_FOCUS, FOCUS_FRAME_DELAY);
        } else if (mState == STATE_IDLE) {
            mHandler.sendEmptyMessage(RESET_TOUCH_FOCUS);
        }
    }

    public void onAutoFocusMoving(boolean moving) {
        Log.d(TAG, "onAutoFocusMoving = " + moving);
        // Ignore if the camera has detected some faces.
        if ((getFrameview() != null && getFrameview().faceExists())) {
            return;
        }

        // Ignore if we have requested autofocus. This method only handles
        // continuous autofocus.
        if (mState != STATE_IDLE && mState != STATE_UNKNOWN) {
            Log.d(TAG, "[onAutoFocusMoving]return,mState = " + mState);
            return;
        }

        // if current focus mode is INFINISTY, then don't show AF box
        if (Parameters.FOCUS_MODE_INFINITY.equals(getCurrentFocusMode(mContext))) {
            Log.d(TAG, "[onAutoFocusMoving]return,current focus mode is INFINISTY.");
            return;
        }
        mListener.setFocusParameters();
        if (hasMultiAFData(mMultiAfWindows)) {
            if (mFocusIndicatorRotateLayout.isFocusing()) {
                mFocusIndicatorRotateLayout.clear();
            }
            handleMultiAfWindow(moving);
            return;
        }
        if (moving) {
            mFocusIndicatorRotateLayout.showStart();
        } else {
            mFocusIndicatorRotateLayout.showSuccess(true);
        }
    }

    private boolean hasMultiAFData(MultiWindow[] windows) {
        boolean result = windows != null && windows.length > 0;
        Log.d(TAG, "hasMultiAFData result = " + result);
        return result;
    }

    private void resetFaceBeautyTouchPosition() {
        Log.d(TAG, "resetFaceBeautyTouchPosition");
        if (mContext.getParameters() != null) {
            mContext.getParameters().set(ParametersHelper.KEY_VIDED_FACE_BEAUTY_TOUCH,
                    "-2000:-2000");
        }
    }

    public void onSingleTapUp(int x, int y) {
        Log.d(TAG, "onSingleTapUp x = " + x + " y = " + y);

        // when devices support both continuous and infinity focus mode.
        String focusMode = getCurrentFocusMode(mContext);
        if (focusMode == null || Parameters.FOCUS_MODE_INFINITY.equals(focusMode)) {
            Log.w(TAG, "[onSingleTapUp]focusMode:" + focusMode);
            return;
        }
        // Check if metering area or focus area is supported.
        if (!mFocusAreaSupported) {
            Log.i(TAG, "[onSingleTapUp] mFocusAreaSupported is false");
            return;
        }
        if (!mInitialized || mState == STATE_FOCUSING_SNAP_ON_FINISH || mState == STATE_UNKNOWN) {
            return;
        }

        // Let users be able to cancel previous touch focus.
        if ((mFocusArea != null)
                && (mState == STATE_FOCUSING || mState == STATE_SUCCESS || mState == STATE_FAIL)) {
            cancelAutoFocus();
        }

        // Initialize variables.
        int focusWidth = mFocusIndicatorRotateLayout.getWidth();
        int focusHeight = mFocusIndicatorRotateLayout.getHeight();
        if (focusWidth == 0 || focusHeight == 0) {
            Log.i(TAG, "UI Component not initialized, cancel this touch");
            return;
        }
        int previewWidth = mPreviewWidth;
        int previewHeight = mPreviewHeight;
        if (mFocusArea == null) {
            mFocusArea = new ArrayList<Area>();
            mFocusArea.add(new Area(new Rect(), 1));
            mMeteringArea = new ArrayList<Area>();
            mMeteringArea.add(new Area(new Rect(), 1));
        }

        // add for vFB ->begin
        int[] nativePoint = calculateTapPoint(x, y);
        if (FeatureSwitcher.isVfbEnable()
                && mContext.getCurrentMode() == ModePicker.MODE_FACE_BEAUTY && nativePoint != null
                && nativePoint.length == 2) {
            Log.d(TAG, "[vFB]set touch point to native ,x = " + nativePoint[0] + ",y = "
                    + nativePoint[1]);
            String value = nativePoint[0] + ":" + nativePoint[1];
            mContext.getParameters().set(ParametersHelper.KEY_VIDED_FACE_BEAUTY_TOUCH, value);
            mHandler.sendEmptyMessageDelayed(RESET_FACE_BEAUTY_TOUCH_POSITION,
                    RESET_FACE_BEAUTY_TOUCH_POSITION_DELAY);
        }
        // add for vFB <-end

        // Convert the coordinates to driver format.
        calculateTapArea(focusWidth, focusHeight, 1f, x, y, previewWidth, previewHeight,
                mFocusArea.get(0).rect);
        calculateTapArea(focusWidth, focusHeight, 1f, x, y, previewWidth, previewHeight,
                mMeteringArea.get(0).rect);

        // Use margin to set the focus indicator to the touched area.
        RelativeLayout.LayoutParams p = (RelativeLayout.LayoutParams) mFocusIndicatorRotateLayout
                .getLayoutParams();
        int left = 0;
        int top = 0;
        left = Util.clamp(x - focusWidth / 2, 0, previewWidth - focusWidth);
        top = Util.clamp(y - focusHeight / 2, 0, previewHeight - focusHeight);

        if (p.getLayoutDirection() != View.LAYOUT_DIRECTION_RTL) {
            p.setMargins(left, top, 0, 0);
        } else {
            // since in RTL language, framework will use marginRight as
            // standard.
            int right = previewWidth - (left + focusWidth);
            p.setMargins(0, top, right, 0);
        }

        // Disable "center" rule because we no longer want to put it in the
        // center.
        int[] rules = p.getRules();
        rules[RelativeLayout.CENTER_IN_PARENT] = 0;
        mFocusIndicatorRotateLayout.requestLayout();

        // Stop face detection because we want to specify focus and metering area.
        // it can't be called twice when focusing.
        mListener.stopFaceDetection();

        // Set the focus area and metering area.
        mListener.setFocusParameters();
        autoFocus();
    }


    public void onPreviewStarted() {
        Log.d(TAG, "onPreviewStarted");
        mState = STATE_IDLE;
    }

    public void onPreviewStopped() {
        Log.d(TAG, "onPreviewStopped");
        mState = STATE_UNKNOWN;
        resetTouchFocus();
        // If auto focus was in progress, it would have been canceled.
        updateFocusUI();
    }

    public void onCameraReleased() {
        onPreviewStopped();
    }

    public void cancelAutoFocus() {
        Log.d(TAG, "Cancel autofocus.");
        // Reset the tap area before calling mListener.cancelAutofocus.
        // Otherwise, focus mode stays at auto and the tap area passed to the
        // driver is not reset.
        resetTouchFocus();
        if (mListener != null) {
            mListener.cancelAutoFocus();
            mListener.startFaceDetection();
        }
        if (getFrameview() != null) {
            getFrameview().resume();
        }
        mState = STATE_IDLE;
        updateFocusUI();
        mHandler.removeMessages(RESET_TOUCH_FOCUS);
    }

    private void autoFocus() {
        Log.d(TAG, "Start autofocus.");
        mListener.autoFocus();
        mState = STATE_FOCUSING;
        // Pause the face view because the driver will keep sending face
        // callbacks after the focus completes.
        if (getFrameview() != null) {
            getFrameview().pause();
        }
        updateFocusUI();
        mHandler.removeMessages(RESET_TOUCH_FOCUS);
    }

    private void capture() {
        if (mListener.capture()) {
            mState = STATE_IDLE;
            mFocusArea = null;
            // Clear focus and frame view ui when capture
            resetTouchFocus();
            updateFocusUI();
            mHandler.removeMessages(RESET_TOUCH_FOCUS);
        }
    }

    /**
     * Gets the current supported focus mode.
     *
     * @return The supported focus mode .
     * @see android.hardware.Parameters#getFocusMode
     */
    public String getFocusMode() {
        Log.d(TAG, "getFocusMode() mOverrideFocusMode=" + mOverrideFocusMode + " mFocusArea="
                + mFocusArea + " mFocusAreaSupported=" + mFocusAreaSupported);
        if (mOverrideFocusMode != null) {
            return mOverrideFocusMode;
        }
        List<String> supportedFocusModes = mParameters.getSupportedFocusModes();

        if (mFocusAreaSupported && mFocusArea != null) {
            // Always use autofocus in tap-to-focus.
            mFocusMode = Parameters.FOCUS_MODE_AUTO;
        } else {
            // The default is continuous autofocus.
            mFocusMode = mContinousFocusMode;

            // Try to find a supported focus mode from the default list.
            if (mFocusMode == null) {
                for (int i = 0; i < mDefaultFocusModes.length; i++) {
                    String mode = mDefaultFocusModes[i];
                    if (isSupported(mode, supportedFocusModes)) {
                        mFocusMode = mode;
                        break;
                    }
                }
            }
        }
        if (!isSupported(mFocusMode, supportedFocusModes)) {
            // For some reasons, the driver does not support the current
            // focus mode. Fall back to auto.
            if (isSupported(Parameters.FOCUS_MODE_AUTO, mParameters.getSupportedFocusModes())) {
                mFocusMode = Parameters.FOCUS_MODE_AUTO;
            } else {
                mFocusMode = mParameters.getFocusMode();
            }
        }

        Log.d(TAG, "getFocusMode() return " + mFocusMode);
        return mFocusMode;
    }

    public List<Area> getFocusAreas() {
        return mFocusArea;
    }

    public List<Area> getMeteringAreas() {
        return mMeteringArea;
    }

    public void updateFocusUI() {
        if (!mInitialized) {
            return;
        }
        if (mMultiZoneAfView != null) {
            mMultiZoneAfView.clear();
        }
        // Show only focus indicator or face indicator.
        boolean faceExists = (getFrameview() != null && getFrameview().faceExists());
        FocusIndicator focusIndicator = (faceExists) ? getFrameview() : mFocusIndicatorRotateLayout;
        Log.d(TAG, "updateFocusUI, faceExists = " + faceExists + ", mState = " + mState
                + " mFocusArea = " + mFocusArea + " focusIndicator = " + focusIndicator);
        if (mState == STATE_IDLE || mState == STATE_UNKNOWN) {
            if (mFocusArea == null) {
                focusIndicator.clear();
            } else {
                // Users touch on the preview and the indicator represents the
                // metering area. Either focus area is not supported or
                // autoFocus call is not required.
                focusIndicator.showStart();
            }
        } else if (mState == STATE_FOCUSING || mState == STATE_FOCUSING_SNAP_ON_FINISH) {
            focusIndicator.showStart();
        } else {
            if (mState == STATE_SUCCESS) {
                if (mParameters != null
                        && Parameters.FOCUS_MODE_AUTO.equals(mParameters.getFocusMode())
                        && "on".equals(mParameters.get(com.mediatek.camera.platform.Parameters
                            .KEY_STEREO_DISTANCE_MODE))
                        && !faceExists) {
                    focusIndicator.needDistanceInfoShow(true);
                }
                focusIndicator.showSuccess(false);
            } else if (mState == STATE_FAIL) {
                focusIndicator.showFail(false);
            }
        }
    }

    public void resetTouchFocus() {
        Log.d(TAG, "resetTouchFocus mInitialized = " + mInitialized);
        if (!mInitialized) {
            return;
        }

        // Put focus indicator to the center.
        RelativeLayout.LayoutParams p = (RelativeLayout.LayoutParams) mFocusIndicatorRotateLayout
                .getLayoutParams();
        int[] rules = p.getRules();
        rules[RelativeLayout.CENTER_IN_PARENT] = RelativeLayout.TRUE;
        p.setMargins(0, 0, 0, 0);
        mFocusIndicatorRotateLayout.clear();
        mState = STATE_IDLE ;
        mFocusArea = null;
        mMeteringArea = null;
    }

    public void calculateTapArea(int focusWidth, int focusHeight, float areaMultiple, int x, int y,
            int previewWidth, int previewHeight, Rect rect) {
        Log.d(TAG, "[calculateTapArea] previewWidth = " + previewWidth + ", previewHeight = "
                + previewHeight + ", mCropPreviewHeight = " + mCropPreviewHeight
                + ", mCropPreviewWidth = " + mCropPreviewWidth + ", x = " + x + ", y = " + y);
        // need recalculate the coordinates, preview is not full screen because navigation bar.
        if (previewHeight >= previewWidth) {
            int y_b = (int) (previewHeight - mCropPreviewHeight) / 2;
            y = y_b + y;
        } else {
            int x_b = (int) (previewWidth - mCropPreviewWidth) / 2;
            x = x_b + x;
        }
        int areaWidth = (int) (focusWidth * areaMultiple);
        int areaHeight = (int) (focusHeight * areaMultiple);
        int left = Util.clamp(x - areaWidth / 2, 0, previewWidth - areaWidth);
        int top = Util.clamp(y - areaHeight / 2, 0, previewHeight - areaHeight);

        RectF rectF = new RectF(left, top, left + areaWidth, top + areaHeight);
        mMatrix.mapRect(rectF);
        Util.rectFToRect(rectF, rect);
    }

    public int[] calculateTapPoint(int x, int y) {
        float[] pts = new float[2];
        pts[0] = (float) x;
        pts[1] = (float) y;
        mObjextMatrix.mapPoints(pts);
        return Util.pointFToPoint(pts);
    }

    public boolean isFocusCompleted() {
        return mState == STATE_SUCCESS || mState == STATE_FAIL || mState == STATE_IDLE  ;
    }

    public boolean isFocusingSnapOnFinish() {
        return mState == STATE_FOCUSING_SNAP_ON_FINISH;
    }

    public void removeMessages() {
        mHandler.removeMessages(RESET_TOUCH_FOCUS);
    }

    public void overrideFocusMode(String focusMode) {
        mOverrideFocusMode = focusMode;
    }

    public void setAwbLock(boolean lock) {
        mAwbLock = lock;
    }

    public void setLockAeNeeded(boolean neededLock) {
        mLockAeNeeded = neededLock;
    }

    public void setAeLock(boolean lock) {
        if (mLockAeNeeded) {
            mAeLock = lock;
        } else {
            mAeLock = false;
        }
    }

    public boolean getAwbLock() {
        return mAwbLock;
    }

    public boolean getAeLock() {
        return mAeLock;
    }

    public void setDistanceInfo(String info) {
        mDistanceInfo = info;
        if (mFocusIndicatorRotateLayout != null) {
            mFocusIndicatorRotateLayout.setDistanceInfo(info);
        }
    }

    /**
     * Set multi-zone af data to focusmanager.
     *
     * @param data
     *            The data of multi-zone af.
     */
    public void setAfData(byte[] data) {
        if (data == null) {
            mMultiAfWindows = null;
            return;
        }
        mMultiAfWindows = getMultiWindows(data);
    }

    public String getDistanceInfo() {
        return mDistanceInfo;
    }

    private static boolean isSupported(String value, List<String> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    private boolean needAutoFocusCall() {
        String focusMode = getFocusMode();
        boolean needAutoFocus = !(focusMode.equals(Parameters.FOCUS_MODE_INFINITY)
                || focusMode.equals(Parameters.FOCUS_MODE_FIXED) || focusMode
                    .equals(Parameters.FOCUS_MODE_EDOF));
        Log.i(TAG, "needAutoFocusCall,needAutoFocus = " + needAutoFocus);
        return needAutoFocus;
    }

    /**
     * Clear UI of continuous focus.
     */
    public void clearFocusAndFaceUi() {
        // Clear face UI
        if (getFrameview() != null) {
            getFrameview().clear();
        }
        clearFocusUi();
    }

    /**
     * Clear all focus UI.
     */
    public void clearFocusUi() {
        if(mFocusIndicatorRotateLayout.isFocusing()){
            mFocusIndicatorRotateLayout.clear();
        }
        mMultiAfWindows = null;
        if (mMultiZoneAfView != null) {
            mMultiZoneAfView.clear();
        }
    }

    // / M: add for focus other capability
    // Note: google default setParameters() will be filled by initial
    // parameters.
    private boolean mMeteringAreaSupported;
    private boolean mAeLockSupported;
    private boolean mAwbLockSupported;
    private boolean mContinousFocusSupported;
    private String mContinousFocusMode;

    private CameraActivity mContext;
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;

    public boolean getAeLockSupported() {
        return mAeLockSupported;
    }

    public boolean getAwbLockSupported() {
        return mAwbLockSupported;
    }

    public boolean getFocusAreaSupported() {
        return mFocusAreaSupported;
    }

    public boolean getMeteringAreaSupported() {
        return mMeteringAreaSupported;
    }

    public boolean getContinousFocusSupported() {
        return mContinousFocusSupported;
    }

    public String getCurrentFocusMode(CameraActivity context) {
        if (context.getParameters() != null) {
            return context.getParameters().getFocusMode();
        } else {
            return null;
        }
    }

    public FocusManager(CameraActivity context, ComboPreferences preferences,
            View focusIndicatorRotate, Parameters parameters, Listener listener,
            boolean mirror, Looper looper, int mode) {
        mContext = context;
        mHandler = new MainHandler(looper);
        mMatrix = new Matrix();
        mObjextMatrix = new Matrix();

        mPreferences = preferences;
        mDefaultFocusModes = getModeDefaultFocusModes(mode);
        mListener = listener;
        if (focusIndicatorRotate != null) {
            setFocusAreaIndicator(focusIndicatorRotate);
        }
        setParameters(parameters);
     // Here we modify current mode continous focus mode.
        mContinousFocusMode = getModeContinousFocusMode(mode);
        setMirror(mirror);

        mContext.addOnOrientationListener(this);
        mContext.addOnParametersReadyListener(this);
        if (mDefaultFocusModes != null) {
            for (int i = 0, len = mDefaultFocusModes.length; i < len; i++) {
                Log.d(TAG, "FocusManager() defaultFocusModes[" + i + "]=" + mDefaultFocusModes[i]);
            }
        }
    }

    @Override
    public void onOrientationChanged(int orientation) {
        if (mOrientation != orientation && mFocusIndicator != null) {
            mOrientation = orientation;
            mFocusIndicatorRotateLayout.setOrientation(mOrientation, true);
            if (mMultiZoneAfView != null) {
                mMultiZoneAfView.setOrientation(orientation);
            }
        }
    }

    @Override
    public void onCameraParameterReady() {
        if (mState == STATE_UNKNOWN) {
            mState = STATE_IDLE;
        }
    }

    public void release() {
        mContext.removeOnOrientationListener(this);
        mContext.removeOnParametersReadyListener(this);
    }

    public FrameView getFrameview() {
        return mContext.getFrameManager().getFrameView();
    }

    private String[] getModeDefaultFocusModes(int mode) {
        if (mode == ModePicker.MODE_VIDEO || mode == ModePicker.MODE_VIDEO_PIP
                || mode == ModePicker.MODE_VIDEO_STEREO) {
            String[] videoModeSupportedList = new String[3];
            videoModeSupportedList[0] = Parameters.FOCUS_MODE_AUTO;
            videoModeSupportedList[1] = Parameters.FOCUS_MODE_CONTINUOUS_VIDEO;
            videoModeSupportedList[2] = Parameters.FOCUS_MODE_INFINITY;
            return videoModeSupportedList;
        } else {
            String[] defaultSupportedList = new String[2];
            defaultSupportedList[0] = Parameters.FOCUS_MODE_CONTINUOUS_PICTURE;
            defaultSupportedList[1] = Parameters.FOCUS_MODE_AUTO;
            return defaultSupportedList;
        }
    }

    private String getModeContinousFocusMode(int mode) {
        // In video mode, should set parameter focus mode FOCUS_MODE_AUTO
        if ((mode == ModePicker.MODE_VIDEO && !isPdafSupported())
                || mode == ModePicker.MODE_VIDEO_PIP
                || mode == ModePicker.MODE_VIDEO_STEREO) {
            return null;
        } else {
            return MATRIX_FOCUS_MODE_CONTINUOUS[mode];
        }
    }

    private boolean isPdafSupported() {
        boolean isSupported = false;
        if (mParameters != null
                && "true".equals(mParameters.get(KEY_PDAF_SUPPORTED))) {
            isSupported = true;
        } else {
            isSupported = false;
        }
        Log.d(TAG, "[isPdafSupported] isSupported = " + isSupported);
        return isSupported;
    }

    private void handleMultiAfWindow(boolean moving) {
        int length = mMultiAfWindows.length;
        if (moving) {
            // Do not care the result of the windows when focusing begin
            for (int i = 0; i < length; i++) {
                mMultiAfWindows[i].mResult = 0;
            }
            if (mMultiZoneAfView != null) {
                mMultiZoneAfView.updateFocusWindows(mMultiAfWindows);
                mMultiZoneAfView.showWindows(true);
            }
        } else {
            // Only the focused window should be shown.
            List<MultiWindow> list = new ArrayList<MultiWindow>();
            for (int i = 0; i < length; i++) {
                if (mMultiAfWindows[i].mResult > 0) {
                    list.add(mMultiAfWindows[i]);
                }
            }
            MultiWindow[] tempWindows = new MultiWindow[list.size()];
            for (int i = 0; i < list.size(); i++) {
                tempWindows[i] = list.get(i);
            }
            if (mMultiZoneAfView != null) {
                mMultiZoneAfView.updateFocusWindows(tempWindows);
                mMultiZoneAfView.showWindows(false);
            }
            mHandler.sendEmptyMessageDelayed(RESET_MULTIAF_FOCUS, FOCUS_FRAME_DELAY);
        }
    }


    /**
     * Get multi-zone Windows from the given data.
     *
     * @param data The data of the windows. There are three information of each window,
     * the x-coordinate and y-coordinate of the window's center which stands for the window
     * position and the result which stands for whether the zone is focused or not.
     * The data structure is as follow():
     * Common information(The first 12 bytes):
     * data[0]~data[2]:the total number of multi zone;
     * data[3]~data[5]:a single focus window width;
     * data[6]~data[8]:a single focus window height;
     * Window information(the other bytes):
     * data[9]~data[11]:the x-coordinate of the first window's center;
     * data[12]~data[14]:the y-coordinate of the first window's center;
     * data[15]~data[17]:the result of the first window.
     * The other window information are as so on.
     *
     * @return All the multi-zone windows information.
     */
    private MultiWindow[] getMultiWindows(byte[] data) {
        Log.d(TAG, "getMultiWindows original data size " + data.length);
        IntBuffer intBuffer = ByteBuffer.wrap(data).order(ByteOrder.nativeOrder()).asIntBuffer();
        // Get common information of af window
        int windowCount = intBuffer.get(0);
        int windowWidth = intBuffer.get(1);
        int windowHeight = intBuffer.get(2);
        // Get the windows buffer(git rid of common information from total data )
        IntBuffer windowBuffer = ByteBuffer
                .wrap(data, COMMON_INFO_LENGTH, data.length - COMMON_INFO_LENGTH)
                .order(ByteOrder.nativeOrder()).asIntBuffer();
        Log.d(TAG, "getMultiWindows windowCount " + windowCount
                + " ,single window (width,height ) from native " + "(" + windowWidth + " ,"
                + windowHeight + ")");
        MultiWindow[] windows = new MultiWindow[windowCount];
        // Get each window information
        int limit = windowBuffer.limit();
        for (int i = 0; i < limit; i = 3 + i) {
            // Step1:Get the original window information
            Rect position = new Rect();
            int x = windowBuffer.get(i);
            int y = windowBuffer.get(i + 1);
            int result = windowBuffer.get(i + 2);
            // Step2:Calculate the bounds of the window
            position.left = x - windowWidth / 2;
            position.top = y - windowHeight / 2;
            position.right = x + windowWidth / 2;
            position.bottom = y + windowHeight / 2;
            // Step3:Save the window information to a MultiWindow
            MultiWindow tempWindow = new MultiWindow(position, result);
            windows[i / 3] = tempWindow;
        }
        return windows;
    }

}
