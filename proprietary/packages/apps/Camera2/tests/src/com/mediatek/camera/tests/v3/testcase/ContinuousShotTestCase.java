package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.ContinuousShotTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.ContinuousShotNameObserver;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.ContinuousShotOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PictureSizeOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchModesOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * For continuous shot test.
 */
@CameraBasicTest
@ContinuousShotTest
public class ContinuousShotTestCase extends BaseCameraTestCase {

    @Test
    @StabilityTest
    public void testCShotCapture() {
        new MetaCase("TC_Camera_ContinuousShot_0001")
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_MULTI_SAVED)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testCshotFrontCameraCapture() {
        new MetaCase("TC_Camera_ContinuousShot_0002")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_NO_SAVED)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testCshotImageName() {
        new MetaCase("TC_Camera_ContinuousShot_0005")
                .observeBegin(new ContinuousShotNameObserver())
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testCshotWithShortTime() {
        new MetaCase("TC_Camera_ContinuousShot_0003, TC_Camera_ContinuousShot_0008")
                .addOperator(new ContinuousShotOperator(500, true))
                .run();
    }

    @Test
    @FunctionTest
    public void testCshotWithThumbnail() {
        new MetaCase("TC_Camera_ContinuousShot_0004")
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_MULTI_SAVED)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testCshotWithPictureSize() {
        new MetaCase("TC_Camera_ContinuousShot_0006")
                .addOperator(new PictureSizeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_MULTI_SAVED)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testCshotRestrictionWithMode() {
        new MetaCase("TC_Camera_ContinuousShot_0010")
                .addOperator(new SwitchModesOperator(new String[]{"Normal", "PIP"}))
                .addChecker(new PreviewChecker())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_NO_SAVED)
                .addOperator(new ContinuousShotOperator(1000, false).ignoreBothPageCheck())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testCshotRestrictionWithASD() {
        new MetaCase("TC_Camera_ContinuousShot_0011")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_AUTO)
                .addChecker(new PreviewChecker())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_MULTI_SAVED)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .run();
    }

    @Test
    @StabilityTest
    public void testCshotWithPauseResumeCamera() {
        new MetaCase("TC_Camera_ContinuousShot_0013")
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_MULTI_SAVED)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }


}
