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
package com.android.camera;

import android.content.Context;
import android.graphics.SurfaceTexture;

import android.hardware.Camera;
import android.hardware.Camera.AsdCallback;
import android.hardware.Camera.AutoRamaCallback;
import android.hardware.Camera.AutoRamaMoveCallback;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.ContinuousShotCallback;
import android.hardware.Camera.DistanceInfoCallback;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.FbOriginalCallback;
//TODO: Dng BSP build error
//import android.hardware.Camera.MetadataCallback;
import android.hardware.Camera.OnZoomChangeListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.StereoCameraDataCallback;
import android.hardware.Camera.StereoCameraWarningCallback;
import android.view.SurfaceHolder;

import com.mediatek.camera.util.ReflectUtil;

import java.io.IOException;
import java.lang.reflect.Method;

public class AndroidCamera implements ICamera {
    private static final String TAG = "AndroidCamera";

    protected Camera mCamera = null;
    Method mSetUncompressedImageCallbackMethod;

    public AndroidCamera(Camera camera) {
        Util.assertError(null != camera);
        mCamera = camera;

        mSetUncompressedImageCallbackMethod =
                ReflectUtil.getMethod(Camera.class, "setUncompressedImageCallback",
                        PictureCallback.class);
    }

    public Camera getInstance() {
        return mCamera;
    }

    public void addCallbackBuffer(byte[] callbackBuffer) {
        mCamera.addCallbackBuffer(callbackBuffer);
    }

    /**
     * set raw image callback when capture dng.
     * @param meta the meta data callback.
     * @param raw16 the raw image callback.
     */
    //TODO: Dng BSP build error
//    public void setRawImageCallback(MetadataCallback meta, PictureCallback raw16) {
//        mCamera.setRaw16Callback(meta, raw16);
//        if (meta != null && raw16 != null) {
//            mCamera.enableRaw16(true);
//        } else {
//            mCamera.enableRaw16(false);
//        }
//    }

    public void autoFocus(AutoFocusCallback cb) {
        mCamera.autoFocus(cb);
    }

    public void cancelAutoFocus() {
        mCamera.cancelAutoFocus();
    }

    public void cancelContinuousShot() {
        mCamera.cancelContinuousShot();
    }

    public void lock() {
        mCamera.lock();
    }

    public Parameters getParameters() {
        return mCamera.getParameters();
    }

    public void release() {
        mCamera.release();
    }

    public void reconnect() throws IOException {
        mCamera.reconnect();
    }

    public void setAsdCallback(AsdCallback cb) {
        mCamera.setAsdCallback(cb);
    }

    public void setAutoFocusMoveCallback(AutoFocusMoveCallback cb) {
        mCamera.setAutoFocusMoveCallback(cb);
    }

    public void setUncompressedImageCallback(PictureCallback cb) {
        if (mSetUncompressedImageCallbackMethod != null) {
            ReflectUtil.callMethodOnObject(mCamera, mSetUncompressedImageCallbackMethod, cb);
        }
    }

    public void setAutoRamaCallback(AutoRamaCallback cb) {
        mCamera.setAutoRamaCallback(cb);
    }

    public void setAutoRamaMoveCallback(AutoRamaMoveCallback cb) {
        mCamera.setAutoRamaMoveCallback(cb);
    }

    public void setFbOriginalCallback(FbOriginalCallback cb) {
        mCamera.setFbOriginalCallback(cb);
    }

    public void setContext(Context context) {
    }

    public void setContinuousShotCallback(ContinuousShotCallback callback) {
        mCamera.setContinuousShotCallback(callback);
    }

    public void setContinuousShotSpeed(int speed) {
        mCamera.setContinuousShotSpeed(speed);
    }

    public void setDisplayOrientation(int degrees) {
        mCamera.setDisplayOrientation(degrees);
    }

    public void setErrorCallback(ErrorCallback cb) {
        mCamera.setErrorCallback(cb);
    }

    public void setFaceDetectionListener(FaceDetectionListener listener) {
        mCamera.setFaceDetectionListener(listener);
    }

    public void setParameters(Parameters params) {
        mCamera.setParameters(params);
    }

    public void setPreviewCallbackWithBuffer(PreviewCallback cb) {
        mCamera.setPreviewCallbackWithBuffer(cb);
    }

    public void setPreviewTexture(SurfaceTexture surfaceTexture) throws IOException {
        mCamera.setPreviewTexture(surfaceTexture);
    }

    public void setPreviewDisplay(SurfaceHolder holder) throws IOException {
        mCamera.setPreviewDisplay(holder);
    }

    /**
     * Set Stereo Data Callback.
     * Need setData callback when stereo capture
     * @param cb need to set.
     */
    public void setDataCallback(StereoCameraDataCallback cb) {
        mCamera.setStereoCameraDataCallback(cb);
    }

    public void setWarningCallback(StereoCameraWarningCallback cb) {
        mCamera.setStereoCameraWarningCallback(cb);
    }

    public void setDistanceInfoCallback(DistanceInfoCallback cb) {
        mCamera.setDistanceInfoCallback(cb);
    }

    public void setZoomChangeListener(OnZoomChangeListener listener) {
        mCamera.setZoomChangeListener(listener);
    }

    public void startAutoRama(int num) {
        mCamera.startAutoRama(num);
    }

    public void startFaceDetection() {
        mCamera.startFaceDetection();
    }

    public void startPreview() {
        mCamera.startPreview();
    }

    public void startSmoothZoom(int value) {
        mCamera.startSmoothZoom(value);
    }

    public void stopAutoRama(int isMerge) {
        mCamera.stopAutoRama(isMerge);
    }

    public void stopFaceDetection() {
        mCamera.stopFaceDetection();
    }

    public void stopPreview() {
        mCamera.stopPreview();
    }

    public void takePicture(ShutterCallback shutter, PictureCallback raw, PictureCallback jpeg) {
        mCamera.takePicture(shutter, raw, jpeg);
    }

    public void takePicture(ShutterCallback shutter, PictureCallback raw, PictureCallback postview,
            PictureCallback jpeg) {
        mCamera.takePicture(shutter, raw, postview, jpeg);
    }

    public void unlock() {
        mCamera.unlock();
    }

    public void setOneShotPreviewCallback(PreviewCallback cb) {
        mCamera.setOneShotPreviewCallback(cb);
    }
}
