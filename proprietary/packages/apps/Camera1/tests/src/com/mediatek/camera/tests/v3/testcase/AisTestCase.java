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
package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.helper.LoggerService;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.AisTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.AisExistedChecker;
import com.mediatek.camera.tests.v3.checker.AisSwitchOnOffChecker;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.SettingSwitchOnOffChecker;
import com.mediatek.camera.tests.v3.observer.AisObserver;
import com.mediatek.camera.tests.v3.operator.AisSwitchOnOffOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SettingSwitchButtonOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * AIS test case.
 */

@CameraFeatureTest
@AisTest
public class AisTestCase extends BaseCameraTestCase {
    private static final int SLEEP_TIME = 1;

    @Override
    public void setUp() {
        // Need to set adb command before opening camera, because adb command isn't worked
        // when set after camera opened.
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            LoggerService.getInstance().setAdbCommand("debug.camera.log.AppStreamMgr", "2");
        }
        super.setUp();
    }

    @Override
    public void tearDown() {
        super.tearDown();
        // Restore adb command.
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            LoggerService.getInstance().setAdbCommand("debug.camera.log.AppStreamMgr", "0");
        }
    }

    /**
     * Test AIS setting must enable in support mode.
     */
    @Test
    @FunctionTest
    public void testAisExistedOrNotInEachMode() {
        new MetaCase("TC_Camera_AIS_0001")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new AisExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_AIS_0001")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new AisExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_AIS_0001")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new AisExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_AIS_0001")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new AisExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_AIS_0001")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new AisExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    /**
     * Stress test for AIS switch on/off.
     */
    @Test
    @StabilityTest
    public void testSwitchAisStress() {
        new MetaCase("TC_Camera_AIS_0003")
                .observeBegin(new AisObserver())
                .addOperator(new AisSwitchOnOffOperator())
                .observeEnd()
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test AIS can be shown in front camera normal mode.
     */
    @Test
    @FunctionTest
    public void testAisExistedInEachCamera() {
        new MetaCase("TC_Camera_AIS_0004")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new AisExistedChecker(false), SettingItemExistedChecker.INDEX_EXISTED)
                .run();
    }

    /**
     * Test pause/resume camera, AIS value can be remembered.
     */
    @Test
    @FunctionTest
    public void testRememberAisSettingAfterPauseResumeByHomeKey() {
        new MetaCase("TC_Camera_AIS_0005")
                .addOperator(new AisSwitchOnOffOperator(),
                        SettingSwitchButtonOperator.INDEX_SWITCH_ON)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new AisSwitchOnOffChecker(),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_ON)
                .run();
    }

    /**
     * Test relaunch camera, AIS value can be remembered.
     */
    @Test
    @FunctionTest
    public void testRememberAisSettingAfterRelaunch() {
        new MetaCase("TC_Camera_AIS_0006")
                .addOperator(new AisSwitchOnOffOperator(),
                        SettingSwitchButtonOperator.INDEX_SWITCH_ON)
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new AisSwitchOnOffChecker(),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_ON)
                .run();
    }

    /**
     * Test AIS value can be remembered in each camera device.
     */
    @Test
    @FunctionTest
    public void testRememberAisSettingForEachCamera() {
        new MetaCase("TC_Camera_AIS_0007")
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addOperator(new AisSwitchOnOffOperator(),
                        SettingSwitchButtonOperator.INDEX_SWITCH_ON)
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_FRONT, SwitchCameraOperator.INDEX_BACK})
                .addChecker(new AisSwitchOnOffChecker(),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_OFF)
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addChecker(new AisSwitchOnOffChecker(),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_ON)
                .addOperator(new AisSwitchOnOffOperator(),
                        SettingSwitchButtonOperator.INDEX_SWITCH_OFF)
                .run();
    }

    /**
     * Test AIS default value.
     */
    @Test
    @FunctionTest
    public void testAisDefaultValue() {
        new MetaCase("TC_Camera_AIS_0008")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new AisSwitchOnOffChecker(),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_OFF)
                .run();
    }

    /**
     * Test supported status in 3rd app.
     */
    @Test
    @FunctionTest
    public void testSupportedStatusIn3rd() {
        new MetaCase("TC_Camera_AIS_0009")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                .addChecker(new AisExistedChecker(true),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    /**
     * Test supported status on API1 & API2.
     */
    @Test
    @FunctionTest
    public void testSupportedStatusOnAPI1API2() {
        if (new AisSwitchOnOffOperator().isSupported()) {
            new MetaCase("TC_Camera_AIS_0010")
                    .addChecker(new AisExistedChecker(false),
                            SettingItemExistedChecker.INDEX_EXISTED)
                    .run();
        } else {
            new MetaCase("TC_Camera_AIS_0010")
                    .addChecker(new AisExistedChecker(false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .run();
        }
    }

    /**
     * Test AIS disabled when hdr is on/auto.
     */
    @Test
    @FunctionTest
    public void testAisDisabledWhenHdrOnAuto() {
        new MetaCase("TC_Camera_AIS_0011")
                .addOperator(new HdrOperator(),
                        new int[]{HdrOperator.INDEX_ON, HdrOperator.INDEX_AUTO})
                .addChecker(new AisExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }
}
