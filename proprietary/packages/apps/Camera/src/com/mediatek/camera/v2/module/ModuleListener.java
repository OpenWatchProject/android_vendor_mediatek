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

package com.mediatek.camera.v2.module;

import com.mediatek.camera.v2.control.IControl.IAaaController;

/**
 * The listener used by 3A, Mode, Addition.
 */
public interface ModuleListener {

    /**
     * RequestTypee decides which type of CaptureRequest will be created.
     */
    public static enum RequestType {
        PREVIEW,             // map to TEMPLATE_PREVIEW
        STILL_CAPTURE,       // map to TEMPLATE_STILL_CAPTURE
        RECORDING,           // map to TEMPLATE_RECORD
        VIDEO_SNAP_SHOT,     // map to TEMPLATE_VIDEO_SNAPSHOT
        ZERO_SHUTTER_DELAY,  // map to TEMPLATE_ZERO_SHUTTER_LAG
        MANUAL,              // map to TEMPLATE_MANUAL
    }
    /**
     * CaptureType decides how we will pass CaptureRequests to CameraCaptureSession.
     */
    public static enum CaptureType {
        CAPTURE,             // map to capture
        CAPTURE_BURST,       // map to captureBurst
        REPEATING_REQUEST,   // map to setRepeatingRequest
        REPEATING_BURST,     // map to setRepeatingBurst
    }

    public void requestChangeCaptureRequets(boolean sync,
            RequestType requestType, CaptureType captureType);

    /**
     *
     * @param requestType the required request, which is one of the {@link RequestType}
     * @param captureType the required capture, which is one of the {@link CaptureType}
     */
    public void requestChangeCaptureRequets(boolean isMainCamera,
            boolean sync, RequestType requestType, CaptureType captureType);

    /**
     *
     * @param sync whether should respond this request immediately
     *        true   request immediately
     *        false  wait all requests been submitted and remove the same request
     *               current design is only for
     *               {@link ISettingChangedListener#onSettingChanged(java.util.Map)}
     */
    public void requestChangeSessionOutputs(boolean sync);

    /**
     *
     * @param sync whether should respond this request immediately
     *        true   request immediately
     *        false  wait all requests been submitted and remove the same request
     *               current design is only for
     *               {@link ISettingChangedListener#onSettingChanged(java.util.Map)}
     */
    public void requestChangeSessionOutputs(boolean sync, boolean isMainCamera);

    /**
     * Get the controller of 3a.
     * @return the controller of 3a.
     */
    public IAaaController get3AController(String cameraId);

    /**
     * Repeating request type is managed by module. Any other component
     * should get request type from module.
     *
     * @return current repeating request type.
     */
    public RequestType getRepeatingRequestType();

    /**
     * if current preview surface have changed,will fired this method.
     */
    public void onPreviewSurfaceReady();

    /**
     * used for Cross Mount when mode is start recording ,will request notify to
     * Module.
     * @param isRecordingStated
     *            true means current is recording.otherwise is not in recording.
     */
    public void requestUpdateRecordingStatus(boolean isRecordingStated);

    /**
     * used for Cross Mount. when mode is doing capture,need notify module,
     * current is capturing. when finished capture, also need update the status
     *
     * @param isCapturing
     *            true means current is in capturing; otherwise is not
     *            capturing.
     */
    public void requestUpdateCaptureStatus(boolean isCapturing);

    /**
     * request change current session.
     * @param sync
     *            true request immediately. false wait all requests been
     *            submitted and remove the same request current design is only
     *            for
     *            {@link ISettingChangedListener#onSettingChanged(java.util.Map)}
     * @param camerId
     *            which camera need to configure session.
     */
    public void requestSessionOutputs(boolean sync, String camerId);

    /**
     * change current request,such as preview and capture.
     * @param cameraId
     *            which camera need to configure the request.
     * @param sync
     *            true request immediately. false wait all requests been
     *            submitted and remove the same request current design is only
     *            for
     *            {@link ISettingChangedListener#onSettingChanged(java.util.Map)}
     * @param requestType
     *            the required request, which is one of the {@link RequestType}
     * @param captureType
     *            the required capture, which is one of the {@link CaptureType}
     */
    public void requestCaptureRequets(String cameraId, boolean sync, RequestType requestType,
            CaptureType captureType);
}
