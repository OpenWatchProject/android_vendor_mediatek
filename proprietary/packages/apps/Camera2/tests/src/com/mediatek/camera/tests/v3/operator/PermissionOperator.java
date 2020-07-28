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

import java.util.List;
import java.util.regex.Pattern;

public class PermissionOperator extends Operator {
    public static final int INDEX_ENABLE_LOCATION = 0;
    public static final int INDEX_ENABLE_ALL = 1;

    private static final LogUtil.Tag TAG = Utils.getTestTag(PermissionOperator.class
            .getSimpleName());
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
        if (index == INDEX_ENABLE_LOCATION) {
            return "Enable location permission of camera in settings";
        } else if (index == INDEX_ENABLE_ALL) {
            return "Enable all permissions of camera in settings";
        } else {
            return null;
        }
    }

    @Override
    protected void doOperate(int index) {
        // launch setting
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

        // open apps page
        if (Build.VERSION.SDK_INT >= 26) {
            UiObject2 appsNotifications =
                    Utils.scrollOnScreenToFind(By.text("Apps & notifications"));
            Utils.assertRightNow(appsNotifications != null,
                    "Can not find Apps & notifications in setting");
            appsNotifications.click();

            if (Build.VERSION.SDK_INT >= 28) {
                UiObject2 seeAll = Utils.scrollOnScreenToFind(
                        By.text(Pattern.compile(".*((?i)See all).*")));
                Utils.assertRightNow(seeAll != null,
                        "Can not find See all in setting");
                seeAll.click();
            } else {
                UiObject2 appInfo = Utils.scrollOnScreenToFind(By.text("App info"));
                Utils.assertRightNow(appInfo != null,
                        "Can not find App info in setting");
                appInfo.click();
            }
        } else {
            UiObject2 apps = Utils.scrollOnScreenToFind(By.text("Apps"));
            Utils.assertRightNow(apps != null);
            apps.click();
        }

        // open camera page
        UiObject2 camera = Utils.scrollOnScreenToFind(By.text("Camera"));
        Utils.assertRightNow(camera != null);
        camera.click();

        // open permissions
        UiObject2 permission = Utils.findObject(By.text("Permissions"));
        if (permission == null) {
            LogHelper.d(TAG, "[doOperate] no [Permissions] options, return");
            return;
        }
        Utils.assertRightNow(permission != null);
        permission.click();

        if (Build.VERSION.CODENAME.equals("Q") || Build.VERSION.SDK_INT >= 29) {
            Utils.assertObject(By.text("ALLOWED"));
        } else {
            Utils.assertObject(By.clazz("android.widget.Switch"));
        }
        enablePermissions(index);

        // exit settings
        Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 5);
    }

    private void enablePermissions(int index) {
        if (Build.VERSION.CODENAME.equals("Q") || Build.VERSION.SDK_INT >= 29) {
            UiObject2 permissionList = Utils.findObject(
                    By.res("com.android.permissioncontroller:id/recycler_view"));
            List<UiObject2> options = permissionList.getChildren();
            int i = 0;
            boolean deniedFind = false;
            for (; i < options.size(); i++) {
                if (!deniedFind) {
                    deniedFind = options.get(i).hasObject(By.text("DENIED"));
                    continue;
                } else {
                    UiObject2 permission = options.get(i).findObject(By.res("android:id/title"));
                    if (permission != null &&
                            !permission.getText().contains("No permissions denied") &&
                            ((index == INDEX_ENABLE_ALL) ||
                                    permission.getText().equals("Location"))) {
                        permission.click();
                        UiObject2 allow = Utils.findObject(
                                By.res("com.android.permissioncontroller:id/allow_radio_button"));
                        Utils.assertRightNow(allow != null);
                        allow.click();
                        Utils.pressBackUtilFindNoObject(By.textContains("ACCESS FOR THIS APP"), 2);
                    }
                }
            }
        } else {
            UiObject2 permissionList = Utils.findObject(By.clazz("android.widget.ListView"));
            List<UiObject2> options = permissionList.getChildren();
            for (int i = 0; i < options.size(); i++) {
                permissionList = Utils.findObject(By.clazz("android.widget.ListView"));
                options = permissionList.getChildren();
                if (index == INDEX_ENABLE_ALL) {
                    UiObject2 switchBtn = options.get(i).findObject(
                            By.clazz("android.widget.Switch"));
                    if (switchBtn != null && switchBtn.isChecked() == false) {
                        switchBtn.click();
                    }
                } else if (index == INDEX_ENABLE_LOCATION) {
                    UiObject2 locationOption = options.get(i).findObject(By.text("Location"));
                    if (locationOption != null) {
                        UiObject2 switchBtn = options.get(i).findObject(
                                By.clazz("android.widget.Switch"));
                        if (switchBtn.isChecked() == false) {
                            switchBtn.click();
                        }
                    }
                }
            }
        }
    }
}
