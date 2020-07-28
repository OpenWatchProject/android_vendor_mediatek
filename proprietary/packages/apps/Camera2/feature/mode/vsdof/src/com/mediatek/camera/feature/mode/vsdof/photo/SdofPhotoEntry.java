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

import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.utils.CameraUtil;

/**
 * Pip photo mode entry.
 */
public class SdofPhotoEntry extends FeatureEntryBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SdofPhotoEntry.class.getSimpleName());
    private static final String MTK_MULTI_CAM_FEATURE_AVAILABLE_MODE
            = "com.mediatek.multicamfeature.availableMultiCamFeatureMode";
    private static final String VSDOF_KEY = "com.mediatek.multicamfeature.multiCamFeatureMode";
    private static final int MTK_MULTI_CAM_FEATURE_MODE_VSDOF = 1;

    /**
     * create an entry.
     *
     * @param context   current activity.
     * @param resources current resources.
     */
    public SdofPhotoEntry(Context context, Resources resources) {
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
        if (CameraUtil.getLogicalCameraId() == null) {
            return false;
        }
        return true;
    }

    @Override
    public String getFeatureEntryName() {
        return SdofPhotoEntry.class.getName();
    }

    @Override
    public Class getType() {
        return ICameraMode.class;
    }

    @Override
    public Object createInstance() {
        return new SdofPhotoMode();
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
        modeItem.mType = "Picture";
        modeItem.mPriority = 20;
        modeItem.mClassName = getFeatureEntryName();
        modeItem.mModeName = mResources.getString(R.string.sdof_mode_name);
        if (CameraUtil.getLogicalCameraId() == null
                && CameraUtil.getDualZoomId() == null) {
            modeItem.mSupportedCameraIds = new String[]{"0"};
        } else if (CameraUtil.getLogicalCameraId() == null
                && CameraUtil.getDualZoomId() != null) {
            modeItem.mSupportedCameraIds = new String[]{"0",
                    CameraUtil.getDualZoomId()};
        } else if (CameraUtil.getLogicalCameraId() != null
                && CameraUtil.getDualZoomId() == null) {
            modeItem.mSupportedCameraIds = new String[]{"0",
                    CameraUtil.getLogicalCameraId()};
        } else {
            modeItem.mSupportedCameraIds = new String[]{"0",
                    CameraUtil.getLogicalCameraId(), CameraUtil.getDualZoomId()};
        }
        return modeItem;
    }
}