package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class SwitchToSlowMotionModeOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SwitchToSlowMotionModeOperator.class.getSimpleName());

    @Override
    protected void doOperate() {
        new SwitchToModeOperator("Slow motion").operate(0);
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Switch to slow motion mode";
    }

    @Override
    public boolean isSupported(int index) {
        if (Utils.isFeatureSupported("com.mediatek.camera.at.slow-motion", null) == null) {
            boolean enable =
                    SystemProperties.getInt("ro.vendor.mtk_slow_motion_support", 0) ==
                    1 ? true : false;
            LogHelper.d(TAG, "[isSupported] slow motion enable = " + enable);
            return enable;
        } else {
            return Utils.isFeatureSupported("com.mediatek.camera.at.slow-motion");
        }
    }
}
