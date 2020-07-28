package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.util.Utils;

public class OpenFlashQuickSwitchOperator extends OpenQuickSwitchOptionOperator {
    private static final String SWITCH_ICON_RESOURCE = "com.mediatek.camera:id/flash_icon";

    @Override
    protected String getSwitchIconResourceId() {
        return SWITCH_ICON_RESOURCE;
    }

    @Override
    public boolean isSupported(int index) {
        boolean auto = Utils.isFeatureSupported("com.mediatek.camera.at.flash.auto");
        boolean on = Utils.isFeatureSupported("com.mediatek.camera.at.flash.on");
        return auto || on;
    }
}
