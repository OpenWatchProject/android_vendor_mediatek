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

import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.PictureSizeTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CapturedPhotoPictureSizeChecker;
import com.mediatek.camera.tests.v3.checker.PictureSizeAsyncChecker;
import com.mediatek.camera.tests.v3.checker.PictureSizeExistedChecker;
import com.mediatek.camera.tests.v3.checker.PictureSizeOptionsChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeChooseOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.SettingRadioChooseOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;

import org.junit.Test;

/**
 * Picture size test case.
 */

@CameraFeatureTest
@PictureSizeTest
public class PictureSizeTestCase extends BaseCameraTestCase {

    /**
     * Test picture size exited in each mode.
     */
    @Test
    @FunctionTest
    public void testPictureSizeExistedOrNotInEachMode() {
        new MetaCase("TC_Camera_Picture_Size_0001")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PictureSizeExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_Picture_Size_0002")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PictureSizeExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Picture_Size_0002")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PictureSizeExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Picture_Size_0002")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PictureSizeExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_Picture_Size_0002")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PictureSizeExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Picture_Size_0002")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PictureSizeExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testPictureSizeOptions() {
        new MetaCase("TC_Camera_Picture_Size_0003, TC_Camera_Picture_Size_0004," +
                " TC_Camera_Picture_Size_0006")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new PictureSizeOptionsChecker())
                .run();

        new MetaCase("TC_Camera_Picture_Size_0009")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PictureSizeOptionsChecker())
                .run();
    }


    /**
     * Test relaunch camera, picture size value can be remembered.
     */
    @Test
    @FunctionTest
    public void testRememberPictureSizeSettingAfterRelaunch() {
        new MetaCase("TC_Camera_Picture_Size_0005")
                .addOperator(new PictureSizeOperator())
                .addOperator(new PauseResumeByBackKeyOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new CapturedPhotoPictureSizeChecker())
                .run();
    }

    /**
     * Test switch camera with different picture size.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraWithDifferentPictureSize() {
        new MetaCase("TC_Camera_Picture_Size_0010")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .run();

        new MetaCase("TC_Camera_Picture_Size_0010")
                .addOperator(new PictureSizeChooseOperator(),
                        SettingRadioChooseOperator.INDEX_FIRST)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testPictureSizeAsyncBetweenCamera() {
        new MetaCase("TC_Camera_Picture_Size_0011")
                .addChecker(new PictureSizeAsyncChecker())
                .run();
    }

    /**
     * Test pause/resume camera, anti flicker value can be remembered.
     */
    @Test
    @FunctionTest
    public void testRememberPictureSizeSettingAfterPauseResumeByHomeKey() {
        new MetaCase("TC_Camera_Picture_Size_0012")
                .addOperator(new PictureSizeOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new CapturedPhotoPictureSizeChecker())
                .run();
    }

    /**
     * Test supported status in 3rd app.
     */
    @Test
    @FunctionTest
    public void testSupportedStatusIn3rd() {
        new MetaCase("TC_Camera_Picture_Size_0013")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                .addChecker(new PictureSizeExistedChecker(true),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();
    }
}
