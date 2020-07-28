package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Utils;

public class WhiteBalanceSettingValueRealTimeChecker extends CheckerOne {
    @Override
    protected void doCheck() {
        new WhiteBalanceSettingValueChecker(TestContext.mLatestWhiteBalanceSettingValue).check(0);
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription() {
        return "Check the value of white balance in setting is same as last setting";
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.white-balance");
    }
}
