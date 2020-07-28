package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.DualCameraTest;
import com.mediatek.camera.tests.v3.annotation.module.DualCameraDeNoiseTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.CapturedPhotoPictureSizeChecker;
import com.mediatek.camera.tests.v3.checker.FlashExifChecker;
import com.mediatek.camera.tests.v3.checker.IsoExifChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.checker.TouchFocusChecker;
import com.mediatek.camera.tests.v3.observer.MediaLocationObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ConnectWifiOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.ForceEnableBmdnDenoiseOperator;
import com.mediatek.camera.tests.v3.operator.ForceEnableMfhrDenoiseOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.IsoOperator;
import com.mediatek.camera.tests.v3.operator.LocationOperator;
import com.mediatek.camera.tests.v3.operator.OnSingleTapUpOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PermissionOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.PressShutterOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.VolumeKeyDownOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@DualCameraTest
@DualCameraDeNoiseTest
public class DenoiseTestCase extends BaseCameraTestCase {
    private static final LogUtil.Tag TAG = Utils.getTestTag(DenoiseTestCase.class.getSimpleName());

    @Test
    @FunctionTest
    public void testCaptureAndRecordAlternateDenoise() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testCaptureAndRecordAlternateDenoise] not support denoise, return");
            return;
        }
        new MetaCase()
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_VIDEO)
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addOperator(new SwitchPhotoVideoOperator(),
                        SwitchPhotoVideoOperator.INDEX_PHOTO)
                .repeatBegin(10)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addChecker(new PreviewChecker())
                .repeatEnd()
                .run();
    }

    /**
     * Test enter and exit function.
     */
    @Test
    @FunctionTest
    public void testEntryAndExit() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testEntryAndExit] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_001")
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test Entry and exit stability.
     */
    @Test
    @StabilityTest
    public void testEntryAndExitStability() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testEntryAndExitStability] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_002")
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test switch mode between photo and video.
     */
    @Test
    @FunctionTest
    public void testSwitchMode() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testSwitchMode] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_004")
                .addChecker(new PreviewChecker())

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)

                .run();
    }

    /**
     * Test switch mode between photo and video.
     */
    @Test
    @FunctionTest
    public void testSingleTouchScreen() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testSingleTouchScreen] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_006")
                .addChecker(new PreviewChecker())
                .addOperator(new OnSingleTapUpOperator())
                .addChecker(new TouchFocusChecker())
                .addOperator(new PressShutterOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture function.
     */
    @Test
    @FunctionTest
    public void testCapture() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testCapture] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_007")
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test capture function stability.
     */
    @Test
    @StabilityTest
    public void testCaptureStability() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testCaptureStability] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_008")
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test capture with different picture size.
     */
    @Test
    @FunctionTest
    public void testCaptureWithDifferentSize() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testCaptureWithDifferentSize] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_012")
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
     * Test capture with location information.
     */
//    @Test
//    @FunctionTest
//    public void testCaptureWithLocation() {
//        if (!isDenoiseSupported()) {
//            LogHelper.d(TAG, "[testCaptureWithLocation] not support denoise, return");
//            return;
//        }
//        new MetaCase("TC_Camera_DualCameraDenoise_013")
//                .addChecker(new PreviewChecker())
//                .addOperator(new ConnectWifiOperator(), ConnectWifiOperator.INDEX_CONNECT)
//                .addOperator(new LocationOperator(), LocationOperator.INDEX_ON_HIGH_ACCURACY)
//                .addOperator(new PermissionOperator(), PermissionOperator.INDEX_ENABLE_ALL)
//
//                .observeBegin(new MediaLocationObserver(), MediaLocationObserver.INDEX_PHOTO)
//                .observeBegin(new PhotoSavedObserver(1))
//                .addOperator(new CapturePhotoOperator())
//                .observeEnd()
//                .observeEnd()
//
//                .addChecker(new PreviewChecker())
//                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
//                .run();
//    }

    /**
     * Test capture with different picture size, flash and iso setting.
     */
    @Test
    @FunctionTest
    public void testCaptureExif() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testCaptureExif] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_014")
                .addOperator(new PictureSizeOperator())
                .acrossBegin()
                .addOperator(new FlashOperator())
                .acrossBegin()
                .addOperator(new IsoOperator())

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()

                .addChecker(new IsoExifChecker())
                .acrossEnd()
                .addChecker(new FlashExifChecker())
                .acrossEnd()

                .addChecker(new CapturedPhotoPictureSizeChecker())
                .run();
    }

    /**
     * Test switch camera.
     */
    @Test
    @FunctionTest
    public void testSwitchCamera() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testSwitchCamera] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_015")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test switch camera stability.
     */
    @Test
    @StabilityTest
    public void testSwitchCameraStability() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testSwitchCameraStability] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_016")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test touch focus and capture.
     */
    @Test
    @FunctionTest
    public void testCaptureWithTouchFocus() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testCaptureWithTouchFocus] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_019")
                .addChecker(new PreviewChecker())
                .addOperator(new OnSingleTapUpOperator())

                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    /**
     * Test pause/resume with home key and power key.
     */
    @Test
    @FunctionTest
    public void testPauseAndResume() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testPauseAndResume] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_020")
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause/resume with home key and power key stability.
     */
    @Test
    @StabilityTest
    public void testPauseAndResumeStability() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testPauseAndResumeStability] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_021")
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    /**
     * Test pause when capturing.
     */
    @Test
    @FunctionTest
    public void testPauseWhenCapturing() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testPauseWhenCapturing] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_022")
                .addChecker(new PreviewChecker())
                .addOperator(new CapturePhotoOperator())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test pause when switch camera.
     */
    @Test
    @FunctionTest
    public void testPauseWhenSwitchCamera() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testPauseWhenSwitchCamera] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_023")
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Test capture with volume key.
     */
    @Test
    @FunctionTest
    public void testCaptureWithVolumeKey() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testCaptureWithVolumeKey] not support denoise, return");
            return;
        }
        new MetaCase("TC_Camera_DualCameraDenoise_024")
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new VolumeKeyDownOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testBmdnDenoiseCapture() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testBmdnDenoiseCapture] not support denoise, return");
            return;
        }

        new MetaCase()
                .addOperator(new ForceEnableBmdnDenoiseOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                // TODO check
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testMfhrDenoiseCapture() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testMfhrDenoiseCapture] not support denoise, return");
            return;
        }

        new MetaCase()
                .addOperator(new ForceEnableMfhrDenoiseOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                // TODO check
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    public void testPlaybackDenoisePhoto() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testPlaybackDenoisePhoto] not support denoise, return");
            return;
        }

        new MetaCase()
                .addOperator(new ForceEnableMfhrDenoiseOperator())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testBackToDenoiseAfterCapture() {
        if (!isDenoiseSupported()) {
            LogHelper.d(TAG, "[testBackToDenoiseAfterCapture] not support denoise, return");
            return;
        }

        new MetaCase()
                .addOperator(new ForceEnableMfhrDenoiseOperator())
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

    public static boolean isDenoiseSupported() {
        return SystemProperties.getInt("ro.vendor.mtk_cam_dualdenoise_support", 0) == 1;
    }
}
