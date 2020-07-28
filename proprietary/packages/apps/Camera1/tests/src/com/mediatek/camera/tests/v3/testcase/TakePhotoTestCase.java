package com.mediatek.camera.tests.v3.testcase;

import android.util.Log;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.NormalPhotoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.annotation.type.SanityTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.checker.IsoExifChecker;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.checker.ThumbnailShownInGalleryChecker;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.operator.BackToCameraOperator;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.ChangeIsoToValueOperator;
import com.mediatek.camera.tests.v3.operator.GoToGalleryOperator;
import com.mediatek.camera.tests.v3.operator.SceneModeOperator;
import com.mediatek.camera.tests.v3.operator.SlideInGalleryOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

public class TakePhotoTestCase extends BaseCameraTestCase {
    private static final String TAG = "TakePhotoTestCase";

    @Override
    public void setUp() {
        Log.i(TAG, "TC_029 version 2017_09_07");
        Log.i(TAG, "setUp begin");
        mNotClearImagesVideos = true;
        super.setUp();
        Log.i(TAG, "setUp end");
    }

    @Test
    @SanityTest
    @CameraBasicTest
    @FunctionTest
    @NormalPhotoTest
    public void testTakePhoto() {
        Log.i(TAG, "testTakePhoto begin");
        Utils.setTimeoutForSanity();
        Log.i(TAG, "takeMainSensor zoom in,zoom out and capture begin");
        // back camera, zoom in, zoom out, capture
        new MetaCase("TC_029")
                .addOperator(new ZoomOperator(10000).ignoreAfterPageCheck(),
                        ZoomOperator.INDEX_ZOOM_IN)
                .addOperator(new ZoomOperator(1000).ignoreBothPageCheck(),
                        ZoomOperator.INDEX_ZOOM_OUT)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator().ignoreBothPageCheck())
                .observeEnd()
                .run();
        Log.i(TAG, "takeMainSensor zoom in,zoom out and capture end");
        // back camera, night scene mode, capture, scene mode off
        Log.i(TAG, "takeMainSensor change scene mode to night and capture begin");
        new MetaCase("TC_029")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator().ignoreAfterPageCheck())
                .observeEnd()
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_OFF)
                .run();
        Log.i(TAG, "takeMainSensor change scene mode to night and capture end");
        // back camera, iso 1600, capture, iso auto
        Log.i(TAG, "takeMainSensor change ISO to 1600 and capture begin");
        new MetaCase("TC_029")
                .addOperator(new ChangeIsoToValueOperator("1600"))
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator().ignoreAfterPageCheck())
                .observeEnd()
                .addChecker(new IsoExifChecker())
                .addOperator(new ChangeIsoToValueOperator("Auto"))
                .run();
        Log.i(TAG, "takeMainSensor change ISO to 1600 and capture end");
        // view pictures in gallery
        new MetaCase("TC_029")
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new SlideInGalleryOperator())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker().ignoreBeforePageCheck())
                .run();
        TestContext.mTotalCaptureAndRecordCount = 0;
        // front camera, zoom in, zoom out, capture
        Log.i(TAG, "takeSubSensor zoom in,zoom out and capture begin");
        new MetaCase("TC_029")
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new PreviewChecker().ignoreBeforePageCheck())
                .addOperator(new ZoomOperator(10000).ignoreBothPageCheck(),
                        ZoomOperator.INDEX_ZOOM_IN)
                .addOperator(new ZoomOperator(1000).ignoreBothPageCheck(),
                        ZoomOperator.INDEX_ZOOM_OUT)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator().ignoreBothPageCheck())
                .observeEnd()
                .run();
        Log.i(TAG, "takeSubSensor zoom in,zoom out and capture end");
        // front camera, night scene mode, capture, scene mode off
        Log.i(TAG, "takeSubSensor change scene mode to night and capture begin");
        new MetaCase("TC_029")
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_NIGHT)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new CapturePhotoOperator().ignoreAfterPageCheck())
                .observeEnd()
                .addOperator(new SceneModeOperator(), SceneModeOperator.INDEX_OFF)
                .run();
        Log.i(TAG, "takeSubSensor change scene mode to night and capture end");
        // view pictures in gallery
        new MetaCase("TC_029")
                .addOperator(new GoToGalleryOperator())
                .addChecker(new ThumbnailShownInGalleryChecker())
                .addOperator(new SlideInGalleryOperator())
                .addOperator(new BackToCameraOperator())
                .addChecker(new PreviewChecker().ignoreBeforePageCheck())
                .run();
        Log.i(TAG, "testTakePhoto end");
    }
}
