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

import java.io.IOException;

public interface ICamera {
    Camera getInstance();

    void addCallbackBuffer(byte[] callbackBuffer);

    /**
     * set the raw image callback for dng capture.
     * @param meta the meta data callback.
     * @param raw16 the raw data callback.
     */
    //TODO: Dng BSP build error
//    void setRawImageCallback(MetadataCallback meta, PictureCallback raw16);

    void autoFocus(AutoFocusCallback cb);

    void cancelAutoFocus();

    void cancelContinuousShot();

    void lock();

    Parameters getParameters();

    void release();

    void reconnect() throws IOException;

    void setAsdCallback(AsdCallback cb);

    void setAutoFocusMoveCallback(AutoFocusMoveCallback cb);

    void setUncompressedImageCallback(PictureCallback cb);

    void setAutoRamaCallback(AutoRamaCallback cb);

    void setAutoRamaMoveCallback(AutoRamaMoveCallback cb);

    /**
     * Set Stereo Data Callback.
     * Need setData callback when stereo capture
     * @param cb need to set.
     */
    void setDataCallback(StereoCameraDataCallback cb);

    void setWarningCallback(StereoCameraWarningCallback cb);

    void setDistanceInfoCallback(DistanceInfoCallback cb);

    void setContext(Context context);

    void setContinuousShotCallback(ContinuousShotCallback callback);

    void setContinuousShotSpeed(int speed);

    void setDisplayOrientation(int degrees);

    void setErrorCallback(ErrorCallback cb);

    void setFaceDetectionListener(FaceDetectionListener listener);

    void setFbOriginalCallback(FbOriginalCallback cb);

    void setParameters(Parameters params);

    void setPreviewCallbackWithBuffer(PreviewCallback cb);

    void setPreviewTexture(SurfaceTexture surfaceTexture) throws IOException;

    void setPreviewDisplay(SurfaceHolder holder) throws IOException;

    void setZoomChangeListener(OnZoomChangeListener listener);

    // void slowdownContinuousShot();
    void startAutoRama(int num);

    void startFaceDetection();

    void startPreview();

    void startSmoothZoom(int value);

    void stopAutoRama(int isMerge);

    void stopFaceDetection();

    void stopPreview();

    void takePicture(ShutterCallback shutter, PictureCallback raw, PictureCallback jpeg);

    void takePicture(ShutterCallback shutter, PictureCallback raw, PictureCallback postview,
            PictureCallback jpeg);

    void unlock();

    public void setOneShotPreviewCallback(PreviewCallback cb);
}
