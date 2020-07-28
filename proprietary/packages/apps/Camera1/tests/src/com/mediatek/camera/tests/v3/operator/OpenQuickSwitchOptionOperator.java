package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public abstract class OpenQuickSwitchOptionOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        UiObject2 quickSwitchIcon = Utils.findObject(By.res(getSwitchIconResourceId()));
        Utils.assertRightNow(quickSwitchIcon != null, "Can not find quick switch icon for " +
                getSwitchIconResourceId());
        quickSwitchIcon.click();
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
        return "Open quick switch option of " + getSwitchIconResourceId();
    }

    protected abstract String getSwitchIconResourceId();
}
