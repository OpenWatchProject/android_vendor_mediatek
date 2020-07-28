package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class IsoExistedChecker extends SettingItemExistedChecker {
    public IsoExistedChecker(boolean isLaunchFromIntent) {
        super("ISO", isLaunchFromIntent);
    }

    @Override
    public boolean isSupported(int index) {
        if (index == SettingItemExistedChecker.INDEX_EXISTED) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.iso");
        } else {
            return true;
        }
    }
}
