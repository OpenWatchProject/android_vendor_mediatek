package com.mediatek.camera.tests.v3.operator;

import android.content.Intent;
import android.os.Build;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class LocationOperator extends Operator {
    public static final int INDEX_OFF = 0;
    public static final int INDEX_ON_HIGH_ACCURACY = 1;

    private static final LogUtil.Tag TAG = Utils.getTestTag(LocationOperator.class
            .getSimpleName());
    private static final String[] LOCATION_MODE = new String[]{
            "",
            "High accuracy"};

    private static final String SYSTEM_SETTING_PACKAGE = "com.android.settings";

    @Override
    protected void doOperate(int index) {
        // launch setting
        Intent intent = Utils.getContext().getPackageManager()
                .getLaunchIntentForPackage(SYSTEM_SETTING_PACKAGE);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK); // Clear out any previous instances
        Utils.getContext().startActivity(intent);
        LogHelper.d(TAG, "[doOperate] wait SYSTEM_SETTING_PACKAGE");
        Utils.assertObject(By.pkg(SYSTEM_SETTING_PACKAGE));
        LogHelper.d(TAG, "[doOperate] wait setting list show out");
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                UiObject2 list = Utils.getUiDevice().findObject(
                        Utils.getSystemSettingListSelector());
                return list != null && list.getChildCount() > 0;
            }
        }, Utils.TIME_OUT_LONG_LONG);
        LogHelper.d(TAG, "[doOperate] setting list showed out");

        // open location page
        if (Build.VERSION.SDK_INT >= 26) {
            UiObject2 securityLocation = Utils.scrollOnScreenToFind(By.textContains("ocation"));
            Utils.assertRightNow(securityLocation != null,
                    "Can not find text contains \'ocation\' in setting");
            securityLocation.click();
        }
        UiObject2 location = Utils.scrollOnScreenToFind(By.text("Location"));
        Utils.assertRightNow(location != null);
        location.click();

        UiObject2 switchButton = Utils.findObject(By.clazz("android.widget.Switch"));
        Utils.assertRightNow(switchButton != null);
        if (index == INDEX_OFF) {
            if (switchButton.isChecked() == false) {
                LogHelper.d(TAG, "[doOperate] already off, do nothing");
            } else {
                switchButton.click();
                Utils.assertObject(By.clazz("android.widget.Switch").checked(false));
            }
        } else {
            // if is switch on, not switch on again
            // if not switch off, switch on it
            if (switchButton.isChecked() == false) {
                switchButton.click();
                Utils.assertObject(By.clazz("android.widget.Switch").checked(true));
            }

            if (Build.VERSION.SDK_INT < 28) {
                // open mode page
                UiObject2 mode = Utils.findObject(By.text("Mode"));
                Utils.assertRightNow(mode != null);
                mode.click();
                Utils.assertObject(By.text("Location mode"));

                // enable index mode
                UiObject2 indexMode = Utils.findObject(By.text(LOCATION_MODE[index]));
                Utils.assertRightNow(indexMode != null);
                UiObject2 radio = indexMode.getParent().getParent().findObject(
                        By.clazz("android.widget.RadioButton"));
                Utils.assertRightNow(radio != null);
                radio.click();

                // check if there is Location Consent pop up, if yes, click agree
                UiObject2 agree = Utils.findObject(By.text("AGREE"), Utils.TIME_OUT_SHORT);
                if (agree != null) {
                    agree.click();
                    Utils.waitNoObject(By.text("AGREE"));
                }

                // check enabled
                Utils.assertCondition(new Condition() {
                    @Override
                    public boolean isSatisfied() {
                        UiObject2 accuracy = Utils.findObject(By.text(LOCATION_MODE[index]));
                        UiObject2 radio = accuracy.getParent().getParent().findObject(
                                By.clazz("android.widget.RadioButton"));
                        return radio.isChecked();
                    }
                });
            }
        }

        // exit settings
        Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 3);
    }

    @Override
    public int getOperatorCount() {
        return 2;
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
        switch (index) {
            case INDEX_OFF:
                return "Disable location in system settings";
            case INDEX_ON_HIGH_ACCURACY:
                return "Enable location in [" + LOCATION_MODE[index] + "] mode";
            default:
                return null;
        }
    }
}
