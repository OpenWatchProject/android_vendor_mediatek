package com.mediatek.camera.tests.v3.operator;

import android.content.Intent;
import android.os.Build;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

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
                        By.res("com.android.settings:id/dashboard_container"));
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

            UiObject2 appInfo = Utils.scrollOnScreenToFind(By.text("App info"));
            Utils.assertRightNow(appInfo != null,
                    "Can not find App info in setting");
            appInfo.click();
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
        Utils.assertObject(By.clazz("android.widget.Switch"));

        enablePermissions(index);
        // exit settings
        Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 4);
    }

    private void enablePermissions(int index) {
        UiObject2 permissionList = Utils.findObject(By.clazz("android.widget.ListView"));
        List<UiObject2> options = permissionList.getChildren();
        for (int i = 0; i < options.size(); i++) {
            permissionList = Utils.findObject(By.clazz("android.widget.ListView"));
            options = permissionList.getChildren();
            if (index == INDEX_ENABLE_ALL) {
                UiObject2 switchBtn = options.get(i).findObject(By.clazz("android.widget.Switch"));
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
