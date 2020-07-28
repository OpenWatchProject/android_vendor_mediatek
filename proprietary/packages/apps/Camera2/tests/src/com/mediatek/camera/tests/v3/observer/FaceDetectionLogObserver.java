package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Utils;

public class FaceDetectionLogObserver extends LogListPrintObserver {
    public static final int INDEX_DISABLE_ENABLE = 0;
    public static final int INDEX_DISABLE = 1;
    public static final int INDEX_ENABLE = 2;

    private static final String[][] TAG = new String[][]{
            {
                    "CamAp_FaceDetection",
                    "CamAp_FaceDetection"
            },
            {
                    "CamAp_FaceDetection",
            },
            {
                    "CamAp_FaceDetection",
            },
    };

    private static final String[][] KEY = new String[][]{
            {
                    "[overrideValues] curValue = off",
                    "[overrideValues] curValue = on",
            },
            {
                    "[overrideValues] curValue = off",
            },
            {
                    "[overrideValues] curValue = on",
            },
    };

    @Override
    protected String[] getObservedTagList(int index) {
        return TAG[index];
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return KEY[index];
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.face-detection");
    }

    @Override
    public int getObserveCount() {
        return 3;
    }
}
