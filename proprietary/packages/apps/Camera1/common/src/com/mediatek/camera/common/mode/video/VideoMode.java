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
package com.mediatek.camera.common.mode.video;

import android.content.ContentValues;
import android.content.Intent;
import android.graphics.Bitmap;
import android.media.CamcorderProfile;
import android.media.MediaCodec;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Build;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;
import android.view.View;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.CameraModeBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.video.device.DeviceControllerFactory;
import com.mediatek.camera.common.mode.video.device.IDeviceController;
import com.mediatek.camera.common.mode.video.recorder.IRecorder;
import com.mediatek.camera.common.mode.video.recorder.NormalRecorder;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI.VideoUIState;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.storage.IStorageService;
import com.mediatek.camera.common.storage.IStorageService.IStorageStateListener;
import com.mediatek.camera.common.storage.MediaSaver.MediaSaverListener;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.portability.MediaRecorderEx;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * VideoMode is use for video record,user can extend it
 * to implement itself features.
 */
public class VideoMode extends CameraModeBase {

    private static final Tag TAG = new Tag(VideoMode.class.getSimpleName());
    private static final String SCENE_MODE_AUTO = "auto-scene-detection";
    private static final String VIDEO_STATUS_KEY = "key_video_status";
    private static final String KEY_STOP_RECORDING = "stop-recording";
    private static final String SCENE_MODE_FIREWORKS = "fireworks";
    private static final String SCENE_MODE_KEY = "key_scene_mode";
    private static final String KEY_RECORDING = "recording";
    private static final String KEY_PREVIEW = "preview";

    private static final int HANDLER_INIT_STATE_MONITOR = 1;
    private static final int HANDLER_STOP_RECORDING = 2;
    private static final int HANDLER_INIT_VIDEO_UI = 3;
    private static final int HANDLER_INIT_DEVICE = 4;
    private static final int HANDLER_OPEN_CAMERA = 5;
    private static final int HANDLER_INIT_DEVICE_MANAGER = 6;

    private static final int VIDEO_BAD_PERFORMANCE_AUTO_STOP = 1;
    private static final int VIDEO_RECORDING_NOT_AVAILABLE = 2;
    private static final int VIDEO_RECORDING_ERROR = 4;
    private static final int REACH_SIZE_LIMIT = 5;

    private StatusMonitor.StatusResponder mVideoStatusResponder;
    private ConditionVariable mWaitStopRecording = new ConditionVariable(true);
    private Lock mResumePauseLock = new ReentrantLock();
    private Lock mLock = new ReentrantLock();
    private boolean mCanPauseResumeRecording = false;
    protected boolean mIsParameterExtraCanUse = false;
    private boolean mCanTakePicture = true;
    private boolean mIsSetEis25 = false;
    private int mOrientationHint = 0;

    protected ModeState mModeState = ModeState.STATE_PAUSED;
    protected Surface mSurface = null;

    protected VideoState mVideoState = VideoState.STATE_UNKNOWN;
    protected IDeviceController mCameraDevice;
    protected ISettingManager mSettingManager;
    protected IStorageService mStorageService;
    protected ICameraContext mCameraContext;
    protected VideoHelper mVideoHelper;
    protected Handler mVideoHandler;
    protected IRecorder mRecorder;
    protected IVideoUI mVideoUi;
    protected String mCameraId;
    protected IAppUi mAppUi;
    protected IApp mApp;
    /**
     * enum video state used to indicate the video mode state.
     */
    protected enum VideoState {
        STATE_UNKNOWN,
        STATE_PREVIEW,
        STATE_PRE_RECORDING,
        STATE_RECORDING,
        STATE_PAUSED,
        STATE_PRE_SAVING,
        STATE_SAVING,
        STATE_REVIEW_UI
    }
    /**
     * enum mode state used to indicate the current mode state.
     */
    protected enum ModeState {
        STATE_RESUMED,
        STATE_PAUSED
    }

    @Override
    public void init(@Nonnull IApp app,
                     @Nonnull ICameraContext cameraContext, boolean isFromLaunch) {
        LogHelper.i(TAG, "[init]");
        super.init(app, cameraContext, isFromLaunch);
        mCameraContext = cameraContext;
        mApp = app;
        initVideoVariables();
        doInitMode();
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        LogHelper.i(TAG, "[resume]");
        super.resume(deviceUsage);
        updateModeState(ModeState.STATE_RESUMED);
        doInitDeviceManager();
        initVideoVariables();
        updateVideoState(VideoState.STATE_UNKNOWN);
        doResumeMode();
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        LogHelper.i(TAG, "[pause]");
        super.pause(nextModeDeviceUsage);
        updateModeState(ModeState.STATE_PAUSED);
        doPauseMode(mNeedCloseCameraIds);
    }

