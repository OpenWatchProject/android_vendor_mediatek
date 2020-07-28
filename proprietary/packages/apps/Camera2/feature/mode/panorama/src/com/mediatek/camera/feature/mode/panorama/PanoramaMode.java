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
package com.mediatek.camera.feature.mode.panorama;

import android.app.Activity;
import android.content.ContentValues;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.media.MediaActionSound;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.CameraSysTrace;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraModeBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingManagerFactory;
import com.mediatek.camera.common.storage.MediaSaver;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.util.ArrayList;

import javax.annotation.Nonnull;

public class PanoramaMode extends CameraModeBase implements
        IPanormaDeviceController.CaptureDataCallback,
        IPanormaDeviceController.DeviceCallback,
        IPanormaDeviceController.PreviewSizeCallback, IAppUiListener.OnGestureListener {

    private static final LogUtil.Tag TAG = new LogUtil.Tag(PanoramaMode.class.getSimpleName());
    protected static final String KEY_PHOTO_CAPTURE = "key_photo_capture";
    protected static final String PHOTO_CAPTURE_START = "start";
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final String JPEG_CALLBACK = "jpeg callback";
    private static final String KEY_FORMTAT = "key_format";
    private static final String MODE_KEY = PanoramaMode.class.getName();

    private Activity mActivity;
    private String mCameraId;
    private MediaActionSound mCameraSound;
    private Thread mLoadSoundTread;
    private IPanormaDeviceController mIPanorma2DeviceController;
    private PanoramaView mPanorama2View;
    private PanoramaModeHelper mPanorama2ModeHelper;
    private StatusMonitor.StatusResponder mPhotoStatusResponder;
    private StatusMonitor.StatusChangeListener mStatusChangeListener = new MyStatusChangeListener();
    private PanoramaHandler mMainHandler;
    private IAppUiListener.ISurfaceStatusListener mISurfaceStatusListener
            = new MySurfaceStausListener();
    private ISettingManager mISettingManager;

    public static final int GUIDE_SHUTTER = 0;
    public static final int GUIDE_MOVE = 1;
    public static final int INFO_UPDATE_PROGRESS = 0;
    public static final int INFO_UPDATE_MOVING = 1;
    public static final int INFO_START_ANIMATION = 2;
    public static final int INFO_IN_CAPTURING = 3;
    public static final int INFO_OUT_OF_CAPTURING = 4;

    private static final int NUM_AUTORAMA_CAPTURE = 9;
    private static final int MSG_FINAL_IMAGE_READY = 1000;
    private static final int MSG_UPDATE_MOVING = 1005;
    private static final int MSG_UPDATE_PROGRESS = 1006;
    private static final int MSG_START_ANIMATION = 1007;
    private static final int MSG_HIDE_VIEW = 1008;
    private static final int MSG_IN_CAPTURING = 1009;
    private static final int MSG_OUT_OF_CAPTURING = 1010;
    private static final int TIME_RESTART_CAPTURE_VIEW_MS = 500;

    private int mPreviewWidth;
    private int mPreviewHeight;
    protected int mCaptureWidth;
    private Runnable mRestartCaptureView;
    // make sure the picture size ratio = mCaptureWidth / mCaptureHeight not to NAN.
    protected int mCaptureHeight = Integer.MAX_VALUE;
    protected volatile boolean mIsResumed = true;
    private State mState = State.STATE_UNKNOWN;
    private boolean mIsShowingCollimatedDrawable;
    private boolean mIsInStopProcess = false;
    private boolean mIsMerging = false;
    private int mCurrentNum = 0;

    enum State {
        STATE_UNKNOWN,
        STATE_IDLE,
        STATE_CAPTURING,
        STATE_CLOSED,
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
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        return true;
    }

    @Override
    public boolean onDoubleTap(float x, float y) {
        return true;
    }

    @Override
    public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
        return true;
    }

    @Override
    public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
        return true;
    }

    @Override
    public boolean onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
        return true;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        return true;
    }


    PanoramaView.OnSaveButtonClickedListener mSaveButtonClickedListener
            = new PanoramaView.OnSaveButtonClickedListener() {
        @Override
        public void onSaveButtonClicked() {
            stopCapture(true);
            mState = State.STATE_IDLE;
        }
    };
    PanoramaView.OnCancelButtonClickedListener mCancelButtonClickedListener
            = new PanoramaView.OnCancelButtonClickedListener() {
        @Override
        public void onCancelButtonClicked() {
            stopCapture(false);
            mState = State.STATE_IDLE;
        }
    };

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        LogHelper.d(TAG, "[resume]+");
        super.resume(deviceUsage);
        mIsResumed = true;
        initSettingManager(mCameraId);
        initStatusMonitor();
        mIPanorma2DeviceController.queryCameraDeviceManager();
        prepareAndOpenCamera(mCameraId);
        LogHelper.d(TAG, "[resume]-");
    }

    @Override
    public void pause(@Nonnull DeviceUsage nextModeDeviceUsage) {
        LogHelper.d(TAG, "[pause]+");
        super.pause(nextModeDeviceUsage);
        mIsResumed = false;
        stopCapture(false);
        mState = State.STATE_CLOSED;
        if (mMainHandler != null) {
            mMainHandler.removeMessages(MSG_HIDE_VIEW);
            mMainHandler.sendEmptyMessage(MSG_HIDE_VIEW);
            if (mIsMerging) {
                mIsMerging = false;
                mMainHandler.removeMessages(MSG_FINAL_IMAGE_READY);
                mMainHandler.sendEmptyMessage(MSG_FINAL_IMAGE_READY);
            }
        }
        //clear the surface listener
        mIApp.getAppUi().clearPreviewStatusListener(mISurfaceStatusListener);
        if (mNeedCloseCameraIds.size() > 0) {
            prePareAndCloseCamera(needCloseCameraSync(), mCameraId);
            recycleSettingManager(mCameraId);
        } else if (mNeedCloseSession) {
            clearAllCallbacks(mCameraId);
            mIPanorma2DeviceController.closeSession();
        } else {
            clearAllCallbacks(mCameraId);
            mIPanorma2DeviceController.stopPreview();
        }
        LogHelper.d(TAG, "[pause]-");
    }

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
                     boolean isFromLaunch) {
        super.init(app, cameraContext, isFromLaunch);
        mActivity = app.getActivity();
        mCameraId = getCameraIdByFacing(mDataStore.getValue(
                KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));
        mIPanorma2DeviceController = new PanoramaDeviceController(mActivity, mICameraContext);
        mMainHandler = new PanoramaHandler(mActivity.getMainLooper());
        mPanorama2View = new PanoramaView(app, Integer.parseInt(mCameraId));
        initPanoramaView();
        initSettingManager(mCameraId);
        initStatusMonitor();
        prepareAndOpenCamera(mCameraId);
        mPanorama2ModeHelper = new PanoramaModeHelper(mICameraContext);
        mCameraSound = new MediaActionSound();
        mLoadSoundTread = new LoadSoundTread();
        mLoadSoundTread.start();
    }

    private void initPanoramaView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mPanorama2View.init();
                mPanorama2View.setSaveButtonClickedListener(mSaveButtonClickedListener);
                mPanorama2View.setCancelButtonClickedListener(mCancelButtonClickedListener);
            }
        });
    }

    private void initSettingManager(String mCameraId) {
        SettingManagerFactory smf = mICameraContext.getSettingManagerFactory();
        mISettingManager = smf.getInstance(mCameraId, getModeKey(), ModeType.PHOTO, mCameraApi);
    }

    private void recycleSettingManager(String cameraId) {
        mICameraContext.getSettingManagerFactory().recycle(cameraId);
    }

    private void initStatusMonitor() {
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        mPhotoStatusResponder = statusMonitor.getStatusResponder(KEY_PHOTO_CAPTURE);
    }

    private void prepareAndOpenCamera(String mCameraId) {
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        statusMonitor.registerValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
        mIPanorma2DeviceController.setDeviceCallback(this);
        mIPanorma2DeviceController.setPreviewSizeReadyCallback(this);
        DeviceInfo info = new DeviceInfo();
        info.setCameraId(mCameraId);
        info.setSettingManager(mISettingManager);
        mIPanorma2DeviceController.openCamera(info);
    }

    @Override
    public void unInit() {
        super.unInit();
        mIPanorma2DeviceController.destroyDeviceController();
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

    private class LoadSoundTread extends Thread {
        @Override
        public void run() {
            mCameraSound.load(MediaActionSound.SHUTTER_CLICK);
        }
    }

    @Override
    protected boolean doShutterButtonClick() {

        return capture();
    }

    private boolean capture() {
        boolean storageReady = mICameraContext.getStorageService().getCaptureStorageSpace() > 0;
        boolean isDeviceReady = mIPanorma2DeviceController.isReadyForCapture();
        LogHelper.i(TAG, "onShutterButtonClick, is storage ready : " + storageReady + "," +
                "isDeviceReady = " + isDeviceReady + "mState = " + mState);
        if (!storageReady || !isDeviceReady || State.STATE_IDLE != mState || mIsMerging) {
            LogHelper.w(TAG, "[capture] return");
            return false;
        }
        if (!startCapture()) {
            LogHelper.w(TAG, "[capture] do not capture.");
            return false;
        }
        mPanorama2View.showGuideView(GUIDE_MOVE);
        mMainHandler.sendEmptyMessage(MSG_IN_CAPTURING);
        return true;
    }

    private boolean startCapture() {
        LogHelper.d(TAG, "[startCapture] mState = " + mState + ",mIsInStopProcess = "
                + mIsInStopProcess);
        if (State.STATE_IDLE == mState && !mIsInStopProcess) {
            mState = State.STATE_CAPTURING;
            mCurrentNum = 0;
            mIsShowingCollimatedDrawable = false;
            mPhotoStatusResponder.statusChanged(KEY_PHOTO_CAPTURE, PHOTO_CAPTURE_START);
            updateModeDeviceState(MODE_DEVICE_STATE_CAPTURING);
            mIPanorma2DeviceController.updateGSensorOrientation(mIApp.getGSensorOrientation());
            mIPanorma2DeviceController.takePicture(this);
            lock3A();
            mIApp.getAppUi().registerGestureListener(this, 0);
            updateView();
            return true;
        }
        return false;
    }

    private void lock3A() {
        mISettingManager.getSettingController().postRestriction(
                PanoramaRestriction.get3aRestriction().
                        getRelation("on", true));
    }

    private void unlock3A() {
        mISettingManager.getSettingController().postRestriction(
                PanoramaRestriction.get3aRestriction()
                        .getRelation("off", true));
    }

    private void updateView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mPanorama2View.show();
            }
        });
    }

    @Override
    public String getModeKey() {
        return MODE_KEY;
    }

    @Override
    protected ISettingManager getSettingManager() {
        return mISettingManager;
    }

    @Override
    public void onPostalgoReceived(int X, int Y, int dir, int isShot) {
        if (isShot == 1) {
            onPictureTaken();
        } else {
            mMainHandler.obtainMessage(MSG_UPDATE_MOVING, X, Y, dir).sendToTarget();
        }
    }

    private void onPictureTaken() {
        LogHelper.d(TAG, "[onPictureTaken] mState = " +
                mState + ", mCurrentNum = " + mCurrentNum);
        if (State.STATE_IDLE == mState) {
            LogHelper.w(TAG, "[onPictureTaken]modeState is STATE_IDLE,return.");
            return;
        }

        if (mCurrentNum == NUM_AUTORAMA_CAPTURE || mIsMerging) {
            LogHelper.d(TAG, "[onPictureTaken] autoRama done, mCurrentNum = " + mCurrentNum);
            mIsMerging = false;

        } else if (mCurrentNum >= 0 && mCurrentNum < NUM_AUTORAMA_CAPTURE) {
            if (mCameraSound != null) {
                mCameraSound.play(MediaActionSound.SHUTTER_CLICK);
            }
            mMainHandler.obtainMessage(MSG_UPDATE_PROGRESS, mCurrentNum, 0).sendToTarget();
            if (0 < mCurrentNum) {
                if (mIsShowingCollimatedDrawable) {
                    mMainHandler.removeCallbacks(mRestartCaptureView);
                }
                mIsShowingCollimatedDrawable = true;
                mRestartCaptureView = new Runnable() {
                    public void run() {
                        mIsShowingCollimatedDrawable = false;
                        mMainHandler.obtainMessage(MSG_START_ANIMATION, mCurrentNum, 0)
                                .sendToTarget();
                    }
                };
                mMainHandler.postDelayed(mRestartCaptureView, TIME_RESTART_CAPTURE_VIEW_MS);
            }
        }
        mCurrentNum++;
        if (mCurrentNum == NUM_AUTORAMA_CAPTURE) {
            stopCapture(true);
            mState = State.STATE_IDLE;
        }
    }

    @Override
    public void onDataReceived(IPanormaDeviceController.DataCallbackInfo dataCallbackInfo) {
        byte[] data = dataCallbackInfo.data;
        int format = dataCallbackInfo.mBufferFormat;
        boolean needUpdateThumbnail = dataCallbackInfo.needUpdateThumbnail;
        boolean needRestartPreview = dataCallbackInfo.needRestartPreview;
        LogHelper.d(TAG, "onDataReceived, data = " + data + ",mIsResumed = " + mIsResumed +
                ",needUpdateThumbnail = " + needUpdateThumbnail + ",needRestartPreview = " +
                needRestartPreview);
        mIsMerging = false;
        if (data != null) {
            CameraSysTrace.onEventSystrace(JPEG_CALLBACK, true);
        }
        //save file first,because save file is in other thread, so will improve the shot to shot
        //performance.
        if (data != null) {
            if (format == ImageFormat.JPEG) {
                saveData(data);
            }
        }
        //update thumbnail
        if (data != null && needUpdateThumbnail) {
            if (format == ImageFormat.JPEG) {
                updateThumbnail(data);
            }
        }
        if (data != null) {
            CameraSysTrace.onEventSystrace(JPEG_CALLBACK, false);
        }

    }

    private void stopCapture(boolean isMerge) {
        LogHelper.d(TAG, "[stopCapture]isMerge = " + isMerge + ", current mState = " + mState);
        if (State.STATE_CAPTURING == mState) {
            mMainHandler.sendEmptyMessage(MSG_OUT_OF_CAPTURING);
            if (mIsMerging) {
                // if current is in the progress merging,means before have stopped
                // the panorama, so can directly return.
                LogHelper.i(TAG, "[stop] current is also in merging,so cancel this time");
                return;
            } else {
                mIsMerging = isMerge;
                if (isMerge) {
                    showSavingProgress();
                    mIPanorma2DeviceController.mergePicture();
                } else {
                    mIPanorma2DeviceController.cancleMergePicture();
                }
                mMainHandler.removeMessages(MSG_UPDATE_MOVING);
                mMainHandler.removeMessages(MSG_HIDE_VIEW);
                mMainHandler.sendEmptyMessage(MSG_HIDE_VIEW);
                unlock3A();
            }
        }
    }

    @Override
    public void onCameraOpened(String cameraId) {
        LogHelper.d(TAG, "[onCameraOpened]");
        mState = State.STATE_IDLE;
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
        mIApp.getAppUi().applyAllUIEnabled(true);
        mIApp.getAppUi().onPreviewStarted(mCameraId);
        updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);

    }

    @Override
    public void onPreviewSizeReady(Size previewSize) {

        updatePictureSizeAndPreviewSize(previewSize);
    }

    private void updatePictureSizeAndPreviewSize(Size previewSize) {
        ISettingManager.SettingController controller = mISettingManager.getSettingController();
        String size = controller.queryValue(KEY_PICTURE_SIZE);
        if (size != null && mIsResumed) {
            String[] pictureSizes = size.split("x");
            mCaptureWidth = Integer.parseInt(pictureSizes[0]);
            mCaptureHeight = Integer.parseInt(pictureSizes[1]);
            mIPanorma2DeviceController.setPictureSize(new Size(mCaptureWidth, mCaptureHeight));
            int width = previewSize.getWidth();
            int height = previewSize.getHeight();
            LogHelper.d(TAG, "[updatePictureSizeAndPreviewSize] picture size: " +
                    mCaptureWidth + " X" + mCaptureHeight + ",current preview size:" +
                    mPreviewWidth + " X " + mPreviewHeight +
                    ",new value :" + width + " X " + height);
            if (width != mPreviewWidth || height != mPreviewHeight) {
                onPreviewSizeChanged(width, height);
            }
        }
    }

    private void onPreviewSizeChanged(int width, int height) {
        mPreviewWidth = width;
        mPreviewHeight = height;
        mIApp.getAppUi().setPreviewSize(mPreviewWidth, mPreviewHeight, mISurfaceStatusListener);
    }

    private class MyStatusChangeListener implements StatusMonitor.StatusChangeListener {

        @Override
        public void onStatusChanged(String key, String value) {

            LogHelper.i(TAG, "[onStatusChanged] key = " + key + ",value = " + value);
            if (KEY_PICTURE_SIZE.equalsIgnoreCase(key)) {
                String[] sizes = value.split("x");
                int captureWidth = Integer.parseInt(sizes[0]);
                int captureHeight = Integer.parseInt(sizes[1]);
                mIPanorma2DeviceController.setPictureSize(new Size(captureWidth, captureHeight));
                Size previewSize = mIPanorma2DeviceController.
                        getPreviewSize((double) captureWidth /
                                captureHeight);
                int width = previewSize.getWidth();
                int height = previewSize.getHeight();
                if (width != mPreviewWidth || height != mPreviewHeight) {
                    onPreviewSizeChanged(width, height);
                }
            }
        }
    }

    private class MySurfaceStausListener implements IAppUiListener.ISurfaceStatusListener {
        @Override
        public void surfaceAvailable(Object surfaceObject, int width, int height) {

            LogHelper.d(TAG, "surfaceAvailable,device controller = " +
                    mIPanorma2DeviceController + ",w = " + width + ",h = " + height);
            if (mModeHandler != null) {
                mModeHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIPanorma2DeviceController != null && mIsResumed) {
                            mIPanorma2DeviceController.updatePreviewSurface(surfaceObject);
                        }
                    }
                });
            }
        }

        @Override
        public void surfaceChanged(Object surfaceObject, int width, int height) {

            LogHelper.d(TAG, "surfaceChanged, device controller = " +
                    mIPanorma2DeviceController + ",w = " + width + ",h = " + height);
            if (mModeHandler != null) {
                mModeHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIPanorma2DeviceController != null && mIsResumed) {
                            mIPanorma2DeviceController.updatePreviewSurface(surfaceObject);
                        }
                    }
                });
            }
        }

        @Override
        public void surfaceDestroyed(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "surfaceDestroyed,device controller = " +
                    mIPanorma2DeviceController);
        }
    }

    private class PanoramaHandler extends Handler {

        public PanoramaHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            if (msg.what != MSG_UPDATE_MOVING) {
                LogHelper.d(TAG, "[handleMessage]msg.what = " + msg.what);
            }

            switch (msg.what) {
                case MSG_FINAL_IMAGE_READY:
                    dismissSavingProcess();
                    resetCapture();
                    break;

                case MSG_UPDATE_MOVING:
                    boolean shown = mIsShowingCollimatedDrawable
                            || State.STATE_CAPTURING != mState || mCurrentNum < 1;
                    LogHelper.d(TAG, "onPostalgoReceived, msg.arg1 = " + msg.arg1 +
                            ",msg.arg1 = " + msg.arg2 + ",msg.obj = " +
                            msg.obj + ",shown = " + shown);
                    mPanorama2View.update(INFO_UPDATE_MOVING, msg.arg1, msg.arg2, msg.obj, shown);
                    break;

                case MSG_UPDATE_PROGRESS:
                    mPanorama2View.update(INFO_UPDATE_PROGRESS, msg.arg1);
                    break;

                case MSG_START_ANIMATION:
                    mPanorama2View.update(INFO_START_ANIMATION, msg.arg1);
                    break;

                case MSG_HIDE_VIEW:
                    mPanorama2View.reset();
                    mPanorama2View.hide();
                    break;

                case MSG_IN_CAPTURING:
                    mPanorama2View.update(INFO_IN_CAPTURING, msg.arg1);
                    break;

                case MSG_OUT_OF_CAPTURING:
                    mPanorama2View.update(INFO_OUT_OF_CAPTURING, msg.arg1);
                    break;

                default:
                    break;
            }
        }
    }

    private void updateThumbnail(byte[] data) {
        Bitmap bitmap = BitmapCreator.createBitmapFromJpeg(data, mIApp.getAppUi()
                .getThumbnailViewWidth());
        mIApp.getAppUi().updateThumbnail(bitmap);
    }

    private void saveData(byte[] data) {
        if (data != null) {
            String fileDirectory = mICameraContext.getStorageService().getFileDirectory();
            Size exifSize = CameraUtil.getSizeFromExif(data);
            ContentValues contentValues = mPanorama2ModeHelper.createContentValues(data,
                    fileDirectory, exifSize.getWidth(), exifSize.getHeight());
            mICameraContext.getMediaSaver().addSaveRequest(data, contentValues, null,
                    mMediaSaverListener);
        }
    }

    private MediaSaver.MediaSaverListener mMediaSaverListener
            = new MediaSaver.MediaSaverListener() {
        @Override
        public void onFileSaved(Uri uri) {
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri);
            mIApp.notifyNewMedia(uri, true);
            mMainHandler.sendEmptyMessage(MSG_FINAL_IMAGE_READY);
        }
    };

    private void showSavingProgress() {
        mIApp.getAppUi().showSavingDialog(null, true);
    }

    private void dismissSavingProcess() {
        LogHelper.d(TAG, "[dismissSavingProcess]");
        mIApp.getAppUi().hideSavingDialog();
    }

    private void resetCapture() {
        LogHelper.d(TAG, "[resetCapture] current mState = " + mState);
        if (State.STATE_CLOSED != mState) {
            unlock3A();
            mPanorama2View.showGuideView(GUIDE_SHUTTER);
        }
        mIApp.getAppUi().unregisterGestureListener(this);
    }

    private void disableAllUIExceptionShutter() {
        mIApp.getAppUi().applyAllUIEnabled(false);
        mIApp.getAppUi().setUIEnabled(IAppUi.SHUTTER_BUTTON, true);
        mIApp.getAppUi().setUIEnabled(IAppUi.SHUTTER_TEXT, false);

    }

    private void clearAllCallbacks(String mCameraId) {
        mIPanorma2DeviceController.setPreviewSizeReadyCallback(null);
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        statusMonitor.unregisterValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
        statusMonitor.unregisterValueChangedListener(KEY_FORMTAT, mStatusChangeListener);
    }

    private void prePareAndCloseCamera(boolean needSync, String mCameraId) {
        clearAllCallbacks(mCameraId);
        mIPanorma2DeviceController.closeCamera(needSync);
        //reset the preview size and preview data.
        mPreviewWidth = 0;
        mPreviewHeight = 0;
    }
}
