package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.Locale;

public class WhiteBalanceIndicatorChecker extends CheckerOne {
    @Override
    protected void doCheck() {
        if (TestContext.mLatestWhiteBalanceSettingValue.toLowerCase(Locale.ENGLISH)
                .equals("auto")) {
            Utils.assertNoObject(By.res("com.mediatek.camera:id/white_balance_indicator"));
        } else {
            Utils.assertObject(By.res("com.mediatek.camera:id/white_balance_indicator"));
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        if (TestContext.mLatestWhiteBalanceSettingValue.toLowerCase(Locale.ENGLISH)
                .equals("auto")) {
            return "Check white balance indicator is hidden";
        } else {
            return "Check white balance indicator is shown";
        }
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.white-balance");
    }
}
