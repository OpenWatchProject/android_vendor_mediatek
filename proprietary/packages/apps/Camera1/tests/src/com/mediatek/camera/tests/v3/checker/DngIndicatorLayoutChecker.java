package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

/**
 * To check dng indicator layout above other settings.
 */

public class DngIndicatorLayoutChecker extends Checker {
    @Override
    public int getCheckCount() {
        return 1;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        return "Check if dng indicator layout above other settings";
    }

    @Override
    protected void doCheck(int index) {
        UiObject2 indicatorList = Utils.findNoObject(By.res(
                "com.mediatek.camera:id/indicator_view"));
        List<UiObject2> childList = indicatorList.getChildren();
        Utils.assertRightNow(childList.get(0).getResourceName().contains("dng_indicator"));
    }
}
