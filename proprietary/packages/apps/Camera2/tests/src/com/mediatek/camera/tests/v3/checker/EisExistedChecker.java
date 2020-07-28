package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class EisExistedChecker extends SettingItemExistedChecker {
    public EisExistedChecker(boolean isLaunchFromIntent) {
        super("EIS", isLaunchFromIntent);
    }

    @Override
    public boolean isSupported(int index) {
        if (index == SettingItemExistedChecker.INDEX_EXISTED) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.eis");
        } else {
            return true;
        }
    }
}
