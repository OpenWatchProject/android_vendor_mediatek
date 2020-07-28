/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common;

import android.app.Activity;
import android.location.Location;

import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.FeatureProvider;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.SettingManagerFactory;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.storage.IStorageService;
import com.mediatek.camera.common.storage.MediaSaver;

import javax.annotation.Nonnull;

/**
 * The camera context which provides single instance services.
 */
public interface ICameraContext {

    /**
     * This will be called when activity onCreate occurs.
     *
     * @param app the instance of IApp.
     * @param activity the camera activity.
     */
    void create(@Nonnull IApp app, @Nonnull Activity activity);

    /**
     * This will be called when activity onResume occurs.
     */
    void resume();

    /**
     * This will be called when activity onPause occurs.
     */
    void pause();

    /**
     * This will be called when activity onDestroy occurs.
     */
    void destroy();
    /**
     * Get the instance of DataStore,return the same instance
     * during the life cycle of Context.
     *
     * @return an instance of DataStore.
     */

    DataStore getDataStore();
    /**
     * Get the instance of storage service.
     * @return an instance of storage service.
     */
    IStorageService getStorageService();

    /**
     * Get an instance of SettingManagerFactory.
     * One Context will get the same instance.
     * @return an instance of SettingManagerFactory.
     */
    SettingManagerFactory getSettingManagerFactory();

    /**
     * Get an instance of CameraDeviceManager by api type..
     * @param api the wanted camera api type.
     * @return an instance of CameraDeviceManager.
     */
    CameraDeviceManager getDeviceManager(@Nonnull CameraApi api);

    /**
     * Get the instance of MediaSaver.
     * @return an instance of MediaSaver.
     */
    MediaSaver getMediaSaver();

    /**
     * Get feature provider instance.
     *
     * @return an instance of feature provider.
     */
    FeatureProvider getFeatureProvider();

    /**
     * Get a status monitor object bound to the input camera id.
     *
     * @param cameraId The id of camera.
     * @return A status monitor object bound to the input camera id.
     */
    StatusMonitor getStatusMonitor(String cameraId);

    /**
     * Get sound play back to play sound.
     *
     * @return an instance of {@link ISoundPlayback}.
     */
    ISoundPlayback getSoundPlayback();

    /**
     * get the current location info.
     * @return the location info. return null if there is no location info.
     */
    Location getLocation();
}