package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.PipTest;
import com.mediatek.camera.tests.v3.annotation.module.PipVideoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.checker.VideoFileSizeChecker;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.OnSingleTapUpOperator;
import com.mediatek.camera.tests.v3.operator.PauseByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.QualityOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.ResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPipOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.TakeVideoSnapShotOperator;
import com.mediatek.camera.tests.v3.operator.VolumeKeyDownOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Pip video mode test cases.
 */
@PipVideoTest
@PipTest
public class PipVideoTestCase extends BaseCameraTestCase {
    /**
     * Test switch mode.
     */
    @Test
    @FunctionTest
    public void testPipVideoModeSwitch() {
        new MetaCase("TC_Camera_PIP_0002")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0004")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test switch camera.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraRecordVideo() {
        new MetaCase("TC_Camera_PIP_0023")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0022")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPipOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0024")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addOperator(new SwitchPipOperator())
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test video recording in different camera.
     */
    @Test
    @FunctionTest
    public void testRecordVideo() {
        new MetaCase("TC_Camera_PIP_0025")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0025")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test interrupt in video preview.
     */
    @Test
    @FunctionTest
    public void testInterruptInVideoPreview() {
        new MetaCase("TC_Camera_PIP_0027")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0031")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0033")
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0033")
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test record with different qualities.
     */
    @Test
    @FunctionTest
    public void testDifferentQuality() {
        new MetaCase("TC_Camera_PIP_0042")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .run();
        new MetaCase("TC_Camera_PIP_0042")
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
     * Test interrupt in video recording.
     */
    @Test
    @FunctionTest
    public void testInterruptInRecording() {
        new MetaCase("TC_Camera_PIP_0046")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0045")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0047")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0048")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test volume key in video preview.
     */
    @Test
    @FunctionTest
    public void testVolumeKeyDownForVideo() {
        new MetaCase("TC_Camera_PIP_0049")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_PIP_0050")
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_PIP_0050")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new VolumeKeyDownOperator())
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_PIP_0051")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0110")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPipOperator())
                .addOperator(new PauseByHomeKeyOperator())
                .addOperator(new ResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }


    /**
     * Test screen on/off stress.
     */
    @Test
    @FunctionTest
    public void testScreenOnOffStress() {
        new MetaCase("TC_Camera_PIP_0077")
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .repeatEnd()
                .run();
    }

    /**
     * Test record stress.
     */
    @Test
    @StabilityTest
    public void testRecordStress() {
        new MetaCase("TC_Camera_PIP_0086")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0086")
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .observeEnd()
                .repeatEnd()
                .run();
    }

    /**
     * Test go to gallery and back.
     */
    @Test
    @FunctionTest
    public void testGotoGalleryAndBack() {
        new MetaCase("TC_Camera_PIP_0096")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())

                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())

                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker())
                .addChecker(new PreviewChecker())

                .run();
    }

    /**
     * Test vss stress.
     */
    @Test
    @StabilityTest
    public void testRecordingVss() {
        new MetaCase("TC_Camera_PIP_0087")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new TakeVideoSnapShotOperator())
                .repeatEnd()
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test thermal stress.
     */
    /*@Test
    @FunctionTest
    public void testRecordingThermal() {
        new MetaCase("TC_Camera_PIP_0107")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new SleepOperator(600))
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .run();
    }*/

    /**
     * Test touch focus in video preview and recording.
     */
    @Test
    @FunctionTest
    public void testTouchFocusForVideoPreviewAndRecording() {
        new MetaCase("TC_Camera_PIP_0114")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new OnSingleTapUpOperator())
                .run();
        new MetaCase("TC_Camera_PIP_0099")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new OnSingleTapUpOperator())
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0114")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new OnLongPressOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test zoom in and zoom out.
     */
    @Test
    @FunctionTest
    public void testZoom() {
        new MetaCase("TC_Camera_PIP_0089")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .run();
        new MetaCase("TC_Camera_PIP_0090")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test switch camera.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraStress() {
        new MetaCase("TC_Camera_PIP_0093")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new SwitchPipOperator())
                .repeatEnd()
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_PIP_0094")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new SwitchPipOperator())
                .repeatEnd()
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .run();
    }
}
