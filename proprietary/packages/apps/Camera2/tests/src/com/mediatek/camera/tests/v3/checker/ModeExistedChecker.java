package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check mode icon is existed or not in mode list"})
public class ModeExistedChecker extends Checker {
    public static final int INDEX_EXISTED = 0;
    public static final int INDEX_NOT_EXISTED = 1;

    private String mModeName;

    public ModeExistedChecker(String modeName) {
        mModeName = modeName;
    }

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.MODE_LIST;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_EXISTED:
                return "Check " + mModeName + " mode is existed in mode list";
            case INDEX_NOT_EXISTED:
                return "Check " + mModeName + " mode is not existed in mode list";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case INDEX_EXISTED:
                Utils.assertObject(By.text(mModeName));
                break;
            case INDEX_NOT_EXISTED:
                Utils.assertNoObject(By.text(mModeName));
        }
    }
}
