package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.DualCameraTest;
import com.mediatek.camera.tests.v3.annotation.module.VsdofTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.FocusUiChecker;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayIconChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.SwitchCameraSupportedChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.checker.VSDOFBarChecker;
import com.mediatek.camera.tests.v3.checker.VideoFileSizeChecker;
import com.mediatek.camera.tests.v3.checker.VssSupportedChecker;
import com.mediatek.camera.tests.v3.observer.DofValueObserver;
import com.mediatek.camera.tests.v3.observer.TouchFocusStateObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.DragVSDOFBarOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeRecordingOperator;
import com.mediatek.camera.tests.v3.operator.QualityOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOneByOneOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.VolumeKeyDownOperator;
import com.mediatek.camera.tests.v3.operator.WhiteBalanceOneByOneOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Vsdof video mode test cases.
 */
@DualCameraTest
@VsdofTest
public class VsdofVideoTestCase extends BaseCameraTestCase {

    /**
     * Test switch mode.
     */
    @Test
    @FunctionTest
    public void testVsdofVideoModeSwitch() {
        new MetaCase("TC_Camera_VSDOF_0057")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0057")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0058")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test interrupt in video preview.
     */
    @Test
    @FunctionTest
    public void testInterruptInVideoPreview() {
        new MetaCase("TC_Camera_VSDOF_0060")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0061")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0062")
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0062")
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
        SwitchToStereoPhotoModeOperator stereoOperator = new SwitchToStereoPhotoModeOperator();
        if (!stereoOperator.isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_VSDOF_0063")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .run();
        new MetaCase("TC_Camera_VSDOF_0064")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
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
        new MetaCase("TC_Camera_VSDOF_0066")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0067")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0068")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0069")
                .addOperator(new SwitchToStereoVideoModeOperator())
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
        new MetaCase("TC_Camera_VSDOF_0070")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_VSDOF_0071")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_VSDOF_0072")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new VolumeKeyDownOperator())
                .addOperator(new PauseResumeByRecentAppOperator())
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
    }

    /**
     * Test record stress.
     */
    @Test
    @StabilityTest
    public void testRecordStress() {
        new MetaCase("TC_Camera_VSDOF_0073")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0074")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
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
        new MetaCase("TC_Camera_VSDOF_0075")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
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
     * Test touch focus in video preview and recording.
     */
    @Test
    @FunctionTest
    public void testTouchFocusForVideoPreviewAndRecording() {
        TouchFocusStateObserver observer = new TouchFocusStateObserver();
        observer.initEnv(null, true, false);
        new MetaCase("TC_Camera_VSDOF_0077")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(observer, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_VSDOF_0078")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .observeBegin(observer, TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 15)
                .addChecker(new FocusUiChecker().ignoreBeforePageCheck(),
                        FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .addOperator(new StopRecordOperator(true))
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test long preview event in video preview.
     */
    @Test
    @FunctionTest
    public void testLongPressForVideoPreviewAndRecording() {
        new MetaCase("TC_Camera_VSDOF_0082")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0083")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new OnLongPressOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test vsdof bar in video preview and recording.
     */
    @Test
    @FunctionTest
    public void testHaveVSDOFBarInPreviewAndRecording() {
        new MetaCase("TC_Camera_VSDOF_0098")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new VSDOFBarChecker(), VSDOFBarChecker.INDEX_HAS_VSDOFBAR)
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addChecker(new VSDOFBarChecker(), VSDOFBarChecker.INDEX_HAS_VSDOFBAR)
                .run();
    }

    /**
     * Test bar drag in video preview and recording.
     */
    @Test
    @FunctionTest
    public void testDragVSDOFBarInPreviewAndRecording() {
        new MetaCase("TC_Camera_VSDOF_0099")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new VSDOFBarChecker(), VSDOFBarChecker.INDEX_HAS_VSDOFBAR)
                .run();
        new MetaCase("TC_Camera_VSDOF_0099")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .observeBegin(new DofValueObserver())
                .addOperator(new DragVSDOFBarOperator())
                .observeEnd()
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0099")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addChecker(new VSDOFBarChecker(), VSDOFBarChecker.INDEX_HAS_VSDOFBAR)
                .acrossBegin()
                .observeBegin(new DofValueObserver())
                .addOperator(new DragVSDOFBarOperator())
                .observeEnd()
                .acrossEnd()
                .run();
    }

    /**
     * Test different white balance mode for recording.
     */
    @Test
    @FunctionTest
    public void testAllWhiteBalanceForRecording() {
        new MetaCase("TC_Camera_VSDOF_0105")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0105")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new WhiteBalanceOneByOneOperator())
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
     * Test different scene mode for recording.
     */
    @Test
    @FunctionTest
    public void testAllSceneModeForRecording() {
        new MetaCase("TC_Camera_VSDOF_0109")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_VSDOF_0109")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SceneModeOneByOneOperator())
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
     * Test don't support vss.
     */
    @Test
    @FunctionTest
    public void testRecordingVss() {
        new MetaCase("TC_Camera_VSDOF_0119")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addChecker(new VssSupportedChecker(), VssSupportedChecker.INDEX_NOT_SUPPORT)
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test not support switch camera.
     */
    @Test
    @FunctionTest
    public void testNotSupportSwitchCamera() {
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
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
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(
                        new MatrixDisplayIconChecker(), MatrixDisplayIconChecker.INDEX_MATRIX_HIDE)
                .run();
    }

    /**
     * Test pause/resume recording.
     */
    @Test
    @FunctionTest
    public void testPauseResumeRecording() {
        new MetaCase("")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeRecordingOperator())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
    }

    /**
     * Test recording with different level.
     */
    @Test
    @FunctionTest
    public void testRecordingWithDifferentLevel() {
        new MetaCase("")
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new DragVSDOFBarOperator())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)


                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())

                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker())
                .addChecker(new PreviewChecker())

                .run();
    }
}
