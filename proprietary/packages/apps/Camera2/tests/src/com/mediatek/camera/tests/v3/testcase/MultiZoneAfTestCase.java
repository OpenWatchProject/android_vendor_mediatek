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


import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.MultiZoomAfTest;
import com.mediatek.camera.tests.v3.annotation.module.ThirdPartyLaunchPhotoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.FocusModeSettingValueChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ContinuousShotOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.FocusModeOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.ZsdOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Test multi-zone ui and flow works normally.
 */
@CameraFeatureTest
@MultiZoomAfTest
public class MultiZoneAfTestCase extends BaseCameraTestCase {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            MultiZoneAfTestCase.class.getSimpleName());
    private static final String ZSD_DEFALUTZ_ON_TAG = "com.mediatek.camera.at.zsd-default-on";
    private static final int mRepeatTime = 2;

    /**
     * Test Multi-zone AF setting should be shown when multi-zone af is supported and no error
     * happens when switch between single point af and multi zone af several times.
     */
    @Test
    @FunctionTest
    public void testFocusModeSwitch() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase()
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_00014")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                // TODO: check multi-zone af UI when back to preview later.
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_SINGLE_POINT_AF)
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_MZAF_0001")
                .addOperator(new FocusModeOperator())
                .runForTimes(mRepeatTime);
    }

    /**
     * Test multi-zone af ui shows normally after do touch focus and ContinuousShot capture
     * several times.
     */
    @Test
    @FunctionTest
    public void testMultiZoneAfUiAfterCsCapture() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase("TC_Camera_MZAF_0003")
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_0003")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .run();
        new MetaCase("TC_Camera_MZAF_0003")
                .addOperator(new TouchFocusOperator(), 15)
                .addOperator(new ContinuousShotOperator())
                // TODO: check multi-zone af ui should shown normally after CS capture.
                .addChecker(new PreviewChecker())
                .runForTimes(mRepeatTime);
    }

    /**
     * Test multi-zone af ui shows normally after do touch focus and ContinuousShot capture
     * several times.
     */
    @Test
    @FunctionTest
    public void testUiHiddenDuringCapture() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase("TC_Camera_MZAF_0004")
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_0004")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                .addOperator(new TouchFocusOperator(), 15)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                // TODO: check multi-zone af ui should not shown when doing capture.
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test multi-zone af mode should be the same after pause and resume camera several times.
     */
    @Test
    @FunctionTest
    public void testAfModeAfterPauseResume() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase("TC_Camera_MZAF_0005")
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_0005")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.MULTI_ZONE_AF))
                .runForTimes(mRepeatTime);
        new MetaCase("TC_Camera_MZAF_0005")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_SINGLE_POINT_AF)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.SINGLE_POINT_AF))
                .runForTimes(mRepeatTime);
    }

    /**
     * Test multi-zone af mode should be the same after exit and open camera again several times.
     */
    @Test
    @FunctionTest
    public void testAfModeAfterExistAndOpenCamera() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase("TC_Camera_MZAF_0006")
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_0006")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.MULTI_ZONE_AF))
                .runForTimes(mRepeatTime);
        new MetaCase("TC_Camera_MZAF_0006")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_SINGLE_POINT_AF)
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.SINGLE_POINT_AF))
                .runForTimes(mRepeatTime);
    }

    /**
     * Test multi-zone af mode should supported and work normally when camera is launched by third
     * party application.
     */
    @Test
    @FunctionTest
    @ThirdPartyLaunchPhotoTest
    public void testAfModeInThirdParty() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase("TC_Camera_MZAF_0007")
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_0007")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                // TODO: check multi-zone af UI when back to preview later.
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.MULTI_ZONE_AF))
                // TODO: check multi-zone af UI when back to preview later.
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_SINGLE_POINT_AF)
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.SINGLE_POINT_AF))
                .run();
        new MetaCase("TC_Camera_MZAF_0007")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_VIDEO)
                .addChecker(new PreviewChecker())
                // TODO: check multi-zone af UI when back to preview later.
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.MULTI_ZONE_AF))
                // TODO: check multi-zone af UI when back to preview later.
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_SINGLE_POINT_AF)
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.SINGLE_POINT_AF))
                .run();
    }

    /**
     * Test multi-zone af works normally after touch focus when scene changes.
     */
    @Test
    @FunctionTest
    public void testCafAfterTaf() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase("TC_Camera_MZAF_0012")
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_0012")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.MULTI_ZONE_AF))
                .addOperator(new TouchFocusOperator())
                // TODO: check multi-zone af UI when back to preview later.
                .run();

    }

    /**
     * Test multi-zone af works normally in video mode.
     */
    @Test
    @FunctionTest
    public void testMultiZoneAfInVideoMode() {
        if (!isMultiZoneAfSupported(0)) {
            return;
        }
        if (!Utils.isFeatureSupported(ZSD_DEFALUTZ_ON_TAG)) {
            new MetaCase("TC_Camera_MZAF_0013")
                    .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                    .run();
        }
        new MetaCase("TC_Camera_MZAF_0013")
                .addOperator(new FocusModeOperator(), FocusModeOperator.INDEX_MULTI_ZONE_AF)
                .addChecker(new FocusModeSettingValueChecker(FocusModeOperator.MULTI_ZONE_AF))
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                // TODO: check multi-zone af UI when back to preview later.
                .run();

    }


    /**
     * Multi-zone af should be test when multi-zone af supported and zsd on.
     *
     * @param cameraId The current camera id,
     * @return Whether multi-zone af can be tested.
     */
    private boolean isMultiZoneAfSupported(int cameraId) {
        LogHelper.d(TAG, "isMultiZoneAfSupported " + (Utils.isFeatureSupported(FocusModeOperator
                .OPTIONS_TAG[cameraId]) &&
                new ZsdOperator().isSupported(cameraId)));
        return Utils.isFeatureSupported(FocusModeOperator.OPTIONS_TAG[cameraId]) &&
                new ZsdOperator().isSupported(cameraId);
    }


}
