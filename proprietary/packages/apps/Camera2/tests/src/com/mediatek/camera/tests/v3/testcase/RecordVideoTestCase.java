package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.Video3DNRTest;
import com.mediatek.camera.tests.v3.annotation.module.VideoRecorderTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.SanityTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.AeAfLockUiChecker;
import com.mediatek.camera.tests.v3.checker.CameraFacingChecker;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayOpenedChecker;
import com.mediatek.camera.tests.v3.checker.PhotoVideoModeChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.RecordingPauseChecker;
import com.mediatek.camera.tests.v3.checker.SwitchCameraSupportedChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.checker.VideoDurationChecker;
import com.mediatek.camera.tests.v3.checker.VideoFileSizeChecker;
import com.mediatek.camera.tests.v3.checker.VssSupportedChecker;
import com.mediatek.camera.tests.v3.observer.AudioPlayedObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.AntiFlickerOperator;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.EisOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryPlayVideoOperator;
import com.mediatek.camera.tests.v3.operator.IncomingCallSimulateOperator;
import com.mediatek.camera.tests.v3.operator.LongPressShutterOperator;
import com.mediatek.camera.tests.v3.operator.MatrixDisplayOpeningOperator;
import com.mediatek.camera.tests.v3.operator.MicroPhoneOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.OnSingleTapUpOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeRecordingOperator;
import com.mediatek.camera.tests.v3.operator.QualityOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOneByOneOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SleepOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.TakeVideoSnapShotOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.VolumeKeyDownOperator;
import com.mediatek.camera.tests.v3.operator.WhiteBalanceOneByOneOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraBasicTest
@VideoRecorderTest
public class RecordVideoTestCase extends BaseCameraTestCase {
    /**
     * test record video.
     * Step 1: take main sensor begin
     * Step 2: swipe to video mode
     * Step 3: click video button and record 15s
     * Step 4: check video number
     * Step 5: take sub sensor begin
     * Step 6: switch to sub sensor
     * Step 7: go to gallery
     */
    @Test
    @SanityTest
    @FunctionTest
    public void testRecordVideo() {
        new MetaCase("TC_030")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new CameraFacingChecker())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator().setDuration(15))
                .observeEnd()
                .addChecker(new VideoDurationChecker(), VideoDurationChecker.INDEX_NORMAL)
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test recording for different quality for back camera.
     * video size should same with video quality.
     */
    @Test
    @FunctionTest
    public void testRecordingForDifferentQualityForBack() {
        new MetaCase("TC_Camera_Video_0004")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0004")
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
     * Test recording for different quality for front camera.
     */
    @Test
    @FunctionTest
    public void testRecordingForDifferentQualityForFront() {
        new MetaCase("TC_Camera_Video_0004")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0004")
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
     * Test vss in back camera for different quality.
     */
    @Test
    @FunctionTest
    @Video3DNRTest
    public void testVssForDifferentQualityForBack() {
        new MetaCase("TC_Camera_Video_0012/TC_Camera_3DNR_0008")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0012/TC_Camera_3DNR_0008")
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new TakeVideoSnapShotOperator())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .observeEnd()
                .run();
    }

    /**
     * Test vss for back camera with eis on.
     */
    @Test
    @FunctionTest
    public void testVssAndEISForBack() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase()
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addOperator(new EisOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new TakeVideoSnapShotOperator())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .observeEnd()
                .run();
    }


    /**
     * Test recording with night mode for back camera.
     */
    @Test
    @FunctionTest
    public void testNightModeRecordingForBack() {
        new MetaCase("TC_Camera_Video_0003")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0003")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
    }

    /**
     * Test during recording click power key for back camera.
     */
    @Test
    @FunctionTest
    public void testRecordingAndPowerKeyForBack() {
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test power key for video preview.
     */
    @Test
    @FunctionTest
    public void testPowerKeyForVideoPreview() {
        new MetaCase("TC_Camera_Video_0057")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0057")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .run();
    }

    /**
     * Stress test power key for video preview.
     */
    @Test
    @StabilityTest
    public void testStressPowerKeyForVideoPreview() {
        new MetaCase("TC_Camera_Video_0058")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0058")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .repeatEnd()
                .run();
    }

    /**
     * Test during recording click recent app key for back camera.
     */
    @Test
    @FunctionTest
    public void testRecordingAndRecentAppForBack() {
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByRecentAppOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test recording for front camera with night mode.
     */
    @Test
    @FunctionTest
    public void testNightModeRecordingForFront() {
        new MetaCase("TC_Camera_Video_0003")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0003")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
    }

    /**
     * Test during recording click power key for front camera.
     */
    @Test
    @FunctionTest
    public void testRecordingAndPowerKeyForFront() {
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test during recording click recent app key for front camera.
     */
    @Test
    @FunctionTest
    public void testRecordingAndRecentAppForFront() {
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0059")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByRecentAppOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test front camera don't support vss.
     */
    @Test
    @FunctionTest
    public void testRecordingVssForFront() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase()
                .addOperator(new QualityOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addChecker(new VssSupportedChecker(), VssSupportedChecker.INDEX_NOT_SUPPORT)
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test during recording click home key for back camera.
     */
    @Test
    @FunctionTest
    @Video3DNRTest
    public void testRecordingAndHomeKeyForBack() {
        new MetaCase("TC_Camera_Video_0059/TC_Camera_3DNR_0007")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0059/TC_Camera_3DNR_0007")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test during recording click home key for front camera.
     */
    @Test
    @FunctionTest
    public void testRecordingAndHomeKeyForFront() {
        new MetaCase("TC_Camera_Video_0061")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0061")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test during recording click back key for back camera.
     */
    @Test
    @FunctionTest
    public void testRecordingAndBackKeyForBack() {
        new MetaCase("TC_Camera_Video_0061")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0061")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new BackToCameraOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch camera for video mode.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraForVideo() {
        new MetaCase("TC_Camera_Video_0046/TC_Camera_Video_0064")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test during recording zoom and touch focus for different quality for back camera.
     */
    @Test
    @FunctionTest
    public void testRecordingZoomForBackDifferentQuality() {
        new MetaCase("TC_Camera_Video_0005/TC_Camera_Video_0006")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0005/TC_Camera_Video_0006")
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .addOperator(new TouchFocusOperator(), 20)
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    /**
     * Test during recording zoom and touch focus for different quality for front camera.
     */
    @Test
    @FunctionTest
    public void testRecordingZoomForFrontDifferentQuality() {
        new MetaCase("TC_Camera_Video_0006")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .run();
        if (Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_FRONT])) {
            new MetaCase("TC_Camera_Video_0006")
                    .addChecker(new PreviewChecker())
                    .acrossBegin()
                    .addOperator(new QualityOperator())
                    .acrossEnd()
                    .addOperator(new StartRecordOperator(true))
                    .addChecker(new RecordingChecker())
                    .observeBegin(new ZoomUiObserver())
                    .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                    .observeEnd()
                    .addOperator(new TouchFocusOperator(), 20)
                    .observeBegin(new ZoomUiObserver())
                    .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                    .observeEnd()
                    .addOperator(new StopRecordOperator(true))
                    .run();
        } else {
            new MetaCase("TC_Camera_Video_0006")
                    .addChecker(new PreviewChecker())
                    .acrossBegin()
                    .addOperator(new QualityOperator())
                    .acrossEnd()
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
    }

    /**
     * Test zoom for video preview.
     */
    @Test
    @FunctionTest
    public void testZoomForVideoPreview() {
        new MetaCase("TC_Camera_Video_0006")
                .addOperator(new SwitchCameraOperator())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
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
     * Test zoom for video recording along time.
     */
    @Test
    @FunctionTest
    public void testRecordingForLongTime() {
        new MetaCase("TC_Camera_Video_0035")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .run();
        new MetaCase("TC_Camera_Video_0035")
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .runForDuration(Utils.TIME_DURATION_FIVE_MIN);
    }


    /**
     * Stress test switch camera.
     */
    @Test
    @StabilityTest
    public void testStressSwitchCameraForVideo() {
        new MetaCase("TC_Camera_Video_0047")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .repeatEnd()
                .run();
    }

    /**
     * Test microphone for video.
     */
    @Test
    @FunctionTest
    public void testMicroPhoneForVideo() {
        new MetaCase("TC_Camera_Video_0005")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .acrossEnd()
                .acrossBegin()
                .addOperator(new MicroPhoneOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
    }

    /**
     * Test audio mode for video.
     */
    /*@Test
    @FunctionTest
    public void testAudioModeForVideo() {
        new MetaCase("TC_Camera_Video_0010")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_Video_0010")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .acrossEnd()
                .acrossBegin()
                .addOperator(new AudioModeOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
    }*/

    /**
     * Test eis for video.
     */
    @Test
    @FunctionTest
    public void testEisForVideo() {
        new MetaCase("TC_Camera_Video_0010")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_Video_0010")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .acrossEnd()
                .acrossBegin()
                .addOperator(new EisOperator())
                .acrossEnd()
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
        new MetaCase("TC_Camera_Video_0010")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_Video_0010")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
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
     * Test different white balance mode for recording.
     */
    @Test
    @FunctionTest
    public void testAllWhiteBalanceForRecording() {
        new MetaCase("TC_Camera_Video_0010")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_Video_0010")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
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
     * Test different anti flicker mode for recording.
     */
    @Test
    @FunctionTest
    public void testAllAntiFlickerForRecording() {
        new MetaCase("TC_Camera_Video_0010")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_Video_0010")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .acrossBegin()
                .addOperator(new AntiFlickerOperator())
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
     * Test pause/resume recording.
     */
    @Test
    @FunctionTest
    public void testPauseResumeRecording() {
        new MetaCase("TC_Camera_Video_0013")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new PauseResumeRecordingOperator())
                .addChecker(new RecordingPauseChecker())
                .addOperator(new PauseResumeRecordingOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .run();
    }

    /**
     * Test can zoom after goto gallery and back to camera.
     */
    @Test
    @FunctionTest
    public void testZoomAfterBackToCameraFromGallery() {
        new MetaCase("TC_Camera_Video_0016")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new ZoomOperator())
                .run();
    }

    /**
     * Test lomo effect can work for different quality.
     */
    @Test
    @FunctionTest
    public void testMatrixDisplayForDifferentQuality() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0020")
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .addOperator(new MatrixDisplayOpeningOperator())
                .addChecker(new MatrixDisplayOpenedChecker())
                .addOperator(new BackToCameraOperator())
                .run();
    }

    /**
     * Test Matrix display can zoom for different quality.
     */
    @Test
    @FunctionTest
    public void testMatrixDisplayCanZoom() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0020")
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .addOperator(new MatrixDisplayOpeningOperator())
                .addChecker(new MatrixDisplayOpenedChecker())
                .addOperator(new OnSingleTapUpOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .addOperator(new OnSingleTapUpOperator())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .run();
    }

    /**
     * Test touch focus for back camera.
     */
    @Test
    @FunctionTest
    public void testTouchFocusForVideoPreview() {
        new MetaCase("TC_Camera_Video_0032")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0032")
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .addOperator(new TouchFocusOperator(), 25)
                .run();
    }

    /**
     * Test long click shutter button for video mode.
     */
    @Test
    @FunctionTest
    public void testLongClickShutterButtonForVideoPreview() {
        new MetaCase("TC_Camera_Video_0034")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .addOperator(new LongPressShutterOperator())
                .run();
    }

    /**
     * Stress test for recording many times.
     */
    @Test
    @StabilityTest
    public void testManyTimesForRecording() {
        new MetaCase("TC_Camera_Video_0035")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0035")
                .addChecker(new PreviewChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .repeatEnd()
                .run();
    }

    /**
     * Test for thumbnail for recording.
     */
    @Test
    @FunctionTest
    public void testThumbnailForRecording() {
        new MetaCase("TC_Camera_Video_0051")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase("TC_Camera_Video_0051")
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test click thumbnail and back to video.
     */
    @Test
    @FunctionTest
    public void testThumbnailAndBackToVideo() {
        new MetaCase("TC_Camera_Video_0052")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new CameraFacingChecker())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new VideoDurationChecker(), VideoDurationChecker.INDEX_NORMAL)
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test volume key.
     */
    @Test
    @FunctionTest
    public void testVolumeKeyDownForVideo() {
        new MetaCase("TC_Camera_Video_0066")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new VolumeKeyDownOperator())
                .addChecker(new RecordingChecker())
                .addOperator(new VolumeKeyDownOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test ui for video.
     */
    @Test
    @FunctionTest
    public void testCheckVideoUI() {
        new MetaCase("TC_Camera_Video_0026")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new CameraFacingChecker())
                .addChecker(new PreviewChecker())
                .addChecker(new PhotoVideoModeChecker(), PhotoVideoModeChecker.INDEX_VIDEO)
                .addChecker(new SwitchCameraSupportedChecker(),
                        SwitchCameraSupportedChecker.INDEX_SUPPORT)
                .run();
    }


    /**
     * Test ae/af lock video.
     */
    @Test
    @FunctionTest
    public void testAEAFLockForVideoPreview() {
        if (Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator.INDEX_BACK])
                && Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_FRONT])) {
            new MetaCase()
                    .addOperator(new SwitchPhotoVideoOperator(),
                            SwitchPhotoVideoOperator.INDEX_VIDEO)
                    .acrossBegin()
                    .addOperator(new SwitchCameraOperator())
                    .acrossEnd()
                    .addChecker(new PreviewChecker())
                    .addChecker(new PhotoVideoModeChecker(), PhotoVideoModeChecker.INDEX_VIDEO)
                    .addChecker(new SwitchCameraSupportedChecker(),
                            SwitchCameraSupportedChecker.INDEX_SUPPORT)
                    .addOperator(new OnLongPressOperator())
                    .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                    .run();
        } else if (Utils.isFeatureSupported(
                TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator.INDEX_BACK])) {
            new MetaCase()
                    .addOperator(new SwitchPhotoVideoOperator(),
                            SwitchPhotoVideoOperator.INDEX_VIDEO)
                    .addChecker(new PreviewChecker())
                    .addChecker(new PhotoVideoModeChecker(), PhotoVideoModeChecker.INDEX_VIDEO)
                    .addChecker(new SwitchCameraSupportedChecker(),
                            SwitchCameraSupportedChecker.INDEX_SUPPORT)
                    .addOperator(new OnLongPressOperator())
                    .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                    .run();
        }
    }

    @Test
    @FunctionTest
    public void testForIncomingCallForBackCamera() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase()
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new SleepOperator(5))
                .addOperator(new IncomingCallSimulateOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .observeBegin(new AudioPlayedObserver())
                .addOperator(new GoToGalleryPlayVideoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testForIncomingCallForFrontCamera() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase()
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .addOperator(new SleepOperator(5))
                .addOperator(new IncomingCallSimulateOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .observeBegin(new AudioPlayedObserver())
                .addOperator(new GoToGalleryPlayVideoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testRecordAndPlayVideoForFrontCamera() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase()
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new RecordVideoOperator().setDuration(5))
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .observeBegin(new AudioPlayedObserver())
                .addOperator(new GoToGalleryPlayVideoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testRecordAndPlayVideoForBackCamera() {
        new MetaCase()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .run();
        new MetaCase()
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new RecordVideoOperator().setDuration(5))
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .observeBegin(new AudioPlayedObserver())
                .addOperator(new GoToGalleryPlayVideoOperator())
                .observeEnd()
                .run();
    }
}
