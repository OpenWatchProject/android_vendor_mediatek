package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.operator.HdrOperator;

@CoverPoint(pointList = {"Check UI status of HDR quick switcher"})
public class HdrQuickSwitchChecker extends QuickSwitchChecker {
    public static final int INDEX_AUTO = 0;
    public static final int INDEX_ON = 1;
    public static final int INDEX_OFF = 2;

    @Override
    protected String getSwitchIconResourceId() {
        return "com.mediatek.camera:id/hdr_icon";
    }

    @Override
    protected String getSwitchIconDescription(int index) {
        switch (index) {
            case INDEX_AUTO:
                return "HDR auto";
            case INDEX_ON:
                return "HDR on";
            case INDEX_OFF:
                return "HDR off";
        }
        return null;
    }

    @Override
    protected int getSwitchIconStatusCount() {
        return 3;
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_AUTO) {
            return new HdrOperator().isSupported(HdrOperator.INDEX_AUTO);
        } else if (index == INDEX_OFF) {
            return new HdrOperator().isSupported(HdrOperator.INDEX_OFF);
        } else if (index == INDEX_ON) {
            return new HdrOperator().isSupported(HdrOperator.INDEX_ON);
        } else {
            return false;
        }
    }
}
