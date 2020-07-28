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

public class FontSizeOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(FontSizeOperator.class.getSimpleName());
    public static final int INDEX_SMALL = 0;
    public static final int INDEX_NORMAL = 1;
    public static final int INDEX_LARGE = 2;
    public static final int INDEX_HUGE = 3;

    private static final String SYSTEM_SETTING_PACKAGE = "com.android.settings";

    private static final String[] FONT_SIZE_LIST_M = new String[]{
            "Small", "Normal", "Large", "Huge"
    };
    private static final String[] FONT_SIZE_LIST_N = new String[]{
            "Small", "Default", "Large", "Largest"
    };

    @Override
    public int getOperatorCount() {
        return 4;
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
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return "Set system font size as " + FONT_SIZE_LIST_N[index] + " in settings";
        } else {
            return "Set system font size as " + FONT_SIZE_LIST_M[index] + " in settings";
        }
    }

    @Override
    protected void doOperate(int index) {
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

        UiObject2 display = Utils.scrollOnScreenToFind(By.text("Display"));
        Utils.assertRightNow(display != null);
        display.click();

        if (Build.VERSION.SDK_INT >= 26) {
            UiObject2 advanced = Utils.scrollOnScreenToFind(By.text("Advanced"));
            Utils.assertRightNow(advanced != null,
                    "Can not find Advanced in setting");
            advanced.click();
        }

        UiObject2 fontSize = Utils.scrollOnScreenToFind(By.text("Font size"));
        Utils.assertRightNow(fontSize != null);
        fontSize.click();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            chooseFontSizeAndroidN(index);
        } else {
            chooseFontSizeAndroidM(index);
        }

        exitSettings();
    }

    private void chooseFontSizeAndroidN(int index) {
        UiObject2 radio = Utils.findObject(By.clazz("android.widget.RadioButton").desc
                (FONT_SIZE_LIST_N[index]));
        Utils.assertRightNow(radio != null);
        radio.click();
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                return Utils.findObject(By.clazz("android.widget.RadioButton").desc
                        (FONT_SIZE_LIST_N[index])).isChecked();
            }
        });
    }

    private void chooseFontSizeAndroidM(int index) {
        UiObject2 radio = Utils.findObject(By.clazz("android.widget.CheckedTextView").desc
                (FONT_SIZE_LIST_M[index]));
        Utils.assertRightNow(radio != null);
        radio.click();
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                return Utils.findObject(By.clazz("android.widget.CheckedTextView").desc
                        (FONT_SIZE_LIST_M[index])).isChecked();
            }
        });
    }

    private void exitSettings() {
        Utils.getUiDevice().pressBack();
        Utils.getUiDevice().pressBack();
        Utils.assertNoObject(By.text("Font size"));
        Utils.getUiDevice().pressBack();
        Utils.assertNoObject(By.pkg(SYSTEM_SETTING_PACKAGE));
    }
}
