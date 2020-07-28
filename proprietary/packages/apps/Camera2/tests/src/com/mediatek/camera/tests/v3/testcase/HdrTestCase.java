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
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.HdrTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CapturedPhotoPictureSizeChecker;
import com.mediatek.camera.tests.v3.checker.FlashQuickSwitchChecker;
import com.mediatek.camera.tests.v3.checker.HdrIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.HdrQuickSwitchChecker;
import com.mediatek.camera.tests.v3.checker.HdrQuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.checker.IndicatorChecker;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayIconChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.QuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.checker.QuickSwitchOptionChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.checker.VideoFileSizeChecker;
import com.mediatek.camera.tests.v3.observer.HdrLogObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.DngOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.OnSingleTapUpOperator;
import com.mediatek.camera.tests.v3.operator.OpenHdrQuickSwitchOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.QualityOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SleepOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.operator.ZsdOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Test case used to check hdr parameters and values.
 */
@CameraFeatureTest
@HdrTest
public class HdrTestCase extends BaseCameraTestCase {
    private static final LogUtil.Tag TAG = Utils.getTestTag(HdrTestCase.class.getSimpleName());

    /**
     * Check photo can be taken successfully with hdr on, zsd on,zoom to 4x and screen size
     * between full screen size and standard size.
     */
    @Test
    @FunctionTest
    public void testCapturePhoto() {
        // when support de-noise, zsd will be set as on dy default, there is not zsd item in
        // setting, so not add ZsdOperator when de-noise
        if (!DenoiseTestCase.isDenoiseSupported()) {
            if (CameraApiHelper.getCameraApiType(null)
                    == CameraDeviceManagerFactory.CameraApi.API1) {
                new MetaCase("TC_Camera_HDR_0018/0034")
                        .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                        // API1, if hdr on, zsd is supported
                        .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                        .acrossBegin()
                        .addOperator(new PictureSizeOperator(), new int[]{0, 1})
                        .acrossEnd()
                        .addChecker(new PreviewChecker())
                        .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                        .observeBegin(new PhotoSavedObserver(1))
                        .addOperator(new CapturePhotoOperator())
                        .observeEnd()
                        .addChecker(new PreviewChecker())
                        .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                        .addChecker(new CapturedPhotoPictureSizeChecker())
                        .run();
            } else {
                new MetaCase("TC_Camera_HDR_0018/0034")
                        .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                        .acrossBegin()
                        .addOperator(new PictureSizeOperator(), new int[]{0, 1})
                        .acrossEnd()
                        .addChecker(new PreviewChecker())
                        .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                        .observeBegin(new PhotoSavedObserver(1))
                        .addOperator(new CapturePhotoOperator())
                        .observeEnd()
                        .addChecker(new PreviewChecker())
                        .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                        .addChecker(new CapturedPhotoPictureSizeChecker())
                        .run();
            }
        } else {
            new MetaCase("TC_Camera_HDR_0018,TC_Camera_HDR_0034")
                    .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                    .acrossBegin()
                    .addOperator(new PictureSizeOperator(), new int[]{0, 1})
                    .acrossEnd()
                    .addChecker(new PreviewChecker())
                    .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                    .observeBegin(new PhotoSavedObserver(1))
                    .addOperator(new CapturePhotoOperator())
                    .observeEnd()
                    .addChecker(new PreviewChecker())
                    .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                    .addChecker(new CapturedPhotoPictureSizeChecker())
                    .run();
        }
    }

