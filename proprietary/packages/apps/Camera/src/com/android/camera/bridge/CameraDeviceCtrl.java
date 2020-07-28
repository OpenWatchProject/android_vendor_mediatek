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
package com.android.camera.bridge;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.os.ConditionVariable;
import android.os.HandlerThread;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.FrameLayout;

import com.android.camera.CameraActivity;
import com.android.camera.CameraDisabledException;
import com.android.camera.CameraHardwareException;
import com.android.camera.CameraHolder;
import com.android.camera.CameraManager;
import com.android.camera.ComboPreferences;
import com.android.camera.FocusManager;
import com.android.camera.Log;
import com.android.camera.ModeChecker;
import com.android.camera.ParametersHelper;
import com.android.camera.R;
import com.android.camera.SaveRequest;
import com.android.camera.Storage;
import com.android.camera.Util;
import com.android.camera.actor.CameraActor;
import com.android.camera.manager.ModePicker;
import com.android.camera.ui.FrameView;
import com.android.camera.ui.PreviewFrameLayout;
import com.android.camera.ui.PreviewSurfaceView;
import com.android.camera.ui.RotateLayout;

import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ModuleManager;
import com.mediatek.camera.util.CameraPerformanceTracker;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.IFocusManager;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.util.ReflectUtil;

import junit.framework.Assert;

import java.lang.reflect.Method;

public class CameraDeviceCtrl implements SurfaceHolder.Callback {
    private final String TAG = "CameraDeviceCtrl";

    private static final int UNKNOWN = -1;
    private static final int CAMERA_HARDWARE_EXCEPTION = -1;
    private static final int CAMERA_DISABLED_EXCEPTION = -2;
    private static final int CAMERA_OPEN_SUCEESS = 0;

    private static final int MSG_OPEN_CAMERA_FAIL = 9;
    private static final int MSG_OPEN_CAMERA_DISABLED = 10;
    private static final int MSG_CAMERA_PREFERENCE_READY = 12;
    private static final int MSG_CAMERA_PARAMETERS_READY = 13;
    private static final int MSG_CAMERA_PREVIEW_DONE = 14;
    private static final int MSG_FOCUS_MANAGER_READY = 15;
    private static final int MSG_CAMERA_OPEN_DONE = 16;
    private static final int MSG_SET_PREVIEW_ASPECT_RATIO = 17;
    private static final int MSG_PREPARE_SURFACE_VIEW = 18;
    private static final int MSG_REMOVE_PREVIEW_COVER = 19;
    private static final int TIME_WAIT_STARTUP_THREAD = 3000;
    private static final int MSG_SET_SURFACE = 1;

    private static Method sCameraSetPropertyMethod = ReflectUtil.getMethod(
            Camera.class, "setProperty", String.class, String.class);

    private final ConditionVariable mWaitCameraStartUpThread = new ConditionVariable();
    private final CameraActivity mCameraActivity;
    private final ComboPreferences mPreferences;
    private final MainHandler mMainHandler;

    private CameraAppUiImpl mCameraAppUi;
    private ISettingCtrl mISettingCtrl;
    private ModuleManager mModuleManager;

    private ICameraDeviceExt mDummyCameraDevice = new DummyCameraDevice();
    private ICameraDeviceExt mCurCameraDevice = mDummyCameraDevice;
    private ICameraDeviceExt mTopCameraDevice = mDummyCameraDevice;
    private ICameraDeviceExt mOldTopCameraDevice = mDummyCameraDevice;

    private RotateLayout mFocusAreaIndicator;
    private FocusManager mFocusManager;
    private CamcorderProfile mProfile;
    private CameraActor mCameraActor;
    private final Object mCameraActorSync = new Object();
    private SurfaceTexture mSurfaceTexture;
    private SurfaceTexture mTopCamSurfaceTexture;
    private PreviewSurfaceView mSurfaceView;
    private View               mSurfaceViewCover;
    private FrameLayout mCurSurfaceViewLayout;
    private FrameLayout mLastSurfaceViewLayout;
    private CameraStartUpThread mCameraStartUpThread;
    private CameraHandler mCameraHandler;

    private String mLastZsdMode;
    private Size mLastPreviewSize;

    private int mLastAudioBitRate = UNKNOWN;
    private int mLastVideoBitRate = UNKNOWN;

    private int mPreviewFrameWidth;
    private int mPreviewFrameHeight;
    private int mUnCropWidth;
    private int mUnCropHeight;

    private int mOrientation = 0; // TODO don't initialize

    private boolean mIsSurfaceTextureReady = true;
    private boolean mIsNeedResetFocus = true;
    private boolean mIsOpenCameraFail = false;
    private boolean mIsFirstOpenCamera = true;
    private boolean mIsWaitForStartUpThread = false;
    private boolean mIsSwitchingPip = false;
    private boolean mIsFirstStartUp = false;
    // need to create new surface when resume from pause
    // or old surface flash dirty buffer.
    private boolean mIsSurfaceClear = true;
    private ConditionVariable mSycForLaunch = new ConditionVariable();

    private CameraState mCameraState = CameraState.STATE_CAMERA_CLOSED;

    private enum CameraState {
        STATE_CAMERA_CLOSED, STATE_OPENING_CAMERA, STATE_CAMERA_OPENED
    }

    public CameraDeviceCtrl(CameraActivity activity, ComboPreferences preferences) {
        mCameraActivity = activity;
        mPreferences = preferences;
        mIsFirstStartUp = true;
        mMainHandler = new MainHandler(mCameraActivity.getMainLooper());
        mCameraStartUpThread = new CameraStartUpThread();
        mCameraStartUpThread.start();
        HandlerThread ht = new HandlerThread("Camera Handler Thread");
        ht.start();
        mCameraHandler = new CameraHandler(ht.getLooper());
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_SURFACEVIEW_CREATE,
                CameraPerformanceTracker.ISBEGIN);
//        Log.d(TAG, "surfaceCreated, mSurfaceHolder = " + holder);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_SURFACEVIEW_CREATE,
                CameraPerformanceTracker.ISEND);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
