package com.mediatek.camera.tests.v3.operator;


import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public abstract class QuickSwitchOptionsOperator extends SettingConfigOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(QuickSwitchOptionsOperator.class
            .getSimpleName());

    @Override
    public int getOperatorCount() {
        return getOptionsCount();
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 entry = Utils.findObject(By.res(getSwitchIconResourceId()).clickable(true));
        Utils.assertRightNow(entry != null);

        if (getSwitchIconDescription(index).equals(entry.getContentDescription())) {
            return;
        }

        entry.click();

        //  2 options case
        if (getOptionsCount() == 2 ||
                Utils.findObject(By.res("com.mediatek.camera:id/quick_switcher_option"),
                        Utils.TIME_OUT_SHORT) == null) {
            Utils.assertObject(By.res(getSwitchIconResourceId()).clickable(true).desc
                    (getSwitchIconDescription(index)));
            return;
        } else {
            UiObject2 option = Utils.findObject(
                    By.res(getOptionsResourceId(index)).clickable(true));
            Utils.assertRightNow(option != null);

            option.click();
            Utils.assertObject(By.res(getSwitchIconResourceId()));
        }
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.PREVIEW;
    }

    protected abstract int getOptionsCount();

    protected abstract String getSwitchIconResourceId();

    protected abstract String getOptionsResourceId(int index);

    protected abstract String getSwitchIconDescription(int index);
}
