package com.mediatek.camera.tests.v3.util;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiWatcher;

import com.mediatek.camera.common.debug.LogUtil;

public class FocusOnSystemUiWatcher implements UiWatcher {
    private static final LogUtil.Tag TAG = Utils.getTestTag(FocusOnSystemUiWatcher.class
            .getSimpleName());

    @Override
    public boolean checkForCondition() {
        if (isFocusOnSystemUi()) {
            LogHelper.d(TAG, "[checkForCondition] focus on system ui, can not find ui on activity");

//            int width = Utils.getUiDevice().getDisplayWidth();
//            int height = Utils.getUiDevice().getDisplayHeight();
//
//            LogHelper.d(TAG, "[checkForCondition] click screen center to trigger right focus");
//            Utils.getUiDevice().click(width / 2, height / 2);
//
//            Utils.waitCondition(new Condition() {
//                @Override
//                public boolean isSatisfied() {
//                    return !isFocusOnSystemUi();
//                }
//            }, Utils.TIME_OUT_SHORT_SHORT);
//
//            if (isFocusOnSystemUi()) {
//                LogHelper.d(TAG, "[checkForCondition] after click, still focus on system ui");
//            } else {
//                LogHelper.d(TAG, "[checkForCondition] after click, not focus on system ui now");
//            }

            Utils.dump();
            return true;
        } else {
            return false;
        }
    }

    private boolean isFocusOnSystemUi() {
        boolean condition1 = "com.android.systemui".equals(
                Utils.getUiDevice().getCurrentPackageName());
        boolean condition2 = !Utils.getUiDevice()
                .hasObject(By.res("com.android.systemui:id/recents_view"));
        boolean condition3 = Utils.getUiDevice()
                .hasObject(By.res("com.android.systemui:id/nav_buttons"));
        boolean condition4 = !Utils.getUiDevice().hasObject(By.text("No recent items"));

        return condition1 && condition2 && condition3 && condition4;
    }
}
