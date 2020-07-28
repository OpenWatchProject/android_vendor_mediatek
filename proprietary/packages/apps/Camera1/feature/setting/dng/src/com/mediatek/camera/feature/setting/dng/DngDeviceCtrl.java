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

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

/**
 * This is for dng device flow.
 */
public class DngDeviceCtrl {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(DngDeviceCtrl.class.getSimpleName());
    private DngParameterConfig mParameterConfig;
    private DngCaptureRequestConfig mCaptureRequestConfig;
    private IDngConfig mDngConfig;

    private IDngConfig.OnDngValueUpdateListener mDngValueUpdateListener;

    /**
     * This is to api1 flow.
     * @param  deviceRequester the device requester.
     * @return  the Api1 interface.
     */
    public ICameraSetting.IParametersConfigure getParametersConfigure(
            ISettingManager.SettingDeviceRequester deviceRequester) {
        if (mParameterConfig == null) {
            mParameterConfig = new DngParameterConfig(deviceRequester);
            mParameterConfig.setDngValueUpdateListener(mDngValueUpdateListener);
            mDngConfig = mParameterConfig;
        }
        return mParameterConfig;
    }

    /**
     * This is to api2 flow.
     * @param  device2Requester the device 2 requester.
     * @return  the Api2 interface.
     */
    public ICameraSetting.ICaptureRequestConfigure getCaptureRequestConfigure(
            ISettingManager.SettingDevice2Requester device2Requester) {
        if (mCaptureRequestConfig == null) {
            mCaptureRequestConfig = new DngCaptureRequestConfig(device2Requester);
            mCaptureRequestConfig.setDngValueUpdateListener(mDngValueUpdateListener);
            mDngConfig = mCaptureRequestConfig;
        }
        return mCaptureRequestConfig;
    }

    /**
     * It is for change setting value request when post restriction.
     */
    public void requestChangeOverrideValues() {
        mDngConfig.requestChangeOverrideValues();
    }

    /**
     * It is for notify the override setting value.
     * @param dngStatus dng value by other restriction.
     */
    public void notifyOverrideValue(String dngStatus) {
        if (mDngConfig != null) {
            mDngConfig.notifyOverrideValue(dngStatus.equals(IDngConfig.DNG_ON));
        }
    }

    /**
     * This is to notify dng value.
     * @param  dngStatus dng value by other restriction.
     * @param isTakePicture true, if taking picture.
     */
    public void setDngStatus(String dngStatus, boolean isTakePicture) {
        if (mDngConfig != null) {
            mDngConfig.setDngStatus(dngStatus.equals(IDngConfig.DNG_ON), isTakePicture);
        }
    }

    /**
     * This is to set face detection update listener.
     * @param  onDngValueUpdateListener the detected face listener.
     *
     */
    public void setDngValueUpdateListener(IDngConfig.OnDngValueUpdateListener
                                                  onDngValueUpdateListener) {
        mDngValueUpdateListener = onDngValueUpdateListener;
    }

    /**
     * Used to release camera device resource.
     */
    public void onModeClosed() {
        if (mDngConfig != null) {
            mDngConfig.onModeClosed();
        }
    }
}
