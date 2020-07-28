package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.ContinuousShotTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.operator.ContinuousShotOperator;

import org.junit.Test;

/**
 * For continuous shot test in intent mode.
 */

@CameraBasicTest
@ContinuousShotTest
public class ContinuousShotIntentTestCase extends BaseIntentPhotoTestCase {

    @Test
    @FunctionTest
    public void testCshotIntentCapture() {
        new MetaCase("TC_Camera_ContinuousShot_0014")
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_NO_SAVED)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .run();
    }
}
