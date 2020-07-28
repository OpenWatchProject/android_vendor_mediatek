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
package com.mediatek.camera.common.loader;

import android.app.Activity;
import android.hardware.Camera.Parameters;

import com.mediatek.camera.common.IAppUi.ModeItem;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.DeviceUsage;

import javax.annotation.Nonnull;

/**
 * Used to describe a advance feature, it maybe a mode or setting.
 * Advance feature is compile independently, it will exit as a JAR or APk, and need
 * plugin.xml has static info for it.
 * When loading the advance feature, loader will find the feature's plugin.xml and find
 * it's FeatureEntry implementer and create the entry instance.
 * Finally, the entry instance will be used to judge the feature is support or not, get
 * feature info and create the feature instance.
 */
public interface IFeatureEntry {

    /**
     * Judge the advance feature is support or not in current platform.
     * @return True is support otherwise false.
     * @param currentCameraApi current camera api type.
     * @param activity the camera activity.
     */
    boolean isSupport(CameraApi currentCameraApi, Activity activity);

    /**
     * Update current mode key to feature entry, dual camera zoom need to set properties
     * in photo and video mode before open camera.
     * @param currentModeKey current mode key.
     */
    void updateCurrentModeKey(String currentModeKey);

    /**
     * Update device usage with mode key, dual camera zoom may update normal
     * photo and video mode's device usage.
     * @param modeKey this device's mode key.
     * @param originalDeviceUsage the original device usage.
     * @return the updated device usage.
     */
    DeviceUsage updateDeviceUsage(String modeKey, DeviceUsage originalDeviceUsage);

    /**
     * Set device spec, this function should be called firstly after construct
     * feature entry.
     *
     * @param deviceSpec the device spec.
     */
    void setDeviceSpec(@Nonnull DeviceSpec deviceSpec);

    /**
     * Notify feature entry before open camera, this event only need to notify setting feature.
     * @param cameraId want to open which camera.
     * @param cameraApi use which api.
     */
    void notifyBeforeOpenCamera(@Nonnull String cameraId, @Nonnull CameraApi cameraApi);

    /**
     * Update parameters.
     *
     * @param cameraId update which camera id's parameters.
     * @param originalParameters the camera id's parameters.
     */
    void setParameters(@Nonnull String cameraId, @Nonnull Parameters originalParameters);

    /**
     * Get the advance feature name.
     * @return Feature name.
     */
    String getFeatureEntryName();

    /**
     * Get feature name, the type is define as a Class.
     * For example,
     * if the feature is a mode ,it will implement the ICameraMode, so the
     * feature type will  be the ICameraMode.class.
     * if it is a setting, the type will be the ICameraSetting.class.
     * @return Feature type.
     */
    Class getType();

    /**
     * Get the stage of this feature.
     *
     * @return The stage of this feature.
     */
    int getStage();

    /**
     * Create the feature instance.
     * @return Return The ICameraMode or ICameraSetting implementer instance.
     */
    Object createInstance();

    /**
     * Get mode item.
     * @return the mode item info.
     */
    ModeItem getModeItem();
}
