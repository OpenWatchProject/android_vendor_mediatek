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

package com.mediatek.camera.mode.panorama;

import android.hardware.Camera;
import android.hardware.Camera.AFDataCallback;
import android.location.Location;
import android.media.MediaActionSound;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.WindowManager;

import com.android.camera.R;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.mode.CameraMode;
import com.mediatek.camera.platform.ICameraAppUi.CommonUiType;
import com.mediatek.camera.platform.ICameraAppUi.ShutterButtonType;
import com.mediatek.camera.platform.ICameraAppUi.SpecViewType;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.AutoFocusMvCallback;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.PanoramaListener;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.PanoramaMvListener;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.platform.IFileSaver.FILE_TYPE;
import com.mediatek.camera.platform.IFileSaver.OnFileSavedListener;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.util.Log;

import junit.framework.Assert;

public class PanoramaMode extends CameraMode {
    private static final String TAG = "PanoramaMode";

    public static final int INFO_UPDATE_PROGRESS = 0;
    public static final int INFO_UPDATE_MOVING = 1;
    public static final int INFO_START_ANIMATION = 2;
    public static final int INFO_IN_CAPTURING = 3;
    public static final int INFO_OUTOF_CAPTURING = 4;

    private static final int SHOW_INFO_LENGTH_LONG = 5 * 1000;
    private static final int NUM_AUTORAMA_CAPTURE = 9;
    private static final int MSG_FINAL_IMAGE_READY = 1000;
    private static final int MSG_ORIENTATION_CHANGED = 1001;
    private static final int MSG_CLEAR_SCREEN_DELAY = 1002;
    private static final int MSG_LOCK_ORIENTATION = 1003;
    private static final int MSG_SAVE_FILE = 1004;
    private static final int MSG_UPDATE_MOVINE = 1005;
    private static final int MSG_UPDATE_PROGRESS = 1006;
    private static final int MSG_START_ANIMATION = 1007;
    private static final int MSG_HIDE_VIEW = 1008;
    private static final int MSG_IN_CAPTURING = 1009;
    private static final int MSG_OUTOF_CAPTURING = 1010;
    private static final int MSG_INIT = 1011;
    private static final int MSG_UNINIT = 1012;
    private static final int MSG_SHOW_INFO = 1013;
    private static final int GUIDE_SHUTTER = 0;
    private static final int GUIDE_MOVE = 1;
    private static final int GUIDE_CAPTURE = 2;

    private int mCurrentNum = 0;
    private int mOrientation = 0;
    private long mCaptureTime = 0;

    private byte[] mJpegImageData = null;

    private boolean mIsShowingCollimatedDrawable;
    private boolean mIsInStopProcess = false;
    private boolean mIsMerging = false;

    private ICameraView mICameraView;

    private Runnable mOnHardwareStop;
    private Runnable mRestartCaptureView;
    private Handler mMainHandler;
    private Object mLock = new Object();
    private MediaActionSound mCameraSound;
    private Thread mLoadSoundTread;

    public PanoramaMode(ICameraContext cameraContext) {
        super(cameraContext);
        Log.i(TAG, "[PanoramaMode]constructor...");

        mICameraView = mICameraAppUi.getCameraView(SpecViewType.MODE_PANORAMA);
        mICameraView.init(mActivity, mICameraAppUi, mIModuleCtrl);

        mCameraSound = new MediaActionSound();
        mLoadSoundTread = new LoadSoundTread();
        mLoadSoundTread.start();
        mMainHandler = new PanoramaHandler(mActivity.getMainLooper());
    }

    @Override
    public void pause() {
        super.pause();
        Log.i(TAG, "[pasue]mMainHandler = " + mMainHandler + ", mIsMerging = " + mIsMerging);
        if (mMainHandler != null) {
            mMainHandler.removeMessages(MSG_HIDE_VIEW);
            mMainHandler.sendEmptyMessage(MSG_HIDE_VIEW);
            if (mIsMerging) {
                mIsMerging = false;
                mMainHandler.removeMessages(MSG_FINAL_IMAGE_READY);
                mMainHandler.sendEmptyMessage(MSG_FINAL_IMAGE_READY);
            }
        }
    }

