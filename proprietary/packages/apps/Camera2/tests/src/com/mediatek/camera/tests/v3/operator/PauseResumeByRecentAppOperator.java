package com.mediatek.camera.tests.v3.operator;

import android.os.Build;
import android.os.RemoteException;
import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class PauseResumeByRecentAppOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PauseResumeByRecentAppOperator.class
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
            if (Build.VERSION.SDK_INT >= 28) {
                Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT * 3);
                int width = Utils.getUiDevice().getDisplayWidth();
                int height = Utils.getUiDevice().getDisplayHeight();
                LogHelper.d(TAG, "[doOperate] Click camera to launch");
                Utils.getUiDevice().click(width / 2, height / 2);
            } else {
                Utils.getUiDevice().pressBack();
            }
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