//        Log.d(TAG, "surfaceChanged, mSurfaceHolder = " + holder + " width = " + width
//                + " height = " + height);
        if (width != mSurfaceView.getMeasuredWidth() || height != mSurfaceView.getMeasuredHeight()) {
            Log.i(TAG, "[surfaceChanged] mSurfaceHolder = " + holder + " width = " + width
                    + " height = " + height + " surface = " + holder.getSurface()
                    + ", mSurfaceView.getMeasuredWidth() = " + mSurfaceView.getMeasuredWidth()
                    + ", mSurfaceView.getMeasuredHeight() = " + mSurfaceView.getMeasuredHeight()
                    + ", surfaceview size is not same as surface size, ignore this callback");
            return;
        }
        // Do not access the camera if camera device is null.
        // Set preview display if the surface is being created. Preview was
        // already started.
        if (mModuleManager.isDisplayUseSurfaceView()) {
            mModuleManager.notifySurfaceViewDisplayIsReady();
        }
        if (mIsFirstStartUp && holder.isCreating() && mModuleManager.isDeviceUseSurfaceView()) {
            mCameraHandler.sendEmptyMessage(MSG_SET_SURFACE);
            return;
        }
        if (holder.isCreating() && mModuleManager.isDeviceUseSurfaceView()) {
            mCurCameraDevice.setPreviewDisplayAsync(holder);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
//         Log.d(TAG, "surfaceDestroyed, mSurfaceHolder = " + holder);
         notifySurfaceViewDestroy(holder);
    }

    public void setModuleManager(ModuleManager manager) {
        mModuleManager = manager;
    }

    public void setSettingCtrl(ISettingCtrl settingCtrl) {
        mISettingCtrl = settingCtrl;
    }

    public void setCameraAppUi(CameraAppUiImpl cameraAppUi) {
        mCameraAppUi = cameraAppUi;
    }

    public void setCameraActor(CameraActor cameraActor) {
        synchronized (mCameraActorSync) {
            mCameraActor = cameraActor;
            mCameraActorSync.notifyAll();
        }
    }

    public void resumeStartUpThread() {
        if (mCameraActor != null) {
            mCameraStartUpThread.resumeThread();
        }
    }

    public void onPause() {
//        Log.d(TAG, "[onPause] mIsOpenCameraFail:" + mIsOpenCameraFail
//                + " isFullScreen = " + mCameraActivity.isFullScreen());
        showRootCover();
        if (mSurfaceView != null) {
            mSurfaceView.shrink();
        }
        mIsSurfaceClear = false;
        releaseSurfaceTexture();
        notifySurfaceViewDestroy(mSurfaceView.getHolder());
        // close camera sync in stereo mode.
        if (mCameraActor.getMode() == ModePicker.MODE_STEREO_CAMERA
                || mCameraActor.getMode() == ModePicker.MODE_VIDEO_STEREO) {
            closeCamera(false);
        } else {
            closeCamera(true);
        }
        mIsOpenCameraFail = false;
        Util.hideAlertDialog(mCameraActivity);
    }

    public void onResume() {
//        Log.d(TAG, "[onResume] mIsOpenCameraFail:" + mIsOpenCameraFail);
        if (mIsOpenCameraFail) {
            return;
        }
        if (mSurfaceView != null && !mIsSurfaceClear) {
            Log.d(TAG, "resume clear SurfaceView");
            mSurfaceView = null;
            mLastSurfaceViewLayout = mCurSurfaceViewLayout;
            detachSurfaceViewLayout();
            createSurfaceView();
        } else {
            setSurfaceViewVisible(View.VISIBLE);
        }
        if (mSurfaceView != null) {
            mSurfaceView.expand();
        }
        Util.hideAlertDialog(mCameraActivity);
        openCamera();
        return;
    }

    public void onDestory() {
        setSurfaceViewVisible(View.GONE);
        mIsSurfaceClear = true;
        mCameraStartUpThread.terminate();
        if (mCameraHandler != null) {
            mCameraHandler.getLooper().quit();
        }
    }

    public void openCamera() {
        Log.d(TAG, "[openCamera] cameraState:" + getCameraState());
        if (getCameraState() != CameraState.STATE_CAMERA_CLOSED) {
            return;
        }
        mCameraStartUpThread.openCamera();
        setCameraState(CameraState.STATE_OPENING_CAMERA);
    }

    public void openCamera(int cameraId) {
        Log.d(TAG, "[openCamera] cameraState:" + getCameraState() + ",cameraId:" + cameraId);
        if (getCameraState() != CameraState.STATE_CAMERA_CLOSED) {
            return;
        }
        mCameraStartUpThread.setCameraId(cameraId);
        mCameraStartUpThread.openCamera();
        setCameraState(CameraState.STATE_OPENING_CAMERA);
    }

    public boolean isCameraOpened() {
        if (getCameraState() != CameraState.STATE_CAMERA_OPENED) {
            return false;
        } else {
            return true;
        }
    }

    public boolean isCameraIdle() {
        // TODO it's right ?
        boolean idle = (mCameraState == CameraState.STATE_CAMERA_OPENED)
                && ((mFocusManager != null) && mFocusManager.isFocusCompleted());
        Log.d(TAG, "isCameraIdle() mCameraState=" + mCameraState + ", return " + idle);
        return idle;
    }

    public void switchCamera(int cameraId) {
        Log.d(TAG, "switchCamera() cameraId=" + cameraId + ", mIsOpenCameraFail:"
                + mIsOpenCameraFail);

        if (mIsOpenCameraFail) {
            return;
        }
        closeCamera(false);

        mCameraAppUi.collapseViewManager(true);
        clearFocusAndFace();

        mCameraAppUi.setCameraId(cameraId);
        // if before switch camera , user is into VFB and then there is no face
        // in preview 5s;
        // so will back to normal mode, but when user switch camera 0-> 1-> 0;
        // at now if there have faces ,will into VFB mode ;but this is not meet
        // with user demand
        // because when in camera 1,is normal mode,when back to 0,also need to
        // normal mode
        // so when into another camera,need reset the tag
        mCameraAppUi.changeBackToVFBModeStatues(false);
        // Restart the camera and initialize the UI. From onCreate.
        mPreferences.setLocalId(mCameraActivity, cameraId);
        SettingUtils.upgradeLocalPreferences(mPreferences.getLocal());
        SettingUtils.writePreferredCameraId(mPreferences, cameraId);

        // here set these variables null to initialize them again.
        unInitializeFocusManager();
        // set camera id
        openCamera(cameraId);
    }

    public void openStereoCamera(int cameraId, boolean needSync) {
        Log.d(TAG, "openStereoCamera() cameraId=" + cameraId
                + ", mIsOpenCameraFail:" + mIsOpenCameraFail);
        if (cameraId != -1) {
            mCameraStartUpThread.setCameraId(cameraId);
            mPreferences.setLocalId(mCameraActivity, cameraId);
            SettingUtils.upgradeLocalPreferences(mPreferences.getLocal());
        }
        mCameraStartUpThread.openCamera();
        setCameraState(CameraState.STATE_OPENING_CAMERA);
        if (needSync) {
            mWaitCameraStartUpThread.close();
            mCameraStartUpThread.resumeThread();
            mWaitCameraStartUpThread.block();
        }
    }

    /**
     * Close Camera (e.g. switch camera/exit camera).
     * @param isExitAp if true, call native method asynchronously
     *  when exit camera.
     */
    public void closeCamera(boolean isExitAp) {
        Log.d(TAG, "[closeCamera] cameraState:" + getCameraState()
                + ", isExitAp:" + isExitAp);
        if (getCameraState() == CameraState.STATE_CAMERA_CLOSED) {
            return;
        }
        mCameraAppUi.setViewState(ViewState.VIEW_STATE_CAMERA_CLOSED);
        waitCameraStartUpThread(true);
        if (mIsOpenCameraFail) {
            Log.d(TAG, "[closeCamera] mIsOpenCameraFail:" + mIsOpenCameraFail);
            return;
        }
        removeAllMessage();
        mCameraActor.onCameraClose();
        clearDeviceCallbacks();
        detachSurfaceViewLayout();
        CameraHolder.instance().release(isExitAp);
        if (mFocusManager != null) {
            mFocusManager.onCameraReleased();
        }
        mOldTopCameraDevice = mTopCameraDevice;
        mCurCameraDevice = mDummyCameraDevice;
        mTopCameraDevice = mDummyCameraDevice;
        mModuleManager.onCameraCloseDone();
        setCameraState(CameraState.STATE_CAMERA_CLOSED);
    }

    public CameraHolder getCameraHolder() {
        return CameraHolder.instance();
    }
    public void onModeChanged(boolean isNeedRestart) {
        Log.d(TAG, "[onModeChanged] isNeedRestart:" + isNeedRestart + ",camera is opened : "
                + isCameraOpened());
        if (isCameraOpened()) {
            mCameraAppUi.clearViewCallbacks();
            mCameraAppUi.applayViewCallbacks();
            unInitializeFocusManager();
            initializeFocusManager();
            applyDeviceCallbacks();
            applyParameters(isNeedRestart);
        }
    }

    public void onOrientationChanged(int orientation) {
        mOrientation = orientation;
        mCurCameraDevice.setJpegRotation(mOrientation);
        mCurCameraDevice.applyParametersToServer();
        mTopCameraDevice.setJpegRotation(mOrientation);
        mTopCameraDevice.applyParametersToServer();
    }

    public void applyParameters(boolean isNeedRestart) {
//        Log.d(TAG, "[applyParameters] isNeedRestart:" + isNeedRestart);

        final boolean isPreviewSizeChanged =
                isPreviewSizeChanged(mCurCameraDevice.getPreviewSize());
        final boolean isPreviewRatioChanged =
                isPreviewRatioChanged(mCurCameraDevice.getPreviewSize());
        final boolean isZsdChanged = isZsdChanged(mCurCameraDevice.getZsdMode());
        final boolean isPictureSizeChanged = mCurCameraDevice.isPictureSizeChanged();
        final boolean isHdrChanged = mCurCameraDevice.isHdrChanged();
        final boolean isPreviewSizeRestartCase = isPreviewSizeChanged && !mIsSwitchingPip;

        final boolean needRestart =
                isZsdChanged || isPreviewSizeRestartCase || isNeedRestart || isHdrChanged;
        if (needRestart) {
            mCameraActor.stopPreview();
            if (getCameraState() != CameraState.STATE_OPENING_CAMERA) {
                prepareSurfaceView(isPreviewRatioChanged);
            }
        }
        if (getCameraState() == CameraState.STATE_OPENING_CAMERA) {
            if (isPreviewRatioChanged && !mIsFirstOpenCamera  && !mIsWaitForStartUpThread) {
                //switch surface view in Ui thread
                //delay 5ms, be sure pause thread before resume thread
                Message msg = mMainHandler.obtainMessage(MSG_PREPARE_SURFACE_VIEW, true);
                mMainHandler.sendMessageDelayed(msg, 5);
                mCameraStartUpThread.pauseThread();
            }
            mIsFirstOpenCamera = false;
            mMainHandler.sendEmptyMessage(MSG_SET_PREVIEW_ASPECT_RATIO);
        } else {
            setPreviewFrameLayoutAspectRatio();
        }

        if (getCameraState() ==
                CameraState.STATE_OPENING_CAMERA && mCameraStartUpThread.isCancel()) {
//            Log.i(TAG, "[applyParameters] cancel applyParameters after setPreviewFrameLayout");
            return;
        }
        prepareParameter(needRestart, isPreviewRatioChanged, isPreviewSizeChanged);
        mMainHandler.sendEmptyMessageDelayed(MSG_CAMERA_PREVIEW_DONE, getDelayTime());

        if (getCameraState() ==
                CameraState.STATE_OPENING_CAMERA && mCameraStartUpThread.isCancel()) {
//            Log.i(TAG, "[applyParameters] cancel applyParameters after prepareParameter");
            return;
        }

        String dngLastState = mCurCameraDevice.getDngState();
        String dngCurrentState = mISettingCtrl.getSettingValue(SettingConstants.KEY_DNG);
        boolean dngChanged = false;
        if (dngCurrentState != null) {
            dngChanged = true;
        }
        boolean isVideoBitRateChanged = isVideoBitRateChanged();
        boolean isAudioBieRateChanged = isAudioBitRateChanged();
        boolean isRemainingChanged = false;
        if (dngLastState != null) {
            isRemainingChanged = dngChanged && !(dngLastState.equals(dngCurrentState));
        }
        if ((isPictureSizeChanged || isPreviewSizeChanged || isRemainingChanged)
                || isVideoBitRateChanged || isAudioBieRateChanged
                || mCameraActivity.isVideoMode()) {
            // put showRemainingAways() after
            // mCameraActor.onCameraParamtersReady() to avoid read
            // parameters when write.
            // videoMode need always show.
            mCameraAppUi.setDngState(mISettingCtrl.getSettingValue(SettingConstants.KEY_DNG));
            mCameraAppUi.showRemainingAways();
        }
        mCurCameraDevice.updateDngState(mISettingCtrl.getSettingValue(SettingConstants.KEY_DNG));
        mCurCameraDevice.updateParameters();
        mTopCameraDevice.updateParameters();
        mLastPreviewSize = mCurCameraDevice.getPreviewSize();
        mLastZsdMode = mCurCameraDevice.getZsdMode();
    }

    public void applyParameterForFocus(final boolean setArea) {
//        Log.d(TAG, "[applyParameterForFocus] setArea:" + setArea);
        applyFocusCapabilities(setArea);
        // Note: here doesn't fetch parameters from server
        // set the focus mode to server
        mCurCameraDevice.applyParametersToServer();
        // add for pip, set top graphic's focus mode to auto
        // Parameters topParameters = mTopCamParameters;
        if (mTopCameraDevice.isSupportFocusMode(Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
            mTopCameraDevice.setFocusMode(Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        }
        // Note: here doesn't fetch parameters from server
        // set the focus mode to server
        mTopCameraDevice.applyParametersToServer();
    }

    public void applyParameterForCapture(final SaveRequest request) {
//        Log.d(TAG, "[applyParameterForCapture] request:" + request);

        // Set rotation and gps data. for picture taken
        mCurCameraDevice.setJpegRotation(mOrientation);
        int jpegRotation = mCurCameraDevice.getJpegRotation();
        request.setJpegRotation(jpegRotation);
        mCurCameraDevice
                .setGpsParameters(mCameraActivity.getLocationManager().getCurrentLocation());
        mCurCameraDevice.setCapturePath(request.getTempFilePath());
        // Note: here doesn't fetch parameters from server
        mCurCameraDevice.applyParametersToServer();

        // Set rotation and gps data. for picture taken
        mTopCameraDevice.setJpegRotation(mOrientation);
        mTopCameraDevice.applyParametersToServer();
    }

    public void startAsyncZoom(final int zoomValue) {
//        Log.d(TAG, "[startAsyncZoom] zoomValue" + zoomValue);
        // Set zoom parameters asynchronously
        if (mCurCameraDevice.isZoomSupported() && mCurCameraDevice.getZoom() != zoomValue) {
            // should set zoom value to mParameters
            mCurCameraDevice.setZoom(zoomValue);
        }
    }

    public void attachSurfaceViewLayout() {
//        Log.d(TAG, "[attachSurfaceViewLayout]" +
//                " begin mCurSurfaceViewLayout = " + mCurSurfaceViewLayout);
        if (mSurfaceView == null) {
            createSurfaceView();
            mSurfaceView.setVisibility(View.VISIBLE);
        }
    }

    public void detachSurfaceViewLayout() {
//        Log.d(TAG, "[detachSurfaceViewLayout] mLastSurfaceViewLayout = "
//              + mLastSurfaceViewLayout);
        if (mLastSurfaceViewLayout != null) {
            FrameLayout surfaceViewRoot =
                    (FrameLayout) mCameraActivity.findViewById(R.id.camera_surfaceview_root);
            surfaceViewRoot.removeViewInLayout(mLastSurfaceViewLayout);
            mLastSurfaceViewLayout.setVisibility(View.GONE);
            mLastSurfaceViewLayout = null;
        }
    }

    public void setPreviewFrameLayoutAspectRatio() {
        // Set the preview frame aspect ratio according to the picture size.
        PreviewFrameLayout previewFrameLayout = (PreviewFrameLayout) mCameraActivity
                .findViewById(R.id.frame);
        if (previewFrameLayout != null && mCurCameraDevice != null) {
            int width = 1;
            int height = 1;
            // any case, we should get the preview size set to native
            Size size = mCurCameraDevice.getPreviewSize();
            if (size == null) {
                return;
            }
            width = size.width;
            height = size.height;
            if (mSurfaceView != null) {
                boolean layoutWillChange = mSurfaceView.setAspectRatio((double) width / height);
                if (!layoutWillChange) {
                    mModuleManager.notifySurfaceViewDisplayIsReady();
                }
            }
            previewFrameLayout.setAspectRatio((double) width / height);
//            Log.i(TAG, "setPreviewFrameLayoutAspectRatio() width="
//                    + width + ", height=" + height);
        }
    }

    public Parameters getParameters() { // not recommended
        return mCurCameraDevice.getParameters();
    }

    public ParametersExt getParametersExt() {
        return mCurCameraDevice.getParametersExt();
    }

    public int getCameraId() {
        int id = mCurCameraDevice.getCameraId();
        return id == mDummyCameraDevice.getCameraId() ? mCameraStartUpThread.getCameraId() : id;
    }

    public int getTopCameraId() {
        return mTopCameraDevice.getCameraId();
    }

    public CameraManager.CameraProxy getCameraDevice() {
        return mCurCameraDevice.getCameraDevice();
    }

    public ICameraDeviceExt getCurCameraDevice() {
        return mCurCameraDevice;
    }

    public ICameraDeviceExt getTopCameraDevice() {
        return mTopCameraDevice;
    }

    public FocusManager getFocusManager() {
        return mFocusManager;
    }

    public PreviewSurfaceView getSurfaceView() {
        return mSurfaceView;
    }

    public boolean isFirstStartUp() {
        return mIsFirstStartUp;
    }
    public int getDisplayOrientation() {
        return mCurCameraDevice.getDisplayOrientation();
    }

    public Parameters getTopParameters() {
//        Log.d(TAG, "getTopCamParameters()");
        return mTopCameraDevice.getParameters();
    }

    public void onSizeChanged(int width, int height) {
        if (mCurCameraDevice != null && mCurCameraDevice.getPreviewSize() != null) {
            Size size = mCurCameraDevice.getPreviewSize();
            int w = size.width;
            int h = size.height;
            double mAspectRatio = (double) w / h;
            if (width > height) {
                mUnCropWidth = Math.max(width, (int) (height * mAspectRatio));
                mUnCropHeight = Math.max(height, (int) (width / mAspectRatio));
            } else {
                mUnCropWidth = Math.max(width, (int) (height / mAspectRatio));
                mUnCropHeight = Math.max(height, (int) (width * mAspectRatio));
            }
        } else {
            // 4:3 mode,parameter is null, then will invoke the following code
            mUnCropWidth = width;
            mUnCropHeight = height;
        }

        if (mFocusManager != null) {
            mFocusManager.setPreviewSize(mUnCropWidth, mUnCropHeight);
            mFocusManager.setCropPreviewSize(width, height);
        }

        mPreviewFrameWidth = width;
        mPreviewFrameHeight = height;

        if (mModuleManager != null) {
            mModuleManager.onPreviewDisplaySizeChanged(width, height);
        }

//        Log.i(TAG, "onSizeChanged() mPreviewFrameWidth = " + mPreviewFrameWidth
//                + ", mPreviewFrameHeight=" + mPreviewFrameHeight
//                + ", mCropWidth = " + mUnCropWidth
//                + ", mCropHeight = " + mUnCropHeight + ", width = " + width + ", height = "
//                + height);
    }

    public void setDisplayOrientation() {
        mCurCameraDevice.setDisplayOrientation(mModuleManager.isDeviceUseSurfaceView());
        int displayOrientation = mCurCameraDevice.getDisplayOrientation();

//        Log.d(TAG, "[setDisplayOrientation] displayOrientation:" + displayOrientation);

        FrameView frameView = mCameraActivity.getFrameView();
        if (frameView != null) {
            frameView.setDisplayOrientation(displayOrientation);
        }
        if (mFocusManager != null) {
            mFocusManager.setDisplayOrientation(displayOrientation);
        }
        int displayRotation = Util.getDisplayRotation(mCameraActivity);
        mModuleManager.setDisplayRotation(displayRotation);
    }

    public int getPreviewFrameHeight() {
        return mPreviewFrameHeight;
    }

    public int getPreviewFrameWidth() {
        return mPreviewFrameWidth;

    }

    public int getUnCropWidth() {
        return mUnCropWidth;
    }

    public int getUnCropHeight() {
        return mUnCropHeight;
    }

    public boolean isOpenCameraFail() {
        return mIsOpenCameraFail;
    }

    public void hideRootCover() {
//        Log.d(TAG, "[hideRootCover]");
        mSurfaceViewCover = mCameraActivity.findViewById(R.id.camera_cover);
        if (mSurfaceViewCover != null &&
            mSurfaceViewCover.getVisibility() != View.INVISIBLE) {
            mSurfaceViewCover.setVisibility(View.INVISIBLE);
        }
    }

    private int getPreferredCameraId(ComboPreferences preferences) {
        int intentCameraId = Util.getCameraFacingIntentExtras(mCameraActivity);
//        Log.i(TAG, "[getPreferredCameraId] intentCameraId = " + intentCameraId);
        if (intentCameraId != UNKNOWN) {
            // Testing purpose. Launch a specific camera through the intent
            // extras.
            return intentCameraId;
        } else {
            return SettingUtils.readPreferredCameraId(preferences);
        }
    }

    private void initializeFocusManager() {
        // Create FocusManager object. startPreview needs it.
//        Log.d(TAG, "[initializeFocusManager]...");
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_INIT_FOCUS_MGR,
                CameraPerformanceTracker.ISBEGIN);
        mFocusAreaIndicator = (RotateLayout) mCameraActivity
                .findViewById(R.id.focus_indicator_rotate_layout);
        int cameraId = mCurCameraDevice.getCameraId();
        CameraInfo info = CameraHolder.instance().getCameraInfo()[cameraId];
        boolean mirror = (info.facing == CameraInfo.CAMERA_FACING_FRONT);
        mFocusManager = new FocusManager(mCameraActivity, mPreferences, mFocusAreaIndicator,
                mCurCameraDevice.getInitialParams(), mCameraActor.getFocusManagerListener(),
                mirror, mCameraActivity.getMainLooper(), mCameraActor.getMode());

        mFocusManager.setPreviewSize(mUnCropWidth, mUnCropHeight);
        mFocusManager.setCropPreviewSize(mPreviewFrameWidth, mPreviewFrameHeight);
        mFocusManager.setDisplayOrientation(getDisplayOrientation());
        // Apply focus capabilities when mode change
        applyFocusCapabilities(true);
        IFocusManager focusManager = new FocusManagerImpl(mFocusManager);
        mModuleManager.setFocusManager(focusManager);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_INIT_FOCUS_MGR,
                CameraPerformanceTracker.ISEND);
    }

    private void initializeSettingController() {
//        Log.d(TAG, "[initializeSettingController]");
        if (!mISettingCtrl.isSettingsInitialized()) {
            mISettingCtrl.initializeSettings(R.xml.camera_preferences, mPreferences.getGlobal(),
                    mPreferences.getLocal());
        }
        mISettingCtrl.updateSetting(mPreferences.getLocal());
        // Workaround for hdr value is on in pip video mode when pause and resume camera
        // again in pip video mode.
        if (mCameraActor.getMode() == ModePicker.MODE_VIDEO_PIP
                && mISettingCtrl.getSetting(SettingConstants.KEY_HDR) != null) {
            mISettingCtrl.onSettingChanged(SettingConstants.KEY_HDR, "off");
        }
        mMainHandler.sendEmptyMessage(MSG_CAMERA_PREFERENCE_READY);
    }

    private boolean isPreviewSizeChanged(Size curPreviewSize) {
        Assert.assertNotNull(curPreviewSize);
        boolean isChanged = false;
        // first time mLastPreviewSize is null, default should return false;
        if (mLastPreviewSize != null && !mLastPreviewSize.equals(curPreviewSize)) {
            isChanged = true;
        }
//        Log.d(TAG, "[isPreviewSizeChanged] isChanged" + isChanged);
        return isChanged;
    }

    private boolean isPreviewRatioChanged(Size curPreviewSize) {
        Assert.assertNotNull(curPreviewSize);
        boolean isChanged = false;
        if (mLastPreviewSize != null) {
            isChanged = !(((double) curPreviewSize.width / curPreviewSize.height) ==
                    ((double) mLastPreviewSize.width / mLastPreviewSize.height));
        }
//        Log.d(TAG, "[isPreviewRatioChanged] isChanged:" + isChanged);
        return isChanged;
    }

    private boolean isZsdChanged(String curZsd) {
        boolean isChanged = false;
        isChanged = curZsd == null ? mLastZsdMode != null : !curZsd.equals(mLastZsdMode);
//        Log.d(TAG, "[isZsdChanged] zsd:" + curZsd + ",oldzsd:" + mLastZsdMode);
        return isChanged;
    }

    private boolean isVideoBitRateChanged() {
        boolean isChanged = false;
        if (mProfile != null) {
            isChanged = (mLastVideoBitRate == UNKNOWN ? true
                    : mLastVideoBitRate != mProfile.videoBitRate);
            mLastVideoBitRate = mProfile.videoBitRate;
        }
//        Log.d(TAG, "[isVideoBitRateChanged] isChanged:" + isChanged);
        return isChanged;
    }

    private boolean isAudioBitRateChanged() {
        boolean isChanged = false;
        if (mProfile != null) {
            isChanged = (mLastAudioBitRate == UNKNOWN ? true
                    : mLastAudioBitRate != mProfile.audioBitRate);
            mLastAudioBitRate = mProfile.audioBitRate;
        }
//        Log.d(TAG, "[isAudioBitRateChanged] isChanged:" + isChanged);
        return isChanged;
    }

    private void prepareSurfaceView(boolean isPreviewRatioChanged) {
//        Log.d(TAG, "[prepareSurfaceView] isPreviewRatioChanged:" + isPreviewRatioChanged);
        if (isPreviewRatioChanged && mSurfaceView != null
                && mSurfaceView.getVisibility() == View.VISIBLE) {
            mSurfaceView = null;
            detachSurfaceViewLayout();
            attachSurfaceViewLayout();
        }
    }

    private void prepareParameter(boolean needRestart, boolean isPreviewRatioChanged,
            final boolean isPreviewSizeChanged) {
        final boolean isDeviceUseSurfaceView = mModuleManager.isDeviceUseSurfaceView();
        final boolean isDisplayUseSurfaceView = mModuleManager.isDisplayUseSurfaceView();
        final boolean isNeedDualCamera = mModuleManager.isNeedDualCamera();
        final boolean isStereoMode = mCameraActivity.getCurrentMode()
                == ModePicker.MODE_STEREO_CAMERA;
//        Log.d(TAG, "[prepareParameter] needRestart:" + needRestart + ",isPreviewRatioChanged:"
//                + isPreviewRatioChanged + ",isPreviewSizeChanged:" + isPreviewSizeChanged
//                + ",isDeviceUseSurfaceView:" + isDeviceUseSurfaceView
//                + ",isDisplayUseSurfaceView:"
//                + isDisplayUseSurfaceView + ",isNeedDualeCamera:" + isNeedDualCamera);

        if (needRestart) {
            switchPreview(isDeviceUseSurfaceView, isDisplayUseSurfaceView, isNeedDualCamera);
        }

        //Must reset the photo parameters when back to video mode.
        //because maybe the parameters is wrong when multiThread issue.
        if (ModePicker.MODE_PHOTO == mCameraActor.getMode()) {
            mCurCameraDevice.setPhotoModeBasicParameters();
        }

        mCurCameraDevice.setDisplayOrientation(isDeviceUseSurfaceView);
        mTopCameraDevice.setDisplayOrientation(isDeviceUseSurfaceView);

        mCurCameraDevice.setJpegRotation(mOrientation);

        mCameraAppUi.setZoomParameter();
        mCurCameraDevice.setPreviewFormat(ImageFormat.YV12);
        // native will change the parameters to default values if scene mode change
        // but the parameters of default maybe not ap want
        // so should call twice applyParametersToServer
        if (mCurCameraDevice.isSceneModeChanged()) {
            mCurCameraDevice.applyParametersToServer();
        }
        mCurCameraDevice.applyParametersToServer();
        if (isPreviewSizeChanged) {
            mCurCameraDevice.fetchParametersFromServer();
        }

        mTopCameraDevice.setJpegRotation(mOrientation);
        mTopCameraDevice.applyParametersToServer();
        if (isPreviewSizeChanged) {
            mTopCameraDevice.fetchParametersFromServer();
        }

        mCameraActor.onCameraParameterReady(needRestart);
        mCurCameraDevice.setOneShotPreviewCallback(mOneShotPreviewCallback);
        mMainHandler.sendEmptyMessage(MSG_CAMERA_PARAMETERS_READY);
    }

    private void turnOnWhenShown() {
//        Log.d(TAG, "[turnOnWhenShown] mCurCameraDevice:" + mCurCameraDevice + ",mFocusManager:"
//                + mFocusManager + ",mIsNeedResetFocus:" + mIsNeedResetFocus);
        // may open camera slow, so parameters may be null.
        if (mCurCameraDevice != mDummyCameraDevice && mFocusManager != null) {
            // when slip back to Camera from Gallery, start continous auto
            // focus.
            if (mIsNeedResetFocus) {
                mFocusManager.overrideFocusMode(null);
                mCurCameraDevice.setFocusMode(mFocusManager.getFocusMode());

            }
            String currentFlashMode = mISettingCtrl.getSettingValue(SettingConstants.KEY_FLASH);
            if (currentFlashMode != null) {
                mCurCameraDevice.getParametersExt().setFlashMode(currentFlashMode);
            }
        }
    }

    private void switchCameraPreview() {
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_SET_PREVIEW_DISP,
                CameraPerformanceTracker.ISBEGIN);
        setSurfaceViewStatusOnUiThread(View.VISIBLE);
        mCurCameraDevice.setPreviewDisplayAsync(mSurfaceView.getHolder());
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_SET_PREVIEW_DISP,
                CameraPerformanceTracker.ISEND);
    }

    private void setSurfaceViewStatusOnUiThread(final int status) {
        mCameraActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                setSurfaceViewVisible(status);
            }
        });
    }

    private void switchPreview(boolean isDeviceUseSurfaceView, boolean isDisplayUseSurfaceView,
            boolean isNeedDualeCamera) {
//        Log.d(TAG, "switchPreview");
        Size size = mCurCameraDevice.getPreviewSize();
        updatePreviewBufferSize(size);
        // SurfaceTexture --> SurfaceView
        if (isDeviceUseSurfaceView) {
            setSurfaceViewStatusOnUiThread(View.VISIBLE);
            mCurCameraDevice.setPreviewDisplayAsync(mSurfaceView.getHolder());
        } else {
            // SurfaceView --> SurfaceTexture
            getSurfaceTexture(isDisplayUseSurfaceView, isNeedDualeCamera);
            if (!isDisplayUseSurfaceView) {
                setSurfaceViewStatusOnUiThread(View.INVISIBLE);
            } else {
                setSurfaceViewStatusOnUiThread(View.VISIBLE);
                setPreviewTextureAsync();
            }
        }
    }

    private void updatePreviewBufferSize(Size size) {
//        Log.d(TAG, "updatePreviewBufferSize");
        int cameraDisplayOrientation = mCurCameraDevice.getCameraDisplayOrientation();
        if (size != null) {
            int width = size.width;
            int height = size.height;
            if (cameraDisplayOrientation % 180 != 0) {
                int tmp = width;
                width = height;
                height = tmp;
            }
            if (mModuleManager != null) {
                mModuleManager.onPreviewBufferSizeChanged(width, height);
            }
        }
    }

    private void getSurfaceTexture(boolean isDisplayUseSurfaceView, boolean isDualCamera) {
//        Log.d(TAG, "[getSurfaceTexture] isDisplayUseSurfaceView:" + isDisplayUseSurfaceView
//                + ",isDualCamera:" + isDualCamera);
        // initialize surface texture
        if (isDisplayUseSurfaceView) {
            mSurfaceTexture = mModuleManager.getBottomSurfaceTexture();
        }
        mTopCamSurfaceTexture = isDualCamera ? mModuleManager.getTopSurfaceTexture() : null;
    }

    private void setPreviewTextureAsync() {
//        Log.d(TAG, "setPreviewTextureAsync() mSurfaceTextureReady=" + mIsSurfaceTextureReady
//                + ", mSurfaceTexture=" + mSurfaceTexture);
        if (mSurfaceTexture != null && mIsSurfaceTextureReady) {
            mCurCameraDevice.setPreviewTextureAsync(mSurfaceTexture);

            mCameraActor.setSurfaceTextureReady(true);
        }
        if (mTopCamSurfaceTexture != null && mIsSurfaceTextureReady) {
            mTopCameraDevice.setPreviewTextureAsync(mTopCamSurfaceTexture);
        }
    }

    private void setSurfaceViewVisible(final int visibility) {
//        Log.d(TAG, "setSurfaceViewVisible visibility = " + visibility);
        if (mSurfaceView == null || (mSurfaceView.getVisibility() == visibility)) {
            return;
        }
        mSurfaceView.setVisibility(visibility);
    }

    private void createSurfaceView() {
        FrameLayout surfaceViewRoot =
                (FrameLayout) mCameraActivity.findViewById(R.id.camera_surfaceview_root);
        mLastSurfaceViewLayout = mCurSurfaceViewLayout;
        mCurSurfaceViewLayout =
                (FrameLayout) mCameraActivity.getLayoutInflater().inflate(
                        R.layout.camera_preview_layout, null);
        mSurfaceView =
                (PreviewSurfaceView) mCurSurfaceViewLayout.findViewById(
                        R.id.camera_preview);
        mSurfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                mCameraActivity.getGestureRecognizer().onTouchEvent(event);
                return true;
            }
        });
        SurfaceHolder surfaceHolder = mSurfaceView.getHolder();
        surfaceHolder.addCallback(this);
        surfaceViewRoot.addView(mCurSurfaceViewLayout);
    }

    public void unInitializeFocusManager() {
        if (mFocusManager != null) {
            mFocusManager.removeMessages();
            mCameraActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mFocusManager.clearFocusAndFaceUi();
                }
            });
            mFocusManager.release();
        }
    }

    private class MainHandler extends Handler {
        MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[MainHandler.handleMessage] msg:" + msg.what);
            switch (msg.what) {
            case MSG_OPEN_CAMERA_FAIL:
                mCameraActor.onCameraOpenFailed();
                Util.showErrorAndFinish(mCameraActivity, R.string.cannot_connect_camera_new);
                mCameraActivity.onCameraOpenFailed();
                break;
            case MSG_OPEN_CAMERA_DISABLED:
                mCameraActor.onCameraDisabled();
                Util.showErrorAndFinish(mCameraActivity, R.string.camera_disabled);
                mCameraActivity.onCameraOpenFailed();
                break;

            case MSG_CAMERA_PREFERENCE_READY:
                mCameraActivity.onCameraPreferenceReady();
                break;

            case MSG_CAMERA_PARAMETERS_READY:
                mCameraActivity.onCameraParametersReady();
                break;

            case MSG_CAMERA_PREVIEW_DONE:
                hideRootCover();
                break;
            case MSG_REMOVE_PREVIEW_COVER:
                hideRootCover();
                break;

            case MSG_CAMERA_OPEN_DONE:
                mCameraAppUi.refreshModeRelated();
                mCameraAppUi.setViewState(ViewState.VIEW_STATE_CAMERA_OPENED);
                mCameraActivity.onCameraOpenDone();
                break;

            case MSG_SET_PREVIEW_ASPECT_RATIO:
                setPreviewFrameLayoutAspectRatio();
                break;

            case MSG_PREPARE_SURFACE_VIEW:
                prepareSurfaceView((Boolean) msg.obj);
                mCameraStartUpThread.resumeThread();
                break;
            default:
                break;
            }
        }
    }

    private void showRootCover() {
//        Log.d(TAG, "[showRootCover]");
        if (mSurfaceViewCover != null && mSurfaceViewCover.getVisibility() != View.VISIBLE) {
            mSurfaceViewCover.setVisibility(View.VISIBLE);
        }
    }
    private void releaseRootCover() {
//        Log.d(TAG, "[releaseRootCover]");
        if (mSurfaceViewCover != null && mSurfaceViewCover.getVisibility() != View.GONE) {
            mSurfaceViewCover.setVisibility(View.GONE);
        }
    }

    // private Parameters mParameters;
    private void applyFocusCapabilities(boolean setArea) {
        FocusManager focusManager = mFocusManager;
        if (focusManager.getAeLockSupported()) {
            mCurCameraDevice.setAutoExposureLock(focusManager.getAeLock());
        }
        if (focusManager.getAwbLockSupported()) {
            mCurCameraDevice.setAutoWhiteBalanceLock(focusManager.getAwbLock());
        }
        if (focusManager.getFocusAreaSupported() && setArea) {
            mCurCameraDevice.setFocusAreas(focusManager.getFocusAreas());
        }
        if (focusManager.getMeteringAreaSupported() && setArea) {
            // Use the same area for focus and metering.
           mCurCameraDevice.setMeteringAreas(focusManager.getMeteringAreas());
        }

        mCameraActor.handleFocus();
        mCurCameraDevice.setFocusMode(focusManager.getFocusMode());
    }


    public void waitCameraStartUpThread(boolean cancel) {
        //Log.i(TAG, "waitCameraStartUpThread(" + cancel + ") begin mCameraStartUpThread="
        //        + mCameraStartUpThread + ",mCameraState:" + mCameraState);
        if (getCameraState() != CameraState.STATE_OPENING_CAMERA) {
            return;
        }
        mIsWaitForStartUpThread = true;
        mWaitCameraStartUpThread.close();
        mCameraStartUpThread.resumeThread();
        if (cancel) {
            mSycForLaunch.open();
            mCameraStartUpThread.cancel();
        }
        if (mIsWaitForStartUpThread) {
            mWaitCameraStartUpThread.block(TIME_WAIT_STARTUP_THREAD);
        }
        Log.d(TAG, "waitCameraStartUpThread() end");
    }

    private void cameraStartUpThreadDone() {
//        Log.d(TAG, "[cameraStartUpThreadDone]");
        mIsWaitForStartUpThread = false;
        mWaitCameraStartUpThread.open();
    }

    private void applyDeviceCallbacks() { // should be checked
        mCurCameraDevice.setErrorCallback(mCameraActor.getErrorCallback());
        mCurCameraDevice.setFaceDetectionListener(mCameraActor.getFaceDetectionListener());
    }

    private void clearDeviceCallbacks() {
        mCurCameraDevice.setErrorCallback(null);
        mCurCameraDevice.setFaceDetectionListener(null);
    }

    private void removeAllMessage() {
//        Log.d(TAG, "[removeAllMessage]");
        mMainHandler.removeMessages(MSG_CAMERA_PREFERENCE_READY);
        mMainHandler.removeMessages(MSG_CAMERA_PREVIEW_DONE);
        mMainHandler.removeMessages(MSG_CAMERA_OPEN_DONE);
        mMainHandler.removeMessages(MSG_SET_PREVIEW_ASPECT_RATIO);
        mMainHandler.removeMessages(MSG_CAMERA_PARAMETERS_READY);
        mMainHandler.removeMessages(MSG_PREPARE_SURFACE_VIEW);
    }

    private int getDelayTime() {
        // 10ms is a workaround for first preview frame arrives
        //TODO when VFB ->Video or video ->VFB
        // 0ms for UI issue when preview size changed
        int delayTime = 0;
        if ("on".equals(mISettingCtrl.getSetting(SettingConstants.KEY_VIDEO_EIS))) {
            delayTime = 20;
        }
        if ((ModePicker.MODE_FACE_BEAUTY == mCameraActivity.getPrevMode() || "on"
                .equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_HDR)))
                && ModePicker.MODE_VIDEO == mCameraActivity.getCurrentMode()) {
            // this just is a workaround method,if video can get the first frame
            // arrived,will delete this
            delayTime = 30;
        }