    @Override
    public boolean close() {
        Log.i(TAG, "[close] ");
        safeStop();
        if (ModeState.STATE_CAPTURING == getModeState()) {
            mICameraView.reset();
            mICameraView.hide();
            mICameraDevice.stopAutoRama(false);
            mICameraDevice.setAutoRamaCallback(null);
            mICameraDevice.setAutoRamaMoveCallback(null);
        }
        if (mCameraSound != null) {
            mCameraSound.release();
            mCameraSound = null;
        }
        mICameraAppUi.setSwipeEnabled(true);
        mICameraView.uninit();

        return true;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        boolean returnValue = true;
        switch (type) {
        case ACTION_ON_CAMERA_OPEN:
            super.updateDevice();
            break;

        case ACTION_ON_CAMERA_CLOSE:
            stopCapture(false);
            setModeState(ModeState.STATE_CLOSED);
            break;

        case ACTION_ON_CAMERA_PARAMETERS_READY:
            super.updateDevice();
            super.updateFocusManager();
            setModeState(ModeState.STATE_IDLE);
            mICameraDevice.setAutoFocusMoveCallback(mAutoFocusMoveCallback);
            String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_AF);
            if (mActivity.getResources().getString(R.string.af_multi_mode).equals(value)) {
                mICameraDevice.getCamera().setAFDataCallback(mAFDataCallback);
            } else {
                mICameraDevice.getCamera().setAFDataCallback(null);
            }
            break;

        case ACTION_PHOTO_SHUTTER_BUTTON_CLICK:
            capture();
            break;

        case ACTION_OK_BUTTON_CLICK:
            stopCapture(true);
            break;

        case ACTION_CANCEL_BUTTON_CLICK:
            if (!mIsInStopProcess) {
                mICameraAppUi.setOkButtonEnabled(false);
                stopCapture(false);
            }
            break;

        case ACTION_ON_COMPENSATION_CHANGED:
            Assert.assertTrue(arg.length == 1);
            mOrientation = (Integer) arg[0];
            mMainHandler.sendEmptyMessage(MSG_ORIENTATION_CHANGED);
            break;

        case ACTION_ON_BACK_KEY_PRESS:
            return onBackPressed();

        case ACTION_SHUTTER_BUTTON_LONG_PRESS:
            mMainHandler.sendEmptyMessage(MSG_SHOW_INFO);
            break;

        case ACTION_ON_FULL_SCREEN_CHANGED:
            Assert.assertTrue(arg.length == 1);
            Log.i(TAG, "[execute]type = " + type + ",full:" + (Boolean) arg[0]);
            if ((Boolean) arg[0]) { // true means :from Gallery go to Camera
                mMainHandler.sendEmptyMessage(MSG_INIT);
            } else { // false means: Camera go to Gallery
                mMainHandler.sendEmptyMessage(MSG_UNINIT);
            }
            break;

        case ACTION_ON_SINGLE_TAP_UP:

            Log.i(TAG, "current state : " + getModeState());

            if (ModeState.STATE_IDLE == getModeState()) {
                //return false means this MSG need supper execute
                returnValue = false;
            }
            break;

        case ACTION_CAN_DO_AUTO_FOCUS:
            returnValue = ModeState.STATE_CAPTURING != getModeState();
            break;

        default:
            returnValue = false;
        }
        Log.i(TAG, "[execute]type = " + type + ",returnValue = " + returnValue);

