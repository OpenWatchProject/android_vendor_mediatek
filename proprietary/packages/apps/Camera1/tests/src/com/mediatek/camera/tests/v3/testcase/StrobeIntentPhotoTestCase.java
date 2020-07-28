package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.StrobeTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.FlashQuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.observer.FlashLogObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;

import org.junit.Test;

@CameraFeatureTest
@StrobeTest
public class StrobeIntentPhotoTestCase extends BaseIntentPhotoTestCase {
    @Test
    @FunctionTest
    public void testStrobeSupported() {
        new MetaCase("TC_Camera_Strobe_0018")
                .addChecker(new FlashQuickSwitchExistedChecker(),
                        FlashQuickSwitchExistedChecker.INDEX_EXIST)
                .addOperator(new FlashOperator())
                .observeBegin(new FlashLogObserver())
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .run();
    }
}
