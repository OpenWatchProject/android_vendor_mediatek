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

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Process;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.loader.FeatureLoader;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.portability.SystemProperties;

/**
 * Disable camera-related activities if there is no camera. This
 * receiver runs when BOOT_COMPLETED intent is received. After running once
 * this receiver will be disabled, so it will not run again.
 */
public class DisableCameraReceiver extends BroadcastReceiver {
    private static final Tag TAG = new Tag(DisableCameraReceiver.class.getSimpleName());
    private static final int DEFAULT_APP_VERSION = 2;
    private static final String ACTIVITIES[] = {
            "com.mediatek.camera.CameraLauncher",
            "com.mediatek.camera.VideoCamera",
            "com.mediatek.camera.CameraActivity",
            "com.mediatek.camera.SecureCameraActivity",
            "com.mediatek.camera.CaptureActivity"};

    @Override
    public void onReceive(final Context context, Intent intent) {
        // Disable camera-related activities if there is no camera.
        boolean needCameraActivity = hasCamera();
        int appVersion = SystemProperties.getInt("ro.vendor.mtk_camera_app_version",
                DEFAULT_APP_VERSION);
        LogHelper.i(TAG, "<DisableCameraReceiver> appVersion = " + appVersion);

        if (appVersion < DEFAULT_APP_VERSION || !needCameraActivity) {
            LogHelper.i(TAG, "camera2 repo: disable all camera activities");
            for (int i = 0; i < ACTIVITIES.length; i++) {
                disableComponent(context, ACTIVITIES[i]);
            }
        } else {
                new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... voids) {
                    CameraApiHelper.getCameraApiType(null);
                    CameraApiHelper.getDeviceSpec(context);
                    FeatureLoader.loadBuildInFeatures(context);
                    FeatureLoader.loadPluginFeatures(context);
                    return null;
                }
            } .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        }
        if(isLowRam()) {
            LogHelper.i(TAG,"kill process for low ram project");
            Process.killProcess(Process.myPid());
        }
        /*else {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && !isGmoRam()) {
                Intent appServiceIntent = new Intent(context, CameraAppService.class);
                context.startService(appServiceIntent);
            }
        }*/
    }

    private boolean hasCamera() {
        int n = Camera.getNumberOfCameras();
        LogHelper.i(TAG, "number of camera: " + n);
        return (n > 0);
    }

    private boolean hasBackCamera() {
        try {
            int n = Camera.getNumberOfCameras();
            CameraInfo info = new CameraInfo();
            for (int i = 0; i < n; i++) {
                Camera.getCameraInfo(i, info);
                if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
                    LogHelper.i(TAG, "back camera found: " + i);
                    return true;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            LogHelper.e(TAG, "camera process killed due to getCameraInfo() error");
            Process.killProcess(Process.myPid());
        }
        LogHelper.i(TAG, "no back camera");
        return false;
    }

    private boolean isLowRam() {
        boolean enable = "true".equals(SystemProperties.getString("ro.config.low_ram","false"));
        LogHelper.i(TAG, "[isLowRam]" + enable);
        return enable;
    }

    private boolean isGmoRam() {
        boolean enable = SystemProperties.getInt("ro.vendor.gmo.ram_optimize", 0) ==
            1 ? true : false;
        LogHelper.i(TAG, "[isGmoRam]" + enable);
        return enable;
    }

    private void disableComponent(Context context, String klass) {
        ComponentName name = new ComponentName(context, klass);
        PackageManager pm = context.getPackageManager();

        // We need the DONT_KILL_APP flag, otherwise we will be killed
        // immediately because we are in the same app.
        pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP);
    }
}
