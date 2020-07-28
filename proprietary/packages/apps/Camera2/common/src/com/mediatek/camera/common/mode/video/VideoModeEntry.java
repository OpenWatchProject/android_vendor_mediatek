/*
 *
 *  *   Copyright Statement:
 *  *
 *  *     This software/firmware and related documentation ("MediaTek Software") are
 *  *     protected under relevant copyright laws. The information contained herein is
 *  *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *  *     the prior written permission of MediaTek inc. and/or its licensors, any
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
 *  *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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

package com.mediatek.camera.common.mode.video;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.provider.MediaStore;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.utils.CameraUtil;

/**
 * Video Mode entry.
 */

public class VideoModeEntry extends FeatureEntryBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(VideoModeEntry.class.getSimpleName());

    /**
     * create an entry.
     *
     * @param context   current activity.
     * @param resources current resources.
     */
    public VideoModeEntry(Context context, Resources resources) {
        super(context, resources);
    }

    @Override
    public boolean isSupport(CameraApi currentCameraApi, Activity activity) {
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        boolean support = !(MediaStore.ACTION_IMAGE_CAPTURE.equals(action) || MediaStore
                .ACTION_VIDEO_CAPTURE.equals(action));
        LogHelper.d(TAG, "[isSupport] : " + support);
        return support;
    }

    /**
     * Get the advance feature name.
     *
     * @return Feature name.
     */
    @Override
    public String getFeatureEntryName() {
        return VideoModeEntry.class.getName();
    }

    /**
     * Get feature name, the type is define as a Class.
     * For excample,
     * if the feature is a mode ,it will implement the ICameraMode, so the
     * feature typle will  be the ICameraMode.class.
     * if it is a setting, the type will be the ICameraSetting.class.
     *
     * @return Feature type.
     */
    @Override
    public Class getType() {
        return ICameraMode.class;
    }

    /**
     * Create the feature instance.
     *
     * @return Return The ICameraMode or ICameraSetting implementer instance.
     */
    @Override
    public Object createInstance() {
        return new VideoMode();
    }

    /**
     * Get mode item.
     *
     * @return the mode item info.
     */
    @Override
    public IAppUi.ModeItem getModeItem() {
        IAppUi.ModeItem modeItem = new IAppUi.ModeItem();
        modeItem.mType = "Video";
        modeItem.mPriority = 10;
        modeItem.mClassName = getFeatureEntryName();
        modeItem.mModeName = mResources.getString(mResources.getIdentifier("normal_mode_title",
                "string", mContext.getPackageName()));
        if (CameraUtil.getLogicalCameraId() == null
                && CameraUtil.getDualZoomId() == null) {
            modeItem.mSupportedCameraIds = new String[]{"0", "1"};
        } else if (CameraUtil.getLogicalCameraId() == null
                && CameraUtil.getDualZoomId() != null) {
            modeItem.mSupportedCameraIds = new String[]{"0", "1",
                    CameraUtil.getDualZoomId()};
        } else if (CameraUtil.getLogicalCameraId() != null
                && CameraUtil.getDualZoomId() == null) {
            modeItem.mSupportedCameraIds = new String[]{"0", "1",
                    CameraUtil.getLogicalCameraId()};
        } else {
            modeItem.mSupportedCameraIds = new String[]{"0", "1",
                    CameraUtil.getLogicalCameraId(), CameraUtil.getDualZoomId()};
        }
        return modeItem;
    }
}
