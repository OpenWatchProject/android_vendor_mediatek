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
package com.mediatek.camera.feature.mode.vsdof.video;

import android.content.ContentValues;
import android.content.Intent;
import android.graphics.Bitmap;
import android.media.CamcorderProfile;
import android.media.MediaCodec;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Build;
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
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraModeBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.storage.IStorageService;
import com.mediatek.camera.common.storage.IStorageService.IStorageStateListener;
import com.mediatek.camera.common.storage.MediaSaver.MediaSaverListener;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.mode.vsdof.video.device.DeviceControllerFactory;
import com.mediatek.camera.feature.mode.vsdof.video.device.ISdofDeviceController;
import com.mediatek.camera.feature.mode.vsdof.video.recorder.IRecorder;
import com.mediatek.camera.feature.mode.vsdof.video.recorder.NormalRecorder;
import com.mediatek.camera.feature.mode.vsdof.video.view.SdofVideoQualitySettingView;
import com.mediatek.camera.feature.mode.vsdof.view.SdofViewCtrl;
import com.mediatek.camera.portability.CameraEx;
import com.mediatek.camera.portability.MediaRecorderEx;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;


/**
 * Sdof video mode implementation.
 */
public class SdofVideoMode extends CameraModeBase {
    private static final Tag TAG = new Tag(SdofVideoMode.class.getSimpleName());
    private static final String PROPERTY_KEY_CLIENT_APP_MODE = "vendor.mtk.client.appmode";
    private static final String APP_MODE_NAME_MTK_DUAL_CAMERA = "MtkStereo";
    private static final String SCENE_MODE_AUTO = "auto-scene-detection";
    private static final String VIDEO_STATUS_KEY = "key_video_status";
    private static final String KEY_STOP_RECORDING = "stop-recording";
    private static final String SCENE_MODE_FIREWORKS = "fireworks";
    private static final String SCENE_MODE_KEY = "key_scene_mode";
    private static final String KEY_RECORDING = "recording";
    private static final String KEY_PREVIEW = "preview";

    private static final int HANDLER_INIT_STATE_MONITOR = 1;
    private static final int HANDLER_INIT_VIDEO_UI = 2;
    private static final int HANDLER_INIT_DEVICE = 3;
    private static final int HANDLER_OPEN_CAMERA = 4;
    private static final int HANDLER_INIT_DEVICE_MANAGER = 6;

    private static final int VIDEO_BAD_PERFORMANCE_AUTO_STOP = 1;
    private static final int VIDEO_RECORDING_NOT_AVAILABLE = 2;
    private static final int BAD_PERFORMANCE_DROP_QUALITY = 3;
    private static final int VIDEO_RECORDING_ERROR = 4;
    private static final int REACH_SIZE_LIMIT = 5;

    private StatusMonitor.StatusResponder mVideoStatusResponder;
    protected ModeState mModeState = ModeState.STATE_PAUSED;
    private Lock mResumePauseLock = new ReentrantLock();
    private Lock mLock = new ReentrantLock();
    private boolean mCanPauseResumeRecording = false;
    private boolean mIsParameterExtraCanUse = false;
    private boolean mIsSetEis25 = false;
    private int mOrientationHint = 0;
    protected Surface mSurface = null;

    protected VideoState mVideoState = VideoState.STATE_UNKNOWN;
    protected ISdofDeviceController mCameraDevice;
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
    private String mLevel;

