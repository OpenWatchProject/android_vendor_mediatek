package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.FaceDetectionTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.observer.FaceDetectionLogObserver;
import com.mediatek.camera.tests.v3.operator.ContinuousShotOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;

import org.junit.Test;

@CameraFeatureTest
@FaceDetectionTest
public class FaceDetectionTestCase extends BaseCameraTestCase {

    @Test
    @FunctionTest
    public void testDisableFdWhenContinuousShot() {
        new MetaCase("TC_Camera_facedetection_0011")
                .observeBegin(new FaceDetectionLogObserver(),
                        FaceDetectionLogObserver.INDEX_DISABLE_ENABLE)
                .addOperator(new ContinuousShotOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testRestrictionWithOtherMode() {
        new MetaCase("TC_Camera_facedetection_0010")
                .observeBegin(new FaceDetectionLogObserver(),
                        FaceDetectionLogObserver.INDEX_DISABLE)
                .addOperator(new SwitchToNormalVideoModeOperator())
                .observeEnd()

                .observeBegin(new FaceDetectionLogObserver(),
                        FaceDetectionLogObserver.INDEX_DISABLE)
                .addOperator(new SwitchToPanoramaModeOperator())
                .observeEnd()

                .observeBegin(new FaceDetectionLogObserver(),
                        FaceDetectionLogObserver.INDEX_DISABLE)
                .addOperator(new SwitchToSlowMotionModeOperator())
                .observeEnd()

                .observeBegin(new FaceDetectionLogObserver(),
                        FaceDetectionLogObserver.INDEX_DISABLE)
                .addOperator(new SwitchToPipPhotoModeOperator())
                .observeEnd()

                .addOperator(new SwitchToNormalVideoModeOperator())
                .observeBegin(new FaceDetectionLogObserver(),
                        FaceDetectionLogObserver.INDEX_DISABLE)
                .addOperator(new SwitchToPipVideoModeOperator())
                .observeEnd()
                .run();
    }

}
