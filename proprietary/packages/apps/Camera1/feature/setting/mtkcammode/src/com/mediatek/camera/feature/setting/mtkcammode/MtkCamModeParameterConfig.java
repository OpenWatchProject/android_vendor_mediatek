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

package com.mediatek.camera.feature.setting.mtkcammode;

import android.app.Activity;
import android.content.Intent;
import android.hardware.Camera;
import android.provider.MediaStore;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.ICameraSetting;

/**
 * Mtk Camera mode implementer for api1.
 */

public class MtkCamModeParameterConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(MtkCamModeParameterConfig.class.getSimpleName());
    private static final String MTK_CAMERA_MODE = "mtk-cam-mode";
    private static final int CAMERA_MODE_MTK_PRV = 1;
    private static final int CAMERA_MODE_MTK_PVO = 2;
    private Activity mActivity;
    private ICameraMode.ModeType mModeType;

    /**
     * mtk cam mode parameter config constructor.
     *
     * @param activity   current activity.
     */
    public MtkCamModeParameterConfig(Activity activity, ICameraMode.ModeType modeType) {
        mActivity = activity;
        if (modeType != null) {
            mModeType = modeType;
        }
    }

    /**
     * Set current mode type.
     *
     * @param modeType   current mode type.
     */
    public void setModeType(ICameraMode.ModeType modeType) {
        mModeType = modeType;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters parameters) {
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        LogHelper.d(TAG, "configParameters: " + parameters.get(MTK_CAMERA_MODE)
                    + ", mModeType: "+ mModeType);
        if (parameters.get(MTK_CAMERA_MODE) != null && !isThirdPartyIntent(mActivity)) {
            if (mModeType.equals(ICameraMode.ModeType.PHOTO)) {
                parameters.set(MTK_CAMERA_MODE, CAMERA_MODE_MTK_PRV);
            } else if (mModeType.equals(ICameraMode.ModeType.VIDEO)) {
                parameters.set(MTK_CAMERA_MODE, CAMERA_MODE_MTK_PVO);
            }
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
    }

    @Override
    public void sendSettingChangeRequest() {
    }

    private boolean isThirdPartyIntent(Activity activity) {
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        boolean value = MediaStore.ACTION_IMAGE_CAPTURE.equals(action) ||
                MediaStore.ACTION_IMAGE_CAPTURE_SECURE.equals(action) ||
                MediaStore.ACTION_VIDEO_CAPTURE.equals(action);
        return value;
    }
}
