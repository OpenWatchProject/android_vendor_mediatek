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
import com.android.camera.FocusManager;
import com.android.camera.FocusManager.Listener;
import com.android.camera.manager.ModePicker;
import com.android.camera.ui.ShutterButton.OnShutterButtonListener;

import android.content.Intent;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.provider.MediaStore;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View.OnClickListener;

import com.mediatek.camera.ICameraMode.CameraModeType;

//just control capture flow, don't set parameters
public abstract class CameraActor {
    private static final String TAG = "CameraActor";

    protected final CameraActivity mContext;
    protected FocusManager mFocusManager;

    public CameraActor(final CameraActivity context) {
        mContext = context;
    }

    public CameraActivity getContext() {
        return mContext;
    }

    public AutoFocusMoveCallback getAutoFocusMoveCallback() {
        return null;
    }

    public ErrorCallback getErrorCallback() {
        return null;
    }

    public FaceDetectionListener getFaceDetectionListener() {
        return null;
    }

    // user action
    public boolean onUserInteraction() {
        return false;
    }

    public boolean onBackPressed() {
        return false;
    }

    public boolean onKeyDown(final int keyCode, final KeyEvent event) {
        return false;
    }

    public boolean onKeyUp(final int keyCode, final KeyEvent event) {
        return false;
    }

    public void onActivityResult(final int requestCode, final int resultCode, final Intent data) {
    }

    public void onMediaEject() {
    }

    public void onRestoreSettings() {
    }

    // public void onConfigurationChanged(Configuration newConfig){}
    // public void onCameraSwitched(int newCameraId){}//not recommended

    // shutter button callback
    public OnShutterButtonListener getVideoShutterButtonListener() {
        return null;
    }

    public OnShutterButtonListener getPhotoShutterButtonListener() {
        return null;
    }

    public OnSingleTapUpListener getonSingleTapUpListener() {
        return null;
    }

    public OnLongPressListener getonLongPressListener() {
        return null;
    }

    public OnClickListener getPlayListener() {
        return null;
    }

    public OnClickListener getRetakeListener() {
        return null;
    }

    public OnClickListener getOkListener() {
        return null;
    }

    public OnClickListener getCancelListener() {
        return null;
    }

    public Listener getFocusManagerListener() {
        return null;
    }

    // camera life cycle
    public void onCameraOpenDone() {
    }// called in opening thread

    public void onCameraOpenFailed() {
    }

    public void onCameraDisabled() {
    }

    public void onCameraParameterReady(boolean startPreview) {
    }// may be called in opening thread

    public void stopPreview() {

    }

    public void onCameraClose() {
    }

    public boolean handleFocus() {
        return false;
    }

    public void release() {
    }

    public void onOrientationChanged(int orientation) {
    }

    public abstract int getMode();

    protected boolean isFromInternal() {
        final Intent intent = mContext.getIntent();
        final String action = intent.getAction();
        Log.i(TAG, "Check action = " + action);
        // menu helper ?
        return (MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA.equals(action));
    }

    public void onDisplayRotate() {
    }

    public void onLongPress(int x, int y) {
    }

    public void setSurfaceTextureReady(boolean ready) {
    }

    public void onCameraDeviceSwitch() {

    }

    public void startFaceDetection() {

    }

    public void stopFaceDetection() {

    }

    public CameraModeType getCameraModeType(int newMode) {
        CameraModeType mode = null;
        switch (newMode) {
        case ModePicker.MODE_PANORAMA:
            mode = CameraModeType.EXT_MODE_PANORAMA;
            break;

        case ModePicker.MODE_PHOTO:
            mode = CameraModeType.EXT_MODE_PHOTO;
            break;

        case ModePicker.MODE_PHOTO_PIP:
            mode = CameraModeType.EXT_MODE_PHOTO_PIP;
            break;

        case ModePicker.MODE_STEREO_CAMERA:
            mode = CameraModeType.EXT_MODE_STEREO_CAMERA;
            break;

        case ModePicker.MODE_PHOTO_STEREO:
            mode = CameraModeType.EXT_MODE_PHOTO_STEREO;
            break;

        case ModePicker.MODE_VIDEO_STEREO:
            mode = CameraModeType.EXT_MODE_VIDEO_STEREO;
            break;

        case ModePicker.MODE_VIDEO:
            mode = CameraModeType.EXT_MODE_VIDEO;
            break;

        case ModePicker.MODE_FACE_BEAUTY:
            mode = CameraModeType.EXT_MODE_FACE_BEAUTY;
            break;

        case ModePicker.MODE_VIDEO_PIP:
            mode = CameraModeType.EXT_MODE_VIDEO_PIP;
            break;

        default:
            break;
        }
        return mode;
    }
}
