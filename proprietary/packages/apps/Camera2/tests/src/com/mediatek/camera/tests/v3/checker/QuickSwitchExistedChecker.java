package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public abstract class QuickSwitchExistedChecker extends Checker {
    public static final int INDEX_EXIST = 0;
    public static final int INDEX_NOT_EXIST = 1;

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
        if (index == INDEX_EXIST) {
            return "Check " + getSwitchIconResourceId() + " icon show in quick switch";
        } else if (index == INDEX_NOT_EXIST) {
            return "Check no " + getSwitchIconResourceId() + " icon in quick switch";
        } else {
            return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        if (index == INDEX_EXIST) {
            Utils.assertObject(By.res("com.mediatek.camera:id/quick_switcher").hasChild(By.res
                    (getSwitchIconResourceId())));
        } else if (index == INDEX_NOT_EXIST) {
            Utils.assertNoObject(By.res("com.mediatek.camera:id/quick_switcher").hasChild(By.res
                    (getSwitchIconResourceId())));
        }

    }

    protected abstract String getSwitchIconResourceId();
}
