/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.tests.v3.checker;

import android.hardware.Camera;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check the Zsd restriction with different mode.
 */

public class ZsdModeRestrictionChecker extends CheckerOne {
    static final private LogUtil.Tag TAG = new LogUtil.Tag(
            ZsdModeRestrictionChecker.class.getSimpleName());

    @Override
    protected void doCheck() {
        // init current is normal photo mode or not
        UiObject2 shutter = Utils.findObject(By.res("com.mediatek.camera:id/shutter_root"));
        boolean isNormalPhotoMode = shutter.getContentDescription().startsWith("PhotoMode is");
        boolean isPipPhotoMode = shutter.getContentDescription().startsWith("PipPhotoMode is");

        // normal photo mode, dng exist
        // other mode, dng not exist
        if (isNormalPhotoMode || isPipPhotoMode) {
            new SettingItemExistedChecker("ZSD", false).check(
                    SettingItemExistedChecker.INDEX_EXISTED);
        } else {
            new SettingItemExistedChecker("ZSD", false).check(
                    SettingItemExistedChecker.INDEX_NOT_EXISTED);
        }

    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Check ZSD restriction with modes.";
    }

    @Override
    public boolean isSupported(int index) {
        if (TestContext.mLatestCameraFacing == Camera.CameraInfo.CAMERA_FACING_BACK) {
            if (Utils.isFeatureSupported("com.mediatek.camera.at.zsd.back", null) == null) {
                String zsdModeValues = TestContext.mBackCameraParameters.get("zsd-mode-values");
                LogHelper.d(TAG, "[isSupported] zsd-mode-values = " + zsdModeValues);
                return zsdModeValues != null && zsdModeValues.contains("on");
            } else {
                return Utils.isFeatureSupported("com.mediatek.camera.at.zsd.back");
            }
        } else {
            if (Utils.isFeatureSupported("com.mediatek.camera.at.zsd.front", null) == null) {
                String zsdModeValues = TestContext.mFrontCameraParameters.get("zsd-mode-values");
                LogHelper.d(TAG, "[isSupported] zsd-mode-values = " + zsdModeValues);
                return zsdModeValues != null && zsdModeValues.contains("on");
            } else {
                return Utils.isFeatureSupported("com.mediatek.camera.at.zsd.front");
            }
        }
    }
}
