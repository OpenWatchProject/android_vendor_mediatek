package com.mediatek.camera.tests.v3.operator;

import android.content.Intent;
import android.os.Build;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.StaleObjectException;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class ConnectWifiOperator extends Operator {
    public static final int INDEX_CONNECT = 0;
    public static final int INDEX_DISCONNECT = 1;

    private static final LogUtil.Tag TAG = Utils.getTestTag(ConnectWifiOperator.class
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
        if (index == INDEX_CONNECT) {
            return "Connect to wifi";
        } else if (index == INDEX_DISCONNECT) {
            return "Disconnect wifi";
        } else {
            return null;
        }
    }

    @Override
    protected void doOperate(int index) {
        if (index == INDEX_CONNECT) {
            switchOn();
        } else if (index == INDEX_DISCONNECT) {
            switchOff();
        }
    }

    private void switchOn() {
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

        // open wifi page
        if (Build.VERSION.SDK_INT >= 26) {
            UiObject2 networkInternet = Utils.scrollOnScreenToFind(By.textContains("Network &"));
            Utils.assertRightNow(networkInternet != null,
                    "Can not find Network & in setting");
            networkInternet.click();
        }
        UiObject2 wifi = Utils.scrollOnScreenToFind(By.text("Wi‑Fi"));
        Utils.assertRightNow(wifi != null, "Can not find Wi‑Fi in setting");
        wifi.click();

        // if is switch on, not switch on again
        // if not switch off, switch on it
        UiObject2 switchButton = Utils.findObject(By.clazz("android.widget.Switch"));
        Utils.assertRightNow(switchButton != null, "Can not find switch button of Wi‑Fi");
        if (switchButton.isChecked() == false) {
            LogHelper.d(TAG, "[doOperate] Switch on wifi");
            switchButton.click();
            Utils.assertObject(By.clazz("android.widget.Switch").checked(true));
        }

        // if has connected to one, return
        if (Utils.waitObject(By.textContains("Connected"), Utils.TIME_OUT_NORMAL)) {
            LogHelper.d(TAG, "[doOperate] Already connected to one network, return");
            Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 5);
            return;
        }

        // if not connected one, connect
        Utils.tryToDoUntilCondition(new Runnable() {
            @Override
            public void run() {
                try {
                    UiObject2 wifiName = null;
                    for (int i = 0; i < Utils.WIFI_NAMES.length; i++) {
                        LogHelper.d(TAG, "[doOperate] start to find wifi " + Utils.WIFI_NAMES[i]);
                        wifiName = Utils.scrollOnScreenToFind(By.text(Utils.WIFI_NAMES[i]));
                        if (wifiName != null) {
                            LogHelper.d(TAG, "[doOperate] find wifi " + Utils.WIFI_NAMES[i]);
                            TestContext.mLatestWifiIndex = i;
                            break;
                        } else {
                            LogHelper.d(TAG, "[doOperate] not find wifi " + Utils.WIFI_NAMES[i]);
                        }
                    }
                    Utils.assertRightNow(wifiName != null, "Can not find suitable wifi");
                    LogHelper.d(TAG, "[doOperate] Click wifi name ["
                            + Utils.WIFI_NAMES[TestContext.mLatestWifiIndex] + "]");
                    wifiName.click();
                } catch (StaleObjectException e) {
                    LogHelper.d(TAG, "[doOperate] StaleObjectException");
                }
            }
        }, new Condition() {
            @Override
            public boolean isSatisfied() {
                return Utils.waitObject(By.clazz("android.widget.EditText"));
            }
        }, 3, "Can not find EditText for input password after click");

        // input password
        UiObject2 wifiPassWord = Utils.findObject(By.clazz("android.widget.EditText"));
        Utils.assertRightNow(wifiPassWord != null, "Can not find EditText for input password");
        LogHelper.d(TAG, "[doOperate] Input password ["
                + Utils.WIFI_PASSWORDS[TestContext.mLatestWifiIndex] + "]");
        wifiPassWord.setText(Utils.WIFI_PASSWORDS[TestContext.mLatestWifiIndex]);

        // click connect
        UiObject2 connect = Utils.findObject(By.text("CONNECT").clazz("android.widget.Button")
                .enabled(true));
        Utils.assertRightNow(connect != null, "Can not find CONNECT button");
        LogHelper.d(TAG, "[doOperate] Click CONNECT button");
        connect.click();

        // check connected
        Utils.assertObject(By.textContains("Connected"), Utils.TIME_OUT_LONG_LONG);
        LogHelper.d(TAG, "[doOperate] Wifi connected");

        // exit settings
        Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 5);
    }

    private void switchOff() {
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

        // open wifi page
        if (Build.VERSION.SDK_INT >= 26) {
            UiObject2 networkInternet = Utils.scrollOnScreenToFind(By.textContains("Network &"));
            Utils.assertRightNow(networkInternet != null,
                    "Can not find Network & in setting");
            networkInternet.click();
        }
        UiObject2 wifi = Utils.scrollOnScreenToFind(By.text("Wi‑Fi"));
        Utils.assertRightNow(wifi != null, "Can not find Wi‑Fi in setting");
        wifi.click();

        // if is switch off, not switch off again
        // if not switch off, switch off it
        UiObject2 switchButton = Utils.findObject(By.clazz("android.widget.Switch"));
        Utils.assertRightNow(switchButton != null, "Can not find switch button of Wi‑Fi");
        if (switchButton.isChecked() == true) {
            LogHelper.d(TAG, "[doOperate] Switch off wifi");
            switchButton.click();
            Utils.assertObject(By.clazz("android.widget.Switch").checked(false));
        }

        // exit settings
        Utils.pressBackUtilFindNoObject(By.pkg(SYSTEM_SETTING_PACKAGE), 5);
    }
}
