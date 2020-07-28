package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.SettingTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PageChecker;
import com.mediatek.camera.tests.v3.checker.SettingIconExistedChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemSingleChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemSummaryChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemTypeAndOrderChecker;
import com.mediatek.camera.tests.v3.checker.SettingListLayoutPositionChecker;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.ChangeAllSettingOneByOneOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPageOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraFeatureTest
@SettingTest
public class SettingTestCase extends BaseCameraTestCase {
    @Test
    @FunctionTest
    public void testChangeAllSettingOneByOne() {
        new MetaCase()
                .addOperator(new ChangeAllSettingOneByOneOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testChangeAllSettingOneByOneInAllMode() {
        new MetaCase()
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(new ChangeAllSettingOneByOneOperator())
                .run();

        new MetaCase()
                .addOperator(new PauseResumeByBackKeyOperator())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .run();

        new MetaCase()
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addOperator(new ChangeAllSettingOneByOneOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testSettingListPosition() {
        new MetaCase("TC_Camera_Setting_0001")
                .addChecker(new SettingListLayoutPositionChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testSettingIconHiddenIfCurrentModeHasNoneSetting() {
        new MetaCase("TC_Camera_Setting_0002")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new SettingIconExistedChecker(false))
                .run();
    }

    @Test
    @FunctionTest
    public void testShowSettingItemByModeType() {
        new MetaCase("TC_Camera_Setting_0003, TC_Camera_Setting_0008")
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_PHOTO)
                .addChecker(new SettingItemTypeAndOrderChecker(
                        SettingItemTypeAndOrderChecker.PHOTO_TYPE))
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addChecker(new SettingItemTypeAndOrderChecker(
                        SettingItemTypeAndOrderChecker.VIDEO_TYPE))
                .run();
    }

    @Test
    @FunctionTest
    public void testSettingSummaryAndTitle() {
        new MetaCase("TC_Camera_Setting_0004")
                .addOperator(new SwitchPhotoVideoOperator())
                .addChecker(new SettingItemSummaryChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testDestroySettingUIAfterPaused() {
        new MetaCase("TC_Camera_Setting_0005")
                .addOperator(new SwitchPageOperator(), SwitchPageOperator.INDEX_SETTINGS)
                .addChecker(new PageChecker(), PageChecker.INDEX_SETTINGS)
                .addOperator(new PauseResumeByHomeKeyOperator().ignoreBothPageCheck())
                .addChecker(new PageChecker(), PageChecker.INDEX_PREVIEW)
                .run();
    }

    @Test
    @FunctionTest
    public void testSettingKeepSameAfterPausedAndResumed() {
        new MetaCase("TC_Camera_Setting_0006")
                .addOperator(new SwitchPhotoVideoOperator())
                .addChecker(new SettingItemSummaryChecker().withPauseAndResumeAfterChangeValue())
                .run();
    }

    @Test
    @FunctionTest
    public void testSettingNotDoubleAfterPowerOffAndOn() {
        new MetaCase("TC_Camera_Setting_0007")
                .addOperator(new PauseResumeByPowerKeyOperator())
                .addOperator(new SwitchPageOperator(), SwitchPageOperator.INDEX_SETTINGS)
                .addChecker(new SettingItemSingleChecker())
                .run();

    }

    @Test
    @StabilityTest
    public void testRepeatingOpenCloseSetting() {
        new MetaCase("TC_Camera_Setting_0010")
                .addOperator(new SwitchPageOperator(), SwitchPageOperator.INDEX_SETTINGS)
                .addOperator(new BackToCameraOperator())
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testOpenSettingFrom3rdParty() {
        new MetaCase("TC_Camera_Setting_0011")
                .addOperator(new ExitCameraOperator())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .addOperator(new SwitchPageOperator(), SwitchPageOperator.INDEX_SETTINGS)
                .addChecker(new PageChecker(), PageChecker.INDEX_SETTINGS)
                .run();
    }
}
