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
 *     MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.camera.feature.mode.longexposure;

import android.app.Activity;
import android.content.ContentValues;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.RelativeLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.IAppUiListener.ISurfaceStatusListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.CameraSysTrace;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.mode.CameraModeBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.photo.device.IDeviceController;
import com.mediatek.camera.common.mode.photo.device.IDeviceController.CaptureDataCallback;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.relation.StatusMonitor.StatusChangeListener;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingManagerFactory;
import com.mediatek.camera.common.storage.MediaSaver.MediaSaverListener;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.mode.longexposure.ILongExposureDeviceController.DeviceCallback;
import com.mediatek.camera.feature.mode.longexposure.ILongExposureDeviceController
        .PreviewSizeCallback;
import com.mediatek.camera.feature.mode.longexposure.LongExposureView.LongExposureViewState;

import java.util.ArrayList;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * Long exposure mode that is used to take long exposure picture.
 */
public class LongExposureMode extends CameraModeBase implements IAppUiListener.OnGestureListener,
        CaptureDataCallback, DeviceCallback, PreviewSizeCallback {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(LongExposureMode.class.getSimpleName());
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final String KEY_SHUTTER_SPEED = "key_shutter_speed";

    private static final String JPEG_CALLBACK = "jpeg callback";

    private Activity mActivity;
    private String mCameraId;
    private LongExposureHandler mLongExposureHandler;
    private RelativeLayout mLongExposureRoot;
    private LongExposureView mLongExposureView;
    private IAppUi.HintInfo mGuideHint;
    private static final int SHOW_INFO_LENGTH_LONG = 5 * 1000;

    private volatile boolean mIsResumed = false;
    private ILongExposureDeviceController mIDeviceController;
    private LongExposureModeHelper mLongExposureModeHelper;
    private ISettingManager mISettingManager;
    private int mPreviewWidth;
    private int mPreviewHeight;

    private ISurfaceStatusListener mISurfaceStatusListener = new SurfaceChangeListener();
    private StatusChangeListener mStatusChangeListener = new MyStatusChangeListener();
    private LongExposureView.OnCountDownFinishListener mOnCountDownFinishListener
            = new MyOnCountDownFinishListener();
    private CaptureAbortListener mOnLongExposureViewClickedListener = new
            CaptureAbortListener();
    private LongExposureView.LongExposureViewState mLastState = LongExposureView
            .LongExposureViewState.STATE_IDLE;

    private final static int GESTURE_PRIORITY = 0;

    private static final int PREVIEW = 0;
    private static final int CAPTURE = 1;
    private static final int ABORT_CAPTURE = 2;

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
                     boolean isFromLaunch) {
        LogHelper.d(TAG, "[init]+");
        super.init(app, cameraContext, isFromLaunch);
        mLongExposureHandler = new LongExposureHandler(Looper.myLooper());
        mActivity = mIApp.getActivity();
        mIApp.getAppUi().registerGestureListener(this, GESTURE_PRIORITY);
        mLongExposureModeHelper = new LongExposureModeHelper(cameraContext);
        mCameraId = getCameraIdByFacing(mDataStore.getValue(
                KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));
        DeviceControllerFactory deviceControllerFactory = new DeviceControllerFactory();
        mIDeviceController = deviceControllerFactory.createDeviceController(app.getActivity(),
                mCameraApi, mICameraContext);
        initSettingManager(mCameraId);
        initLongExposureView();
        prepareAndOpenCamera(mCameraId);
        LogHelper.d(TAG, "[init]-");
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        LogHelper.d(TAG, "[resume]+");
        super.resume(deviceUsage);
        mIsResumed = true;
        mIDeviceController.queryCameraDeviceManager();
        initSettingManager(mCameraId);
        prepareAndOpenCamera(mCameraId);
        LogHelper.d(TAG, "[resume]-");
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        LogHelper.d(TAG, "[pause]+");
        super.pause(nextModeDeviceUsage);
        updateUiState(LongExposureViewState.STATE_ABORT);
        mIsResumed = false;
        mIApp.getAppUi().clearPreviewStatusListener(mISurfaceStatusListener);
        if (mNeedCloseCameraIds.size() > 0) {
            prePareAndCloseCamera(needCloseCameraSync(), mCameraId);
            recycleSettingManager(mCameraId);
        }else if(mNeedCloseSession) {
            LogHelper.i(TAG, "[mNeedCloseSession] : " + mNeedCloseSession);
            mIDeviceController.closeSession();
        } else {
            clearAllCallbacks(mCameraId);
            mIDeviceController.stopPreview();
        }
        LogHelper.d(TAG, "[pause]-");
    }
    @Override
    public DeviceUsage getDeviceUsage(@Nonnull DataStore dataStore, DeviceUsage oldDeviceUsage) {
        ArrayList<String> openedCameraIds = new ArrayList<>();
        String cameraId = getCameraIdByFacing(dataStore.getValue(
                KEY_CAMERA_SWITCHER, null, dataStore.getGlobalScope()));
        openedCameraIds.add(cameraId);
        updateModeDefinedCameraApi();
        return new DeviceUsage(DeviceUsage.DEVICE_TYPE_NORMAL, mCameraApi, openedCameraIds,
                DeviceUsage.BUFFER_FLOW_TYPE_POSTALGO);
    }

    @Override
    public void unInit() {
        LogHelper.i(TAG, "[unInit]+");
        super.unInit();
        mIApp.getAppUi().unregisterGestureListener(this);
        mIApp.getAppUi().hideScreenHint(mGuideHint);
        mIDeviceController.destroyDeviceController();
        LogHelper.i(TAG, "[unInit]-");
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return true;
    }

    @Override
    public boolean onShutterButtonClick() {
        boolean storageReady = mICameraContext.getStorageService().getCaptureStorageSpace() > 0;
        boolean isDeviceReady = mIDeviceController.isReadyForCapture();
        LogHelper.i(TAG, "[onShutterButtonClick], is storage ready : " + storageReady +
                "," + " isDeviceReady = " + isDeviceReady + ",getModeDeviceStatus() " +
                getModeDeviceStatus());
        if (MODE_DEVICE_STATE_CAPTURING.equals(getModeDeviceStatus())) {
            doAbort();
            return true;
        }
        if (storageReady && isDeviceReady) {
            updateModeDeviceState(MODE_DEVICE_STATE_CAPTURING);
            mIApp.getAppUi().hideScreenHint(mGuideHint);
            mLongExposureHandler.sendEmptyMessage(CAPTURE);
            updateUiState(LongExposureViewState.STATE_CAPTURE);
        }
        return true;
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        return false;
    }

    @Override
    public void onDataReceived(IDeviceController.DataCallbackInfo dataCallbackInfo) {
        byte[] data = dataCallbackInfo.data;
        LogHelper.d(TAG, "[onDataReceived] data = " + data);
        updateUiState(LongExposureViewState.STATE_PREVIEW);
        CameraSysTrace.onEventSystrace(JPEG_CALLBACK, true);
        //save file first,because save file is in other thread, so will improve the shot to shot
        //performance.
        if (data != null) {
            saveData(data);
            updateThumbnail(data);
        }

        if (mIsResumed && mCameraApi == CameraDeviceManagerFactory.CameraApi.API1) {
            mIDeviceController.startPreview();
        }
        CameraSysTrace.onEventSystrace(JPEG_CALLBACK, false);
    }

    @Override
    public void onPostViewCallback(byte[] data) {
    }

    @Override
    protected ISettingManager getSettingManager() {
        return mISettingManager;
    }

    @Override
    public void onCameraOpened(String cameraId) {
        updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
    }

    @Override
    public void beforeCloseCamera() {
        updateModeDeviceState(MODE_DEVICE_STATE_CLOSED);
    }

    @Override
    public void afterStopPreview() {
        updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
    }

    @Override
    public void onPreviewCallback(byte[] data, int format) {
        if (!mIsResumed) {
            return;
        }
        LogHelper.d(TAG, "[onPreviewCallback]");
        mIApp.getAppUi().onPreviewStarted(mCameraId);
        updateUiState(LongExposureViewState.STATE_PREVIEW);
        updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
        mLongExposureHandler.sendEmptyMessage(PREVIEW);
    }

    @Override
    public void onPreviewSizeReady(Size previewSize) {
        LogHelper.d(TAG, "[onPreviewSizeReady] previewSize: " + previewSize.toString());
        updatePictureSizeAndPreviewSize(previewSize);
    }

    @Override
    public boolean onDown(MotionEvent event) {
        return true;
    }

    @Override
    public boolean onUp(MotionEvent event) {
        return true;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        return true;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float dx, float dy) {
        return true;
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
        return true;
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
        return true;
    }

    private void onPreviewSizeChanged(int width, int height) {
        mPreviewWidth = width;
        mPreviewHeight = height;
        mIApp.getAppUi().setPreviewSize(mPreviewWidth, mPreviewHeight, mISurfaceStatusListener);
    }

    private void prepareAndOpenCamera(String cameraId) {
        mCameraId = cameraId;
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        statusMonitor.registerValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
        //before open camera, prepare the device callback and size changed callback.
        mIDeviceController.setDeviceCallback(this);
        mIDeviceController.setPreviewSizeReadyCallback(this);
        //prepare device info.
        DeviceInfo info = new DeviceInfo();
        info.setCameraId(mCameraId);
        info.setSettingManager(mISettingManager);
        mIDeviceController.openCamera(info);
    }

    private void prePareAndCloseCamera(boolean needSync, String cameraId) {
        clearAllCallbacks(cameraId);
        mIDeviceController.closeCamera(needSync);
        //reset the preview size and preview data.
        mPreviewWidth = 0;
        mPreviewHeight = 0;
    }

    private void clearAllCallbacks(String cameraId) {
        mIDeviceController.setPreviewSizeReadyCallback(null);
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(cameraId);
        statusMonitor.unregisterValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
    }

    private void initSettingManager(String cameraId) {
        SettingManagerFactory smf = mICameraContext.getSettingManagerFactory();
        mISettingManager = smf.getInstance(
                cameraId,
                getModeKey(),
                ModeType.PHOTO,
                mCameraApi);
    }

    private void initLongExposureView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                initGuideHint();
                mActivity.getLayoutInflater().inflate(R
                                .layout.longexposurecapture,
                        mIApp.getAppUi().getModeRootView(), true);
                mLongExposureRoot = (RelativeLayout) mActivity.findViewById(R.id
                        .long_exposure_ui);
                mLongExposureView = (LongExposureView) mActivity.findViewById(R.id
                        .long_exposure_progress);
                mLongExposureView.setViewStateChangedListener(mOnLongExposureViewClickedListener);
                mLongExposureView.setAddCountDownListener(mOnCountDownFinishListener);
                mIApp.getAppUi().showScreenHint(mGuideHint);
            }
        });
    }

    private void initGuideHint() {
        mGuideHint = new IAppUi.HintInfo();
        int id = mIApp.getActivity().getResources().getIdentifier("hint_text_background",
                "drawable", mIApp.getActivity().getPackageName());
        mGuideHint.mBackground = mIApp.getActivity().getDrawable(id);
        mGuideHint.mType = IAppUi.HintType.TYPE_AUTO_HIDE;
        mGuideHint.mDelayTime = SHOW_INFO_LENGTH_LONG;
        mGuideHint.mHintText = mActivity.getString(R.string.long_exposure_guide_hint);

    }

    private void recycleSettingManager(String cameraId) {
        mICameraContext.getSettingManagerFactory().recycle(cameraId);
    }

    private void updatePictureSizeAndPreviewSize(Size previewSize) {
        ISettingManager.SettingController controller = mISettingManager.getSettingController();
        String size = controller.queryValue(KEY_PICTURE_SIZE);
        if (size != null && mIsResumed) {
            String[] pictureSizes = size.split("x");
            int captureWidth = Integer.parseInt(pictureSizes[0]);
            int captureHeight = Integer.parseInt(pictureSizes[1]);
            mIDeviceController.setPictureSize(new Size(captureWidth, captureHeight));
            int width = previewSize.getWidth();
            int height = previewSize.getHeight();
            LogHelper.d(TAG, "[updatePictureSizeAndPreviewSize] picture size: " + captureWidth +
                    " X" + captureHeight + ",current preview size:" + mPreviewWidth + " X " +
                    mPreviewHeight + ",new value :" + width + " X " + height);
            if (width != mPreviewWidth || height != mPreviewHeight) {
                onPreviewSizeChanged(width, height);
            }
        }

    }

    private void saveData(byte[] jpegData) {
        LogHelper.d(TAG, "[saveData]");
        if (jpegData != null) {
            String fileDirectory = mICameraContext.getStorageService().getFileDirectory();
            Size exifSize = CameraUtil.getSizeFromExif(jpegData);
            ContentValues contentValues = mLongExposureModeHelper.createContentValues(jpegData,
                    fileDirectory, exifSize.getWidth(), exifSize.getHeight());
            mICameraContext.getMediaSaver().addSaveRequest(jpegData, contentValues, null,
                    mMediaSaverListener);
        }
    }

    private void updateThumbnail(byte[] jpegData) {
        Bitmap bitmap = BitmapCreator.createBitmapFromJpeg(jpegData, mIApp.getAppUi()
                .getThumbnailViewWidth());
        mIApp.getAppUi().updateThumbnail(bitmap);
    }


    private void updateUiState(LongExposureViewState state) {
        if (mLongExposureView == null) {
            return;
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                LogHelper.d(TAG, "[updateUiState] state = " + state + ",mLastState " + mLastState);
                if (state == mLastState) {
                    return;
                }
                mLastState = state;
                mLongExposureView.updateViewState(state);
                if (LongExposureViewState.STATE_PREVIEW == state) {
                    mIApp.getAppUi().animationEnd(IAppUi.AnimationType.TYPE_CAPTURE);
                    mLongExposureRoot.setVisibility(View.INVISIBLE);
                    mIApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
                    mIApp.getAppUi().applyAllUIEnabled(true);
                } else if (LongExposureViewState.STATE_ABORT == state) {
                    mLongExposureRoot.setVisibility(View.INVISIBLE);
                    mLongExposureRoot.setClickable(false);
                    mIApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
                    mIApp.getAppUi().applyAllUIEnabled(false);
                } else if (LongExposureViewState.STATE_CAPTURE == state) {
                    mIApp.getAppUi().animationStart(IAppUi.AnimationType.TYPE_CAPTURE, null);
                    ISettingManager.SettingController controller = mISettingManager
                            .getSettingController();
                    String speed = controller.queryValue(KEY_SHUTTER_SPEED);
                    LogHelper.d(TAG, "[updateUi] mShutterSpeed speed = " + speed);
                    if (speed != null && !LongExposureModeHelper.EXPOSURE_TIME_AUTO.equals(speed)) {
                        mIApp.getAppUi().applyAllUIVisibility(View.INVISIBLE);
                        mLongExposureRoot.setVisibility(View.VISIBLE);
                        mLongExposureRoot.setClickable(true);
                        mLongExposureView.setCountdownTime(Integer.parseInt(speed));
                        mLongExposureView.startCountDown();
                    } else {
                        mIDeviceController.setNeedWaitPictureDone(true);
                    }
                }
            }
        });
    }

    private MediaSaverListener mMediaSaverListener = new MediaSaverListener() {

        @Override
        public void onFileSaved(Uri uri) {
            mIApp.notifyNewMedia(uri, true);
        }
    };

    /**
     * surface changed listener.
     */
    private class SurfaceChangeListener implements ISurfaceStatusListener {

        @Override
        public void surfaceAvailable(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "[surfaceAvailable] device controller = " + mIDeviceController
                    + ",mIsResumed = " + mIsResumed + ",w = " + width + ",h = " + height);
            if (mIDeviceController != null && mIsResumed) {
                mIDeviceController.updatePreviewSurface(surfaceObject);
            }
        }

        @Override
        public void surfaceChanged(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "[surfaceChanged] device controller = " + mIDeviceController
                    + ",mIsResumed = " + mIsResumed + ",w = " + width + ",h = " + height);
            if (mIDeviceController != null && mIsResumed) {
                mIDeviceController.updatePreviewSurface(surfaceObject);
            }
        }

        @Override
        public void surfaceDestroyed(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "[surfaceDestroyed] device controller = " + mIDeviceController);
        }
    }

    /**
     * Handler to handle message which comes from device controller and setting manager.
     */
    private final class LongExposureHandler extends Handler {

        public LongExposureHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage] " + msg.what);
            switch (msg.what) {
                case PREVIEW:
                    break;
                case CAPTURE:
                    mIDeviceController.updateGSensorOrientation(mIApp.getGSensorOrientation());
                    mIDeviceController.takePicture(LongExposureMode.this);
                    break;
                case ABORT_CAPTURE:
                    mIDeviceController.stopCapture();
                    break;
                default:
                    break;
            }
        }
    }

    private final class MyOnCountDownFinishListener implements LongExposureView
            .OnCountDownFinishListener {
        @Override
        public void countDownFinished(boolean isFullProgress) {
            mIDeviceController.setNeedWaitPictureDone(isFullProgress);
        }
    }

    /**
     * Status change listener implement.
     */
    private class MyStatusChangeListener implements StatusChangeListener {
        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.d(TAG, "[onStatusChanged] key = " + key + ",value = " + value);
            if (KEY_PICTURE_SIZE.equalsIgnoreCase(key)) {
                String[] sizes = value.split("x");
                int captureWidth = Integer.parseInt(sizes[0]);
                int captureHeight = Integer.parseInt(sizes[1]);
                mIDeviceController.setPictureSize(new Size(captureWidth, captureHeight));
                Size previewSize = mIDeviceController.getPreviewSize((double) captureWidth /
                        captureHeight);
                int width = previewSize.getWidth();
                int height = previewSize.getHeight();
                if (width != mPreviewWidth || height != mPreviewHeight) {
                    onPreviewSizeChanged(width, height);
                }
            }
        }
    }

    private class CaptureAbortListener implements LongExposureView
            .OnCaptureAbortedListener {
        @Override
        public void onCaptureAbort() {
            doAbort();
        }
    }

    private void doAbort() {
        ISettingManager.SettingController controller = mISettingManager
                .getSettingController();
        String speed = controller.queryValue(KEY_SHUTTER_SPEED);
        if (LongExposureModeHelper.EXPOSURE_TIME_AUTO.equals(speed)) {
            return;
        }
        if (!MODE_DEVICE_STATE_CAPTURING.equals(getModeDeviceStatus())) {
            LogHelper.w(TAG, "[doAbort] mode device state " + getModeDeviceStatus());
            return;
        }
        if (mLastState == LongExposureViewState.STATE_ABORT) {
            LogHelper.w(TAG, "[doAbort] still during aborting");
            return;
        }
        updateUiState(LongExposureViewState.STATE_ABORT);
        mLongExposureHandler.sendEmptyMessage(ABORT_CAPTURE);
    }

}