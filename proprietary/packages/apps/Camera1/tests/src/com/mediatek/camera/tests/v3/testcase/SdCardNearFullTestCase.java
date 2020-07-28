package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.ModeCommonTest;
import com.mediatek.camera.tests.v3.annotation.module.StorageTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.StorageHintChecker;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.operator.CaptureOrRecordUntilFullOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.FillStorageOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;

import org.junit.Test;

@StorageTest
public class SdCardNearFullTestCase extends BaseSdCardNearFullTestCase {
    @Test
    @FunctionTest
    @CameraBasicTest
    @ModeCommonTest
    public void testCaptureWhenSdCardNearFull() {
        new MetaCase("TEMP_MODE_0033")
                .addOperator(new ExitCameraOperator())
                .addOperator(new FillStorageOperator(), FillStorageOperator.INDEX_NEAR_FULL)
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_NORMAL)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .acrossEnd()
                .addChecker(new StorageHintChecker(), StorageHintChecker.INDEX_HIDE)
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_MULTI_SAVED)
                .addChecker(new PreviewChecker())
                .addOperator(new CaptureOrRecordUntilFullOperator())
                .observeEnd()
                .addChecker(new StorageHintChecker(), StorageHintChecker.INDEX_SHOW)
                .run();
    }
}
