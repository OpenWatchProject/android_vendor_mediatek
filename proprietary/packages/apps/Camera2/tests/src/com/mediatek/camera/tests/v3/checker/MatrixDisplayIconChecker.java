package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class MatrixDisplayIconChecker extends Checker {
    public static final int INDEX_MATRIX_SHOW = 0;
    public static final int INDEX_MATRIX_HIDE = 1;

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_MATRIX_SHOW) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.matrix-display");
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
            case INDEX_MATRIX_HIDE:
                return "Check don't show matrix display icon";
            case INDEX_MATRIX_SHOW:
                return "Check should show matrix display icon";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case INDEX_MATRIX_HIDE:
                Utils.findNoObject(By.res("com.mediatek.camera:id/effect"));
                break;
            case INDEX_MATRIX_SHOW:
                Utils.findObject(By.res("com.mediatek.camera:id/effect"));
                break;
        }

    }
}
