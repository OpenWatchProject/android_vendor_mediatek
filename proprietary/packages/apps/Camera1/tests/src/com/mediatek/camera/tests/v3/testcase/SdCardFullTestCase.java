package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.DngTest;
import com.mediatek.camera.tests.v3.annotation.module.ModeCommonTest;
import com.mediatek.camera.tests.v3.annotation.module.StorageTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.DngIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.IndicatorChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.StorageHintChecker;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.operator.CaptureOrRecordOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.DngOperator;
import com.mediatek.camera.tests.v3.operator.SettingSwitchButtonOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;

import org.junit.Test;

@StorageTest
public class SdCardFullTestCase extends BaseSdCardFullTestCase {
    @Test
    @FunctionTest
    @CameraBasicTest
    @ModeCommonTest
    public void testCaptureWhenSdCardFull() {
        new MetaCase("TEMP_MODE_0031")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addChecker(new StorageHintChecker(), StorageHintChecker.INDEX_SHOW)
                .addChecker(new PreviewChecker())
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_NO_SAVED)
                .addOperator(new CaptureOrRecordOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    @CameraBasicTest
    @DngTest
    public void testDngCaptureWhenSDcardFull() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_dng_0011")
                .addOperator(new DngOperator(), SettingSwitchButtonOperator.INDEX_SWITCH_ON)
                .addChecker(new DngIndicatorChecker(), IndicatorChecker.INDEX_SHOW)
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_NO_SAVED)
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .run();
    }
}
