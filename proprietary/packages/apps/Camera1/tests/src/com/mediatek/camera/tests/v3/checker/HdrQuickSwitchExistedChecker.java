package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class HdrQuickSwitchExistedChecker extends QuickSwitchExistedChecker {
    @Override
    protected String getSwitchIconResourceId() {
        return "com.mediatek.camera:id/hdr_icon";
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_EXIST) {
            boolean auto = Utils.isFeatureSupported("com.mediatek.camera.at.hdr.auto");
            boolean on = Utils.isFeatureSupported("com.mediatek.camera.at.hdr.on");
            return auto || on;
        } else {
            return true;
        }
    }
}
