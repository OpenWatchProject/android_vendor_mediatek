package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public abstract class QuickSwitchOnOffOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(QuickSwitchOnOffOperator.class
            .getSimpleName());

    @Override
    public int getOperatorCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    protected void doOperate(int index) {
        String contentDesc = getSwitchIconContentDesc(index);
        UiObject2 switchIcon = Utils.findObject(By.res(getSwitchIconResourceId()).clickable(true),
                isNeedAssert() ? Utils.TIME_OUT_NORMAL : Utils.TIME_OUT_SHORT_SHORT);
        if (isNeedAssert()) {
            Utils.assertRightNow(switchIcon != null);
        } else if (switchIcon == null) {
            LogHelper.d(TAG, "[doOperate] not find switch icon, do nothing, return");
            return;
        }

        if (switchIcon.getContentDescription().equals(contentDesc)) {
            return;
        } else {
            switchIcon.click();
            if (isNeedAssert()) {
                Utils.assertObject(By.res(getSwitchIconResourceId()).desc(contentDesc));
            } else {
                Utils.waitObject(By.res(getSwitchIconResourceId()).desc(contentDesc),
                        Utils.TIME_OUT_SHORT_SHORT);
            }

        }
    }

    protected abstract String getSwitchIconContentDesc(int index);

    protected abstract String getSwitchIconResourceId();

    protected abstract boolean isNeedAssert();
}
