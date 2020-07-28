package com.mediatek.camera.tests.v3.checker;


import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class VssSupportedChecker extends Checker {
    public static final int INDEX_SUPPORT = 0;
    public static final int INDEX_NOT_SUPPORT = 1;

    @Override
    protected void doCheck(int index) {
        if (index == INDEX_SUPPORT) {
            Utils.findObject(By.res("com.mediatek.camera:id/btn_vss"));
        } else if (index == INDEX_NOT_SUPPORT) {
            Utils.findNoObject(By.res("com.mediatek.camera:id/btn_vss"));
        }
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_SUPPORT) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.vss");
        } else {
            return true;
        }
    }

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public String getDescription(int index) {
        if (index == INDEX_SUPPORT) {
            return "Check should support vss";
        } else if (index == INDEX_NOT_SUPPORT) {
            return "Check don't support vss";
        } else {
            return null;
        }
    }
}
