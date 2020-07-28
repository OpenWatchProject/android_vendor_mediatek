package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;


public class PressShutterOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        UiObject2 shutter = Utils.findObject(Utils.getShutterSelector());
        shutter.click();
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
        return "Press shutter button, no matter photo/video";
    }
}
