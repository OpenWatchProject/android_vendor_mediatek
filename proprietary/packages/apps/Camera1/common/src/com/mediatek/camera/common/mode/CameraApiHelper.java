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
import android.hardware.camera2.CameraManager;
import android.os.Build;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.DeviceSpec;
import com.mediatek.camera.portability.SystemProperties;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
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
    private static final int API_NOT_SPECIFIED = -1;
    private static final int API_UNKNOWN = 0;
    private static final int API1 = 1;
    private static final int API2 = 2;
    private static final String API_VERSION_XML_PATH =
            "/data/local/tmp/mtk_camera_app_api_version.xml";
    private static final int sApiByUserCommand =
            SystemProperties.getInt("vendor.mtk.camera.app.api.version", API_UNKNOWN);
    private static final int sApiByProject =
            SystemProperties.getInt("ro.vendor.mtk_camera_app_api_version", API_UNKNOWN);
    private static int sApiByUserXml = API_NOT_SPECIFIED;
    private static DeviceSpec sDeviceSpec = new DeviceSpec();

    /**
     * Get Camera API type by mode class name.
     *
     * @param modeName modeName to get camera api type.
     * @return the camera api type.
     */
    public static CameraApi getCameraApiType(@Nullable String modeName) {
        if (isApi2Enable()) {
            return CameraApi.API2;
        }
        return CameraApi.API1;
    }

    /**
     * Check whether api2 is enable.
     *
     * @return true use api2, false use api1.
     */
    private static boolean isApi2Enable() {
        parseApiFromXml();
        // if user specified api, use it
        if (sApiByUserXml > API_UNKNOWN || sApiByUserCommand > API_UNKNOWN) {
            return sApiByUserXml == API2 || sApiByUserCommand == API2;
        }
        // if ProjectConfig.mk specified api, use it
        if (sApiByProject > API_UNKNOWN) {
            return sApiByProject == API2;
        }
        // camera2 repo use API2 as default API.
        return false;
    }

    private static synchronized void parseApiFromXml() {
        if (API_NOT_SPECIFIED == sApiByUserXml) {
            LogHelper.d(TAG, "[parseApiFromXml]+");
            File file = new File(API_VERSION_XML_PATH);
            if (file != null && file.exists()) {
                FileInputStream inputStream;
                try {
                    inputStream = new FileInputStream(API_VERSION_XML_PATH);
                } catch (FileNotFoundException e) {
                    LogHelper.w(TAG, "parseApiFromXml with FileNotFoundException.");
                    e.printStackTrace();
                    inputStream = null;
                }
                sApiByUserXml = readApiFromXml(inputStream);
            } else {
                sApiByUserXml = API_UNKNOWN;
            }
            LogHelper.d(TAG, "[parseApiFromXml]-");
        }
    }

    //TODO When an advanced feature can be set to go API1 or API2 in the future,
    // we need to refactor this code.
    private static int readApiFromXml(InputStream inputStream) {
        int api = API_UNKNOWN;
        if (inputStream != null) {
            try {
                XmlPullParserFactory pullFactory = XmlPullParserFactory.newInstance();
                XmlPullParser parser = pullFactory.newPullParser();
                parser.setInput(inputStream, "UTF-8");
                int eventType = parser.getEventType();
                while (eventType != XmlPullParser.END_DOCUMENT) {
                    switch (eventType) {
                        case XmlPullParser.START_TAG:
                            String tagName = parser.getName();
                            if ("api".equals(tagName)) {
                                api = Integer.valueOf(parser.nextText());
                            }
                            break;
                        default:
                            break;
                    }
                    eventType = parser.next();
                }
            } catch (XmlPullParserException e) {
                LogHelper.w(TAG, "readApiFromXml with XmlPullParserException.");
                e.printStackTrace();
            } catch (IOException e) {
                LogHelper.w(TAG, "readApiFromXml with IOException.");
                e.printStackTrace();
            }
        }
        LogHelper.i(TAG, "readApiFromXml api:" + api);
        return api;
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

    @SuppressWarnings("deprecation")
    private static void createDeviceSpec(Context context) {
        if (sDeviceSpec.getDefaultCameraApi() != null) {
            return;
        }
        CameraApi defaultCameraApi = getCameraApiType(null);
        int cameraNum = Camera.getNumberOfCameras();
        ConcurrentHashMap<String, DeviceDescription>
                deviceDescriptionMap = new ConcurrentHashMap<>();
        if (cameraNum > 0) {
            for (int i = 0; i < cameraNum; i++) {
                Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
                Camera.getCameraInfo(i, cameraInfo);
                DeviceDescription deviceDescription = new DeviceDescription(cameraInfo);
                deviceDescriptionMap.put(String.valueOf(i), deviceDescription);
            }
            sDeviceSpec.setDefaultCameraApi(defaultCameraApi);
            sDeviceSpec.setDeviceDescriptions(deviceDescriptionMap);
        }

        LogHelper.i(TAG, "[createDeviceSpec] context: " + context + ", default api:"
                + defaultCameraApi + ", deviceDescriptionMap:"
                + deviceDescriptionMap + " cameraNum " + cameraNum);
    }
}
