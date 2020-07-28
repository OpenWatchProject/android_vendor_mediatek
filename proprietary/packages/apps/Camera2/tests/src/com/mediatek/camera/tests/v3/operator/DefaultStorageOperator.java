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

public class DefaultStorageOperator extends Operator {
    public static final int INDEX_INTERNAL_STORAGE = 0;
    public static final int INDEX_SD_CARD = 1;

    private static final LogUtil.Tag TAG = Utils.getTestTag(DefaultStorageOperator.class
            .getSimpleName());
    private static final String[] DEFAULT_STORAGE = new String[]{
            "Internal shared storage",
            "SD card"
    };

    private static final String SYSTEM_SETTING_PACKAGE = "com.android.settings";

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
        return "Change default storage to " + DEFAULT_STORAGE[index] + " in system settings";
    }

    @Override
    protected void doOperate(int index) {
        if (Build.VERSION.SDK_INT >= 28) {
            LogHelper.d(TAG,
                    "[doOperate] after P, can not change default storage, do nothing, return");
            return;
        }

        // check default storage first
        String defaultPath = Utils.getDefaultStoragePath();
        switch (index) {
            case INDEX_INTERNAL_STORAGE:
                if (defaultPath != null && defaultPath.startsWith("/storage/emulated/")) {
                    LogHelper.d(TAG,
                            "[doOperate] Default storage is internal storage already, return");
                    return;
                }
                break;
            case INDEX_SD_CARD:
                if (defaultPath != null && defaultPath.startsWith("/storage/")
                        && !defaultPath.startsWith("/storage/emulated/")) {
                    LogHelper.d(TAG, "[doOperate] Default storage is sd card already, return");
                    return;
                }
                break;
            default:
                break;
        }

        // launch settings
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

        // open storage page
        UiObject2 storage = Utils.scrollOnScreenToFind(By.text("Storage"));
        Utils.assertRightNow(storage != null, "Can not find Storage item in setting");
        storage.click();

        //choose default storage
        Utils.assertObject(By.text("Default write disk"));
        setUpSdCardIfCorrupted();
        UiObject2 defaultStorage = Utils.scrollOnScreenToFind(
                By.textContains(DEFAULT_STORAGE[index]));
        if (Build.VERSION.SDK_INT >= 26
                && defaultStorage == null && index == INDEX_INTERNAL_STORAGE) {
            Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 2);
            return;
        } else {
            Utils.assertRightNow(defaultStorage != null,
                    "Can not find Default write disk in Storage");
        }

        UiObject2 defaultRadio = defaultStorage.getParent().getParent().findObject(
                By.clazz("android.widget.RadioButton"));
        Utils.assertRightNow(defaultRadio != null,
                "Can not find radio button of Default write disk");
        defaultRadio.click();

        // exit settings
        Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 2);
    }

    private void setUpSdCardIfCorrupted() {
        UiObject2 corrupted = Utils.scrollOnScreenToFind(
                By.text("Corrupted"), Utils.SCROLL_TIMES_LESS);
        if (corrupted == null) {
            LogHelper.d(TAG, "[setUpSdCardIfCorrupted] not find Corrupted, return");
            return;
        }
        corrupted.click();

        UiObject2 setup = Utils.findObject(By.text("SET UP"));
        Utils.assertRightNow(setup != null, "Not find set up button after click sd card");
        setup.click();

        UiObject2 portableStorage = Utils.findObject(By.text("Use as portable storage"));
        Utils.assertRightNow(portableStorage != null,
                "Not find <Use as portable storage> after click set up");
        portableStorage.click();

        UiObject2 next = Utils.findObject(
                By.clazz("android.widget.Button").text("NEXT").enabled(true));
        Utils.assertRightNow(next != null,
                "Not find <NEXT> button after click <Use as portable storage>");
        next.click();

        UiObject2 eraseAndFormat = Utils.findObject(
                By.clazz("android.widget.Button").text("ERASE & FORMAT").enabled(true));
        Utils.assertRightNow(eraseAndFormat != null,
                "Not find <ERASE & FORMAT> button after click <NEXT>");
        eraseAndFormat.click();

        UiObject2 done = Utils.findObject(
                By.clazz("android.widget.Button").text("DONE").enabled(true),
                Utils.TIME_OUT_LONG_LONG);
        Utils.assertRightNow(done != null,
                "Not find <DONE> button after click <ERASE & FORMAT>");
        done.click();

        Utils.assertObject(By.text("Default write disk"));
    }
}