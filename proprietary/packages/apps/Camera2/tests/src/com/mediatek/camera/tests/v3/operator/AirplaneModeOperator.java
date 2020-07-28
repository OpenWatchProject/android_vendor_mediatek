package com.mediatek.camera.tests.v3.operator;

import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.provider.Settings;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.ReflectUtils;
import com.mediatek.camera.tests.v3.util.Utils;

public class AirplaneModeOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            AirplaneModeOperator.class.getSimpleName());
    public static final int INDEX_ENABLE = 0;
    public static final int INDEX_DISABLE = 1;


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
        switch (index) {
            case INDEX_ENABLE:
                return "Enable airplane mode";
            case INDEX_DISABLE:
                return "Disable airplane mode";
            default:
                return null;
        }
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 object = Utils.scrollFromTopOnScreenToFind(
                By.clazz("android.widget.Switch").desc("Airplane mode"));
        Utils.assertRightNow(object != null);

        switch (index) {
            case INDEX_ENABLE:
                if (object.getText().equals("Off")) {
                    LogHelper.d(TAG, "[doOperate] airplane mode is off, click to turn on");
                    object.click();
                    Utils.assertObject(By.clazz("android.widget.Switch").desc("Airplane mode")
                            .text("On"));
                } else {
                    LogHelper.d(TAG, "[doOperate] airplane mode is on, do nothing");
                }
                break;
            case INDEX_DISABLE:
                if (object.getText().equals("On")) {
                    LogHelper.d(TAG, "[doOperate] airplane mode is on, click to turn off");
                    object.click();
                    Utils.assertObject(By.clazz("android.widget.Switch").desc("Airplane mode")
                            .text("Off"));
                } else {
                    LogHelper.d(TAG, "[doOperate] airplane mode is off, do nothing");
                }
                break;
            default:
                break;
        }

        object = Utils.pressBackUtilFindNoObject(By.pkg("com.android.systemui"), 2);
        Utils.assertRightNow(object == null);
    }

    private void setAirplaneMode(Context context, boolean enable) {
        // Change the system setting
        Settings.Global.putInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON,
                enable ? 1 : 0);

        // Post the intent
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intent.putExtra("state", enable);
        context.sendBroadcastAsUser(intent,
                (UserHandle) ReflectUtils.getFieldOnObject(
                        UserHandle.class,
                        "ALL",
                        UserHandle.class));
    }
}
