package com.mediatek.camera.tests.v3.util;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;
import android.support.test.uiautomator.UiWatcher;

import com.mediatek.camera.common.debug.LogUtil;

public class UnExceptedPopupUiWatcher implements UiWatcher {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            UnExceptedPopupUiWatcher.class.getSimpleName());

    @Override
    public boolean checkForCondition() {
        UiObject2 allow = Utils.getUiDevice().findObject(By.text("ALLOW"));
        if (allow != null) {
            LogHelper.d(TAG, "[checkForCondition] find ALLOW, click");
            allow.click();
        }

        UiObject2 agree = Utils.getUiDevice().findObject(By.text("AGREE"));
        if (agree != null) {
            LogHelper.d(TAG, "[checkForCondition] find AGREE, click");
            agree.click();
        }

        UiObject2 notResponding =
                Utils.getUiDevice().findObject(By.textContains("isn't responding"));
        UiObject2 closeApp = null;
        if (notResponding != null) {
            closeApp = Utils.getUiDevice().findObject(By.text("Close app"));
            if (closeApp != null) {
                closeApp.click();
            }
        }
        return allow != null || agree != null || (notResponding != null && closeApp != null);
    }
}
