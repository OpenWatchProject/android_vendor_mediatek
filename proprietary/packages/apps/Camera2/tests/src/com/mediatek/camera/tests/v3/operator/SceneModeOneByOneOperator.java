package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.Utils;

public class SceneModeOneByOneOperator extends SettingRadioOptionsOneByOneOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SceneModeOneByOneOperator.class
            .getSimpleName());
    private static final String TITLE = "Scene mode";

    public SceneModeOneByOneOperator() {
        super(TITLE, false);
    }

    @Override
    protected void doOperate(int index) {
        super.doOperate(index);
    }

    @Override
    public String getDescription(int index) {
        return super.getDescription(index);
    }
}
