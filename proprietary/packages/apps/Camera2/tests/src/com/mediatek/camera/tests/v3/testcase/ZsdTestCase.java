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

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.ZsdTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.SettingSwitchOnOffChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ZsdModeRestrictionChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.ZsdOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Zsd Test Case.
 */

@CameraBasicTest
@ZsdTest
public class ZsdTestCase extends BaseCameraTestCase {

    @Test
    @FunctionTest
    public void testZsdModeRestrictionWithBackCamera() {
        new MetaCase("TC_Camera_ZSD_0001")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addChecker(new ZsdModeRestrictionChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testZsdModeRestrictionWithFrontCamera() {
        new MetaCase("TC_Camera_ZSD_0001, TC_Camera_ZSD_0008")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .run();
        new MetaCase("TC_Camera_ZSD_0001, TC_Camera_ZSD_0008")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addChecker(new ZsdModeRestrictionChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testZsdModeRestrictionWithIntentPhoto() {
        new MetaCase("TC_Camera_ZSD_0011")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new SettingItemExistedChecker("ZSD", true),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testZsdCaptureWithNormalMode() {
        new MetaCase("TC_Camera_ZSD_0002")
                .addOperator(new SwitchCameraOperator())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    public void testZsdCaptureWithPipMode() {
        new MetaCase("TC_Camera_ZSD_0002")
                .addOperator(new SwitchToPipPhotoModeOperator())

                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @StabilityTest
    public void testZsdSwitchStability() {
        new MetaCase("TC_Camera_ZSD_0003")
                .addChecker(new PreviewChecker())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .addChecker(new PreviewChecker())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testNonZsdCaptureWithNormalMode() {
        new MetaCase("TC_Camera_ZSD_0004")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .addChecker(new PreviewChecker())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    public void testNonZsdCaptureWithPipMode() {
        new MetaCase("TC_Camera_ZSD_0004")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .addChecker(new PreviewChecker())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @StabilityTest
    public void testZsdCaptureStability() {
        new MetaCase("TC_Camera_ZSD_0005")
                .addChecker(new PreviewChecker())
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .addChecker(new PreviewChecker())

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testZsdStatusWithPauseResume() {
        new MetaCase("TC_Camera_ZSD_0009")
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new SettingSwitchOnOffChecker("ZSD"),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_ON)

                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new SettingSwitchOnOffChecker("ZSD"),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_OFF)

                .run();
    }

    @Test
    @FunctionTest
    public void testZsdStatusWithExit() {
        new MetaCase("TC_Camera_ZSD_0009")
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .addOperator(new ExitCameraOperator())
                .addOperator(new LaunchCameraOperator())
                .addChecker(new SettingSwitchOnOffChecker("ZSD"),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_ON)

                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)
                .addOperator(new ExitCameraOperator())
                .addOperator(new LaunchCameraOperator())
                .addChecker(new SettingSwitchOnOffChecker("ZSD"),
                        SettingSwitchOnOffChecker.INDEX_SWITCH_OFF)

                .run();
    }

    @Test
    @FunctionTest
    public void testZsdWithDifferentPreviewSize() {
        new MetaCase()
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .acrossBegin()
                .addOperator(new PictureSizeOperator(), new int[]{0, 1})
                .acrossEnd()
                .repeatBegin(10)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .repeatEnd()
                .run();
    }
}