//        Log.d(TAG, "[getDelayTime]delaytime = " + delayTime);
        return delayTime;
    }

    private class CameraHandler extends Handler {
        CameraHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(final Message msg) {
//            Log.d(TAG, "handleMessage msg.what = " + msg.what);
            switch (msg.what) {
            case MSG_SET_SURFACE:
                CameraPerformanceTracker.onEvent(TAG,
                        CameraPerformanceTracker.NAME_SET_PREVIEW_DISP,
                        CameraPerformanceTracker.ISBEGIN);
                mCurCameraDevice.setPreviewDisplayAsync(mSurfaceView
                        .getHolder());
                CameraPerformanceTracker.onEvent(TAG,
                        CameraPerformanceTracker.NAME_SET_PREVIEW_DISP,
                        CameraPerformanceTracker.ISEND);
                mSycForLaunch.open();
            }
        }
    }
    private class CameraStartUpThread extends Thread {

        private volatile boolean mOpenCamera = false;
        private volatile boolean mIsActive = true;

        private ConditionVariable mConditionVariable = new ConditionVariable();
        private CameraManager.CameraProxy mTopCamDevice;
        private CameraManager.CameraProxy mCameraDevice;
        private Parameters mParameters;
        private Parameters mTopCamParameters;

        private int mTopCamId;
        private int mCameraId;

        private boolean mCancel = false;

        public CameraStartUpThread() {
            mCameraId = getPreferredCameraId(mPreferences);
            mPreferences.setLocalId(mCameraActivity, mCameraId);
            SettingUtils.upgradeLocalPreferences(mPreferences.getLocal());
            SettingUtils.writePreferredCameraId(mPreferences, mCameraId);
        }

        @Override
        public void run() {
            while (mIsActive) {
                synchronized (this) {
                    if (!mOpenCamera) {
                        cameraStartUpThreadDone();
                        waitWithoutInterrupt(this);
                        continue;
                    }
                }
                mOpenCamera = false;
                if (mIsFirstStartUp) {
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_START_UP,
                            CameraPerformanceTracker.ISBEGIN);
                    int openResult = firstOpenCamera();
                    if (CAMERA_OPEN_SUCEESS != openResult) {
                        setCameraState(CameraState.STATE_CAMERA_CLOSED);
                        mIsFirstStartUp = false;
                        continue;
                    }
                    ModeChecker.updateModeMatrix(mCameraActivity, mCameraId);
                    mCurCameraDevice.setDisplayOrientation(true);
                    mCurCameraDevice.setPreviewSize();
                    // if open camera too quickly, here may be run with
                    // setCameraActor at the same time, this will make CameraStartUpThread
                    // pause forever, so we synchronize it for workaround.
                    synchronized (mCameraActorSync) {
                        if (mCameraActor == null) {
                            try {
                                mCameraActorSync.wait();
                            } catch (InterruptedException e) {
                                Log.e(TAG, "mCameraActorSync.wait with InterruptedException");
                            }
                        }
                    }
                    mCameraActor.onCameraOpenDone();
                    mModuleManager.onCameraOpen();
                    initializeFocusManager();
                    if (mCameraActivity.getCurrentMode() == ModePicker.MODE_PHOTO) {
                        mCurCameraDevice.setPhotoModeParameters(
                                mCameraActivity.isNonePickIntent());
                    }
                    if (mCancel) {
                        Log.d(TAG, "[mIsFirstStartUp.run] cancel after openCamera");
                        mIsFirstStartUp = false;
                        continue;
                    }

                    if (mCameraActivity.isVideoCaptureIntent()) {
                        initializeSettingController();
                        mModuleManager.setModeSettingValue(
                                mCameraActor.getCameraModeType(mCameraActor.getMode()), "on");
                    }
                    applyFirstParameters();
                    //Block startUp thread when not set surface to native
                    mSycForLaunch.block(500);
                    mCurCameraDevice.setOneShotPreviewCallback(mOneShotPreviewCallback);
                    mMainHandler.sendEmptyMessage(MSG_CAMERA_PARAMETERS_READY);
                    mMainHandler.sendEmptyMessage(MSG_CAMERA_PREVIEW_DONE);
                    mMainHandler.sendEmptyMessage(MSG_CAMERA_OPEN_DONE);
                    Storage.mkFileDir(Storage.getFileDirectory());
                    clearDeviceCallbacks();
                    applyDeviceCallbacks();
                    mCameraAppUi.clearViewCallbacks();
                    mCameraAppUi.applayViewCallbacks();
                    if (mCancel) {
                        Log.d(TAG, "[mIsFirstStartUp.run]" +
                                " cancel before initializeSettingController");
                        mIsFirstStartUp = false;
                        continue;
                    }
                    if (!mCameraActivity.isVideoCaptureIntent()) {
                        initializeSettingController();
                        mModuleManager.setModeSettingValue(
                                mCameraActor.getCameraModeType(mCameraActor.getMode()), "on");
                    }
                    if (mCancel) {
                        Log.d(TAG, "[mIsFirstStartUp.run] cancel before applySecondParameters");
                        mIsFirstStartUp = false;
                        continue;
                    }
                    applySecondParameters();
                    setCameraState(CameraState.STATE_CAMERA_OPENED);
                    mIsFirstStartUp = false;
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_START_UP,
                            CameraPerformanceTracker.ISEND);
                    continue;
                }
                int result = openCamera(mModuleManager.isNeedDualCamera());
                if (CAMERA_OPEN_SUCEESS != result) {
                    setCameraState(CameraState.STATE_CAMERA_CLOSED);
                    continue;
                }
                if (mCancel) {
                    Log.d(TAG, "[CameraStartUpThread.run] cancel after openCamera");
                    continue;
                }
                ModeChecker.updateModeMatrix(mCameraActivity, mCameraId);
                unInitializeFocusManager();
                initializeFocusManager();
                setDisplayOrientation();
                mCurCameraDevice.setPreviewSize();
                if (mCancel) {
                    Log.d(TAG, "[CameraStartUpThread.run] cancel after focusManager");
                    continue;
                }
                if (mCameraActivity.getCurrentMode() == ModePicker.MODE_PHOTO) {
                    mCurCameraDevice.setPhotoModeParameters(mCameraActivity.isNonePickIntent());
                } else if (mCameraActivity.getCurrentMode() == ModePicker.MODE_VIDEO &&
                                              !mCameraActivity.isVideoCaptureIntent()) {
                    mCurCameraDevice.getParametersExt().setCameraMode(
                            ParametersExt.CAMERA_MODE_MTK_VDO);
                }
                clearDeviceCallbacks();
                applyDeviceCallbacks();
                initializeSettingController();
                if (mCancel) {
                    Log.d(TAG, "[CameraStartUpThread.run] cancel after settingCtrl");
                    continue;
                }
                mCameraAppUi.clearViewCallbacks();
                mCameraAppUi.applayViewCallbacks();
                // TODO
                if (mCameraActivity.isVideoCaptureIntent()
                        || (mCameraActor.getMode() != ModePicker.MODE_VIDEO
                        && mCameraActor.getMode() != ModePicker.MODE_VIDEO_PIP)) {
                    mModuleManager.setModeSettingValue(
                            mCameraActor.getCameraModeType(mCameraActor.getMode()), "on");
                }

                if (mCancel) {
                    Log.d(TAG, "[CameraStartUpThread.run] cancel after set setting value");
                    continue;
                }
                applyParameters(true);
                if (mCancel) {
                    Log.d(TAG, "[CameraStartUpThread.run] cancel after applyParameters");
                    continue;
                }
                setCameraState(CameraState.STATE_CAMERA_OPENED);
                mMainHandler.sendEmptyMessage(MSG_CAMERA_OPEN_DONE);
                Storage.mkFileDir(Storage.getFileDirectory());
            }
        }

        public void pauseThread() {
            Log.d(TAG, "pause CameraStartUpThread");
            mConditionVariable.close();
            mConditionVariable.block();
        }

        public void resumeThread() {
            Log.d(TAG, "resume CameraStartUpThread");
            mConditionVariable.open();
        }

        public void setCameraId(int cameraId) {
            mCameraId = cameraId;
        }

        public int getCameraId() {
            return mCameraId;
        }

        public void cancel() {
            mCancel = true;
        }

        public boolean isCancel() {
            return mCancel;
        }

        public synchronized void openCamera() {
            mOpenCamera = true;
            mCancel = false;
            notifyAll();
        }

        public synchronized void terminate() {
            mIsActive = false;
            notifyAll();
        }

        private int openCamera(boolean isDualCamera) {
//            Log.d(TAG, "[run.openCamera] isDualCamera:" + isDualCamera
//                  + "mCameraId:" + mCameraId);
            int cameraId = getPreferredCameraId(mPreferences);
            if (mCameraId != cameraId) {
                SettingUtils.writePreferredCameraId(mPreferences, mCameraId);
            }
            try {
                // TODO will delete this
                if (mCameraActivity.isNeedOpenStereoCamera() && sCameraSetPropertyMethod != null) {
                    String ROPERTY_KEY_CLIENT_APPMODE = "vendor.client.appmode";
                    String APP_MODE_NAME_MTK_DUAL_CAMERA = "MtkStereo";
                    ReflectUtil.callMethodOnObject(null, sCameraSetPropertyMethod,
                            ROPERTY_KEY_CLIENT_APPMODE, APP_MODE_NAME_MTK_DUAL_CAMERA);
                }
                Util.openCamera(mCameraActivity, isDualCamera, mCameraId);
                mCameraDevice = CameraHolder.instance().getCameraProxy(mCameraId);
                mTopCamId = (mCameraId == CameraHolder.instance().getBackCameraId()) ? CameraHolder
                        .instance().getFrontCameraId() : CameraHolder.instance().getBackCameraId();
                mTopCamDevice = CameraHolder.instance().getCameraProxy(mTopCamId);
            } catch (CameraHardwareException e) {
                Log.e(TAG, "[run.openCamera]CameraHardwareException e:" + e);
                mIsOpenCameraFail = true;
                mMainHandler.sendEmptyMessage(MSG_OPEN_CAMERA_FAIL);
                return CAMERA_HARDWARE_EXCEPTION;
            } catch (CameraDisabledException e) {
                Log.e(TAG, "[runopenCamera]CameraDisabledException e:" + e);
                mIsOpenCameraFail = true;
                mMainHandler.sendEmptyMessage(MSG_OPEN_CAMERA_DISABLED);
                return CAMERA_DISABLED_EXCEPTION;
            }
            mParameters = (mCameraDevice == null) ? null : CameraHolder.instance()
                    .getOriginalParameters(mCameraId);
            mTopCamParameters = (mTopCamDevice == null) ? null : CameraHolder.instance()
                    .getOriginalParameters(mTopCamId);
            mCameraActor.onCameraOpenDone();

            if (mCameraDevice != null && mParameters != null) {
                mCurCameraDevice = new CameraDeviceExt(mCameraActivity, mCameraDevice, mParameters,
                        mCameraId, mPreferences);
                String defaultZsdValue = mCurCameraDevice.getParametersExt().getZSDMode();
//                Log.d(TAG, "[run.openCamera], Get default ZSD value " +
//                        "from parameters, value:" + defaultZsdValue);
                SettingUtils.setZsdDefaultValue(defaultZsdValue);
                String defaultAntiBandingValue = mCurCameraDevice
                        .getParametersExt().getAntibanding();
                SettingUtils.setAntiBandingDefaultValue(defaultAntiBandingValue);
            } else {
                Log.d(TAG, "[openCamera fail],mCameraDevice:" + mCameraDevice + ",mParameters:"
                        + mParameters);
            }
            if (mTopCamDevice != null && mTopCamParameters != null) {
                //record parameter when pause activity
                //restore parameter when resume activity
                if (mOldTopCameraDevice.getCameraId() == mTopCamId) {
                    mTopCamParameters = mOldTopCameraDevice.getParameters();
                    mOldTopCameraDevice = mDummyCameraDevice;
                }
                mTopCameraDevice = new CameraDeviceExt(mCameraActivity, mTopCamDevice,
                        mTopCamParameters, mTopCamId, mPreferences);
            } else {
                Log.d(TAG, "[openCamera fail],mTopCamDevice:" + mTopCamDevice
                        + ",mTopCamParameters:" + mTopCamParameters);
            }
            mIsOpenCameraFail = false;
            mModuleManager.onCameraOpen();
            return CAMERA_OPEN_SUCEESS;
        }

        private void waitWithoutInterrupt(Object object) {
            try {
                object.wait();
            } catch (InterruptedException e) {
                Log.w(TAG, "unexpected interrupt: " + object);
            }
        }

        private int firstOpenCamera() {
            Log.d(TAG, "[run.firstOpenCamera] mCameraId:" + mCameraId);
            try {
                Util.openCamera(mCameraActivity, false, mCameraId);
                mCameraDevice = CameraHolder.instance().getCameraProxy(mCameraId);
            } catch (CameraHardwareException e) {
                Log.e(TAG, "[run.firstOpenCamera]CameraHardwareException e:" + e);
                mIsOpenCameraFail = true;
                mMainHandler.sendEmptyMessage(MSG_OPEN_CAMERA_FAIL);
                return CAMERA_HARDWARE_EXCEPTION;
            } catch (CameraDisabledException e) {
                Log.e(TAG, "[run.firstOpenCamera]CameraDisabledException e:" + e);
                mIsOpenCameraFail = true;
                mMainHandler.sendEmptyMessage(MSG_OPEN_CAMERA_DISABLED);
                return CAMERA_DISABLED_EXCEPTION;
            }
            mParameters = (mCameraDevice == null) ? null : CameraHolder.instance()
                    .getOriginalParameters(mCameraId);
            if (mCameraDevice != null && mParameters != null) {
                mCurCameraDevice = new CameraDeviceExt(mCameraActivity, mCameraDevice, mParameters,
                        mCameraId, mPreferences);
                String defaultZsdValue = mCurCameraDevice.getParametersExt().getZSDMode();
                Log.d(TAG, "[run.firstOpenCamera], Get default ZSD value " +
                        "from parameters, value:" + defaultZsdValue);
                SettingUtils.setZsdDefaultValue(defaultZsdValue);
                String defaultAntiBandingValue = mCurCameraDevice
                        .getParametersExt().getAntibanding();
                SettingUtils.setAntiBandingDefaultValue(defaultAntiBandingValue);
            } else {
                Log.d(TAG, "[openCamera fail],mCameraDevice:" + mCameraDevice + ",mParameters:"
                        + mParameters);
            }
            mIsOpenCameraFail = false;
//            Log.i(TAG, "[run.firstOpenCamera] Open Camera done");
            return CAMERA_OPEN_SUCEESS;
        }
    }


    private void applyFirstParameters() {
        Log.d(TAG, "applyFirstParameters");
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_APPLY_FIRST_PARAMS,
                CameraPerformanceTracker.ISBEGIN);
        mIsFirstOpenCamera = false;
        mMainHandler.sendEmptyMessage(MSG_SET_PREVIEW_ASPECT_RATIO);
        switchCameraPreview();
        mCurCameraDevice.setJpegRotation(mOrientation);
        mCameraAppUi.setZoomParameter();
        mCurCameraDevice.setDisplayOrientation(true);
        mCurCameraDevice.setPreviewFormat(ImageFormat.YV12);
        // Set auto focus before startPreview to improve first launch AF done quality
        applyFocusCapabilities(false);
        // Camera do not open zsd mode launched by 3rd party.
        if (!mCameraActivity.isImageCaptureIntent() && !mCameraActivity.isVideoCaptureIntent()) {
            mCurCameraDevice.getParametersExt().setZSDMode(SettingUtils
                    .getPreferenceValue(mCameraActivity, mPreferences,
                            SettingConstants.ROW_SETTING_ZSD, SettingUtils.getZsdDefaultValue()));
        }

        mCurCameraDevice.getParametersExt().set(ParametersHelper.KEY_FIRST_PREVIEW_FRAME,
                Util.FIRST_PREVIEW_BLACK_ON);
        mCurCameraDevice.applyParametersToServer();
        mCameraActor.onCameraParameterReady(true);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_APPLY_FIRST_PARAMS,
                CameraPerformanceTracker.ISEND);
    }

    private void applySecondParameters() {
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_APPLY_SECOND_PARAMS,
                CameraPerformanceTracker.ISBEGIN);
        mCurCameraDevice.getParametersExt().set(ParametersHelper.KEY_FIRST_PREVIEW_FRAME,
                Util.FIRST_PREVIEW_BLACK_OFF);
        turnOnWhenShown();
        mCurCameraDevice.applyParametersToServer();
        mCameraActor.onCameraParameterReady(false);
        mMainHandler.sendEmptyMessage(MSG_CAMERA_PARAMETERS_READY);
        mCameraAppUi.setDngState(mISettingCtrl.getSettingValue(SettingConstants.KEY_DNG));
        mCameraAppUi.showRemainingAways();
        mCurCameraDevice.updateParameters();
        mLastPreviewSize = mCurCameraDevice.getPreviewSize();
        mLastZsdMode = mCurCameraDevice.getZsdMode();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_APPLY_SECOND_PARAMS,
                CameraPerformanceTracker.ISEND);
    }

    private CameraState getCameraState() {
        return mCameraState;
    }

    private void setCameraState(CameraState cameraState) {
        Log.d(TAG, "[setCameraState] cameraState:" + cameraState);
        mCameraState = cameraState;
    }

    // ///TODO....
    public void doSwitchCameraDevice() {
        Log.i(TAG, "doSwitchCameraDevice");
        if (getCameraState() == CameraState.STATE_CAMERA_CLOSED) {
            return;
        }
        mIsSwitchingPip = true;
        // if face detection is enable, stop face detection first
        mCameraAppUi.resetZoom();
        mCameraAppUi.setZoomParameter();
        mCurCameraDevice.stopFaceDetection();
        clearDeviceCallbacks();
        mCameraAppUi.dismissInfo();
        ICameraDeviceExt temp = mCurCameraDevice;
        mCurCameraDevice = mTopCameraDevice;
        mTopCameraDevice = temp;
        mCameraStartUpThread.setCameraId(mCurCameraDevice.getCameraId());

        applyDeviceCallbacks();
        initializeFocusManager();
        mCameraAppUi.collapseViewManager(true);
        clearFocusAndFace();
        // here set these variables null to initialize them again.
        // Restart the camera and initialize the UI. From onCreate.
        mPreferences.setLocalId(mCameraActivity, mCurCameraDevice.getCameraId());
        SettingUtils.upgradeLocalPreferences(mPreferences.getLocal());
        SettingUtils.writePreferredCameraId(mPreferences, mCurCameraDevice.getCameraId());
        mCurCameraDevice.setPreviewSize();
        mISettingCtrl.updateSetting(mPreferences.getLocal());
        // switch camera, preference will update, so need notify preference ready.
        mMainHandler.sendEmptyMessage(MSG_CAMERA_PREFERENCE_READY);
        // notify apply new settings
        if (mCameraActor.getMode() == ModePicker.MODE_PHOTO_PIP) {
            mISettingCtrl.onSettingChanged(SettingConstants.KEY_PHOTO_PIP, "on");
        } else {
            mISettingCtrl.onSettingChanged(SettingConstants.KEY_VIDEO_PIP, "on");
        }
        // apply new parameters
        applyParameters(false);
        mIsSwitchingPip = false;
    }

    private void clearFocusAndFace() {
        if (mCameraActivity.getFrameView() != null) {
            mCameraActivity.getFrameView().clear();
        }
        if (mFocusManager != null) {
            mFocusManager.removeMessages();
        }
    }

    private void releaseSurfaceTexture() {
//        Log.d(TAG, "[releaseSurfaceTexture]");
        mTopCamSurfaceTexture = null;
        mSurfaceTexture = null;
    }

    private void notifySurfaceViewDestroy(SurfaceHolder holder) {
//        Log.d(TAG, "[notifySurfaceViewDestroy]");
        if (mModuleManager.isDisplayUseSurfaceView()) {
            mModuleManager.notifySurfaceViewDestroyed(holder.getSurface());
        }
    }

    private PreviewCallback mOneShotPreviewCallback = new PreviewCallback() {

        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
//            Log.d(TAG, "[mOneShotPreviewCallback.onPreviewFrame]");
            detachSurfaceViewLayout();
            View intentCover = mCameraActivity.findViewById(R.id.intentcover);
            if (intentCover != null) {
                intentCover.setVisibility(View.GONE);
            }
            hideRootCover();
        }

    };
}
