package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.util.Utils;

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
            return Utils.isFeatureSupported("com.mediatek.camera.at.hdr.auto");
        } else if (index == INDEX_OFF) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.hdr.off");
        } else if (index == INDEX_ON) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.hdr.on");
        } else {
            return false;
        }
    }
}
