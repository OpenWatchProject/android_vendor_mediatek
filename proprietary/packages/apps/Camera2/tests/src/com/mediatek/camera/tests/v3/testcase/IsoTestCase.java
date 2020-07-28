package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.IsoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.IsoExifChecker;
import com.mediatek.camera.tests.v3.checker.IsoExistedChecker;
import com.mediatek.camera.tests.v3.checker.IsoSettingValueChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ChangeAntiFlickerToValueOperator;
import com.mediatek.camera.tests.v3.operator.ChangeIsoToValueOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.IsoOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraFeatureTest
@IsoTest
public class IsoTestCase extends BaseCameraTestCase {
    @Test
    @FunctionTest
    @MmTest
    public void testCaptureInEveryIso() {
        new MetaCase("TC_Camera_ISO_0001/AE_002")
                .addOperator(new IsoOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new IsoExifChecker())
                .run();
        new MetaCase("TC_Camera_ISO_0001/AE_002")
                .addOperator(new ChangeAntiFlickerToValueOperator("50Hz"))
                .addOperator(new ChangeIsoToValueOperator("1600"))
                .addChecker(new PreviewChecker())
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addChecker(new IsoExifChecker().setExpectedIsoValue(1600))
                .run();
    }

    @Test
    @FunctionTest
    public void testIsoExistedOrNotInEveryMode() {
        new MetaCase("TC_Camera_ISO_0003")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new IsoExistedChecker(false), SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_ISO_0003")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new IsoExistedChecker(false), SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_ISO_0003")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new IsoExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_ISO_0003")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new IsoExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_ISO_0003")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new IsoExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_ISO_0003")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new IsoExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testIsoExistedInEveryCamera() {
        new MetaCase("TC_Camera_ISO_0005")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new IsoExistedChecker(false), SettingItemExistedChecker.INDEX_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testRememberIsoSettingForEachCamera() {
        new MetaCase("TC_Camera_ISO_0006")
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addOperator(new ChangeIsoToValueOperator("800"))
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_FRONT, SwitchCameraOperator.INDEX_BACK})
                .addChecker(new IsoSettingValueChecker("Auto"))
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addChecker(new IsoSettingValueChecker("800"))
                .addOperator(new ChangeIsoToValueOperator("Auto"))
                .run();
    }

    @Test
    @FunctionTest
    public void testIsoDefaultValue() {
        new MetaCase("TC_Camera_ISO_0007")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new IsoSettingValueChecker("Auto"))
                .run();
    }

    @Test
    @FunctionTest
    public void testRememberIsoSettingAfterPauseResumeByHomeKey() {
        new MetaCase("TC_Camera_ISO_0008")
                .addOperator(new ChangeIsoToValueOperator("800"))
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new IsoSettingValueChecker("800"))
                .run();
    }

    @Test
    @FunctionTest
    public void testNotRememberIsoSettingAfterRelaunch() {
        new MetaCase("TC_Camera_ISO_0009")
                .addOperator(new ChangeIsoToValueOperator("800"))
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new IsoSettingValueChecker("Auto"))
                .run();
    }

    @Test
    @FunctionTest
    public void testSupportedStatusOnAPI1API2() {
        if (new IsoOperator().isSupported()) {

            new MetaCase("TC_Camera_ISO_0010")
                    .addChecker(new IsoExistedChecker(false),
                            SettingItemExistedChecker.INDEX_EXISTED)
                    .run();
        } else {
            new MetaCase("TC_Camera_ISO_0010")
                    .addChecker(new IsoExistedChecker(false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .run();
        }
    }

    @Test
    @FunctionTest
    public void testIsoDisabledWhenHdrOnAuto() {
        new MetaCase("TC_Camera_ISO_0011")
                .addOperator(new HdrOperator(),
                        new int[]{HdrOperator.INDEX_ON, HdrOperator.INDEX_AUTO})
                .addChecker(new IsoExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testIsoDisabledWhenSceneModeNotOffAuto() {
        new MetaCase("TC_Camera_ISO_0012")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addChecker(new IsoExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testSupportedStatusOnProject() {
        if (Utils.isFeatureSupported("com.mediatek.camera.at.iso")) {
            new MetaCase("TC_Camera_ISO_0013")
                    .addChecker(new IsoExistedChecker(false),
                            SettingItemExistedChecker.INDEX_EXISTED)
                    .run();
        } else {
            new MetaCase("TC_Camera_ISO_0013")
                    .addChecker(new IsoExistedChecker(false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .run();
        }
    }
}
