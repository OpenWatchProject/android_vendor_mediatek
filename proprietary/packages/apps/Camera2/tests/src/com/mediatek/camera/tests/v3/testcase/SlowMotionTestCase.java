package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.SlowMotionTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.AeAfLockUiChecker;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayIconChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.SettingIconExistedChecker;
import com.mediatek.camera.tests.v3.checker.SwitchCameraSupportedChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.checker.TouchFocusChecker;
import com.mediatek.camera.tests.v3.checker.VideoDurationChecker;
import com.mediatek.camera.tests.v3.checker.VssSupportedChecker;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryPlayVideoOperator;
import com.mediatek.camera.tests.v3.operator.LongPressShutterOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeRecordingOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SleepOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.VolumeKeyDownOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Test case for slow motion.
 */
@CameraBasicTest
@SlowMotionTest
public class SlowMotionTestCase extends BaseCameraTestCase {

    /**
     * Test enter and exit slow motion.
     */
    @Test
    @FunctionTest
    public void testCanRecordingForSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0001/TC_Camera_Slow_Motion_0035")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
    }

    /**
     * Stress test recording slow motion.
     */
    @Test
    @StabilityTest
    public void testStressRecordingForSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0037")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .observeEnd()
                .repeatEnd()
                .run();
    }

    /**
     * Test enter and exit slow motion.
     */
    @Test
    @FunctionTest
    public void testSlowMotionResult() {
        new MetaCase("TC_Camera_Slow_Motion_0004")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addChecker(new VideoDurationChecker(), VideoDurationChecker.INDEX_SLOW_MOTION)
                .run();
    }

    /**
     * Test slow motion preview support touch focus.
     */
    @Test
    @FunctionTest
    public void testSlowMotionPreviewSupportTouchFocus() {
        new MetaCase("TC_Camera_Slow_Motion_0005/" +
                "TC_Camera_Slow_Motion_0009/TC_Camera_Slow_Motion_0034")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new TouchFocusOperator(), 20)
                .addChecker(new TouchFocusChecker())
                .run();
    }

    /**
     * Test slow motion preview support ae/af lock.
     */
    @Test
    @FunctionTest
    public void testSlowMotionPreviewSupportAEAFLock() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_Slow_Motion_0005/TC_Camera_Slow_Motion_0068")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new OnLongPressOperator())
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .run();
    }

    /**
     * Test slow motion preview support zoom.
     */
    @Test
    @FunctionTest
    public void testSlowMotionPreviewSupportZoom() {
        new MetaCase("TC_Camera_Slow_Motion_0005")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
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
     * Test slow motion recording support zoom.
     */
    @Test
    @FunctionTest
    public void testSlowMotionRecordingSupportZoom() {
        new MetaCase("TC_Camera_Slow_Motion_0006")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test slow motion recording support touch focus.
     */
    @Test
    @FunctionTest
    public void testSlowMotionRecordingSupportTouchFocus() {
        new MetaCase("TC_Camera_Slow_Motion_0006/TC_Camera_Slow_Motion_0009")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new TouchFocusOperator(), 20)
                .addChecker(new TouchFocusChecker())
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test slow motion recording support ae/af lock.
     */
    @Test
    @FunctionTest
    public void testSlowMotionRecordingSupportAEAFLock() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_Slow_Motion_0006")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new OnLongPressOperator())
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test second recording slow motion the result file duration should right.
     */
    @Test
    @FunctionTest
    public void testSlowMotionRecordingTwice() {
        new MetaCase("TC_Camera_Slow_Motion_0007")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .observeEnd()
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addChecker(new VideoDurationChecker(), VideoDurationChecker.INDEX_SLOW_MOTION)
                .run();
    }

    /**
     * Test after back from gallery can recording.
     */
    @Test
    @FunctionTest
    public void testCanRecordingAfterBackFromGallery() {
        new MetaCase("TC_Camera_Slow_Motion_0010")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new VideoDurationChecker(), VideoDurationChecker.INDEX_SLOW_MOTION)
                .run();
    }

    /**
     * Test slow motion don't support matrix display.
     */
    @Test
    @FunctionTest
    public void testMatrixDisplayForSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0011")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(
                        new MatrixDisplayIconChecker(), MatrixDisplayIconChecker.INDEX_MATRIX_HIDE)
                .run();
    }

    /**
     * Test enter and exit slow motion.
     */
    @Test
    @FunctionTest
    public void testEnterAndExitSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0012")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToModeOperator("Normal"))
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Stress test enter and exit slow motion.
     */
    @Test
    @StabilityTest
    public void testStressEnterAndExitSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0013")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToModeOperator("Normal"))
                .addChecker(new PreviewChecker())
                .repeatEnd()
                .run();
    }

    /**
     * Test check no settings icon.
     */
    @Test
    @FunctionTest
    public void testSettingIconInSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0014")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new SettingIconExistedChecker(false))
                .run();
    }

    /**
     * Test slow motion don't support vss.
     */
    @Test
    @FunctionTest
    public void testVssForSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0021")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addChecker(new VssSupportedChecker(), VssSupportedChecker.INDEX_NOT_SUPPORT)
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test after back from gallery can zoom.
     */
    @Test
    @FunctionTest
    public void testCanZoomAfterBackFromGallery() {
        new MetaCase("TC_Camera_Slow_Motion_0024/TC_Camera_Slow_Motion_0049")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     *
     */
    @Test
    @FunctionTest
    public void testLongPressShutterButton() {
        new MetaCase("TC_Camera_Slow_Motion_0025/TC_Camera_Slow_Motion_0036")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new LongPressShutterOperator())
                .run();
    }


    /**
     *
     */
    @Test
    @FunctionTest
    public void testVolumeKeyForSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0026/TC_Camera_Slow_Motion_0060")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new VolumeKeyDownOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test slow motion don't support switch camera.
     */
    @Test
    @FunctionTest
    public void testSwitchIconForSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0047")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new SwitchCameraSupportedChecker(),
                        SwitchCameraSupportedChecker.INDEX_NOT_SUPPORT)
                .run();
    }


    /**
     * Test enter and exit slow motion.
     */
    @Test
    @FunctionTest
    public void testThumbnailForSlowMotion() {
        new MetaCase("TC_Camera_Slow_Motion_0048")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test during recording click back key.
     */
    @Test
    @FunctionTest
    public void testRecordingAndBackKey() {
        new MetaCase("TC_Camera_Slow_Motion_0058")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new BackToCameraOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test during recording click power key.
     */
    @Test
    @FunctionTest
    public void testRecordingAndPowerKey() {
        new MetaCase("TC_Camera_Slow_Motion_0056")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test during recording click recent key.
     */
    @Test
    @FunctionTest
    public void testRecordingAndRecentApp() {
        new MetaCase("TC_Camera_Slow_Motion_0056")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByRecentAppOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test during preview click power key.
     */
    @Test
    @FunctionTest
    public void testPreviewAndPowerKey() {
        new MetaCase("TC_Camera_Slow_Motion_0054/TC_Camera_Slow_Motion_0069")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .run();
    }

    /**
     * Stress test preview click power key.
     */
    @Test
    @StabilityTest
    public void testStressPreviewAndPowerKey() {
        new MetaCase("TC_Camera_Slow_Motion_0055")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .repeatEnd()
                .run();
    }

    /**
     * Test during recording click home key.
     */
    @Test
    @FunctionTest
    public void testRecordingAndHomeKey() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testPauseResumeRecordingAndPlay() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .repeatBegin(10)
                .addOperator(new PauseResumeRecordingOperator())
                .addOperator(new SleepOperator(3))
                .addOperator(new PauseResumeRecordingOperator())
                .repeatEnd()
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addOperator(new GoToGalleryPlayVideoOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testRecordingAndPlay() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator().setDuration(120))
                .observeEnd()
                .addOperator(new GoToGalleryPlayVideoOperator())
                .run();
    }

}
