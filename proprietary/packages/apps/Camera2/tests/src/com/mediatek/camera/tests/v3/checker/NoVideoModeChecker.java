package com.mediatek.camera.tests.v3.checker;


import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class NoVideoModeChecker extends Checker {
    @Override
    public float getCheckCoverage() {
        return super.getCheckCoverage();
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
        return "Check no video type mode";
    }

    @Override
    protected void doCheck(int index) {
        Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text").text("Picture"));
        Utils.assertNoObject(By.res("com.mediatek.camera:id/shutter_text").text("Video"));
    }
}
