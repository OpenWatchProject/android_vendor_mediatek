package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * close alarm timer.
 */

public class CloseAlarmClockOperator extends Operator {
    @Override
    public int getOperatorCount() {
        return 1;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return null;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        return "Close alarm";
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 object = Utils.scrollFromTopOnScreenToFind(
                By.pkg("com.android.deskclock").desc("Stop")
        );
        if (object != null) {
            object.click();
            Utils.waitSafely(1000);
        }
    }
}
