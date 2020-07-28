package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.WhiteBalanceTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceExifChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceExistedChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceSettingValueChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceSettingValueRealTimeChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.observer.WhiteBalanceLogObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.FontSizeOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.WhiteBalanceOneByOneOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraFeatureTest
@WhiteBalanceTest
public class WhiteBalanceTestCase extends BaseCameraTestCase {
    @Test
    @FunctionTest
    public void testCaptureInEveryWhiteBalance() {
        new MetaCase("TC_Camera_White_Balance_0001/TC_Camera_White_Balance_0002")
                .addOperator(new WhiteBalanceOneByOneOperator())
                .addChecker(new WhiteBalanceIndicatorChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .observeBegin(new WhiteBalanceLogObserver())
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .addChecker(new WhiteBalanceExifChecker())
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testNotRememberWbSettingAfterRelaunch() {
        new MetaCase("TC_Camera_White_Balance_0003/AWB_005")
                .addOperator(new WhiteBalanceOneByOneOperator(),
                        WhiteBalanceOneByOneOperator.INDEX_SECOND_OPTION)
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new WhiteBalanceSettingValueChecker("Auto"))
                .run();
    }

    @Test
    @FunctionTest
    public void testRememberWbSettingAfterPauseResumeByHomeKey() {
        new MetaCase("TC_Camera_White_Balance_0004")
                .addOperator(new WhiteBalanceOneByOneOperator(),
                        WhiteBalanceOneByOneOperator.INDEX_SECOND_OPTION)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .run();
        new MetaCase("TC_Camera_White_Balance_0004")
                .addChecker(new WhiteBalanceSettingValueChecker(
                        TestContext.mLatestWhiteBalanceSettingValue))
                .run();
    }

    @Test
    @FunctionTest
    public void testWbSettingKeepSameWhenEnterSettingAgain() {
        new MetaCase("TC_Camera_White_Balance_0005")
                .addOperator(new WhiteBalanceOneByOneOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new WhiteBalanceSettingValueRealTimeChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testWbExistedOrNotInEveryMode() {
        new MetaCase("TC_Camera_White_Balance_0006")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_White_Balance_0006")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_White_Balance_0006")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_White_Balance_0006")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_White_Balance_0006")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_White_Balance_0006")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testWbDisabledWhenHdrOnAuto() {
        new MetaCase("TC_Camera_White_Balance_0007")
                .addOperator(new HdrOperator(),
                        new int[]{HdrOperator.INDEX_ON, HdrOperator.INDEX_AUTO})
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testWbDisabledWhenInSomeSceneMode() {
        new MetaCase("TC_Camera_White_Balance_0008")
                .addOperator(new SceneModeOperator(),
                        new int[]{SceneModeOperator.INDEX_CANDLE_LIGHTS,
                                SceneModeOperator.INDEX_SUNSET,
                                SceneModeOperator.INDEX_LANDSCAPE})
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testCaptureInEverySupportModeInEveryWbMode() {
        new MetaCase("TC_Camera_White_Balance_0009")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .acrossBegin()
                .addOperator(new WhiteBalanceOneByOneOperator())
                .addChecker(new WhiteBalanceIndicatorChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .observeBegin(new WhiteBalanceLogObserver())
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .addChecker(new WhiteBalanceExifChecker())
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_White_Balance_0009")
                .addOperator(new SwitchToPanoramaModeOperator())
                .acrossBegin()
                .addOperator(new WhiteBalanceOneByOneOperator())
                .addChecker(new WhiteBalanceIndicatorChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .observeBegin(new WhiteBalanceLogObserver())
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .addChecker(new WhiteBalanceExifChecker())
                .acrossEnd()
                .run();

        new MetaCase("TC_Camera_White_Balance_0009")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .acrossBegin()
                .addOperator(new WhiteBalanceOneByOneOperator())
                .addChecker(new WhiteBalanceIndicatorChecker())
                .observeBegin(new VideoSavedObserver())
                .observeBegin(new WhiteBalanceLogObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .observeEnd()
                .acrossEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testChangeWbModeInMaxFontSize() {
        new MetaCase("TC_Camera_White_Balance_0011")
                .addOperator(new FontSizeOperator(), FontSizeOperator.INDEX_HUGE).run();

        new MetaCase("TC_Camera_White_Balance_0011")
                .addOperator(new WhiteBalanceOneByOneOperator())
                .addChecker(new WhiteBalanceIndicatorChecker())
                .run();

        new MetaCase("TC_Camera_White_Balance_0011")
                .addOperator(new FontSizeOperator(), FontSizeOperator.INDEX_NORMAL).run();
    }

    @Test
    @FunctionTest
    public void testWbExistedInEveryCamera() {
        new MetaCase("TC_Camera_White_Balance_0012")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new WhiteBalanceExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testRememberWbSettingForEachCamera() {
        new MetaCase("TC_Camera_White_Balance_0013")
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addOperator(new WhiteBalanceOneByOneOperator(),
                        WhiteBalanceOneByOneOperator.INDEX_SECOND_OPTION)
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_FRONT, SwitchCameraOperator.INDEX_BACK})
                .addChecker(new WhiteBalanceSettingValueChecker("Auto"))
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addChecker(new WhiteBalanceSettingValueRealTimeChecker())
                .addOperator(new WhiteBalanceOneByOneOperator(),
                        WhiteBalanceOneByOneOperator.INDEX_FIRST_OPTION_AUTO)
                .run();
    }

    @Test
    @FunctionTest
    public void testWbDefaultValue() {
        new MetaCase("TC_Camera_White_Balance_0014")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new WhiteBalanceSettingValueChecker("Auto"))
                .run();
    }

    @Test
    @FunctionTest
    public void testSupportedStatusOnAPI1API2() {
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
            case API2:
                new MetaCase("TC_Camera_White_Balance_0016")
                        .addChecker(new WhiteBalanceExistedChecker(false),
                                SettingItemExistedChecker.INDEX_EXISTED)
                        .run();
                break;
        }
    }

    @Test
    @FunctionTest
    public void testSupportedStatusOnProject() {
        if (Utils.isFeatureSupported("com.mediatek.camera.at.white-balance")) {
            new MetaCase("TC_Camera_White_Balance_0017")
                    .addChecker(new WhiteBalanceExistedChecker(false),
                            SettingItemExistedChecker.INDEX_EXISTED)
                    .run();
        } else {
            new MetaCase("TC_Camera_White_Balance_0017")
                    .addChecker(new WhiteBalanceExistedChecker(false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .run();
        }
    }
}
