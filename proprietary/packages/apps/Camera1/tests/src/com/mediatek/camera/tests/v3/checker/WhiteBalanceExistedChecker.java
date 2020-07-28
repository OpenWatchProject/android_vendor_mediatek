package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class WhiteBalanceExistedChecker extends SettingItemExistedChecker {
    public WhiteBalanceExistedChecker(boolean isLaunchFromIntent) {
        super("White balance", isLaunchFromIntent);
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_EXISTED) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.white-balance");
        } else {
            return true;
        }
    }
}
