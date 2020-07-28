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
import com.mediatek.camera.tests.v3.annotation.module.ContinuousFocusTest;
import com.mediatek.camera.tests.v3.annotation.module.PipVideoTest;
import com.mediatek.camera.tests.v3.annotation.module.TouchFocusTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.FocusUiChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.FocusModeChangeObserver;
import com.mediatek.camera.tests.v3.observer.FocusSoundObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.TouchFocusStateObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SleepOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.TakeVideoSnapShotOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.operator.ZsdOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Test focus state is right.
 */
@CameraFeatureTest
public class FocusTestCase extends BaseCameraTestCase {

    /**
     * Check touch focus state and UI is right.
     * Test focus mode will change to continuous mode 3s later after touch focus done.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testTouchFocusStateAndUi() {
        TouchFocusStateObserver observerInVideoMode = new TouchFocusStateObserver();
        observerInVideoMode.initEnv(null, false, true);
        //4s used wait for mode change log.
        int delayTime = 4;
        new MetaCase("TC_Camera_Focus_0001/0002/0007")
                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                //2s later,focus UI should not dismiss
                .addOperator(new SleepOperator(2))
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .run();
        new MetaCase("TC_Camera_Focus_0001/0002/0007")
                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator(), 25)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                //3s later,focus mode should change to continuous mode.
                .observeBegin(new FocusModeChangeObserver(),
                        FocusModeChangeObserver.INDEX_CONTINUOUS_PICTURE)
                .addOperator(new SleepOperator(delayTime))
                .run();

        new MetaCase("TC_Camera_Focus_0001/0002/0007")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .observeBegin(observerInVideoMode, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator(), 30)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                //3s later,focus mode should change to continuous mode.
                .observeBegin(new FocusModeChangeObserver(),
                        FocusModeChangeObserver.INDEX_CONTINUOUS_VIDEO)
                .addOperator(new SleepOperator(delayTime))
                .run();
    }

    /**
     * Check can do touch focus after take picture.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testTouchFocusAfterTakePic() {
        Operator touchFocusOperator = new TouchFocusOperator();
        new MetaCase("TC_Camera_Focus_0028")
                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(touchFocusOperator, 4)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addOperator(touchFocusOperator, 8)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .run();
    }

    /**
     * Step1:Open camera and do TAF.
     * Step2:Slide to video mode and do TAF.
     * Step3:Press home key and open camera again,do TAF.
     * Step4:Slide to photo mode and do TAF.
     * Check TAF should work normally when press home and open camera again.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testTouchFocusValidWhenOpenCameraAgain() {
        Operator touchFocusOperator = new TouchFocusOperator();
        new MetaCase("TC_Camera_Focus_0030")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(touchFocusOperator, 15)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(touchFocusOperator, 25)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(2);
    }

    /**
     * Check can take picture successfully after TAF with zsd on.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testTakePicAfterTouchFocusWithZsdOn() {
        Operator touchFocusOperator = new TouchFocusOperator();
        int indexZsdOn = 0;
        int takePhotoCount = 10;
        new MetaCase("TC_Camera_Focus_0024")
                .addOperator(new ZsdOperator(), indexZsdOn)
                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(touchFocusOperator, 8)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .repeatBegin(takePhotoCount)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .repeatEnd()
                .run();
    }

    /**
     * Test no error happens pause and resume camera after touch focus once.
     */
    @Test
    @StabilityTest
    @TouchFocusTest
    public void testStressPauseResumeAfterTaf() {
        Operator touchFocusOperator = new TouchFocusOperator();
        new MetaCase("TC_Camera_Focus_0005")
                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .acrossBegin()
                .addOperator(touchFocusOperator)
                .acrossEnd()
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .addChecker(new PreviewChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .repeatEnd()
                .run();
    }

    /**
     * Test no focus sound played when switch between photo mode and video mode.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testNoFocusSoundWhenModeSwitch() {
        new MetaCase("TC_Camera_Focus_0008")
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_NO_FOCUS_SOUND)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .observeEnd()
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_NO_FOCUS_SOUND)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .observeEnd()
                .run();
    }

    /**
     * Test focus sound should play in video preview and mute during video recording.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testFocusSoundInVideoMode() {
        Operator touchFocusOperator = new TouchFocusOperator();
        new MetaCase("TC_Camera_Focus_0009/0010")
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_HAS_FOCUS_SOUND)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addOperator(touchFocusOperator, 10)
                .observeEnd()
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_NO_FOCUS_SOUND)
                .addOperator(new StartRecordOperator(true))
                .addOperator(touchFocusOperator, 10)
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addChecker(new PreviewChecker())
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_HAS_FOCUS_SOUND)
                .addOperator(touchFocusOperator, 10)
                .observeEnd()
                .run();
    }

//    /**
//     * Test no focus sound and focus ui when do continuous shot with strobe on.
//     */
//    @Test
//    @FunctionTest
//    @TouchFocusTest
//    @StrobeTest
//    @ContinuousShotTest
//    public void testFocusInCsWithStrobe() {
//        new MetaCase("TC_Camera_Focus_0012")
//                .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
//                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_NO_FOCUS_SOUND)
//                .observeBegin(new FocusUiNotAppearedObserver())
//                .addOperator(new ContinuousShotOperator(5000, false).ignoreAfterPageCheck())
//                .observeEnd()
//                .observeEnd()
//                .run();
//    }

    /**
     * Test VSS can be done after touch focus during recording.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testVssAfterTafDuringRecord() {
        Operator touchFocusOperator = new TouchFocusOperator();
        new MetaCase("TC_Camera_Focus_0015")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new StartRecordOperator(true))
                .addOperator(touchFocusOperator, 10)
                .addOperator(new TakeVideoSnapShotOperator())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
    }

    /**
     * Test no focus UI shown during capture.
     */
    @Test
    @FunctionTest
    @ContinuousFocusTest
    public void testNoFocusUiDuringCapture() {
        new MetaCase("TC_Camera_Focus_0011")
                .addOperator(new CapturePhotoOperator())
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_NO_AF_UI)
                .run();
    }

