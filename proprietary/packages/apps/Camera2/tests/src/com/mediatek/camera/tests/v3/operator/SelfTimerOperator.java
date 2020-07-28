package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.Utils;

public class SelfTimerOperator extends SettingRadioButtonOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SelfTimerOperator.class.getSimpleName());

    public static final int INDEX_OFF = 0;
    public static final int INDEX_2_SECONDS = 1;
    public static final int INDEX_10_SECONDS = 2;

    private static final String TITLE = "Self timer";
    private static final String[] OPTIONS = {"Off", "2 seconds", "10 seconds"};

    @Override
    protected int getSettingOptionsCount() {
        return OPTIONS.length;
    }

    @Override
    protected String getSettingTitle() {
        return TITLE;
    }

    @Override
    protected String getSettingOptionTitle(int index) {
        return OPTIONS[index];
    }
}
