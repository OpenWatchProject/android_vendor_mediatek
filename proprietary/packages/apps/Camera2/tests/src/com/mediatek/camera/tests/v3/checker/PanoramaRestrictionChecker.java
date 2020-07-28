package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class PanoramaRestrictionChecker extends Checker {
    @Override
    public float getCheckCoverage() {
        return super.getCheckCoverage();
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.panorama");
    }

    @Override
    public int getCheckCount() {
        return 1;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        return "Check panorama restrictions";
    }

    @Override
    protected void doCheck(int index) {
        Utils.assertNoObject(By.res("com.mediatek.camera:id/camera_switcher"));
        Utils.assertNoObject(By.res("com.mediatek.camera:id/hdr_icon"));
        Utils.assertNoObject(By.res("com.mediatek.camera:id/flash_icon"));
        Utils.assertNoObject(By.res("com.mediatek.camera:id/lomo_effect_indicator"));
        new SettingItemExistedChecker("Scene mode", false).check(
                SettingItemExistedChecker.INDEX_NOT_EXISTED);
        new AisExistedChecker(false).check(SettingItemExistedChecker.INDEX_NOT_EXISTED);
        new SettingItemExistedChecker("Self timer", false).check(
                SettingItemExistedChecker.INDEX_NOT_EXISTED);
        new SettingItemExistedChecker("ZSD", false).check(
                SettingItemExistedChecker.INDEX_NOT_EXISTED);
        new SettingItemExistedChecker("RAW(.DNG)", false).check(
                SettingItemExistedChecker.INDEX_NOT_EXISTED);
        new SettingItemExistedChecker("Picture size", false).check(
                SettingItemExistedChecker.INDEX_NOT_EXISTED);
        //TODO: should check 3A lock and CShot here.
    }
}
