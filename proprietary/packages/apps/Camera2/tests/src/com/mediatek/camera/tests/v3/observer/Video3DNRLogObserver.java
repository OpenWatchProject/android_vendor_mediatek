package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class Video3DNRLogObserver extends LogListPrintObserver {
    public static final int INDEX_ON = 0;
    public static final int INDEX_OFF = 1;
    private CameraDeviceManagerFactory.CameraApi mCameraApi = null;

    private static final String[][] TAGS_API1 = new String[][]{
            {"CamAp_NoiseReductionParametersConfig",
                    "isp_mgr_nr3d"
            },
            {"CamAp_NoiseReductionParametersConfig",
                    "mtkcam-FeatureSettingPolicy"
            }
    };

    private static final String[][] TAGS_API2 = new String[][]{
            {"CamAp_NoiseReductionCaptureRequestConfig",
                    "isp_mgr_nr3d"
            },
            {"CamAp_NoiseReductionCaptureRequestConfig",
                    "mtkcam-FeatureSettingPolicy"
            }
    };

    private static final String[][] KEY_API1 = new String[][]{
            {"[configParameters] value = on",
                    "NR3D_EN(1), SL2E_EN(1)"
            },
            {"[configParameters] value = off",
                    "[evaluateStreamConfiguration] 3DNR mode : 0"
            }
    };

    private static final String[][] KEY_API2 = new String[][]{
            {"[configCaptureRequest] current nose reduction value = on",
                    "NR3D_EN(1), SL2E_EN(1)"
            },
            {"[configCaptureRequest] current nose reduction value = off",
                    "[evaluateStreamConfiguration] 3DNR mode : 0"
            }
    };

    public Video3DNRLogObserver setCameraApi(CameraDeviceManagerFactory.CameraApi api) {
        mCameraApi = api;
        return this;
    }

    @Override
    protected String[] getObservedTagList(int index) {
        switch (mCameraApi == null ? CameraApiHelper.getCameraApiType(null) : mCameraApi) {
            case API1:
                return TAGS_API1[index];
            case API2:
                return TAGS_API2[index];
            default:
                return null;
        }
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        switch (mCameraApi == null ? CameraApiHelper.getCameraApiType(null) : mCameraApi) {
            case API1:
                return KEY_API1[index];
            case API2:
                return KEY_API2[index];
            default:
                return null;
        }
    }

    @Override
    protected void applyAdbCommandBeforeStarted() {
        setAdbCommand("camera.p2tpipedump.enable", "1");
    }

    @Override
    protected void clearAdbCommandAfterStopped() {
        setAdbCommand("camera.p2tpipedump.enable", "0");
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.3dnr");
    }

    @Override
    public int getObserveCount() {
        return 2;
    }
}
