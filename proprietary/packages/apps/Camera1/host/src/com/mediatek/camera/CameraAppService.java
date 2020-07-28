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

package com.mediatek.camera;

import android.app.Activity;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.IBinder;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.debug.profiler.IPerformanceProfile;
import com.mediatek.camera.common.debug.profiler.PerformanceTracker;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.loader.FeatureLoader;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.utils.CameraUtil;

/**
 * Camera app service implementation, which extends {@link CameraPrewarmService},
 * when our min sdk version >= M, we will change to extends
 * android.service.media.CameraPrewarmService.
 */
public class CameraAppService extends CameraPrewarmService {
    private static final Tag TAG = new Tag("PreWarmService");

    @Override
    public void onCreate() {
        super.onCreate();
        LogHelper.i(TAG, "[onCreate]");
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                IPerformanceProfile profile = PerformanceTracker.create(TAG, "Loading").start();
                CameraApiHelper.getCameraApiType(null);
                profile.mark("getCameraApiType");
                CameraApiHelper.getDeviceSpec(getApplicationContext());
                profile.mark("getDeviceSpec");
                FeatureLoader.loadBuildInFeatures(getApplicationContext());
                profile.mark("load build in features");
                FeatureLoader.loadPluginFeatures(getApplicationContext());
                profile.mark("load plugin features");
                CameraUtil.delStereoThumb(CameraAppService.this);
                profile.stop();
                return null;
            }
        } .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogHelper.i(TAG, "[onDestroy]");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        LogHelper.i(TAG, "[onStartCommand]");
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        IBinder binder = super.onBind(intent);
        return binder;
    }

    @Override
    public void onPrewarm() {

    }

    @Override
    public void onCooldown(boolean b) {

    }

    /**
     * This is a hook load to open camera for mtk camera app.
     * @param activity the camera launch activity.
     */
    public static void launchCamera(Activity activity) {
        LogHelper.d(TAG, "[launchCamera]+");
        String defaultMode = "com.mediatek.camera.common.mode.photo.PhotoMode";
        String cameraId = "0";
        CameraDeviceManagerFactory.CameraApi apiType =
                CameraApiHelper.getCameraApiType(null);
        //TODO enable API2, remove this
        if (CameraApi.API2.equals(apiType)) {
            LogHelper.w(TAG, "[launchCamera]- API2 current not support preWarm open camera.");
            return;
        }
        FeatureLoader.updateSettingCurrentModeKey(activity, defaultMode);
        CameraDeviceManagerFactory deviceManagerFactory = CameraDeviceManagerFactory.getInstance();
        try {
            deviceManagerFactory.getCameraDeviceManager(activity, apiType).openCamera(
                    cameraId, null, null);
        } catch (CameraOpenException e) {
            LogHelper.e(TAG, "[launchCamera] e:" + e);
            e.printStackTrace();
        }
        LogHelper.i(TAG, "[launchCamera]- id:" + cameraId + ", api:" + apiType);
    }
    /**
     * Release camera device manager.
     * @param activity the camera launch activity.
     */
    public static void exitCamera(Activity activity) {
        CameraDeviceManagerFactory.CameraApi apiType =
                CameraApiHelper.getCameraApiType(null);
        CameraDeviceManagerFactory deviceManagerFactory = CameraDeviceManagerFactory.getInstance();
        deviceManagerFactory.getCameraDeviceManager(activity, apiType).recycle();
    }

    /**
     * set current activity instance.
     * @param activity current activity.
     */
    public static void setCurrentActivity(Activity activity) {
        CameraDeviceManagerFactory.setCurrentActivity(activity);
    }

    /**
     * Release camera factory.
     * @param activity the release.
     */
    public static void release(Activity activity) {
        CameraDeviceManagerFactory.release(activity);
    }
}
