/*
 *
 *  *   Copyright Statement:
 *  *
 *  *     This software/firmware and related documentation ("MediaTek Software") are
 *  *     protected under relevant copyright laws. The information contained herein is
 *  *     confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 *  *     the prior written permission of MediaTek inc. and/or its licensor, any
 *  *     reproduction, modification, use or disclosure of MediaTek Software, and
 *  *     information contained herein, in whole or in part, shall be strictly
 *  *     prohibited.
 *  *
 *  *     MediaTek Inc. (C) 2016. All rights reserved.
 *  *
 *  *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *  *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *  *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *  *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *  *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *  *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *  *     NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *  *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *  *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *  *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *  *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *  *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *  *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *  *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *  *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *  *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *  *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *  *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *  *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *  *
 *  *     The following software/firmware and/or related documentation ("MediaTek
 *  *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *  *     any receiver's applicable license agreements with MediaTek Inc.
 *
 */

package com.mediatek.camera.common.loader;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.hardware.Camera.Parameters;
import android.provider.MediaStore;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.DeviceUsage;

import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nonnull;

/**
 * FeatureEntry base implementation.
 */
abstract public class FeatureEntryBase implements IFeatureEntry {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(FeatureEntryBase.class.getSimpleName());

    private static final int DEFAULT_STAGE = 2;

    protected Resources mResources;
    protected DeviceSpec mDeviceSpec;
    protected CameraApi mDefaultCameraApi;
    protected String mCurrentModeKey;
    protected Context mContext;

    /**
     * create an entry.
     * @param context current activity.
     * @param resources current resources.
     */
    public FeatureEntryBase(Context context, Resources resources) {
        if (context instanceof Activity) {
           mContext = ((Activity) context).getApplicationContext();
        } else {
            mContext = context;
        }

        mResources = mContext.getResources();
    }

    @Override
    public void updateCurrentModeKey(String currentModeKey) {
        mCurrentModeKey = currentModeKey;
    }

    @Override
    public DeviceUsage updateDeviceUsage(String modeKey,
                                         DeviceUsage originalDeviceUsage) {
        return originalDeviceUsage;
    }

    @Override
    public void setDeviceSpec(@Nonnull DeviceSpec deviceSpec) {
        mDeviceSpec = deviceSpec;
        mDefaultCameraApi = mDeviceSpec.getDefaultCameraApi();
    }

    @Override
    public void notifyBeforeOpenCamera(@Nonnull String cameraId, @Nonnull CameraApi cameraApi) {
    }

    @Override
    public void setParameters(@Nonnull String cameraId,
                              @Nonnull Parameters originalParameters) {
        ConcurrentHashMap<String, DeviceDescription> descriptionMap =
                mDeviceSpec.getDeviceDescriptionMap();
        if (cameraId != null && descriptionMap.containsKey(cameraId)) {
            descriptionMap.get(cameraId).setParameters(originalParameters);
        }
    }

    @Override
    public int getStage() {
        return DEFAULT_STAGE;
    }

    @Override
    public IAppUi.ModeItem getModeItem() {
        return null;
    }

    /**
     * Judge current is launch by intent.
     * @param activity the launch activity.
     * @return true means is launch by intent; otherwise is false.
     */
    protected boolean isThirdPartyIntent(Activity activity) {
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        boolean value = MediaStore.ACTION_IMAGE_CAPTURE.equals(action) ||
                MediaStore.ACTION_VIDEO_CAPTURE.equals(action);
        return value;
    }
}
