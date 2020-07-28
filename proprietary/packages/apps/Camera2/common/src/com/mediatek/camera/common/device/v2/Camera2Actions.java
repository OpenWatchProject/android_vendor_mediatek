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

package com.mediatek.camera.common.device.v2;

/**
 * A helper class to define operations on camera2 as constant.
 */
class Camera2Actions {
    //Session Type.
    static final int CREATE_CAPTURE_SESSION = 101;
    static final int CREATE_REPROCESSABLE_CAPTURE_SESSION = 102;
    static final int CREATE_CONSTRAINED_HIGH_SPEED_CAPTURE_SESSION = 103;
    static final int CREATE_CAPTURE_REQUEST = 104;
    static final int CREATE_REPROCESSABLE_CAPTURE_REQUEST = 105;
    static final int CLOSE_DEVICE = 106;

    //Request Type.
    static final int PREPARE = 201;
    static final int CAPTURE = 202;
    static final int CAPTURE_BURST = 203;
    static final int SET_REPEATING_REQUEST = 204;
    static final int SET_REPEATING_BURST = 205;
    static final int STOP_REPEATING = 206;
    static final int ABORT_CAPTURES = 207;
    static final int GET_INPUT_SURFACE = 208;
    static final int CLOSE_SESSION = 209;
    static final int IS_REPROCESSABLE = 210;
    static final int CREATE_HIGH_SPEED_REQUEST = 211;
    static final int FINALIZE_OUTPUTCONFIGURATIONS = 212;

    /**
     * Stringify operation constant to string.
     *
     * @param operation the camera operation
     * @return Stringified result.
     */
    static String stringify(int operation) {
        switch (operation) {
            case CREATE_CAPTURE_SESSION:
                return "createCaptureSession";
            case CREATE_REPROCESSABLE_CAPTURE_SESSION:
                return "createReprocessableCaptureSession";
            case CREATE_CONSTRAINED_HIGH_SPEED_CAPTURE_SESSION:
                return "createConstrainedHighSpeedCaptureSession";
            case CREATE_CAPTURE_REQUEST:
                return "createCaptureRequest";
            case CREATE_REPROCESSABLE_CAPTURE_REQUEST:
                return "createReprocessCaptureRequest";
            case CLOSE_DEVICE:
                return "close device";
            case PREPARE:
                return "prepare";
            case CAPTURE:
                return "capture";
            case CAPTURE_BURST:
                return "captureBurst";
            case SET_REPEATING_REQUEST:
                return "setRepeatingRequest";
            case SET_REPEATING_BURST:
                return "setRepeatingBurst";
            case STOP_REPEATING:
                return "stopRepeating";
            case ABORT_CAPTURES:
                return "abortCaptures";
            case GET_INPUT_SURFACE:
                return "getInputSurface";
            case CLOSE_SESSION:
                return "close session";
            case CREATE_HIGH_SPEED_REQUEST:
                return "create high speed request";
            case FINALIZE_OUTPUTCONFIGURATIONS:
                return "finalize output configurations";
            default:
                return "UNKNOWN(" + operation + ")";
        }
    }

    /**
     * Jude the message whether is a session message type or not.
     * @param message which message need jude..
     * @return true means is session type.
     */
    static boolean isSessionMessageType(int message) {
        boolean value;
        switch (message) {
            case CREATE_CAPTURE_SESSION:
            case CREATE_REPROCESSABLE_CAPTURE_SESSION:
            case CREATE_CONSTRAINED_HIGH_SPEED_CAPTURE_SESSION:
            case CREATE_CAPTURE_REQUEST:
            case CREATE_REPROCESSABLE_CAPTURE_REQUEST:
            case CLOSE_DEVICE:
                value = true;
                break;
            default:
                value = false;
        }
        return value;
    }

    private Camera2Actions() {
        throw new AssertionError();
    }
}