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

package com.mediatek.camera.feature.setting.dng;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Build;
import android.util.Size;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.portability.ReflectUtil;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Dng entry for feature provider.
 */

public class DngEntry extends FeatureEntryBase {
    private static final Tag TAG = new Tag(DngEntry.class.getSimpleName());
    private static final int DEFAULT_DNG_STAGE = 1;
    /**
     * create an entry.
     *
     * @param context current activity.
     * @param resources current resources.
     */
    public DngEntry(Context context, Resources resources) {
        super(context, resources);
    }

    @Override
    public boolean isSupport(CameraApi currentCameraApi, Activity activity) {
        if (isThirdPartyIntent(activity)) {
            return false;
        }
        if (CameraApi.API1.equals(currentCameraApi)) {
            return isAPi1SupportDng("0");
        }
        if (CameraApi.API2.equals(currentCameraApi)) {
            return isAPi2SupportDng("0");
        }
        return false;
    }

    @Override
    public int getStage() {
        return DEFAULT_DNG_STAGE;
    }

    @Override
    public String getFeatureEntryName() {
        return DngEntry.class.getName();
    }

    @Override
    public Class getType() {
        return ICameraSetting.class;
    }

    @Override
    public Object createInstance() {
        return new Dng();
    }

    private boolean isAPi1SupportDng(String cameraId) {
        ConcurrentHashMap<String, DeviceDescription>
                deviceDescriptionMap = mDeviceSpec.getDeviceDescriptionMap();
        if (cameraId == null || deviceDescriptionMap == null || deviceDescriptionMap.size() <= 0) {
            return false;
        }

        Method enableRaw16Method =
                ReflectUtil.getMethod(Camera.class, "enableRaw16", boolean.class);
        if (enableRaw16Method == null) {
            LogHelper.d(TAG, "no dng interface !");
            return false;
        }

        if (!deviceDescriptionMap.containsKey(cameraId)) {
            return false;
        }
        Camera.Parameters parameters = deviceDescriptionMap.get(cameraId).getParameters();
        if (parameters == null) {
            return false;
        }
        return "true".equals(parameters.get("dng-supported"));
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private boolean isAPi2SupportDng(String cameraId) {
        ConcurrentHashMap<String, DeviceDescription>
                deviceDescriptionMap = mDeviceSpec.getDeviceDescriptionMap();
        if (cameraId == null || deviceDescriptionMap == null || deviceDescriptionMap.size() <= 0) {
            return false;
        }
        if (!deviceDescriptionMap.containsKey(cameraId)) {
            return false;
        }
        CameraCharacteristics cs = deviceDescriptionMap.get(cameraId).getCameraCharacteristics();
        if (cs == null) {
            return false;
        } else {
            int[] caplist = cs.get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES);
            if (caplist == null) {
                return false;
            }
            List<Integer> capArray = new ArrayList<Integer>(caplist.length);
            for (int cap : caplist) {
                capArray.add(cap);
            }
            if (capArray.contains(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES_RAW)) {
                return isRawSizeValid(cs);
            }
        }
        return false;
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private boolean isRawSizeValid(CameraCharacteristics cs) {
        StreamConfigurationMap config =
                cs.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] rawSizes = config.getOutputSizes(ImageFormat.RAW_SENSOR);
        if (rawSizes == null) {
            LogHelper.e(TAG, "[isDngSupported]" +
                    " No capture sizes available for raw format");
            return false;
        }
        Rect activeSize = cs.get(CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
        if (activeSize == null) {
            LogHelper.e(TAG, "[isDngSupported] Active array is null");
            return false;
        } else {
            Size activeArraySize = new Size(activeSize.width(), activeSize.height());
            for (Size size : rawSizes) {
                if (size.getWidth() == activeArraySize.getWidth()
                        && size.getHeight() == activeArraySize.getHeight()) {
                    return true;
                }
            }
            return false;
        }
    }
}
