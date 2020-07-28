package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class FlashInVideoLogObserver extends LogListPrintObserver {
    public static final int INDEX_AUTO = 0;
    public static final int INDEX_ON = 1;
    public static final int INDEX_OFF = 2;

    private static final String[] TAG_API1_AUTO = new String[]{
            "CamAp_FlashParameterCo"
    };

    private static final String[] KEY_API1_AUTO = new String[]{
            "[configParameters], value = auto"
    };


    private static final String[] TAG_API1_ON = new String[]{
            "CamAp_FlashParameterCo",
            "flashlight_drv.cpp"
    };

    private static final String[] KEY_API1_ON = new String[]{
            "[configParameters], value = on",
            "setOnOff() isOn = 1"
    };

    private static final String[] TAG_API1_OFF = new String[]{
            "CamAp_FlashParameterCo",
            "flashlight_drv.cpp"
    };

    private static final String[] KEY_API1_OFF = new String[]{
            "[configParameters], value = off",
            "setOnOff() isOn = 0"
    };

    private static final String[] TAG_API2_AUTO = new String[]{
            "CamAp_FlashRequestConf"
    };

    private static final String[] KEY_API2_AUTO = new String[]{
            "[configParameters], value = auto"
    };


    private static final String[] TAG_API2_ON = new String[]{
            "CamAp_FlashRequestConf",
            "flashlight_drv.cpp"
    };

    private static final String[] KEY_API2_ON = new String[]{
            "[configParameters], value = on",
            "setOnOff() isOn = 1"
    };

    private static final String[] TAG_API2_OFF = new String[]{
            "CamAp_FlashRequestConf",
            "flashlight_drv.cpp"
    };

    private static final String[] KEY_API2_OFF = new String[]{
            "[configParameters], value = off",
            "setOnOff() isOn = 0"
    };

    private static final String[][] TAG_API1_LIST = new String[][]{
            TAG_API1_AUTO,
            TAG_API1_ON,
            TAG_API1_OFF
    };

    private static final String[][] KEY_API1_LIST = new String[][]{
            KEY_API1_AUTO,
            KEY_API1_ON,
            KEY_API1_OFF
    };

    private static final String[][] TAG_API2_LIST = new String[][]{
            TAG_API2_AUTO,
            TAG_API2_ON,
            TAG_API2_OFF
    };

    private static final String[][] KEY_API2_LIST = new String[][]{
            KEY_API2_AUTO,
            KEY_API2_ON,
            KEY_API2_OFF
    };

    @Override
    protected String[] getObservedTagList(int index) {
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
                return TAG_API1_LIST[index];
            case API2:
                return TAG_API2_LIST[index];
            default:
                return null;
        }
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
                return KEY_API1_LIST[index];
            case API2:
                return KEY_API2_LIST[index];
            default:
                return null;
        }
    }

    @Override
    public int getObserveCount() {
        return 3;
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_AUTO) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.flash.auto");
        } else if (index == INDEX_OFF) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.flash.off");
        } else if (index == INDEX_ON) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.flash.on");
        } else {
            return false;
        }
    }
}
