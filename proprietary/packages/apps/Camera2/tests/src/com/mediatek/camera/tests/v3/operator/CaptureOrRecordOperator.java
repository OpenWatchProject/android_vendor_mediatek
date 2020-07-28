package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class CaptureOrRecordOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        UiObject2 shutter = Utils.findObject(Utils.getShutterSelector());
        Utils.assertRightNow(shutter != null, "Can not find shutter");
        if (shutter.getContentDescription().equals("Picture")) {
            new CapturePhotoOperator().operate(0);
        } else {
            new StartRecordOperator(true).operate(0);
            new SleepOperator(2).operate(0);
            new StopRecordOperator(true).operate(0);
        }
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
        return "Press current shutter to capture one photo, or record one video";
    }
}