    /**
     * Check photo can be taken successfully with hdr on/auto/off, zsd on/off and screen size
     * between full screen size and standard size.
     */
    @Test
    @FunctionTest
    public void testCapturePhotoMore() {
        // when support de-noise, zsd will be set as on dy default, there is not zsd item in
        // setting, so not add ZsdOperator when de-noise
        if (!DenoiseTestCase.isDenoiseSupported()) {
            if (CameraApiHelper.getCameraApiType(null)
                    == CameraDeviceManagerFactory.CameraApi.API1) {
                new MetaCase("TC_Camera_HDR_0018/0034")
                        .addOperator(new HdrOperator())
                        // API1, if hdr on, zsd is supported
                        .acrossBegin()
                        .addOperator(new ZsdOperator())
                        .acrossEnd()
                        .acrossBegin()
                        .addOperator(new PictureSizeOperator())
                        .acrossEnd()
                        .addChecker(new PreviewChecker())
                        .acrossBegin()
                        .addOperator(new ZoomOperator())
                        .acrossEnd()
                        .observeBegin(new PhotoSavedObserver(1))
                        .addOperator(new CapturePhotoOperator())
                        .observeEnd()
                        .addChecker(new PreviewChecker())
                        .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                        .addChecker(new CapturedPhotoPictureSizeChecker())
                        .run();
            } else {
                new MetaCase("TC_Camera_HDR_0018/0034")
                        .addOperator(new HdrOperator(),
                                new int[]{HdrOperator.INDEX_AUTO, HdrOperator.INDEX_OFF})
                        .acrossBegin()
                        .addOperator(new ZsdOperator())
                        .acrossEnd()
                        .acrossBegin()
                        .addOperator(new PictureSizeOperator())
                        .acrossEnd()
                        .addChecker(new PreviewChecker())
                        .acrossBegin()
                        .addOperator(new ZoomOperator())
                        .acrossEnd()
                        .observeBegin(new PhotoSavedObserver(1))
                        .addOperator(new CapturePhotoOperator())
                        .observeEnd()
                        .addChecker(new PreviewChecker())
                        .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                        .addChecker(new CapturedPhotoPictureSizeChecker())
                        .run();
                new MetaCase("TC_Camera_HDR_0018/0034")
                        .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                        .acrossBegin()
                        .addOperator(new PictureSizeOperator())
                        .acrossEnd()
                        .addChecker(new PreviewChecker())
                        .acrossBegin()
                        .addOperator(new ZoomOperator())
                        .acrossEnd()
                        .observeBegin(new PhotoSavedObserver(1))
                        .addOperator(new CapturePhotoOperator())
                        .observeEnd()
                        .addChecker(new PreviewChecker())
                        .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                        .addChecker(new CapturedPhotoPictureSizeChecker())
                        .run();
            }
        } else {
            new MetaCase("TC_Camera_HDR_0018,TC_Camera_HDR_0034")
                    .addOperator(new HdrOperator())
                    .acrossBegin()
                    .addOperator(new PictureSizeOperator())
                    .acrossEnd()
                    .addChecker(new PreviewChecker())
                    .acrossBegin()
                    .addOperator(new ZoomOperator())
                    .acrossEnd()
                    .observeBegin(new PhotoSavedObserver(1))
                    .addOperator(new CapturePhotoOperator())
                    .observeEnd()
                    .addChecker(new PreviewChecker())
                    .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                    .addChecker(new CapturedPhotoPictureSizeChecker())
                    .run();
        }
    }

    /**
     * Check vHDR recording size should be right for different quality.
     */
    @Test
    @FunctionTest
    public void testRecordWithDifferentVideoQuality() {
        HdrOperator hdrOperator = new HdrOperator().initHdrOptions(true);
        if (!hdrOperator.isSupported(HdrOperator.INDEX_ON)) {
            new MetaCase("TC_Camera_HDR_0022")
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_VIDEO)
                    .addChecker(new PreviewChecker())
                    .addChecker(new HdrQuickSwitchExistedChecker(),
                            HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                    .run();
            return;
        }
        new MetaCase("TC_Camera_HDR_0022")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                        .INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        QuickSwitchExistedChecker.INDEX_EXIST)
                .run();

