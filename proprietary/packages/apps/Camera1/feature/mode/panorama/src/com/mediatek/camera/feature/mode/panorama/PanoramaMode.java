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

import android.content.ContentValues;
import android.graphics.Bitmap;
import android.media.MediaActionSound;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.storage.MediaSaver.MediaSaverListener;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * Panorama photo mode.
 */

public class PanoramaMode extends PanoramaBase implements IAppUiListener.OnGestureListener {
    private static final LogUtil.Tag TAG = new LogUtil.
            Tag(PanoramaMode.class.getSimpleName());
    private static final String MODE_KEY = PanoramaMode.class.getName();
    private static final int NUM_AUTORAMA_CAPTURE = 9;
    private static final int MSG_FINAL_IMAGE_READY = 1000;
    private static final int MSG_SAVE_FILE = 1004;
    private static final int MSG_UPDATE_MOVING = 1005;
    private static final int MSG_UPDATE_PROGRESS = 1006;
    private static final int MSG_START_ANIMATION = 1007;
    private static final int MSG_HIDE_VIEW = 1008;
    private static final int MSG_IN_CAPTURING = 1009;
    private static final int MSG_OUT_OF_CAPTURING = 1010;
    private static final int TIME_RESTART_CAPTURE_VIEW_MS = 500;

    private MediaActionSound mCameraSound;
    private Thread mLoadSoundTread;
    private Handler mMainHandler;
    private Runnable mOnHardwareStop;
    private Runnable mRestartCaptureView;
    private Object mLock = new Object();
    private State mState = State.STATE_UNKNOWN;

    private PanoramaView mPanoramaView;

    private boolean mIsShowingCollimatedDrawable;
    private boolean mIsInStopProcess = false;
    private boolean mIsMerging = false;
    private boolean mIsCameraPreviewStarted = false;
    private int mCurrentNum = 0;
    private byte[] mJpegImageData = null;

    private PanoramaView.OnSaveButtonClickedListener mSaveButtonClickedListener =
            new PanoramaView.OnSaveButtonClickedListener() {
                @Override
                public void onSaveButtonClicked() {
                    if (mCurrentNum > 0) {
                        mPanoramaView.update(INFO_OUT_OF_CAPTURING);
                        stopCapture(true);
                    }
                }
            };

    private PanoramaView.OnCancelButtonClickedListener mCancelButtonClickedListener =
            new PanoramaView.OnCancelButtonClickedListener() {
                @Override
                public void onCancelButtonClicked() {
                    mPanoramaView.update(INFO_OUT_OF_CAPTURING);
                    stopCapture(false);
                }
            };

    /**
     * This is usd for tag panorama state.
     */
    enum State {
        STATE_UNKNOWN,
        STATE_IDLE,
        STATE_CAPTURING,
        STATE_CLOSED,
    }

    public static final int INFO_UPDATE_PROGRESS = 0;
    public static final int INFO_UPDATE_MOVING = 1;
    public static final int INFO_START_ANIMATION = 2;
    public static final int INFO_IN_CAPTURING = 3;
    public static final int INFO_OUT_OF_CAPTURING = 4;

