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
package com.android.camera.actor;

import com.android.camera.CameraActivity;
import com.android.camera.CameraActivity.OnLongPressListener;
import com.android.camera.CameraActivity.OnSingleTapUpListener;
import com.android.camera.CameraErrorCallback;
import com.android.camera.CameraHolder;
import com.android.camera.manager.ModePicker;
import com.android.camera.manager.RecordingView;
import com.android.camera.ui.ShutterButton;
import com.android.camera.ui.ShutterButton.OnShutterButtonListener;

import android.app.Activity;
import android.content.Intent;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.ErrorCallback;
import android.view.KeyEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.View.OnClickListener;

import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.ModuleManager;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraAppUi.CommonUiType;
import com.mediatek.camera.util.Log;

//public class VideoActor extends CameraActor implements FocusManager.Listener {
public class VideoActor extends CameraActor {
    private static final String TAG = "VideoActor";

    protected CameraActivity mCameraActivity;
    protected RecordingView mRecordingView;

    private int mCurrentMode = ModePicker.MODE_VIDEO;

    private ModuleManager mModuleManager;
    private CameraErrorCallback mErrorCallback;

    private final ICameraAppUi mCameraAppUI;

    public VideoActor(CameraActivity context, ModuleManager moduleManager, int mode) {
        super(context);
        Log.i(TAG, "[VideoActor]constructor...");

        mCameraActivity = getContext();
        mErrorCallback = new CameraErrorCallback(mCameraActivity);
        mCameraAppUI = mCameraActivity.getCameraAppUI();
        mModuleManager = moduleManager;
        prepareCurrentMode(mode);
    }

    @Override
    public void onMediaEject() {
        Log.i(TAG, "[onMediaEject]...");
        mModuleManager.onMediaEject();
    }

    @Override
    public void onCameraClose() {
        Log.i(TAG, "[onCameraClose]");
        mModuleManager.onCameraClose();
    }
    @Override
    public void onCameraParameterReady(boolean startPreview) {
        Log.i(TAG, "[onCameraParameterReady]startPreview = " + startPreview);
        mModuleManager.onCameraParameterReady(startPreview);
        mCameraActivity.setCameraState(CameraActivity.STATE_IDLE);
    }

    @Override
    public void stopPreview() {
        Log.i(TAG,
                "[stopPreview] mVideoContext.getCameraState()=" + mCameraActivity.getCameraState());
        // since start preview is async, so during start preview, stop preview
        // may be called!
        // change photo mode to video mode, preview must stopped.
        // so we should not stop this case and native handle multiple call stop
        // preview.
        if (mModuleManager.stopPreview()) {
            return;
        }
        if (mCameraActivity.getCameraDevice() != null) {
            mCameraActivity.getCameraDevice().stopPreview();
            mCameraActivity.setCameraState(CameraActivity.STATE_PREVIEW_STOPPED);
        }
    }

    @Override
    public void onRestoreSettings() {
        mModuleManager.onRestoreSettings();
    }

    @Override
    public int getMode() {
        return mCurrentMode;
    }

    @Override
    public OnShutterButtonListener getVideoShutterButtonListener() {
        return mVideoShutterListener;
    }

    @Override
    public OnShutterButtonListener getPhotoShutterButtonListener() {
        return mPhotoShutterListener;
    }

    @Override
    public boolean onUserInteraction() {
        if (!mModuleManager.onUserInteraction()) {
            mCameraActivity.keepScreenOnAwhile();
            return true;
        }
        return false;
    }

    @Override
    public void setSurfaceTextureReady(boolean ready) {
        mModuleManager.setSurfaceTextureReady(ready);
    }

    @Override
    public boolean onBackPressed() {
        Log.i(TAG, "[onBackPressed]");
        return mModuleManager.onBackPressed();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return mModuleManager.onKeyDown(keyCode, event);
    }

    @Override
    public OnLongPressListener getonLongPressListener() {
        return mOnLongPressListener;
    }

    public ErrorCallback getErrorCallback() {
        return mErrorCallback;
    }

    public OnSingleTapUpListener getonSingleTapUpListener() {
        return mTapupListener;
    }

