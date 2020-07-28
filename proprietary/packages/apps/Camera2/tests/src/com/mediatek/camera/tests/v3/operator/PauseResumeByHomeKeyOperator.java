package com.mediatek.camera.tests.v3.operator;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Build;
import android.os.RemoteException;
import android.support.test.InstrumentationRegistry;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.BySelector;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

public class PauseResumeByHomeKeyOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PauseResumeByHomeKeyOperator.class
            .getSimpleName());

    /**
     * Default constructor.
     */
    public PauseResumeByHomeKeyOperator() {
    }

    @Override
    protected void doOperate() {
        // Pause camera
        LogHelper.d(TAG, "[doOperate] Press home key");
        Utils.getUiDevice().pressHome();

        // Wait for exit
        Utils.assertNoObject(By.pkg("com.mediatek.camera"));

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
        return "Press home key to pause, then relaunch from recent app list";
    }

    private static String getLauncherPackageName() {
        // Create launcher Intent
        final Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);

        // Use PackageManager to get the launcher package name
        PackageManager pm = InstrumentationRegistry.getContext().getPackageManager();
        ResolveInfo resolveInfo = pm.resolveActivity(intent, PackageManager.MATCH_DEFAULT_ONLY);
        return resolveInfo.activityInfo.packageName;
    }
}
