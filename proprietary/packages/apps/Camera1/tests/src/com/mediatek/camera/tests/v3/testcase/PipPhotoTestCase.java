package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.PipTest;
import com.mediatek.camera.tests.v3.annotation.module.PipPhotoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.PerformanceTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PipModeChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.OnSingleTapUpOperator;
import com.mediatek.camera.tests.v3.operator.PauseByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.PressShutterOperator;
import com.mediatek.camera.tests.v3.operator.ResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPipOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.operator.ZsdOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Normal photo mode test case.
 */
@PipTest
@PipPhotoTest
public class PipPhotoTestCase extends BaseCameraTestCase {

    /**
     * Test enter and exit function.
     */
    @Test
    @FunctionTest
    public void testEntryAndExit() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0001")
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test switch pip when zsd on and zad off.
     */
    @Test
    @FunctionTest
    public void testSwitchPip() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0019")
                // open zsd, close zsd
                .acrossBegin()
                .addOperator(new ZsdOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                // switch pip
                .addOperator(new SwitchPipOperator(), SwitchPipOperator.INDEX_TOP_GRAPHIC)
                .addOperator(new SwitchPipOperator(), SwitchPipOperator.INDEX_TOP_GRAPHIC)
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test switch camera.
     */
    @Test
    @FunctionTest
    public void testSwitchCamera() {
        if (!isSupported()) {
            return;
        }
    }

    /**
     * Test press back key and enter camera again.
     * TODO check photo mode after enter camera again
     */
    @Test
    @FunctionTest
    public void testPressBackKey() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0026")
                .addOperator(new PauseResumeByBackKeyOperator())
                .run();
    }


    /**
     * Test press back key and enter camera again.
     * TODO need check front camera or back camera
     */
    @Test
    @FunctionTest
    public void testFrontCameraSwitchToPip() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0028")
                // switch camera
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                // switch to pip
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                // photo mode
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test press home key and enter camera again.
     */
    @Test
    @FunctionTest
    public void testPressHomeKey() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0030")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test press power key and enter camera again.
     */
    @Test
    @FunctionTest
    public void testPressPowerKey() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0032")
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test press recent key and enter camera again.
     */
    @Test
    @FunctionTest
    public void testPressRecentKey() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("")
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test zsd capture.
     * TODO check go to gallery
     */
    @Test
    @FunctionTest
    public void testZsdCapture() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0034")
                // open zsd, close zsd
                .acrossBegin()
                .addOperator(new ZsdOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                //capture

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture use different picture size.
     */
    @Test
    @FunctionTest
    public void testDifferentSizeCapture() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0038")
                // open zsd, close zsd
                .acrossBegin()
                .addOperator(new PictureSizeOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                //capture
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture when switch pip.
     */
    @Test
    @FunctionTest
    public void testCaptureAndSwitch() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0039")
                //capture
                .addOperator(new PressShutterOperator())
                .addOperator(new SwitchPipOperator())
                .addChecker(new PreviewChecker())

                .addOperator(new SwitchPipOperator())
                .addOperator(new PressShutterOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test press power key and enter camera again.
     */
    @Test
    @StabilityTest
    public void testPressPowerKeyStress() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0076")
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test flash capture.
     */
    @Test
    @FunctionTest
    public void testFlashCapture() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0079")
                .acrossBegin()
                .addOperator(new FlashOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .run();
    }

    /**
     * Test switch pip photo mode and photo mode.
     */
    @Test
    @StabilityTest
    public void testPhotoToPipPhotoStress() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0081")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addOperator(new SwitchToPipPhotoModeOperator())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test non zsd capture.
     */
    @Test
    @StabilityTest
    public void testNonZsdCaptureStress() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        disableZsd();
        new MetaCase("TC_Camera_PIP_0083")
                .addOperator(new PressShutterOperator())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test zsd capture.
     */
    @Test
    @StabilityTest
    public void testZsdCaptureStress() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        enableZsd();
        new MetaCase("TC_Camera_PIP_0084")
                .addOperator(new PressShutterOperator())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test refocus and capture.
     */
    @Test
    @FunctionTest
    public void testRefocusAndCapture() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0085")
                .addOperator(new OnSingleTapUpOperator())
                .addOperator(new PressShutterOperator())
                .run();
    }

    /**
     * Test zoom in/out and swich pip.
     */
    @Test
    @FunctionTest
    public void testZoomAndSwitchPip() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0088")
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .addOperator(new SwitchPipOperator())
                .addChecker(new PreviewChecker())

                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .addOperator(new SwitchPipOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test zoom in/out and capture.
     */
    @Test
    @FunctionTest
    public void testZoomAndCapture() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0091")
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()

                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()

                .run();
    }

    /**
     * Test switch pip stress.
     * TODO switch camera operator
     */
    @Test
    @StabilityTest
    public void testSwitchPipStress() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0092")
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new SwitchPipOperator())
                .repeatEnd()

                // .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                // .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                // .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                // .repeatEnd()

                .run();
    }

    /**
     * Test capture and go to gallery and go back.
     */
    @Test
    @FunctionTest
    public void testGotoGallery() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0095")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())

                .addOperator(new BackToCameraOperator())
                .addChecker(new PipModeChecker(), PipModeChecker.PIP_PHOTO_MODE)

