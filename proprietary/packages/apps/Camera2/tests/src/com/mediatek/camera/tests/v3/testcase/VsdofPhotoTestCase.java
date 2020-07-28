package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.DualCameraTest;
import com.mediatek.camera.tests.v3.annotation.module.VsdofTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.ExifChecker;
import com.mediatek.camera.tests.v3.checker.FocusUiChecker;
import com.mediatek.camera.tests.v3.checker.HdrQuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.checker.IsoExifChecker;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayIconChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SwitchCameraSupportedChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.checker.VSDOFBarChecker;
import com.mediatek.camera.tests.v3.observer.DofValueObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.TouchFocusStateObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.DragVSDOFBarOperator;
import com.mediatek.camera.tests.v3.operator.FontSizeOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.IsoOperator;
import com.mediatek.camera.tests.v3.operator.LocationOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOneByOneOperator;
import com.mediatek.camera.tests.v3.operator.SleepOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.VolumeKeyDownOperator;
import com.mediatek.camera.tests.v3.operator.WhiteBalanceOneByOneOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Vsdof photo mode test cases.
 */
@DualCameraTest
@VsdofTest
public class VsdofPhotoTestCase extends BaseCameraTestCase {

    /**
     * Test switch mode.
     */
    @Test
    @FunctionTest
    public void testSwitchModes() {
        new MetaCase("TC_Camera_VSDOF_0001")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0002")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0003")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0004")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0005")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0006")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test interrupt in preview.
     */
    @Test
    @FunctionTest
    public void testInterruptInPreview() {
        new MetaCase("TC_Camera_VSDOF_0007")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0008")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0009")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0010")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test volume key.
     */
    @Test
    @FunctionTest
    public void testVolumeKey() {
        new MetaCase("TC_Camera_VSDOF_0011")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new VolumeKeyDownOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test all picture sizes.
     */
    @Test
    @FunctionTest
    public void testCaptureAllPictureSizes() {
        new MetaCase("TC_Camera_VSDOF_0013")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0013")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PictureSizeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test all scene mode.
     */
    @Test
    @FunctionTest
    public void testCaptureAllSceneMode() {
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SceneModeOneByOneOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test all wb and capture.
     */
    @Test
    @FunctionTest
    public void testCaptureAllWB() {
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new WhiteBalanceOneByOneOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test capture stability.
     */
    @Test
    @StabilityTest
    public void testPauseAndResumeStability() {
        new MetaCase("TC_Camera_VSDOF_0015")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0015")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
        new MetaCase("TC_Camera_VSDOF_0015")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test touch focus and capture.
     */
    @Test
    @FunctionTest
    public void testCaptureWithTouchFocus() {
        new MetaCase("TC_Camera_VSDOF_0017")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new TouchFocusOperator(), 15)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test pause event when capturing.
     */
    @Test
    @FunctionTest
    public void testPauseWhenCapturing() {
        new MetaCase("TC_Camera_VSDOF_0018")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0019")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addOperator(new PauseResumeByRecentAppOperator())
                .run();
    }

    /**
     * Test go to gallery and back.
     */
    @Test
    @FunctionTest
    public void testGotoGalleryAndBack() {
        new MetaCase("TC_Camera_VSDOF_0016")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
        new MetaCase("TC_Camera_VSDOF_0020")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())

                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test touch focus.
     */
    @Test
    @FunctionTest
    public void testTouchAF() {
        TouchFocusStateObserver observer = new TouchFocusStateObserver();
        observer.initEnv(null, true, false);
        new MetaCase("TC_Camera_VSDOF_0022")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(observer, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .run();
    }

    /**
     * Test capture stability.
     */
    @Test
    @StabilityTest
    public void testCaptureStability() {
        new MetaCase("TC_Camera_VSDOF_0024")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new CapturePhotoOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test touch focus.
     */
    @Test
    @FunctionTest
    public void testOnLongPress() {
        new MetaCase("TC_Camera_VSDOF_0025")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new OnLongPressOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture with iso setting.
     */
    @Test
    @FunctionTest
    public void testCaptureIsoExif() {
        new MetaCase("TC_Camera_VSDOF_0033")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0033")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new IsoOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new IsoExifChecker())
                .run();
    }

    /**
     * Test capture exif with setting.
     */
    @Test
    @FunctionTest
    public void testCaptureExif() {
        new MetaCase("")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ExifChecker())
                .run();
    }

    /**
     * Change the system font size to huge and test capture function.
     */
    @Test
    @FunctionTest
    public void testCaptureWithHugeFontSize() {
        new MetaCase("TC_Camera_VSDOF_0051")
                .addOperator(new FontSizeOperator(), FontSizeOperator.INDEX_HUGE)
                .run();

        new MetaCase("TC_Camera_VSDOF_0051")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test have bar in preview.
     */
    @Test
    @FunctionTest
    public void testHaveVSDOFBar() {
        new MetaCase("TC_Camera_VSDOF_0053")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new VSDOFBarChecker(), VSDOFBarChecker.INDEX_HAS_VSDOFBAR)
                .run();
    }

    /**
     * Test bar drag in preview.
     */
    @Test
    @FunctionTest
    public void testDragVSDOFBar() {
        new MetaCase("TC_Camera_VSDOF_0054")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new VSDOFBarChecker(), VSDOFBarChecker.INDEX_HAS_VSDOFBAR)
                .acrossBegin()
                .observeBegin(new DofValueObserver())
                .addOperator(new DragVSDOFBarOperator())
                .observeEnd()
                .acrossEnd()
                .run();
    }

    /**
     * Test Location.
     */
    @Test
    @FunctionTest
    public void testLocation() {
        new MetaCase("TC_Camera_VSDOF_0127")
                .addOperator(new LocationOperator())
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test zoom in and zoom out.
     */
    @Test
    @FunctionTest
    public void testZoom() {
        new MetaCase("TC_Camera_VSDOF_0135")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .run();
    }

    /**
     * Test not support matrix Display.
     */
    @Test
    @FunctionTest
    public void testNotSupportHdr() {
        new MetaCase("TC_Camera_VSDOF_0134")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(
                        new HdrQuickSwitchExistedChecker(),
                        HdrQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                .run();
    }

    /**
     * Test not support matrix Display.
     */
    @Test
    @FunctionTest
    public void testNotSupportSwitchCamera() {
        new MetaCase("TC_Camera_VSDOF_0136")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new SwitchCameraSupportedChecker(),
                        SwitchCameraSupportedChecker.INDEX_NOT_SUPPORT)
                .run();
    }

    /**
     * Test not support matrix Display.
     */
    @Test
    @FunctionTest
    public void testNotSupportMatrixDisplay() {
        new MetaCase("TC_Camera_VSDOF_0138")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(
                        new MatrixDisplayIconChecker(), MatrixDisplayIconChecker.INDEX_MATRIX_HIDE)
                .run();
    }

    /**
     * Test zoom in and zoom out.
     */
    @Test
    @FunctionTest
    public void testNotSupportDualZoom() {
        new MetaCase("TC_Camera_VSDOF_0145")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testPreviewAndCapture() {
        new MetaCase("")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                // must sleep 10s
                .addOperator(new SleepOperator(10))
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test preview touch with different level.
     */
    @Test
    @FunctionTest
    public void testPreviewTouchWithDifferentLevel() {
        TouchFocusStateObserver observer = new TouchFocusStateObserver();
        observer.initEnv(null, true, false);
        new MetaCase("")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .addOperator(new DragVSDOFBarOperator())
                .observeBegin(observer, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test capture with different level.
     */
    @Test
    @FunctionTest
    public void testCaptureWithDifferentLevel() {
        new MetaCase("")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .addOperator(new DragVSDOFBarOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)


                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())

                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test capture and touch with different level.
     */
    @Test
    @FunctionTest
    public void testCaptureAndTouchWithDifferentLevel() {
        TouchFocusStateObserver observer = new TouchFocusStateObserver();
        observer.initEnv(null, true, false);
        new MetaCase("")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())

                .addOperator(new DragVSDOFBarOperator())
                .observeBegin(observer, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)


                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())

                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker())
                .addChecker(new PreviewChecker())

                .run();
    }
}
