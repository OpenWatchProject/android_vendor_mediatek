package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class GoToGalleryOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        UiObject2 thumb = Utils.findObject(
                By.res("com.mediatek.camera:id/thumbnail").clickable(true).enabled(true));
        Utils.assertRightNow(thumb != null);
        thumb.click();
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Click thumbnail to launch gallery";
    }
}
