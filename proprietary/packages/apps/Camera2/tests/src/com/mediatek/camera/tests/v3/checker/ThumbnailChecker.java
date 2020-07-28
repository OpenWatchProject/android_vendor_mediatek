package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class ThumbnailChecker extends Checker {
    public static final int INDEX_HAS_THUMB = 0;
    public static final int INDEX_NO_THUMB = 1;

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
            case INDEX_HAS_THUMB:
                return "Check thumbnail view has content";
            case INDEX_NO_THUMB:
                return "Check thumbnail view does not have content";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case INDEX_HAS_THUMB:
                Utils.assertObject(By.res("com.mediatek.camera:id/thumbnail").desc("Has Content"));
                break;
            case INDEX_NO_THUMB:
                Utils.assertObject(By.res("com.mediatek.camera:id/thumbnail").desc("No Content"));
                break;
        }
    }
}
