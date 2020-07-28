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

package com.mediatek.camera.mode.pip;

import android.graphics.SurfaceTexture;
import android.hardware.Camera.Size;
import android.location.Location;
import android.media.MediaActionSound;
import android.media.MediaRecorder;
import android.os.ConditionVariable;
import android.os.SystemClock;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.View.OnClickListener;

import com.android.camera.R;
//TODO: can not reference the Google package
import com.android.camera.Storage;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.ICameraAddition.AdditionActionType;
import com.mediatek.camera.ICameraMode.ModeState;
import com.mediatek.camera.ISettingRule.MappingFinder;
import com.mediatek.camera.mode.VideoMode;
import com.mediatek.camera.mode.pip.recorder.MediaRecorderWrapper;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.IFileSaver.FILE_TYPE;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class PipVideoMode extends VideoMode implements MediaRecorderWrapper.OnInfoListener,
        PipController.Listener, ICameraAppUi.GestureListener {
    private static final String TAG = "PipVideoMode";
    private static final String OVERRIDE_ZSD_VALUE = "off";

    private boolean mIsRecordingStarted = false;
    private boolean mStopVideoRecording = false;
    private static final Long VIDEO_4G_SIZE = 4 * 1024 * 1024 * 1024L;

    private ConditionVariable mStopConditionVariableSync = new ConditionVariable();
    private MediaRecorderWrapper mRecorder;
    private PipController mPipController;

    public PipVideoMode(ICameraContext cameraContext) {
        super(cameraContext);
        Log.d(TAG, "[PipVideoMode]constructor...");

        mRecordingView.setListener(mPIPVideoPauseResumeListner);
        mICameraAppUi.setGestureListener(this);
        mPipController = PipController.instance(cameraContext.getActivity());
        mPipController.init(cameraContext, this);
        mStopConditionVariableSync.open();
        mCameraSound.load(MediaActionSound.START_VIDEO_RECORDING);
        setPipSettingRules(mICameraContext);
        setAntiFlickerRules(mICameraContext);
        setZsdRules(mICameraContext);
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        Log.d(TAG, "[execute]type = " + type);
        mAdditionManager.execute(type, true, arg);
        switch (type) {
        case ACTION_ON_CAMERA_OPEN:
            super.updateDevice();
            break;

        case ACTION_ON_CAMERA_PARAMETERS_READY:
            doOnCameraParameterReady(((Boolean) arg[0]).booleanValue());
            if (ModeState.STATE_RECORDING != getModeState()) {
                setModeState(ModeState.STATE_IDLE);
            }
            break;

        case ACTION_PHOTO_SHUTTER_BUTTON_CLICK:
            takeASnapshot();
            break;

        case ACTION_NOTIFY_SURFCEVIEW_DESTROYED:
            if (mPipController != null) {
                mPipController.notifySurfaceViewDestroyed((Surface) arg[0]);
            }
            break;

        case ACTION_VIDEO_SHUTTER_BUTTON_CLICK:
            onVideoShutterButtonClick();
            break;
        case ACTION_SHUTTER_BUTTON_FOCUS:
            boolean pressed = ((Boolean) arg[0]).booleanValue();
            Log.d(TAG, "ispressed  = " + pressed);
            if (!pressed) {
                mIFocusManager.onShutterUp();
            }
            break;

        case ACTION_ON_LONG_PRESS:
            if (mICameraDevice != null
                && mICameraDevice.getParameters() != null
                && mICameraDevice.getParameters().FOCUS_MODE_AUTO.equals(
                        mIFocusManager.getFocusMode())) {
                mIFocusManager.cancelAutoFocus();
            }
            break;

        case ACTION_ORITATION_CHANGED:
            if (mPipController != null) {
                mPipController.onGSensorOrientationChanged((Integer) arg[0]);
            }
            break;

        case ACTION_ON_COMPENSATION_CHANGED:
            if (mPipController != null) {
                mPipController.onViewOrienationChanged((Integer) arg[0]);
            }
            if (mRecordingView != null) {
                mRecordingView.onOrientationChanged((Integer) arg[0]);
            }
            break;

        case ACTION_ON_SINGLE_TAP_UP:
            if (arg[0] != null && arg[1] != null && arg[2] != null) {
                onSingleTapUp((View) arg[0], (Integer) arg[1], (Integer) arg[2]);
            }
            break;

        case ACTION_ON_CAMERA_CLOSE:
            onCameraClose();
            if (mPipController != null) {
                mPipController.pause();
            }
            break;

        case ACTION_ON_MEDIA_EJECT:
            onMediaEject();
            break;

        case ACTION_ON_RESTORE_SETTINGS:
            onRestoreSettings();
            break;

        case ACTION_ON_KEY_EVENT_PRESS:
            return onKeyDown((Integer) arg[0], (KeyEvent) arg[1]);

        case ACTION_ON_BACK_KEY_PRESS:
            return onBackPressed();

        case ACTION_ON_USER_INTERACTION:
            return onUserInteraction();

        case ACTION_SET_DISPLAYROTATION:
            setDisplayRotation((Integer) arg[0]);
            break;

        case ACTION_ON_PREVIEW_BUFFER_SIZE_CHANGED:
            onPreviewBufferSizeChanged((Integer) arg[0], (Integer) arg[1]);
            break;

        case ACTION_ON_STOP_PREVIEW:
            stopPreview();
            break;

        case ACTION_SWITCH_DEVICE:
            switchDevice();
            return false;

        case ACTION_NOTIFY_SURFCEVIEW_DISPLAY_IS_READY:
            mPipController.setPreviewSurface(mIModuleCtrl.getPreviewSurface());
            break;

        default:
            return false;
        }

        return true;
    }

    @Override
    public void resume() {
        Log.d(TAG, "[resume]");
        if (mPipController != null) {
            mPipController.resume();
        }
    }

    @Override
    public void pause() {
        Log.d(TAG, "[pause]");
        if (mPipController != null) {
            mPipController.pause();
        }
    }

    @Override
    public boolean close() {
        Log.d(TAG, "[close]...");
        if (mIFocusManager != null) {
            // mIFocusManager.removeMessages();TODO
        }
        if (mICameraDevice != null) {
            mICameraDevice.cancelAutoFocus();
        }
        if (mRecordingView != null) {
            mRecordingView.uninit();
        }
        // pip <--> pip, there is no need to recreate pip controller
        if (mPipController != null
                && CameraModeType.EXT_MODE_PHOTO_PIP != mIModuleCtrl.getNextMode()) {
            mPipController.unInit(mICameraContext.getActivity());
            mPipController = null;
        }
        mIsAutoFocusCallback = false;
        mIFocusManager = null;
        mIsMediaRecoderRecordingPaused = false;
        return true;
    }

    @Override
    public CameraModeType getCameraModeType() {
        return CameraModeType.EXT_MODE_VIDEO_PIP;
    }

    @Override
    public void doStartPreview() {
        Log.d(TAG, "[doStartPreview()]");
        mICameraDeviceManager.getCameraDevice(mICameraDeviceManager.getCurrentCameraId())
                .startPreview();
        mICameraDeviceManager.getCameraDevice(getTopCameraId()).startPreview();
        mIFocusManager.onPreviewStarted();
        mAdditionManager.execute(AdditionActionType.ACTION_ON_START_PREVIEW);
    }

    @Override
    public void stopPreview() {
        Log.d(TAG, "[stopPreview()]");
        if (mPipController != null) {
            mPipController.stopSwitchPip();
        }
        if (mICameraDeviceManager.getCameraDevice(
                mICameraDeviceManager.getCurrentCameraId()) != null) {
            mICameraDeviceManager.getCameraDevice(mICameraDeviceManager.getCurrentCameraId())
            .stopPreview();
        }
        if (mICameraDeviceManager.getCameraDevice(getTopCameraId()) != null) {
            mICameraDeviceManager.getCameraDevice(getTopCameraId()).stopPreview();
        }
        mAdditionManager.execute(AdditionActionType.ACTION_ON_STOP_PREVIEW);
    }

    @Override
    public boolean onVideoShutterButtonClick() {
        Log.d(TAG, "[Video.onShutterButtonClick] (" + ") mMediaRecorderRecording="
                + mIsMediaRecorderRecording);
        if (ModeState.STATE_CLOSED == getModeState()) {
            Log.i(TAG, "[Video.onShutterButtonClick]mode state is closed,return");
            return false;
        }
        if (mIsMediaRecorderRecording) {
            stopVideoRecordingAsync(true);
        } else {
            // Do not recording if there is not enough storage.
            if (Storage.getLeftSpace() <= 0) {
                backToLastModeIfNeed();
                Log.i(TAG, "[Video.onShutterButtonClick]current left space is full,return");
                return false;
            }
            mCameraSound.play(MediaActionSound.START_VIDEO_RECORDING);
            mICameraAppUi.setSwipeEnabled(false);
            startVideoRecording();
            // we should enable swiping when cannot record
            if (!mIsMediaRecorderRecording) {
                mICameraAppUi.setSwipeEnabled(true);
            }
            // because pip video is not a video mode for native, so auto focus
            // should be triggered by ap ,if current focus mode is auto focus,
            // do autofocus
            if (mICameraDeviceManager.getCameraDevice(mICameraDeviceManager.getCurrentCameraId())
                    .getParameters().FOCUS_MODE_AUTO.equals(mIFocusManager.getFocusMode())) {
                autoFocus();
            }
        }
        return true;
    }

    @Override
    public void initializeNormalRecorder() {
        Log.d(TAG, "[initializeRecorder]");
        initializePipRecorder();
        if (mRecorder == null) {
            Log.e(TAG, "[initializeRecorder] Fail to initialize media recorder.", new Throwable());
            return;
        }
    }

    @Override
    public boolean startNormalRecording() {
        Log.d(TAG, "[startNormalRecording()]");
        boolean isSuccess = true;
        try {
            mPipController.startPushVideoBuffer();
            mRecorder.start();
            // mediaRecorder will set video-size to parameters ,so after start
            // recording use fetchParametersFromServer to update parameters to
            // make sure Camera ap's parameters is the latest
            Log.d(TAG, "fetchParametersFromServer begin");
            mICameraDeviceManager.getCameraDevice(mICameraDeviceManager.getCurrentCameraId())
                    .fetchParametersFromServer();
            mICameraDeviceManager.getCameraDevice(getTopCameraId()).fetchParametersFromServer();
            Log.d(TAG, "fetchParametersFromServer end");
        } catch (RuntimeException e) {
            Log.e(TAG, "[startNormalRecording()] Could not start media recorder. ", e);
            isSuccess = false;
            releaseMediaRecorder();
        }
        return isSuccess;
    }

    @Override
    protected void updateViewState(boolean hide) {
        super.updateViewState(hide);
        if (mPipController != null) {
            mPipController.hideModeViews(hide);
        }
    }

    @Override
    public void stopVideoRecordingAsync(boolean isShowSaving) {
        mStopVideoRecording = true;
        Log.d(TAG, "[stopVideoRecordingAsync()] mMediaRecorderRecording="
                + mIsMediaRecorderRecording + ", isVideoProcessing()" + isVideoProcessing()
                + ", mStopVideoRecording =" + mStopVideoRecording);
        mICameraAppUi.changeZoomForQuality();
        mICameraAppUi.setSwipeEnabled(true);
        mHandler.removeMessages(UPDATE_RECORD_TIME);
        mICameraAppUi.setVideoShutterMask(false);
        if (ModeState.STATE_SAVING == getModeState()) {
            return;
        }
        setModeState(ModeState.STATE_SAVING);
        mRecordingView.hide();
        if (mIsMediaRecorderRecording) {
            mICameraAppUi.setVideoShutterEnabled(false);
            if (isShowSaving) { // TODO isShowSaving should always true
                mICameraAppUi.showProgress(mActivity.getResources().getString(R.string.saving));
            }
            mVideoSavingTask = new SavingTask();
            mVideoSavingTask.start();
        } else {
            releaseMediaRecorder();
            if (mStoppingAction == STOP_RETURN_UNVALID) {
                mVideoModeHelper.doReturnToCaller(false, mCurrentVideoUri);
            }
        }
        if (isVideoProcessing()) {
            mStopConditionVariableSync.close();
        }
    }

    @Override
    public boolean takeASnapshot() {
        if (mStopVideoRecording) {
            return false;
        }
        Log.d(TAG, "[takeASnapshot] Video snapshot ");
        mPipController.takeVideoSnapshot(mIModuleCtrl.getOrientation(), isBackBottom());
        if (!mIModuleCtrl.isVideoCaptureIntent()) {
            mICameraAppUi.updateSnapShotUIView(true);
        }
        return true;
    }

    @Override
    public void onPIPPictureTaken(byte[] jpegData) {
        Log.d(TAG, "[onPIPPictureTaken]");
        if (jpegData == null) {
            Log.i(TAG, "[onPIPPictureTaken] jpegData is null return");
            return;
        }
        mIFileSaver.init(FILE_TYPE.JPEG, 0, null, -1);
        long time = System.currentTimeMillis();
        String title = Util.createNameFormat(time,
                mActivity.getString(R.string.image_file_name_format))
                + ".jpg";
        mIFileSaver.savePhotoFile(jpegData, null, time, mIModuleCtrl.getLocation(), 0, null);
        mHandler.sendEmptyMessage(UPDATE_SNAP_UI);
    }

    @Override
    public void stopRecording() {
        Log.d(TAG, "[stopRecording] begin");
        mPipController.stopPushVideoBuffer();
        // stop receive video frame,play stop sound
        playSound(MediaActionSound.START_VIDEO_RECORDING);
        // stop audio encoder, video encoder, media muxer
        try {
            mRecorder.stop();
        } catch (RuntimeException e) {
            throw e;
        } finally {
            mStopConditionVariableSync.open();
        }
        Log.d(TAG, "[stopRecording] end");
    }

    @Override
    public void doAfterStopRecording(boolean fail) {
        if (!mIModuleCtrl.isNonePickIntent()) {
            if (!fail && mStoppingAction != STOP_RETURN_UNVALID) {
                if (mIModuleCtrl.isQuickCapture()) {
                    mStoppingAction = STOP_RETURN;
                } else {
                    mStoppingAction = STOP_SHOW_ALERT;
                }
            }
        } else if (fail) {
            mStoppingAction = STOP_FAIL;
        }
        // always release media recorder
        releaseMediaRecorder();
        addVideoToMediaStore();
        synchronized (mVideoSavingTask) {
            mVideoSavingTask.notifyAll();
            mHandler.removeCallbacks(mVideoSavedRunnable);
            mHandler.post(mVideoSavedRunnable);
        }
    }

    @Override
    public void addVideoToMediaStore() {
        if (mVideoFileDescriptor == null) {
            mIFileSaver.init(
                    FILE_TYPE.PIPVIDEO,
                    mProfile.fileFormat,
                    Integer.toString(mProfile.videoFrameWidth) + "x"
                            + Integer.toString(mProfile.videoFrameHeight), Util
                            .getRecordingRotation(mIModuleCtrl.getOrientation(),
                                    mICameraDeviceManager.getCurrentCameraId(),
                                    mICameraDeviceManager.getCameraInfo(mICameraDeviceManager
                                            .getCurrentCameraId())));

            mIFileSaver.saveVideoFile(mIModuleCtrl.getLocation(), mVideoTempPath,
                    computeDuration(), 0, mFileSavedListener);
        }
    }

    @Override
    protected void releaseMediaRecorder() {
        Log.d(TAG, "[releaseMediaRecorder()] mRecorder=" + mRecorder
                + " mRecorderCameraReleased = " + mIsRecorderCameraReleased);
        if (mRecorder != null && !mIsRecorderCameraReleased) {
            cleanupEmptyFile();
            mRecorder.release();
            mIsRecorderCameraReleased = true;
            mHandler.post(mReleaseOnInfoListener);
        }
        mVideoFilename = null;
    }

    @Override
    protected void pauseVideoRecording() {
        Log.d(TAG, "[pauseVideoRecording()] mMediaRecorderRecording =" + mIsMediaRecorderRecording
                + " mMediaRecoderRecordingPaused = " + mIsMediaRecoderRecordingPaused);
        if (canDoPauseResumeAction()) {
            mRecordingView.setRecordingIndicator(false);
            try {
                mRecorder.pause(mRecorder);
            } catch (IllegalStateException e) {
                Log.e(TAG, "[pauseVideoRecording()]  Could not pause media recorder. ");
            }
            mRecordingPausedDuration = SystemClock.uptimeMillis() - mRecordingStartTime;
            mIsMediaRecoderRecordingPaused = true;
        }
    }

    @Override
    protected void waitForRecorder() {
        // pip mode should not wait recorder, to not blocking main thread
        // instead to show saving progress dialog
        // but should wait stop command is done, because stop recoding need a
        // frame before this stop preview should not be called
        mStopConditionVariableSync.block();
        Log.d(TAG, "[waitForRecorder] end");
    }

    @Override
    protected void updateRecordingTime() {
        super.updateRecordingTime();
        Log.d(TAG, "mTotalRecordingDuration = " + mTotalRecordingDuration);
    }

    // Video PIP, VSS always enable
    @Override
    public void initializeShutterStatus() {
        mICameraAppUi.setPhotoShutterEnabled(true);
    }

    @Override
    public boolean isDisplayUseSurfaceView() {
        Log.d(TAG, "[isUseSurfaceView]");
        return true;
    }

    @Override
    public boolean isDeviceUseSurfaceView() {
        Log.d(TAG, "[isDeviceUseSurfaceView]");
        return false;
    }

    @Override
    public boolean isRestartCamera() {
        Log.d(TAG, "[isRestartCamera]");
        return true;
    }

    @Override
    public boolean isNeedDualCamera() {
        Log.d(TAG, "[isNeedDualCamera]");
        return true;
    }

    @Override
    public int getGSensorOrientation() {
        return mIModuleCtrl.getOrientation();
    }

    @Override
    public int getViewRotation() {
        return mIModuleCtrl.getOrientationCompensation();
    }

    @Override
    public void canDoStartPreview() {

    };

    @Override
    public int getButtomGraphicCameraId() {
        return mICameraDeviceManager.getCurrentCameraId();
    }

    @Override
    public void switchPIP() {
        Log.d(TAG, "switchPIP");
        if (mIFocusManager != null) {
            mIFocusManager.cancelAutoFocus();
        }
        mIModuleCtrl.switchCameraDevice();
        updateDevice();
    }

    @Override
    public SurfaceTexture getBottomSurfaceTexture() {
        Log.d(TAG, "[getBottomSurfaceTexture]");
        return mPipController.getBottomSurfaceTexture();
    }

    @Override
    public SurfaceTexture getTopSurfaceTexture() {
        Log.d(TAG, "[getTopSurfaceTexture]");
        return mPipController.getTopSurfaceTexture();
    }

    /***************************** Gesture Listener **************************************/
    @Override
    public boolean onDown(float x, float y, int width, int height) {
        if (mPipController != null && getModeState() != ModeState.STATE_FOCUSING) {
            return mPipController.onDown(x, y, width, height);
        }
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        return false;
    }

    @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        if (mPipController != null) {
            return mPipController.onScroll(dx, dy, totalX, totalY);
        }
        return false;
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        if (mPipController != null && getModeState() != ModeState.STATE_FOCUSING) {
            mAdditionManager.execute(AdditionActionType.ACTION_ON_SWITCH_PIP);
            return mPipController.onSingleTapUp(x, y);
        }
        return false;
    }

    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        return false;
    }

    @Override
    public boolean onUp() {
        if (mPipController != null) {
            return mPipController.onUp();
        }
        return false;
    }

    @Override
    public boolean onDoubleTap(float x, float y) {
        return false;
    }

    @Override
    public boolean onScale(float focusX, float focusY, float scale) {
        return false;
    }

    @Override
    public boolean onScaleBegin(float focusX, float focusY) {
        return false;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        if (mPipController != null && getModeState() != ModeState.STATE_FOCUSING) {
            return mPipController.onLongPress(x, y);
        }
        return false;
    }

    @Override
    public void onInfo(MediaRecorderWrapper mr, int what, int extra) {
        Log.d(TAG, "[onInfo] what = " + what + "   extra = " + extra);
        if (MediaRecorderWrapper.MEDIA_RECORDER_INFO_START_TIMER == what) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                        mRecordingStartTime = SystemClock.uptimeMillis();
                        updateRecordingTime();
                }
            });
            mIsRecordingStarted = true;
        } else if (MediaRecorderWrapper.MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED == what) {
            if (mIsMediaRecorderRecording) {
                stopVideoRecordingAsync(true);
                mICameraAppUi.showToastForShort(R.string.video_reach_size_limit);
            }
        }
    }

    private boolean canDoPauseResumeAction() {
        return mIsMediaRecorderRecording &&
                !mIsMediaRecoderRecordingPaused &&
                ModeState.STATE_SAVING != getModeState();
    }

    private int getTopCameraId() {
        return mICameraDeviceManager.getCurrentCameraId() == mICameraDeviceManager
                .getBackCameraId() ? mICameraDeviceManager.getFrontCameraId()
                : mICameraDeviceManager.getBackCameraId();

    }

    private boolean isBackBottom() {
        return mICameraDeviceManager.getCurrentCameraId() == mICameraDeviceManager
                .getBackCameraId();
    }

    private void setDisplayRotation(int displayRotation) {
        Log.i(TAG, "[setDisplayOrientation] displayRotation = " + displayRotation);
        mPipController.setDisplayRotation(displayRotation);
    }

    private void onPreviewBufferSizeChanged(int width, int height) {
        Log.i(TAG, "[onPreviewBufferSizeChanged] width = " + width + " height = " + height);
        mPipController.setPreviewTextureSize(width, height);
    }

    private void initializePipRecorder() {
        Log.d(TAG, "[initializePipRecorder]...");
        mProfile = mVideoPreviewSizeRule.getProfile();
        mRecorder = new MediaRecorderWrapper();
        // the first thing must do is audio source / video source
        if (mIsRecordAudio) {
            mRecorder.setAudioSource(MediaRecorder.AudioSource.CAMCORDER);
            mRecorder.setAudioChannels(mProfile.audioChannels);
        }
        mRecorder.setVideoSource(MediaRecorder.VideoSource.SURFACE);

        // the second should do is setoutputformat
        mRecorder.setOutputFormat(mProfile.fileFormat);

        mRecorder.setVideoFrameRate(mProfile.videoFrameRate);
        // there should be set preview size (consider mCameraDisplayOrientation)
        Size size = mICameraDevice.getParameters().getPreviewSize();
        int width = (size == null ? 0 : size.width);
        int height = (size == null ? 0 : size.height);
        mRecorder.setVideoSize(width, height);
        mRecorder.setVideoEncodingBitRate(mProfile.videoBitRate);
        mRecorder.setVideoEncoder(mProfile.videoCodec);
        if (mIsRecordAudio) {
            mRecorder.setAudioEncoder(mProfile.audioCodec);
            mRecorder.setAudioEncodingBitRate(mProfile.audioBitRate);
            mRecorder.setAudioSamplingRate(mProfile.audioSampleRate);
        }

        // Get location
        Location loc = mIModuleCtrl.getLocation();
        if (loc != null) {
            mRecorder.setLocation((long) loc.getLatitude(), (long) loc.getLongitude());
        }
        // Set maximum file size.
        long maxFileSize = Storage.getAvailableSpace() - Storage.RECORD_LOW_STORAGE_THRESHOLD;
        if (maxFileSize >= VIDEO_4G_SIZE) {
            maxFileSize = VIDEO_4G_SIZE;
        }
        mRecorder.setMaxFileSize(maxFileSize);
        generateVideoFilename(mProfile.fileFormat, null);
        mRecorder.setOutputFile(mVideoFilename);
        mRecorder.setParametersExtra();
        // because of preview buffer,orientation should be considered again
        // should always get the back camera Id as reference
        int orientation = Util.getRecordingRotation(mIModuleCtrl.getOrientation(),
                mICameraDeviceManager.getBackCameraId(),
                mICameraDeviceManager.getCameraInfo(mICameraDeviceManager.getBackCameraId()));
        mRecorder.setOrientationHint(orientation);
        try {
            mRecorder.prepare();
        } catch (IOException e) {
            Log.e(TAG, "[initializepipRecorder] prepare failed", e);
            releaseMediaRecorder();
            throw new RuntimeException(e);
        }
        mRecorder.setOnInfoListener(this);
        mPipController.prepareRecording();
        mPipController.setRecordingSurface(mRecorder.getSurface(),
                Util.getSensorOrientation(
                   mICameraDeviceManager.getCameraInfo(mICameraDeviceManager.getBackCameraId())));
        // mRecorder.setOnstartRecording(this);
        // if main thread is waited
        // mReleaseOnInfoListener will run after initializeNormalRecorder,
        // we should not allow this
        mHandler.removeCallbacks(mReleaseOnInfoListener);
        mIsRecordingStarted = false;
    }

    // for pause/resume
    private OnClickListener mPIPVideoPauseResumeListner = new OnClickListener() {
        public void onClick(View v) {
            Log.d(TAG, "[mPIPVideoPauseResumeListner.onClick()] mMediaRecoderRecordingPaused="
                    + mIsMediaRecoderRecordingPaused + ",mMediaRecorderRecording = "
                    + mIsMediaRecorderRecording + " mIsRecordingStarted = " + mIsRecordingStarted);
            // return for recorder is busy or if the recording has already
            // stopped because of some info,
            // it will not response the restart.
            if ((!mIsMediaRecorderRecording) || !mIsRecordingStarted) {
                return;
            }
            if (mIsMediaRecoderRecordingPaused) {
                mRecordingView.setRecordingIndicator(true);
                try {
                    mRecorder.resume(mRecorder);
                    mRecordingStartTime = SystemClock.uptimeMillis() - mRecordingPausedDuration;
                    mRecordingPausedDuration = 0;
                    mIsMediaRecoderRecordingPaused = false;
                } catch (IllegalStateException e) {
                    Log.e(TAG,
                            "[mPIPVideoPauseResumeListner.onClick()]" +
                            " Could not start media recorder. ",
                            e);
                    mICameraAppUi.showToast(R.string.toast_video_recording_not_available);
                    releaseMediaRecorder();
                }
            } else {
                pauseVideoRecording();
            }
            Log.d(TAG, "[mPIPVideoPauseResumeListner.onClick()] end.");
        }
    };

    private void switchDevice() {
        if (mPipController != null) {
            mPipController.switchPIP();
        }
    }

    private void setPipSettingRules(ICameraContext cameraContext) {
        Log.d(TAG, "[setPipSettingRules]...");
        PipVideoQualityRule videoQualityRule = new PipVideoQualityRule(cameraContext,
                SettingConstants.KEY_VIDEO_PIP);
        videoQualityRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_VIDEO_PIP, SettingConstants.KEY_VIDEO_QUALITY,
                videoQualityRule);
    }

    private void setAntiFlickerRules(ICameraContext cameraContext) {
        PipAntiFlickRule antiflickRule = new PipAntiFlickRule(cameraContext);
        antiflickRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_VIDEO_PIP, SettingConstants.KEY_ANTI_BANDING,
                antiflickRule);
    }

    private void setZsdRules(ICameraContext cameraContext) {
        PipZsdRule zsdRule = new PipZsdRule(cameraContext);
        zsdRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_VIDEO_PIP, SettingConstants.KEY_CAMERA_ZSD,
                zsdRule);
    }

    private class PipAntiFlickRule implements ISettingRule {
        private List<String> mConditions = new ArrayList<String>();
        private List<List<String>> mResults = new ArrayList<List<String>>();
        private List<MappingFinder> mMappingFinders = new ArrayList<MappingFinder>();
        private ICameraDevice mBackCamDevice;
        private ICameraDevice mTopCamDevice;
        private ISettingCtrl mISettingCtrl;
        private Parameters mParameters;
        private Parameters mTopParameters;
        private ICameraDeviceManager deviceManager;
        private ICameraContext mCameraContext;
        private boolean mSwitchingPip = false;
        private String mCurrentAntiFlickValue = null;

        public PipAntiFlickRule(ICameraContext cameraContext) {
            Log.d(TAG, "[PipAntiFlickRule]constructor...");
            mCameraContext = cameraContext;
        }

        @Override
        public void execute() {
            deviceManager = mCameraContext.getCameraDeviceManager();
            mBackCamDevice = deviceManager.getCameraDevice(deviceManager.getCurrentCameraId());
            if (mTopCamDevice != null) {
                mSwitchingPip = (mTopCamDevice.getCameraId() == deviceManager.getCurrentCameraId());
            } else {
                mSwitchingPip = false;
            }
            mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
            mISettingCtrl = mCameraContext.getSettingController();
            mParameters = mBackCamDevice.getParameters();
            if (mTopCamDevice != null) {
                mTopParameters = mTopCamDevice.getParameters();
            }
            String conditionValue = mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO_PIP);
            int index = conditionSatisfied(conditionValue);
            String antiFlickValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_ANTI_BANDING);
            Log.d(TAG, "[execute]PipAntiFlickRule index = "
                    + index + " antiFlickValue = " + antiFlickValue
                    + " mSwitchingPip = " + mSwitchingPip);
            if (index == -1) {
                mParameters.setAntibanding(antiFlickValue);
            } else {
                if (mSwitchingPip) {
                    antiFlickValue = mCurrentAntiFlickValue;
                    mISettingCtrl.setSettingValue(
                            SettingConstants.KEY_ANTI_BANDING, antiFlickValue, deviceManager
                            .getCurrentCameraId());
                    ListPreference pref = mISettingCtrl.getListPreference(
                            SettingConstants.KEY_ANTI_BANDING);
                    if (pref != null) {
                        pref.setValue(antiFlickValue);
                    }
                }
                mCurrentAntiFlickValue = antiFlickValue;
                mParameters.setAntibanding(antiFlickValue);
                if (mTopParameters != null) {
                    mTopParameters.setAntibanding(antiFlickValue);
                }
            }
        }

        @Override
        public void addLimitation(String condition, List<String> result,
                MappingFinder mappingFinder) {
            Log.i(TAG, "[addLimitation]condition = " + condition);
            mConditions.add(condition);
            mResults.add(result);
            mMappingFinders.add(mappingFinder);
        }

        private int conditionSatisfied(String conditionValue) {
            int index = mConditions.indexOf(conditionValue);
            return index;
        }
    }

    private class PipZsdRule implements ISettingRule {
        private List<String> mConditions = new ArrayList<String>();
        private List<List<String>> mResults = new ArrayList<List<String>>();
        private List<MappingFinder> mMappingFinders = new ArrayList<MappingFinder>();
        private ICameraDevice mBackCamDevice;
        private ICameraDevice mTopCamDevice;
        private ISettingCtrl mISettingCtrl;
        private Parameters mParameters;
        private Parameters mTopParameters;
        private ICameraDeviceManager deviceManager;
        private ICameraContext mCameraContext;
        private boolean mSwitchingPip = false;
        private String mCurrentAntiFlickValue = null;

        public PipZsdRule(ICameraContext cameraContext) {
            Log.d(TAG, "[PipZsdRule]constructor...");
            mCameraContext = cameraContext;
        }

        @Override
        public void execute() {
            deviceManager = mCameraContext.getCameraDeviceManager();
            mBackCamDevice = deviceManager.getCameraDevice(deviceManager.getCurrentCameraId());
            if (mTopCamDevice != null) {
                mSwitchingPip = (mTopCamDevice.getCameraId() == deviceManager.getCurrentCameraId());
            } else {
                mSwitchingPip = false;
            }
            mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
            mISettingCtrl = mCameraContext.getSettingController();
            mParameters = mBackCamDevice.getParameters();
            if (mTopCamDevice != null) {
                mTopParameters = mTopCamDevice.getParameters();
            }
            String conditionValue = mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO_PIP);
            int index = conditionSatisfied(conditionValue);
            String zsdValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_CAMERA_ZSD);
            if (zsdValue == null) {
                Log.d(TAG, "[PipZsdRule.execute] don't support zsd!");
                return;
            }
            Log.d(TAG, "[execute]PipZsdRule zsdValue = " + zsdValue + ", index = " + index);
            if (index == -1) {
                mParameters.setZSDMode(zsdValue);
            } else {
                mParameters.setZSDMode(OVERRIDE_ZSD_VALUE);
                if (mTopParameters != null) {
                    mTopParameters.setZSDMode(OVERRIDE_ZSD_VALUE);
                }
            }
        }

        @Override
        public void addLimitation(String condition, List<String> result,
                MappingFinder mappingFinder) {
            Log.d(TAG, "[addLimitation]condition = " + condition);
            mConditions.add(condition);
            mResults.add(result);
            mMappingFinders.add(mappingFinder);
        }

        private int conditionSatisfied(String conditionValue) {
            int index = mConditions.indexOf(conditionValue);
            return index;
        }
    }
}
