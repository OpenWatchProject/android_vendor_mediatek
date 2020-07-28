package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.operator.HdrOperator;

public class HdrQuickSwitchExistedChecker extends QuickSwitchExistedChecker {
    @Override
    protected String getSwitchIconResourceId() {
        return "com.mediatek.camera:id/hdr_icon";
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_EXIST) {
            return new HdrOperator().isSupported(HdrOperator.INDEX_AUTO)
                    || new HdrOperator().isSupported(HdrOperator.INDEX_ON);
        } else {
            return true;
        }
    }
}
