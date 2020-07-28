/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 *     the prior written permission of MediaTek inc. and/or its licensor, any
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
 *     NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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

import android.content.Context;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.os.Build;
import android.os.Process;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.DeviceSpec;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.SystemProperties;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nullable;

/**
 * <p>This class is used to query debug properties set by user or ProjectConfig.mk.
 * these debug properties includes:</p>
 * <li>Camera application choose api1 or api2.</li>
 * <li>//TODO add more properties</li>
 * <p>Note:After modifying the debugging property, you need to kill and restart
 * mediatek.camera process to take effect.</p>
 */
public class CameraApiHelper {
    private static final Tag TAG = new Tag(CameraApiHelper.class.getSimpleName());
    private static final int API_UNKNOWN = 0;
    private static DeviceSpec sDeviceSpec = new DeviceSpec();
    private static Boolean sForceCreate = false;

    /**
     * Get Camera API type by mode class name.
     *
     * @param modeName modeName to get camera api type.
     * @return the camera api type.
     */
    public static CameraApi getCameraApiType(@Nullable String modeName) {
    	return CameraApi.API2;
    }

    /**
     * Get device spec.
     * @param context the context to access camera manager.
     * @return the device spec.
     */
    public static DeviceSpec getDeviceSpec(Context context) {
        createDeviceSpec(context);
        return sDeviceSpec;
    }

    /**
     * force CreateDeviceSpec.
     * @param force true to force CreateDeviceSpec no matter it was created or not.
     */
    public static void setForceCreateDeviceSpec(boolean force) {
        sForceCreate = force;
    }

    @SuppressWarnings("deprecation")
    private static void createDeviceSpec(Context context) {
        if (!sForceCreate && sDeviceSpec.getDefaultCameraApi() != null) {
            return;
        }
        LogHelper.i(TAG, "[createDeviceSpec]+");
        CameraApi defaultCameraApi = getCameraApiType(null);
        int cameraNum = 0;
        String[] idList = null;
        try {
            idList = CameraUtil.getCameraManager(context).getCameraIdList();
        } catch (Exception e) {
            e.printStackTrace();
            LogHelper.e(TAG, "[createDeviceSpec] camera id list null");
        }
        if (idList == null || idList.length == 0) {
            LogHelper.e(TAG, "camera process killed due to camera num is 0," +
                    " mostly camerahalserver happened some error" +
                    " and still in progress of relaunch");
            Process.killProcess(Process.myPid());
        }
        cameraNum = idList.length;
        ConcurrentHashMap<String, DeviceDescription>
                deviceDescriptionMap = new ConcurrentHashMap<>();
        if (cameraNum > 0) {
            for (String id : idList) {
                DeviceDescription deviceDescription = new DeviceDescription(null);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                    CameraCharacteristics characteristics = CameraUtil.getCharatics(context, id);
                    if (characteristics != null) {
                        deviceDescription.setCameraCharacteristics(characteristics);
                        deviceDescription.storeCameraCharacKeys(characteristics);
                        CameraUtil.initLogicalCameraId(deviceDescription, id);
                    }
                }
                deviceDescriptionMap.put(id, deviceDescription);
            }
            sDeviceSpec.setDefaultCameraApi(defaultCameraApi);
            sDeviceSpec.setDeviceDescriptions(deviceDescriptionMap);
        }
        LogHelper.i(TAG, "[createDeviceSpec]- context: " + context + ", default api:"
                + defaultCameraApi + ", deviceDescriptionMap:"
                + deviceDescriptionMap + " cameraNum " + cameraNum);
    }
}