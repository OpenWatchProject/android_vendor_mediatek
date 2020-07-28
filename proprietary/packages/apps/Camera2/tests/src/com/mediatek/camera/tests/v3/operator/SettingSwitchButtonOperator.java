package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class SettingSwitchButtonOperator extends SettingConfigOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SettingSwitchButtonOperator.class
            .getSimpleName());
    public static final int INDEX_SWITCH_ON = 0;
    public static final int INDEX_SWITCH_OFF = 1;

    private String mSettingTitle;

    public SettingSwitchButtonOperator(String settingTitle) {
        mSettingTitle = settingTitle;
    }

    @Override
    public int getOperatorCount() {
        return 2;
    }

    @Override
    protected void doOperate(int index) {
        switch (index) {
            case INDEX_SWITCH_ON:
                Utils.assertRightNow(switchOn(mSettingTitle));
                break;
            case INDEX_SWITCH_OFF:
                Utils.assertRightNow(switchOff(mSettingTitle));
                break;
        }
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
            case INDEX_SWITCH_ON:
                return "Switch setting [" + mSettingTitle + "] on";
            case INDEX_SWITCH_OFF:
                return "Switch setting [" + mSettingTitle + "] off";
        }
        return null;
    }

    private boolean switchOn(String settingName) {
        UiObject2 switchButton = findSwitchButton(settingName);
        if (switchButton == null) {
            LogHelper.d(TAG, "[switchOn] switchButton is null, return");
            return false;
        } else if ("ON".equals(switchButton.getText())) {
            return true;
        } else {
            switchButton.click();
        }
        return Utils.waitCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                UiObject2 switchButton = findSwitchButton(settingName);
                return "ON".equals(switchButton.getText());
            }
        });
    }

    private boolean switchOff(String settingName) {
        UiObject2 switchButton = findSwitchButton(settingName);
        if (switchButton == null) {
            LogHelper.d(TAG, "[switchOff] switchButton is null, return");
            return false;
        } else if ("OFF".equals(switchButton.getText())) {
            return true;
        } else {
            switchButton.click();
        }
        return Utils.waitCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                UiObject2 switchButton = findSwitchButton(settingName);
                return "OFF".equals(switchButton.getText());
            }
        });
    }

    private UiObject2 findSwitchButton(String settingName) {
        UiObject2 settingItem = Utils.scrollOnScreenToFind(By.text(settingName));
        if (settingItem == null) {
            LogHelper.d(TAG, "[findSwitchButton] not find setting [" + settingName + "], return");
            return null;
        } else {
            return settingItem.getParent().getParent()
                    .findObject(By.clazz("android.widget.Switch"));
        }
    }
}
