package com.mediatek.camera.tests.v3.testcase;

import android.os.Build;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.DngTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.checker.CameraFacingChecker;
import com.mediatek.camera.tests.v3.checker.DngIndicatorChecker;
import com.mediatek.camera.tests.v3.checker.DngIndicatorLayoutChecker;
import com.mediatek.camera.tests.v3.checker.DngModeRestrictionChecker;
import com.mediatek.camera.tests.v3.checker.HdrQuickSwitchChecker;
import com.mediatek.camera.tests.v3.checker.IndicatorChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.SettingSwitchOnOffChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.DngSavedObserver;
import com.mediatek.camera.tests.v3.observer.HeapMemoryObserver;
import com.mediatek.camera.tests.v3.observer.MediaLocationObserver;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CaptureOrRecordOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ChangeAllSettingOneByOneOperator;
import com.mediatek.camera.tests.v3.operator.ConnectWifiOperator;
import com.mediatek.camera.tests.v3.operator.DngOperator;
import com.mediatek.camera.tests.v3.operator.FontSizeOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.LocationOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PermissionOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SelfTimerOperator;
import com.mediatek.camera.tests.v3.operator.SettingSwitchButtonOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalPhotoModeOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraBasicTest
@DngTest
public class DngTestCase extends BaseCameraTestCase {

    @Test
    @StabilityTest
    //memory check
    public void testDngCaptureWithRepeatPauseResume() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_dng_0001," +
                " TC_Camera_dng_0006, TC_Camera_dng_0012, TC_Camera_dng_0014")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new CameraFacingChecker())
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addChecker(new DngIndicatorChecker(), DngIndicatorChecker.INDEX_SHOW)
                .addChecker(new PreviewChecker())
                .observeBegin(new HeapMemoryObserver())
                .observeBegin(new DngSavedObserver(), DngSavedObserver.INDEX_ON)
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .observeEnd()
                .addOperator(new PauseResumeByHomeKeyOperator())
                .repeatEnd()
                .run();
    }

    @Test
    @StabilityTest
    public void testDngRepeatCapture() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_dng_0013")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new CameraFacingChecker())
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addChecker(new DngIndicatorChecker(), DngIndicatorChecker.INDEX_SHOW)
                .addChecker(new PreviewChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .observeBegin(new DngSavedObserver(), DngSavedObserver.INDEX_ON)
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testDngWithAllSettingSwitch() {
        new MetaCase("TC_Camera_dng_0003")
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addOperator(new ChangeAllSettingOneByOneOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testDngRestrictionWithHdr() {
        Operator dngOperator = new DngOperator();
        Checker dngChecker = new SettingSwitchOnOffChecker("RAW(.DNG)");
        Operator hdrOperator = new HdrOperator();
        Checker hdrChecker = new HdrQuickSwitchChecker();

        //TC_Camera_dng_0002
        if (!dngOperator.isSupported() || !hdrOperator.isSupported()) {
            return;
        }

        dngOperator.operate(DngOperator.INDEX_SWITCH_ON);
        hdrChecker.check(HdrQuickSwitchChecker.INDEX_OFF);

        hdrOperator.operate(HdrOperator.INDEX_ON);
        dngChecker.check(SettingSwitchOnOffChecker
                .INDEX_SWITCH_OFF);

        if (hdrOperator.isSupported(HdrOperator.INDEX_AUTO)) {
            dngOperator.operate(DngOperator.INDEX_SWITCH_ON);
            hdrChecker.check(HdrQuickSwitchChecker.INDEX_OFF);

            hdrOperator.operate(HdrOperator.INDEX_AUTO);
            dngChecker.check(SettingSwitchOnOffChecker
                    .INDEX_SWITCH_OFF);
        }
    }

    @Test
    @FunctionTest
    public void testDngRestrictionWithMode() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_dng_0004")
                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
                .addChecker(new DngModeRestrictionChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testDngCaptureWithThumbnail() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_dng_0005, TC_Camera_dng_0007, TC_Camera_dng_0009")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new CameraFacingChecker())
                .acrossBegin()
                .addOperator(new DngOperator())
                .addChecker(new DngIndicatorChecker())
                .addChecker(new PreviewChecker())
                .observeBegin(new DngSavedObserver())
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .acrossEnd()
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testDngCaptureWithModesSwitch() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase()
                .addOperator(new SwitchToNormalPhotoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new DngOperator(), SettingSwitchButtonOperator.INDEX_SWITCH_ON)
                .addChecker(new DngIndicatorChecker(), IndicatorChecker.INDEX_SHOW)
                .run();
        new MetaCase("TC_Camera_dng_0004")
                .addOperator(new SwitchAllModeOperator(true))
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchPhotoVideoOperator())
                .acrossEnd()
                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_ONE_SAVED)
                .addOperator(new CaptureOrRecordOperator())
                .observeEnd()
                .run();
    }

//    @Test
//    @FunctionTest
//    public void testDngImageDetailInfo() {
//        if (!new DngOperator().isSupported()) {
//            return;
//        }
//        if (Build.VERSION.SDK_INT < 23) {
//            return;
//        }
//        new MetaCase("TC_Camera_dng_0010")
//                .addOperator(new ConnectWifiOperator(), ConnectWifiOperator.INDEX_CONNECT)
//                .addOperator(new LocationOperator(), LocationOperator.INDEX_ON_HIGH_ACCURACY)
//                .addOperator(new PermissionOperator(), PermissionOperator.INDEX_ENABLE_ALL)
//                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
//                .addChecker(new DngIndicatorChecker(), DngIndicatorChecker.INDEX_SHOW)
//                .observeBegin(new MediaLocationObserver(), MediaLocationObserver.INDEX_PHOTO)
//                .observeBegin(new DngSavedObserver(), DngSavedObserver.INDEX_ON)
//                .addOperator(new CapturePhotoOperator())
//                .observeEnd()
//                .observeEnd()
//                .run();
//    }

    @Test
    @FunctionTest
    public void testDngSupportState() {
        if (new DngOperator().isSupported()) {
            new MetaCase("TC_Camera_dng_0018")
                    .addChecker(new SettingItemExistedChecker("RAW(.DNG)", false),
                            SettingItemExistedChecker.INDEX_EXISTED)
                    .run();
        } else {
            new MetaCase("TC_Camera_dng_0018")
                    .addChecker(new SettingItemExistedChecker("RAW(.DNG)", false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .run();
        }
    }

    @Test
    @StabilityTest
    public void testDngSettingRepeatSwitch() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_dng_0019")
                .addOperator(new SettingSwitchButtonOperator("RAW(.DNG)"))
                .addChecker(new SettingSwitchOnOffChecker("RAW(.DNG)"))
                .runForTimes(Utils.STABILITY_REPEAT_TIMES);
    }

    @Test
    @FunctionTest
    public void testDngIndicatorLayout() {
        new MetaCase("TC_Camera_dng_0008")
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addOperator(new SelfTimerOperator(), SelfTimerOperator.INDEX_10_SECONDS)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addChecker(new DngIndicatorLayoutChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testDngIndicatorWithLargeSize() {
        if (!new DngOperator().isSupported()) {
            return;
        }
        new MetaCase("TC_Camera_dng_0021")
                .addOperator(new FontSizeOperator(), FontSizeOperator.INDEX_HUGE)
                .addOperator(new DngOperator(), DngOperator.INDEX_SWITCH_ON)
                .addChecker(new DngIndicatorChecker(), DngIndicatorChecker.INDEX_SHOW)
                .run();
    }
}
