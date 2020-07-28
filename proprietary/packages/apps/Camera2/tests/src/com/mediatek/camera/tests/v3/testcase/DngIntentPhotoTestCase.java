package com.mediatek.camera.tests.v3.testcase;

import com.mediatek.camera.tests.v3.annotation.group.CameraBasicTest;
import com.mediatek.camera.tests.v3.annotation.module.DngTest;
import com.mediatek.camera.tests.v3.annotation.module.ThirdPartyLaunchPhotoTest;
import com.mediatek.camera.tests.v3.annotation.type.FunctionTest;
import com.mediatek.camera.tests.v3.arch.MetaCase;
import com.mediatek.camera.tests.v3.checker.SettingItemExistedChecker;

import org.junit.Test;

/**
 * For dng check in intent photo mode.
 */

@CameraBasicTest
@DngTest
public class DngIntentPhotoTestCase extends BaseIntentPhotoTestCase {

    @Test
    @FunctionTest
    @ThirdPartyLaunchPhotoTest
    public void testIntentModeDngSetting() {
        new MetaCase("TC_Camera_dng_0015")
                .addChecker(new SettingItemExistedChecker("RAW(.DNG)", true),
                        SettingItemExistedChecker.INDEX_NOT_EXISTED)
                .run();
    }
}