    private SdofVideoQualitySettingView mSdofVideoQualitySettingView;
    private SdofViewCtrl mSdofViewCtrl = new SdofViewCtrl();

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
        HandlerThread ht = new HandlerThread("Video Handler Thread");
        ht.start();
        mVideoHandler = new VideoHandler(ht.getLooper());
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_STATE_MONITOR);
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_DEVICE);
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_VIDEO_UI);
        mVideoHandler.obtainMessage(HANDLER_OPEN_CAMERA, false).sendToTarget();
        mSdofViewCtrl.setViewChangeListener(mViewChangeListener);
        mSdofViewCtrl.init(app);
        mSdofViewCtrl.showView();
        mSdofViewCtrl.onOrientationChanged(mIApp.getGSensorOrientation());
        mSdofVideoQualitySettingView = new SdofVideoQualitySettingView(
                mVideoQualityChangeListener,
                app.getActivity(), cameraContext.getDataStore());
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        LogHelper.i(TAG, "[resume]");
        super.resume(deviceUsage);
        updateModeState(ModeState.STATE_RESUMED);
        initVideoVariables();
        updateVideoState(VideoState.STATE_UNKNOWN);
        mVideoHandler.sendEmptyMessage(HANDLER_INIT_DEVICE_MANAGER);
        if (!mVideoHandler.hasMessages(HANDLER_INIT_STATE_MONITOR)) {
            mVideoHandler.sendEmptyMessage(HANDLER_INIT_STATE_MONITOR);
        }
        if (!mVideoHandler.hasMessages(HANDLER_OPEN_CAMERA)) {
            mVideoHandler.obtainMessage(HANDLER_OPEN_CAMERA, false).sendToTarget();
        }
        mCameraContext.getStorageService().registerStorageStateListener(mStorageStateListener);
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        LogHelper.i(TAG, "[pause]");
        super.pause(nextModeDeviceUsage);
        updateModeState(ModeState.STATE_PAUSED);
        if (mCameraDevice == null || mVideoHandler == null) {
            LogHelper.e(TAG, "[pause] error return ");
            return;
        }
        mVideoHandler.removeMessages(HANDLER_OPEN_CAMERA);
        mCameraContext.getStorageService().unRegisterStorageStateListener(mStorageStateListener);
        stopRecording(true, true);
        if (mNeedCloseCameraIds != null) {
            mVideoHelper.startChangeModeAnimation(mCameraId);
        }
        mSurface = null;
        if (mNeedCloseCameraIds == null || mNeedCloseCameraIds.size() > 0) {
            mCameraDevice.closeCamera(true);
            mCameraContext.getSettingManagerFactory().recycle(mCameraId);
        } else {
            mCameraDevice.stopPreview();
        }
    }

    @Override
    public void unInit() {
        LogHelper.i(TAG, "[unInit]");
        super.unInit();
        mApp.getAppUi().removeSettingView(mSdofVideoQualitySettingView);
        mSdofViewCtrl.unInit();
        if (mVideoHandler != null) {
            mVideoHandler.getLooper().quit();
        }
        release();
    }

    @Override
    protected ISettingManager getSettingManager() {
        return mSettingManager;
    }

    @Override
    public DeviceUsage getDeviceUsage(@Nonnull DataStore dataStore, DeviceUsage oldDeviceUsage) {
        ArrayList<String> openedCameraIds = new ArrayList<>();
        String cameraId = getCameraIdByFacing(dataStore.getValue(
                KEY_CAMERA_SWITCHER, null, dataStore.getGlobalScope()));
        openedCameraIds.add(cameraId);
        updateModeDefinedCameraApi();
        return new DeviceUsage(DeviceUsage.DEVICE_TYPE_STEREO_VSDOF, mCameraApi, openedCameraIds);
    }

    @Override
    public boolean onUserInteraction() {
        switch (getVideoState()) {
            case STATE_PREVIEW:
            case STATE_UNKNOWN:
            case STATE_PRE_SAVING:
            case STATE_REVIEW_UI:
            case STATE_SAVING:
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
        LogHelper.d(TAG, "video onShutterButtonClick mVideoState = " + mVideoState);
        if (mCameraContext.getStorageService().getRecordStorageSpace() <= 0) {
            LogHelper.d(TAG, "video onShutterButtonClick storage is full");
            return false;
        }
        switch (getVideoState()) {
            case STATE_PREVIEW:
                mAppUi.applyAllUIEnabled(false);
                startRecording();
                return true;
            case STATE_UNKNOWN:
            case STATE_PRE_RECORDING:
            case STATE_PRE_SAVING:
            case STATE_REVIEW_UI:
            case STATE_SAVING:
                return true;
            case STATE_RECORDING:
            case STATE_PAUSED:
                stopRecording(true, false);
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
                stopRecording(true, false);
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
        mSdofViewCtrl.onOrientationChanged(orientation);
    }

    /**
     * sub class maybe override it.
     */
    protected CamcorderProfile getProfile() {
        return mCameraDevice.getCamcorderProfile();
    }

    /**
     * This method maybe override by sub class.
     * then sub class will use itself's recorder to recording.
     */
    protected boolean initRecorder(boolean isStartRecording) {
        LogHelper.d(TAG, "[initRecorder]");
        if (mRecorder != null) {
            mRecorder.reset();
            mRecorder.release();
        }
        mRecorder = new NormalRecorder();
        try {
            mRecorder.init(configRecorderSpec(isStartRecording));
            setMediaRecorderParameters();
            if (CameraDeviceManagerFactory.CameraApi.API1 == mCameraApi) {
                mRecorder.prepare();
            } else {
                initForHal3(isStartRecording);
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            mRecorder.release();
            mRecorder = null;
            return false;
        } catch (RuntimeException e) {
            e.printStackTrace();
            mRecorder.release();
            mRecorder = null;
            return false;
        }
        return true;
    }

    /**
     * This method maybe override by sub class.
     * then sub class will use itself's video ui to show special ui.
     */
    protected void initVideoUi() {
        mVideoUi = mAppUi.getVideoUi();
    }

    /**
     * This method maybe override by sub class.
     * then sub class will use itself's device.
     */
    protected void initCameraDevice(CameraDeviceManagerFactory.CameraApi api) {
        LogHelper.i(TAG, "[initCameraDevice] api = " + api);
        mCameraDevice = DeviceControllerFactory.createDeviceCtroller(mApp.getActivity(), api,
                mCameraContext);
        mCameraDevice.setVsDofLevelParameter(mLevel);
    }

    /**
     * third ap such as mms will override this method and do nothing.
     * because mms not need to save file
     */
    protected void addFileToMediaStore() {
        ContentValues contentValues =
                mVideoHelper.prepareContentValues(true, mOrientationHint, null);
        mCameraContext.getMediaSaver().addSaveRequest(modifyContentValues(contentValues),
                mVideoHelper.getVideoTempPath(), mFileSavedListener);
    }

    /**
     * this method add for override.
     * sub class can use this method to modify recorder spec's items which are
     * different from default spec values.
     *
     * @param recorderSpec
     * @param isRecording
     */
    protected IRecorder.RecorderSpec modifyRecorderSpec(
            IRecorder.RecorderSpec recorderSpec, boolean isRecording) {
        return recorderSpec;
    }

    /**
     * this method add for override
     * sub class can modify default ui spec to show some thing they want.
     * video ui
     *
     * @param spec the ui spec
     * @return the result spec.
     */
    protected IVideoUI.UISpec modifyUISpec(IVideoUI.UISpec spec) {
        return spec;
    }

    /**
     * this method add for override.
     * sub class can modify default contentValues to add extend info and insert them to db.
     *
     * @param contentValues the ContentValues which maybe modified.
     * @return after modify the result contentValues.
     */
    protected ContentValues modifyContentValues(ContentValues contentValues) {
        return contentValues;
    }

    /**
     * use safe way to update VideoState.
     *
     * @param state the new state.
     */
    protected void updateVideoState(VideoState state) {
        LogHelper.d(TAG, "[updateVideoState] new state = " + state + " old state =" + mVideoState);
        mLock.lock();
        try {
            mVideoState = state;
        } finally {
            mLock.unlock();
        }
    }

    /**
     * use safe to get VideoState.
     *
     * @return the video state.
     */
    protected VideoState getVideoState() {
        mLock.lock();
        try {
            return mVideoState;
        } finally {
            mLock.unlock();
        }
    }

    /**
     * mode state as video state additional can handle video flow better.
     *
     * @param state
     */
    private void updateModeState(ModeState state) {
        LogHelper.d(TAG, "[updateModeState] new state = " + state + " old state =" + mModeState);
        mResumePauseLock.lock();
        try {
            mModeState = state;
        } finally {
            mResumePauseLock.unlock();
        }
    }

    /**
     * use safe to get ModeState.
     *
     * @return the mode state.
     */
    private ModeState getModeState() {
        mResumePauseLock.lock();
        try {
            return mModeState;
        } finally {
            mResumePauseLock.unlock();
        }
    }

    /**
     * sub class can override this function to use it's restriction.
     *
     * @return the current restriction
     */
    protected Relation getPreviewedRestriction() {
        Relation relation = VideoRestriction.getPreviewRelation().getRelation(KEY_PREVIEW, true);
        String sceneValue = mSettingManager.getSettingController().queryValue(SCENE_MODE_KEY);
        if (!SCENE_MODE_AUTO.equals(sceneValue) && !SCENE_MODE_FIREWORKS.equals(sceneValue)) {
            relation.addBody(
                    SCENE_MODE_KEY, sceneValue, VideoRestriction.getVideoSceneRestriction());
        }
        String videoQualityId = mSettingManager.getSettingController()
                .queryValue(VideoRestriction.KEY_VIDEO_QUALITY);
        relation.addBody(VideoRestriction.KEY_VIDEO_QUALITY, videoQualityId, videoQualityId);
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
        if (mRecorder != null) {
            mRecorder.release();
            mRecorder = null;
        }
        if (mVideoUi != null) {
            mVideoUi.unInitVideoUI();
            mVideoUi = null;
        }
        if (mCameraDevice != null) {
            mCameraDevice.release();
            mCameraDevice = null;
        }
        mVideoHandler = null;
    }

    protected MediaSaverListener mFileSavedListener = new MediaSaverListener() {
        @Override
        public void onFileSaved(Uri uri) {
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri);
            mApp.notifyNewMedia(uri, true);
            if (getVideoState() == VideoState.STATE_RECORDING ||
                    getVideoState() == VideoState.STATE_PAUSED) {
                return;
            }
            Bitmap bitmap = BitmapCreator.createBitmapFromVideo(mVideoHelper.getVideoFilePath(),
                    mAppUi.getThumbnailViewWidth());
            if (bitmap != null) {
                mAppUi.updateThumbnail(bitmap);
            }
            if (VideoState.STATE_SAVING == getVideoState()) {
                updateVideoState(VideoState.STATE_PREVIEW);
            }
            mAppUi.applyAllUIEnabled(true);
            mAppUi.hideSavingDialog();
            mAppUi.applyAllUIVisibility(View.VISIBLE);
        }
    };

    protected IStorageStateListener mStorageStateListener = new IStorageStateListener() {
        @Override
        public void onStateChanged(int storageState, Intent intent) {
            if (Intent.ACTION_MEDIA_EJECT.equals(intent.getAction())) {
                LogHelper.i(TAG, "[onStateChanged] storage out service Intent.ACTION_MEDIA_EJECT");
                stopRecording(false, false);
            }
        }
    };

    private final SdofVideoQualitySettingView.Listener mVideoQualityChangeListener
            = new SdofVideoQualitySettingView.Listener() {
        @Override
        public void onQualityChanged(String newQuality) {
            mCameraDevice.updateVideoQuality(newQuality);
        }
    };


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
                    LogHelper.i(TAG, "[setProperty] stereo camera mode");
                    CameraEx.setProperty(PROPERTY_KEY_CLIENT_APP_MODE,
                            APP_MODE_NAME_MTK_DUAL_CAMERA);
                    mCameraDevice.openCamera(
                            mSettingManager, mCameraId, false, mRestrictionProvider);
                    break;
                case HANDLER_INIT_STATE_MONITOR:
                    initStatusMonitor();
                    break;
                default:
                    break;
            }
        }
    }

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

    protected ISdofDeviceController.DeviceCallback getPreviewStartCallback() {
        return mPreviewStartCallback;
    }

    private ISdofDeviceController.SettingConfigCallback mSettingConfigCallback =
            new ISdofDeviceController.SettingConfigCallback() {
                @Override
                public void onConfig(Size previewSize) {
                    LogHelper.i(TAG, "[SettingConfigCallback] previewSize = "
                            + previewSize.toString());
                    mVideoHelper.releasePreviewFrameData();
                    mVideoHelper.updateSize(previewSize);
                    mAppUi.setPreviewSize(previewSize.getWidth(), previewSize.getHeight(),
                            mVideoHelper.getSurfaceListener());
                    initRecorderForHal3();
                }
            };
    private ISdofDeviceController.RestrictionProvider mRestrictionProvider =
            new ISdofDeviceController.RestrictionProvider() {
                @Override
                public Relation getRestriction() {
                    return getPreviewedRestriction();
                }
            };

    private void deleteCurrentFile() {
        if (mVideoHelper.getVideoTempPath() != null) {
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
            mVideoUi.showInfo(VIDEO_RECORDING_ERROR);
            updateVideoState(VideoState.STATE_PREVIEW);
            mAppUi.applyAllUIEnabled(true);
            return;
        }
        mCanPauseResumeRecording = false;
        updateModeDeviceState(MODE_DEVICE_STATE_RECORDING);
        mVideoStatusResponder.statusChanged(VIDEO_STATUS_KEY, KEY_RECORDING);
        mVideoHelper.pauseAudioPlayBack(mApp);
        mVideoUi.initVideoUI(configUISpec());
        mAppUi.applyAllUIVisibilityImmediately(View.GONE);
        mAppUi.setUIVisibilityImmediately(IAppUi.INDICATOR, View.VISIBLE);
        mAppUi.setUIVisibilityImmediately(IAppUi.PREVIEW_FRAME, View.VISIBLE);
        mAppUi.setUIVisibilityImmediately(IAppUi.SCREEN_HINT, View.VISIBLE);
        mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_PRE_RECORDING);
        mVideoUi.updateOrientation(mApp.getGSensorOrientation());
        if (CameraDeviceManagerFactory.CameraApi.API2 == mCameraApi) {
            mCameraDevice.startRecording();
        }
        boolean startSuccess = true;
        try {
            mRecorder.start();
            if (CameraDeviceManagerFactory.CameraApi.API1 == mCameraApi) {
                mCameraDevice.startRecording();
            }
        } catch (RuntimeException e) {
            if (mRecorder != null) {
                mRecorder.reset();
                mRecorder.release();
                mRecorder = null;
                if (mCameraDevice.getCamera() != null) {
                    mCameraDevice.lockCamera();
                }
            }
            updateVideoState(VideoState.STATE_PREVIEW);
            mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_PREVIEW);
            mAppUi.applyAllUIVisibility(View.VISIBLE);
            mAppUi.applyAllUIEnabled(true);
            startSuccess = false;
            mVideoUi.showInfo(VIDEO_RECORDING_ERROR);
            e.printStackTrace();
        }
        if (!startSuccess) {
            //update video status when recording fail
            mVideoStatusResponder.statusChanged(VIDEO_STATUS_KEY, KEY_PREVIEW);
            return;
        }
        mApp.enableKeepScreenOn(true);
        updateVideoState(VideoState.STATE_RECORDING);
        if (!mIsParameterExtraCanUse) {
            mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_RECORDING);
        }
        LogHelper.d(TAG, "[startRecording] - ");
    }

    private void stopRecording(boolean isNeedSaveVideo, boolean isFromPause) {
        LogHelper.d(TAG, "[stopRecording]isNeedSaveVideo = " + isNeedSaveVideo
                + "VideoState = " + mVideoState);
        if (getVideoState() == VideoState.STATE_PRE_SAVING
                || getVideoState() == VideoState.STATE_SAVING) {
            return;
        }
        if (getVideoState() == VideoState.STATE_RECORDING
                || getVideoState() == VideoState.STATE_PAUSED) {
            updateVideoState(VideoState.STATE_PRE_SAVING);
            mAppUi.setUIVisibility(IAppUi.INDICATOR, View.INVISIBLE);
            mAppUi.setUIVisibility(IAppUi.PREVIEW_FRAME, View.INVISIBLE);
            mAppUi.setUIVisibility(IAppUi.SCREEN_HINT, View.INVISIBLE);
            mAppUi.showSavingDialog(null, true);
            mVideoHelper.releaseAudioFocus(mApp);
            mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_PREVIEW);
            LogHelper.i(TAG, "mIsSetEis25  = " + mIsSetEis25);
            if (mIsSetEis25) {
                mVideoHelper.stopEis25();
            }
            //update video status when stop recording
            mVideoStatusResponder.statusChanged(VIDEO_STATUS_KEY, KEY_PREVIEW);
            mCameraDevice.stopRecording();
            mCanPauseResumeRecording = false;
            try {
                LogHelper.d(TAG, "[stopRecording] media recorder stop + ");
                mRecorder.stop();
                LogHelper.d(TAG, "[stopRecording] media recorder stop - ");
                if (mIsSetEis25) {
                    mCameraDevice.stopPreview();
                    mCameraDevice.startPreview();
                }
            } catch (RuntimeException e) {
                deleteCurrentFile();
                e.printStackTrace();
                isNeedSaveVideo = false;
                updateVideoState(VideoState.STATE_PREVIEW);
                initRecorderForHal3();
            }
            mAppUi.applyAllUIEnabled(false);
            if (!isNeedSaveVideo) {
                mAppUi.hideSavingDialog();
                mAppUi.applyAllUIVisibility(View.VISIBLE);
                mAppUi.applyAllUIEnabled(true);
                return;
            }
            mApp.enableKeepScreenOn(true);
            updateVideoState(VideoState.STATE_SAVING);
            addFileToMediaStore();
        } else {
            updateVideoState(VideoState.STATE_UNKNOWN);
            deleteCurrentFile();
            return;
        }
        mApp.enableKeepScreenOn(false);
        if (!isFromPause) {
            mCameraDevice.postRecordingRestriction(getRecordedRestriction(false), true);
            updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
            initRecorderForHal3();
        }
        LogHelper.d(TAG, "[stopRecording] -");
    }

    private void initRecorderForHal3() {
        if (CameraDeviceManagerFactory.CameraApi.API2 == mCameraApi) {
            mApp.getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    initRecorder(false);
                }
            });
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
            mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_PAUSE_RECORDING);
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
            mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_RESUME_RECORDING);
            MediaRecorderEx.resume(mRecorder.getMediaRecorder());
        } catch (IllegalStateException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "[resumeRecording] -");
    }

    private boolean isSupportPauseResume() {
        boolean isSupport = false;
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M) {
            isSupport = true;
        }
        LogHelper.d(TAG, "[isSupportPauseResume] isSupport = " + isSupport);
        return isSupport;
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
        spec.isSupportedVss = false;
        spec.pauseResumeListener = mPauseResumeListener;
        spec = modifyUISpec(spec);
        return spec;
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
                mCameraId, getModeKey(), ICameraMode.ModeType.VIDEO, mCameraApi);
    }

    private View.OnClickListener mPauseResumeListener = new View.OnClickListener() {
        public void onClick(View view) {
            LogHelper.d(TAG, "[mPauseResumeListener]  click video mVideoState = " + mVideoState
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

    private View.OnClickListener mStopRecordingListener = new View.OnClickListener() {
        public void onClick(View view) {
            LogHelper.i(TAG, "[mStopRecordingListener] click video state = " + mVideoState);
            stopRecording(true, false);
        }
    };

    private ISdofDeviceController.DeviceCallback mPreviewStartCallback = new
            ISdofDeviceController.DeviceCallback() {
                @Override
                public void onCameraOpened(String cameraId) {
                    initSupportedVideoProfiles();
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
            };

    private MediaRecorder.OnErrorListener mOnErrorListener = new MediaRecorder.OnErrorListener() {
        @Override
        public void onError(MediaRecorder mr, int what, int extra) {
            LogHelper.e(TAG, "[onError] what = " + what + ". extra = " + extra);
            if (MediaRecorder.MEDIA_RECORDER_ERROR_UNKNOWN == what
                    || mVideoHelper.MEDIA_ENCODER_ERROR == extra) {
                stopRecording(true, false);
            }
        }
    };

    private MediaRecorder.OnInfoListener mOnInfoListener = new MediaRecorder.OnInfoListener() {
        @Override
        public void onInfo(MediaRecorder mr, int what, int extra) {
            LogHelper.d(TAG, "MediaRecorder =" + mr + "what = " + what + " extra = " + extra);
            switch (what) {
                case MediaRecorder.MEDIA_RECORDER_INFO_MAX_DURATION_REACHED:
                    stopRecording(true, false);
                    break;
                case MediaRecorder.MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED:
                    stopRecording(true, false);
                    mVideoUi.showInfo(REACH_SIZE_LIMIT);
                    break;
                case VideoHelper.MEDIA_INFO_RECORDING_SIZE:
                    if (getVideoState() == VideoState.STATE_RECORDING) {
                        mVideoUi.updateRecordingSize(extra);
                    }
                    break;
                case VideoHelper.MEDIA_INFO_START_TIMER:
                    mCanPauseResumeRecording = true;
                    mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_RECORDING);
                    if (CameraDeviceManagerFactory.CameraApi.API1 == mCameraApi) {
                        mIsSetEis25 = mVideoHelper.startEis25(mSettingManager, mRecorder);
                    }
                    break;
                case VideoHelper.MEDIA_INFO_WRITE_SLOW:
                    mVideoUi.showInfo(VIDEO_BAD_PERFORMANCE_AUTO_STOP);
                    stopRecording(true, false);
                    break;
                default:
                    break;
            }
        }
    };

    private boolean canSelectCamera(@Nonnull String newCameraId) {
        boolean value = true;
        if (newCameraId == null || mCameraId.equalsIgnoreCase(newCameraId)
                || getVideoState() != VideoState.STATE_PREVIEW) {
            value = false;
        }
        LogHelper.d(TAG, "[canSelectCamera] +: " + value);
        return value;
    }

    private void doCameraSelect(String newCameraId) {
        LogHelper.i(TAG, "[doCameraSelect] + mVideoState = " + mVideoState);
        if (getVideoState() == VideoState.STATE_PREVIEW
                && getModeState() == ModeState.STATE_RESUMED) {
            updateVideoState(VideoState.STATE_UNKNOWN);
            mCameraDevice.closeCamera(true);
            mCameraContext.getSettingManagerFactory().recycle(mCameraId);

            mCameraId = newCameraId;
            mSettingManager = mCameraContext.getSettingManagerFactory().getInstance(
                    mCameraId, getModeKey(), ModeType.VIDEO, mCameraApi);
            mVideoHelper.releasePreviewFrameData();
            mCameraDevice.setPreviewCallback(
                    mVideoHelper.getPreviewFrameCallback(), getPreviewStartCallback());
            mVideoHandler.obtainMessage(HANDLER_OPEN_CAMERA, true).sendToTarget();
        }
    }

    private SdofViewCtrl.ViewChangeListener mViewChangeListener
            = new SdofViewCtrl.ViewChangeListener() {
        @Override
        public void onVsDofLevelChanged(String level) {
            mLevel = level;
            if (mCameraDevice != null) {
                mCameraDevice.setVsDofLevelParameter(level);
            }
        }

        @Override
        public void onTouchPositionChanged(String value) {

        }
    };

    private void initSupportedVideoProfiles() {
        List<String> videoQualitys = new ArrayList<>();
        videoQualitys.add(ISdofDeviceController.QUALITY_FHD);
        // find default video quality
        String videoQuality = mDataStore.getValue(
                SdofVideoQualitySettingView.KEY_SDOF_VIDEO_QUALITY,
                videoQualitys.get(0),
                mDataStore.getGlobalScope());
        // update video quality view
        mSdofVideoQualitySettingView.setEntryValues(videoQualitys);
        mSdofVideoQualitySettingView.setDefaultValue(videoQuality);

        mApp.getAppUi().addSettingView(mSdofVideoQualitySettingView);
        LogHelper.i(TAG, "[initSupportedVideoProfiles] current quality:" + videoQualitys.get(0));
    }
}