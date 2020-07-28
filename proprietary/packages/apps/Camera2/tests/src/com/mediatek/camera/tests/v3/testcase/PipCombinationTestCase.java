package com.mediatek.camera.tests.v3.testcase;


import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.tests.v3.annotation.group.PipTest;
import com.mediatek.camera.tests.v3.annotation.module.PipCombinationTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.PerformanceTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.AntiFlickerExistedChecker;
import com.mediatek.camera.tests.v3.checker.AntiFlickerSettingValueChecker;
import com.mediatek.camera.tests.v3.checker.DngIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.EisExistedChecker;
import com.mediatek.camera.tests.v3.checker.EvUiChecker;
import com.mediatek.camera.tests.v3.checker.HdrIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.HdrQuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.checker.ImagePropertiesUiValueChecker;
import com.mediatek.camera.tests.v3.checker.IsoExistedChecker;
import com.mediatek.camera.tests.v3.checker.IsoSettingValueChecker;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayIconChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.SceneModeSummaryChecker;
import com.mediatek.camera.tests.v3.checker.SelfTimerIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceExistedChecker;
import com.mediatek.camera.tests.v3.observer.DualZoomRatioObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ChangeAntiFlickerToValueOperator;
import com.mediatek.camera.tests.v3.operator.ChangeIsoToValueOperator;
import com.mediatek.camera.tests.v3.operator.DngOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.ImagePropertiesValueOperator;
import com.mediatek.camera.tests.v3.operator.LongPressShutterOperator;
import com.mediatek.camera.tests.v3.operator.MicroPhoneOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SelfTimerOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPipOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Pip Combination test case.
 */
@PipTest
@PipCombinationTest
public class PipCombinationTestCase extends BaseCameraTestCase {
    private static final String SELFTIME = "Self timer";
    private static final String SCENEMODE = "Scene mode";
    private static final String AUDIOMODE = "Audio mode";
    private static final String IMAGEPROPERTIES = "Image properties";

    /**
     * Test photo mode to video mode.
     */
    @Test
    @FunctionTest
    public void testPhotoModeToVideoMode() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0008")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test video mode to photo mode.
     */
    @Test
    @FunctionTest
    public void testVideoModeToPhotoMode() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToNormalVideo();
        switchNormalVideoToPipVideo();

        new MetaCase("TC_Camera_PIP_0009")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .addChecker(new PreviewChecker())