    @Override
    public void unInit() {
        LogHelper.i(TAG, "[unInit]");
        super.unInit();
        release();
    }

    @Override
    protected ISettingManager getSettingManager() {
        return mSettingManager;
    }

    @Override
    public boolean onCameraSelected(@Nonnull String newCameraId) {
        LogHelper.i(TAG, "[onCameraSelected] new id = " + newCameraId + " old id = " + mCameraId);
        mAppUi.applyAllUIEnabledImmediately(false);
        if (canSelectCamera(newCameraId)) {
            mVideoHelper.startSwitchCameraAnimation(mCameraId);
            mIApp.getAppUi().onCameraSelected(newCameraId);
            mCameraDevice.setPreviewCallback(null, getPreviewStartCallback());
            doCameraSelect(newCameraId);
            return true;
        } else {
            mAppUi.applyAllUIEnabledImmediately(true);
            return false;
        }
    }

    @Override
    public boolean onUserInteraction() {
        switch (getVideoState()) {
            case STATE_PREVIEW:
            case STATE_UNKNOWN:
            case STATE_PRE_SAVING:
            case STATE_SAVING:
            case STATE_REVIEW_UI:
                super.onUserInteraction();
                return true;
            case STATE_PRE_RECORDING:
            case STATE_RECORDING:
            case STATE_PAUSED:
                return true;
            default:
                return false;
        }
    }

    @Override
    public boolean onShutterButtonClick() {
        LogHelper.i(TAG, "video onShutterButtonClick mVideoState = " + mVideoState);
        switch (getVideoState()) {
            case STATE_PREVIEW:
                if (mCameraContext.getStorageService().getRecordStorageSpace() <= 0) {
                    LogHelper.d(TAG, "video onShutterButtonClick storage is full");
                    return false;
                } else if (mCameraDevice.isReadyForCapture()) {
                    mAppUi.applyAllUIEnabledImmediately(false);
                    startRecording();
                    return true;
                }
            case STATE_UNKNOWN:
            case STATE_PRE_RECORDING:
            case STATE_PRE_SAVING:
            case STATE_REVIEW_UI:
            case STATE_SAVING:
                return true;
            case STATE_RECORDING:
            case STATE_PAUSED:
                mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
                return true;
            default:
                return false;
        }
    }

    @Override
    public boolean onBackPressed() {
        LogHelper.d(TAG, "onBackPressed mVideoState = " + mVideoState);
        switch (getVideoState()) {
            case STATE_UNKNOWN:
            case STATE_SAVING:
            case STATE_PRE_SAVING:
            case STATE_PRE_RECORDING:
                return true;
            case STATE_PAUSED:
            case STATE_RECORDING:
                mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
                return true;
            default:
                return false;
        }
    }

    @Override
    public void onOrientationChanged(int orientation) {
        if (mVideoUi != null) {
            mVideoUi.updateOrientation(orientation);
        }
    }

    protected CamcorderProfile getProfile() {
        return mCameraDevice.getCamcorderProfile();
    }

    protected boolean initRecorder(boolean isStartRecording) {
        LogHelper.d(TAG, "[initRecorder]");
        releaseRecorder();
        mRecorder = new NormalRecorder();
        try {
            mRecorder.init(configRecorderSpec(isStartRecording));
            setMediaRecorderParameters();
            if (CameraApi.API1 == mCameraApi) {
                mRecorder.prepare();
            } else {
                initForHal3(isStartRecording);
            }
        } catch (RuntimeException e) {
            e.printStackTrace();
            releaseRecorder();
            return false;
        }
        return true;
    }

    protected void initVideoUi() {
        mVideoUi = mAppUi.getVideoUi();
    }

    protected void initCameraDevice(CameraApi api) {
        mCameraDevice = DeviceControllerFactory.createDeviceCtroller(
                mApp.getActivity(), api, mCameraContext);
    }

    protected void addFileToMediaStore() {
        ContentValues contentValues
                = mVideoHelper.prepareContentValues(true, mOrientationHint, null);
        mCameraContext.getMediaSaver().addSaveRequest(modifyContentValues(contentValues),
                mVideoHelper.getVideoTempPath(), mFileSavedListener);
    }

