package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.DualCameraTest;
import com.mediatek.camera.tests.v3.annotation.module.VsdofTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.SanityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SleepOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoVideoModeOperator;

import org.junit.Test;

/**
 * Vsdof sanity test cases.
 */
@DualCameraTest
@VsdofTest
public class BokehSanityTestCase extends BaseCameraTestCase {
    private static final int SLEEP_TIME = 10;


    /**
     * Switch to vsdof photo mode, preview 10s.
     */
    @Test
    @SanityTest
    @FunctionTest
    @DualCameraTest
    public void testVsdofPreview() {
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SleepOperator(SLEEP_TIME))
                .run();
    }

    /**
     * Switch to vsdof photo mode, take picture.
     */
    @Test
    @SanityTest
    @FunctionTest
    @DualCameraTest
    public void testVsdofCapture() {
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    /**
     * Switch to vsdof video mode, preview 10s.
     */
    @Test
    @SanityTest
    @FunctionTest
    @DualCameraTest
    public void testVsdofVideoPreview() {
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SleepOperator(SLEEP_TIME))
                .run();
    }

    /**
     * Switch to vsdof video mode, record one video 10s.
     */
    @Test
    @SanityTest
    @FunctionTest
    @DualCameraTest
    public void testVsdofRecord() {
        new MetaCase()
                .addOperator(new SwitchToStereoPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator().setDuration(SLEEP_TIME))
                .observeEnd()
                .run();
    }
}
