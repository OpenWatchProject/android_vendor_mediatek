package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {
        "Check DNG setting item is visible only in normal photo mode, and invisible in other mode"})
public class DngModeRestrictionChecker extends CheckerOne {
    @Override
    protected void doCheck() {
        // init current is normal photo mode or not
        UiObject2 shutter = Utils.findObject(By.res("com.mediatek.camera:id/shutter_root"));
        boolean isNormalPhotoMode = shutter.getContentDescription().startsWith("PhotoMode is");

        // normal photo mode, dng exist
        // other mode, dng not exist
        if (isNormalPhotoMode) {
            new SettingItemExistedChecker("RAW(.DNG)", false).check(
                    SettingItemExistedChecker.INDEX_EXISTED);
        } else {
            new SettingItemExistedChecker("RAW(.DNG)", false).check(
                    SettingItemExistedChecker.INDEX_NOT_EXISTED);
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Check dng setting is visible only when normal photo mode, is invisible when others";
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.dng.on");
    }
}
