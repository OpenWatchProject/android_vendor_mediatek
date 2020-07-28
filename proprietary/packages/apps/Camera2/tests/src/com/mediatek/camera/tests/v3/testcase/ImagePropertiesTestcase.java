package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.ImagePropertiesTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.ImagePropertiesOptionsChecker;
import com.mediatek.camera.tests.v3.checker.ImagePropertiesUiValueChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.observer.ImagePropertiesValueObserver;
import com.mediatek.camera.tests.v3.operator.ExitCameraOperator;
import com.mediatek.camera.tests.v3.operator.HdrOperator;
import com.mediatek.camera.tests.v3.operator.ImagePropertiesValueOperator;
import com.mediatek.camera.tests.v3.operator.LaunchCameraOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByBackKeyOperator;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.util.Utils;

import org.junit.Test;

/**
 * Image properties test case.
 */
@CameraFeatureTest
@ImagePropertiesTest
public class ImagePropertiesTestcase extends BaseCameraTestCase {

    @Test
    @FunctionTest
    public void testImagePropertiesIsSupported() {
        if (Utils.isFeatureSupported("com.mediatek.camera.at.image-properties")) {
            new MetaCase("TC_Camera_Image_Properties_0001, TC_Camera_Image_Properties_0008," +
                    " TC_Camera_Image_Properties_0014")
                    .addOperator(new SwitchCameraOperator())
                    .addChecker(new SettingItemExistedChecker("Image properties", false),
                            SettingItemExistedChecker.INDEX_EXISTED)
                    .addChecker(new ImagePropertiesOptionsChecker())
                    .run();
        } else {
            new MetaCase("TC_Camera_Image_Properties_0001")
                    .addOperator(new SwitchCameraOperator())
                    .addChecker(new SettingItemExistedChecker("Image properties", false),
                            SettingItemExistedChecker.INDEX_NOT_EXISTED)
                    .run();
        }
    }

//    @Test
//    @FunctionTest
//    public void testImagePropertiesValueIsWorked() {
//        new MetaCase("TC_Camera_Image_Properties_0002")
//                .observeBegin(new ImagePropertiesValueObserver())
//                .addOperator(new ImagePropertiesValueOperator())
//                .observeEnd()
//                .run();
//    }

    @Test
    @FunctionTest
    public void testImagePropertiesRestoreAfterDestroyed() {
        new MetaCase("TC_Camera_Image_Properties_0003, TC_Camera_Image_Properties_0004")
                .addOperator(new ImagePropertiesValueOperator(),
                        ImagePropertiesValueOperator.LEVEL_HIGH)
                .observeBegin(new ImagePropertiesValueObserver(),
                        ImagePropertiesValueObserver.LEVEL_MEDIUM)
                .addOperator(new PauseResumeByBackKeyOperator())
                .observeEnd()
                .run();
    }

    @Test
    @FunctionTest
    public void testImagePropertiesUIIsRightAfterChanged() {
        new MetaCase("TC_Camera_Image_Properties_0005")
                .addOperator(new ImagePropertiesValueOperator(),
                        ImagePropertiesValueOperator.LEVEL_HIGH)
                .addChecker(new ImagePropertiesUiValueChecker(),
                        ImagePropertiesUiValueChecker.LEVEL_HIGH)
                .run();
    }

    @Test
    @FunctionTest
    public void testImagePropertiesUiValueMappingPreview() {
        new MetaCase("TC_Camera_Image_Properties_0006")
                .addOperator(new ImagePropertiesValueOperator(),
                        ImagePropertiesValueOperator.LEVEL_HIGH)
                .observeBegin(new ImagePropertiesValueObserver(),
                        ImagePropertiesValueObserver.LEVEL_MEDIUM)
                .addOperator(new HdrOperator(), HdrOperator.INDEX_ON)
                .observeEnd()
                .addOperator(new PauseResumeByHomeKeyOperator())
                .observeBegin(new ImagePropertiesValueObserver(),
                        ImagePropertiesValueObserver.LEVEL_HIGH)
                .addOperator(new HdrOperator(), HdrOperator.INDEX_OFF)
                .observeEnd()
                .addChecker(new ImagePropertiesUiValueChecker(),
                        ImagePropertiesUiValueChecker.LEVEL_HIGH)
                .run();
    }

    @Test
    @FunctionTest
    public void testImagePropertiesValueAsyncBetweenCamera() {
        new MetaCase("TC_Camera_Image_Properties_0009, TC_Camera_Image_Properties_0010")
                .addOperator(new ImagePropertiesValueOperator(),
                        ImagePropertiesValueOperator.LEVEL_HIGH)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_FRONT)
                .addChecker(new ImagePropertiesUiValueChecker(),
                        ImagePropertiesUiValueChecker.LEVEL_MEDIUM)
                .addOperator(new ImagePropertiesValueOperator(),
                        ImagePropertiesValueOperator.LEVEL_LOW)
                .addOperator(new SwitchCameraOperator(), SwitchCameraOperator.INDEX_BACK)
                .addChecker(new ImagePropertiesUiValueChecker(),
                        ImagePropertiesUiValueChecker.LEVEL_HIGH)
                .run();
    }

    @Test
    @FunctionTest
    public void testImagePropertiesUnsupportedIn3rdParty() {
        new MetaCase("TC_Camera_Image_Properties_0011")
                .addOperator(new ImagePropertiesValueOperator(),
                        ImagePropertiesValueOperator.LEVEL_HIGH)
                .addOperator(new ExitCameraOperator())
                .observeBegin(new ImagePropertiesValueObserver(),
                        ImagePropertiesValueObserver.LEVEL_MEDIUM)
                .addOperator(new LaunchCameraOperator(), LaunchCameraOperator.INDEX_INTENT_PHOTO)
                .observeEnd()
                .addChecker(new SettingItemExistedChecker("Image properties", true),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }
}
