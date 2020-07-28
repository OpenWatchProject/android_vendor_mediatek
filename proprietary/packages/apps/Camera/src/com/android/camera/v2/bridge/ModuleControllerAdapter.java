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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.android.camera.v2.bridge;

import android.app.Activity;
import android.view.KeyEvent;
import android.view.View;

import com.android.camera.v2.CameraModule;
import com.android.camera.v2.app.AppController;
import com.android.camera.v2.app.CameraAppUI;
import com.android.camera.v2.module.ModuleController;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.platform.module.ModuleCreator;

import junit.framework.Assert;

/**
 *  This adapter is used to adapt app level calls to module level.
 */
public class ModuleControllerAdapter extends CameraModule implements ModuleController {
    public static final int CAMERA_MODULE_INDEX = 0;
    public static final int DUAL_CAMERA_MODULE_INDEX = 1;

    private final com.mediatek.camera.v2.platform.module.ModuleController mCurrentModule;

    public ModuleControllerAdapter(AppController appController, int moduleIndex) {
        super(appController);
        Assert.assertNotNull(appController);
        mCurrentModule = ModuleCreator.create(appController.getAppControllerAdapter(),
                moduleIndex == DUAL_CAMERA_MODULE_INDEX);
    }

    @Override
    public String getModuleStringIdentifier() {
        return null;
    }

    @Override
    public void init(Activity activity, boolean isSecureCamera,
            boolean isCaptureIntent) {
        mCurrentModule.open(activity, isSecureCamera, isCaptureIntent);
    }

    @Override
    public void resume() {
        mCurrentModule.resume();
    }

    @Override
    public void pause() {
        mCurrentModule.pause();
    }

    @Override
    public void destroy() {
        mCurrentModule.close();
    }

    @Override
    public void onPreviewVisibilityChanged(int visibility) {
        int remotePreviewVisibility = visibility;
        int localPreviewVisibility;
        switch (remotePreviewVisibility) {
        case CameraAppUI.VISIBILITY_UNCOVERED:
            localPreviewVisibility = AppUi.PREVIEW_VISIBILITY_UNCOVERED;
            break;
        case CameraAppUI.VISIBILITY_COVERED:
            localPreviewVisibility = AppUi.PREVIEW_VISIBILITY_COVERED;
            break;
        default:
            localPreviewVisibility = AppUi.PREVIEW_VISIBILITY_UNCOVERED;
            break;
        }
        mCurrentModule.onPreviewVisibilityChanged(localPreviewVisibility);
    }

    @Override
    public void onLayoutOrientationChanged(boolean isLandscape) {
        mCurrentModule.onLayoutOrientationChanged(isLandscape);
    }

    @Override
    public void onOrientationChanged(int orientation) {
        mCurrentModule.onOrientationChanged(orientation);
    }

    @Override
    public boolean onBackPressed() {
        return mCurrentModule.onBackPressed();
    }

    @Override
    public void onBeforeCameraPicked(String newCameraId) {
        mCurrentModule.onBeforeCameraPicked(newCameraId);
    }

    @Override
    public void onCameraPicked(String newCameraId) {
        mCurrentModule.onCameraPicked(newCameraId);
    }

    @Override
    public boolean isUsingBottomBar() {
        return false;
    }

    @Override
    @Deprecated
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    @Deprecated
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    @Deprecated
    public void onSingleTapUp(View view, int x, int y) {

    }

    @Override
    public String getPeekAccessibilityString() {
        return null;
    }
}
