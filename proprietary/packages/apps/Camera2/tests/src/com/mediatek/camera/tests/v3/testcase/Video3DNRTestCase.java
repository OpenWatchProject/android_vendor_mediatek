package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.Video3DNRTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayOpenedChecker;
import com.mediatek.camera.tests.v3.observer.Video3DNRLogObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.MatrixDisplayEffectSelectOperator;
import com.mediatek.camera.tests.v3.operator.MatrixDisplayOpeningOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.QualityOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SleepOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.ZsdOperator;

import org.junit.Test;

@CameraFeatureTest
@Video3DNRTest
public class Video3DNRTestCase extends BaseCameraTestCase {
    @Test
    @FunctionTest
    public void testRestrictionWithOtherMode() {
        new MetaCase("TC_Camera_3DNR_0009")
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_OFF)
                .addOperator(new MatrixDisplayOpeningOperator())
                .addChecker(new MatrixDisplayOpenedChecker())
                .addOperator(new MatrixDisplayEffectSelectOperator(),
                        MatrixDisplayEffectSelectOperator.INDEX_EFFECT_MODE_AQUA)
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .run();

//        new MetaCase("TC_Camera_3DNR_0009")
//                .observeBegin(new Video3DNRLogObserver().setCameraApi(CameraDeviceManagerFactory
//                        .CameraApi.API2), Video3DNRLogObserver.INDEX_OFF)
//                .addOperator(new SwitchToSlowMotionModeOperator())
//                .observeEnd()
//                .addOperator(new PauseResumeByBackKeyOperator())
//                .run();

        new MetaCase("TC_Camera_3DNR_0009")
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_OFF)
                .addOperator(new SwitchToPipPhotoModeOperator())
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .run();

        new MetaCase("TC_Camera_3DNR_0009")
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_OFF)
                .addOperator(new SwitchToPipVideoModeOperator())
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .run();

        new MetaCase("TC_Camera_3DNR_0009")
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_OFF)
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .run();

        new MetaCase("TC_Camera_3DNR_0009")
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_OFF)
                .addOperator(new SwitchToStereoVideoModeOperator())
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .run();

        new MetaCase("TC_Camera_3DNR_0009")
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_OFF)
                .addOperator(new SwitchToStereoModeOperator())
                .observeEnd()
                .addOperator(new PauseResumeByBackKeyOperator())
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testForceEnable3DNRRecording() {
        new MetaCase()
                .addOperator(new SwitchToNormalVideoModeOperator())
                .run();

        new MetaCase()
                .addOperator(new QualityOperator())
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_ON)
                .observeBegin(new VideoSavedObserver(false))
                .addOperator(new RecordVideoOperator().setDuration(10))
                .observeEnd()
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testForceEnable3DNRPreview() {
        new MetaCase()
                .observeBegin(new Video3DNRLogObserver(), Video3DNRLogObserver.INDEX_ON)
                .addOperator(new PauseResumeByBackKeyOperator())
                .observeEnd()
                .run();

        new MetaCase()
                .repeatBegin(2)
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_ON)
                // sleep 10s must
                .addOperator(new SleepOperator(10))
                .addOperator(new ZsdOperator(), ZsdOperator.INDEX_SWITCH_OFF)
                // sleep 10s must
                .addOperator(new SleepOperator(10))
                .repeatEnd()
                .run();
    }
}
