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

import android.hardware.camera2.CameraCharacteristics;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.StrobeTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.CapturedPhotoPictureSizeChecker;
import com.mediatek.camera.tests.v3.checker.FlashExifChecker;
import com.mediatek.camera.tests.v3.checker.FlashQuickSwitchChecker;
import com.mediatek.camera.tests.v3.checker.FlashQuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.QuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.checker.QuickSwitchOptionChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.observer.FlashLogObserver;
import com.mediatek.camera.tests.v3.observer.FlashlightObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.FlashlightOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.OpenFlashQuickSwitchOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.ZsdOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraFeatureTest
@StrobeTest
public class StrobeTestCase extends BaseCameraTestCase {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            StrobeTestCase.class.getSimpleName());
    private static final int CONTROL_AE_MODE_ON_EXTERNAL_FLASH = 5;
    private static final int INDEX_BACK = 0;
    /**
     * Check photo can be taken successfully with flash on/auto/off, zsd on/off and screen size
     * between full screen size and standard size.
     */
    @Test
    @FunctionTest
    public void testCapturePhoto() {
        // when support de-noise, zsd will be set as on dy default, there is not zsd item in
        // setting, so not add ZsdOperator when de-noise
        if (DenoiseTestCase.isDenoiseSupported()) {
            new MetaCase()
                    .addOperator(new FlashOperator())
                    .acrossBegin()
                    .addOperator(new PictureSizeOperator())
                    .acrossEnd()
                    .addChecker(new PreviewChecker())
                    .observeBegin(new PhotoSavedObserver(1))
                    .addOperator(new CapturePhotoOperator())
                    .observeEnd()
                    .addChecker(new PreviewChecker())
                    .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                    .addChecker(new CapturedPhotoPictureSizeChecker())
                    .run();
        } else {
            new MetaCase()
                    .addOperator(new FlashOperator())
                    .acrossBegin()
                    .addOperator(new ZsdOperator())
                    .acrossEnd()
                    .acrossBegin()
                    .addOperator(new PictureSizeOperator())
                    .acrossEnd()
                    .addChecker(new PreviewChecker())
                    .observeBegin(new PhotoSavedObserver(1))
                    .addOperator(new CapturePhotoOperator())
                    .observeEnd()
                    .addChecker(new PreviewChecker())
                    .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                    .addChecker(new CapturedPhotoPictureSizeChecker())
                    .run();
        }
    }

    @Test
    @FunctionTest
    public void testRememberStrobeSettingAfterRelaunch() {
        new MetaCase("TC_Camera_Strobe_0002/0017")
                .addOperator(new FlashOperator())
                .addChecker(new FlashQuickSwitchChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new FlashQuickSwitchChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testHideQuickSwitchOptionAfterCapture() {
        new MetaCase("TC_Camera_Strobe_0003")
                .addOperator(new OpenFlashQuickSwitchOperator())
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_SHOW)
                .addOperator(new CapturePhotoOperator())
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_HIDE)
                .run();
    }

    @Test
    @FunctionTest
    public void testHideQuickSwitchOptionAfterSwitchToVideo() {
        new MetaCase("TC_Camera_Strobe_0004")
                .addOperator(new OpenFlashQuickSwitchOperator())
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_SHOW)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_HIDE)
                .run();
    }

    @Test
    @FunctionTest
    public void testStrobeAutoOnOff() {
        new MetaCase("TC_Camera_Strobe_0005/0006/0007/0008/0009/0010")
                .observeBegin(new FlashLogObserver())
                .addOperator(new FlashOperator())
                .observeEnd()

                .observeBegin(new PhotoSavedObserver(1))
                .observeBegin(new FlashLogObserver())
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .addChecker(new FlashExifChecker())

                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new FlashQuickSwitchChecker())

                .observeBegin(new VideoSavedObserver())
                .observeBegin(new FlashLogObserver().initEnv(null, true))
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .observeEnd()

                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testStrobeCorrectInFrontCamera() {
        new MetaCase("TC_Camera_Strobe_0011")
                .observeBegin(new FlashLogObserver(), FlashLogObserver.INDEX_ON)
                .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                .observeEnd()
                .addOperator(new SwitchCameraOperator(true), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .run();
        if (isPanelFlashSupported()) {
            new MetaCase("TC_Camera_Strobe_0011")
                    .addChecker(new FlashQuickSwitchExistedChecker(),
                            QuickSwitchExistedChecker.INDEX_EXIST)
                    .run();
        } else {
            new MetaCase("TC_Camera_Strobe_0011")
                    .addChecker(new FlashQuickSwitchExistedChecker(),
                            QuickSwitchExistedChecker.INDEX_NOT_EXIST)
                    .run();
        }
        new MetaCase("TC_Camera_Strobe_0011")
                .addOperator(new SwitchCameraOperator(true), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_ON)
                .run();
    }

    @Test
    @FunctionTest
    public void testSupportStatusInEveryMode() {
        new MetaCase("TC_Camera_Strobe_0001/0012")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new FlashQuickSwitchExistedChecker(),
                        FlashQuickSwitchExistedChecker.INDEX_EXIST)
                .acrossBegin()
                .observeBegin(new FlashLogObserver())
                .addOperator(new FlashOperator())
                .observeEnd()
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_Strobe_0001/0012")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new FlashQuickSwitchExistedChecker(),
                        FlashQuickSwitchExistedChecker.INDEX_EXIST)
                .acrossBegin()
                .observeBegin(new FlashLogObserver().initEnv(null, true))
                .addOperator(new FlashOperator())
                .observeEnd()
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_Strobe_0001/0012")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new FlashQuickSwitchExistedChecker(),
                        FlashQuickSwitchExistedChecker.INDEX_EXIST)
                .acrossBegin()
                .observeBegin(new FlashLogObserver())
                .addOperator(new FlashOperator())
                .observeEnd()
                .acrossEnd()
                .run();

        if (CameraDeviceManagerFactory.CameraApi.API2
                .equals(CameraApiHelper.getCameraApiType(null))) {
            new MetaCase("TC_Camera_Strobe_0001/0012")
                    .addOperator(new SwitchToPipVideoModeOperator())
                    .addChecker(new FlashQuickSwitchExistedChecker(),
                            FlashQuickSwitchExistedChecker.INDEX_EXIST)
                    .acrossBegin()
                    .observeBegin(new FlashLogObserver().
                            initEnv(CameraDeviceManagerFactory.CameraApi.API2, true))
                    .addOperator(new FlashOperator())
                    .observeEnd()
                    .acrossEnd()
                    .run();
        } else {
            new MetaCase("TC_Camera_Strobe_0001/0012")
                    .addOperator(new SwitchToPipVideoModeOperator())
                    .addChecker(new FlashQuickSwitchExistedChecker(),
                            FlashQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                    .run();
        }

        new MetaCase("TC_Camera_Strobe_0001/0012")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new FlashQuickSwitchExistedChecker(),
                        FlashQuickSwitchExistedChecker.INDEX_EXIST)
                .acrossBegin()
                .observeBegin(new FlashLogObserver().initEnv(
                        CameraDeviceManagerFactory.CameraApi.API2, true),
                        new int[]{FlashLogObserver.INDEX_ON, FlashLogObserver.INDEX_OFF})
                .addOperator(new FlashOperator(), new int[]{FlashOperator.INDEX_ON,
                        FlashOperator.INDEX_OFF})
                .observeEnd()
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_Strobe_0001/0012")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new FlashQuickSwitchExistedChecker(),
                        FlashQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                .run();
    }

    /**
     * Test strobe value should be sync during when mode switch between photo and video.
     */
    @Test
    @FunctionTest
    public void testStrobeSyncWhenSwitchMode() {
        new MetaCase("TC_Camera_Strobe_0021")
                .addOperator(new FlashOperator())
                .addChecker(new FlashQuickSwitchChecker())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addChecker(new FlashQuickSwitchChecker())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .addChecker(new PreviewChecker())
                .addChecker(new FlashQuickSwitchChecker())
                .run();
    }

    @Test
    @StabilityTest
    public void testSwitchStrobeModeStress() {
        new MetaCase("TC_Camera_Strobe_0013")
                .observeBegin(new FlashLogObserver())
                .addOperator(new FlashOperator())
                .observeEnd()
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @StabilityTest
    public void testCaptureInStrobeOnAutoModeStress() {
        new MetaCase("TC_Camera_Strobe_0015")
                .observeBegin(new FlashLogObserver(),
                        new int[]{FlashLogObserver.INDEX_ON, FlashLogObserver.INDEX_AUTO})
                .addOperator(new FlashOperator(),
                        new int[]{FlashOperator.INDEX_ON, FlashOperator.INDEX_AUTO})
                .observeEnd()
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .observeBegin(new PhotoSavedObserver(1))
                .observeBegin(new FlashLogObserver(),
                        new int[]{FlashLogObserver.INDEX_ON, FlashLogObserver.INDEX_AUTO})
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .addChecker(new FlashExifChecker(),
                        new int[]{FlashExifChecker.INDEX_ON, FlashExifChecker.INDEX_AUTO})
                .repeatEnd()
                .run();
    }

    @Test
    @StabilityTest
    public void testCheckStrobeAfterPauseResumeStress() {
        new MetaCase("TC_Camera_Strobe_0016")
                .addOperator(new FlashOperator(), FlashOperator.INDEX_AUTO)
                .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_AUTO)
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_AUTO)
                .repeatEnd()
                .run();
    }

    /**
     * Test flashlight should be disable when open camera application.
     */
    @Test
    @FunctionTest
    public void testFlashlightDisableWhenOpenCamera() {
        new MetaCase("Camera_743")
                //List camera in recent app list
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                //Open flashlight
                .addOperator(new FlashlightOperator(), FlashlightOperator.INDEX_ENABLE)
                //Check flashlight be off when open camera by app list
                .observeBegin(new FlashlightObserver(), FlashlightObserver.INDEX_OFF)
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_NORMAL)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testCaptureInEveryStrobeMode() {
        new MetaCase("STROBE_001")
                .addOperator(new FlashOperator())
                .addChecker(new FlashQuickSwitchChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new FlashExifChecker())
                .run();
        new MetaCase("STROBE_001")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                        .INDEX_VIDEO)
                .acrossBegin()
                .observeBegin(new FlashLogObserver().initEnv(null, true))
                .addOperator(new FlashOperator())
                .observeEnd()
                .addChecker(new FlashQuickSwitchChecker())
                .observeBegin(new VideoSavedObserver())
                .observeBegin(new FlashLogObserver().initEnv(null, true))
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .observeEnd()
                .acrossEnd()
                .run();
    }

    private boolean isPanelFlashSupported() {
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
                return false;
            case API2:
                CameraCharacteristics characteristics =
                        TestContext.mBackCameraCharacteristics;
                boolean isSupported = false;
                if (characteristics == null) {
                    LogHelper.e(TAG, "isPanelFlashSupported characteristics is null");
                    isSupported = false;
                }
                if (TestContext.mLatestCameraFacing == INDEX_BACK) {
                    isSupported = false;
                } else {
                    characteristics = TestContext.mFrontCameraCharacteristics;
                    isSupported = isExternalFlashSupported(characteristics);
                }
                LogHelper.d(TAG, "isPanelFlashSupported isSupported " + isSupported);
                return isSupported;
            default:
                return false;
        }
    }

    private boolean isExternalFlashSupported(CameraCharacteristics characteristics) {
        boolean isSupported = false;
        int[] availableAeModes = characteristics.get(
                CameraCharacteristics.CONTROL_AE_AVAILABLE_MODES);
        if (availableAeModes == null) {
            return false;
        }
        loop:
        for (int mode : availableAeModes) {
            switch (mode) {
                case CONTROL_AE_MODE_ON_EXTERNAL_FLASH:
                    isSupported = true;
                    break loop;
                default:
                    break;
            }
        }
        return isSupported;
    }

}
