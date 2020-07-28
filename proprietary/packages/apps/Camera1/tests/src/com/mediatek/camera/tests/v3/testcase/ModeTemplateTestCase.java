package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.ModeCommonTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.ExifChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.observer.ThumbnailUpdatedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CaptureOrRecordOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.FontSizeOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.LongPressShutterOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.OnSingleTapUpOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PressShutterOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToModeOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraBasicTest
@ModeCommonTest
public class ModeTemplateTestCase extends BaseCameraTestCase {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ModeTemplateTestCase.class
            .getSimpleName());

    @Test
    @StabilityTest
    public void testEnterExitCameraStress() {
        new MetaCase()
                .addOperator(new ExitCameraOperator().ignoreBothPageCheck())
                .addOperator(new LaunchCameraOperator().ignoreBothPageCheck(),
                        LaunchCameraOperator.INDEX_NORMAL)
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testSwitchModeWithNormalMode() {
        new MetaCase("TEMP_MODE_0001")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, false, false))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToModeOperator("Normal"))
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @StabilityTest
    public void testSwitchModeWithNormalModeStress() {
        new MetaCase("TEMP_MODE_0002")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, false, false))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToModeOperator("Normal"))
                .addChecker(new PreviewChecker())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testAcrossSwitchAllMode() {
        Operator switchAllModeInPhotoVideoOperator
                = new SwitchAllModeInPhotoVideoOperator(true, true, true);
        int[] across1 = new int[switchAllModeInPhotoVideoOperator.getOperatorCount()];
        int[] across2 = new int[switchAllModeInPhotoVideoOperator.getOperatorCount()];
        for (int i = 0; i < switchAllModeInPhotoVideoOperator.getOperatorCount(); i++) {
            across1[i] = i;
            across2[i] = switchAllModeInPhotoVideoOperator.getOperatorCount() - i - 1;
        }

        new MetaCase("TEMP_MODE_0004")
                .addOperator(switchAllModeInPhotoVideoOperator, across1)
                .addOperator(switchAllModeInPhotoVideoOperator, across2)
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testSwitchModeWithNormalModeInMaxFontSize() {
        new MetaCase("TEMP_MODE_0008")
                .addOperator(new FontSizeOperator(), FontSizeOperator.INDEX_HUGE).run();

        new MetaCase("TEMP_MODE_0008")
                .addOperator(new SwitchAllModeOperator(true))
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToModeOperator("Normal"))
                .acrossBegin()
                .addOperator(new SwitchPhotoVideoOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .run();

        new MetaCase("TEMP_MODE_0008")
                .addOperator(new FontSizeOperator(), FontSizeOperator.INDEX_NORMAL).run();
    }

    @Test
    @FunctionTest
    public void testPressOnPreview() {
        Operator touchFocusOperator = new OnSingleTapUpOperator();

        new MetaCase("TEMP_MODE_0011")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(touchFocusOperator, touchFocusOperator.getOperatorCount() / 2)
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testLongPressOnPreview() {
        Operator aeAfLockOperator = new OnLongPressOperator();

        new MetaCase("TEMP_MODE_0012")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(aeAfLockOperator, aeAfLockOperator.getOperatorCount() / 2)
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testPressShutter() {
        new MetaCase("TEMP_MODE_0013")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(new PressShutterOperator())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testLongPressShutter() {
        new MetaCase("TEMP_MODE_0014")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(new LongPressShutterOperator())
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testZoomOnPreview() {
        new MetaCase("TEMP_MODE_0015")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testZoomDuringRecording() {
        new MetaCase("TEMP_MODE_0016")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(false, true, true))
                .addOperator(new StartRecordOperator(true))
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .addOperator(new StopRecordOperator(true))
                .run();
    }

    @Test
    @FunctionTest
    public void testCaptureAndRecord() {
        new MetaCase("TEMP_MODE_0017/TEMP_MODE_0041/TEMP_MODE_0042/TEMP_MODE_0043/TEMP_MODE_0058")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .observeBegin(new ThumbnailUpdatedObserver())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_ONE_SAVED)
                .addOperator(new CaptureOrRecordOperator())
                .observeEnd()
                .observeEnd()
                .addChecker(new ExifChecker())
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @StabilityTest
    public void testCaptureAndRecordStress() {
        new MetaCase("TEMP_MODE_0018")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addChecker(new PreviewChecker())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_ONE_SAVED)
                .addOperator(new CaptureOrRecordOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testPauseResume() {
        new MetaCase("TEMP_MODE_0048")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @StabilityTest
    public void testPauseResumeStress() {
        new MetaCase("TEMP_MODE_0049")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testExitWhenPreview() {
        new MetaCase("TEMP_MODE_0051")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_NORMAL)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .run();
    }
}
