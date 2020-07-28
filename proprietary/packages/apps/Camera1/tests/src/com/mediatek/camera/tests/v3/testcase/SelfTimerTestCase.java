package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.SelfTimerTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraFacingChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SelfTimerIndicatorChecker;
import com.mediatek.camera.tests.v3.observer.SelfTimerPhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.SelfTimerViewObserver;
import com.mediatek.camera.tests.v3.operator.AlarmClockOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.CloseAlarmClockOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SelfTimerOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;

import org.junit.Test;

/**
 * For selftimer auto test.
 */

@CameraBasicTest
@SelfTimerTest
public class SelfTimerTestCase extends BaseCameraTestCase {

    @Test
    @FunctionTest
    public void testSelfTimerCapture() {
        new MetaCase("TC_Camera_SelfTimer_0001")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new CameraFacingChecker())
                .acrossBegin()
                .addOperator(new SelfTimerOperator())
                .observeBegin(new SelfTimerViewObserver())
                .observeBegin(new SelfTimerPhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .acrossEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testSelfTimerCaptureWithAlarm() {
        new MetaCase("TC_Camera_SelfTimer_0007")
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .addOperator(new AlarmClockOperator(9))
                .addChecker(new PreviewChecker())
                .observeBegin(new SelfTimerPhotoSavedObserver(1),
                        SelfTimerPhotoSavedObserver.SELF_TIMER_10)
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addOperator(new CloseAlarmClockOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testSelfTimerSettingWithModes() {
        new MetaCase("TC_Camera_SelfTimer_0003")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_2_SECONDS)
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .acrossBegin()
                .addOperator(new SwitchAllModeInPhotoVideoOperator(false, true, true))
                .acrossEnd()
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_HIDE)
                .run();
    }

    @Test
    @FunctionTest
    public void testSelfTimerWithHomeKey() {
        new MetaCase("TC_Camera_SelfTimer_0004")
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .observeBegin(new SelfTimerViewObserver())
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .run();
    }

    @Test
    @FunctionTest
    public void testSelfTimerValueWithSwitchCamera() {
        new MetaCase("TC_Camera_SelfTimer_0011")
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .run();
    }

    @Test
    @FunctionTest
    public void testSelfTimerValueWithBackKey() {
        new MetaCase("TC_Camera_SelfTimer_0012")
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_HIDE)
                .run();
    }
}
