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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

package com.mediatek.camera.common.device.v1;

/**
 * A helper class used to define camera operations as constant.
 */
class CameraActions {
    // Camera instance operation
    static final int CLOSE = 1;
    static final int RECONNECT = 2;
    static final int UNLOCK = 3;
    static final int LOCK = 4;
    static final int INIT_PARAMETERS = 5;
    // Preview operation
    static final int SET_PREVIEW_TEXTURE = 101;
    static final int START_PREVIEW = 102;
    static final int STOP_PREVIEW = 103;
    static final int SET_PREVIEW_CALLBACK_WITH_BUFFER = 104;
    static final int ADD_CALLBACK_BUFFER = 105;
    static final int SET_PREVIEW_DISPLAY = 106;
    static final int SET_PREVIEW_CALLBACK = 107;
    static final int SET_ONE_SHOT_PREVIEW_CALLBACK = 108;
    // Parameters operation
    static final int SET_PARAMETERS = 201;
    static final int GET_PARAMETERS = 202;
    static final int GET_ORIGINAL_PARAMETERS = 203;
    // Focus, Zoom operation
    static final int AUTO_FOCUS = 301;
    static final int CANCEL_AUTO_FOCUS = 302;
    static final int SET_AUTO_FOCUS_MOVE_CALLBACK = 303;
    static final int SET_ZOOM_CHANGE_LISTENER = 304;
    static final int START_SMOOTH_ZOOM = 305;
    static final int STOP_SMOOTH_ZOOM = 306;
    // Face detection operation
    static final int SET_FACE_DETECTION_LISTENER = 461;
    static final int START_FACE_DETECTION = 462;
    static final int STOP_FACE_DETECTION = 463;
    // Presentation operation
    static final int ENABLE_SHUTTER_SOUND = 501;
    static final int SET_DISPLAY_ORIENTATION = 502;
    // Capture operation
    static final int TAKE_PICTURE = 601;
    // vendor operation
    static final int SEND_COMMAND = 701;
    static final int SET_VENDOR_DATA_CALLBACK = 702;

    /**
     * Stringify operation constant to string.
     *
     * @param operation the camera operation
     * @return Stringified result.
     */
    static String stringify(int operation) {
        switch (operation) {
            case CLOSE:
                return "close";
            case RECONNECT:
                return "reconnect";
            case UNLOCK:
                return "unlock";
            case LOCK:
                return "lock";
            case INIT_PARAMETERS:
                return "initOriginalParameters";
            case SET_PREVIEW_TEXTURE:
                return "setPreviewTexture";
            case START_PREVIEW:
                return "startPreview";
            case STOP_PREVIEW:
                return "stopPreview";
            case SET_PREVIEW_CALLBACK_WITH_BUFFER:
                return "setPreviewCallbackWithBuffer";
            case ADD_CALLBACK_BUFFER:
                return "addCallbackBuffer";
            case SET_PREVIEW_DISPLAY:
                return "setPreviewDisplay";
            case SET_PREVIEW_CALLBACK:
                return "setPreviewCallback";
            case SET_ONE_SHOT_PREVIEW_CALLBACK:
                return "setOneShotPreviewCallback";
            case SET_PARAMETERS:
                return "setParameters";
            case GET_PARAMETERS:
                return "getParameters";
            case GET_ORIGINAL_PARAMETERS:
                return "getOriginalParameters";
            case AUTO_FOCUS:
                return "autofocus";
            case CANCEL_AUTO_FOCUS:
                return "cancelAutofocus";
            case SET_AUTO_FOCUS_MOVE_CALLBACK:
                return "setAutofocusMoveCallback";
            case SET_ZOOM_CHANGE_LISTENER:
                return "setZoomChangeListener";
            case START_SMOOTH_ZOOM:
                return "startSmoothZoom";
            case STOP_SMOOTH_ZOOM:
                return "stopSmoothZoom";
            case SET_FACE_DETECTION_LISTENER:
                return "setFaceDetectionListener";
            case START_FACE_DETECTION:
                return "startFaceDetection";
            case STOP_FACE_DETECTION:
                return "stopFaceDetection";
            case ENABLE_SHUTTER_SOUND:
                return "enableShutterSound";
            case SET_DISPLAY_ORIENTATION:
                return "setDisplayOrientation";
            case TAKE_PICTURE:
                return "takePicture";
            case SEND_COMMAND:
                return "sendCommand";
            case SET_VENDOR_DATA_CALLBACK:
                return "setVendorDataCallback";
            default:
                return "UNKNOWN(" + operation + ")";
        }
    }

    private CameraActions() {
        throw new AssertionError();
    }
}