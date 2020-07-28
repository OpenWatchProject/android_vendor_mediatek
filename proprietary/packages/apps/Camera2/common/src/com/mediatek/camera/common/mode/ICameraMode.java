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

package com.mediatek.camera.common.mode;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.relation.DataStore;

import java.util.ArrayList;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * An interface used to define advanced camera feature.
 * Such as PIP,Panorama,NormalPhoto and NormalVideo, etc.
 */
public interface ICameraMode {
    String MODE_DEVICE_STATE_UNKNOWN = "unknown";
    String MODE_DEVICE_STATE_OPENED = "opened";
    String MODE_DEVICE_STATE_CLOSED = "closed";
    String MODE_DEVICE_STATE_PREVIEWING = "previewing";
    String MODE_DEVICE_STATE_CAPTURING = "capturing";
    String MODE_DEVICE_STATE_RECORDING = "recording";

    /**
     * Define the mode type to distinguish mode is photo mode
     * or video mode.
     */
    enum ModeType {
        PHOTO,
        VIDEO,
    }

    /**
     * Initialize the mode,some register things must be done.
     * 1.register shutter button listener with high priority.
     * 2.register gesture event listener with high priority.
     *
     * @param app the instance of IApp.
     * @param cameraContext the instance of ICameraContext.
     * @param isFromLaunch true means is from launch;otherwise is false;
     */
    void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext, boolean isFromLaunch);

    /**
     * Resume the mode.
     * @param deviceUsage current mode devise usage.
     */
    void resume(@Nonnull DeviceUsage deviceUsage);

    /**
     * Pause the mode, if no need to close camera, preview must be stopped.
     *
     * @param nextModeDeviceUsage will create new mode's device usage. if this parameters is null,
     *                           means just leave current mode,such as leave camera activity.
     */
    void pause(@Nullable DeviceUsage nextModeDeviceUsage);

    /**
     * Un-initialize the mode, some unregister things must be done.
     * 1.unregister shutter button listener.
     * 2.unregister gesture event listener.
     */
    void unInit();

    /**
     * Notify mode the camera id is clicked, want to trigger switch camera.
     * @param newCameraId the new camera id.
     * @return true if can do switch camera, false can't do switch camera.
     */
    boolean onCameraSelected(@Nonnull String newCameraId);

    /**
     * Get the mode key.
     * @return the mode key.
     */
    String getModeKey();

    /**
     * Get current mode want to use which camera api.
     * @return the camera api.
     */
    CameraApi getCameraApi();

    /**
     * Get the mode's device usage, this used to judge whether need restart camera device.
     *
     * @param dataStore dataStore used to query current camera id.
     * @param oldDeviceUsage old mode's device usage.
     * @return an instance of {@link DeviceUsage}
     */
    DeviceUsage getDeviceUsage(@Nonnull DataStore dataStore, DeviceUsage oldDeviceUsage);

    /**
     * Notify that the user has interacted with the device.
     * @return True if the event is consumed.
     */
    boolean onUserInteraction();

    /**
     * Check the mode is idle or not.
     * @return true if mode is idle, false or not.
     */
    boolean isModeIdle();
}
