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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.v2.platform.device;

import com.android.camera.v2.bridge.device.CameraDeviceManagerImpl;
import com.mediatek.camera.v2.platform.device.CameraDeviceProxy.CameraSessionCallback;

import android.app.Activity;
import android.content.Context;
import android.hardware.camera2.CameraManager;
import android.os.Handler;

/**
 * The camera device manager is responsible for instantiating {@link CameraDeviceProxy}
 * instances.
 */
public abstract class CameraDeviceManager {
    /**
     * A callback objects for receiving updates about the state of a camera device.
     *
     */
    public static abstract class CameraStateCallback {
        /**
         * The method called when a camera device has finished opening.
         * @param camera The camera device that has become opened.
         */
        public abstract void onOpened(CameraDeviceProxy camera);
        /**
         * The method called when a camera device has encountered a serious error.
         * @param error The error code.
         */
        public abstract void onError(int error);
        /**
         * The method called when a camera device is no longer available for
         * use.
         * @param camera The device that has been disconnected.
         */
        public void onDisconnected(CameraDeviceProxy camera){}
    }

    public static CameraDeviceManager get(Activity activity) {
        return create(activity);
    }
    /**
     * Get a new camera device manager instance.
     * @param activity The Context the view is running in.
     * @return New camera device manager.
     */
    public static CameraDeviceManager getNew(Activity activity) {
        CameraManager cameraManager = new CameraManager(activity);
        return new CameraDeviceManagerImpl(cameraManager, activity);
    }
    public abstract void open(
            String cameraId,
            CameraStateCallback stateCallback,
            CameraSessionCallback sessionCallback,
            Handler handler);

    public abstract void openSync(String cameraId,
            CameraStateCallback stateCallback,
            CameraSessionCallback sessionCallback,
            Handler handler);

    /**
     * Get the currently connected camera.
     * @return Connected camera list.
     */
    public abstract String[] getCameraIdList();

    /**
     * Creates a new camera manager that is based on Camera2 API, if available,
     * or otherwise uses the portability API.
     */
    private static CameraDeviceManager create(Activity activity) {
        CameraManager cameraManager = (CameraManager) activity
                .getSystemService(Context.CAMERA_SERVICE);
        return new CameraDeviceManagerImpl(cameraManager, activity);
    }
}
