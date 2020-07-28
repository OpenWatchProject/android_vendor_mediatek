package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.util.Utils;

public class WhiteBalanceSettingValueChecker extends SettingRadioOptionChecker {

    public WhiteBalanceSettingValueChecker(String whiteBalanceValue) {
        super("White balance", whiteBalanceValue);
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.white-balance");
    }
}
