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
import android.util.Range;
import android.util.Rational;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.EvTest;
import com.mediatek.camera.tests.v3.annotation.module.ThirdPartyLaunchPhotoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.EvUiChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.ExposureCompensationObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.EvUiOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.IntentReviewUIOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.OnSingleTapUpOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
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
 * Test exposure ui and flow works normally.Can not test the brightness of the preview when ev
 * changed.
 */
@CameraFeatureTest
@EvTest
public class EvTestCase extends BaseCameraTestCase {

    /**
     * Test EV UI should be shown normally after press power key to lock and unlock camera.
     */
    @Test
    @FunctionTest
    public void testUiAfterPauseResumeByPowerKey() {
        ExposureCompensationObserver observer = new ExposureCompensationObserver(0);
        observer.initEnv(null, true);
        new MetaCase("TC_Camera_EV_0001")
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())
                .observeBegin(observer)
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())
                .run();
    }

    /**
     * Test EV supported in every modes,EV seek bar should be shown when supported and
     * hidden otherwise.
     * Test no error happens when update ev when touch focus and ev supported.
     */
    @Test
    @FunctionTest
    public void testSupportStatusInEveryMode() {
        ExposureCompensationObserver observer = new ExposureCompensationObserver(0);
        observer.initEnv(null, true);
        ExposureCompensationObserver api2Oserver = new ExposureCompensationObserver(0);
        api2Oserver.initEnv(CameraDeviceManagerFactory.CameraApi.API2, true);
        new MetaCase("TC_Camera_EV_0002,TC_Camera_EV_0003,TC_Camera_EV_0006")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator())
                .observeBegin(observer)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .observeEnd()
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 10)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())

                .observeBegin(observer)
                .addOperator(new SwitchToNormalVideoModeOperator())
                .observeEnd()
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())

                .observeBegin(observer)
                .addOperator(new SwitchToPanoramaModeOperator())
                .observeEnd()
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 20)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())

                .observeBegin(observer)
                .addOperator(new SwitchToPipPhotoModeOperator())
                .observeEnd()
                .addOperator(new OnSingleTapUpOperator().ignoreBothPageCheck(), 25)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_NOT_EXIST)

                .observeBegin(observer)
                .addOperator(new SwitchToPipVideoModeOperator())
                .observeEnd()
                .addOperator(new OnSingleTapUpOperator().ignoreBothPageCheck(), 30)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_NOT_EXIST)

                .observeBegin(api2Oserver)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .observeEnd()
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 35)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())

                .observeBegin(observer)
                .addOperator(new SwitchToStereoModeOperator())
                .observeEnd()
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 30)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())

                .observeBegin(observer)
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .observeEnd()
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 25)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())

                .observeBegin(observer)
                .addOperator(new SwitchToStereoVideoModeOperator())
                .observeEnd()
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 20)
                .addChecker(new EvUiChecker().ignoreBeforePageCheck(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())
                .run();
    }

    /**
     * Test EV works normally both in front and back camera.
     */
    @Test
    @FunctionTest
    public void testEvInBackAndFrontCamera() {
        if (Utils.isFeatureSupported(
                TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator.INDEX_BACK])) {
            new MetaCase("TC_Camera_EV_0004")
                    .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                    .addOperator(new OnSingleTapUpOperator(), 15)
                    .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                    .addOperator(new EvUiOperator())
                    .run();
        } else {
            new MetaCase("TC_Camera_EV_0004")
                    .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                    .addOperator(new OnSingleTapUpOperator(), 15)
                    .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)
                    .run();
        }

        if (Utils.isFeatureSupported(
                TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator.INDEX_FRONT])) {
            new MetaCase("TC_Camera_EV_0004")
                    .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                    .addOperator(new OnSingleTapUpOperator(), 15)
                    .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                    .addOperator(new EvUiOperator())
                    .run();
        } else {
            new MetaCase("TC_Camera_EV_0004")
                    .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                    .addOperator(new OnSingleTapUpOperator(), 15)
                    .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)
                    .run();
        }
    }

    /**
     * Test EV should be restore to 0 when onSingleTapUp.
     */
    @Test
    @FunctionTest
    public void testEvRestoreToZeroBySingleTapUp() {
        ExposureCompensationObserver observer = new ExposureCompensationObserver(0);
        observer.initEnv(null, true);
        new MetaCase("TC_Camera_EV_0005")
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator(), EvUiOperator.INDEX_UP)
                .observeBegin(observer)
                .addOperator(new TouchFocusOperator(), 30)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .observeEnd()
                .run();
    }

    /**
     * Test EV is 0 and works normally when back from gallery.
     */
    @Test
    @FunctionTest
    public void testEvBackFromGallery() {
        ExposureCompensationObserver observer = new ExposureCompensationObserver(0);
        observer.initEnv(null, true);
        new MetaCase("TC_Camera_EV_0008")
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator(), EvUiOperator.INDEX_UP)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .observeBegin(observer)
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()

                .addOperator(new TouchFocusOperator())
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator())
                .run();
    }

    /**
     * Test can take picture with different EV.
     */
    @Test
    @FunctionTest
    public void testPictureWithDiffEv() {
        new MetaCase("TC_Camera_EV_0015")
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator(), EvUiOperator.INDEX_UP)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()

                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator(), EvUiOperator.INDEX_DOWN)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()


                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator(), EvUiOperator.INDEX_DOWN_UP)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
    }

    /**
     * Open camera by MMS,test no error happens when update EV and capture.
     */
    @Test
    @FunctionTest
    @ThirdPartyLaunchPhotoTest
    public void testEvRestoreAfterCaptureInThirdParty() {
        new MetaCase()
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_EXIST)
                .addOperator(new EvUiOperator(), EvUiOperator.INDEX_DOWN)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addOperator(new IntentReviewUIOperator(), IntentReviewUIOperator.INDEX_SAVE)
                .run();
    }

    /**
     * Test EV UI and value should be right when change different scene modes.
     */
    @Test
    @FunctionTest
    public void testEvWithDiffSceneMode() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        int evIndex = 1;
        if (CameraDeviceManagerFactory.CameraApi.API1
                .equals(CameraApiHelper.getCameraApiType(null)) && TestContext
                .mBackCameraParameters != null) {
            evIndex = (int) (evIndex / TestContext.mBackCameraParameters
                    .getExposureCompensationStep());
        }
        if (CameraDeviceManagerFactory.CameraApi.API2
                .equals(CameraApiHelper.getCameraApiType(null)) && TestContext
                .mBackCameraCharacteristics != null) {
            evIndex = (int) (evIndex / getExposureCompensationStep(TestContext
                    .mBackCameraCharacteristics));
        }
        new MetaCase("TC_Camera_EV_0009")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .acrossBegin()
                .addOperator(new SceneModeOperator(), new int[]{
                        SceneModeOperator.INDEX_NIGHT,
                        SceneModeOperator.INDEX_SUNSET,
                        SceneModeOperator.INDEX_PARTY,
                        SceneModeOperator.INDEX_PORTRAIT,
                        SceneModeOperator.INDEX_LANDSCAPE,
                        SceneModeOperator.INDEX_NIGHT_PORTRAIT,
                        SceneModeOperator.INDEX_THEATRE,
                        SceneModeOperator.INDEX_STEADY_PHOTO,
                        SceneModeOperator.INDEX_FIREWORKS,
                        SceneModeOperator.INDEX_SPORTS,
                        SceneModeOperator.INDEX_CANDLE_LIGHTS
                })
                .acrossEnd()
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)
                .run();

        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .observeBegin(new ExposureCompensationObserver(evIndex))
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_BEACH)
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)
                .run();

        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .observeBegin(new ExposureCompensationObserver(evIndex))
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_SNOW)
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addOperator(new OnLongPressOperator(), 15)
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)
                .run();
    }

    private float getExposureCompensationStep(CameraCharacteristics characteristics) {
        if (!isExposureCompensationSupported(characteristics)) {
            return -1.0f;
        }
        Rational compensationStep = characteristics.get(
                CameraCharacteristics.CONTROL_AE_COMPENSATION_STEP);
        return (float) compensationStep.getNumerator() / compensationStep.getDenominator();
    }

    private boolean isExposureCompensationSupported(CameraCharacteristics characteristics) {
        Range<Integer> compensationRange =
                characteristics.get(CameraCharacteristics.CONTROL_AE_COMPENSATION_RANGE);
        return compensationRange.getLower() != 0 || compensationRange.getUpper() != 0;
    }

}
