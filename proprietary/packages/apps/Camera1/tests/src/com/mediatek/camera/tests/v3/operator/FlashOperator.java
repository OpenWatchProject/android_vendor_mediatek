package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.util.Utils;

public class FlashOperator extends QuickSwitchOptionsOperator {
    public static final int INDEX_AUTO = 0;
    public static final int INDEX_ON = 1;
    public static final int INDEX_OFF = 2;

    private static final String SWITCH_ICON_RESOURCE = "com.mediatek.camera:id/flash_icon";
    private static final String[] OPTION_RESOURCES = {
            "com.mediatek.camera:id/flash_auto",
            "com.mediatek.camera:id/flash_on",
            "com.mediatek.camera:id/flash_off"};
    private static final String[] OPTIONS_TAG = {
            "com.mediatek.camera.at.flash.auto",
            "com.mediatek.camera.at.flash.on",
            "com.mediatek.camera.at.flash.off",
    };

    @Override
    protected int getOptionsCount() {
        return OPTION_RESOURCES.length;
    }

    @Override
    protected String getSwitchIconResourceId() {
        return SWITCH_ICON_RESOURCE;
    }

    @Override
    protected String getOptionsResourceId(int index) {
        return OPTION_RESOURCES[index];
    }

    @Override
    protected String getSwitchIconDescription(int index) {
        switch (index) {
            case INDEX_AUTO:
                return "flash auto";
            case INDEX_OFF:
                return "flash off";
            case INDEX_ON:
                return "flash on";
            default:
                break;
        }
        return null;
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported(OPTIONS_TAG[index]);
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_AUTO:
                return "Switch flash as auto";
            case INDEX_OFF:
                return "Switch flash as off";
            case INDEX_ON:
                return "Switch flash as on";
            default:
                break;
        }
        return null;
    }
}
