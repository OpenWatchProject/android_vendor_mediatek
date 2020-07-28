package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.LocationTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.PreviewChecker;
import com.mediatek.camera.tests.v3.observer.MediaLocationObserver;
import com.mediatek.camera.tests.v3.observer.MediaSavedObserver;
import com.mediatek.camera.tests.v3.operator.CaptureOrRecordOperator;
import com.mediatek.camera.tests.v3.operator.ConnectWifiOperator;
import com.mediatek.camera.tests.v3.operator.LocationOperator;
import com.mediatek.camera.tests.v3.operator.PermissionOperator;
import com.mediatek.camera.tests.v3.operator.SwitchAllModeInPhotoVideoOperator;

import org.junit.Test;

/**
 * For location test in camera mode capture.
 */
@CameraBasicTest
@LocationTest
public class LocationTestCase extends BaseCameraTestCase {

//    @Test
//    @FunctionTest
//    public void testLocationWithModesCapture() {
//        new MetaCase("TC_Camera_location_0007")
//                .addOperator(new ConnectWifiOperator(), ConnectWifiOperator.INDEX_CONNECT)
//                .addOperator(new LocationOperator(), LocationOperator.INDEX_ON_HIGH_ACCURACY)
//                .addOperator(new PermissionOperator(), PermissionOperator.INDEX_ENABLE_ALL)
//                .run();
//        new MetaCase("TC_Camera_location_0007")
//                .addOperator(new SwitchAllModeInPhotoVideoOperator(true, true, true))
//                .addChecker(new PreviewChecker())
//                .observeBegin(new MediaLocationObserver(),
//                        MediaLocationObserver.INDEX_PHOTO_OR_VIDEO)
//                .observeBegin(new MediaSavedObserver(), MediaSavedObserver.INDEX_ONE_SAVED)
//                .addOperator(new CaptureOrRecordOperator())
//                .observeEnd()
//                .observeEnd()
//                .run();
//    }
}
