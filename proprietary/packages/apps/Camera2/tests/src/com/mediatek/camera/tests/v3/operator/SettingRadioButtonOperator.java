package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.StaleObjectException;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

public abstract class SettingRadioButtonOperator extends SettingConfigOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SettingRadioButtonOperator.class
            .getSimpleName());

    @Override
    public int getOperatorCount() {
        return getSettingOptionsCount();
    }

    @Override
    protected void doOperate(int index) {
        Utils.assertRightNow(setSettingRadioButton(getSettingTitle(), getSettingOptionTitle
                (index)));
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
        return "Set [" + getSettingTitle() + "] as " + getSettingOptionTitle(index);
    }

    protected abstract int getSettingOptionsCount();

    protected abstract String getSettingTitle();

    protected abstract String getSettingOptionTitle(int index);

    public boolean setSettingRadioButton(String title, String option) {
        LogHelper.d(TAG, "[setSettingRadioButton] title = " + title + ", option = " + option);
        UiObject2 settingItemEntry = Utils.scrollOnScreenToFind(By.text(title));
        if (settingItemEntry == null) {
            LogHelper.d(TAG, "[setSettingRadioButton] settingItemEntry is null, return false");
            return false;
        }
        settingItemEntry.click();
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                UiObject2 toolbar =
                        Utils.findObject(By.res("com.mediatek.camera:id/toolbar"));
                if (toolbar == null) {
                    return false;
                } else {
                    return toolbar.hasObject(By.text(title));
                }
            }
        });
        UiObject2 optionEntry = Utils.scrollOnScreenToFind(By.text(option));
        if (optionEntry == null) {
            Utils.getUiDevice().pressBack();
            LogHelper.d(TAG, "[setSettingRadioButton] optionEntry is null, return false");
            return false;
        }

        int tryTimes = 10;
        for (int i = 0; i < tryTimes; i++) {
            try {
                List<UiObject2> titleAndSummary = optionEntry.getParent().getChildren();
                String optionTitleText = titleAndSummary.get(0).getText();
                String optionSummaryText = titleAndSummary.size() == 2 ?
                        titleAndSummary.get(1).getText() : "summaryText";

                UiObject2 radioButton = findRadioButton(optionEntry);
                if (radioButton == null) {
                    Utils.getUiDevice().pressBack();
                    LogHelper.d(TAG, "[setSettingRadioButton] radioButton is null, return false");
                    return false;
                } else if (radioButton.isChecked()) {
                    Utils.getUiDevice().pressBack();
                    LogHelper.d(TAG,"[setSettingRadioButton] radioButton is checked " +
                            "already, return true");
                    return true;
                }
                LogHelper.d(TAG, "[setSettingRadioButton] click");
                radioButton.click();

                return Utils.waitCondition(new Condition() {
                    @Override
                    public boolean isSatisfied() {
                        boolean titleAsSummary = Utils.scrollOnScreenToFind(
                                By.res("android:id/summary")
                                        .clazz("android.widget.TextView")
                                        .text(optionTitleText)) != null;
                        boolean summaryAsSummary = Utils.scrollOnScreenToFind(
                                By.res("android:id/summary").clazz("android.widget.TextView")
                                        .text(optionSummaryText)) != null;
                        return titleAsSummary || summaryAsSummary;
                    }
                });
            } catch (StaleObjectException e) {
                LogHelper.e(TAG, "[setSettingRadioButton] StaleObjectException " + e + ", retry");
            }
        }
        Utils.assertRightNow(false);
        return false;
    }

    private UiObject2 findRadioButton(UiObject2 object) {
        return object.getParent().getParent().findObject(By.clazz("android.widget.RadioButton"));
    }
}
