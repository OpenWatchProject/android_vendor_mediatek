package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class EisLogObserver extends LogPrintObserver {
    public static final int INDEX_ON = 0;
    public static final int INDEX_OFF = 1;
    private CameraDeviceManagerFactory.CameraApi mCameraApi = null;

    private static final String[] TAGS_API1 = new String[]{
            "CamAp_EISParametersCon",
            "CamAp_EISParametersCon"
    };

    private static final String[] TAGS_API2 = new String[]{
            "CamAp_EISCaptureReques",
            "CamAp_EISCaptureReques"
    };

    private static final String[] KEYS = new String[]{
            "eis = on",
            "eis = off"
    };

    @Override
    protected String getObserveLogTag(int index) {
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
                return TAGS_API1[index];
            case API2:
                return TAGS_API2[index];
            default:
                return null;
        }
    }

    @Override
    protected String getObserveLogKey(int index) {
        return KEYS[index];
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.eis");
    }

    @Override
    public int getObserveCount() {
        return 2;
    }
}