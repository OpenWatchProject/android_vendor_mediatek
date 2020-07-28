package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.Locale;

public class WhiteBalanceLogObserver extends LogPrintObserver {
    @Override
    protected String getObserveLogTag(int index) {
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
                return "CamAp_WhiteBalancePara";
            case API2:
                return "CamAp_WhiteBalanceCapt";
            default:
                return null;
        }
    }

    @Override
    protected String getObserveLogKey(int index) {
        return "value:" + TestContext.mLatestWhiteBalanceSettingValue
                .replace(" ", "-").toLowerCase(Locale.ENGLISH).toString();
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.white-balance");
    }

    @Override
    public int getObserveCount() {
        return 1;
    }
}
