package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.CameraExitedChecker;
import com.mediatek.camera.tests.v3.checker.CameraLaunchedChecker;
import com.mediatek.camera.tests.v3.checker.NoImagesVideosChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.observer.CopyRecordingFileObserver;
import com.mediatek.camera.tests.v3.observer.VideoSavedObserver;
import com.mediatek.camera.tests.v3.operator.AirplaneModeOperator;
import com.mediatek.camera.tests.v3.operator.ClearImagesVideosOperator;
import com.mediatek.camera.tests.v3.operator.ClearSharePreferenceOperator;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.RecordVideoOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPhotoVideoOperator;
import com.mediatek.camera.tests.v3.operator.VideoQualityOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

// Test case cooperation with another running apk, not used to run by adb command
public class CoreDomainStressTest {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            CoreDomainStressTest.class.getSimpleName());
    private static int RECORD_DURATION = 60;

    @Test
    public void testEnableAirplaneMode() {
        new MetaCase()
                .addOperator(new AirplaneModeOperator(), AirplaneModeOperator.INDEX_ENABLE)
                .run();
    }

    @Test
    public void testCopyWhenRecording4k2k() {
        new MetaCase()
                .addOperator(new ClearSharePreferenceOperator())
                .addOperator(new ClearImagesVideosOperator())
                .addChecker(new NoImagesVideosChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_NORMAL)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addOperator(new VideoQualityOperator(true), VideoQualityOperator.INDEX_BACK_4K)
                .observeBegin(new VideoSavedObserver())
                .observeBegin(new CopyRecordingFileObserver())
                .addOperator(new RecordVideoOperator().setDuration(RECORD_DURATION)
                        .ignoreCheckDurationText())
                .observeEnd()
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .run();
    }

    @Test
    public void testCopyWhenRecordingFhd() {
        new MetaCase()
                .addOperator(new ClearSharePreferenceOperator())
                .addOperator(new ClearImagesVideosOperator())
                .addChecker(new NoImagesVideosChecker())
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_NORMAL)
                .addChecker(new CameraLaunchedChecker(), CameraLaunchedChecker.INDEX_NORMAL)
                .addChecker(new PreviewChecker())
                .addOperator(new SwitchPhotoVideoOperator(), SwitchPhotoVideoOperator.INDEX_VIDEO)
                .addOperator(new VideoQualityOperator(true), VideoQualityOperator.INDEX_BACK_FHD)
                .observeBegin(new VideoSavedObserver())
                .observeBegin(new CopyRecordingFileObserver())
                .addOperator(new RecordVideoOperator().setDuration(RECORD_DURATION)
                        .ignoreCheckDurationText())
                .observeEnd()
                .observeEnd()
                .addChecker(new PreviewChecker())
                .addOperator(new ExitCameraOperator())
                .addChecker(new CameraExitedChecker())
                .run();
    }
}
