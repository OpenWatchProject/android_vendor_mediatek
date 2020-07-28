package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.annotation.from.MmTest;
import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.EisTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.PerformanceTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.EisExistedChecker;
import com.mediatek.camera.tests.v3.checker.EisSwitchOnOffChecker;
import com.mediatek.camera.tests.v3.checker.FocusUiChecker;
import com.mediatek.camera.tests.v3.checker.MatrixDisplayOpenedChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.RecordingChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailChecker;
import com.mediatek.camera.tests.v3.checker.VideoFileSizeChecker;
import com.mediatek.camera.tests.v3.checker.VideoFpsChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceIndicatorChecker;
import com.mediatek.camera.tests.v3.observer.EisLogObserver;
import com.mediatek.camera.tests.v3.observer.EisPreviewLogObserver;
import com.mediatek.camera.tests.v3.observer.FlashLogObserver;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.ThumbnailUpdatedObserver;
import com.mediatek.camera.tests.v3.observer.TouchFocusStateObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomRatioObserver;
import com.mediatek.camera.tests.v3.operator.AntiFlickerOperator;
import com.mediatek.camera.tests.v3.operator.EisOperator;
import com.mediatek.camera.tests.v3.operator.FlashOperator;
import com.mediatek.camera.tests.v3.operator.MatrixDisplayOpeningOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.QualityOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.StopRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToNormalVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToPipVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToSlowMotionModeOperator;
import com.mediatek.camera.tests.v3.operator.SwitchToStereoVideoModeOperator;
import com.mediatek.camera.tests.v3.operator.TakeVideoSnapShotOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.WhiteBalanceOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraFeatureTest
@EisTest
public class EisTestCase extends BaseCameraTestCase {
    @Test
    @FunctionTest
    public void testEisSupported() {
        new MetaCase("TC_Camera_Eis_0001")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new EisExistedChecker(false),
                        Utils.isFeatureSupported("com.mediatek.camera.at.eis") ?
                                EisExistedChecker.INDEX_EXISTED : EisExistedChecker
                                .INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Eis_0001")
                .observeBegin(new EisLogObserver())
                .addOperator(new EisOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testRecordInDiffQualityWithEisEnableForBack() {
        new MetaCase("TC_Camera_Eis_0002")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_Eis_0002")
                .addOperator(new QualityOperator())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new VideoFileSizeChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testRecordInDiffQualityWithEisEnableForFront() {
        new MetaCase("TC_Camera_Eis_0003")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_Eis_0003")
                .addOperator(new QualityOperator())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new VideoFileSizeChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testPreviewNormalInDiffQualityWithEisEnable() {
        new MetaCase("TC_Camera_Eis_0004")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .run();
        // TODO: need remove when EIS hal3 ready.
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API2) {
            return;
        }
        new MetaCase("TC_Camera_Eis_0004")
                .observeBegin(new EisPreviewLogObserver(), EisPreviewLogObserver.INDEX_ON)
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testRememberEisSettingForEachCamera() {
        new MetaCase("TC_Camera_ISO_0005")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_FRONT, SwitchCameraOperator.INDEX_BACK})
                .addChecker(new EisSwitchOnOffChecker(), EisSwitchOnOffChecker.INDEX_SWITCH_OFF)
                .addOperator(new SwitchCameraOperator(), new int[]{SwitchCameraOperator
                        .INDEX_BACK, SwitchCameraOperator.INDEX_FRONT})
                .addChecker(new EisSwitchOnOffChecker(), EisSwitchOnOffChecker.INDEX_SWITCH_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_OFF)
                .run();
    }

    @Test
    @FunctionTest
    @MmTest
    public void testZoomDuringRecordingWithEisEnable() {
        new MetaCase("TC_Camera_Eis_0006")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_Eis_0006")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addOperator(new StartRecordOperator(false))
                .addChecker(new RecordingChecker())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StopRecordOperator(false))
                .observeEnd()
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MIN)
                .addOperator(new StartRecordOperator(false))
                .addChecker(new RecordingChecker())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StopRecordOperator(false))
                .observeEnd()
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @StabilityTest
    public void testVideoSavedWhenPauseResumeWithEisEnable() {
        new MetaCase("TC_Camera_Eis_0007")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .addOperator(new StartRecordOperator(false))
                .addChecker(new RecordingChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeEnd()
                .addChecker(new EisSwitchOnOffChecker(), EisSwitchOnOffChecker.INDEX_SWITCH_ON)
                .addOperator(new StartRecordOperator(false))
                .addChecker(new RecordingChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new PauseResumeByBackKeyOperator())
                .observeEnd()
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new EisSwitchOnOffChecker(), EisSwitchOnOffChecker.INDEX_SWITCH_ON)
                .addOperator(new StartRecordOperator(false))
                .addChecker(new RecordingChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .observeBegin(new ZoomRatioObserver(), ZoomRatioObserver.INDEX_ZOOM_IN_LEVEL_MAX)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new ZoomRatioObserver(), ZoomRatioObserver.INDEX_ZOOM_OUT_LEVEL_MIN)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testEisRecordingInNightMode() {
        new MetaCase("TC_Camera_Eis_0009")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .run();
        new MetaCase("TC_Camera_Eis_0009")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testEisRecordingInShadeMode() {
        new MetaCase("TC_Camera_Eis_0010")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_Eis_0010")
                .addOperator(new QualityOperator(), QualityOperator.QUALITY_INDEX_MAX)
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .addOperator(new WhiteBalanceOperator(), WhiteBalanceOperator.INDEX_SHADE)
                .addChecker(new WhiteBalanceIndicatorChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @StabilityTest
    public void testEisRecordingFor1Hour() {
        new MetaCase("TC_Camera_Eis_0011")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator().setDuration(
                        Utils.STABILITY_CONTINUE_SECONDS))
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testVssWhenEisRecording() {
        new MetaCase("TC_Camera_Eis_0012")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .run();
        new MetaCase("TC_Camera_Eis_0012")
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .acrossBegin()
                .addOperator(new QualityOperator())
                .acrossEnd()
                .addOperator(new StartRecordOperator(false))
                .addChecker(new RecordingChecker())
                .observeBegin(new ZoomRatioObserver(), ZoomRatioObserver.INDEX_ZOOM_IN_LEVEL_MAX)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new TakeVideoSnapShotOperator())
                .observeEnd()
                .observeBegin(new ZoomRatioObserver(), ZoomRatioObserver.INDEX_ZOOM_OUT_LEVEL_MIN)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .observeBegin(new ThumbnailUpdatedObserver())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new StopRecordOperator(false))
                .observeEnd()
                .observeEnd()
                .addChecker(new VideoFileSizeChecker())
                .addChecker(new ThumbnailChecker(), ThumbnailChecker.INDEX_HAS_THUMB)
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_OFF)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_OFF)
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testSuspendAndWakeUpDuringEisRecording() {
        new MetaCase("TC_Camera_Eis_0013")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .addOperator(new StartRecordOperator(false))
                .addChecker(new RecordingChecker())
                .observeBegin(new VideoSavedObserver())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testTouchFocusWhenEisEnable() {
        new MetaCase("TC_Camera_Eis_0015/TC_Camera_Eis_0018")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()

                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator(), 10)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()

                .observeBegin(new ZoomRatioObserver(), ZoomRatioObserver.INDEX_ZOOM_IN_LEVEL_MAX)
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()

                .observeBegin(new TouchFocusStateObserver(), TouchFocusStateObserver.INDEX_BACK)
                .addOperator(new TouchFocusOperator(), 15)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testStrobeWhenEisEnable() {
        new MetaCase("TC_Camera_Eis_0016")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .run();

        new MetaCase("TC_Camera_Eis_0016")
                .addOperator(new QualityOperator(), 0)
                .observeBegin(new FlashLogObserver().initEnv(null, true), FlashLogObserver.INDEX_ON)
                .addOperator(new FlashOperator(), FlashOperator.INDEX_ON)
                .observeEnd()
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .observeBegin(new FlashLogObserver().initEnv(null, true),
                        FlashLogObserver.INDEX_OFF)
                .addOperator(new FlashOperator(), FlashOperator.INDEX_OFF)
                .observeEnd()
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .run();
    }

    @Test
    @StabilityTest
    public void testSwitchOnOffEisStress() {
        new MetaCase("TC_Camera_Eis_0020")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_OFF)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_OFF)
                .observeEnd()
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testMatrixDisplayWhenEisEnable() {
        new MetaCase("TC_Camera_Eis_0021")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .addOperator(new MatrixDisplayOpeningOperator())
                .addChecker(new MatrixDisplayOpenedChecker())
                .run();
    }

    @Test
    @FunctionTest
    public void testEisSupportedInEveryMode() {
        new MetaCase("TC_Camera_Eis_0022")
                .addOperator(new SwitchToSlowMotionModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new EisExistedChecker(false), EisExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Eis_0022")
                .addOperator(new SwitchToPipVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new EisExistedChecker(false), EisExistedChecker.INDEX_NOT_EXISTED)
                .run();

        new MetaCase("TC_Camera_Eis_0022")
                .addOperator(new SwitchToStereoVideoModeOperator())
                .addChecker(new PreviewChecker())
                .addChecker(new EisExistedChecker(false), EisExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }

    /**
     * There is no environment request of EIS performance test case. Different platform has
     * different standard,may 24/30fps.
     */
    @Test
    @PerformanceTest
    public void testEisRecordingFps() {
        new MetaCase("TC_Camera_Eis_0019")
                .addOperator(new SwitchToNormalVideoModeOperator())
                .addChecker(new PreviewChecker())
                .observeBegin(new EisLogObserver(), EisLogObserver.INDEX_ON)
                .addOperator(new EisOperator(), EisOperator.INDEX_SWITCH_ON)
                .observeEnd()
                .addOperator(new AntiFlickerOperator(), 1)
                .observeBegin(new VideoSavedObserver())
                .addOperator(new RecordVideoOperator())
                .observeEnd()
                .addChecker(new VideoFpsChecker(24))
                .run();
    }
}
