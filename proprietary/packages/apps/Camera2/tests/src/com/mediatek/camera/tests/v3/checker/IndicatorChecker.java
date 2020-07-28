package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.BySelector;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public abstract class IndicatorChecker extends Checker {
    private static final LogUtil.Tag TAG = Utils.getTestTag(IndicatorChecker.class.getSimpleName());
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
        switch (index) {
            case INDEX_SHOW:
                return "Check " + getIndicatorIconResourceId() + " is shown";
            case INDEX_HIDE:
                return "Check " + getIndicatorIconResourceId() + " is hide";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        BySelector selector = By.clazz("android.widget.ImageView").res
                (getIndicatorIconResourceId());
        switch (index) {
            case INDEX_HIDE:
                Utils.assertNoObject(selector);
                break;
            case INDEX_SHOW:
                Utils.assertObject(selector);
                break;
        }
    }

    protected abstract String getIndicatorIconResourceId();
}
