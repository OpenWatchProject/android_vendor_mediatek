package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;


public class SwitchCameraSupportedChecker extends Checker {
    public static final int INDEX_SUPPORT = 0;
    public static final int INDEX_NOT_SUPPORT = 1;

    @Override
    protected void doCheck(int index) {
        if (index == INDEX_SUPPORT) {
            Utils.findObject(By.res("com.mediatek.camera:id/camera_switcher"));
        } else if (index == INDEX_NOT_SUPPORT) {
            Utils.findNoObject(By.res("com.mediatek.camera:id/camera_switcher"));
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
        if (index == INDEX_SUPPORT) {
            return "Check should support switch camera ";
        } else if (index == INDEX_NOT_SUPPORT) {
            return "Check don't support switch camera";
        } else {
            return null;
        }
    }
}
