package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.IsoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.IsoExistedChecker;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;

import org.junit.Test;

@CameraFeatureTest
@IsoTest
public class IsoIntentPhotoTestCase extends BaseIntentPhotoTestCase {
    @Test
    @FunctionTest
    public void testNoIsoInSetting() {
        new MetaCase("TC_Camera_ISO_0002")
                .addChecker(new IsoExistedChecker(true),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }
}
