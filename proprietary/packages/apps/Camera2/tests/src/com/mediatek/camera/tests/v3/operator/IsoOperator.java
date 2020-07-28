package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Utils;

public class IsoOperator extends SettingRadioOptionsOneByOneOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(IsoOperator.class
            .getSimpleName());
    private static final String TITLE = "ISO";

    public IsoOperator() {
        super(TITLE, false);
    }

    @Override
    public int getOperatorCount() {
        if (Utils.isFeatureSupported("com.mediatek.camera.at.iso")) {
            return super.getOperatorCount();
        } else {
            return 0;
        }
    }

    @Override
    protected void doOperate(int index) {
        super.doOperate(index);
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.iso");
    }

    @Override
    public String getDescription(int index) {
        if (isSupported(index)) {
            TestContext.mLatestIsoSettingValue = getSettingOptionTitle(index);
            return super.getDescription(index);
        } else {
            TestContext.mLatestIsoSettingValue = "";
            return "Set iso value, not supported on current device";
        }
    }
}
