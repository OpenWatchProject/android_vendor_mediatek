package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.SecureCameraTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.DngIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.NoImagesVideosChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ClearImagesVideosOperator;
import com.mediatek.camera.tests.v3.operator.ContinuousShotOperator;
import com.mediatek.camera.tests.v3.operator.DngOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;

import org.junit.Test;

/**
 * For secure camera auto test.
 */

@CameraBasicTest
@SecureCameraTest
public class SecureCameraTestCase extends BaseSecureCameraTestCase {

    @Test
    @FunctionTest
    public void testSecureCameraCapture() {
        new MetaCase("TC_Camera_SecureCamera_0001, TC_Camera_SecureCamera_0002")
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_NO_THUMB)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();

    }

    @Test
    @FunctionTest
    public void testSecureCameraPauseResume() {
        new MetaCase("TC_Camera_SecureCamera_0003")
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_SECURE_CAMERA)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_SECURE_CAMERA)
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_NO_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    public void testSecureCameraWitchGallery() {
        new MetaCase("TC_Camera_SecureCamera_0004")
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new ClearImagesVideosOperator())
                .addChecker(new NoImagesVideosChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_NO_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    public void testSecureCameraDngCapture() {
        new MetaCase("TC_Camera_dng_0016")
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addChecker(new DngIndicatorChecker())
                .observeBegin(new PhotoSavedObserver(2))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testSecureCameraCSPauseResume() {
        new MetaCase("TC_Camera_SecureCamera_0005")
                .addChecker(new PreviewChecker())
                .addOperator(new ContinuousShotOperator(5000, false).ignoreBothPageCheck())
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_SECURE_CAMERA)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_SECURE_CAMERA)
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_NO_THUMB)
                .run();
    }

    @Test
    @FunctionTest
    public void testSecureCameraRecord() {
        new MetaCase("TC_Camera_SecureCamera_0006")
                .addChecker(new PreviewChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_NO_THUMB)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .run();

    }
}
