package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class PanoramaModeChecker extends Checker {
    public static final int PANORAMA_MODE = 0;
    public static final int NOT_PANORAMA_MODE = 1;

    @Override
    public float getCheckCoverage() {
        return super.getCheckCoverage();
    }

    @Override
    public boolean isSupported(int index) {
        if (index == PANORAMA_MODE) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.panorama");
        } else {
            return true;
        }
    }

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case PANORAMA_MODE:
                return "Current mode is panorama mode";
            case NOT_PANORAMA_MODE:
                return "Current mode is not panorama mode";
            default:
                return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case PANORAMA_MODE:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_root")
                        .descContains("PanoramaMode is previewing").enabled(true));
                break;
            case NOT_PANORAMA_MODE:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_root")
                        .descContains("is previewing").enabled(true));
        }
    }
}
