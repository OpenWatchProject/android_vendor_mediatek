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
package com.mediatek.camera.common.mode.video.device;

import android.hardware.Camera.CameraInfo;
import android.media.CamcorderProfile;
import android.view.Surface;

import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.utils.Size;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * The interface for video to use camera device.
 * this interface will implement by api1 and api2 to
 * handle hal1 and hal3 flow.
 */
public interface IDeviceController {
    /**
     * This interface is used to callback jpeg data to mode.
     */
    interface JpegCallback {
        /**
         * Notify jpeg data is generated.
         * @param jpegData jpeg data.
         */
        void onDataReceived(byte[] jpegData);
    }
    /**
     * This callback is used for preview.
     */
    interface PreviewCallback {
        /**
         * When preview data is received,will fired this function.
         * @param data the preview data.
         * @param format the preview format.
         * @param previewCameraId the preview cameraId.
         */
        void onPreviewCallback(byte[] data, int format, String previewCameraId);
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
         * Notified call stop preview immediately.
         */
        void afterStopPreview();
        /**
         * Notified before do close camera.
         */
        void beforeCloseCamera();
        /**
         * preview is start.
         */
        void onPreviewStart();
        /**
         * onError for device.
         */
        void onError();
    }
    /**
     * Notify setting is config done.
     */
    interface SettingConfigCallback {
        /**
         * Notify setting is config done and notify preview size.
         * @param previewSize preview size
         */
        void onConfig(Size previewSize);
    }

    /**
     * used for mode to post restriction.
     *
     * @param relations the post restrictions
     * @param isNeedConfigRequest whether need config request immediately
     */
    void postRecordingRestriction(List<Relation> relations, boolean isNeedConfigRequest);

    /**
     * Used for device to get restriction and post it to settings.
     */
    interface RestrictionProvider {
        /**
         * Device will use this function to get current.
         * restriction and post it to settings in preview.
         * @return current restriction
         */
        Relation getRestriction();

    }
    /**
     * Set a callback for preview.
     * @param callback1 the preview call back the use tell mode preview frame is arrived.
     * @param callback2 the preview start call back,this call after start preview.
     */
    void setPreviewCallback(PreviewCallback callback1, DeviceCallback callback2);

    /**
     * set a call back for setting config.
     * @param callback the call back.
     */
    void setSettingConfigCallback(SettingConfigCallback callback);


    /**
     * should update device manager when mode resume, before open camera.
     */
    void queryCameraDeviceManager();


    /**
     * open camera with specified camera id.
     *
     * @param settingManager the instance of SettingManager.
     * @param cameraId camera id.
     * @param sync whether need sync call.
     * @param relation the restriction
     */
    void openCamera(@Nonnull ISettingManager settingManager,
                    @Nonnull String cameraId,
                    boolean sync,
                    RestrictionProvider relation);
    /**
     * update preview surface.
     * @param surfaceObject surface holder instance.
     */
    void updatePreviewSurface(Object surfaceObject);

    /**
     * For API1 will directly call stop preview.
     * For API2 will call session's abort captures.
     */
    void stopPreview();

    /**
     * For API1 will directly call start preview.
     * For API2 will call config session.
     */
    void startPreview();
    /**
     * For API1 will directly call takePicture.
     * For API2 will call STILL_CAPTURE capture.
     *
     * @param callback jpeg data callback.
     */
    void takePicture(@Nonnull JpegCallback callback);

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
     * lock camera.
     */
    void lockCamera();

    /**
     * unlock camera.
     */
    void unLockCamera();
    /**
     * Used for start recording.
     * and feature can do some thing by it implement.
     */
    void startRecording();

    /**
     * Used for stop recording.
     * and feature can do some thing by it implement.
     */
    void stopRecording();

    /**
     * For api1 maybe new feature will use camera device.
     * @return Camera device.
     */
    CameraProxy getCamera();

    /**
     * config Camera used for set some thing to camera.
     * and feature can do some thing by it implement.
     * @param surface surface for capture.
     * @param isNeedWaitConfigSession whether need wait done config session.
     */
    void configCamera(Surface surface, boolean isNeedWaitConfigSession);

    /**
     * get camera info for video use this to get orientation or.
     * other info
     * @param cameraId the camera id.
     * @return CameraInfo the camera info.
     */
    CameraInfo getCameraInfo(int cameraId);

    /**
     * Charge whether the current camera id support vss.
     * @param cameraId current camera id
     * @return the result
     */
    boolean isVssSupported(int cameraId);

    /**
     * Get current camcorder profile.
     * @return the result
     */
    CamcorderProfile getCamcorderProfile();

    /**
     * Used to release device.
     */
    void release();

    /**
     * prevent settings changed until new state come.
     */
    void preventChangeSettings();

    /**
     * Check whether can take picture or not.
     * @return true means can take picture; otherwise can not take picture.
     */
    boolean isReadyForCapture();
}
