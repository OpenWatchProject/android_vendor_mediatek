package com.mediatek.camera.tests.v3.operator;

import android.os.RemoteException;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

public class ClearRecentAppListOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ClearRecentAppListOperator.class
            .getSimpleName());

    @Override
    protected void doOperate() {
        try {
            // Enter recent app
            LogHelper.d(TAG, "[doOperate] Press recent app icon");
            Utils.getUiDevice().pressRecentApps();

            // Wait for system ui
            Utils.assertObject(By.pkg("com.android.systemui"));

            // wait no recent or clear all
            Utils.waitCondition(new Condition() {
                @Override
                public boolean isSatisfied() {
                    boolean noRecent = Utils.getUiDevice().hasObject(By.text("No recent items"));
                    if (noRecent) {
                        return true;
                    }
                    boolean clearAll = Utils.scrollUpOnScreenToFind(By.text("CLEAR ALL")) != null;
                    if (clearAll) {
                        return true;
                    }
                    return false;
                }
            });

            // if no recent
            if (Utils.getUiDevice().hasObject(By.text("No recent items"))) {
                LogHelper.d(TAG, "[doOperate] No recent items, press back and return");
                Utils.getUiDevice().pressBack();
                Utils.assertNoObject(By.text("No recent items"));
                return;
            }

            // if clear all
            // Click clear all
            UiObject2 clear = Utils.getUiDevice().findObject(By.text("CLEAR ALL"));
            Utils.assertRightNow(clear != null, "Cannot find CLEAR ALL button in recent apps");
            LogHelper.d(TAG, "[doOperate] Click clear all icon");
            clear.click();

            // Wait done
            Utils.assertNoObject(By.text("CLEAR ALL"));

        } catch (RemoteException e) {
            LogHelper.d(TAG, "[doOperate] RemoteException pop up", e);
            Utils.assertRightNow(false, "RemoteException pop up");
        }
    }

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Clear recent apps list";
    }
}
