package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraFeatureTest;
import com.mediatek.camera.tests.v3.annotation.module.WhiteBalanceTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;
import com.mediatek.camera.tests.v3.checker.WhiteBalanceExistedChecker;

import org.junit.Test;

@CameraFeatureTest
@WhiteBalanceTest
public class WhiteBalanceIntentPhotoTestCase extends BaseIntentPhotoTestCase {
    @Test
    @FunctionTest
    public void testNoWbInSetting() {
        new MetaCase("TC_Camera_White_Balance_0015")
                .addChecker(new WhiteBalanceExistedChecker(true),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }
}
