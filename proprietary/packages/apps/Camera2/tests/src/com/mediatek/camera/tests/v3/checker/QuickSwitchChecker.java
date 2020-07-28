package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public abstract class QuickSwitchChecker extends Checker {
    @Override
    public int getCheckCount() {
        return getSwitchIconStatusCount();
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        return "Check the description of " + getSwitchIconResourceId() + " is " +
                getSwitchIconDescription(index);
    }

    @Override
    protected void doCheck(int index) {
        Utils.assertObject(By.res("com.mediatek.camera:id/quick_switcher")
                .hasChild(By.res(getSwitchIconResourceId())
                        .clazz("android.widget.ImageView")
                        .desc(getSwitchIconDescription(index))));
    }

    protected abstract String getSwitchIconResourceId();

    protected abstract String getSwitchIconDescription(int index);

    protected abstract int getSwitchIconStatusCount();
}
