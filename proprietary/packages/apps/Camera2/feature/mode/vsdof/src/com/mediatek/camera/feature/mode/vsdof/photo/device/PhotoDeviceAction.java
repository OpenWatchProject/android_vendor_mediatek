/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.mode.vsdof.photo.device;

/**
 * this is used for device controller action.
 */

public class PhotoDeviceAction {
    static final int OPEN_CAMERA = 1;
    static final int UPDATE_PREVIEW_SURFACE = 2;
    static final int SET_PREVIEW_CALLBACK = 3;
    static final int START_PREVIEW = 4;
    static final int STOP_PREVIEW = 5;
    static final int TAKE_PICTURE = 6;
    static final int UPDATE_G_SENSOR_ORIENTATION = 7;
    static final int CLOSE_CAMERA = 8;
    static final int GET_PREVIEW_SIZE = 9;
    static final int SET_PREVIEW_SIZE_READY_CALLBACK = 10;
    static final int SET_PICTURE_SIZE = 11;
    static final int REQUEST_CHANGE_SETTING_VALUE = 12;
    static final int REQUEST_CHANGE_COMMAND = 13;
    static final int REQUEST_CHANGE_COMMAND_IMMEDIATELY = 14;
    static final int IS_READY_FOR_CAPTURE = 15;
    static final int DESTROY_DEVICE_CONTROLLER = 16;
    static final int SET_VSDOF_LEVEL_PARAMETER = 17;
    static final int GET_STEREO_PICTURE_SIZES = 18;
    static final int SET_STEREO_WARNING_CALLBACK = 19;


    static final int ON_CAMERA_OPENED = 201;
    static final int ON_CAMERA_CLOSED = 202;
    static final int ON_CAMERA_DISCONNECTED = 203;
    static final int ON_CAMERA_ERROR = 204;

    static String stringify(int operation) {
        switch (operation) {
            case OPEN_CAMERA:
                return "openCamera";
            case UPDATE_PREVIEW_SURFACE:
                return "updatePreviewSurface";
            case SET_PREVIEW_CALLBACK:
                return "setPreviewCallback";
            case START_PREVIEW:
                return "startPreview";
            case STOP_PREVIEW:
                return "stopPreview";
            case TAKE_PICTURE:
                return "takePicture";
            case UPDATE_G_SENSOR_ORIENTATION:
                return "updateGSensorOrientation";
            case CLOSE_CAMERA:
                return "closeCamera";
            case GET_PREVIEW_SIZE:
                return "getPreviewSize";
            case SET_PREVIEW_SIZE_READY_CALLBACK:
                return "setPreviewSizeReadyCallback";
            case SET_PICTURE_SIZE:
                return "setPictureSize";
            case REQUEST_CHANGE_SETTING_VALUE:
                return "requestChangeSettingValue";
            case REQUEST_CHANGE_COMMAND:
                return "requestChangeCommand";
            case REQUEST_CHANGE_COMMAND_IMMEDIATELY:
                return "requestChangeCommandImmediately";
            case IS_READY_FOR_CAPTURE:
                return "isReadyForCapture";
            case ON_CAMERA_OPENED:
                return "onCameraOpened";
            case ON_CAMERA_CLOSED:
                return "onCameraClosed";
            case ON_CAMERA_DISCONNECTED:
                return "onCameraDisconnected";
            case ON_CAMERA_ERROR:
                return "onCameraError";
            case DESTROY_DEVICE_CONTROLLER:
                return "destroyDeviceControllerThread";
            case SET_VSDOF_LEVEL_PARAMETER:
                return "setVsdofLevelParameter";
            case GET_STEREO_PICTURE_SIZES:
                return "getStereoPicturesizes";
            default:
                return "UNKNOWN(" + operation + ")";
        }
    }
}
