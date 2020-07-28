package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class FlashQuickSwitchChecker extends QuickSwitchChecker {
    public static final int INDEX_AUTO = 0;
    public static final int INDEX_ON = 1;
    public static final int INDEX_OFF = 2;

    private static final String SWITCH_ICON_RESOURCE = "com.mediatek.camera:id/flash_icon";
    private static final String[] SWITCH_IDON_DESCRIPTION = new String[]{
            "flash auto",
            "flash on",
            "flash off",
    };

    @Override
    protected String getSwitchIconResourceId() {
        return SWITCH_ICON_RESOURCE;
    }

    @Override
    protected String getSwitchIconDescription(int index) {
        return SWITCH_IDON_DESCRIPTION[index];
    }

    @Override
    protected int getSwitchIconStatusCount() {
        return SWITCH_IDON_DESCRIPTION.length;
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_AUTO) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.flash.auto");
        } else if (index == INDEX_OFF) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.flash.off");
        } else if (index == INDEX_ON) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.flash.on");
        } else {
            return false;
        }
    }
}
