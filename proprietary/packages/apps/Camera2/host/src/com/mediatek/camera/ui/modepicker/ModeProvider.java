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
package com.mediatek.camera.ui.modepicker;

import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUi.ModeItem;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.loader.DeviceDescription;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentSkipListMap;

/**
 * Provider will store current support mode list and provide apis for query.
 */

public class ModeProvider {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ModeProvider.class.getSimpleName());
    private ConcurrentSkipListMap<String, LinkedHashMap<String, ModeItem>> mModeMap
            = new ConcurrentSkipListMap<>();

    private LinkedHashMap<String, ModeItem> mModeBackup = new LinkedHashMap<>();
    private Context mContext;
    public ModeProvider(Context context) {
        mContext = context;
    }

    public void registerMode(ModeItem item) {
        if (item == null) {
            LogHelper.e(TAG, "Mode item is null!");
            return;
        }
        LogHelper.d(TAG, "registerMode mode name " + item.mModeName + " type " + item.mType);
        mModeBackup.put(item.mClassName, item);
        if (mModeMap.containsKey(item.mModeName)) {
            LinkedHashMap<String, ModeItem> map = mModeMap.get(item.mModeName);
            map.put(item.mType, item);
        } else {
            LinkedHashMap<String, ModeItem> map = new LinkedHashMap<>();
            map.put(item.mType, item);
            mModeMap.put(item.mModeName, map);
        }
    }

    /**
     * Clear all modes.
     */
    public void clearAllModes() {
        mModeMap.clear();
        mModeBackup.clear();
    }

    /**
     * Get mode item by the class name.
     * @param className The mode's class name.
     * @return Mode item.
     */
    public ModeItem getMode(String className) {
        LogHelper.d(TAG, "getMode className = " + className);
        if (className == null) {
            LogHelper.e(TAG, "Class name is null!");
            return  null;
        }
        ModeItem item = null;

        if (mModeBackup.containsKey(className)) {
            item = mModeBackup.get(className);
        }
        return item;
    }

    /**
     * Get current mode support types, the type is such as "Picture" and "Video".
     * @param modeName Current mode name.
     * @param deviceId Camera id.
     * @return Support types.
     */
    public String[] getModeSupportTypes(String modeName, String deviceId) {
        LogHelper.d(TAG, "getModeSupportTypes modeName " + modeName + " deviceId " + deviceId);
        HashSet<String> supportTypes = new HashSet<>();
        if (mModeMap.containsKey(modeName)) {
            Map<String, ModeItem> modeList = mModeMap.get(modeName);
            for (Map.Entry<String, ModeItem> entry : modeList.entrySet()) {
                ModeItem item = entry.getValue();
                for (int i = 0; i < item.mSupportedCameraIds.length; i++) {
                    if (getCameraFacingById(item.mSupportedCameraIds[i])
                            == getCameraFacingById(deviceId)) {
                        LogHelper.d(TAG, "find one type = " + entry.getKey());
                        supportTypes.add(entry.getKey());
                    }
                }
            }
        }
        return supportTypes.toArray(new String[supportTypes.size()]);
    }


    private int getCameraFacingById(String deviceId) {


        ConcurrentHashMap<String, DeviceDescription> deviceDescriptions =
                CameraApiHelper.getDeviceSpec(mContext).getDeviceDescriptionMap();

        DeviceDescription description = deviceDescriptions.get(deviceId);

        if (description != null) {
            CameraCharacteristics characteristics = description.getCameraCharacteristics();
            return characteristics.get(CameraCharacteristics.LENS_FACING);
        } else {
            return -1;
        }
    }

    /**
     * Get mode entry name by the mode name and type.
     * @param modeName Mode name string.
     * @param modeType Mode type string.
     * @return The mode entry string.
     */
    public ModeItem getModeEntryName(String modeName, String modeType) {
        ModeItem item = null;
        if (mModeMap.containsKey(modeName)) {
            Map<String, ModeItem> modeList = mModeMap.get(modeName);
            if (modeList.size() == 1) {
                item = (ModeItem) (modeList.values().toArray()[0]);
            } else {
                item = modeList.get(modeType);
            }

        }
        return item;
    }

    public Map<String, LinkedHashMap<String, IAppUi.ModeItem>> getModes() {
        return mModeMap;
    }

    public Map<String, ModeItem> getModes2() {
        return mModeBackup;
    }
}
