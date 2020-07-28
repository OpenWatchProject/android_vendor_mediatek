package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.checker.PageChecker;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

// Change setting radio values one by one, operation count is (radio options count + 1)
// or (radio options count), if you want to set radio value as origin value, set
// resetValueAsOriginAtTheEnd as true, and the operation count will be (radio options count + 1)
public class SettingRadioOptionsOneByOneOperator extends SettingRadioButtonOperator {

    private String mSettingTitle;
    protected String[] mOptionTitleArray;
    protected String[] mOptionSummaryArray;
    private boolean mResetValueAsOriginAtTheEnd;

    public SettingRadioOptionsOneByOneOperator(String settingTitle,
                                               boolean resetValueAsOriginAtTheEnd) {
        mSettingTitle = settingTitle;
        mResetValueAsOriginAtTheEnd = resetValueAsOriginAtTheEnd;
    }

    @Override
    protected int getSettingOptionsCount() {
        initSettingOptions();
        return mOptionTitleArray.length;
    }

    @Override
    protected String getSettingTitle() {
        return mSettingTitle;
    }

    @Override
    protected String getSettingOptionTitle(int index) {
        initSettingOptions();
        return mOptionTitleArray[index];
    }

    private void initSettingOptions() {
        if (mOptionTitleArray == null) {
            new SwitchPageOperator().operate(SwitchPageOperator.INDEX_SETTINGS);
            new PageChecker().check(PageChecker.INDEX_SETTINGS);

            // open radio list
            UiObject2 settingEntry = Utils.scrollOnScreenToFind(By.text(mSettingTitle));
            Utils.assertRightNow(settingEntry != null);
            settingEntry.click();
            Utils.assertObject(By.clazz("android.widget.RadioButton"));

            // get all radio options
            List<String> optionTitleList = new ArrayList<>();
            List<String> optionSummaryList = new ArrayList<>();
            getAllRadioOptions(optionTitleList, optionSummaryList);

            if (mResetValueAsOriginAtTheEnd) {
                mOptionTitleArray = new String[optionTitleList.size() + 1];
                mOptionSummaryArray = new String[optionSummaryList.size() + 1];
            } else {
                mOptionTitleArray = new String[optionTitleList.size()];
                mOptionSummaryArray = new String[optionSummaryList.size()];
            }
            optionTitleList.toArray(mOptionTitleArray);
            optionSummaryList.toArray(mOptionSummaryArray);

            // get origin value
            if (mResetValueAsOriginAtTheEnd) {
                String originTitleValue = getCurrentSelectedRadioTitle();
                mOptionTitleArray[mOptionTitleArray.length - 1] = originTitleValue;

                String originTitleSummary = getCurrentSelectedRadioSummary();
                if (originTitleSummary != null) {
                    mOptionSummaryArray[mOptionSummaryArray.length - 1] = originTitleSummary;
                }
            }

            // press back to return setting list
            Utils.getUiDevice().pressBack();
            Utils.assertNoObject(By.clazz("android.widget.RadioButton"));
        }
    }

    private void getAllRadioOptions(List<String> optionTitleList, List<String> optionSummaryList) {
        boolean hasAddRadioInThisLoop;
        do {
            hasAddRadioInThisLoop = false;
            UiObject2 radioList = Utils.findObject(By.res("android:id/list"));
            List<UiObject2> radioLayoutList = radioList.getChildren();
            int radioCount = radioLayoutList.size();
            for (int i = 0; i < radioCount; i++) {
                UiObject2 radioTitle = radioLayoutList.get(i).findObject(
                        By.clazz("android.widget.TextView").res("android:id/title"));
                if (radioTitle == null) {
                    continue;
                }

                String radioOption = radioTitle.getText();
                if (!optionTitleList.contains(radioOption)) {
                    optionTitleList.add(radioOption);
                    hasAddRadioInThisLoop = true;
                    UiObject2 summary = radioLayoutList.get(i).findObject(
                            By.clazz("android.widget.TextView").res("android:id/summary"));
                    if (summary != null) {
                        optionSummaryList.add(summary.getText());
                    }
                }
            }
            Utils.scrollDownOnObject(radioList);
        } while (hasAddRadioInThisLoop == true);
    }

    private String getCurrentSelectedRadioTitle() {
        Utils.scrollOnScreenToMapCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                UiObject2 radio = Utils.getUiDevice().findObject(
                        By.res("android:id/checkbox").checked(true));
                if (radio == null) {
                    return false;
                } else {
                    UiObject2 title = radio.getParent().getParent().findObject(
                            By.res("android:id/title"));
                    return title != null;
                }
            }
        }, 5);

        UiObject2 radioButton = Utils.findObject(By.res("android:id/checkbox").checked(true));
        Utils.assertRightNow(radioButton != null);

        UiObject2 title = radioButton.getParent().getParent().findObject(
                By.res("android:id/title"));
        Utils.assertRightNow(title != null);

        return title.getText();
    }

    private String getCurrentSelectedRadioSummary() {
        UiObject2 radioButton = Utils.findObject(By.res("android:id/checkbox").checked(true));
        Utils.assertRightNow(radioButton != null);

        UiObject2 summary = radioButton.getParent().getParent().findObject(
                By.res("android:id/summary"));

        if (summary == null) {
            return null;
        } else {
            return summary.getText();
        }
    }
}
