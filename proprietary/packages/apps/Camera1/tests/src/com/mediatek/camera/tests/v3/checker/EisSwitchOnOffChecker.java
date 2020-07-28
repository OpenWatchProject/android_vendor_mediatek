package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class EisSwitchOnOffChecker extends SettingSwitchOnOffChecker {
    public EisSwitchOnOffChecker() {
        super("EIS");
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.eis");
    }
}
