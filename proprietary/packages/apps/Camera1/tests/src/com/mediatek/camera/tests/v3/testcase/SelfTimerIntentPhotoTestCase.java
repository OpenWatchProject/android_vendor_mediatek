package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.SelfTimerTest;
import com.mediatek.camera.tests.v3.annotation.module.ThirdPartyLaunchPhotoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SelfTimerIndicatorChecker;
import com.mediatek.camera.tests.v3.observer.SelfTimerViewObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.SelfTimerOperator;

import org.junit.Test;

/**
 * For self timer check in intent photo mode.
 */
@CameraBasicTest
@SelfTimerTest
@ThirdPartyLaunchPhotoTest
public class SelfTimerIntentPhotoTestCase extends BaseIntentPhotoTestCase {

    @Test
    @FunctionTest
    public void testIntentSelfTimerCapture() {
        new MetaCase("TC_Camera_SelfTimer_0014")
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .observeBegin(new SelfTimerViewObserver(), SelfTimerViewObserver.TIMER_10_VIEW)
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentSelfTimerWithPowerKey() {
        new MetaCase("TC_Camera_SelfTimer_0008")
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .observeBegin(new SelfTimerViewObserver(), SelfTimerViewObserver.TIMER_10_VIEW)
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new SelfTimerIndicatorChecker(), SelfTimerIndicatorChecker.INDEX_SHOW)
                .run();
    }
}