                .run();
    }

    /**
     * Test capture 20 times.
     */
    @Test
    @StabilityTest
    public void testCapture20Times() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0108")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test press home key and switch PIP.
     */
    @Test
    @StabilityTest
    public void testPressHomeKeyAndSwitchPip() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0108")
                .addOperator(new SwitchPipOperator().ignoreBothPageCheck(),
                        SwitchPipOperator.INDEX_TOP_GRAPHIC)
                .addOperator(new PauseByHomeKeyOperator().ignoreBothPageCheck())
                .addOperator(new ResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test press home key and switch PIP.
     * TODO check file name
     */
    @Test
    @FunctionTest
    public void testCaptureAndCheckFileName() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0127")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test press home key and switch PIP.
     * TODO check exif information
     */
    @Test
    @FunctionTest
    public void testCaptureAndCheckExif() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0129")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test photo mode to pip photo mode.
     */
    @Test
    @PerformanceTest
    public void testPhotoModeToPipPhotoMode() {
        if (!isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_PIP_0130")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test pip photo mode to photo mode.
     */
    @Test
    @PerformanceTest
    public void testPipPhotoModeToPhotoMode() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0131")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test panorama mode to pip photo mode.
     */
    @Test
    @PerformanceTest
    public void testPanoramaToPipPhotoMode() {
        if (!isSupported()) {
            return;
        }
        switcToPanoramaMde();
        new MetaCase("TC_Camera_PIP_0132")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test pip photo mode to panorama mode.
     */
    @Test
    @PerformanceTest
    public void testPipPhotoModeToPanorama() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0133")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test slow motion mode to pip mode.
     */
    @Test
    @PerformanceTest
    public void testSlowMotionToPipPhotoMode() {
        if (!isSupported()) {
            return;
        }
        switchToSlowMotion();
        new MetaCase("TC_Camera_PIP_0134")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test pip photo to slow motion mode.
     */
    @Test
    @PerformanceTest
    public void testPipPhotoToSlowMotion() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        new MetaCase("TC_Camera_PIP_0135")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test disable zsd and switch pip.
     */
    @Test
    @PerformanceTest
    public void testDisableZsdAndSwitchPip() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        disableZsd();
        new MetaCase("TC_Camera_PIP_0140")
                .addOperator(new SwitchPipOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test enable zsd and switch pip.
     */
    @Test
    @PerformanceTest
    public void testEnableZsdAndSwitchPip() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        enableZsd();
        new MetaCase("TC_Camera_PIP_0141")
                .addOperator(new SwitchPipOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test disable zsd and shot to preview.
     */
    @Test
    @PerformanceTest
    public void testDisableZsdAndShotToPreview() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        disableZsd();
        new MetaCase("TC_Camera_PIP_0145")
                .addOperator(new PressShutterOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test enable zsd and shot to preview.
     */
    @Test
    @PerformanceTest
    public void testEnableZsdAndShotToPreview() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        enableZsd();
        new MetaCase("TC_Camera_PIP_0146")
                .addOperator(new PressShutterOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test disable zsd and shot to shot.
     */
    @Test
    @PerformanceTest
    public void testDisableZsdAndShotToShot() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        disableZsd();
        new MetaCase("TC_Camera_PIP_0148")
                .addOperator(new PressShutterOperator())
                .addChecker(new PreviewChecker())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test enable zsd and shot to shot.
     */
    @Test
    @PerformanceTest
    public void testEnableZsdAndShotToShot() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        enableZsd();
        new MetaCase("TC_Camera_PIP_0149")
                .addOperator(new PressShutterOperator())
                .addChecker(new PreviewChecker())

                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test disable zsd and shot to thumbnail.
     */
    @Test
    @PerformanceTest
    public void testDisableZsdAndShotToThumbnail() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        disableZsd();
        new MetaCase("TC_Camera_PIP_0150")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test enable zsd and shot to thumbnail.
     */
    @Test
    @PerformanceTest
    public void testEnableZsdAndShotToThumbnail() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        enableZsd();
        new MetaCase("TC_Camera_PIP_0151")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test disable zsd and shot to jpeg.
     */
    @Test
    @PerformanceTest
    public void testDisableZsdAndShotToJpeg() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        disableZsd();
        new MetaCase("TC_Camera_PIP_0152")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test enable zsd and shot to Jpeg.
     */
    @Test
    @PerformanceTest
    public void testEnableZsdAndShotToJpeg() {
        if (!isSupported()) {
            return;
        }
        switchToPipPhotoMode();
        enableZsd();
        new MetaCase("TC_Camera_PIP_0153")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())

                .run();
    }

    private void switchToPipPhotoMode() {
        new MetaCase("")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    private void switcToPanoramaMde() {
        new MetaCase("")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    private void switchToSlowMotion() {
        new MetaCase("")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())

                .run();
    }

    private void disableZsd() {
        new MetaCase("")
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)
                .addChecker(new PreviewChecker())

                .run();
    }

    private void enableZsd() {
        new MetaCase("")
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                .addChecker(new PreviewChecker())

                .run();
    }

    private boolean isSupported() {
        return Utils.isFeatureSupported("com.mediatek.camera.at.pip");
    }
}
