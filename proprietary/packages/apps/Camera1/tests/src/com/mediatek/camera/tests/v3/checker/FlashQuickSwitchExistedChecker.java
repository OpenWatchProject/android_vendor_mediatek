package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class FlashQuickSwitchExistedChecker extends QuickSwitchExistedChecker {
    @Override
    protected String getSwitchIconResourceId() {
        return "com.mediatek.camera:id/flash_icon";
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_EXIST) {
            boolean auto = Utils.isFeatureSupported("com.mediatek.camera.at.flash.auto");
            boolean on = Utils.isFeatureSupported("com.mediatek.camera.at.flash.on");
            return auto || on;
        } else {
            return true;
        }
    }
}
