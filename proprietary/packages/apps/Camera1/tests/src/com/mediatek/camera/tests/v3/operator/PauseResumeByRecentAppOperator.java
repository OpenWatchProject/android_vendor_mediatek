package com.mediatek.camera.tests.v3.operator;

import android.os.RemoteException;
import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class PauseResumeByRecentAppOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PauseResumeByHomeKeyOperator.class
            .getSimpleName());

    @Override
    protected void doOperate() {
        try {
            // Enter recent app
            LogHelper.d(TAG, "[doOperate] Press recent to pause");
            Utils.getUiDevice().pressRecentApps();

            // Wait for exit
            Utils.assertNoObject(By.pkg("com.mediatek.camera"));

            // Return to camera
            Utils.getUiDevice().pressBack();
        } catch (RemoteException e) {
            LogHelper.d(TAG, "[doOperate] RemoteException pop up", e);
            Utils.assertRightNow(false);
        }
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Press recent app to pause, then press back to resume";
    }

}