    public static final int GUIDE_SHUTTER = 0;
    public static final int GUIDE_MOVE = 1;

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
            boolean isFromLaunch) {
        LogHelper.i(TAG, "[init]+");
        super.init(app, cameraContext, isFromLaunch);
        mCameraSound = new MediaActionSound();
        mLoadSoundTread = new LoadSoundTread();
        mLoadSoundTread.start();
        mMainHandler = new PanoramaHandler(mIApp.getActivity().getMainLooper());
        mPanoramaView = new PanoramaView(mIApp, Integer
                .parseInt(mCameraId));
        mPanoramaView.init();
        mPanoramaView.setSaveButtonClickedListener(mSaveButtonClickedListener);
        mPanoramaView.setCancelButtonClickedListener(mCancelButtonClickedListener);
        LogHelper.i(TAG, "[init]-");
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        LogHelper.i(TAG, "[pause]+ mMainHandler = " + mMainHandler
                + ", mIsMerging = " + mIsMerging);
        // if camera released, native will stop panorama proactively.
        // to stop panorama by AP's commend, stopCapture should be called before camera release.
        stopCapture(false);
        super.pause(nextModeDeviceUsage);
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
        LogHelper.i(TAG, "[pause]-");
    }

    @Override
    public void unInit() {
        LogHelper.i(TAG, "[unInit]+");
        super.unInit();
        if (mCameraSound != null) {
            mCameraSound.release();
            mCameraSound = null;
        }
        mPanoramaView.unInit();
        LogHelper.i(TAG, "[unInit]-");
    }

    @Override
    public boolean onShutterButtonClick() {
        return capture();
    }

    @Override
    public void onCameraOpened() {
        super.onCameraOpened();
        LogHelper.d(TAG, "[onCameraOpened]");
        mState = State.STATE_IDLE;
        Relation relation = PanoramaRestriction.getRestriction().getRelation("on", true);
        String pictureSizes = mISettingManager.getSettingController().
                queryValue("key_picture_size");
        relation.addBody("key_picture_size", pictureSizes, pictureSizes);
        mISettingManager.getSettingController().postRestriction(relation);
        mISettingManager.getSettingController().addViewEntry();
        mISettingManager.getSettingController().refreshViewEntry();
    }

    @Override
    public void beforeCloseCamera() {
        super.beforeCloseCamera();
        mIsCameraPreviewStarted = false;
    }

    @Override
    public void onCameraPreviewStarted() {
        super.onCameraPreviewStarted();
        mIsCameraPreviewStarted = true;
    }

    @Override
    public void onCameraPreviewStopped() {
        super.onCameraPreviewStopped();
        LogHelper.d(TAG, "[onCameraPreviewStopped]");
        mIsCameraPreviewStarted = false;
    }

    @Override
    public String getModeKey() {
        return MODE_KEY;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        return true;
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

    /**
     * Load sound thread.
     */
    private class LoadSoundTread extends Thread {
        @Override
        public void run() {
            mCameraSound.load(MediaActionSound.SHUTTER_CLICK);
        }
    }

    /**
     * The panorama handler.
     */
    private class PanoramaHandler extends Handler {
        public PanoramaHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {

            if (msg.what != MSG_UPDATE_MOVING) {
                LogHelper.d(TAG, "[handleMessage]msg.what = " + msg.what);
            }

            switch (msg.what) {
                case MSG_FINAL_IMAGE_READY:
                    dismissSavingProcess();
                    resetCapture();
                    break;

                case MSG_SAVE_FILE:
                    saveFile();
                    break;

                case MSG_UPDATE_MOVING:
                    boolean shown = mIsShowingCollimatedDrawable
                            || State.STATE_CAPTURING != mState || mCurrentNum < 1;
                    mPanoramaView.update(INFO_UPDATE_MOVING, msg.arg1, msg.arg2, shown);
                    break;

                case MSG_UPDATE_PROGRESS:
                    mPanoramaView.update(INFO_UPDATE_PROGRESS, msg.arg1);
                    break;

                case MSG_START_ANIMATION:
                    mPanoramaView.update(INFO_START_ANIMATION, msg.arg1);
                    break;

                case MSG_HIDE_VIEW:
                    mPanoramaView.reset();
                    mPanoramaView.hide();
                    break;

                case MSG_IN_CAPTURING:
                    mPanoramaView.update(INFO_IN_CAPTURING, msg.arg1);
                    break;

                case MSG_OUT_OF_CAPTURING:
                    mPanoramaView.update(INFO_OUT_OF_CAPTURING, msg.arg1);
                    break;

                default:
                    break;
            }
        }
    }

    private void onHardwareStopped(boolean isMerge) {
        LogHelper.d(TAG, "[onHardwareStopped]isMerge = " + isMerge);
        if (isMerge) {
            mIPanoramaDeviceController.setAutoRamaCallback(null);
        }
        onCaptureDone(isMerge);
    }

    private void onCaptureDone(boolean isMerge) {
        LogHelper.d(TAG, "[onCaptureDone] isMerge = " + isMerge);
        if (isMerge && mJpegImageData != null) {
            mMainHandler.sendEmptyMessage(MSG_SAVE_FILE);
        } else {
            resetCapture();
        }
        mState = State.STATE_IDLE;
        updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
    }

    private void resetCapture() {
        LogHelper.d(TAG, "[resetCapture] current mState = " + mState);
        if (State.STATE_CLOSED != mState) {
            unlock3A();
            mPanoramaView.showGuideView(GUIDE_SHUTTER);
        }
        mIApp.getAppUi().unregisterGestureListener(this);
    }

    private boolean capture() {
        boolean storageReady = mICameraContext.getStorageService().getCaptureStorageSpace() > 0;
        LogHelper.i(TAG, "[capture] current mState = " + mState + ", mIsMerging = " + mIsMerging +
                ", storageReady = " + storageReady + ", mIsCameraPreviewStarted = "
                + mIsCameraPreviewStarted);
        if (!storageReady || State.STATE_IDLE != mState || mIsMerging
                || !mIsCameraPreviewStarted) {
            LogHelper.w(TAG, "[capture] return");
            return false;
        }

        if (!startCapture()) {
            LogHelper.w(TAG, "[capture] do not capture.");
            return false;
        }
        mPanoramaView.showGuideView(GUIDE_MOVE);
        mMainHandler.sendEmptyMessage(MSG_IN_CAPTURING);
        return true;
    }

    private boolean startCapture() {
        LogHelper.d(TAG, "[startCapture] mState = " + mState + ",mIsInStopProcess = "
                + mIsInStopProcess);
        if (State.STATE_IDLE == mState && !mIsInStopProcess) {
            mState = State.STATE_CAPTURING;
            updateModeDeviceState(MODE_DEVICE_STATE_CAPTURING);
            mCurrentNum = 0;
            mIsShowingCollimatedDrawable = false;
            mIPanoramaDeviceController.updateGSensorOrientation(mIApp.getGSensorOrientation());
            mIPanoramaDeviceController.setAutoRamaCallback(new VendorDataCallback());
            mIPanoramaDeviceController.startAutoRama(NUM_AUTORAMA_CAPTURE);
            lock3A();
            mIApp.getAppUi().registerGestureListener(this, 0);
            mPanoramaView.show();
            return true;
        }

        return false;
    }

    /**
     * Panorama data callback.
     */
    private class VendorDataCallback implements CameraProxy.VendorDataCallback {

        /**
         * Called when vendor-defined data is available.
         *
         * @param message message object from native.
         */
        @Override
        public void onDataTaken(Message message) {
            LogHelper.d(TAG, "[onDataTaken] message = " + message.what);
        }

        /**
         * Called when vendor-defined data is available.
         *
         * @param msgId The id of message which contains the callback info.
         * @param data  Callback data.
         * @param arg1  The first argument.
         * @param arg2  The second argument.
         */
        @Override
        public void onDataCallback(int msgId, byte[] data, int arg1, int arg2) {
            if (arg2 == -1) {
                onPictureTaken(data);
            } else {
                mMainHandler.obtainMessage(MSG_UPDATE_MOVING, arg1, arg2).sendToTarget();
            }
        }
    }

    private void onPictureTaken(byte[] jpegData) {
        LogHelper.d(TAG, "[onPictureTaken] mState = " + mState + ", mCurrentNum = " + mCurrentNum);
        if (State.STATE_IDLE == mState) {
            LogHelper.w(TAG, "[onPictureTaken]modeState is STATE_IDLE,return.");
            return;
        }

        if (mCurrentNum == NUM_AUTORAMA_CAPTURE || mIsMerging) {
            LogHelper.d(TAG, "[onPictureTaken] autoRama done, mCurrentNum = " + mCurrentNum);
            mJpegImageData = jpegData;
            mIsMerging = false;
            onHardwareStopped(true);

        } else if (mCurrentNum >= 0 && mCurrentNum < NUM_AUTORAMA_CAPTURE) {
            if (mCameraSound != null) {
                mCameraSound.play(MediaActionSound.SHUTTER_CLICK);
            }
            mMainHandler.obtainMessage(MSG_UPDATE_PROGRESS, mCurrentNum, 0).sendToTarget();
            if (0 < mCurrentNum) {
                if (mIsShowingCollimatedDrawable) {
                    mMainHandler.removeCallbacks(mRestartCaptureView);
                    mMainHandler.removeCallbacks(mOnHardwareStop);
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
            stop(true);
        }
    }

    private void stop(boolean isMerge) {
        LogHelper.d(TAG, "[stop]isMerge = " + isMerge + ", mState=" + mState + ",mIsMerging = "
                + mIsMerging);

        if (State.STATE_CAPTURING != mState) {
            LogHelper.i(TAG, "[stop] current mode state is not capturing, so return");
            return;
        }

        if (mIsMerging) {
            // if current is in the progress merging,means before have stopped
            // the panorama, so can directly return.
            LogHelper.i(TAG, "[stop] current is also in merging,so cancel this time");
            return;
        } else {
            mIsMerging = isMerge;
            if (isMerge) {
                showSavingProgress();
            }
            mMainHandler.removeMessages(MSG_UPDATE_MOVING);
            mMainHandler.removeMessages(MSG_HIDE_VIEW);
            mMainHandler.sendEmptyMessage(MSG_HIDE_VIEW);
            stopAsync(isMerge);
        }
    }

    private void stopAsync(final boolean isMerge) {
        LogHelper.d(TAG, "[stopAsync]isMerge=" + isMerge +
                ", mIsInStopProcess = " + mIsInStopProcess);

        if (mIsInStopProcess) {
            return;
        }

        Thread stopThread = new Thread(new Runnable() {
            public void run() {
                doStop(isMerge);
                mOnHardwareStop = new Runnable() {
                    public void run() {
                        if (!isMerge) {
                            // if isMerge is true, onHardwareStopped
                            // will be called in onCapture.
                            onHardwareStopped(false);
                        }
                    }
                };
                mMainHandler.post(mOnHardwareStop);

                synchronized (mLock) {
                    mIsInStopProcess = false;
                    mLock.notifyAll();
                }
            }
        });
        synchronized (mLock) {
            mIsInStopProcess = true;
        }
        stopThread.start();
    }

    private void doStop(boolean isMerge) {
        LogHelper.d(TAG, "[doStop] isMerge=" + isMerge);
        mIPanoramaDeviceController.stopAutoRama(isMerge);
    }

    private void stopCapture(boolean isMerge) {
        LogHelper.d(TAG, "[stopCapture]isMerge = " + isMerge + ", current mState = " + mState);
        if (State.STATE_CAPTURING == mState) {
            mMainHandler.sendEmptyMessage(MSG_OUT_OF_CAPTURING);
            stop(isMerge);
        }
    }

    private void showSavingProgress() {
        mIApp.getAppUi().showSavingDialog(null, true);
    }

    private void dismissSavingProcess() {
        LogHelper.d(TAG, "[dismissSavingProcess]");
        mIApp.getAppUi().hideSavingDialog();
    }

    private void saveFile() {
        LogHelper.d(TAG, "[saveFile] mJpegImageData = " + mJpegImageData);
        if (mJpegImageData != null) {
            Bitmap bitmap = BitmapCreator.createBitmapFromJpeg(mJpegImageData, mIApp.getAppUi()
                    .getThumbnailViewWidth());
            mIApp.getAppUi().updateThumbnail(bitmap);
            Size jpegSize = CameraUtil.getSizeFromExif(mJpegImageData);
            String fileDirectory = mICameraContext.getStorageService().getFileDirectory();
            ContentValues contentValues = mPhotoModeHelper.createContentValues(mJpegImageData,
                    fileDirectory, jpegSize.getWidth(), jpegSize.getHeight());
            mICameraContext.getMediaSaver().addSaveRequest(mJpegImageData, contentValues, null,
                    mMediaSaverListener);
        }
    }

    private MediaSaverListener mMediaSaverListener = new MediaSaverListener() {

        @Override
        public void onFileSaved(Uri uri) {
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri);
            mIApp.notifyNewMedia(uri, true);
            mMainHandler.sendEmptyMessage(MSG_FINAL_IMAGE_READY);
        }
    };

    private void lock3A() {
        if (!mIsCameraPreviewStarted) {
            LogHelper.d(TAG, "[lock3A] Preview not started, do nothing ");
            return;
        }
        mISettingManager.getSettingController().postRestriction(
                PanoramaRestriction.get3aRestriction().getRelation("on", true));
        mIPanoramaDeviceController.configParameters();
    }

    private void unlock3A() {
        if (!mIsCameraPreviewStarted) {
            LogHelper.d(TAG, "[unlock3A] Preview not started, do nothing");
            return;
        }
        mISettingManager.getSettingController().postRestriction(
                PanoramaRestriction.get3aRestriction().getRelation("off", true));
        mIPanoramaDeviceController.configParameters();
    }
}
