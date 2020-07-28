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

package com.mediatek.camera.v2.control.exposure;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.view.ViewGroup;

import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;

/**
 * Interact with camera module to manage exposure.
 */
public interface IExposure {
    /**
     * The life cycle corresponding to camera activity onCreate.
     *
     * @param activity
     *            Camera activity.
     * @param parentView
     *            The root view of camera activity.
     * @param isCaptureIntent
     *            {@link com.android.camera.v2.CameraActivityBridge #isCaptureIntent()}.
     */

    public void open(Activity activity, ViewGroup parentView, boolean isCaptureIntent);
    /**
     * The life cycle corresponding to camera activity onResume.
     */

    public void resume();
    /**
     * The life cycle corresponding to camera activity onPause.
     */

    public void pause();
    /**
     * The life cycle corresponding to camera activity onDestory.
     */

    public void close();

    /**
     * {@link PreviewStatusListener
     * #onPreviewLayoutChanged(android.view.View, int, int, int, int, int, int, int, int)}.
     *
     * @param previewArea
     *            The preview area.
     */

    public void onPreviewAreaChanged(RectF previewArea);
    /**
     * {@link com.android.camera.v2.app.GestureManager.GestureNotifier
     * #onSingleTapUp(float, float)}.
     *
     * @param x The x-coordinate.
     * @param y The y-coordinate.
     */
    public void onSingleTapUp(float x, float y);

    /**
     * Configuring capture requests.
     * @param requestType The required request, which is one of the {@link RequestType}.
     * @param requestBuilder The builders of capture requests.
     * @param captureType The required capture, which is one of the {@link CaptureType}.
     * @param bottomCamera Whether it is the main camera or not.
     */
    public void configuringSessionRequest(RequestType requestType,
            CaptureRequest.Builder requestBuilder, CaptureType captureType, boolean bottomCamera);

    /**
     * {@link android.hardware.camera2.CameraCaptureSession.CaptureCallback#onCaptureStarted}.
     *
     * @param request
     *            The request for the capture that just begun.
     * @param timestamp
     *            The timestamp at start of capture for a regular request, or the timestamp at the
     *            input image's start of capture for a reprocess request, in nanoseconds.
     * @param frameNumber
     *            The the frame number for this capture.
     */

    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp, long frameNumber);

    /**
     * {@see
     * android.hardware.camera2.CameraCaptureSession.CaptureCallback#onCaptureSequenceCompleted}.
     *
     * @param request
     *            The request that was given to the CameraDevice.
     * @param result
     *            The total output metadata from the capture, including the final capture
     *            parameters and the state of the camera system during capture.
     */

    public void onPreviewCaptureCompleted(CaptureRequest request, TotalCaptureResult result);

    /**
     * {@see android.hardware.camera2.CaptureRequest#CONTROL_AE_PRECAPTURE_TRIGGER
     * android.control.aePrecaptureTrigger}.
     */
    public void aePreTriggerAndCapture();
}
