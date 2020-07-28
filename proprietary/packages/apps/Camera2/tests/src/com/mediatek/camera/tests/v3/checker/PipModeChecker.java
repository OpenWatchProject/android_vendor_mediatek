package com.mediatek.camera.tests.v3.checker;


import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Pip mode checker, pip photo mode or pip video mode.
 */
public class PipModeChecker extends Checker {
    public static final int PIP_PHOTO_MODE = 0;
    public static final int PIP_VIDEO_MODE = 1;
    public static final int PHOTO_MODE = 2;
    public static final int VIDEO_MODE = 3;

    @Override
    public float getCheckCoverage() {
        return super.getCheckCoverage();
    }

    @Override
    public boolean isSupported(int index) {
        if (index == PIP_PHOTO_MODE || index == PIP_VIDEO_MODE) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.pip");
        } else {
            return true;
        }
    }

    @Override
    public int getCheckCount() {
        return 4;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case PIP_PHOTO_MODE:
                return "Current mode is PIP photo mode";
            case PIP_VIDEO_MODE:
                return "Current mode is PIP video mode";
            case PHOTO_MODE:
                return "Current mode is photo mode";
            case VIDEO_MODE:
                return "Current mode is video mode";
            default:
                return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case PIP_PHOTO_MODE:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_root")
                        .descContains("PipPhotoMode is previewing").enabled(true));
                break;
            case PIP_VIDEO_MODE:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_root")
                        .descContains("PipVideoMode is previewing").enabled(true));
                break;
            case PHOTO_MODE:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_root")
                        .descContains("PhotoMode is previewing").enabled(true));
                break;
            case VIDEO_MODE:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_root")
                        .descContains("VideoMode is previewing").enabled(true));
                break;
            default:
                break;
        }
    }
}
