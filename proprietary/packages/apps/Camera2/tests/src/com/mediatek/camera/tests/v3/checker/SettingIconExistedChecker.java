package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check whether setting icon is existed.
 */
public class SettingIconExistedChecker extends CheckerOne {

    private boolean mIsExisted;

    /**
     * the construction.
     *
     * @param isExisted the icon is hope to existed.
     */
    public SettingIconExistedChecker(boolean isExisted) {
        mIsExisted = isExisted;
    }

    @Override
    public int getCheckCount() {
        return 1;
    }

    @Override
    protected void doCheck() {
        UiObject2 settingEntry = Utils.findObject(By.res("com.mediatek.camera:id/setting_view"));
        if (mIsExisted) {
            Utils.assertRightNow(settingEntry != null, "Can not find setting entry");
        } else {
            Utils.assertRightNow(settingEntry == null, "Expected no setting entry, but find out");
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.MODE_LIST;
    }

    @Override
    public String getDescription() {
        if (mIsExisted) {
            return "Check setting icon is existed in mode list";
        } else {
            return "Check setting icon is not existed in mode list";
        }
    }

}
