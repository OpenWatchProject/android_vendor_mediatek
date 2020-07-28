package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class QuickSwitchOptionChecker extends Checker {
    public static final int INDEX_SHOW = 0;
    public static final int INDEX_HIDE = 1;

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
        if (index == INDEX_SHOW) {
            return "Check quick switch option is shown";
        } else if (index == INDEX_HIDE) {
            return "Check quick switch option is hidden";
        } else {
            return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        if (index == INDEX_SHOW) {
            Utils.assertObject(By.res("com.mediatek.camera:id/quick_switcher_option"));
        } else if (index == INDEX_HIDE) {
            Utils.assertNoObject(By.res("com.mediatek.camera:id/quick_switcher_option"));
        }
    }
}
