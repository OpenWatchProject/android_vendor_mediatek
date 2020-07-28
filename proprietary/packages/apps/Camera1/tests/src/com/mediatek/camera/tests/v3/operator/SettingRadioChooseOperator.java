package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

public abstract class SettingRadioChooseOperator extends SettingConfigOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SettingRadioChooseOperator.class
            .getSimpleName());
    public static final int INDEX_FIRST = 0;
    public static final int INDEX_SECOND = 1;

    @Override
    public int getOperatorCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.SETTINGS;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_FIRST:
                return "Choose the first option of setting [" + getSettingTitle() + "]";
            case INDEX_SECOND:
                return "Choose the second option of setting [" + getSettingTitle() + "]";
            default:
                return null;
        }
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 settingItemEntry = Utils.scrollOnScreenToFind(By.text(getSettingTitle()));
        Utils.assertRightNow(settingItemEntry != null, "Can not find setting item [" +
                getSettingTitle() + "]");
        settingItemEntry.click();
        Utils.assertObject(By.clazz("android.widget.RadioButton"));

        UiObject2 radioList = Utils.findObject(By.res("android:id/list"));
        List<UiObject2> radioLayoutList = radioList.getChildren();
        Utils.assertRightNow(radioLayoutList.size() >= 2, "Radio options count < 2 of setting [" +
                getSettingTitle() + "]");

        UiObject2 radioButton = radioLayoutList.get(index).findObject(
                By.clazz("android.widget.RadioButton"));
        Utils.assertRightNow(radioButton != null, "Can not find radio button for " + index +
                "item for setting item [" + getSettingTitle() + "]");
        String title = radioLayoutList.get(index).findObject(By.res("android:id/title")).getText();
        String summary = radioLayoutList.get(index).findObject(By.res("android:id/summary"))
                .getText();
        radioButton.click();

        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                boolean titleAsSummary = Utils.getUiDevice().findObject(
                        By.res("android:id/summary")
                                .clazz("android.widget.TextView")
                                .text(title)) != null;
                boolean summaryAsSummary = Utils.getUiDevice().findObject(
                        By.res("android:id/summary").clazz("android.widget.TextView")
                                .text(summary)) != null;
                return titleAsSummary || summaryAsSummary;
            }
        });
    }

    protected abstract String getSettingTitle();
}