    protected IRecorder.RecorderSpec modifyRecorderSpec(
            IRecorder.RecorderSpec recorderSpec, boolean isRecording) {
        return recorderSpec;
    }

    protected IVideoUI.UISpec modifyUISpec(IVideoUI.UISpec spec) {
        return spec;
    }

    protected ContentValues modifyContentValues(ContentValues contentValues) {
        return contentValues;
    }

    protected void updateVideoState(VideoState state) {
        LogHelper.d(TAG, "[updateVideoState] new state = " + state + " old state =" + mVideoState);
        mLock.lock();
        try {
            mVideoState = state;
        } finally {
            mLock.unlock();
        }
    }

    protected VideoState getVideoState() {
        mLock.lock();
        try {
            return mVideoState;
        } finally {
            mLock.unlock();
        }
    }

    private void updateModeState(ModeState state) {
        LogHelper.d(TAG, "[updateModeState] new state = " + state + " old state =" + mModeState);
        mResumePauseLock.lock();
        try {
            mModeState = state;
        } finally {
            mResumePauseLock.unlock();
        }
    }

    private ModeState getModeState() {
        mResumePauseLock.lock();
        try {
            return mModeState;
        } finally {
            mResumePauseLock.unlock();
        }
    }

    protected Relation getPreviewedRestriction() {
        Relation relation = VideoRestriction.getPreviewRelation().getRelation(KEY_PREVIEW, true);
        String sceneValue = mSettingManager.getSettingController().queryValue(SCENE_MODE_KEY);
        if (!SCENE_MODE_AUTO.equals(sceneValue) && !SCENE_MODE_FIREWORKS.equals(sceneValue)) {
            relation.addBody(
                    SCENE_MODE_KEY, sceneValue, VideoRestriction.getVideoSceneRestriction());
        }
        return relation;
    }

    protected List<Relation> getRecordedRestriction(boolean isRecording) {
        List<Relation> relationList = new ArrayList<>();
        if (isRecording) {
            if (!mVideoHelper.isPDAFSupported(mCameraApi)) {
                relationList.add(VideoRestriction.getRecordingRelationForMode()
                        .getRelation(KEY_RECORDING, true));
            }
        } else {
            relationList.add(VideoRestriction.getRecordingRelationForMode()
                    .getRelation(KEY_STOP_RECORDING, true));
        }
        return relationList;
    }

    protected void release() {
        if (mVideoHandler != null) {
            mVideoHandler.getLooper().quit();
            mVideoHandler = null;
        }
        releaseRecorder();
        if (mVideoUi != null) {
            mVideoUi.unInitVideoUI();
            mVideoUi = null;
        }
        if (mCameraDevice != null) {
            mCameraDevice.release();
            mCameraDevice = null;
        }
    }