                .run();
    }


    /**
     * Test PIP not support matrix display.
     */
    @Test
    @FunctionTest
    public void testMatrixDisplay() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0055")
                .addChecker(new PreviewChecker())
                .addChecker(new MatrixDisplayIconChecker(),
                        MatrixDisplayIconChecker.INDEX_MATRIX_HIDE)

                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addChecker(new MatrixDisplayIconChecker(),
                        MatrixDisplayIconChecker.INDEX_MATRIX_HIDE)

                .run();
    }

    /**
     * Test PIP not support anti flicker.
     * TODO check preview not support anti flicker
     */
    @Test
    @FunctionTest
    public void testAntiFlicker() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0056")
                .addOperator(new ChangeAntiFlickerToValueOperator("60Hz"))
                .addChecker(new PreviewChecker())

                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new AntiFlickerSettingValueChecker("60Hz"))

                .run();

    }


    /**
     * Test PIP not support AWB.
     * TODO check preview not support AWB
     */
    @Test
    @FunctionTest
    public void testPipAWB() {
        if (!isSupported()) {
            return;
        }
        //photo
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0053")
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)

                .run();
        // video
        switchPipPhotoToPipVideo();
        new MetaCase("TC_Camera_PIP_0053")
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)

                .run();


    }


    /**
     * Test PIP not support EV.
     * TODO check preview not support EV
     */
    @Test
    @FunctionTest
    public void testPipEV() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0054")
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)

                .run();

        // video
        switchPipPhotoToPipVideo();
        new MetaCase("TC_Camera_PIP_0054")
                .addChecker(new EvUiChecker(), EvUiChecker.INDEX_NOT_EXIST)

                .run();
    }

    /**
     * Test PIP not support scene mode.
     * TODO check preview not support scene
     */
    @Test
    @FunctionTest
    public void testPipSceneMode() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0058")
                .addChecker(new SettingItemExistedChecker(SCENEMODE, false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)

                .run();

        // video
        switchPipPhotoToPipVideo();
        new MetaCase("TC_Camera_PIP_0058")
                .addChecker(new SettingItemExistedChecker(SCENEMODE, false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)

                .run();
    }

    /**
     * Test PIP not support auto scene detection.
     * TODO check preview not support auto scene detection
     */
    @Test
    @FunctionTest
    public void testPipAutoSceneDetection() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0059")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_AUTO)
                .run();
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0059")
                .addChecker(new SettingItemExistedChecker(SCENEMODE, false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        switchPipPhotoToNormalPhoto();
        new MetaCase("TC_Camera_PIP_0059")
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_AUTO))
                .run();

        // video
        switchNormalPhotoToNormalVideo();
        new MetaCase("TC_Camera_PIP_0059")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .run();
        switchNormalVideoToPipVideo();
        new MetaCase("TC_Camera_PIP_0059")
                .addChecker(new SettingItemExistedChecker(SCENEMODE, false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        switchPipVideoToNormalVideo();
        new MetaCase("TC_Camera_PIP_0059")
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_NIGHT))
                .run();
    }


    /**
     * Test PIP not support DNG.
     * TODO check preview not support DNG
     */
    @Test
    @FunctionTest
    public void testPipDNG() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0060")
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addChecker(new PreviewChecker())

                .run();

        switchNormalPhotoToPipPhotoMode();

        new MetaCase("TC_Camera_PIP_0060")
                .addChecker(new DngIndicatorChecker(),
                        DngIndicatorChecker.INDEX_HIDE)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()

                .run();

        switchPipPhotoToNormalPhoto();

        new MetaCase("TC_Camera_PIP_0060")
                .addChecker(new DngIndicatorChecker(),
                        DngIndicatorChecker.INDEX_SHOW)

                .run();
    }

    /**
     * Test PIP not support DNG.
     * TODO check preview not support DNG
     */
    @Test
    @FunctionTest
    public void testPipSelfTimer() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0061")
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .addChecker(new PreviewChecker())
                .addChecker(new SelfTimerIndicatorChecker(),
                        SelfTimerIndicatorChecker.INDEX_SHOW)

                .run();

        switchNormalPhotoToPipPhotoMode();

        new MetaCase("TC_Camera_PIP_0061")
                .addChecker(new SelfTimerIndicatorChecker(),
                        SelfTimerIndicatorChecker.INDEX_HIDE)
                .addChecker(new SettingItemExistedChecker(SELFTIME, false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)

                .run();

        switchPipPhotoToNormalPhoto();

        new MetaCase("TC_Camera_PIP_0061")
                .addChecker(new SelfTimerIndicatorChecker(),
                        SelfTimerIndicatorChecker.INDEX_SHOW)

                .run();
    }

    /**
     * Test PIP microphone recording.
     * TODO check video disable microphone or enable microphone.
     */
    @Test
    @FunctionTest
    public void testMicrophoneRecording() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToNormalVideo();
        switchNormalVideoToPipVideo();
        new MetaCase("TC_Camera_PIP_0062")
                .acrossBegin()
                .addOperator(new MicroPhoneOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())

                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()

                .run();

    }

    /**
     * Test PIP photo flash capture.
     */
    @Test
    @FunctionTest
    public void testPipPhotoFlashCapture() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0067")
                .acrossBegin()
                .addOperator(new FlashOperator())
                .acrossEnd()

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()

                .run();
    }

    /**
     * Test PIP video flash capture.
     */
    @Test
    @FunctionTest
    public void testPipVideoFlashCapture() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToNormalVideo();
        switchNormalVideoToPipVideo();
        // API1 pip video not support flash video.
        if (CameraApiHelper.getCameraApiType(null) ==
                CameraDeviceManagerFactory.CameraApi.API1) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0068")
                .acrossBegin()
                .addOperator(new FlashOperator())
                .acrossEnd()

                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()

                .run();
    }

    /**
     * Test PIP zoom.
     */
    @Test
    @FunctionTest
    public void testPipZoom() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        if (isDualCameraZoom()) {
            new MetaCase("TC_Camera_PIP_0069")
                    .observeBegin(new DualZoomRatioObserver(true))
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .observeEnd()

                    .addOperator(new SwitchPipOperator(),
                            SwitchPipOperator.INDEX_TOP_GRAPHIC)
                    .addChecker(new PreviewChecker())


                    .observeBegin(new DualZoomRatioObserver(true))
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .observeEnd()

                    .run();
        } else {
            new MetaCase("TC_Camera_PIP_0069")
                    .observeBegin(new ZoomUiObserver())
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .observeEnd()

                    .addOperator(new SwitchPipOperator(),
                            SwitchPipOperator.INDEX_TOP_GRAPHIC)
                    .addChecker(new PreviewChecker())


                    .observeBegin(new ZoomUiObserver())
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .observeEnd()

                    .run();
        }

        switchPipPhotoToPipVideo();

        if (isDualCameraZoom()) {
            new MetaCase("TC_Camera_PIP_0069")
                    // make sure can do zoom out
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .addChecker(new PreviewChecker())

                    .observeBegin(new DualZoomRatioObserver(false))
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_OUT)
                    .observeEnd()

                    .addOperator(new SwitchPipOperator())
                    .addChecker(new PreviewChecker())
                    // make sure can do zoom out
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .addChecker(new PreviewChecker())

                    .observeBegin(new DualZoomRatioObserver(false))
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_OUT)
                    .observeEnd()

                    .run();

        } else {
            new MetaCase("TC_Camera_PIP_0069")
                    // make sure can do zoom out
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .addChecker(new PreviewChecker())
                    .observeBegin(new ZoomUiObserver())
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_OUT)
                    .observeEnd()

                    .addOperator(new SwitchPipOperator())
                    .addChecker(new PreviewChecker())
                    // make sure can do zoom out
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_IN)
                    .addChecker(new PreviewChecker())

                    .observeBegin(new ZoomUiObserver())
                    .addOperator(new ZoomOperator(),
                            ZoomOperator.INDEX_ZOOM_OUT)
                    .observeEnd()

                    .run();
        }
    }

    /**
     * Test PIP not support hdr.
     */
    @Test
    @FunctionTest
    public void testPipHdr() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0070")
                .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                .addChecker(new HdrIndicatorChecker(),
                        HdrIndicatorChecker.INDEX_SHOW)
                .addChecker(new PreviewChecker())

                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new HdrIndicatorChecker(),
                        HdrIndicatorChecker.INDEX_HIDE)
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())

                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new HdrIndicatorChecker(),
                        HdrIndicatorChecker.INDEX_SHOW)
                .addChecker(new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_EXIST)

                .run();
    }

    /**
     * Test PIP not support eis.
     */
    @Test
    @FunctionTest
    public void testPipEis() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToNormalVideo();
        switchNormalVideoToPipVideo();
        new MetaCase("TC_Camera_PIP_0071")
                .addChecker(new EisExistedChecker(false),
                        EisExistedChecker.INDEX_NOT_EXISTED)

                .run();
    }

    /**
     * Test PIP not audio mode.
     */
    @Test
    @FunctionTest
    public void testPipAudioMode() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToNormalVideo();
        switchNormalVideoToPipVideo();
        new MetaCase("TC_Camera_PIP_0072")
                .addChecker(new SettingItemExistedChecker(AUDIOMODE, false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)

                .run();
    }

    /**
     * Test PIP long press.
     * TODO long press do nothing.
     */
    @Test
    @FunctionTest
    public void testPipLongPress() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0073")
                .addOperator(new LongPressShutterOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test PIP switch mode and press home key.
     */
    @Test
    @StabilityTest
    public void testSwitchModeAndPressHomeKey() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0074")
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_PHOTO)

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);

    }

    /**
     * Test PIP switch photo and video.
     */
    @Test
    @StabilityTest
    public void testSwitchPhotoAndVideo() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0078")
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_PHOTO)

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test PIP switch pip video and normal video.
     */
    @Test
    @StabilityTest
    public void testSwitchNormalVideoAndPipVideo() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToNormalVideo();
        new MetaCase("TC_Camera_PIP_0082")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addOperator(new SwitchToNormalVideoModeOperator())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test PIP switch pip video and normal video.
     * TODO need check pip preview not image properties effect
     */
    @Test
    @FunctionTest
    public void testPipImageProperties() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0105")
                .addOperator(new ImagePropertiesValueOperator(),
                        ImagePropertiesValueOperator.LEVEL_LOW)
                .addChecker(new PreviewChecker())

                .run();
        switchNormalPhotoToPipPhotoMode();

        new MetaCase("TC_Camera_PIP_0105")
                .addChecker(new SettingItemExistedChecker(IMAGEPROPERTIES, false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)

                .addOperator(new SwitchToNormalPhotoModeOperator())

                .addChecker(new PreviewChecker())
                .addChecker(new ImagePropertiesUiValueChecker(),
                        ImagePropertiesUiValueChecker.LEVEL_LOW)

                .run();
    }

    /**
     * Test PIP not support ISO.
     * TODO need check JPEG ISO data
     */
    @Test
    @FunctionTest
    public void testPipISO() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0106")
                .addOperator(new ChangeIsoToValueOperator("1600"))
                .addChecker(new PreviewChecker())
                // switch to pip photo
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new IsoExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                // pip capture
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                // switch to normal photo
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new IsoSettingValueChecker("1600"))

                .run();

    }

    /**
     * Test PIP photo to video.
     */
    @Test
    @PerformanceTest
    public void testPipPhotoToVideo() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0138")
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test PIP video to photo.
     */
    @Test
    @PerformanceTest
    public void testPipVideoToPhoto() {
        if (!isSupported()) {
            return;
        }
        switchNormalPhotoToNormalVideo();
        switchNormalVideoToPipVideo();
        new MetaCase("TC_Camera_PIP_0139")
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_PHOTO)
                .addChecker(new PreviewChecker())

                .run();
    }


    private void switchNormalPhotoToPipPhotoMode() {
        new MetaCase("")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }


    private void switchPipPhotoToPipVideo() {
        new MetaCase("")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)

                .run();
    }

    private void switchPipPhotoToNormalPhoto() {
        new MetaCase("")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    private void switchNormalPhotoToNormalVideo() {
        new MetaCase("")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())

                .run();
    }


    private void switchNormalVideoToPipVideo() {
        new MetaCase("")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    private void switchPipVideoToNormalVideo() {
        new MetaCase("")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    private boolean isDualCameraZoom() {
        boolean issDualCameraSupport =
                SystemProperties.getInt("ro.vendor.mtk_cam_dualzoom_support", 0) ==
                1 ? true : false;
        boolean issSuperDenoiseSupport =
                SystemProperties.getInt("ro.vendor.mtk_cam_dualdenoise_support", 0) ==
                1 ? true : false;
        boolean isDualCameraSupportInOneLoad =
                SystemProperties.getInt("vendor.debug.dualcam.mode", 0) == 1 ? true : false;
        if (issDualCameraSupport && !issSuperDenoiseSupport) {
            return true;
        }
        if (issDualCameraSupport && issSuperDenoiseSupport && isDualCameraSupportInOneLoad) {
            return true;
        }
        return false;
    }

    private boolean isSupported() {
        return Utils.isFeatureSupported("com.mediatek.camera.at.pip");
    }
}
