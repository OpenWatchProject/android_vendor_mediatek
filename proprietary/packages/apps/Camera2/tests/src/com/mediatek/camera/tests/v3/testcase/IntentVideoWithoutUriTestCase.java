package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.ThirdPartyLaunchVideoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.VideoReviewUIChecker;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.VideoReviewUIOperator;

import org.junit.Test;

@CameraBasicTest
@ThirdPartyLaunchVideoTest
public class IntentVideoWithoutUriTestCase extends BaseIntentVideoWithoutUriTestCase {
    @Test
    @FunctionTest
    public void testIntentVideoWithoutUri() {
        new MetaCase("TC_Camera_Intent_Video_0008")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(true))
                .addChecker(new VideoReviewUIChecker())
                .addOperator(
                        new VideoReviewUIOperator(), VideoReviewUIOperator.REVIEW_OPERATION_PLAY)
                .run();
    }
}
