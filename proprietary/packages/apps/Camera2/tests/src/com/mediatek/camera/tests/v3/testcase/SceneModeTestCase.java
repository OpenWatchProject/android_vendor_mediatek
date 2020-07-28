package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.helper.LoggerService;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.SceneModeTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.FlashQuickSwitchChecker;
import com.mediatek.camera.tests.v3.checker.FlashQuickSwitchExistedChecker;
import com.mediatek.camera.tests.v3.checker.IsoSettingValueChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SceneModeIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.SceneModeOptionsChecker;
import com.mediatek.camera.tests.v3.checker.SceneModeSummaryChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceSettingValueChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.SceneModeValueObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.IsoOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.WhiteBalanceOperator;

import org.junit.Test;

/**
 * Scene mode test case.
 */
@CameraFeatureTest
@SceneModeTest
public class SceneModeTestCase extends BaseCameraTestCase {

    @Override
    public void setUp() {
        // Need to set adb command before opening camera, because adb command isn't worked
        // when set after camera opened.
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            LoggerService.getInstance().setAdbCommand("debug.camera.log.AppStreamMgr", "2");
        }
        super.setUp();
    }

    @Override
    public void tearDown() {
        super.tearDown();
        // Restore adb command.
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            LoggerService.getInstance().setAdbCommand("debug.camera.log.AppStreamMgr", "0");
        }
    }

    @Test
    @FunctionTest
    public void testSceneModeOptionsIsCompleted() {
        new MetaCase("TC_Camera_Scene_Mode_0001," +
                " TC_Camera_Scene_Mode_0013, TC_Camera_Scene_Mode_0017")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new SceneModeOptionsChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testFireworksUnsupportedInVideo() {
        new MetaCase("TC_Camera_Scene_Mode_0002")
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_FIREWORKS)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_FIREWORKS)
                .observeEnd()
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_OFF)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .observeEnd()
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_OFF))
                .addChecker(new SceneModeOptionsChecker()
                        .except(new String[]{SceneModeOptionsChecker.VALUE_FIREWORKS}))
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneKeepSameBetweenPhotoAndVideo() {
        // test scene mode is keep same when switch between photo and video
        // except "auto", "fireworks" scene mode.
        new MetaCase("TC_Camera_Scene_Mode_0003")
                // set sunset in photo mode.
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_SUNSET)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_SUNSET)
                .observeEnd()
                // switch to video mode and check scene mode is sunset.
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_SUNSET)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .observeEnd()
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_SUNSET))
                // change scene mode as candle light in video mode.
                .observeBegin(new SceneModeValueObserver(),
                        SceneModeValueObserver.INDEX_CANDLE_LIGHTS)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_CANDLE_LIGHTS)
                .observeEnd()
                // switch to photo mode and check scene mode is candle light.
                .observeBegin(new SceneModeValueObserver(),
                        SceneModeValueObserver.INDEX_CANDLE_LIGHTS)
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .observeEnd()
                .addChecker(new SceneModeSummaryChecker(
                        SceneModeSummaryChecker.VALUE_CANDLE_LIGHTS))
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneKeepSameAfterPausedAndResumed() {
        new MetaCase("TC_Camera_Scene_Mode_0004")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_NIGHT)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeEnd()
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_NIGHT))
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneTurnToOffAfterDestroyed() {
        new MetaCase("TC_Camera_Scene_Mode_0005")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT_PORTRAIT)
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_OFF)
                .addOperator(new PauseResumeByBackKeyOperator())
                .observeEnd()
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_OFF))
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneModeIndicator() {
        new MetaCase("TC_Camera_Scene_Mode_0006")
                .addOperator(new SceneModeOperator())
                .addChecker(new SceneModeIndicatorChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneModeRestriction() {
        new MetaCase("TC_Camera_Scene_Mode_0007")
                .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_FIREWORKS)
                .addChecker(new FlashQuickSwitchExistedChecker(),
                        FlashQuickSwitchExistedChecker.INDEX_NOT_EXIST)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_OFF)
                .addChecker(new FlashQuickSwitchChecker(), FlashQuickSwitchChecker.INDEX_ON)
                .run();

        if (CameraDeviceManagerFactory.CameraApi.API1
                == CameraApiHelper.getCameraApiType(null)) {
            new MetaCase("TC_Camera_Scene_Mode_0007")
                    .addOperator(new IsoOperator(), 1)
                    .addOperator(new WhiteBalanceOperator(), WhiteBalanceOperator.INDEX_SHADE)
                    .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_SUNSET)
                    .addChecker(new SettingItemExistedChecker("ISO", false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .addChecker(new SettingItemExistedChecker("White balance", false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_OFF)
                    .addChecker(new WhiteBalanceSettingValueChecker("Shade"))
                    .addChecker(new IsoSettingValueChecker("100"))
                    .run();
        } else if (CameraDeviceManagerFactory.CameraApi.API2
                == CameraApiHelper.getCameraApiType(null)) {
            new MetaCase("TC_Camera_Scene_Mode_0007")
                    .addOperator(new IsoOperator(), 1)
                    .addOperator(new WhiteBalanceOperator(), WhiteBalanceOperator.INDEX_SHADE)
                    .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_SUNSET)
                    .addChecker(new SettingItemExistedChecker("White balance", false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_OFF)
                    .addChecker(new WhiteBalanceSettingValueChecker("Shade"))
                    .addChecker(new IsoSettingValueChecker("100"))
                    .run();
        }
    }

    @Test
    @FunctionTest
    public void testSceneModeAfterTurnOnAndOffHdrInVideo() {
        HdrOperator operator = new HdrOperator().initHdrOptions(true);
        if (operator.isSupported(HdrOperator.INDEX_ON)) {
            new MetaCase("TC_Camera_Scene_Mode_0008")
                    .addOperator(new SwitchPhotoVideoOperator(),
                            SwitchPhotoVideoOperator.INDEX_VIDEO)
                    .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_SUNSET)
                    .addOperator(operator, HdrOperator.INDEX_ON)
                    .addChecker(new SettingItemExistedChecker("Scene mode", false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .addOperator(operator, HdrOperator.INDEX_OFF)
                    .addChecker(new SettingItemExistedChecker("Scene mode", false),
                            SettingItemExistedChecker.INDEX_EXISTED)
                    .run();
        }
    }

    @Test
    @FunctionTest
    public void testAutoSceneModeIsWork() {
        new MetaCase("TC_Camera_Scene_Mode_0009")
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_AUTO)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_AUTO)
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testAutoSceneModeIsWorkAfterPausedAndResumed() {
        new MetaCase("TC_Camera_Scene_Mode_0010")
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_AUTO)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_AUTO)
                .observeEnd()
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_AUTO)
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testAutoIsWorkAfterTakePicture() {
        new MetaCase("TC_Camera_Scene_Mode_0011")
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_AUTO)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_AUTO)
                .observeEnd()
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_AUTO)
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneModeAfterAutoSceneDetectionStopped() {
        new MetaCase("TC_Camera_Scene_Mode_0012")
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_AUTO)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_AUTO)
                .observeEnd()
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_SNOW)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_SNOW)
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneMode() {
        new MetaCase("TC_Camera_Scene_Mode_0014")
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_BEACH)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_BEACH)
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_Scene_Mode_0014")
                .observeBegin(new SceneModeValueObserver())
                .addOperator(new SceneModeOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testAsyncValueBetweenCamera() {
        new MetaCase("TC_Camera_Scene_Mode_0018")
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_NIGHT)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .observeEnd()
                // switch to front camera and check the scene mode is off.
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_OFF)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .observeEnd()
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_OFF))
                // set the scene mode as sports in the front camera.
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_SPORTS)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_SPORTS)
                .observeEnd()
                // switch to back camera and check the scene mode is night.
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_NIGHT)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .observeEnd()
                .addChecker(new SceneModeSummaryChecker(SceneModeSummaryChecker.VALUE_NIGHT))
                .run();
    }

    @Test
    @FunctionTest
    public void testSceneModeUnsupportedIn3rdParty() {
        new MetaCase("TC_Camera_Scene_Mode_0019, TC_Camera_Scene_Mode_0020")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addOperator(new ExitCameraOperator())
                .observeBegin(new SceneModeValueObserver(), SceneModeValueObserver.INDEX_OFF)
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .observeEnd()
                .addChecker(new SettingItemExistedChecker("Scene mode", true),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testCaptureInEverySceneMode() {
        new MetaCase("AE_010")
                .addOperator(new SceneModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .run();
    }
}
