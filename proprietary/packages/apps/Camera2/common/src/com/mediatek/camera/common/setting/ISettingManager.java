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
import android.hardware.Camera.Parameters;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.params.OutputConfiguration;
import android.view.Surface;

import com.mediatek.camera.common.bgservice.CaptureSurface;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * A manager used to manage Setting features,it provides:
 * 1.SettingConfig, used by mode to config setting parameters.
 * 2.SettingController, used by mode and setting.
 * 3.SettingRegistry, used by FeatureLoader to register/unregister setting instance.
 */
@SuppressWarnings("deprecation")
public interface ISettingManager {
    /**
     * This interface is used by mode and setting.
     */
    interface SettingController {
        /**
         * Get the camera id that the setting binds to.
         *
         * @return The camera id that the settin binds to.
         */
        String getCameraId();
        /**
         * Query the current value of specified setting.
         *
         * @param key The string used to indicate setting.
         * @return The current value of specified setting.
         *
         */
        String queryValue(String key);
        /**
         * Query the platform supported values of specified setting.
         *
         * @param key The string used to indicate setting.
         * @return The supported values of specified setting.
         *
         */
        List<String> querySupportedPlatformValues(String key);
        /**
         * Receive restriction from features and transfer it to restriction dispatcher.
         *
         * @param relation The object to carry restriction info.
         */
        void postRestriction(Relation relation);

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
         * Refresh setting view entry after relation executed.
         */
        void refreshViewEntry();

        /**
         * Get an instance of {@link StatusMonitor}, used by mode and settings.
         * @return an instance of StatusMonitor.
         */
        StatusMonitor getStatusMonitor();

        /**
         * Register a setting item to setting manager.
         *
         * @param setting The setting item that register to setting manager.
         */
        void registerSettingItem(ICameraSetting setting);

        /**
         * unRegister a setting item to setting manager.
         *
         * @param setting The setting item that unregister to setting manager.
         */
        void unRegisterSettingItem(ICameraSetting setting);
    }

    /**
     * This interface is used by mode to config API1 parameters or command.
     */
    interface SettingDeviceConfigurator {
        /**
         * Set the original parameters to setting items after open camera in Camera API1.
         *
         * @param originalParameters The {@link Parameters} object.
         */
        void setOriginalParameters(Parameters originalParameters);

        /**
         * Configure setting value to camera parameters in camera API1.
         * @param parameters The instance of {@link Camera.Parameters} for setting to configure
         * its value in it.
         * @return whether need restart preview when change this parameters.
         */
        boolean configParameters(Camera.Parameters parameters);

        /**
         * Just configure one setting parameters that indicated by the input key.
         *
         * @param parameters The instance of {@link Camera.Parameters} for setting to configure
         *                   its value in it.
         * @param key The string indicate the setting going to configure.
         * @return whether need restart preview when change this parameters.
         */
        boolean configParametersByKey(Camera.Parameters parameters, String key);

        /**
         * Configure command by CameraProxy.
         *
         * @param key send command to which ICameraSetting.
         * @param cameraProxy The instance of {@link CameraProxy} for setting to send
         * command to camera framework.
         */
        void configCommand(String key, CameraProxy cameraProxy);

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
     *  This interface is used by mode to config API2 related.
     */
    interface SettingDevice2Configurator {
        /**
         * Set the camera characteristics to setting items in Camera API2.
         *
         * @param characteristics The {@link CameraCharacteristics} object.
         */
        void setCameraCharacteristics(CameraCharacteristics characteristics);

        /**
         * Config setting's value to {@link Builder}.
         *
         * @param builder the builder to be configured.
         */
        void configCaptureRequest(@Nonnull Builder builder);

        /**
         * Config setting's surface to session.
         *
         * @param surfaceList the list used to hold surfaces.
         */
        void configSessionSurface(@Nonnull List<Surface> surfaceList);

        /**
         * StettingManager implement for mode to get raw OutputConfiguration.
         * @return the raw OutputConfiguration.
         */
        OutputConfiguration getRawOutputConfiguration();

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
     * This interface is used by setting to send request to API1 device.
     */
    interface SettingDeviceRequester {
        /**
         * Request to change Parameters/CaptureRequest by setting.
         *
         * @param key The key of setting.
         */
        void requestChangeSettingValue(String key);

        /**
         * Request to send command by setting. If parameters is being configured currently,
         * this command request will be handled after parameters configuration is finished.
         *
         * @param key The key of setting.
         */
        void requestChangeCommand(String key);

        /**
         * Request to change Parameters and just configure it by setting.
         *
         * @param key The key of setting.
         */
        void requestChangeSettingValueJustSelf(String key);


        /**
         * Request to send command by setting and be handled immediately.
         *
         * @param key The key of setting.
         */
        void requestChangeCommandImmediately(String key);
    }

    /**
     * This interface is used by setting to send request to API2 device.
     */
    interface SettingDevice2Requester {
        /**
         * Request create and change repeating capture request value.
         * After config mode will set new repeating request to session.
         */
        void createAndChangeRepeatingRequest();

        /**
         * Request create and configure specified type request value.
         *
         * @param templateType specified capture request type.
         * @return an {@link Builder} instance after configured.
         */
        Builder createAndConfigRequest(int templateType);

        /**
         * get the capture surface that mode used to config session.
         * It used to share the capture surface with mode that configed.
         * It usually is used for normal mode, if your feature used in other modes, you should
         * check if the surface can be shared.
         * @return the capture surface.
         */
        CaptureSurface getModeSharedCaptureSurface() throws IllegalStateException;

        Surface getModeSharedPreviewSurface() throws IllegalStateException;

        Surface getModeSharedThumbnailSurface() throws IllegalStateException;

        /**
         * Get current alive capture session, setting will use this session to
         * do it's own capture.
         *
         * @return the capture session.
         */
        Camera2CaptureSessionProxy getCurrentCaptureSession();

        /**
         * Request restart capture session, mode will abort current capture session,
         * and create a new one.
         */
        void requestRestartSession();

        /**
         * Get current repeating template type.
         *
         * @return the repeating template type.
         */
        int getRepeatingTemplateType();
    }

    /**
     * Create setting item whose stage equal with the input stage.
     *
     * @param stage The input stage.
     */
    void createSettingsByStage(int stage);

    /**
     * Create all the setting items.
     */
    void createAllSettings();

    /**
     * Set mode listener object to setting manager when it is created.
     *
     * @param settingDeviceRequester The instance used to send request to mode.
     */
    void updateModeDeviceRequester(@Nonnull SettingDeviceRequester settingDeviceRequester);

    /**
     * Set mode listener object to setting manager when it is created.
     *
     * @param settingDevice2Requester The instance used to send request to mode.
     */
    void updateModeDevice2Requester(@Nonnull SettingDevice2Requester settingDevice2Requester);

    /**
     * Update mode device state.
     * @param modeName the mode name.
     * @param newState the new mode device state.
     */
    void updateModeDeviceStateToSetting(String modeName, String newState);

    /**
     * Get the instance of {@link SettingDeviceConfigurator} used
     * to do setting configuration by API1.
     *
     * @return the instance of SettingDeviceConfigurator.
     */
    SettingDeviceConfigurator getSettingDeviceConfigurator();

    /**
     * Get the instance of {@link SettingDevice2Configurator} used
     * to do setting configuration by API2.
     *
     * @return the instance of {@link SettingDevice2Configurator}.
     */
    SettingDevice2Configurator getSettingDevice2Configurator();

    /**
     * Get setting controller object.
     *
     * @return The instance of {@link SettingController}.
     */
    SettingController getSettingController();
}