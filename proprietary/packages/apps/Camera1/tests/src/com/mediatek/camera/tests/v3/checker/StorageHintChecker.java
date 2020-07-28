package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class StorageHintChecker extends Checker {
    public static final int INDEX_SHOW = 0;
    public static final int INDEX_HIDE = 1;
    private static final String HINT = "The default storage is full,please release some space or " +
            "manage the storage options in settings";

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
            return "Check storage full hint is showing";
        } else if (index == INDEX_HIDE) {
            return "Check storage full hint is hide";
        } else {
            return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        if (index == INDEX_SHOW) {
            Utils.assertObject(By.text(HINT));
        } else if (index == INDEX_HIDE) {
            Utils.assertNoObject(By.text(HINT));
        }
    }
}
