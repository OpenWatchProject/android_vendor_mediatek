package com.mediatek.camera.tests.v3.operator;


import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Utils;

public class WhiteBalanceOperator extends SettingRadioButtonOperator {
    public static final int INDEX_AUTO = 0;
    public static final int INDEX_INCANDESCENT = 1;
    public static final int INDEX_DAYLIGHT = 2;
    public static final int INDEX_FLUORESCENT = 3;
    public static final int INDEX_CLOUDY = 4;
    public static final int INDEX_TWILIGHT = 5;
    public static final int INDEX_SHADE = 6;
    public static final int INDEX_WARM_FLUORESCENT = 7;

    private static final String[] OPTIONS = new String[]{
            "Auto",
            "Incandescent",
            "Daylight",
            "Fluorescent",
            "Cloudy",
            "Twilight",
            "Shade",
            "Warm fluorescent"
    };

    @Override
    public String getDescription(int index) {
        TestContext.mLatestWhiteBalanceSettingValue = OPTIONS[index];
        return super.getDescription(index);
    }

    @Override
    protected int getSettingOptionsCount() {
        return OPTIONS.length;
    }

    @Override
    protected String getSettingTitle() {
        return "White balance";
    }

    @Override
    protected String getSettingOptionTitle(int index) {
        return OPTIONS[index];
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.white-balance");
    }
}
