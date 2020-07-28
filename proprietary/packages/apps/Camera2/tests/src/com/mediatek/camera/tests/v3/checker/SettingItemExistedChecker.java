package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.operator.SwitchPageOperator;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check one setting item is existed in setting list or not"})
public class SettingItemExistedChecker extends Checker {
    public static final int INDEX_EXISTED = 0;
    public static final int INDEX_NOT_EXISTED = 1;

    private String mSettingTitle;
    private boolean mIsLaunchFromIntent;

    public SettingItemExistedChecker(String settingTitle, boolean isLaunchFromIntent) {
        mSettingTitle = settingTitle;
        mIsLaunchFromIntent = isLaunchFromIntent;
    }

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        switch (index) {
            case INDEX_EXISTED:
                return Page.SETTINGS;
            case INDEX_NOT_EXISTED:
                if (mIsLaunchFromIntent) {
                    return Page.SETTINGS;
                } else {
                    return Page.MODE_LIST;
                }
        }
        return null;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_EXISTED:
                return "Check setting item [" + getSettingTitle() + "] is existed";
            case INDEX_NOT_EXISTED:
                return "Check setting item [" + getSettingTitle() + "] is not existed";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        UiObject2 settingItem;
        switch (index) {
            case INDEX_EXISTED:
                settingItem = Utils.scrollOnScreenToFind(By.text(getSettingTitle()));
                Utils.assertRightNow(settingItem != null,
                        "Can not find setting item [" + getSettingTitle() + "]");
                break;
            case INDEX_NOT_EXISTED:
                if (mIsLaunchFromIntent) {
                    settingItem = Utils.scrollOnScreenToFindNo(By.text(getSettingTitle()),
                            Utils.SCROLL_TIMES_LESS);
                    Utils.assertRightNow(settingItem == null,
                            "Launch from intent, expected no setting item [" + getSettingTitle()
                                    + ", but find out");
                } else {
                    UiObject2 settingEntry = Utils.findObject(
                            By.res("com.mediatek.camera:id/setting_view"));
                    // when no setting entry, not existed is true
                    if (settingEntry == null) {
                        break;
                    } else {
                        new SwitchPageOperator().operate(SwitchPageOperator.INDEX_SETTINGS);
                        settingItem = Utils.scrollOnScreenToFindNo(By.text(getSettingTitle()),
                                Utils.SCROLL_TIMES_LESS);
                        Utils.assertRightNow(settingItem == null,
                                "Expected no setting item [" + getSettingTitle()
                                        + ", but find out");
                    }
                }
                break;
        }
    }

    protected String getSettingTitle() {
        return mSettingTitle;
    }
}
