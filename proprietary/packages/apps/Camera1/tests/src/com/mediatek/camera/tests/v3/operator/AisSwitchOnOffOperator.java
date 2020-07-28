package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.util.Utils;

public class AisSwitchOnOffOperator extends SettingSwitchButtonOperator {
    public AisSwitchOnOffOperator() {
        super("Anti-shake");
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.anti-shake");
    }
}
