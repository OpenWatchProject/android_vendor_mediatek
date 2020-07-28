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

package com.mediatek.camera.common.mode.photo.device;

import android.graphics.ImageFormat;
import android.net.Uri;
import com.mediatek.camera.common.mode.photo.DeviceInfo;
import com.mediatek.camera.common.utils.Size;

import javax.annotation.Nonnull;

/**
 * Define the interaction apis between {@link PhotoMode} and Camera Device,
 * include {@link CameraProxy} and {@link Camera2Proxy}.
 *
 * This make photo mode no need to care the implementation of camera device,
 * photo mode control flow should compatible {@link CameraProxy} and {@link Camera2Proxy}
 */
public interface IDeviceController {

    /**
     * Used for wrapping data callback.
     */
    class DataCallbackInfo {
        public byte[] data;
        public boolean needUpdateThumbnail;
        public boolean needRestartPreview;
        public int mBufferFormat = ImageFormat.JPEG;
        public int imageWidth;
        public int imageHeight;
    }

    /**
     * This interface is used to callback uri to mode.
     */
    interface CaptureImageSavedCallback {
        /**
         * After saved image notify mode.
         * @param saved uri.
         */
        void onFinishSaved(Uri uri);
    }

    /**
     * This interface is used to callback jpeg data to mode.
     */
    interface CaptureDataCallback {
        /**
         * Notify jpeg data is generated.
         * @param dataCallbackInfo data callback info.
         */
        void onDataReceived(DataCallbackInfo dataCallbackInfo);

        /**
         * When post view callback will notify.
         * @param data post view data.
         */
        void onPostViewCallback(byte[] data);
    }

    /**
     * This callback to notify device state.
     */
    interface DeviceCallback {
        /**
         * Notified when camera opened done with camera id.
         * @param cameraId the camera is opened.
         */
        void onCameraOpened(String cameraId);

        /**
         * Notified before do close camera.
         */
        void beforeCloseCamera();

        /**
         * Notified call stop preview immediately.
         */
        void afterStopPreview();

        /**
         * When preview data is received,will fired this function.
         * @param data the preview data.
         * @param format the preview format.
         */
        void onPreviewCallback(byte[] data, int format);
    }

    /**
     * This callback is used for notify camera is opened,you can use it for get parameters..
     */
    interface PreviewSizeCallback {
        /**
         * When camera is opened will be called.
         * @param previewSize current preview size.
         */
        void onPreviewSizeReady(Size previewSize);
    }


    /**
     * should update device manager when mode resume, before open camera.
     */
    void queryCameraDeviceManager();


    /**
     * open camera with specified camera id.
     * @param info the camera info which will be opened.
     */
    void openCamera(DeviceInfo info);

    /**
     * update preview surface.
     * @param surfaceObject surface holder instance.
     */
    void updatePreviewSurface(Object surfaceObject);

    /**
     * Set a callback for device.
     * @param callback the device callback.
     */
    void setDeviceCallback(DeviceCallback callback);

    /**
     * For API1 will directly call start preview.
     * For API2 will first create capture session and then set repeating requests.
     */
    void startPreview();

    /**
     * For API1 will directly call stop preview.
     * For API2 will call session's abort captures.
     */
    void stopPreview();

    /**
     * For API1 will directly call takePicture.
     * For API2 will call STILL_CAPTURE capture.
     *
     * @param callback jpeg data callback.
     */
    void takePicture(@Nonnull IDeviceController.CaptureDataCallback callback);

    /**
     * update current GSensor orientation.the value will be 0/90/180/270;
     * @param orientation current GSensor orientation.
     */
    void updateGSensorOrientation(int orientation);
    /**
     * close camera.
     * @param sync whether need sync call.
     */
    void closeCamera(boolean sync);

    /**
     * abort capture and close session;
     */
    void closeSession();

    /**
     * Get the preview size with target ratio.
     * @param targetRatio current ratio.
     * @return current preview size.
     */
    Size getPreviewSize(double targetRatio);

    /**
     * Set a camera opened callback.
     * @param callback camera opened callback.
     */
    void setPreviewSizeReadyCallback(PreviewSizeCallback callback);

    /**
     * Set the new picture size.
     * @param size current picture size.
     */
    void setPictureSize(Size size);

    /**
     * Check whether can take picture or not.
     * @return true means can take picture; otherwise can not take picture.
     */
    boolean isReadyForCapture();

    /**
     * When don't need the device controller need destroy the device controller.
     * such as handler.
     */
    void destroyDeviceController();

    /**
     * Set ZSD status
     */
    void setZSDStatus(String value);

    /**
     * Set image format
     */
    void setFormat(String value);

    void setSavedDataCallback(CaptureImageSavedCallback callback);
}