    protected MediaSaverListener mFileSavedListener = new MediaSaverListener() {
        @Override
        public void onFileSaved(Uri uri) {
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri);
            mApp.notifyNewMedia(uri, true);
            updateThumbnail();
            if (VideoState.STATE_SAVING == getVideoState()) {
                updateVideoState(VideoState.STATE_PREVIEW);
            }
            mAppUi.applyAllUIEnabled(true);
            mAppUi.hideSavingDialog();
            mAppUi.applyAllUIVisibility(View.VISIBLE);
        }
    };

    protected MediaSaverListener mVssSavedListener = new MediaSaverListener() {
        @Override
        public void onFileSaved(Uri uri) {
            LogHelper.d(TAG, "[onFileSaved] mVssSavedListener uri = " + uri);
            mApp.notifyNewMedia(uri, true);
        }
    };

    protected IStorageStateListener mStorageStateListener = new IStorageStateListener() {
        @Override
        public void onStateChanged(int storageState, Intent intent) {
            if (Intent.ACTION_MEDIA_EJECT.equals(intent.getAction())) {
                LogHelper.i(TAG, "[onStateChanged] storage out service Intent.ACTION_MEDIA_EJECT");
                mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
            }
        }
    };

    protected void setMediaRecorderParameters() {
        try {
            int infoNumber = mVideoHelper.MEDIA_INFO.length;
            for (int i = 0; i < infoNumber; i++) {
                MediaRecorderEx.setParametersExtra(
                        mRecorder.getMediaRecorder(),
                        mVideoHelper.RECORDER_INFO_SUFFIX
                                + mVideoHelper.MEDIA_INFO[i]);
            }
            mIsParameterExtraCanUse = true;
            MediaRecorderEx.setVideoBitOffSet(mRecorder.getMediaRecorder(), 1,
                    true);

        } catch (IllegalStateException ex) {
            mIsParameterExtraCanUse = false;
            ex.printStackTrace();
        } catch (RuntimeException ex) {
            mIsParameterExtraCanUse = false;
            ex.printStackTrace();
        } catch (Exception ex) {
            mIsParameterExtraCanUse = false;
            ex.printStackTrace();
        }
    }

    protected IDeviceController.DeviceCallback getPreviewStartCallback() {
        return mPreviewStartCallback;
    }

    /**
     * use to handle some thing for video mode.
     */
    private class VideoHandler extends Handler {
        /**
         * the construction method.
         */
        VideoHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage] msg = " + msg.what);
            switch (msg.what) {
                case HANDLER_INIT_DEVICE:
                    initCameraDevice(mCameraApi);
                    mCameraDevice.setSettingConfigCallback(mSettingConfigCallback);
                    mVideoHelper = new VideoHelper(mCameraContext, mApp, mCameraDevice);
                    mCameraDevice.setPreviewCallback(
                            mVideoHelper.getPreviewFrameCallback(), getPreviewStartCallback());
                    break;
                case HANDLER_INIT_DEVICE_MANAGER:
                    mCameraDevice.queryCameraDeviceManager();
                    break;
                case HANDLER_INIT_VIDEO_UI:
                    initVideoUi();
                    break;
                case HANDLER_OPEN_CAMERA:
                    mCameraDevice.openCamera(
                            mSettingManager, mCameraId, (boolean) msg.obj, mRestrictionProvider);
                    break;
                case HANDLER_INIT_STATE_MONITOR:
                    initStatusMonitor();
                    break;
                case HANDLER_STOP_RECORDING:
                    stopRecording();
                    break;
                default:
                    break;
            }
        }
    }

    private IDeviceController.SettingConfigCallback mSettingConfigCallback =
                    new IDeviceController.SettingConfigCallback() {
        @Override
        public void onConfig(Size previewSize) {
            mVideoHelper.releasePreviewFrameData();
            mVideoHelper.updateSize(previewSize);
            mAppUi.setPreviewSize(previewSize.getWidth(),
                    previewSize.getHeight(), mVideoHelper.getSurfaceListener());
            initRecorderForHal3();
        }
    };
    private IDeviceController.RestrictionProvider mRestrictionProvider
            = new IDeviceController.RestrictionProvider() {
        @Override
        public Relation getRestriction() {
            return getPreviewedRestriction();
        }
    };

    private void updateThumbnail() {
        Bitmap bitmap = BitmapCreator.createBitmapFromVideo(
                mVideoHelper.getVideoFilePath(), mAppUi.getThumbnailViewWidth());
        if (bitmap != null) {
            mAppUi.updateThumbnail(bitmap);
        }
    }

    private void deleteCurrentFile() {
        if (mVideoHelper != null && mVideoHelper.getVideoTempPath() != null) {
            mVideoHelper.deleteVideoFile(mVideoHelper.getVideoTempPath());
        }
    }

    private void startRecording() {
        LogHelper.d(TAG, "[startRecording] + ");
        if (getModeState() == ModeState.STATE_PAUSED) {
            LogHelper.e(TAG, "[startRecording] error mode state is paused");
            return;
        }
        updateVideoState(VideoState.STATE_PRE_RECORDING);
        mCameraDevice.postRecordingRestriction(getRecordedRestriction(true), false);
        if (!initRecorder(true)) {
            initRecorderFail();
            return;
        }
        prepareStartRecording();
        boolean startSuccess = true;
        try {
            mRecorder.start();
            mWaitStopRecording.close();
            if (CameraApi.API1 == mCameraApi) {
                mCameraDevice.startRecording();
            }
        } catch (RuntimeException e) {
            startSuccess = false;
            startRecordingFail();
            e.printStackTrace();
        }
        if (!startSuccess) {
            mVideoStatusResponder.statusChanged(VIDEO_STATUS_KEY, KEY_PREVIEW);
            return;
        }
        mApp.enableKeepScreenOn(true);
        updateVideoState(VideoState.STATE_RECORDING);
        if (!mIsParameterExtraCanUse) {
            mVideoUi.updateUIState(VideoUIState.STATE_RECORDING);
        }
        LogHelper.d(TAG, "[startRecording] - ");
    }

    private void stopRecording() {
        LogHelper.d(TAG, "[stopRecording]+ VideoState = " + mVideoState);
        if (getVideoState() == VideoState.STATE_RECORDING
                || getVideoState() == VideoState.STATE_PAUSED) {
            prepareStopRecording();
            boolean isNeedSaveVideo = true;
            try {
                LogHelper.i(TAG, "[stopRecording] media recorder stop + ");
                mRecorder.stop();
                LogHelper.i(TAG, "[stopRecording] media recorder stop - ");
            } catch (RuntimeException e) {
                deleteCurrentFile();
                e.printStackTrace();
                isNeedSaveVideo = false;
            }
            if (mIsSetEis25 && getModeState() == ModeState.STATE_RESUMED) {
                mCameraDevice.stopPreview();
                mCameraDevice.startPreview();
            }
            doAfterRecorderStoped(isNeedSaveVideo);
            mApp.enableKeepScreenOn(false);
            if (getModeState() == ModeState.STATE_RESUMED) {
                mCameraDevice.postRecordingRestriction(getRecordedRestriction(false), true);
                updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
            }
            mWaitStopRecording.open();
        }
        LogHelper.d(TAG, "[stopRecording] -");
    }

    private void initRecorderFail() {
        mVideoUi.showInfo(VIDEO_RECORDING_ERROR);
        if (mCameraDevice.getCamera() != null) {
            mCameraDevice.lockCamera();
        }
        updateVideoState(VideoState.STATE_PREVIEW);
        mAppUi.applyAllUIEnabled(true);
        mCameraDevice.postRecordingRestriction(getRecordedRestriction(false), true);
    }

    private void prepareStartRecording() {
        updateModeDeviceState(MODE_DEVICE_STATE_RECORDING);
        mVideoStatusResponder.statusChanged(VIDEO_STATUS_KEY, KEY_RECORDING);
        mVideoHelper.pauseAudioPlayBack(mApp);
        mCanTakePicture = true;
        mCanPauseResumeRecording = false;
        mVideoUi.initVideoUI(configUISpec());
        mAppUi.applyAllUIVisibilityImmediately(View.GONE);
        mAppUi.setUIVisibilityImmediately(IAppUi.INDICATOR, View.VISIBLE);
        mAppUi.setUIVisibilityImmediately(IAppUi.PREVIEW_FRAME, View.VISIBLE);
        mAppUi.setUIVisibilityImmediately(IAppUi.SCREEN_HINT, View.VISIBLE);
        if (CameraApi.API2 == mCameraApi) {
            mCameraDevice.startRecording();
        }
        mVideoUi.updateUIState(VideoUIState.STATE_PRE_RECORDING);
        mVideoUi.updateOrientation(mApp.getGSensorOrientation());
    }

    private void startRecordingFail() {
        releaseRecorder();
        if (mCameraDevice.getCamera() != null) {
            mCameraDevice.lockCamera();
        }
        updateVideoState(VideoState.STATE_PREVIEW);
        mVideoUi.updateUIState(VideoUIState.STATE_PREVIEW);
        mAppUi.applyAllUIVisibility(View.VISIBLE);
        mAppUi.applyAllUIEnabled(true);
        mVideoUi.showInfo(VIDEO_RECORDING_ERROR);
    }

    private void prepareStopRecording() {
            updateVideoState(VideoState.STATE_PRE_SAVING);
            mAppUi.applyAllUIEnabled(false);
            mAppUi.setUIVisibility(IAppUi.INDICATOR, View.INVISIBLE);
            mAppUi.setUIVisibility(IAppUi.PREVIEW_FRAME, View.INVISIBLE);
            mAppUi.setUIVisibility(IAppUi.SCREEN_HINT, View.INVISIBLE);
            mAppUi.showSavingDialog(null, true);
            mVideoHelper.releaseAudioFocus(mApp);
            mVideoUi.updateUIState(VideoUIState.STATE_PREVIEW);
            LogHelper.i(TAG, "mIsSetEis25  = " + mIsSetEis25);
            if (mIsSetEis25) {
                mVideoHelper.stopEis25();
            }
            mVideoStatusResponder.statusChanged(VIDEO_STATUS_KEY, KEY_PREVIEW);
            mCameraDevice.stopRecording();
            mCanPauseResumeRecording = false;
    }

    private void doAfterRecorderStoped(boolean isNeedSaveVideo) {
        if (isNeedSaveVideo) {
            mApp.enableKeepScreenOn(true);
            updateVideoState(VideoState.STATE_SAVING);
            addFileToMediaStore();
        } else {
            mAppUi.hideSavingDialog();
            mAppUi.applyAllUIVisibility(View.VISIBLE);
            mAppUi.applyAllUIEnabled(true);
            updateVideoState(VideoState.STATE_PREVIEW);
        }
    }

    private void releaseRecorder() {
        if (mRecorder != null) {
            mRecorder.reset();
            mRecorder.release();
            mRecorder = null;
        }
    }

    private void initRecorderForHal3() {
        if (CameraApi.API2 == mCameraApi) {
            initRecorder(false);
        }
    }

    private void initForHal3(boolean isStartRecording) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            if (mSurface == null) {
                mSurface = MediaCodec.createPersistentInputSurface();
            }
            mRecorder.getMediaRecorder().setInputSurface(mSurface);
        }
        try {
            mRecorder.prepare();
        } catch (RuntimeException e) {
            if (!isStartRecording) {
                mCameraDevice.configCamera(null, false);
            }
            throw new RuntimeException(e);
        }
        if (mSurface != null) {
            mCameraDevice.configCamera(mSurface, isStartRecording);
        } else {
            mCameraDevice.configCamera(mRecorder.getSurface(), isStartRecording);
        }
    }

    private void pauseRecording() {
        LogHelper.d(TAG, "[pauseRecording] +");
        try {
            mVideoUi.updateUIState(VideoUIState.STATE_PAUSE_RECORDING);
            MediaRecorderEx.pause(mRecorder.getMediaRecorder());
        } catch (IllegalStateException e) {
            mVideoUi.showInfo(VIDEO_RECORDING_NOT_AVAILABLE);
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[pauseRecording] -");
    }

    private void resumeRecording() {
        LogHelper.d(TAG, "[resumeRecording] +");
        try {
            mVideoUi.updateUIState(VideoUIState.STATE_RESUME_RECORDING);
            MediaRecorderEx.resume(mRecorder.getMediaRecorder());
        } catch (IllegalStateException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[resumeRecording] -");
    }

    private boolean isSupportPauseResume() {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M) {
            return true;
        }
        return false;
    }

    private IRecorder.RecorderSpec configRecorderSpec(boolean isStartRecording) {
        IRecorder.RecorderSpec recorderSpec = mVideoHelper.configRecorderSpec(
                getProfile(), mCameraId, mCameraApi, mSettingManager);
        mOrientationHint = recorderSpec.orientationHint;
        recorderSpec.infoListener = mOnInfoListener;
        recorderSpec.errorListener = mOnErrorListener;
        recorderSpec.releaseListener = mOnInfoListener;
        recorderSpec = modifyRecorderSpec(recorderSpec, isStartRecording);
        return recorderSpec;
    }

    private IVideoUI.UISpec configUISpec() {
        IVideoUI.UISpec spec = new IVideoUI.UISpec();
        spec.isSupportedPause = isSupportPauseResume();
        spec.recordingTotalSize = 0;
        spec.stopListener = mStopRecordingListener;
        spec.isSupportedVss = isVssSupported();
        spec.vssListener = mVssListener;
        spec.pauseResumeListener = mPauseResumeListener;
        spec = modifyUISpec(spec);
        return spec;
    }

    private boolean isVssSupported() {
        return mCameraDevice.isVssSupported(Integer.parseInt(mCameraId));
    }

    private void initStatusMonitor() {
        mVideoStatusResponder = mCameraContext.getStatusMonitor(mCameraId)
                .getStatusResponder(VIDEO_STATUS_KEY);
    }

    private void initVideoVariables() {
        mAppUi = mApp.getAppUi();
        mCameraId = getCameraIdByFacing(mDataStore.getValue(
                KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));
        mStorageService = mCameraContext.getStorageService();
        mSettingManager = mCameraContext.getSettingManagerFactory().getInstance(
                mCameraId, getModeKey(), ModeType.VIDEO, mCameraApi);
    }

    private void doInitMode() {
        HandlerThread ht = new HandlerThread("Video Handler Thread");
        ht.start();
        mVideoHandler = new VideoHandler(ht.getLooper());
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_STATE_MONITOR);
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_DEVICE);
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_VIDEO_UI);
        mVideoHandler.obtainMessage(HANDLER_OPEN_CAMERA, false).sendToTarget();
    }

    private void doInitDeviceManager() {
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_DEVICE_MANAGER);
    }

    private void doResumeMode() {
        if (!mVideoHandler.hasMessages(HANDLER_INIT_STATE_MONITOR)) {
            mVideoHandler.sendEmptyMessage(HANDLER_INIT_STATE_MONITOR);
        }
        if (!mVideoHandler.hasMessages(HANDLER_OPEN_CAMERA)) {
            mVideoHandler.obtainMessage(HANDLER_OPEN_CAMERA, false).sendToTarget();
        }
        mCameraContext.getStorageService().registerStorageStateListener(mStorageStateListener);
    }

    private void pauseForRecorder() {
        if (getVideoState() == VideoState.STATE_RECORDING
                || getVideoState() == VideoState.STATE_PAUSED
                || getVideoState() == VideoState.STATE_PRE_RECORDING) {
            mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
        } else if (getVideoState() == VideoState.STATE_REVIEW_UI
                || getVideoState() == VideoState.STATE_PREVIEW) {
            updateVideoState(VideoState.STATE_UNKNOWN);
        }
    }

    private void pauseForDevice(ArrayList<String> needCloseCameraIdList) {
        mWaitStopRecording.block();
        if (needCloseCameraIdList == null || needCloseCameraIdList.size() > 0) {
            mCameraDevice.preventChangeSettings();
            mCameraDevice.closeCamera(true);
            mCameraContext.getSettingManagerFactory().recycle(mCameraId);
        } else {
            // this animation is no need right now so just mark it
            // if some day need can open it.
            // mVideoHelper.startChangeModeAnimation(mCameraId);
            mCameraDevice.stopPreview();
        }
    }

    private void doPauseMode(ArrayList<String> needCloseCameraIdList) {
        mVideoHandler.removeMessages(HANDLER_OPEN_CAMERA);
        if (mCameraDevice != null) {
            mCameraContext.getStorageService().unRegisterStorageStateListener(
                    mStorageStateListener);
            pauseForRecorder();
            pauseForDevice(needCloseCameraIdList);
            mSurface = null;
        }
    }

    private View.OnClickListener mPauseResumeListener = new View.OnClickListener() {
        public void onClick(View view) {
            LogHelper.d(TAG, "[mPauseResumeListener] click video mVideoState = " + mVideoState
                    + " mCanPauseResumeRecording = " + mCanPauseResumeRecording);
            if (mCanPauseResumeRecording) {
                if (getVideoState() == VideoState.STATE_RECORDING) {
                    pauseRecording();
                    updateVideoState(VideoState.STATE_PAUSED);
                } else if (getVideoState() == VideoState.STATE_PAUSED) {
                    resumeRecording();
                    updateVideoState(VideoState.STATE_RECORDING);
                }
            }
        }
    };

    private View.OnClickListener mVssListener = new View.OnClickListener() {
        public void onClick(View view) {
            LogHelper.i(TAG, "[mVssListener] click video state = "
                                + mVideoState + "mCanTakePicture = " + mCanTakePicture);
            if ((getVideoState() == VideoState.STATE_PAUSED ||
                    getVideoState() == VideoState.STATE_RECORDING) && mCanTakePicture) {
                mAppUi.animationStart(IAppUi.AnimationType.TYPE_CAPTURE, null);
                mCameraDevice.updateGSensorOrientation(mApp.getGSensorOrientation());
                mCameraDevice.takePicture(mJpegCallback);
                mCanTakePicture = false;
            }
        }
    };

    private IDeviceController.JpegCallback mJpegCallback = new IDeviceController.JpegCallback() {
        @Override
        public void onDataReceived(byte[] jpegData) {
            LogHelper.d(TAG, "[onDataReceived] jpegData = " + jpegData);
            if (jpegData != null) {
                int orientation = CameraUtil.getOrientationFromExif(jpegData);
                Size size = CameraUtil.getSizeFromExif(jpegData);
                mCameraContext.getMediaSaver().addSaveRequest(jpegData,
                        mVideoHelper.prepareContentValues(false, orientation, size),
                        null, mVssSavedListener);
            }
            mCanTakePicture = true;
        }
    };

    private View.OnClickListener mStopRecordingListener = new View.OnClickListener() {
        public void onClick(View view) {
            LogHelper.i(TAG, "[mStopRecordingListener] click video state = " + mVideoState);
            mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
        }
    };

    private IDeviceController.DeviceCallback mPreviewStartCallback = new
            IDeviceController.DeviceCallback() {
                @Override
                public void onCameraOpened(String cameraId) {
                    updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
                }

                @Override
                public void afterStopPreview() {
                    updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
                }

                @Override
                public void beforeCloseCamera() {
                    updateModeDeviceState(MODE_DEVICE_STATE_CLOSED);
                }

                @Override
                public void onPreviewStart() {
                    if (getModeState() == ModeState.STATE_PAUSED) {
                        LogHelper.e(TAG, "[onPreviewStart] error mode state is paused");
                        return;
                    }
                    updateVideoState(VideoState.STATE_PREVIEW);
                    mAppUi.applyAllUIEnabled(true);
                    updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
                    LogHelper.d(TAG, "[onPreviewStart]");
                }

                @Override
                public void onError() {
                    if (getVideoState() == VideoState.STATE_PAUSED
                            || getVideoState() == VideoState.STATE_RECORDING) {
                        mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
                    }
                }
            };

    private MediaRecorder.OnErrorListener mOnErrorListener = new MediaRecorder.OnErrorListener() {
        @Override
        public void onError(MediaRecorder mr, int what, int extra) {
            LogHelper.e(TAG, "[onError] what = " + what + ". extra = " + extra);
            if (MediaRecorder.MEDIA_RECORDER_ERROR_UNKNOWN == what
                        || mVideoHelper.MEDIA_ENCODER_ERROR == extra) {
                mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
            }
        }
    };

    private MediaRecorder.OnInfoListener mOnInfoListener = new MediaRecorder.OnInfoListener() {
        @Override
        public void onInfo(MediaRecorder mr, int what, int extra) {
            LogHelper.d(TAG, "MediaRecorder =" + mr + "what = " + what + " extra = " + extra);
            switch (what) {
                case MediaRecorder.MEDIA_RECORDER_INFO_MAX_DURATION_REACHED:
                    mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
                    break;
                case MediaRecorder.MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED:
                    mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
                    mVideoUi.showInfo(REACH_SIZE_LIMIT);
                    break;
                case VideoHelper.MEDIA_INFO_RECORDING_SIZE:
                    if (getVideoState() == VideoState.STATE_RECORDING) {
                        mVideoUi.updateRecordingSize(extra);
                    }
                    break;
                case VideoHelper.MEDIA_INFO_START_TIMER:
                    mCanPauseResumeRecording = true;
                    mVideoUi.updateUIState(VideoUIState.STATE_RECORDING);
                    if (CameraApi.API1 == mCameraApi) {
                        mIsSetEis25 = mVideoHelper.startEis25(mSettingManager, mRecorder);
                    }
                    mAppUi.setUIEnabled(mAppUi.SHUTTER_BUTTON, true);
                    break;
                case VideoHelper.MEDIA_INFO_WRITE_SLOW:
                    mVideoUi.showInfo(VIDEO_BAD_PERFORMANCE_AUTO_STOP);
                    mVideoHandler.sendEmptyMessage(HANDLER_STOP_RECORDING);
                    break;
                default:
                    break;
            }
        }
    };

    private boolean canSelectCamera(@Nonnull String newCameraId) {
        if (newCameraId == null || mCameraId.equalsIgnoreCase(newCameraId)
                || getVideoState() != VideoState.STATE_PREVIEW) {
            return false;
        }
        return true;
    }

    private void doCameraSelect(String newCameraId) {
        LogHelper.i(TAG, "[doCameraSelect] + mVideoState = " + mVideoState);
        if (getVideoState() == VideoState.STATE_PREVIEW
                && getModeState() == ModeState.STATE_RESUMED) {
            mCameraDevice.preventChangeSettings();
            updateVideoState(VideoState.STATE_UNKNOWN);
            mCameraDevice.closeCamera(true);
            mCameraContext.getSettingManagerFactory().recycle(mCameraId);
            mCameraId = newCameraId;
            mSettingManager = mCameraContext.getSettingManagerFactory().getInstance(
                    mCameraId, getModeKey(), ModeType.VIDEO, mCameraApi);
            mVideoHelper.releasePreviewFrameData();
            mCameraDevice.setPreviewCallback(
                    mVideoHelper.getPreviewFrameCallback(), getPreviewStartCallback());
            mVideoHandler.obtainMessage(HANDLER_OPEN_CAMERA, false).sendToTarget();
        }
    }

}
