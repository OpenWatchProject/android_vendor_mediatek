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

package com.mediatek.camera.feature.setting.superdenoise;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.portability.CameraEx;
import com.mediatek.camera.portability.SystemProperties;

import javax.annotation.Nonnull;

/**
 * Super de-noise entry for feature provider.
 */

public class SuperDenoiseEntry extends FeatureEntryBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SuperDenoiseEntry.class.getSimpleName());
    private static final boolean sIsSuperDenoiseSupport =
            SystemProperties.getInt("ro.vendor.mtk_cam_dualdenoise_support", 0) == 1 ? true : false;
    private static final boolean sIsDualCameraSupport =
            SystemProperties.getInt("ro.vendor.mtk_cam_dualzoom_support", 0) == 1 ? true : false;
    private static final boolean sIsDenoiseSupportInOneLoad =
            SystemProperties.getInt("vendor.debug.dualcam.mode", 0) == 2 ? true : false;
    private static final String PHOTO_MODE = "com.mediatek.camera.common.mode.photo.PhotoMode";
    private static final String PROPERTY_KEY_CLIENT_APP_MODE = "vendor.mtk.client.appmode";
    private static final String APP_MODE_NAME_MTK_DUAL_CAMERA = "MtkDualCam";
    /**
     * create an entry.
     *
     * @param context   current activity.
     * @param resources current resources.
     */
    public SuperDenoiseEntry(Context context, Resources resources) {
        super(context, resources);
    }

    @Override
    public boolean isSupport(CameraApi currentCameraApi, Activity activity) {
        return isSuperDenoiseSupport() && !isThirdPartyIntent(activity);
    }


    @Override
    public void updateCurrentModeKey(String currentModeKey) {
        mCurrentModeKey = currentModeKey;
        LogHelper.d(TAG, "[updateCurrentModeKey]" + currentModeKey);
    }

    @Override
    public DeviceUsage updateDeviceUsage(String modeKey,
                                         DeviceUsage originalDeviceUsage) {
        LogHelper.d(TAG, "[updateDeviceUsage]" + modeKey);
        if (isSuperDenoiseSupport() && isSuperDenoiseMode(modeKey)) {
            originalDeviceUsage.updateDeviceType(DeviceUsage.DEVICE_TYPE_STEREO);
        }
        return originalDeviceUsage;
    }

    @Override
    public void notifyBeforeOpenCamera(@Nonnull String cameraId,
                                       @Nonnull CameraApi cameraApi) {
        LogHelper.d(TAG, "[notifyBeforeOpenCamera]" + cameraId);
        if (isSuperDenoiseSupport()) {
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(Integer.valueOf(cameraId), info);
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_BACK
                    && isSuperDenoiseMode(mCurrentModeKey)) {
                LogHelper.i(TAG, "[setProperty] Dual de-noise mode");
                CameraEx.setProperty(PROPERTY_KEY_CLIENT_APP_MODE,
                        APP_MODE_NAME_MTK_DUAL_CAMERA);
            }
        }
    }

    @Override
    public int getStage() {
        return 1;
    }

    /**
     * Get the advance feature name.
     * @return Feature name.
     */
    @Override
    public String getFeatureEntryName() {
        return SuperDenoiseEntry.class.getName();
    }

    /**
     * Get feature name, the type is defined as a Class.
     * It is a setting, the type is ICameraSetting.class.
     * @return Feature type.
     */
    @Override
    public Class getType() {
        return ICameraSetting.class;
    }

    /**
     * Create the super de-noise instance.
     * @return Return The instance.
     */
    @Override
    public Object createInstance() {
        return new SuperDenoise();
    }

    private boolean isSuperDenoiseMode(String modeKey) {
        if (modeKey == null) {
            return false;
        }
        return modeKey.equals(PHOTO_MODE);
    }

    private boolean isSuperDenoiseSupport() {
        LogHelper.d(TAG, "isSuperDenoiseSupport: " + sIsSuperDenoiseSupport
                + ", isDualCameraSupport: " + sIsDualCameraSupport);
        if (sIsSuperDenoiseSupport && sIsDualCameraSupport) {
            LogHelper.d(TAG, "isDenoiseSupportInOneLoad: " + sIsDenoiseSupportInOneLoad);
            return sIsDenoiseSupportInOneLoad;
        }
        return sIsSuperDenoiseSupport;
    }
}
