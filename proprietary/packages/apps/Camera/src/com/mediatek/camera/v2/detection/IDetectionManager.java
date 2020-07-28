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

package com.mediatek.camera.v2.detection;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.view.ViewGroup;

import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;

import java.util.Map;

/**
 * Interact with camera module and manager sub detection features.
 */
public interface IDetectionManager {
    /**
     * Interface used for get and change a capture request.
     */
    public interface IDetectionListener {
        /**
         * Get the current repeating request type {@link RequestType}.
         *
         * @return The current type of capture request.
         */
        public RequestType getRepeatingRequestType();

        /**
         * Request change capture request.
         *
         * @param sync
         *            Whether should respond this request immediately, true request
         *            immediately,false wait all requests been submitted and remove the same request
         *            current design is only for
         *            {@link ISettingChangedListener#onSettingChanged(java.util.Map)}.
         * @param requestType
         *            The required request, which is one of the {@link RequestType}.
         * @param captureType
         *            The required capture, which is one of the {@link CaptureType}.
         */
        public void requestChangeCaptureRequest(boolean sync, RequestType requestType,
                CaptureType captureType);
    }

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
    void open(Activity activity, ViewGroup parentView, boolean isCaptureIntent);

    /**
     * The life cycle corresponding to camera activity onResume.
     */
    void resume();

    /**
     * The life cycle corresponding to camera activity onPause.
     */
    void pause();

    /**
     * The life cycle corresponding to camera activity onDestory.
     */
    void close();

    /**
     * {@link com.android.camera.v2.app.GestureManager.GestureNotifier
     * #onSingleTapUp(float, float)}.
     *
     * @param x
     *            The x-coordinate.
     * @param y
     *            The y-coordinate.
     */
    void onSingleTapUp(float x, float y);

    /**
     * {@link com.android.camera.v2.app.GestureManager.GestureNotifier #onLongPress(float, float)}.
     *
     * @param x
     *            The x-coordinate.
     * @param y
     *            The y-coordinate.
     */
    void onLongPressed(float x, float y);

    /**
     * {@link PreviewStatusListener
     * #onPreviewLayoutChanged(android.view.View, int, int, int, int, int, int, int, int)}.
     *
     * @param previewArea
     *            The preview area.
     */
    void onPreviewAreaChanged(RectF previewArea);

    /**
     * {@link com.android.camera.v2.CameraActivityBridge #onOrientationChanged(int)}.
     *
     * @param orientation
     *            The current G-sensor orientation.
     */
    void onOrientationChanged(int orientation);

    /**
     * Configuring capture requests.
     *
     * @param requestBuilders The builders of capture requests.
     * @param captureType {@link CaptureType}.
     */
    void configuringSessionRequests(Map<RequestType, CaptureRequest.Builder> requestBuilders,
            CaptureType captureType);

    /**
     * This method is called when the camera device has started capturing the output image for the
     * request, at the beginning of image exposure.
     *
     * @param request
     *            The request for the capture that just begun.
     * @param timestamp
     *            The time stamp at start of capture, in nanoseconds.
     * @param frameNumber
     *            The frame number for this capture.
     */
    void onCaptureStarted(CaptureRequest request, long timestamp, long frameNumber);

    /**
     * This method is called when an image capture has fully completed and all the result metadata
     * is available.
     *
     * @param request
     *            The request that was given to the CameraDevice
     *
     * @param result
     *            The total output metadata from the capture, including the final capture parameters
     *            and the state of the camera system during capture.
     */
    void onCaptureCompleted(CaptureRequest request, TotalCaptureResult result);
}