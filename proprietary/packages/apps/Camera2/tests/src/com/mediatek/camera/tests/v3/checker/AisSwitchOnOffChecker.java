package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class AisSwitchOnOffChecker extends SettingSwitchOnOffChecker {
    public AisSwitchOnOffChecker() {
        super("Anti-shake");
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.anti-shake");
    }
}
