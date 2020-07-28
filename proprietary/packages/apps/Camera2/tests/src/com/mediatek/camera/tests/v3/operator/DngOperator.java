package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.util.Utils;

public class DngOperator extends SettingSwitchButtonOperator {

    public DngOperator() {
        super("RAW(.DNG)");
    }

    @Override
    public boolean isSupported(int index) {
        switch (index) {
            case SettingSwitchButtonOperator.INDEX_SWITCH_ON:
                return Utils.isFeatureSupported("com.mediatek.camera.at.dng.on");
            case SettingSwitchButtonOperator.INDEX_SWITCH_OFF:
                return Utils.isFeatureSupported("com.mediatek.camera.at.dng.off");
        }
        return super.isSupported(index);
    }
}
