package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class VSDOFBarChecker extends Checker {
    public static final int INDEX_HAS_VSDOFBAR = 0;
    public static final int INDEX_NO_VSDOFBAR = 1;

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_HAS_VSDOFBAR) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.vsdof");
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
            case INDEX_HAS_VSDOFBAR:
                return "Check VSDOF bar is exist";
            case INDEX_NO_VSDOFBAR:
                return "Check VSDOF bar is not exist";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case INDEX_HAS_VSDOFBAR:
                Utils.assertObject(
                        By.res("com.mediatek.camera:id/sdof_bar"));
                break;
            case INDEX_NO_VSDOFBAR:
                Utils.assertNoObject(
                        By.res("com.mediatek.camera:id/sdof_bar"));
                break;
        }
    }
}
