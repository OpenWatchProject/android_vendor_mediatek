package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.DigitalZoomTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraFacingChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.CaptureOrRecordOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * For digital zoom test.
 */
@CameraBasicTest
@DigitalZoomTest
public class ZoomTestCase extends BaseCameraTestCase {

    @Test
    @FunctionTest
    public void testBasicZoomBack() {
        new MetaCase("TC_Camera_zoom_0001")
                .observeBegin(new ZoomUiObserver())
                .acrossBegin()
                .addOperator(new ZoomOperator())
                .acrossEnd()
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testBasicZoomFront() {
        new MetaCase("TC_Camera_zoom_0002")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new CameraFacingChecker(), CameraFacingChecker.INDEX_FRONT)
                .observeBegin(new ZoomUiObserver())
                .acrossBegin()
                .addOperator(new ZoomOperator())
                .acrossEnd()
                .observeEnd()
                .run();
    }

    @Test
    @StabilityTest
    public void testZoomWithCapture() {
        new MetaCase("TC_Camera_zoom_0003, TC_Camera_zoom_0004, TC_Camera_zoom_0008")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addChecker(new PreviewChecker())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_ONE_SAVED)
                .addOperator(new CaptureOrRecordOperator())
                .observeEnd()
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testZoomWithBackKey() {
        new MetaCase("TC_Camera_zoom_0005")
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .run();
    }

    @Test
    @StabilityTest
    public void testZoomWithRepeatCapture() {
        new MetaCase("TC_Camera_zoom_0006")
                .observeBegin(new ZoomUiObserver())
                .acrossBegin()
                .addOperator(new ZoomOperator())
                .acrossEnd()
                .observeEnd()
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testZoomWithCameraSwitch() {
        new MetaCase("TC_Camera_zoom_0010")
                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .run();
    }
}
