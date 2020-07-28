package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check matrix display is closed.
 */
public class MatrixDisplayClosedChecker extends CheckerOne {
    @Override
    protected void doCheck() {
        Utils.assertNoObject(By.res("com.mediatek.camera:id/lomo_effect_layout"));
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Check matrix display is closed";
    }
}
