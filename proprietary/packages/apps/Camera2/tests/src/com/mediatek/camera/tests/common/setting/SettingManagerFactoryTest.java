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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.tests.common.setting;

import android.support.test.uiautomator.UiDevice;

import com.mediatek.camera.CameraActivity;
import com.mediatek.camera.common.CameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.SettingManager;
import com.mediatek.camera.common.setting.SettingManagerFactory;
import com.mediatek.camera.tests.CameraTestCaseBase;

/**
 * Functional tests for {@link com.mediatek.camera.common.setting.SettingManagerFactory}.
 */
public class SettingManagerFactoryTest extends CameraTestCaseBase<CameraActivity> {
    private static final String TAG = "SettingManagerTest";
    private IApp mApp;
    private UiDevice mUiDevice;
    private SettingManagerFactory mSettingManagerFactory;
    CameraContext mContext = new CameraContext();
    private final static String BACK_CAMERA = "0";
    private final static String FRONT_CAMERA = "1";

    /**
     * Constructor of SettingManagerTest.
     */
    public SettingManagerFactoryTest() {
        super(CameraActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mApp = (CameraActivity) mActivity;
        //FIXME CameraActivity will create mContext and mSettingManagerFactory,case has its own's
        mContext.create(mApp, mActivity);
        mSettingManagerFactory = mContext.getSettingManagerFactory();
        assertNotNull("mSettingManagerFactory is null", mSettingManagerFactory);
    }

    /**
     * Test SettingManager should be created and recycle successfully.
     */
    public void testCreateSettingMg() {
        SettingManager backSettingManagerOne = (SettingManager) mSettingManagerFactory
                .getInstance(
                        BACK_CAMERA,
                        "test-mode-key",
                        ICameraMode.ModeType.PHOTO,
                        CameraApi.API1);
        assertNotNull("backSettingManagerOne is null", backSettingManagerOne);
        //after recycle,will create a new SettingManager with the same camera id
        mSettingManagerFactory.recycle(BACK_CAMERA);
        SettingManager backSettingManagerTwo = (SettingManager) mSettingManagerFactory
                .getInstance(BACK_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        assertNotNull("backSettingManagerTwo is null", backSettingManagerTwo);
        assertNotSame(backSettingManagerOne + " is the same with " + backSettingManagerTwo,
                backSettingManagerOne, backSettingManagerTwo);
    }

    /**
     * Test same camera id will have only one SettingManager instance.
     */
    public void testSettingManagerIsSameWithSameCameraId() {
        SettingManager backSettingManagerOne = (SettingManager) mSettingManagerFactory
                .getInstance(BACK_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        assertNotNull("settingManagerOne is null", backSettingManagerOne);
        SettingManager backSettingManagerTwo = (SettingManager) mSettingManagerFactory
                .getInstance(BACK_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        assertEquals(backSettingManagerOne + " not equals to " + backSettingManagerTwo,
                backSettingManagerOne, backSettingManagerTwo);
    }

    /**
     * Test different camera id will have different SettingManager instance.
     */
    public void testSettingManagerNotSameWithDifCameraId() {
        SettingManager backSettingManager = (SettingManager) mSettingManagerFactory
                .getInstance(BACK_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        assertNotNull("settingManagerOne is null", backSettingManager);
        SettingManager frontSettingManager = (SettingManager) mSettingManagerFactory
                .getInstance(FRONT_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        assertNotNull("settingManagerOne is null", frontSettingManager);
        assertNotSame(backSettingManager + " is the same with " + frontSettingManager,
                backSettingManager, frontSettingManager);
    }

    /**
     * Test SettingManager will be recycle successfully.
     */
    public void testSettingManagerRecycle() {
        SettingManager backSettingManagerOne = (SettingManager) mSettingManagerFactory
                .getInstance(BACK_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        mSettingManagerFactory.recycle(BACK_CAMERA);
        SettingManager frontSettingManagerOne = (SettingManager) mSettingManagerFactory
                .getInstance(FRONT_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        //after recycleAll,should create new SettingManager for special camera id.
        mSettingManagerFactory.recycleAll();
        SettingManager backSettingManagerTwo = (SettingManager) mSettingManagerFactory
                .getInstance(BACK_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        SettingManager frontSettingManagerTwo = (SettingManager) mSettingManagerFactory
                .getInstance(FRONT_CAMERA, "test-mode-key",
                        ICameraMode.ModeType.PHOTO, CameraApi.API1);
        assertNotSame(backSettingManagerOne + " is the same with " + backSettingManagerTwo,
                backSettingManagerOne, backSettingManagerTwo);
        assertNotSame(frontSettingManagerOne + " is the same with " + frontSettingManagerTwo,
                frontSettingManagerOne, frontSettingManagerTwo);
    }
}