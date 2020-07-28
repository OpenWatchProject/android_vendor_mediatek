package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.ThirdPartyLaunchVideoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.StabilityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.AeAfLockUiChecker;
import com.mediatek.camera.tests.v3.checker.FocusUiChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.VideoReviewUIChecker;
import com.mediatek.camera.tests.v3.checker.VssSupportedChecker;
import com.mediatek.camera.tests.v3.observer.AudioPlayedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.AntiFlickerOperator;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.EisOperator;
import com.mediatek.camera.tests.v3.operator.LongPressShutterOperator;
import com.mediatek.camera.tests.v3.operator.MicroPhoneOperator;
import com.mediatek.camera.tests.v3.operator.OnLongPressOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByPowerKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByRecentAppOperator;
import com.mediatek.camera.tests.v3.operator.StartRecordOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.TouchFocusOperator;
import com.mediatek.camera.tests.v3.operator.VideoReviewUIOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

@CameraBasicTest
@ThirdPartyLaunchVideoTest
public class IntentVideoWithUriTestCase extends BaseIntentVideoWithUriTestCase {

    @Test
    @FunctionTest
    public void testIntentVideoWithUri() {
        new MetaCase("TC_Camera_Intent_Video_0002/" +
                "TC_Camera_Intent_Video_0003/" +
                "TC_Camera_Intent_Video_0004/" +
                "TC_Camera_Intent_Video_0032/")
                .addOperator(new SwitchCameraOperator())
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false).ignoreBothPageCheck())
                .addChecker(new VideoReviewUIChecker())
                .addOperator(
                        new VideoReviewUIOperator(), VideoReviewUIOperator.REVIEW_OPERATION_PLAY)
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForSave() {
        new MetaCase("TC_Camera_Intent_Video_0005")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false).ignoreBothPageCheck())
                .addChecker(new VideoReviewUIChecker())
                .addOperator(
                        new VideoReviewUIOperator(), VideoReviewUIOperator.REVIEW_OPERATION_SAVE)
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForRecordingFocus() {
        new MetaCase("TC_Camera_Intent_Video_0013")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false).ignoreBothPageCheck())
                .addOperator(new TouchFocusOperator().ignoreBothPageCheck(), 20)
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForSettingCheck() {
        new MetaCase("TC_Camera_Intent_Video_0012 /" +
                "TC_Camera_Intent_Video_0047")
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator(true))
                .acrossEnd()
                .addChecker(new SettingItemExistedChecker("EIS", true),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .addChecker(new SettingItemExistedChecker("Microphone", true),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .addChecker(new SettingItemExistedChecker("Anti flicker", true),
                        SettingItemExistedChecker.INDEX_EXISTED)
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForSettings() {
        new MetaCase("TC_Camera_Intent_Video_0016")
                .addChecker(new PreviewChecker())
                .addOperator(new EisOperator())
                .acrossBegin()
                .addOperator(new MicroPhoneOperator())
                .acrossEnd()
                .acrossBegin()
                .addOperator(new AntiFlickerOperator())
                .acrossEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForPreviewZoom() {
        new MetaCase("TC_Camera_Intent_Video_0017")
                .addChecker(new PreviewChecker())
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .addOperator(new TouchFocusOperator(), 15)
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForVss() {
        new MetaCase("TC_Camera_Intent_Video_0018")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false).ignoreAfterPageCheck())
                .addChecker(new VssSupportedChecker(), VssSupportedChecker.INDEX_NOT_SUPPORT)
                .run();
    }

//    @Test
//    @FunctionTest
//    public void testIntentVideoForPauseRecording() {
//        new MetaCase("TC_Camera_Intent_Video_0019")
//                .addChecker(new PreviewChecker())
//                .addOperator(new StartRecordOperator(false).ignoreAfterPageCheck())
//                .addOperator(new PauseResumeRecordingOperator().ignoreBothPageCheck())
//                .addChecker(new RecordingPauseChecker())
//                .addOperator(new PauseResumeRecordingOperator().ignoreBothPageCheck())
//                .run();
//    }

    @Test
    @FunctionTest
    public void testIntentVideoForPreviewFocus() {
        new MetaCase("TC_Camera_Intent_Video_0031")
                .addChecker(new PreviewChecker())
                .addOperator(new TouchFocusOperator(), 20)
                .addChecker(new FocusUiChecker(), FocusUiChecker.INDEX_HAS_TAF_UI)
                .run();
    }


    @Test
    @FunctionTest
    public void testIntentVideoLongPressShutter() {
        new MetaCase("TC_Camera_Intent_Video_0033 /" +
                "TC_Camera_Intent_Video_0023")
                .addChecker(new PreviewChecker())
                .addOperator(new LongPressShutterOperator())
                .run();
    }

    @Test
    @StabilityTest
    public void testIntentVideoRecordingStress() {
        new MetaCase("TC_Camera_Intent_Video_0034")
                .addChecker(new PreviewChecker())
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addOperator(new StartRecordOperator(false))
                .addChecker(new VideoReviewUIChecker())
                .addOperator(new VideoReviewUIOperator())
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForSwitchCamera() {
        new MetaCase("TC_Camera_Intent_Video_0044/" +
                "TC_Camera_Intent_Video_0045/" +
                "TC_Camera_Intent_Video_0060")
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .run();
    }

    @Test
    @StabilityTest
    public void testIntentVideoForSwitchCameraStress() {
        new MetaCase("TC_Camera_Intent_Video_0046")
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new PreviewChecker())
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForReviewUI() {
        new MetaCase("TC_Camera_Intent_Video_0049")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false).ignoreBothPageCheck())
                .addChecker(new VideoReviewUIChecker())
                .addOperator(
                        new VideoReviewUIOperator(), VideoReviewUIOperator.REVIEW_OPERATION_PLAY)
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false).ignoreBothPageCheck())
                .addChecker(new VideoReviewUIChecker())
                .addOperator(
                        new VideoReviewUIOperator(), VideoReviewUIOperator.REVIEW_OPERATION_SAVE)
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoPreviewForPauseResumePowerKey() {
        new MetaCase("TC_Camera_Intent_Video_0053")
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByPowerKeyOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoPreviewForPauseResumeHomeKey() {
        new MetaCase("TC_Camera_Intent_Video_0053")
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoPreviewForPauseResumeRecentKey() {
        new MetaCase("TC_Camera_Intent_Video_0053")
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByRecentAppOperator())
                .run();
    }

    @Test
    @StabilityTest
    public void testIntentVideoPreviewForPauseResumeHomeKeyStress() {
        new MetaCase("TC_Camera_Intent_Video_0054")
                .repeatBegin(Utils.STABILITY_REPEAT_TIMES)
                .addChecker(new PreviewChecker())
                .addOperator(new PauseResumeByHomeKeyOperator())
                .repeatEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoRecordingForPauseResumePowerKey() {
        new MetaCase("TC_Camera_Intent_Video_0055")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false))
                .addOperator(new PauseResumeByPowerKeyOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoRecordingForPauseResumeHomeKey() {
        new MetaCase("TC_Camera_Intent_Video_0055")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false))
                .addOperator(new PauseResumeByHomeKeyOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoRecordingForPauseResumeRecentKey() {
        new MetaCase("TC_Camera_Intent_Video_0055")
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false))
                .addOperator(new PauseResumeByRecentAppOperator())
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoPreviewForBackKey() {
        new MetaCase("TC_Camera_Intent_Video_0056")
                .addChecker(new PreviewChecker())
                .addOperator(new BackToCameraOperator().ignoreBothPageCheck())
                .run();
    }

    @Test
    @FunctionTest
    public void testIntentVideoForAEAFLock() {
        if (!Utils.isFeatureSupported(TouchFocusOperator.OPTIONS_TAG[TouchFocusOperator
                .INDEX_BACK])) {
            return;
        }
        new MetaCase("TC_Camera_Intent_Video_0069")
                .addChecker(new PreviewChecker())
                .addOperator(new OnLongPressOperator(), 20)
                .addChecker(new AeAfLockUiChecker(), AeAfLockUiChecker.INDEX_EXIST)
                .run();
    }

    @Test
    @FunctionTest
    public void testForIncomingCall() {
        new MetaCase()
                .addChecker(new PreviewChecker())
                .acrossBegin()
                .addOperator(new SwitchCameraOperator(true))
                .acrossEnd()
                .addChecker(new PreviewChecker())
                .addOperator(new StartRecordOperator(false).ignoreBothPageCheck())
                .addChecker(new VideoReviewUIChecker())
                .observeBegin(new AudioPlayedObserver())
                .addOperator(
                        new VideoReviewUIOperator(), VideoReviewUIOperator.REVIEW_OPERATION_PLAY)
                .observeEnd()
                .run();
    }
}
