package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;


public class PauseResumeRecordingOperator extends OperatorOne {

    @Override
    protected void doOperate() {
        UiObject2 shutter = Utils.findObject(By.res("com.mediatek" +
                ".camera:id/btn_pause_resume"));
        Utils.assertRightNow(shutter != null);
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
        return "Pause/Resume recording ";
    }
}
