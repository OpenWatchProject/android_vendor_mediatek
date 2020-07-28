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

package com.mediatek.camera.common.device;


/**
 * This class is used for wrapping camera open exception. Include camera access exception:if a
 * camera device could not be queried or opened by the android.hardware.camera2.CameraManager}, or
 * if the connection to an opened android.hardware.camera2.CameraDevice is no longer valid.
 * <p>
 * also will be throw SecurityException if the application does not have permission to access the
 * camera.
 * </p>
 * <p>
 * this exception also include the runtime exception which throw by android.hardware.camera#{@link
 * open(int cameraId)},but this runtime exception will be wrapped to HARDWARE_EXCEPTION,you can show
 * a toast to user the camera open fail dialog.
 * </p>
 */
public class CameraOpenException extends Exception {
    private final ExceptionType mReason;

    /**
     * Which exception type maybe occur.
     */
    public enum ExceptionType {
        // Camera access exception type.
        /**
         * The camera device is removable and has been disconnected from the Android device, or the
         * camera id used with {@link android.hardware.camera2.CameraManager#openCamera} is no
         * longer valid, or the camera service has shut down the connection due to a higher-priority
         * access request for the camera device.
         */
        CAMERA_DISCONNECTED,

        /**
         * if the application does not have permission to access the camera.
         */
        SECURITY_EXCEPTION,

        /**
         * hardware exception if opening the camera fails (for example, if the camera is in use by
         * another process or device policy manager has disabled the camera).This exception type
         * just throw by android.hardware.camera#{@link open(int cameraId)}.
         */
        HARDWARE_EXCEPTION,
    }

    /**
     * Create a camera open exception with the exception type.
     * @param type Which exception is occured.
     */
    public CameraOpenException(ExceptionType type) {
        super(getDefaultMessage(type));
        mReason = type;
    }

    /**
     * Create a camera open exception with the exception type.
     * @param type Which exception is occured.
     * @param message which message want to show.
     */
    public CameraOpenException(ExceptionType type, String message) {
        super(message);
        mReason = type;
    }

    /**
     * Create a camera open exception with the exception type.
     * @param type Which exception is occured.
     * @param message which message want to display.
     * @param cause A throwable will be display.
     */
    public CameraOpenException(ExceptionType type, String message, Throwable cause) {
        super(message, cause);
        mReason = type;
    }

    /**
     * Create a camera open exception with the exception type.
     * @param type Which exception is occured.
     * @param casue A throwable will be display.
     */
    public CameraOpenException(ExceptionType type, Throwable casue) {
        super(getDefaultMessage(type), casue);
        mReason = type;
    }

    /**
     * The type for the failure to access the camera.
     * @return the exception type.
     */
    public ExceptionType getExceptionType() {
        return mReason;
    }

    /**
     * Get the default exception message with the exception type.
     * @param type which exception type is occur.
     * @return return the default message with the exception.
     */
    public static String getDefaultMessage(ExceptionType type) {
        switch (type) {

            case CAMERA_DISCONNECTED:
                return "The camera device is removable and has been disconnected from the " +
                        "Android device, or the camera service has shut down the connection due " +
                        "to a higher-priority access request for the camera device.";

            case SECURITY_EXCEPTION:
                return "the application does not have permission to access the camera";

            case HARDWARE_EXCEPTION:
                return "opening the camera fails (for example, if the camera is in use by another" +
                        " process or device policy manager has disabled the camera).";

            default:
                return "Unknown camera open exception, need check the case type";
        }
    }
}
