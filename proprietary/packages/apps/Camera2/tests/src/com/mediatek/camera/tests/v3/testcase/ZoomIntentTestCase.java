package com.mediatek.camera.tests.v3.testcase;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.DigitalZoomTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.observer.PhotoSavedObserver;
import com.mediatek.camera.tests.v3.observer.UiAppearedObserver;
import com.mediatek.camera.tests.v3.observer.ZoomUiObserver;
import com.mediatek.camera.tests.v3.operator.CapturePhotoOperator;
import com.mediatek.camera.tests.v3.operator.SaveIntentImageOperator;
import com.mediatek.camera.tests.v3.operator.ZoomOperator;

import org.junit.Test;

/**
 * For test zoom in intent photo mode.
 */

@CameraBasicTest
@DigitalZoomTest
public class ZoomIntentTestCase extends BaseIntentPhotoTestCase {

    @Test
    @FunctionTest
    public void testZoomIntentCapture() {
        new MetaCase("TC_Camera_zoom_0007")
                .observeBegin(new ZoomUiObserver())
                .addOperator(new ZoomOperator(), ZoomOperator.INDEX_ZOOM_IN)
                .observeEnd()
                .observeBegin(new UiAppearedObserver(
                        By.res("com.mediatek.camera:id/review_btn_root"),
                        "intent captured image review"))
                .addOperator(new CapturePhotoOperator())
                .observeEnd()
                .addOperator(new ZoomOperator().ignoreBothPageCheck(), ZoomOperator.INDEX_ZOOM_OUT)
                .observeBegin(new PhotoSavedObserver(1))
                .addOperator(new SaveIntentImageOperator(), SaveIntentImageOperator.INDEX_SAVE)
                .observeEnd()
                .run();
    }
}
