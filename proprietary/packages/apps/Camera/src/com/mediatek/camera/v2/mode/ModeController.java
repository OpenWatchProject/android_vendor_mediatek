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

package com.mediatek.camera.v2.mode;


import java.util.List;
import java.util.Map;

import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.stream.StreamManager;

import android.graphics.RectF;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.view.Surface;
import android.view.ViewGroup;

/**
 *  This controller is provided to <item>Module</item>,
 *  <item>Module</item> can use these interfaces
 *  to control mode. Interfaces are divided into the following categories:
 *  <p>
 *  <li> Life cycle.</li>
 *  <li> App level resources.</li>
 *  <li> Shutter button.</li>
 *  <li> Gestures.</li>
 *  <li> Session outputs, requests, results.</li>
 */
public interface ModeController {
    // life cycle
    public void open(
            StreamManager streamManager,
            ViewGroup parentView,
            boolean isCaptureIntent);
    public void resume();
    /**
     *  Notify mode activity pause event happen
     *  for preview draw buffer opengl error.
     */
    public void onActivityPause();
    public void pause();
    public void close();

    // app level resources
    public void prepareSurfaceBeforeOpenCamera();
    public void onOrientationChanged(int orientation);
    public void onPreviewVisibilityChanged(int visibility);
    public void onPreviewAreaChanged(RectF previewArea);
    public boolean onBackPressed();
    public boolean switchCamera(String cameraId);

    // shutter button
    public void onShutterPressed(boolean isVideo);
    public void onShutterClicked(boolean isVideo);
    public void onShutterLongPressed(boolean isVideo);
    public void onShutterReleased(boolean isVideo);

    // <item>Mode</item> receives gestures from <item>Module</item>
    public ModeGestureListener getModeGestureListener();
    public interface ModeGestureListener {
        public boolean onDown(float x, float y);
        public boolean onScroll(float dx, float dy, float totalX, float totalY);
        public boolean onSingleTapUp(float x, float y);
        public boolean onLongPress(float x, float y);
        public boolean onUp();
    }

    // Session outputs and requests
    public void configuringSessionOutputs(
            List<Surface> sessionOutputSurfaces,
            boolean bottomCamera);
    public void configuringSessionRequests(
            Map<RequestType, CaptureRequest.Builder> requestBuilders,
            boolean bottomCamera);
    // Session results
    public void onFirstFrameAvailable();
    public void onPreviewCaptureStarted(
            CaptureRequest request,
            long timestamp,
            long frameNumber);
    public void onPreviewCaptureCompleted(
            CaptureRequest request,
            TotalCaptureResult result);
    public CaptureCallback getCaptureCallback();

    // TODO review ui seems not common ui, it's better move to
    // <item>Stream</item> or <item>Mode</item>
    public void onPlay();
    public void onRetake();
    public void onOkClick();
    public void onCancelClick();

    /**
     * Notify mode current preview camera have changed,.
     * @param previewCameraId
     *            current preview camera id.
     * @param hasOpenedCamera
     *            current has opened camera id.
     */
    public void updatePreviewCamera(List<String> previewCameraId, List<String> hasOpenedCamera);

    /**
     * Create a new camera capture session by providing the target output set of
     * Surfaces to the camera device.
     * @param sessionOutputSurfaces
     *            which surfaces need to be configured.
     * @param cameraId
     *            Which camera will be configured.
     */
    public void configuringSessionOutputs(List<Surface> sessionOutputSurfaces, String cameraId);

    /**
     * Configure request such as preview and still capture.
     * @param requestBuilders
     *            A {@link CaptureRequest.Builder} for new capture requests,
     *            initialized with template for a target use case. The settings
     *            are chosen to be the best options for the specific camera
     *            device, so it is not recommended to reuse the same request for
     *            a different camera device; create a builder specific for that
     *            device and template and override the settings as desired,
     *            instead.
     * @param cameraId which camera will be requested.
     */
    public void configuringSessionRequests(
            Map<RequestType, CaptureRequest.Builder> requestBuilders, String cameraId);

    /**
     * update the remote camera status when remote camera is changed.
     * @param event orientation changed will be triggered this event.
     * @param value the orientation value.
     * @param cameraId which camera is changed.
     */
    public void onServiceEventUpdate(int event, int value, String cameraId);
}
