package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Take vss .
 */

public class TakeVideoSnapShotOperator extends OperatorOne {

    @Override
    protected void doOperate() {
        UiObject2 shutterStop = Utils.findObject(By.res("com.mediatek.camera:id/btn_vss"));
        Utils.assertRightNow(shutterStop != null);
        shutterStop.click();

    }

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
        return "Take video snap shot ";
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.vss");
    }
}
