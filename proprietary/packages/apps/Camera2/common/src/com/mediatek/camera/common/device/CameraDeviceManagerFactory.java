/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
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
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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

package com.mediatek.camera.common.device;

import android.app.Activity;
import android.content.Context;
import android.os.Build;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.v1.CameraDeviceManagerImpl;
import com.mediatek.camera.common.device.v2.Camera2DeviceManagerImpl;

import javax.annotation.Nonnull;

/**
 * A factory class for {@link CameraDeviceManager}.
 */
public class CameraDeviceManagerFactory {
    private static final Tag TAG = new Tag("CamDeviceMgrFac");
    private static final int FIRST_SDK_WITH_API_2 = 21;
    private static CameraDeviceManagerFactory sCameraDeviceManagerFactory;
    private static Activity sCurrentActivity;
    private CameraDeviceManager mCameraDeviceManager;
    private CameraDeviceManager mCamera2DeviceManager;

    /**
     * Used to indicate which camera framework should be used.
     */
    public enum CameraApi {
        /** Use the {@link android.hardware.Camera} class. */
        API1,
        /** Use the {@link android.hardware.camera2} package. */
        API2
    }

    /**
     * Application context singleton.
     */
    private CameraDeviceManagerFactory() {
    }

    /**
     * Create camera device manager factory by activity.
     * @return the instance of device manager factory.
     */
    public static synchronized CameraDeviceManagerFactory getInstance() {
        if (sCameraDeviceManagerFactory == null) {
            sCameraDeviceManagerFactory = new CameraDeviceManagerFactory();
        }
        return sCameraDeviceManagerFactory;
    }

    /**
     * Returns the android camera implementation of
     * {@link com.mediatek.camera.common.device.CameraDeviceManager}.
     *
     * <p>To clean up the resources allocated by this call, be sure to invoke
     * recycle with the same {@code api} value provided
     * here.</p>
     *
     * @param context The application context.
     * @param api Which camera framework to use.
     * @return The {@link CameraDeviceManager} to control the camera device.
     *
     * @throws UnsupportedOperationException If {@code CameraApi.API_2} was
     *                                       requested on an unsupported device.
     */
    @Nonnull
    public synchronized CameraDeviceManager getCameraDeviceManager(Context context, CameraApi api)
                    throws UnsupportedOperationException {
        LogHelper.d(TAG, "getCameraDeviceManager,context = " + context + ",api = " + api);
        checkConditionsBeforeGetManager(context, api);
        switch (api) {
            case API1:
                return getApi1DeviceManager(context);
            case API2:
                return getApi2DeviceManager(context);
            default:
                throw new UnsupportedOperationException("Get camera device manager,the API: " + api
                                + " version don't support");
        }
    }

    /**
     * Because camera process is single instance and this factory is static but
     * camera activity maybe has many because test case will lunch camera by intent.
     * @param activity the current activity.
     */
    public static void setCurrentActivity(Activity activity) {
        sCurrentActivity = activity;
    }

    /**
     * Release camera device manager factory.
     * @param activity the release.
     */
    public static synchronized void release(Activity activity) {
        if (sCurrentActivity != activity) {
            LogHelper.i(TAG, "[release] return for other activity is used");
            return;
        }
        if (sCameraDeviceManagerFactory != null) {
            sCameraDeviceManagerFactory.releaseAllCameraDeviceManagerInstance();
            sCameraDeviceManagerFactory = null;
        }
       sCurrentActivity = null;
    }

    private void releaseAllCameraDeviceManagerInstance() {
        if (mCamera2DeviceManager != null) {
            mCamera2DeviceManager.recycle();
            mCamera2DeviceManager = null;
        }
    }

    private void checkConditionsBeforeGetManager(Context context, CameraApi api) {
        if (api == null) {
            throw new UnsupportedOperationException(
                            "Get camera device manager,API version is not allowed to null");
        }
        if (context == null && CameraApi.API2 == api) {
            throw new UnsupportedOperationException(
                            "Get camera device manager with api 2 must need context");
        }
    }

    private CameraDeviceManager getApi1DeviceManager(Context context) {
        if (mCameraDeviceManager == null) {
            mCameraDeviceManager = new CameraDeviceManagerImpl(context);
        }
        LogHelper.d(TAG, "getApi1DeviceManager: " + mCameraDeviceManager);
        return mCameraDeviceManager;
    }

    private CameraDeviceManager getApi2DeviceManager(Context context) {
        if (CameraApi.API2 == getHighestSupportedApi()) {
            if (mCamera2DeviceManager == null) {
                mCamera2DeviceManager = new Camera2DeviceManagerImpl(context);
            }
            LogHelper.d(TAG, "getApi2DeviceManager: " + mCamera2DeviceManager);
            return mCamera2DeviceManager;
        }
        throw new UnsupportedOperationException(
                        "Get camera device manager,API2 is not supported on this project");
    }

    private CameraApi getHighestSupportedApi() {
        if (Build.VERSION.SDK_INT >= FIRST_SDK_WITH_API_2 || Build.VERSION.CODENAME.equals("L")) {
            return CameraApi.API2;
        } else {
            return CameraApi.API1;
        }
    }
}