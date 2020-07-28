package com.mediatek.camera.tests.v3.operator;

import android.os.Build;
import android.os.RemoteException;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.BySelector;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

public class PauseResumeByBackKeyOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PauseResumeByBackKeyOperator.class
            .getSimpleName());

    @Override
    protected void doOperate() {
        // Pause camera
        LogHelper.d(TAG, "[doOperate] Press back key and wait for camera exit");
        Utils.pressBackUtilFindNoObject(By.pkg("com.mediatek.camera"), 2);

        // Enter recent app
        try {
            LogHelper.d(TAG, "[doOperate] Press recent key");
            Utils.getUiDevice().pressRecentApps();
        } catch (RemoteException e) {
            LogHelper.d(TAG, "[doOperate] RemoteException pop up", e);
            Utils.assertRightNow(false);
        }

        // Resume camera
        if (Build.VERSION.SDK_INT >= 28) {
            Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT * 3);
            int width = Utils.getUiDevice().getDisplayWidth();
            int height = Utils.getUiDevice().getDisplayHeight();
            LogHelper.d(TAG, "[doOperate] Click camera to launch");
            Utils.getUiDevice().click(width / 2, height / 2);
        } else {
            UiObject2 scrollView = Utils.findObject(By.clazz("android.widget.ScrollView")
                    .hasChild(By.clazz("android.widget.FrameLayout")));
            Utils.assertRightNow(scrollView != null);

            List<UiObject2> appList = scrollView.getChildren();
            BySelector cameraSelector = By.res("com.android.systemui:id/title")
                    .clazz("android.widget.TextView").text("Camera");
            UiObject2 cameraEntry = appList.get(appList.size() - 1).findObject(cameraSelector);
            Utils.assertRightNow(cameraEntry != null);

            LogHelper.d(TAG, "[doOperate] Click camera to launch");
            cameraEntry.click();
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
        return "Press back key to exit, then relaunch from recent app list";
    }
}
