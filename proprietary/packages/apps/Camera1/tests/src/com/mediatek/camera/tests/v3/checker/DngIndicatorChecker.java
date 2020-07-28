package com.mediatek.camera.tests.v3.checker;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check dng_indicator icon is shown or hidden"})
public class DngIndicatorChecker extends IndicatorChecker {
    @Override
    protected String getIndicatorIconResourceId() {
        return "com.mediatek.camera:id/dng_indicator";
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_SHOW) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.dng.on");
        } else {
            return true;
        }
    }
}
