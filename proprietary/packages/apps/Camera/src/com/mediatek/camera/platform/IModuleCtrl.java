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

package com.mediatek.camera.platform;

import com.android.camera.ComboPreferences;

import android.content.Intent;
import android.hardware.Camera.Face;
import android.location.Location;
import android.net.Uri;
import android.view.Surface;

import com.mediatek.camera.ICameraMode.CameraModeType;

//TODO: IMoudleController is the same as IPhotoController, need to do refactoring
public interface IModuleCtrl {
    /**
     * start face detection
     *
     * @return
     */
    public void startFaceDetection();

    /**
     * stop face detection
     */
    public void stopFaceDetection();

    /**
     * Locks the system orientation.
     *
     * @return
     */
    public boolean lockOrientation();

    /**
     * Unlocks the system orientation.
     *
     * @return
     */
    public boolean unlockOrientation();

    /**
     * get image orientation for preview.
     * @return orientation.
     */
    public int getJpegOrientation();

    public void setOrientation(boolean lock, int orientation);

    /**
     * enable the system orientation listener.
     *
     * @return
     */
    public boolean enableOrientationListener();

    /**
     * disable the system orientation listener.
     *
     * @return
     */
    public boolean disableOrientationListener();

    /**
     * apply focus parameters to camera device
     *
     * @return
     */
    public boolean applyFocusParameters(boolean setArea);

    /**
     * get display rotation
     *
     * @return
     */
    public int getDisplayRotation();

    /**
     * get display orientation
     *
     * @return
     */
    public int getOrientation();

    /**
     * get location
     *
     * @return
     */
    public Location getLocation();

    /**
     * check current is video capture intent
     *
     * @return is true means current camera is create from video capture
     *         intent,such as from MMS false means is not video capture intent
     */
    public boolean isVideoCaptureIntent();

    /**
     * check current is just from click the launcher camera icon,not is video
     * capture intent and video wallpaper intent
     *
     * @return true means current is from 3rd party false means current is just
     *         from launcher
     */
    public boolean isNonePickIntent();

    /**
     * get current activity's intent if there have a intent
     *
     * @return the intent of the object
     */
    public Intent getIntent();

    /**
     * return true means current is quick capture, in other words, when take a
     * pictured will return the 3rd party of APP, not stated in camera activity;
     */
    public boolean isQuickCapture();

    /**
     * back to former mode when leave current mode
     */
    public void backToLastMode();

    public Uri getSaveUri();

    public String getCropValue();

    public int getResolution();

    public void setResultAndFinish(int resultCode);

    public void setResultAndFinish(int resultCode, Intent data);

    public boolean getSurfaceTextureReady();

    public boolean isSecureCamera();

    public boolean isImageCaptureIntent();

    /**
     * if camera is started by 3rd party,when finished the pictre/video, need
     * back to the 3rd party
     *
     * @param resultCode
     *            The result code to propagate back to the originating activity,
     *            often RESULT_CANCELED or RESULT_OK
     * @param data
     *            The data to propagate back to the originating activity
     */
    public void backToCallingActivity(int resultCode, Intent data);

    /**
     * get the camera's preference current just use the Camera's preference,will
     * inner do to this TODO
     *
     * @return the preference of camera
     */
    public ComboPreferences getComboPreferences();

    public int getOrientationCompensation();

    public int getDisplayOrientation();

    public void switchCameraDevice();

    public CameraModeType getNextMode();

    public CameraModeType getPrevMode();

    /**
     * set the tag for current is in FaceBeauty view
     *
     * @param isEnable
     *            true means current is face beauty view, false means just in FD
     *            view
     * @return true means have set success; otherwise if fail
     */
    public boolean setFaceBeautyEnalbe(boolean isEnable);

    /**
     * initialize the frame view ,such as facebeauty view
     *
     * @param isEnableObject
     *            if is true means need initialize Object Tracking if is false
     *            means need initialize face view
     * @return current not use the return value
     */
    public boolean initializeFrameView(boolean isEnableObject);

    /**
     * clear the frame view if you don't need the view
     *
     * @return
     */
    public boolean clearFrameView();

    /**
     * set the face which is detected
     *
     * @param faces
     *            which the face detection is detected
     */
    public void setFaces(Face[] faces);

    /**
     * get preview surface
     */
    public Surface getPreviewSurface();

    /**
     * Return true if first start up camera
     */
    public boolean isFirstStartUp();

    /**
     * Notify the preview started status for remove the old surface.
     */
    public void previewStarted();
}
