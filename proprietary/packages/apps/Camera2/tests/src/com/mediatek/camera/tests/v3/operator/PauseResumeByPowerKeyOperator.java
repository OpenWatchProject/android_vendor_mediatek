package com.mediatek.camera.tests.v3.operator;

import android.os.RemoteException;
import android.support.test.uiautomator.By;
import android.view.KeyEvent;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class PauseResumeByPowerKeyOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PauseResumeByPowerKeyOperator.class
            .getSimpleName());
    private static final int TRY_UNLOCK_TIMES = 5;
    private static final int UNLOCK_SWIPE_STEPS = 5;

    @Override
    protected void doOperate() {
        String packageBeforeSuspend = Utils.getUiDevice().getCurrentPackageName();

        // power key to pause
        LogHelper.d(TAG, "[doOperate] Press power key to pause");
        Utils.getUiDevice().pressKeyCode(KeyEvent.KEYCODE_POWER);
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                try {
                    return Utils.getUiDevice().isScreenOn() == false;
                } catch (RemoteException e) {
                    return false;
                }
            }
        });
        // wait a moment
        Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT);
        // power key to resume
        LogHelper.d(TAG, "[doOperate] Press power key to resume");
        Utils.getUiDevice().pressKeyCode(KeyEvent.KEYCODE_POWER);
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                try {
                    return Utils.getUiDevice().isScreenOn() == true;
                } catch (RemoteException e) {
                    return false;
                }
            }
        });

        // wait old package or clock screen
        Utils.waitCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                boolean returnToPackage = Utils.getUiDevice().hasObject(
                        By.pkg(packageBeforeSuspend));
                boolean inLockScreen = Utils.getUiDevice().hasObject(
                        By.res("com.android.systemui:id/clock_view"));
                return returnToPackage || inLockScreen;
            }
        });

        // if there is lock screen, swipe to unlock
        if (Utils.findObject(By.res("com.android.systemui:id/clock_view"),
                Utils.TIME_OUT_SHORT_SHORT) != null) {
            int tryTimes = 0;
            boolean unlocked;
            do {
                int startX = Utils.getUiDevice().getDisplayWidth() / 2;
                int endX = Utils.getUiDevice().getDisplayWidth() / 2;
                int startY = Utils.getUiDevice().getDisplayHeight();
                int endY = Utils.getUiDevice().getDisplayHeight() / 4;
                LogHelper.d(TAG, "[doOperate] Try to unlock screen");
                Utils.getUiDevice().swipe(startX, startY, endX, endY, UNLOCK_SWIPE_STEPS);
                unlocked = Utils.waitNoObject(By.res("com.android.systemui:id/clock_view"),
                        Utils.TIME_OUT_SHORT_SHORT);
                tryTimes++;
            } while (tryTimes < TRY_UNLOCK_TIMES && unlocked == false);
        }
        Utils.assertObject(By.pkg(packageBeforeSuspend));
        LogHelper.d(TAG, "[doOperate] Not lock screen now");
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
        return "Press power key to pause, then power key again to resume";
    }
}
