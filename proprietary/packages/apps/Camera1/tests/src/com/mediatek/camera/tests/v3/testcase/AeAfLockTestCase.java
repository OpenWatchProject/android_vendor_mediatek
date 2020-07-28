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
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.AeAfLockTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.AeAfLockUiChecker;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.EvUiChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.observer.AeLockObserver;
import com.mediatek.camera.tests.v3.observer.AfLockObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.EvUiOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Test AE/AF Lock Ui and status is right.
 */
@CameraFeatureTest
@AeAfLockTest
public class AeAfLockTestCase extends BaseCameraTestCase {

    /**
     * Test AE/AF Lock supported in every modes,AE/AF Lock UI should be shown when supported and
     * hidden otherwise.
     * Test no error happens when update ev when AE/AF Lock supported.
     */
    @Test
    @FunctionTest
    public void testSupportStatusInEveryMode() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0001/0004/0007")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())

                .addOperator(new SwitchToNormalVideoModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)

                .addOperator(new SwitchToPanoramaModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToPipPhotoModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToPipVideoModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToSlowMotionModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)

                .addOperator(new SwitchToStereoModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)

                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)

                .addOperator(new SwitchToStereoVideoModeOperator())
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .run();
    }

    /**
     * Test AE/AF Lock UI should still shown after capture.
     */
    @Test
    @FunctionTest
    public void testUiShownAfterCapture() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0003")
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeEnd()
                .run();
    }

    /**
     * Test EV can be changed with night scene mode when switch to main sensor from sub sensor.
     */
    @Test
    @FunctionTest
    public void testUiWithSceneModeNight() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0005")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)
                .run();
    }

    /**
     * Test EV view is shown normally when do AE/AF Lock during touch focus.
     */
    @Test
    @FunctionTest
    public void testUiAfterSingleTapUp() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0005")
                .addOperator(new TouchFocusOperator(), 15)
                .observeBegin(new AeLockObserver(), AeLockObserver.INDEX_LOCK)
                .observeBegin(new AfLockObserver())
                .addOperator(new OnLongPressOperator(), 25)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeEnd()
                .observeEnd()
                .run();
    }

    /**
     * Test AE/AF is unLock after pause and resume camera several times.
     */
    @Test
    @StabilityTest
    public void testAeAfUnLockAfterPauseResume() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0011")
                .observeBegin(new AfLockObserver())
                .observeBegin(new AeLockObserver(), AeLockObserver.INDEX_LOCK)
                .addOperator(new OnLongPressOperator(), 25)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeEnd()
                .observeEnd()
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_NOT_EXIST)
                .repeatEnd()
                .run();
    }

    /**
     * Test AE/AF is unLock after exist and open camera again.
     */
    @Test
    @FunctionTest
    public void testAeAfUnLockAfterExistCamera() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        AeLockObserver observer = new AeLockObserver();
        observer.initEnv(null, true);
        new MetaCase("TC_Camera_AE/AFLock_0012")
                .observeBegin(new AfLockObserver())
                .observeBegin(new AeLockObserver(), AeLockObserver.INDEX_LOCK)
                .addOperator(new OnLongPressOperator(), 25)
                .observeEnd()
                .observeEnd()
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .observeBegin(observer)
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_NORMAL)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_NOT_EXIST)
                .observeEnd()
                .run();
    }

    /**
     * Test AE/AF Lock works normally in camera with is open by third party.
     */
    @Test
    @FunctionTest
    public void testAeAfLockInThirdParty() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0013")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                .observeBegin(new AfLockObserver())
                .observeBegin(new AeLockObserver(), AeLockObserver.INDEX_LOCK)
                .addOperator(new OnLongPressOperator(), 25)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeEnd()
                .observeEnd()

                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new AfLockObserver())
                .observeBegin(new AeLockObserver(), AeLockObserver.INDEX_LOCK)
                .addOperator(new OnLongPressOperator(), 25)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeEnd()
                .observeEnd()
                .run();
    }

    /**
     * Test no error happens when do AE/AF Lock and update EV during slow motion recording.
     */
    @Test
    @FunctionTest
    public void testAeAfLockDuringSlowMotionRecording() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        AeLockObserver observer = new AeLockObserver();
        observer.initEnv(CameraDeviceManagerFactory.CameraApi.API2, false);
        new MetaCase("TC_Camera_AE/AFLock_0014")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .observeBegin(new AfLockObserver())
                .observeBegin(observer, AeLockObserver.INDEX_LOCK)
                .addOperator(new OnLongPressOperator(), 20)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeEnd()
                .observeEnd()
                .addOperator(new EvUiOperator())
                .addOperator(new StopRecordOperator(true))
                .addOperator(new PauseResumeByRecentAppOperator())
                .run();
    }


    /**
     * Test AE/AF Lock UI should be hidden and AE/AF is unLocked after mode changed.
     */
    @Test
    @FunctionTest
    public void testCheckAeAfLockAfterModeChange() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0002")
                .observeBegin(new AfLockObserver())
                .observeBegin(new AeLockObserver(), AeLockObserver.INDEX_LOCK)
                .addOperator(new OnLongPressOperator(), 20)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .observeEnd()
                .observeEnd()
                .addOperator(new EvUiOperator())
                .observeBegin(new AeLockObserver(), AeLockObserver.INDEX_UNLOCK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_NOT_EXIST)
                .observeEnd()
                .run();
    }

    /**
     * Test no error happens and picture is saved when capture 3000 times.
     */
    @Test
    @StabilityTest
    public void testStressCaptureWithAeAfLock() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_AE/AFLock_0010")
                .addOperator(new OnLongPressOperator(), 25)
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new CapturePhotoOperator())
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .repeatEnd()
                .run();
    }
}