    public void prepareCurrentMode(int newMode) {
        Log.i(TAG, "[prepareCurrentMode] newMode = " + newMode);
        mCurrentMode = newMode;
        CameraModeType mode = getCameraModeType(mCurrentMode);
        if (mode == null) {
            mode = CameraModeType.EXT_MODE_VIDEO;
        }
        mModuleManager.createMode(mode);
    }

    public int getRecordingRotation(int orientation, int cameraId) {
        Log.i(TAG, "[getRecordingRotation]orientation = " + orientation +
                ",cameraId = " + cameraId);
        int rotation;
        if (orientation != OrientationEventListener.ORIENTATION_UNKNOWN) {
            CameraInfo info = CameraHolder.instance().getCameraInfo()[cameraId];
            if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
                rotation = (info.orientation - orientation + 360) % 360;
            } else { // back-facing camera
                rotation = (info.orientation + orientation) % 360;
            }
        } else {
            // Get the right original orientation
            CameraInfo info = CameraHolder.instance().getCameraInfo()[cameraId];
            rotation = info.orientation;
        }

        return rotation;
    }

    @Override
    public void release() {
        mModuleManager.closeMode();
        releaseActor();
    }

    @Override
    public OnClickListener getOkListener() {
        return mOkListener;
    }

    @Override
    public OnClickListener getCancelListener() {
        return mCancelListener;
    }

    public void releaseActor() {
        Log.i(TAG, "[releaseActor]...");
        mVideoShutterListener = null;
        if (mCameraActivity.getFocusManager() != null) {
            mCameraActivity.getFocusManager().removeMessages();
        }
        if (mRecordingView != null) {
            mRecordingView.uninit();
        }
        mFocusManager = null;
    }

    private OnShutterButtonListener mPhotoShutterListener = new OnShutterButtonListener() {
        @Override
        public void onShutterButtonLongPressed(ShutterButton button) {
            mModuleManager.onShutterButtonLongPressed();
        }

        @Override
        public void onShutterButtonFocus(ShutterButton button, boolean pressed) {
        }

        @Override
        public void onShutterButtonClick(ShutterButton button) {
            mModuleManager.onPhotoShutterButtonClick();
        }
    };

    private OnSingleTapUpListener mTapupListener = new OnSingleTapUpListener() {
        public void onSingleTapUp(View view, int x, int y) {
            mModuleManager.onSingleTapUp(view, x, y);
        }
    };

    private CameraActivity.OnLongPressListener mOnLongPressListener =
            new CameraActivity.OnLongPressListener() {
        @Override
        public void onLongPress(View view, int x, int y) {
            mModuleManager.onLongPress(view, x, y);
        }
    };

    private OnClickListener mOkListener = new OnClickListener() {
        public void onClick(View v) {
            mModuleManager.onOkButtonPress();
        }
    };

    private OnClickListener mCancelListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            mModuleManager.onCancelButtonPress();
            mCameraActivity.setResultExAndFinish(Activity.RESULT_CANCELED, new Intent());
        }
    };

    private void showVideoSnapshotUI(boolean enabled) {
        Log.d(TAG, " [showVideoSnapshotUI]enabled = " + enabled);
        if (!mCameraActivity.isVideoCaptureIntent()) {
            mCameraActivity.showBorder(enabled);
            // mVideoContext.getZoomManager().setEnabled(!enabled);
            mCameraAppUI.getCameraView(CommonUiType.ZOOM).setEnabled(!enabled);
            mCameraAppUI.getPhotoShutter().setEnabled(!enabled);
        }
    }

    private OnShutterButtonListener mVideoShutterListener = new OnShutterButtonListener() {
        @Override
        public void onShutterButtonLongPressed(ShutterButton button) {
        }

        @Override
        public void onShutterButtonFocus(ShutterButton button, boolean pressed) {
            mModuleManager.onShutterButtonFocus(pressed);
        }

        @Override
        public void onShutterButtonClick(ShutterButton button) {
            // video mode will set keep screen on so this should call
            // CameraActivity resetScreenOn to avoid CameraActivity affect video
            // mode
            mCameraActivity.resetScreenOn();
            mModuleManager.onVideoShutterButtonClick();
        }
    };
}
