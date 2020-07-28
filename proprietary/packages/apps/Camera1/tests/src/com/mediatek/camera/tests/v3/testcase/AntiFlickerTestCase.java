package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.AntiFlickerTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.AntiFlickerExistedChecker;
import com.mediatek.camera.tests.v3.checker.AntiFlickerSettingValueChecker;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.operator.ChangeAntiFlickerToValueOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPanoramaModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipPhotoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;

import org.junit.Test;

/**
 * Anti-flicker test case.
 */

@CameraFeatureTest
@AntiFlickerTest
public class AntiFlickerTestCase extends BaseCameraTestCase {

    /**
     * Test relaunch camera, anti flicker value should not be remembered.
     */
    @Test
    @FunctionTest
    public void testNotRememberAntiFlickerSettingAfterRelaunch() {
        new MetaCase("TC_Camera_Anti_Flicker_0003")
                .addOperator(new ChangeAntiFlickerToValueOperator("60Hz"))
                .addOperator(new PauseResumeByBackKeyOperator())
                .addChecker(new AntiFlickerSettingValueChecker("Auto"))
                .run();
    }

    /**
     * Test pause/resume camera, anti flicker value can be remembered.
     */
    @Test
    @FunctionTest
    public void testRememberAntiFlickerSettingAfterPauseResumeByHomeKey() {
        new MetaCase("TC_Camera_Anti_Flicker_0004")
                .addOperator(new ChangeAntiFlickerToValueOperator("60Hz"))
                .addOperator(new PauseResumeByHomeKeyOperator())
                .addChecker(new AntiFlickerSettingValueChecker("60Hz"))
                .run();
    }

    /**
     * Test anti flicker setting must enable in support mode.
     */
    @Test
    @FunctionTest
    public void testAntiFlickerExistedOrNotInEachMode() {
        new MetaCase("TC_Camera_Anti_Flicker_0005")
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_Anti_Flicker_0005")
                .addOperator(new SwitchToPanoramaModeOperator())
                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_Anti_Flicker_0005")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();

        new MetaCase("TC_Camera_Anti_Flicker_0005")
                .addOperator(new SwitchToPipPhotoModeOperator())
                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Anti_Flicker_0005")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Anti_Flicker_0005")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    /**
     * Test anti flicker can be shown in front camera normal mode.
     */
    @Test
    @FunctionTest
    public void testAntiFlickerExistedInEachCamera() {
        new MetaCase("TC_Camera_Anti_Flicker_0006")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new AntiFlickerExistedChecker(false),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();
    }

    /**
     * Test anti flicker value can be remembered in each camera device.
     */
    @Test
    @FunctionTest
    public void testRememberAntiFlickerSettingForEachCamera() {
        new MetaCase("TC_Camera_Anti_Flicker_0007")
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addOperator(new ChangeAntiFlickerToValueOperator("60Hz"))
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_FRONT, SwitchCameraOperator.INDEX_BACK})
                .addChecker(new AntiFlickerSettingValueChecker("Auto"))
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addChecker(new AntiFlickerSettingValueChecker("60Hz"))
                .addOperator(new ChangeAntiFlickerToValueOperator("Auto"))
                .run();
    }

    /**
     * Test anti flicker default value.
     */
    @Test
    @FunctionTest
    public void testAntiFlickerDefaultValue() {
        new MetaCase("TC_Camera_Anti_Flicker_0008")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new AntiFlickerSettingValueChecker("Auto"))
                .run();
    }

    /**
     * Test supported status in 3rd app.
     */
    @Test
    @FunctionTest
    public void testSupportedStatusIn3rd() {
        new MetaCase("TC_Camera_Anti_Flicker_0009")
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addChecker(new PreviewChecker())
                .addChecker(new AntiFlickerExistedChecker(true),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();
    }

    /**
     * Test supported status on API1 & API2.
     */
    @Test
    @FunctionTest
    public void testSupportedStatusOnAPI1API2() {
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
                new MetaCase("TC_Camera_Anti_Flicker_0010")
                        .addChecker(new AntiFlickerExistedChecker(false),
                                SettingItemExistedChecker.INDEX_EXISTED)
                        .run();
                break;
            case API2:
                new MetaCase("TC_Camera_Anti_Flicker_0010")
                        .addChecker(new AntiFlickerExistedChecker(false),
                                SettingItemExistedChecker.INDEX_EXISTED)
                        .run();
                break;
            default:
                break;
        }
    }
}
