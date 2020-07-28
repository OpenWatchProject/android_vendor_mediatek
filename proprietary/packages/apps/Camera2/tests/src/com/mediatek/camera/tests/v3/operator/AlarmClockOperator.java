package com.mediatek.camera.tests.v3.operator;

import android.content.Intent;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * set alarm timer.
 */

public class AlarmClockOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            AlarmClockOperator.class.getSimpleName());
    private Integer mAlarmNumber;

    public AlarmClockOperator(int number) {
        mAlarmNumber = new Integer(number);
    }

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
        return "set alarm timer";
    }

    @Override
    protected void doOperate(int index) {
        Intent intent = Utils.getContext().getPackageManager()
                .getLaunchIntentForPackage("com.android.deskclock");
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK); // Clear out any previous instances
        Utils.getContext().startActivity(intent);
        Utils.assertObject(By.pkg("com.android.deskclock"));

        UiObject2 timer = Utils.findObject(By.desc("Timer"));
        timer.click();

        UiObject2 addTimer = Utils.findObject(By.desc("Add Timer"));
        if (addTimer != null) {
            addTimer.click();
            Utils.waitSafely(1000);
        }

        String numberString = mAlarmNumber.toString();
        for (int i = 0; i < numberString.length(); i++) {
            String subnum = numberString.substring(i, i + 1);
            UiObject2 num = Utils.findObject(By.text(subnum));
            if (num == null) {
                LogHelper.e(TAG, "can not set the timer to alarm, return.");
                Utils.getUiDevice().pressHome();
                return;
            }
            num.click();
            Utils.waitSafely(1000);
            LogHelper.d(TAG, "timer set num: " + subnum);
        }

        if (Utils.waitObject(By.desc("Start"))) {
            UiObject2 startButton = Utils.findObject(By.desc("Start"));
            startButton.click();
            Utils.waitSafely(1000);
        }
        Utils.getUiDevice().pressBack();
    }
}
