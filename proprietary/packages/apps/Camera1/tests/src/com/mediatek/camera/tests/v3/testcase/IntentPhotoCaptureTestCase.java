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
import com.mediatek.camera.tests.v3.annotation.module.ThirdPartyLaunchPhotoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CameraFacingChecker;
import com.mediatek.camera.tests.v3.checker.IntentReviewUIChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ContinuousShotOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.FontSizeOperator;
import com.mediatek.camera.tests.v3.operator.IntentReviewUIOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.VolumeKeyDownOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Third party launch photo mode test case.
 */
@CameraBasicTest
@ThirdPartyLaunchPhotoTest
public class IntentPhotoCaptureTestCase extends BaseIntentPhotoTestCase {

    /**
     * Test third party launch and capture with back and front camera.
     */
    @Test
    @FunctionTest
    public void testCapture() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0011")
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_BACK)
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_FRONT)
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())
                .run();
    }

    /**
     * Test enter and exit function.
     */
    @Test
    @FunctionTest
    public void testEntryAndExit() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0001")
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator())
                .run();
    }

    /**
     * Test Entry and exit stability.
     */
    @Test
    @StabilityTest
    public void testEntryAndExitStability() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0002")
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Change the system font size to huge and test capture function.
     */
    @Test
    @FunctionTest
    public void testCaptureWithHugeFontSize() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0006")
                .addOperator(new FontSizeOperator(), FontSizeOperator.INDEX_HUGE).run();

        new MetaCase("TC_Camera_3rd party Launch Photo_0006")
                .addChecker(new PreviewChecker())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())
                .run();
    }

    /**
     * Test touch focus when preview is ready.
     */
    @Test
    @FunctionTest
    public void testTouchFocus() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0009")
                .addChecker(new PreviewChecker())
                .addOperator(new TouchFocusOperator())
                .run();
    }

    /**
     * Test long press shutter to capture more than one picture.
     */
    @Test
    @FunctionTest
    public void testLongPressShutter() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0012")
                .addChecker(new PreviewChecker())
                .addOperator(new ContinuousShotOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test zoom in and zoom out.
     */
    @Test
    @FunctionTest
    public void testZoom() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0013")
                .addChecker(new PreviewChecker())

                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()

                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .run();
    }

    /**
     * Test capture when zoom.
     */
    @Test
    @FunctionTest
    public void testCaptureWhenZoom() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0014")
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()

                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())

                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)

                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_FRONT)

                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()

                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())
                .run();
    }

    /**
     * Test capture with different picture size.
     */
    @Test
    @FunctionTest
    public void testBackCameraCaptureWithDifferentSize() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0019")

                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .acrossBegin()

                .addOperator(new PictureSizeOperator())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker(), IntentReviewUIChecker.INDEX_PHOTO)

                .addOperator(new IntentReviewUIOperator(), IntentReviewUIOperator.INDEX_RETAKE)
                .addChecker(new PreviewChecker())
                .acrossEnd()
                .run();
    }

    /**
     * Test capture with different picture size.
     */
    @Test
    @FunctionTest
    public void testFrontCameraCaptureWithDifferentSize() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0019")
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_FRONT)
                .run();

        new MetaCase()
                .addOperator(new PictureSizeOperator())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker(), IntentReviewUIChecker.INDEX_PHOTO)

                .addOperator(new IntentReviewUIOperator(), IntentReviewUIOperator.INDEX_RETAKE)
                .addChecker(new PreviewChecker())
                .run();
    }


    /**
     * Test switch camera.
     */
    @Test
    @FunctionTest
    public void testSwitchCamera() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0026")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test switch camera stability.
     */
    @Test
    @StabilityTest
    public void testSwitchCameraStability() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0027")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test touch focus and capture.
     */
    @Test
    @FunctionTest
    public void testCaptureWithTouchFocus() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0028")
                .addChecker(new PreviewChecker())
                .addOperator(new TouchFocusOperator())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())
                .run();
    }

    /**
     * Test pause/resume with home key and power key.
     */
    @Test
    @FunctionTest
    public void testPauseAndResume() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0029")
                .addOperator(new SwitchCameraOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause/resume with recent app.
     */
    @Test
    @FunctionTest
    public void testPauseAndResumeWithRecentApp() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0029")
                .addOperator(new SwitchCameraOperator())
                .addOperator(new PauseResumeByRecentAppOperator())
                .run();
    }

    /**
     * Test pause/resume stability.
     */
    @Test
    @StabilityTest
    public void testPauseAndResumeStability() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0030")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test pause when capturing.
     */
    @Test
    @FunctionTest
    public void testPauseWhenCapturing() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0031")
                .addChecker(new PreviewChecker())
                .addOperator(new CapturePhotoOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause when switch camera.
     */
    @Test
    @FunctionTest
    public void testPauseWhenSwitchCamera() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0032")
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture with volume key.
     */
    @Test
    @FunctionTest
    public void testCaptureWithVolumeKey() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0033")
                .addChecker(new PreviewChecker())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new IntentReviewUIChecker())
                .run();
    }

//    /**
//     * Test  review ui save button.
//     */
//    @Test
//    @FunctionTest
//    public void testSaveButtonInReviewUI() {
//        new MetaCase("TC_Camera_3rd party Launch Photo_0042")
//                .addChecker(new PreviewChecker())
//                .addOperator(new CapturePhotoOperator())
//                .addChecker(new IntentReviewUIChecker())
//
//                .addOperator(new IntentReviewUIOperator(), IntentReviewUIOperator.INDEX_SAVE)
//                .addChecker(new CameraExitedChecker())
//                .run();
//    }

    /**
     * Test review ui retake button.
     */
    @Test
    @FunctionTest
    public void testRetakeButtonInReviewUI() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0043")
                .addChecker(new PreviewChecker())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_NO_SAVED)
                .addOperator(new VolumeKeyDownOperator().ignoreBothPageCheck())
                .observeEnd()
                .addOperator(new IntentReviewUIOperator(), IntentReviewUIOperator.INDEX_RETAKE)
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause and resume when show review ui.
     */
    @Test
    @FunctionTest
    public void testPauseResumeFromReviewUI() {
        new MetaCase("TC_Camera_3rd party Launch Photo_0043")
                .addChecker(new PreviewChecker())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new IntentReviewUIChecker())

                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

}