    /**
     * Test no focus UI during pip recording.
     */
    @Test
    @FunctionTest
    @ContinuousFocusTest
    @PipVideoTest
    public void testNoFocusUiDuringPipRecord() {
        new MetaCase("TC_Camera_Focus_0016")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addOperator(new RecordVideoOperator())
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_NO_AF_UI)
                .run();
    }

    /**
     * Test no error happens to capture 3000 times after touch focus.
     */
    @Test
    @StabilityTest
    @TouchFocusTest
    public void testStressPictureAfterTaf() {
        Operator touchFocusOperator = new TouchFocusOperator();
        new MetaCase("TC_Camera_Focus_0022")
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(touchFocusOperator, 20)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .repeatEnd()
                .run();
    }

    /**
     * Test no error and focus works normally after zoom in and zoom out and switch camera again.
     */
    @Test
    @StabilityTest
    @TouchFocusTest
    public void testFocusWithZoomAndSwitchCamera() {
        new MetaCase("TC_Camera_Focus_0013")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                //touch focus ui,state and sound is right in slow motion preview
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_HAS_FOCUS_SOUND)
                .observeBegin(new TouchFocusStateObserver())
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(), FocusUiChecker
                        .INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeEnd()
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test no error and focus works normally after zoom in and zoom out and switch camera again.
     */
    @Test
    @StabilityTest
    @TouchFocusTest
    public void testFocusWithZoomDuringPipRecord() {
        TouchFocusStateObserver touchFocusStateObserver = new TouchFocusStateObserver();
        touchFocusStateObserver.initEnv(null, false, true);
        new MetaCase("TC_Camera_Focus_0014")
                //switch to pip video and start recording
                .addOperator(new SwitchToPipVideoModeOperator())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                //zoom in and zoom out
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                //touch focus ui,state and sound is right in slow motion preview
                .observeBegin(touchFocusStateObserver, TouchFocusStateObserver.INDEX_BACK)
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_NO_FOCUS_SOUND)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(), FocusUiChecker
                        .INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeEnd()
                //Press power key to lock and unlock screen
                .addOperator(new PauseResumeByPowerKeyOperator())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test focus works normally in slow motion.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testFocusInSlowMotion() {
        TouchFocusStateObserver observerDuringRecording = new TouchFocusStateObserver();
        //force set API level to API2 before slow motion test case.
        observerDuringRecording.initEnv(CameraDeviceManagerFactory.CameraApi.API2, false, false);
        new MetaCase("TC_Camera_Focus_0017")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                //touch focus ui,state and sound is right during slow motion recording
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_NO_FOCUS_SOUND)
                .observeBegin(observerDuringRecording, TouchFocusOperator.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(), FocusUiChecker
                        .INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeEnd()
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                //touch focus ui,state and sound is right in slow motion preview
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_HAS_FOCUS_SOUND)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 25)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .run();
    }

    /**
     * Test focus works normally when zsd on and back from galley.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testFocusAndBackFromGalleryWithZsdOn() {
        int indexZsdOn = 0;
        new MetaCase("TC_Camera_Focus_0029")
                .addOperator(new ZsdOperator(), indexZsdOn)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                //touch focus ui,state and sound is right in slow motion preview
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_HAS_FOCUS_SOUND)
                .observeBegin(new TouchFocusStateObserver(), TouchFocusOperator.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeEnd()
                .run();
    }

    /**
     * Test focus works normally in photo mode which launched by third party.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testFocusInThirdPartyPhotoMode() {
        new MetaCase("TC_Camera_Focus_0023")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                //touch focus ui,state and sound is right in slow motion preview
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_HAS_FOCUS_SOUND)
                .observeBegin(new TouchFocusStateObserver(), TouchFocusOperator.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeEnd()
                .run();
    }

    /**
     * Test focus works normally in video mode which launched by third party.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testFocusInThirdPartyVideoMode() {
        TouchFocusStateObserver touchFocusStateObserver = new TouchFocusStateObserver();
        new MetaCase("TC_Camera_Focus_0023")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_VIDEO)
                .addChecker(new PreviewChecker())
                //touch focus ui,state and sound is right in slow motion preview
                .observeBegin(new FocusSoundObserver(), FocusSoundObserver.INDEX_HAS_FOCUS_SOUND)
                .observeBegin(touchFocusStateObserver, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeEnd()
                .run();
    }

    /**
     * Test touch focus works normally in every capture mode.
     */
    @Test
    @FunctionTest
    @TouchFocusTest
    public void testTouchFocusWithDifferentCaptureMode() {
        TouchFocusStateObserver observer = new TouchFocusStateObserver();
        observer.initEnv(null, true, false);
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .run();

        new MetaCase()
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addChecker(new PreviewChecker())
                .observeBegin(observer, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .run();

        new MetaCase()
                .addOperator(new ExitCameraOperator())
                .addOperator(new LaunchCameraOperator())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .run();

        new MetaCase()
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))

                .addChecker(new PreviewChecker())
                .observeBegin(observer, TouchFocusStateObserver.INDEX_FRONT)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .run();
    }
}
