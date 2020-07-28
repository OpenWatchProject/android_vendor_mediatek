package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

// Change all setting values in one operation, only one operation count
public class ChangeAllSettingOneByOneOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ChangeAllSettingOneByOneOperator.class
            .getSimpleName());
    private static final int OPERATE_TYPE_SWITCH = 0;
    private static final int OPERATE_TYPE_RADIO = 1;

    @Override
    protected void doOperate() {
        UiObject2 settingEnter = Utils.findObject(By.res("com.mediatek" +
                ".camera:id/setting_view").clickable(true));
        if (settingEnter != null) {
            LogHelper.d(TAG, "[doOperate] enter setting");
            settingEnter.click();
        } else if (Utils.findObject(By.text("Settings")) != null) {
            LogHelper.d(TAG, "[doOperate] already in setting");
        } else {
            LogHelper.d(TAG, "[doOperate] settingEnter is null, return");
            return;
        }

        List<String> hasChangedSettingList = new ArrayList<>();
        boolean hasChangeSettingInThisLoop;
        do {
            hasChangeSettingInThisLoop = false;

            Utils.assertCondition(new Condition() {
                @Override
                public boolean isSatisfied() {
                    return getCurrentSettingList().getChildCount() > 0;
                }
            });

            // get setting name and operate type
            UiObject2 settingList = getCurrentSettingList();
            int settingNum = settingList.getChildCount();
            String[] settingNameList = new String[settingNum];
            int[] settingOperateList = new int[settingNum];
            List<UiObject2> settingUiObjectList = settingList.getChildren();
            for (int i = 0; i < settingNum; i++) {
                UiObject2 settingTitle = settingUiObjectList.get(i)
                        .findObject(By.res("android:id/title"));
                if (settingTitle == null) {
                    continue;
                }
                settingNameList[i] = settingTitle.getText();
                settingOperateList[i] = getSettingOperateType(settingUiObjectList.get(i));
            }

            // chang setting value by operation type
            for (int i = 0; i < settingNum; i++) {
                if (settingNameList[i] != null
                        && hasChangedSettingList.contains(settingNameList[i]) == false) {
                    if (settingNameList[i].equals("Image properties")) {
                        //
                    } else if (settingOperateList[i] == OPERATE_TYPE_RADIO) {
                        changeRadioSettingValue(settingNameList[i]);
                    } else {
                        changeSwitchSettingValue(settingNameList[i]);
                    }
                    // if has changed on setting, add to hasChangedSettingList
                    hasChangedSettingList.add(settingNameList[i]);
                    hasChangeSettingInThisLoop = true;
                }
            }

            // Scroll list
            Utils.scrollDownOnObject(getCurrentSettingList());
        } while (hasChangeSettingInThisLoop == true); // do util no new setting

        Utils.pressBackUtilFindNoObject(By.text("settings"), 1);
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.MODE_LIST;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Change all settings value in current mode one by one";
    }

    private UiObject2 getCurrentSettingList() {
        UiObject2 settingContainer = Utils.findObject(
                By.res("com.mediatek.camera:id/setting_container"));
        Utils.assertRightNow(settingContainer != null, "Can not find setting_container");
        UiObject2 settingList = settingContainer.findObject(By.clazz("android.widget.ListView"));
        Utils.assertRightNow(settingList != null, "Can not find ListView in setting_container");
        return settingList;
    }

    private int getSettingOperateType(UiObject2 settingItemRelativeLayout) {
        if (settingItemRelativeLayout.findObject(By.clazz("android.widget.Switch")) != null) {
            return OPERATE_TYPE_SWITCH;
        } else {
            return OPERATE_TYPE_RADIO;
        }
    }

    private UiObject2 getSwitchButton(String settingTitle) {
        return Utils.findObject(By.text(settingTitle)).getParent().getParent()
                .findObject(By.clazz("android.widget.Switch"));
    }

    private void changeSwitchSettingValue(String settingTitle) {
        Operator switchOperator = new SettingSwitchButtonOperator(settingTitle);
        UiObject2 switchButton = getSwitchButton(settingTitle);
        if (switchButton.getText().equals("ON")) {
            switchOperator.operate(SettingSwitchButtonOperator.INDEX_SWITCH_OFF);
            switchOperator.operate(SettingSwitchButtonOperator.INDEX_SWITCH_ON);
        } else {
            switchOperator.operate(SettingSwitchButtonOperator.INDEX_SWITCH_ON);
            switchOperator.operate(SettingSwitchButtonOperator.INDEX_SWITCH_OFF);
        }
    }

    private void changeRadioSettingValue(String settingTitle) {
        Operator operator = new SettingRadioOptionsOneByOneOperator(
                settingTitle, true);
        int count = operator.getOperatorCount();
        operator.ignoreBeforePageCheck();
        operator.ignoreAfterPageCheck();
        for (int i = 0; i < count; i++) {
            operator.operate(i);
        }
    }
}
