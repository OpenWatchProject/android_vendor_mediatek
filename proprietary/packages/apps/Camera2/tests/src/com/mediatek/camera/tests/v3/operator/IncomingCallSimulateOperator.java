package com.mediatek.camera.tests.v3.operator;

import android.content.Intent;
import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class IncomingCallSimulateOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(IncomingCallSimulateOperator.class
            .getSimpleName());
    private static final String INCOMING_CALL_SIMULATE_PACKAGE = "com.android.gallery3d";

    @Override
    protected void doOperate() {
        LogHelper.d(TAG, "[doOperate] launch simulate activity");
        Intent intent = Utils.getContext().getPackageManager()
                .getLaunchIntentForPackage(INCOMING_CALL_SIMULATE_PACKAGE);
        Utils.getContext().startActivity(intent);
        Utils.assertObject(By.pkg(INCOMING_CALL_SIMULATE_PACKAGE));
        LogHelper.d(TAG, "[doOperate] activity launched, answer call for 10s");
        // wait 10s to simulate answer call
        Utils.waitSafely(10000);
        LogHelper.d(TAG, "[doOperate] Press back key to end call");
        // Press back key to simulate end call
        Utils.pressBackUtilFindNoObject(By.pkg(INCOMING_CALL_SIMULATE_PACKAGE), 2);
    }

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Simulate incoming call, answer call and end call";
    }
}
