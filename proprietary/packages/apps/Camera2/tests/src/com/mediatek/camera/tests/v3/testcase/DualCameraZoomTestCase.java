package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.DualCameraTest;
import com.mediatek.camera.tests.v3.annotation.module.DualCameraZoomTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraFacingChecker;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.DualZoomBarValueChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.DualZoomRatioObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.DragDualZoomBarOperator;
import com.mediatek.camera.tests.v3.operator.ForceEnableTeleCameraOperator;
import com.mediatek.camera.tests.v3.operator.ForceEnableWildCameraOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPipOperator;
import com.mediatek.camera.tests.v3.operator.SwitchTeleAndWildOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.TakeVideoSnapShotOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Dual camera zoom test case.
 */
@DualCameraTest
@DualCameraZoomTest
public class DualCameraZoomTestCase extends BaseCameraTestCase {
    private static final LogUtil.Tag TAG
            = Utils.getTestTag(DualCameraZoomTestCase.class.getSimpleName());
    /**
     * Test switch mode between photo and video in normal mode.
     */
    @Test
    @FunctionTest
    public void testSwitchPhotoAndVideoInNormal() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchPhotoAndVideoInNormal] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0001")
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0001")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch all mode in normal photo mode.
     */
    @Test
    @FunctionTest
    public void testSwitchAllModeInNormalPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchAllModeInNormalPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0002")
                .acrossBegin()
                .addOperator(new SwitchAllModeOperator(true))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator())
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0002")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch all mode in normal video mode.
     */
    @Test
    @FunctionTest
    public void testSwitchAllModeInNormalVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchAllModeInNormalVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0003")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchAllModeOperator(true))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator())
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0003")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch camera in normal photo mode.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraInNormalPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchCameraInNormalPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0004")
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0004")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch camera in normal video mode.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraInNormalVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchCameraInNormalVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0005")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0005")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test pause and resume device in normal photo mode.
     */
    @Test
    @FunctionTest
    public void testPauseAndResumeInNormalPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPauseAndResumeInNormalPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0006")
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0006")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause and resume device in normal video mode.
     */
    @Test
    @FunctionTest
    public void testPauseAndResumeInNormalVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPauseAndResumeInNormalVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0007")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0007")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture different zoom ratio in normal photo mode.
     */
    @Test
    @FunctionTest
    public void testCaptureDifferentZoomRatioInNormal() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG,
                    "[testCaptureDifferentZoomRatioInNormal] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0008")
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test record different zoom ratio in normal video mode.
     */
    @Test
    @FunctionTest
    public void testRecordDifferentZoomRatioInNormal() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG,
                    "[testRecordDifferentZoomRatioInNormal] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0009")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())

                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .acrossBegin()
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new TakeVideoSnapShotOperator())
                .observeEnd()
                .observeEnd()
                .acrossEnd()
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pinch to zoom in normal photo mode.
     */
    @Test
    @FunctionTest
    public void testPinchToZoomInNormalPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPinchToZoomInNormalPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0010")
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new DualZoomRatioObserver(false))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .run();
    }

    /**
     * Test pinch to zoom in normal video mode.
     */
    @Test
    @FunctionTest
    public void testPinchToZoomInNormalVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPinchToZoomInNormalVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0011")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new DualZoomRatioObserver(false))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .run();
    }

    /**
     * Test switch tele and wild in normal photo mode.
     */
    @Test
    @FunctionTest
    public void testSwitchTeleAndWildInNormalPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchTeleAndWildInNormalPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0012")
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_TELE)
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_WILD)
                .runForTimes(3);
    }

    /**
     * Test switch tele and wild in normal video mode.
     */
    @Test
    @FunctionTest
    public void testSwitchTeleAndWildInNormalVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchTeleAndWildInNormalVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0013")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_TELE)
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_WILD)
                .runForTimes(3);
    }

    /**
     * Test switch mode between photo and video in pip mode.
     */
    @Test
    @FunctionTest
    public void testSwitchPhotoAndVideoInPip() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchPhotoAndVideoInPip] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0018")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0018")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch all mode in pip photo mode.
     */
    @Test
    @FunctionTest
    public void testSwitchAllModeInPipPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchAllModeInPipPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0019")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchAllModeOperator(true))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator())
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0019")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch all mode in pip video mode.
     */
    @Test
    @FunctionTest
    public void testSwitchAllModeInPipVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchAllModeInPipVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0020")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchAllModeOperator(true))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator())
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0020")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch camera in pip photo mode.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraInPipPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchCameraInPipPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0021")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPipOperator(), SwitchPipOperator.INDEX_TOP_GRAPHIC)
                .addChecker(new PreviewChecker())
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0021")
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0021")
                .addOperator(new SwitchPipOperator(), SwitchPipOperator.INDEX_TOP_GRAPHIC)
                .addChecker(new PreviewChecker())
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0021")
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test switch camera in pip video mode.
     */
    @Test
    @FunctionTest
    public void testSwitchCameraInPipVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchCameraInPipVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0022")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPipOperator(), SwitchPipOperator.INDEX_TOP_GRAPHIC)
                .addChecker(new PreviewChecker())
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0022")
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0022")
                .addOperator(new SwitchPipOperator(), SwitchPipOperator.INDEX_TOP_GRAPHIC)
                .addChecker(new PreviewChecker())
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0022")
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();
    }

    /**
     * Test pause and resume device in pip photo mode.
     */
    @Test
    @FunctionTest
    public void testPauseAndResumeInPipPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPauseAndResumeInPipPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0023")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0023")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause and resume device in pip video mode.
     */
    @Test
    @FunctionTest
    public void testPauseAndResumeInPipVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPauseAndResumeInPipVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0024")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0024")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture different zoom ratio in pip photo mode.
     */
    @Test
    @FunctionTest
    public void testCaptureDifferentZoomRatioInPip() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testCaptureDifferentZoomRatioInPip] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0025")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeEnd()
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test record different zoom ratio in pip video mode.
     */
    @Test
    @FunctionTest
    public void testRecordDifferentZoomRatioInPip() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testRecordDifferentZoomRatioInPip] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0026")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())

                .observeBegin(new VideoSavedObserver())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new RecordingChecker())
                .acrossBegin()
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new DragDualZoomBarOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new TakeVideoSnapShotOperator())
                .observeEnd()
                .observeEnd()
                .acrossEnd()
                .addOperator(new StopRecordOperator(true))
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pinch to zoom in pip photo mode.
     */
    @Test
    @FunctionTest
    public void testPinchToZoomInPipPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPinchToZoomInPipPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0027")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new DualZoomRatioObserver(false))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .run();
    }

    /**
     * Test pinch to zoom in pip video mode.
     */
    @Test
    @FunctionTest
    public void testPinchToZoomInPipVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPinchToZoomInPipVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0028")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new DualZoomRatioObserver(true))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new DualZoomRatioObserver(false))
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .run();
    }

    /**
     * Test switch tele and wild in pip photo mode.
     */
    @Test
    @FunctionTest
    public void testSwitchTeleAndWildInPipPhoto() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchTeleAndWildInPipPhoto] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0029")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_TELE)
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_WILD)
                .runForTimes(3);
    }

    /**
     * Test switch tele and wild in pip video mode.
     */
    @Test
    @FunctionTest
    public void testSwitchTeleAndWildInPipVideo() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchTeleAndWildInPipVideo] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0030")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_TELE)
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_WILD)
                .runForTimes(3);
    }

    /**
     * Test switch all mode in panoroma mode.
     */
    @Test
    @FunctionTest
    public void testSwitchAllModeInPanoroma() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testSwitchAllModeInPanoroma] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0032")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchAllModeOperator(true))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator())
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0032")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause and resume device in panoroma mode.
     */
    @Test
    @FunctionTest
    public void testPauseAndResumeInPanorama() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testPauseAndResumeInPanorama] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0033")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0033")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture different zoom ratio in panorama mode.
     */
    @Test
    @FunctionTest
    public void testCaptureDifferentZoomRatioInPanorama() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG,
                    "[testCaptureDifferentZoomRatioInPanorama] not support dual zoom, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraZoom_0034")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();

        new MetaCase("TC_Camera_DualCameraZoom_0034")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_TELE)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testRecordWithWild() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testRecordWithWild] not support dual zoom, return");
            return;
        }
        new MetaCase()
                .addOperator(new ForceEnableWildCameraOperator())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testRecordWithTele() {
        if (!Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom")) {
            LogHelper.d(TAG, "[testRecordWithTele] not support dual zoom, return");
            return;
        }
        new MetaCase()
                .addOperator(new ForceEnableTeleCameraOperator())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addOperator(new SwitchTeleAndWildOperator())
                .addChecker(new DualZoomBarValueChecker(), DualZoomBarValueChecker.INDEX_IS_TELE)
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }
}
