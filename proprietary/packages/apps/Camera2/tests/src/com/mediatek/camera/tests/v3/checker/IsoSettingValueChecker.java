package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class IsoSettingValueChecker extends SettingRadioOptionChecker {
    public IsoSettingValueChecker(String isoValue) {
        super("ISO", isoValue);
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.iso");
    }
}
