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

package com.mediatek.camera.feature.mode.vsdof.photo;

import com.mediatek.camera.common.setting.ISettingManager;

/**
 * When camera will be open,can be wrapped the parameters to the info.
 */
public class DeviceInfo {
    private ISettingManager mSettingManager;
    private String mCameraId;
    private boolean mNeedSync;
    private boolean mNeedFastStartPreview;

    /**
     * Set the camera id.
     * @param cameraId which camera need opened.
     */
    void setCameraId(String cameraId) {
        mCameraId = cameraId;
    }

    /**
     * Add a setting manager to the info.
     * @param settingManager current setting manager.
     */
    void setSettingManager(ISettingManager settingManager) {
        mSettingManager = settingManager;
    }

    /**
     * Add whether need sync open the camera.
     * @param needSync whether need sync the camera.
     */
    void setNeedOpenCameraSync(boolean needSync) {
        mNeedSync = needSync;
    }

    /**
     * Add whether need fast start preview.
     * @param needFastStartPreview whether need fast start perview.
     */
    void setNeedFastStartPreview(boolean needFastStartPreview) {
        mNeedFastStartPreview = needFastStartPreview;
    }

    /**
     * Get the setting manager.
     * @return current setting manager.
     */
    public ISettingManager getSettingManager() {
        return mSettingManager;
    }

    /**
     * Get the camera id from info.
     * @return current camera id.
     */
    public String getCameraId() {
        return mCameraId;
    }

    /**
     * Get whether need open sync the camera.
     * @return whether open sync the camera.
     */
    public boolean getNeedOpenCameraSync() {
        return mNeedSync;
    }

    /**
     * Get whether fast start preview.
     * @return true means need fast preview.
     */
    public boolean getNeedFastStartPreview() {
        return mNeedFastStartPreview;
    }
}
