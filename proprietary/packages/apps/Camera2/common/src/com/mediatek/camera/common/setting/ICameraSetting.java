/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
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
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.setting;

import android.hardware.Camera;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.view.Surface;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * This interface defines the base operations of setting.
 */
public interface ICameraSetting {

    /**
     * Define the setting type to distinguish setting is used for photo mode
     * or video mode or both.
     */
    enum SettingType {
        PHOTO,
        VIDEO,
        PHOTO_AND_VIDEO,
    }

    /**
     * Define interface to send request to configure setting value
     * when setting value is changed.
     */
    interface ISettingChangeRequester {
        /**
         * Send request when setting value is changed.
         */
        void sendSettingChangeRequest();
    }

    /**
     * Define the API for setting in camera API1.
     */
    interface IParametersConfigure extends ISettingChangeRequester {
        /**
         * Set the original parameters to setting items after open camera in Camera API1.
         *
         * @param originalParameters The {@link Camera.Parameters} object.
         */
        void setOriginalParameters(Camera.Parameters originalParameters);

        /**
         * Configure setting value to camera parameters in camera API1.
         *
         * @param parameters The instance of {@link Camera.Parameters} for setting to configure
         *                   its value in it.
         * @return If setting want to restart preview, return true, otherwise, return false.
         *
         */
        boolean configParameters(Camera.Parameters parameters);

        /**
         * Setting send command to camera device.
         *
         * @param cameraProxy The instance of {@link CameraProxy}.
         */
        void configCommand(CameraProxy cameraProxy);
    }

    /**
     * Preview state callback.
     */
    interface PreviewStateCallback {
        /**
         * When preview stopped, this method will be invoked.
         */
        void onPreviewStopped();

        /**
         * When preview started, this method will be invoked.
         */
        void onPreviewStarted();
    }

    /**
     * Define the API for setting in camera API2.
     */
    interface ICaptureRequestConfigure extends ISettingChangeRequester {
        /**
         * Set the camera characteristics to setting items in Camera API2.
         *
         * @param characteristics The {@link CameraCharacteristics} object.
         */
        void setCameraCharacteristics(CameraCharacteristics characteristics);

        /**
         * Configure setting value to capture request in camera API2.
         *
         * @param captureBuilder The capture builder used to configure value.
         */
        void configCaptureRequest(CaptureRequest.Builder captureBuilder);

        /**
         * Setting may want to configure session. In this method, setting should add its
         * surface to the given list. Mode will configure the surface in the list to camera
         * device.
         *
         * @param surfaces The list of surface.
         */
        void configSessionSurface(List<Surface> surfaces);

        /**
         * Setting may want to capture a raw image.
         * called by {@link SettingManager.getRawOutputConfiguration}
         *
         * @return surface the raw surface.
         */
        Surface configRawSurface();

        /**
         * Setting should implement {@link android.hardware.camera2.CameraCaptureSession
         * .CaptureCallback} to receive repeating capture callback.
         *
         * @return an implementer of {@link android.hardware.camera2
         *         .CameraCaptureSession.CaptureCallback}.
         */
        CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback();
    }

    /**
     * Initialize setting. This will be called when do open camera.
     *
     * @param app the instance of IApp.
     * @param cameraContext the CameraContext.
     * @param settingController the SettingController.
     */
    void init(IApp app,
              ICameraContext cameraContext,
              SettingController settingController);

    /**
     * Set API1 & API2 setting device requester.
     *
     * @param settingDeviceRequester API1 setting device requester.
     * @param settingDevice2Requester API2 setting device requester.
     */
    void setSettingDeviceRequester(
            @Nonnull ISettingManager.SettingDeviceRequester settingDeviceRequester,
            @Nonnull ISettingManager.SettingDevice2Requester settingDevice2Requester);

    /**
     * Update mode device state.
     * @param newState the new mode device state.
     */
    void updateModeDeviceState(String newState);

    /**
     * Un-initialize setting, this will be called before close camera.
     */
    void unInit();

    /**
     * When the camera which this settings binds to is opened or turn as main camera,
     * this method will be invoked. In this method, setting should add its setting view
     * entry to view tree.
     */
    void addViewEntry();

    /**
     * When the camera which this settings binds to is closed or turn as minor camera,
     * this method will be invoked. In this method, setting should remove its setting view
     * entry from view tree.
     */
    void removeViewEntry();

    /**
     * Refresh setting view entry after restriction posed or mode changed.
     */
    void refreshViewEntry();

    /**
     * The method will be invoked when all the settings is initialized. In this method,
     * setting should post its restriction based on current value.
     */
    void postRestrictionAfterInitialized();

    /**
     * This method will be invoked when enter a new mode.
     *
     * @param modeKey The string to indicate mode.
     * @param modeType The mode type, see
     *                 {@link com.mediatek.camera.common.mode.ICameraMode.ModeType}.
     */
    void onModeOpened(String modeKey, ICameraMode.ModeType modeType);

    /**
     * This method will be invoked when old mode closed.
     *
     * @param modeKey The string to indicate mode.
     */
    void onModeClosed(String modeKey);

    /**
     * Get setting type.
     *
     * @return return the setting type.
     */
    SettingType getSettingType();

    /**
     * Return key of setting, which is identifier of this feature.
     * @return The key of setting.
     */
    String getKey();

    /**
     * Return the current value of setting. Such as "on", "off".
     * @return The current value of setting.
     */
    String getValue();

    /**
     * Return current values that setting can use.
     *
     * @return Current values that setting can use.
     */
    List<String> getEntryValues();

    /**
     * Return the values supported by setting in current platform.
     *
     * @return The values supported by setting.
     */
    List<String> getSupportedPlatformValues();

    /**
     * Get the instance of IParametersConfigure. This used by mode to config Parameters
     *
     * @return an instance of {@Link IParametersConfigure}.
     */
    IParametersConfigure getParametersConfigure();

    /**
     * Get the instance of IParametersConfigure. This used by mode to config capture request.
     *
     * @return an instance of {@Link ICaptureRequestConfigure}.
     */
    ICaptureRequestConfigure getCaptureRequestConfigure();

    /**
     * If ICameraSetting needs to know preview status, must provide an instance to
     * SettingManager.
     * @return an instance of {@link PreviewStateCallback} or null.
     */
    PreviewStateCallback getPreviewStateCallback();

    /**
     * Setting's value may be limited as one value or a few values when other settings'
     * value is changed or mode changed. This method gives the value this setting should
     * be set and values this setting can be selected when other setting's value is changed
     * or mode changed.
     *
     * If the current value and supported values are set as null object. it means clear the
     * before overrides overridden by the changed setting or mode which is indicated by
     * input given key.
     *
     * @param headerKey The key indicates the setting/mode that limits this setting.
     * @param currentValue The value this setting should be set.
     * @param supportValues The values can be selected by this setting.
     */
    void overrideValues(@Nonnull String headerKey, String currentValue,
                        List<String> supportValues);
}
