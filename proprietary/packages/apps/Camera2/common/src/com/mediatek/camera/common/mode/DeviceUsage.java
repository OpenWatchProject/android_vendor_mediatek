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

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;

import java.util.ArrayList;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * This class is used to describe how mode use camera device,
 * this is used to judge whether need to close camera when pause mode.
 */
public class DeviceUsage {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(DeviceUsage.class.getSimpleName());
    public static final String DEVICE_TYPE_NORMAL = "normal";
    public static final String DEVICE_TYPE_STEREO = "stereo";
    public static final String DEVICE_TYPE_STEREO_VSDOF = "vsdof";
    public static final String BUFFER_FLOW_TYPE_NORMAL = "normal";
    public static final String BUFFER_FLOW_TYPE_POSTALGO = "postalgo";

    private String mDeviceType;
    private final CameraApi mCameraApi;
    private ArrayList<String> mNeedOpenedCameraIdList;
    private String mBufferFlowType = BUFFER_FLOW_TYPE_NORMAL;

    /**
     * Construct a device spec with specified parameters.
     *
     * @param deviceType device usage.
     * @param cameraApi api type.
     * @param cameraIdList camera id list.
     */
    public DeviceUsage(@Nonnull String deviceType,
                       @Nonnull CameraApi cameraApi,
                       @Nonnull ArrayList<String> cameraIdList) {
        mDeviceType = deviceType;
        mCameraApi = cameraApi;
        mNeedOpenedCameraIdList = cameraIdList;
    }

    /**
     * Construct a device spec with specified parameters.
     *
     * @param deviceType device usage.
     * @param cameraApi api type.
     * @param cameraIdList camera id list.
     * @param bufferFlowType buffer flow type.
     */
    public DeviceUsage(@Nonnull String deviceType,
                       @Nonnull CameraApi cameraApi,
                       @Nonnull ArrayList<String> cameraIdList,
                       @Nonnull String bufferFlowType) {
        mDeviceType = deviceType;
        mCameraApi = cameraApi;
        mNeedOpenedCameraIdList = cameraIdList;
        mBufferFlowType = bufferFlowType;
    }

    /**
     * Update device type, dual camera zoom setting may update device type.
     * @param deviceType the new device type.
     */
    public void updateDeviceType(String deviceType) {
        mDeviceType = deviceType;
    }

    /**
     * Get device type, it a string, ex: normal, stereo.
     * This can be defined by mode to any non-null string.
     *
     * @return the device usage.
     */
    public String getDeviceType() {
        return mDeviceType;
    }

    /**
     * Get api type.
     *
     * @return the api type.
     */
    public CameraApi getCameraApi() {
        return mCameraApi;
    }

    /**
     * Get camera id list.
     *
     * @return the camera id with list.
     */
    public ArrayList<String> getCameraIdList() {
        return mNeedOpenedCameraIdList;
    }

    /**
     * update camera id list.
     *
     * @param  cameraList camera list.
     */
    public void updateCameraIdList(ArrayList<String> cameraList) {
        mNeedOpenedCameraIdList = cameraList;
    }


    /**
     * Get needed closed camera id list.
     *
     * @param newModeDeviceUsage new device usage.
     * @return an list describes needed closed camera ids.
     */
    public ArrayList<String> getNeedClosedCameraIds(@Nullable DeviceUsage newModeDeviceUsage) {
        if (newModeDeviceUsage == null) {
            //means need close the old mode have opened camera.
            return mNeedOpenedCameraIdList;
        }

        // if API type or device type is different, restart all opened camera devices.
        if (!mCameraApi.equals(newModeDeviceUsage.getCameraApi()) ||
                !mDeviceType.equals(newModeDeviceUsage.getDeviceType())) {
            LogHelper.d(TAG, "[getNeedClosedCameraIds] newModeDeviceUsage getDeviceType "
                    + newModeDeviceUsage.getDeviceType());
            return mNeedOpenedCameraIdList;
        }

        // if the size is not same, need close all the have opened camera.
        boolean isSameSize = mNeedOpenedCameraIdList.size() ==
                newModeDeviceUsage.getCameraIdList().size();
        if (!isSameSize) {
            LogHelper.w(TAG, "[getNeedClosedCameraIds] isSameSize false");
            return mNeedOpenedCameraIdList;
        }

        //if size is the same one, but the value maybe the same one.
        //such as: need close camera ids : [0], but new mode camera ids: [1]
        //so this case also need close the old camera 0;
        boolean isSameValue = true;
        for (int i = 0; i < mNeedOpenedCameraIdList.size(); i++) {
            if (!newModeDeviceUsage.getCameraIdList().contains(mNeedOpenedCameraIdList.get(i))) {
                isSameValue = false;
                break;
            }
        }
        LogHelper.d(TAG, "[getNeedClosedCameraIds] isSameVale:" + isSameValue
                + ", mNeedOpenedCameraIdList :" + mNeedOpenedCameraIdList);
        if (isSameValue) {
            return new ArrayList<>();
        }
        return mNeedOpenedCameraIdList;
    }

    public String getBufferFlowType() {
        return mBufferFlowType;
    }
}