        return returnValue;
    }

    private boolean capture() {
        Log.i(TAG, "[capture] current state = " + getModeState() + ",mIsMerging = " + mIsMerging);
        if (!isEnoughSpace() || ModeState.STATE_IDLE != getModeState() || mIsMerging) {
            Log.w(TAG, "[capture]return,mIsCameraClosed = " + getModeState());
            return false;
        }

        if (!startCapture()) {
            Log.w(TAG, "[capture]not capture.");
            return false;
        }

        // make sure focus UI be cleared before capture.
        mIFocusManager.resetTouchFocus();
        mIFocusManager.updateFocusUI();
        mIFocusManager.setAwbLock(true);
        mIModuleCtrl.applyFocusParameters(false);
        mICameraAppUi.switchShutterType(ShutterButtonType.SHUTTER_TYPE_OK_CANCEL);
        mIFileSaver.init(FILE_TYPE.PANORAMA, 0, null, -1);
        mCaptureTime = System.currentTimeMillis();

        mICameraAppUi.setSwipeEnabled(false);
        mICameraAppUi.showRemaining();
        mICameraAppUi.setViewState(ViewState.VIEW_STATE_CONTINUOUS_CAPTURE);
        ICameraView thumbnailView = mICameraAppUi.getCameraView(CommonUiType.THUMBNAIL);
        if (thumbnailView != null) {
            thumbnailView.hide();
        }
        mMainHandler.sendEmptyMessage(MSG_IN_CAPTURING);
        mIModuleCtrl.stopFaceDetection();
        mICameraDevice.setAutoFocusMoveCallback(null);
        showGuideString(GUIDE_MOVE);
        mMainHandler.postDelayed(mFalseShutterCallback, 300);

        return true;
    }

    private void stopCapture(boolean isMerge) {
        Log.d(TAG, "[stopCapture]isMerge = " + isMerge + ",current mode state = " + getModeState());
        if (ModeState.STATE_CAPTURING == getModeState()) {
            mMainHandler.sendEmptyMessage(MSG_OUTOF_CAPTURING);
            stop(isMerge);
        }
    }

    private boolean onBackPressed() {
        if (ModeState.STATE_CAPTURING == getModeState()) {
            stopCapture(false);
            return true;
        } else {
            return false;
        }
    }

    private boolean startCapture() {
        Log.d(TAG, "[startCapture]modeState = " + getModeState() + ",mIsInStopProcess = "
                + mIsInStopProcess);
        if (ModeState.STATE_IDLE == getModeState() && !mIsInStopProcess) {
            setModeState(ModeState.STATE_CAPTURING);
            mCurrentNum = 0;
            mIsShowingCollimatedDrawable = false;
            mICameraDevice.setAutoRamaCallback(mPanoramaCallback);
            mICameraDevice.setAutoRamaMoveCallback(mPanoramaMVCallback);
            mICameraDevice.startAutoRama(NUM_AUTORAMA_CAPTURE);
            mICameraAppUi.setOkButtonEnabled(false);
            mICameraView.show();
            return true;
        }

        return false;
    }

    private class LoadSoundTread extends Thread {
        @Override
        public void run() {
            mCameraSound.load(MediaActionSound.SHUTTER_CLICK);
        }
    }

    private class PanoramaHandler extends Handler {
        public PanoramaHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg.what != MSG_UPDATE_MOVINE) {
                Log.i(TAG, "[handleMessage]msg.what = " + msg.what);
            }

            switch (msg.what) {
            case MSG_FINAL_IMAGE_READY:
                mICameraAppUi.dismissProgress();
                mICameraAppUi.setSwipeEnabled(true);
                resetCapture();
                break;

            case MSG_CLEAR_SCREEN_DELAY:
                mActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                break;

            case MSG_LOCK_ORIENTATION:
                mIModuleCtrl.lockOrientation();
                break;

            case MSG_SAVE_FILE:
                saveFile();
                break;

            case MSG_UPDATE_MOVINE:
                boolean shown = mIsShowingCollimatedDrawable
                        || ModeState.STATE_CAPTURING != getModeState() || mCurrentNum < 1;
                mICameraView.update(INFO_UPDATE_MOVING, msg.arg1, msg.arg2, shown);
                break;

            case MSG_UPDATE_PROGRESS:
                mICameraView.update(INFO_UPDATE_PROGRESS, msg.arg1);
                break;

            case MSG_START_ANIMATION:
                mICameraView.update(INFO_START_ANIMATION, msg.arg1);
                break;

            case MSG_HIDE_VIEW:
                mICameraView.reset();
                mICameraView.hide();
                break;

            case MSG_IN_CAPTURING:
                mICameraView.update(INFO_IN_CAPTURING, msg.arg1);
                break;

            case MSG_OUTOF_CAPTURING:
                mICameraView.update(INFO_OUTOF_CAPTURING, msg.arg1);
                break;

            case MSG_INIT:
                mICameraView.init(mActivity, mICameraAppUi, mIModuleCtrl);
                break;

            case MSG_UNINIT:
                mICameraView.uninit();
                break;

            case MSG_SHOW_INFO:
                String showInfoStr = mActivity.getString(R.string.pano_dialog_title)
                        + mActivity.getString(R.string.camera_continuous_not_supported);
                mICameraAppUi.showInfo(showInfoStr);
                break;

            case MSG_ORIENTATION_CHANGED:
                mICameraView.onOrientationChanged(mOrientation);
                break;

            default:
                break;
            }
        }
    }

    private void resetCapture() {
        Log.d(TAG, "[resetCapture]...current mode state = " + getModeState());
        if (ModeState.STATE_CLOSED != getModeState()) {
            mIFocusManager.setAeLock(false);
            mIFocusManager.setAwbLock(false);
            mIModuleCtrl.applyFocusParameters(false);
            mIModuleCtrl.startFaceDetection();
            mICameraDevice.setAutoFocusMoveCallback(mAutoFocusMoveCallback);
            showGuideString(GUIDE_SHUTTER);
        }
        mICameraAppUi.switchShutterType(ShutterButtonType.SHUTTER_TYPE_PHOTO_VIDEO);
        mICameraAppUi.restoreViewState();
        mICameraAppUi.setSwipeEnabled(true);
    }

    private final AutoFocusMvCallback mAutoFocusMoveCallback = new AutoFocusMvCallback() {
        @Override
        public void onAutoFocusMoving(boolean moving, Camera camera) {
            Log.i(TAG, "[onAutoFocusMoving]moving = " + moving);
            mIFocusManager.onAutoFocusMoving(moving);
        }
    };

    private final AFDataCallback mAFDataCallback = new AFDataCallback() {
        @Override
        public void onAFData(byte[] data, Camera camera) {
            String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_AF);
            boolean isMultiAfMode = mActivity.getResources().getString(R.string.af_multi_mode)
                    .equals(value) ? true : false;
            if (data != null && data.length > 0 && isMultiAfMode) {
                mIFocusManager.setAfData(data);
            } else {
                Log.w(TAG, "onAFData AF data is got in single AF mode with isMultiAfMode = "
                        + isMultiAfMode);
                mIFocusManager.setAfData(null);
            }
        }
    };

    private void saveFile() {
        Log.d(TAG, "[saveFile]...");
        Location location = mIModuleCtrl.getLocation();
        mIFileSaver.savePhotoFile(mJpegImageData, null, mCaptureTime, location, 0,
                mFileSaverListener);
        mMainHandler.sendEmptyMessage(MSG_OUTOF_CAPTURING);
    }

    private OnFileSavedListener mFileSaverListener = new OnFileSavedListener() {
        @Override
        public void onFileSaved(Uri uri) {
            Log.i(TAG, "[onFileSaved]uri = " + uri);
            // if (ModeState.STATE_CLOSED == getModeState()) {
            // Log.w(TAG, "[onFileSaved]current state is STATE_CLOSED,return.");
            // return;
            // }

            mMainHandler.sendEmptyMessage(MSG_FINAL_IMAGE_READY);
        }
    };

    private Runnable mFalseShutterCallback = new Runnable() {
        @Override
        public void run() {
            mIFocusManager.resetTouchFocus();
            mIFocusManager.updateFocusUI();
        }
    };

    private PanoramaMvListener mPanoramaMVCallback = new PanoramaMvListener() {
        @Override
        public void onFrame(int xy, int direction) {
            mMainHandler.obtainMessage(MSG_UPDATE_MOVINE, xy, direction).sendToTarget();
        }
    };

    private PanoramaListener mPanoramaCallback = new PanoramaListener() {
        @Override
        public void onCapture(byte[] jpegData) {
            onPictureTaken(jpegData);
        }
    };

    private void onPictureTaken(byte[] jpegData) {
        Log.d(TAG, "[onPictureTaken]modeState = " + getModeState() + ",mCurrentNum = "
                + mCurrentNum);
        if (ModeState.STATE_IDLE == getModeState()) {
            Log.w(TAG, "[onPictureTaken]modeState is STATE_IDLE,return.");
            return;
        }

        if (mCurrentNum == NUM_AUTORAMA_CAPTURE || mIsMerging) {
            Log.d(TAG, "[onPictureTaken]autorama done,mCurrentNum = " + mCurrentNum);
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
                mMainHandler.postDelayed(mRestartCaptureView, 500);
            }
        }

        mCurrentNum++;
        if (mCurrentNum == 2) {
            mICameraAppUi.setOkButtonEnabled(true);
        }
        if (mCurrentNum == NUM_AUTORAMA_CAPTURE) {
            stop(true);
        }
    }

    private void stop(boolean isMerge) {
        Log.d(TAG, "[stop]isMerge = " + isMerge + ",modeState=" + getModeState() + ",mIsMerging = "
                + mIsMerging);

        if (ModeState.STATE_CAPTURING != getModeState()) {
            Log.i(TAG, "[stop] current mode state is not capturing,so return");
            return;
        }

        if (mIsMerging) {
            // if current is in the progress merging,means before have stopped
            // the panorama, so can directly return.
            Log.i(TAG, "[stop] current is also in merging,so cancle this time");
            return;
        } else {
            mIsMerging = isMerge;
            if (!isMerge) {
                mICameraDevice.setAutoRamaCallback(null);
            } else {
                mICameraAppUi.showProgress(mActivity.getString(R.string.saving));
                mICameraAppUi.dismissInfo();
            }
            mICameraDevice.setAutoRamaMoveCallback(null);
            mMainHandler.removeMessages(MSG_UPDATE_MOVINE);
            mMainHandler.removeMessages(MSG_HIDE_VIEW);
            mMainHandler.sendEmptyMessage(MSG_HIDE_VIEW);
            stopAsync(isMerge);
            mICameraAppUi.setSwipeEnabled(true);
            mIModuleCtrl.unlockOrientation();
        }
    }

    private void stopAsync(final boolean isMerge) {
        Log.i(TAG, "[stopAsync]isMerge=" + isMerge + ",mIsInStopProcess = " + mIsInStopProcess);

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
        Log.d(TAG, "[doStop]isMerge=" + isMerge);
        mICameraDevice.stopAutoRama(isMerge);
    }

    private void onHardwareStopped(boolean isMerge) {
        Log.d(TAG, "[onHardwareStopped]isMerge = " + isMerge);
        if (isMerge) {
            mICameraDevice.setAutoRamaCallback(null);
        }

        onCaptureDone(isMerge);
    }

    private void onCaptureDone(boolean isMerge) {
        Log.d(TAG, "[onCaptureDone]isMerge = " + isMerge);
        if (isMerge && mJpegImageData != null) {
            mMainHandler.sendEmptyMessage(MSG_SAVE_FILE);
        } else {
            resetCapture();
        }
        setModeState(ModeState.STATE_IDLE);
    }

    // do the stop sequence carefully in order not to cause driver crash.
    private void safeStop() {
        Log.i(TAG, "[safeStop] check stopAsync thread state, if running,we must wait");
        while (mIsInStopProcess) {
            try {
                synchronized (mLock) {
                    mLock.wait();
                }
            } catch (InterruptedException e) {
                Log.w(TAG, "InterruptedException in waitLock");
            }
        }
    }

    private void showGuideString(int step) {
        int guideId = 0;
        switch (step) {
        case GUIDE_SHUTTER:
            guideId = R.string.panorama_guide_shutter;
            break;

        case GUIDE_MOVE:
            guideId = R.string.panorama_guide_choose_direction;
            break;

        case GUIDE_CAPTURE:
            guideId = R.string.panorama3d_guide_capture;
            break;

        default:
            break;
        }

        // show current guide
        if (guideId != 0) {
            mICameraAppUi.showInfo(mActivity.getString(guideId), SHOW_INFO_LENGTH_LONG);
        }
    }
}