        new MetaCase("TC_Camera_HDR_0022")
                .acrossBegin()
                .observeBegin(new HdrLogObserver())
                .addOperator(hdrOperator)
                .observeEnd()
                .acrossEnd()
                .addChecker(new HdrQuickSwitchChecker())
                //change video quality
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addChecker(new VideoFileSizeChecker())
                .run();
    }

    /**
     * Test hdr value should be remember when relaunch camera.
     */
    @Test
    @FunctionTest
    public void testRememberHdrSettingAfterRelaunch() {
        new MetaCase("TC_Camera_HDR_0031")
                .addOperator(new HdrOperator())
                .addChecker(new HdrQuickSwitchChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new HdrQuickSwitchChecker())
                .run();
    }

    /**
     * Test hdr option view should shown when click quick switch view,still shown when click the
     * center of the screen and finally hidden after capture.
     */
    @Test
    @FunctionTest
    public void testHideQuickSwitchOptionAfterCapture() {
        HdrOperator photoHdroperator = new HdrOperator().initHdrOptions(false);
        if (!photoHdroperator.isSupported(HdrOperator.INDEX_AUTO)) {
            return;
        }
        new MetaCase("TC_Camera_HDR_0017/0028")
                .addOperator(new OpenHdrQuickSwitchOperator(false))
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_SHOW)
                //check hdr option view still shown when single tap up the center of the screen
                .addOperator(new OnSingleTapUpOperator(), 20)
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_SHOW)
                .addOperator(new CapturePhotoOperator())
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_HIDE)
                .run();
    }


    /**
     * Test hdr and flash interaction.
     */
    @Test
    @FunctionTest
    public void testHdrInteractWithFlash() {
        HdrOperator photoHdrOperator = new HdrOperator().initHdrOptions(false);
        if (photoHdrOperator.isSupported(HdrOperator.INDEX_AUTO)) {
            new MetaCase("TC_Camera_HDR_0014/0015")
                    .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_AUTO)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_OFF)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)

                    .addOperator(new FlashOperator(), FlashOperator.INDEX_AUTO)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_AUTO)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_OFF)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addOperator(new ExitCameraOperator())
                    .addChecker(new CameraExitedChecker())
                    .addOperator(new LaunchCameraOperator())
                    .addChecker(new PreviewChecker())
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_OFF)
                    .run();
        }
        if (photoHdrOperator.isSupported(HdrOperator.INDEX_ON)) {
            new MetaCase("TC_Camera_HDR_0014/0015")
                    .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_ON)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_OFF)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)

                    .addOperator(new FlashOperator(), FlashOperator.INDEX_AUTO)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_ON)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addOperator(photoHdrOperator, HdrOperator.INDEX_OFF)
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addOperator(new ExitCameraOperator())
                    .addChecker(new CameraExitedChecker())
                    .addOperator(new LaunchCameraOperator())
                    .addChecker(new PreviewChecker())
                    .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_OFF)
                    .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_OFF)
                    .run();
        }
    }

    /**
     * Test  hdr option view should shown when click quick switch view and hidden when switch to
     * video mode.
     */
    @Test
    @FunctionTest
    public void testHideQuickSwitchOptionAfterSwitchToVideo() {
        HdrOperator photoHdroperator = new HdrOperator().initHdrOptions(false);
        if (!photoHdroperator.isSupported(HdrOperator.INDEX_ON)) {
            new MetaCase("TC_Camera_HDR_0023")
                    .addOperator(new OpenHdrQuickSwitchOperator(false))
                    .addChecker(new HdrQuickSwitchExistedChecker(),
                            HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                    .run();
            return;
        }
        if (!photoHdroperator.isSupported(HdrOperator.INDEX_AUTO)) {
            new MetaCase("TC_Camera_HDR_0023")
                    .addOperator(new OpenHdrQuickSwitchOperator(false))
                    .addChecker(new QuickSwitchOptionChecker(),
                            QuickSwitchOptionChecker.INDEX_HIDE)
                    .run();
            return;
        }
        new MetaCase("TC_Camera_HDR_0023")
                .addOperator(new OpenHdrQuickSwitchOperator(false))
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_SHOW)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addChecker(new QuickSwitchOptionChecker(), QuickSwitchOptionChecker.INDEX_HIDE)
                .run();
    }

    /**
     * Test  hdr indicator should be shown when hdr on and hidden when hdr off.
     */
    @Test
    @FunctionTest
    public void testHdrIndicator() {
        new MetaCase("TC_Camera_HDR_0016")
                .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                .addChecker(new HdrIndicatorChecker(), IndicatorChecker.INDEX_SHOW)

                .addOperator(new HdrOperator(), HdrOperator.INDEX_OFF)
                .addChecker(new HdrIndicatorChecker(), IndicatorChecker.INDEX_HIDE)
                .run();
    }


    /**
     * Test no error happens when choose during hdr auto,off and on in photo mode.
     */
    @Test
    @FunctionTest
    public void testHdrAutoOnOffPhoto() {
        new MetaCase("TC_Camera_HDR_0001/0003/0012/0026")
                .acrossBegin()
                .observeBegin(new HdrLogObserver())
                .addOperator(new HdrOperator())
                .observeEnd()
                .addChecker(new HdrQuickSwitchChecker())
                .acrossEnd()

                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()

                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_EXIST)
                .run();
    }

    /**
     * Test no error happens when choose during hdr auto,off and on in video mode.
     */
    @Test
    @FunctionTest
    public void testHdrAutoOnOffVideo() {
        HdrOperator videoHdroperator = new HdrOperator().initHdrOptions(true);
        if (!videoHdroperator.isSupported(HdrOperator.INDEX_ON)) {
            new MetaCase("TC_Camera_HDR_0004/0008/0009/0010/0011/0021/0023/0033")
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_VIDEO)
                    .addChecker(new PreviewChecker())
                    .addChecker(new HdrQuickSwitchExistedChecker(),
                            HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                    .run();
            return;
        }

        new MetaCase("TC_Camera_HDR_0004/0008/0009/0010/0011/0021/0023/0033")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        QuickSwitchExistedChecker.INDEX_EXIST)
                .observeBegin(new HdrLogObserver())
                .addOperator(new HdrOperator())
                .observeEnd()
                .addChecker(new HdrQuickSwitchChecker())

                .observeBegin(new VideoSavedObserver())
                .observeBegin(new HdrLogObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .observeEnd()

                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test hdr supported conditions in other modes and settings.
     */
    @Test
    @FunctionTest
    public void testSupportStatusInEveryMode() {

        new MetaCase("TC_Camera_HDR_0020")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_EXIST)
                .addChecker(new MatrixDisplayIconChecker(),
                        MatrixDisplayIconChecker.INDEX_MATRIX_SHOW)
                .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                .addChecker(new MatrixDisplayIconChecker(),
                        MatrixDisplayIconChecker.INDEX_MATRIX_HIDE)
                .addOperator(new HdrOperator(), HdrOperator.INDEX_OFF)
                .addChecker(new MatrixDisplayIconChecker(),
                        MatrixDisplayIconChecker.INDEX_MATRIX_SHOW)
                .run();

        new MetaCase("TC_Camera_HDR_0024")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToStereoModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .run();
    }


    /**
     * Test hdr value should be sync when switch between photo and video mode.
     */
    @Test
    @FunctionTest
    public void testHdrSyncWhenSwitchMode() {
        HdrOperator photoHdroperator = new HdrOperator().initHdrOptions(false);
        HdrOperator videoHdroperator = new HdrOperator().initHdrOptions(true);
        if (photoHdroperator.isSupported(HdrOperator.INDEX_ON)
                && videoHdroperator.isSupported(HdrOperator.INDEX_ON)) {
            new MetaCase("TC_Camera_HDR_0013")
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_PHOTO)
                    .addChecker(new PreviewChecker())
                    .addOperator(photoHdroperator, HdrOperator.INDEX_ON)
                    .addChecker(new HdrQuickSwitchChecker(), HdrOperator.INDEX_ON)
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_VIDEO)
                    .addChecker(new PreviewChecker())
                    .addChecker(new HdrQuickSwitchChecker(), HdrOperator.INDEX_ON)
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_PHOTO)
                    .addChecker(new PreviewChecker())
                    .addChecker(new HdrQuickSwitchChecker(), HdrOperator.INDEX_ON)
                    .run();
        }
        if (photoHdroperator.isSupported(HdrOperator.INDEX_AUTO)
                && videoHdroperator.isSupported(HdrOperator.INDEX_AUTO)) {
            new MetaCase("TC_Camera_HDR_0013")
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_PHOTO)
                    .addChecker(new PreviewChecker())
                    .addOperator(photoHdroperator, HdrOperator.INDEX_AUTO)
                    .addChecker(new HdrQuickSwitchChecker(), HdrOperator.INDEX_AUTO)
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_VIDEO)
                    .addChecker(new PreviewChecker())
                    .addChecker(new HdrQuickSwitchChecker(), HdrOperator.INDEX_AUTO)
                    .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator
                            .INDEX_PHOTO)
                    .addChecker(new PreviewChecker())
                    .addChecker(new HdrQuickSwitchChecker(), HdrOperator.INDEX_AUTO)
                    .run();
        }
    }

    /**
     * Test hdr does not supported when camera is launched by third party.
     */
    @Test
    @FunctionTest
    public void testNoHdrInThirdParty() {
        new MetaCase("TC_Camera_HDR_0032")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_VIDEO)
                .addChecker(new PreviewChecker())
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                .run();
    }

    /**
     * Test hdr is off when interact with flash and dng.
     */
    @Test
    @FunctionTest
    public void testHdrInteractWithDngAndFlash() {
        HdrOperator photoHdroperator = new HdrOperator().initHdrOptions(false);
        if (!photoHdroperator.isSupported(HdrOperator.INDEX_AUTO)) {
            return;
        }

        new MetaCase("TC_Camera_HDR_0025")
                .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_OFF)
                .addOperator(new HdrOperator(), HdrOperator.INDEX_AUTO)
                .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_OFF)
                .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_OFF)
                .addOperator(new HdrOperator(), HdrOperator.INDEX_AUTO)
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_OFF)
                .run();
    }


    /**
     * Test no error happens when stress switch during hdr off ,auto and on.
     */
    @Test
    @StabilityTest
    public void testSwitchHdrModeStress() {
        new MetaCase()
                .observeBegin(new HdrLogObserver())
                .addOperator(new HdrOperator())
                .observeEnd()
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test no error happens when capture hdr auto and on photos.
     */
    @Test
    @StabilityTest
    public void testCaptureInHdrOnAutoModeStress() {
        new MetaCase("TC_Camera_HDR_0029")
                .observeBegin(new HdrLogObserver(),
                        new int[]{HdrLogObserver.INDEX_ON, HdrLogObserver.INDEX_AUTO})
                .addOperator(new HdrOperator(),
                        new int[]{HdrOperator.INDEX_ON, HdrOperator.INDEX_AUTO})
                .observeEnd()
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .observeBegin(new PhotoSavedObserver(1))
                .observeBegin(new HdrLogObserver(),
                        new int[]{HdrLogObserver.INDEX_ON, HdrLogObserver.INDEX_AUTO})
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .repeatEnd()
                .run();
    }

    /**
     * Test hdr value should be right when stress pause and resume camera.
     */
    @Test
    @StabilityTest
    public void testCheckHdrAfterPauseResumeStress() {
        new MetaCase("TC_Camera_HDR_0030")
                .addOperator(new HdrOperator(), HdrOperator.INDEX_AUTO)
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_AUTO)
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_AUTO)
                .repeatEnd()
                .run();
    }

    /**
     * Test hdr value should be sync when stress switch camera.
     */
    @Test
    @StabilityTest
    public void testHdrSyncWhenSwitchCameraStress() {
        new MetaCase("TC_Camera_HDR_0005/0019")
                .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_ON)
                .run();
        new MetaCase()
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new HdrQuickSwitchChecker(), HdrQuickSwitchChecker.INDEX_ON)
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    @MmTest
    public void testVHdrPreview() {
        if (!new HdrOperator().initHdrOptions(true).isSupported(HdrOperator.INDEX_ON)) {
            LogHelper.d(TAG, "[testVHdrPreview] not support vhdr on, return");
            return;
        }
        new MetaCase()
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new HdrOperator().initHdrOptions(true), HdrOperator.INDEX_ON)
                .addChecker(new HdrIndicatorChecker(), HdrIndicatorChecker.INDEX_SHOW)
                .addOperator(new SleepOperator(5))
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addOperator(new HdrOperator().initHdrOptions(true), HdrOperator.INDEX_OFF)
                .addChecker(new HdrIndicatorChecker(), HdrIndicatorChecker.INDEX_HIDE)
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testVHdrRecording() {
        if (!new HdrOperator().initHdrOptions(true).isSupported(HdrOperator.INDEX_ON)) {
            LogHelper.d(TAG, "[testVHdrRecording] not support vhdr on, return");
            return;
        }
        new MetaCase()
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addOperator(new HdrOperator().initHdrOptions(true), HdrOperator.INDEX_ON)
                .addChecker(new HdrIndicatorChecker(), HdrIndicatorChecker.INDEX_SHOW)
                .run();
        new MetaCase()
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new RecordVideoOperator().setDuration(5))
                .observeEnd()
                .run();
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .run();
        new MetaCase()
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new RecordVideoOperator().setDuration(5))
                .observeEnd()
                .run();
    }

}
