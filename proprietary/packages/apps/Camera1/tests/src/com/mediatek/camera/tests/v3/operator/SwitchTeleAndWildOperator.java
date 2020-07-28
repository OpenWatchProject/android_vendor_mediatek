package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Switch tele and wild camera on dual camera.
 */
public class SwitchTeleAndWildOperator extends OperatorOne {

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Click at zoom text";
    }

    @Override
    protected void doOperate() {
        UiObject2 zoomBarText =
                Utils.findObject(By.res("com.mediatek.camera:id/zoom_rotate_layout")
                        .enabled(true));
        zoomBarText.click();
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
    }
}
