package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check one setting item is switch on or off by UI status"})
public class SettingSwitchOnOffChecker extends Checker {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SettingSwitchOnOffChecker.class
            .getSimpleName());

    public static final int INDEX_SWITCH_ON = 0;
    public static final int INDEX_SWITCH_OFF = 1;

    private String mSettingTitle;

    public SettingSwitchOnOffChecker(String settingTitle) {
        mSettingTitle = settingTitle;
    }

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_SWITCH_ON:
                return "Check setting item [" + getSettingTitle() + "] is switched on";
            case INDEX_SWITCH_OFF:
                return "Check setting item [" + getSettingTitle() + "] is switched off";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        UiObject2 settingItemEntry = Utils.findObject(By.text(getSettingTitle()));
        Utils.assertRightNow(settingItemEntry != null, "Can not find setting item [" +
                getSettingTitle() + "]");

        UiObject2 switchButton = findSwitchButton(settingItemEntry);
        Utils.assertRightNow(switchButton != null, "Can not find switch button for setting item [" +
                getSettingTitle() + "]");

        switch (index) {
            case INDEX_SWITCH_ON:
                Utils.assertRightNow("ON".equals(switchButton.getText()),
                        "Expected switch button of setting item [" + getSettingTitle()
                                + "] is ON, but find " + switchButton.getText());
                break;
            case INDEX_SWITCH_OFF:
                Utils.assertRightNow("OFF".equals(switchButton.getText()),
                        "Expected switch button of setting item [" + getSettingTitle()
                                + "] is OFF, but find " + switchButton.getText());
                break;
        }
    }

    protected String getSettingTitle() {
        return mSettingTitle;
    }

    private UiObject2 findSwitchButton(UiObject2 object) {
        return object.getParent().getParent().findObject(By.clazz("android.widget.Switch"));
    }
}
