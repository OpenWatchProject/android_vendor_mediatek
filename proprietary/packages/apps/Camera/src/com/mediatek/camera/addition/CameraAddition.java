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
package com.mediatek.camera.addition;

import android.app.Activity;

import com.mediatek.camera.ICameraAddition;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraMode.ActionType;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.IFeatureConfig;
import com.mediatek.camera.platform.IFileSaver;
import com.mediatek.camera.platform.IFocusManager;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.util.Log;

import junit.framework.Assert;

public abstract class CameraAddition implements ICameraAddition {
    private static final String TAG = "CameraAddition";

    protected ICameraContext mICameraContext;
    protected Activity mActivity;
    protected ICameraDeviceManager mICameraDeviceManager;
    protected ISettingCtrl mISettingCtrl;
    protected IFileSaver mIFileSaver;
    protected ICameraAppUi mICameraAppUi;
    protected IModuleCtrl mIModuleCtrl;
    protected ICameraDevice mICameraDevice;
    protected IFocusManager mIFocusManager;
    protected IFeatureConfig mIFeatureConfig;

    public CameraAddition(ICameraContext cameraContext) {
        Assert.assertNotNull(cameraContext);
        mICameraContext = cameraContext;

        mActivity = cameraContext.getActivity();
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
        mISettingCtrl = cameraContext.getSettingController();
        mIFileSaver = cameraContext.getFileSaver();
        mICameraAppUi = cameraContext.getCameraAppUi();
        mIModuleCtrl = cameraContext.getModuleController();
        mIFeatureConfig = cameraContext.getFeatureConfig();

        Assert.assertNotNull(mActivity);
        Assert.assertNotNull(mICameraDeviceManager);
        Assert.assertNotNull(mISettingCtrl);
        Assert.assertNotNull(mIFileSaver);
        Assert.assertNotNull(mICameraAppUi);
        Assert.assertNotNull(mIModuleCtrl);
        Assert.assertNotNull(mIFeatureConfig);
    }

    @Override
    public void resume() {

    }

    @Override
    public void pause() {

    }

    @Override
    public void destory() {

    }

    @Override
    public boolean isOpen() {
        return false;
    }

    @Override
    public void setListener(Listener listener) {

    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        return false;
    }

    @Override
    public boolean execute(AdditionActionType type, Object... arg) {
        return false;
    }

    protected void updateCameraDevice() {
        int camerId = mICameraDeviceManager.getCurrentCameraId();
        mICameraDevice = mICameraDeviceManager.getCameraDevice(camerId);
    }

    protected void updateFocusManager() {
        mIFocusManager = mICameraContext.getFocusManager();
    }
}
