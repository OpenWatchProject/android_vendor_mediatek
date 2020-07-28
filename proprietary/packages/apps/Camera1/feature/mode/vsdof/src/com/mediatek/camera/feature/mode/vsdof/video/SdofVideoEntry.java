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

package com.mediatek.camera.feature.mode.vsdof.video;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.hardware.Camera;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.ICameraMode;


/**
 * Sdof video mode entry.
 */
public class SdofVideoEntry extends FeatureEntryBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SdofVideoEntry.class.getSimpleName());
    private static final String KEY_VSDOF_SUPPORTED = "stereo-vsdof-mode-values";
    private static final String KEY_VSDOF_SCENARIO = "vsdof-scenario-values";
    private static final String KEY_RECORD = "record";
    /**
     * create an entry.
     *
     * @param context   current activity.
     * @param resources current resources.
     */
    public SdofVideoEntry(Context context, Resources resources) {
        super(context, resources);
    }

    @Override
    public boolean isSupport(CameraApi currentCameraApi, Activity activity) {
        if (isThirdPartyIntent(activity)) {
            LogHelper.i(TAG, "[isSupport] false, third party intent.");
            return false;
        }
        if (mDeviceSpec.getDeviceDescriptionMap().size() < 2) {
            LogHelper.i(TAG, "[isSupport] false, camera ids < 2");
            return false;
        }
        if (CameraApi.API1.equals(currentCameraApi)) {
            return isVsdofSupported(
                    mDeviceSpec.getDeviceDescriptionMap().get("0").getParameters());
        }
        return false;
    }

    @Override
    public DeviceUsage updateDeviceUsage(String modeKey, DeviceUsage originalDeviceUsage) {
        originalDeviceUsage.updateDeviceType(DeviceUsage.DEVICE_TYPE_STEREO);
        return originalDeviceUsage;
    }

    @Override
    public String getFeatureEntryName() {
        return SdofVideoEntry.class.getName();
    }

    @Override
    public Class getType() {
        return ICameraMode.class;
    }

    @Override
    public Object createInstance() {
        return new SdofVideoMode();
    }
    /**
     * Get mode item if the feature is a mode.
     *
     * @return the mode item info.
     */
    @Override
    public IAppUi.ModeItem getModeItem() {
        IAppUi.ModeItem modeItem = new IAppUi.ModeItem();
        modeItem.mModeSelectedIcon = mResources.getDrawable(R.drawable.ic_sdof_mode_selected);
        modeItem.mModeUnselectedIcon = mResources.getDrawable(R.drawable.ic_sdof_mode_unselected);
        modeItem.mShutterIcon = null;
        modeItem.mType = "Video";
        modeItem.mPriority = 20;
        modeItem.mClassName = getFeatureEntryName();
        modeItem.mModeName = mResources.getString(R.string.sdof_mode_name);
        modeItem.mSupportedCameraIds = new String[]{"0"};
        return modeItem;
    }

    private boolean isVsdofSupported(Camera.Parameters parameters) {
        if (parameters == null) {
            LogHelper.i(TAG, "[isVsdofSupported] parameters is null!");
            return false;
        }
        String str = parameters.get(KEY_VSDOF_SUPPORTED);
        String scenario = parameters.get(KEY_VSDOF_SCENARIO);
        LogHelper.i(TAG, "[isVsdofSupported] support:" + str + ", scenario :" + scenario);
        if ("off".equals(str) || null == str) {
            return false;
        } else if (scenario != null && !scenario.contains(KEY_RECORD)) {
            return false;
        } else {
            return true;
        }
    }
}