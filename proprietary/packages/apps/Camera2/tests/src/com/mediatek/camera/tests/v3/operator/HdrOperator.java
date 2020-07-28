package com.mediatek.camera.tests.v3.operator;

import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.text.TextUtils;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

public class HdrOperator extends QuickSwitchOptionsOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(HdrOperator.class.getSimpleName());
    public static final int INDEX_AUTO = 0;
    public static final int INDEX_ON = 1;
    public static final int INDEX_OFF = 2;
    /**
     * Index of back camera.
     */
    public static final int INDEX_BACK = 0;
    /**
     * Index of front camera.
     */
    public static final int INDEX_FRONT = 1;

    private boolean mIsVHDR = false;
    private Camera.Parameters mParameters = TestContext.mBackCameraParameters;
    private CameraCharacteristics mCharacteristics = TestContext.mBackCameraCharacteristics;

    private static final String KEY_HDR_VIDEO_VALUES = "video-hdr-values";
    private static final String HDR_KEY_AVAILABLE_HDR_MODES_PHOTO =
            "com.mediatek.hdrfeature.availableHdrModesPhoto";
    private static final String HDR_KEY_AVAILABLE_HDR_MODES_VIDEO =
            "com.mediatek.hdrfeature.availableHdrModesVideo";
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_OFF = new int[]{0};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_ON = new int[]{1};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_AUTO = new int[]{2};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON = new int[]{3};
    private static final int[] CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO = new int[]{4};

    private static final String SWITCH_ICON_RESOURCE = "com.mediatek.camera:id/hdr_icon";
    private static final String[] OPTION_RESOURCES = {
            "com.mediatek.camera:id/hdr_auto",
            "com.mediatek.camera:id/hdr_on",
            "com.mediatek.camera:id/hdr_off"};

    /**
     * Initialize hdr options.
     *
     * @param isVHDR True if init vHdr options,false if init cHdr options.
     * @return The hdr operator.
     */
    public HdrOperator initHdrOptions(boolean isVHDR) {
        mIsVHDR = isVHDR;
        return this;
    }

    @Override
    protected int getOptionsCount() {
        return 3;
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
                return "HDR auto";
            case INDEX_ON:
                return "HDR on";
            case INDEX_OFF:
                return "HDR off";
        }
        return null;
    }

    @Override
    public boolean isSupported(int index) {
        return isHdrSupported(index);
    }

    @Override
    public String getDescription(int index) {
        String currentMode = getCurrentMode();
        switch (index) {
            case INDEX_AUTO:
                return "Switch hdr as auto in " + currentMode;
            case INDEX_OFF:
                return "Switch hdr as off in " + currentMode;
            case INDEX_ON:
                return "Switch hdr as on in " + currentMode;
            default:
                return "";
        }
    }

    private boolean isHdrSupported(int index) {
        boolean isSupported = false;
        String cameraInfo = getCameraInfo();
        String option = getOptionFromIndex(index);
        String currentMode = getCurrentMode();
        switch (CameraApiHelper.getCameraApiType(null)) {
            case API1:
                initParameters();
                if (mParameters == null) {
                    LogHelper.e(TAG, "mParameters is null");
                    return false;
                }
                isSupported = isSupportedInParameters(index);
                LogHelper.d(TAG, option + " isSupported " + isSupported + " in " + currentMode +
                        " in API1 " + cameraInfo);
                return isSupported;
            case API2:
                initCharacteristics();
                if (mCharacteristics == null) {
                    LogHelper.e(TAG, "mCharacteristics is null");
                    return false;
                }
                isSupported = isSupportedInCharacteristics(index);
                LogHelper.d(TAG, option + " isSupported " + isSupported + " in " + currentMode +
                        " in API2 " + cameraInfo);
                return isSupported;
            default:
                LogHelper.d(TAG, option + " isSupported " + isSupported + " in " + currentMode);
                return false;
        }
    }

    private void initParameters() {
        if (TestContext.mLatestCameraFacing == INDEX_BACK) {
            mParameters = TestContext.mBackCameraParameters;
        } else if (TestContext.mLatestCameraFacing == INDEX_FRONT) {
            mParameters = TestContext.mFrontCameraParameters;
        }
    }

    private void initCharacteristics() {
        if (TestContext.mLatestCameraFacing == INDEX_BACK) {
            mCharacteristics = TestContext.mBackCameraCharacteristics;
        } else if (TestContext.mLatestCameraFacing == INDEX_FRONT) {
            mCharacteristics = TestContext.mFrontCameraCharacteristics;
        }
    }

    /**
     * Check hdr capability in API1.
     *
     * @param index The index of current hdr value.
     * @return Whether the hdr option is supported in current platform.
     */
    private boolean isSupportedInParameters(int index) {
        if (mIsVHDR) {
            return isVHDRSupportedInParameters(index);
        } else {
            return isCHDRSupportedInParameters(index);
        }
    }

    private boolean isCHDRSupportedInParameters(int index) {
        switch (index) {
            case INDEX_AUTO:
                return isHdrDetectionSupportedInParameters();
            case INDEX_OFF:
            case INDEX_ON:
                return isHdrSceneModeSupportInParameters();
            default:
                return false;
        }
    }

    private boolean isVHDRSupportedInParameters(int index) {
        switch (index) {
            case INDEX_AUTO:
                return isHdrDetectionSupportedInParameters()
                        && isVideoHdrSupportedInParameters()
                        && isSingleFrameHDRSupported();
            case INDEX_OFF:
            case INDEX_ON:
                return isVideoHdrSupportedInParameters();
            default:
                return false;
        }
    }

    /**
     * Check hdr capability in API2.
     *
     * @param index The index of current hdr value.
     * @return Whether the hdr option is supported in current platform.
     */
    private boolean isSupportedInCharacteristics(int index) {
        if (mIsVHDR) {
            return isVHDRSupportedInCharacteristics(index);
        } else {
            return isCHDRSupportedInCharacteristics(index);
        }
    }

    private boolean isCHDRSupportedInCharacteristics(int index) {
        switch (index) {
            case INDEX_AUTO:
                return isHdrDetectionSupportInCharacteristics();
            case INDEX_OFF:
            case INDEX_ON:
                return isHdrSceneModeSupportInCharacteristics();
            default:
                return false;
        }
    }

    private boolean isVHDRSupportedInCharacteristics(int index) {
        switch (index) {
            case INDEX_AUTO:
                return isVideoHdrDetectionSupportInCharacteristics();
            case INDEX_OFF:
            case INDEX_ON:
                return isVideoHdrSupportInCharacteristics();
            default:
                return false;
        }
    }

    private boolean isHdrSceneModeSupportInParameters() {
        boolean enable = false;
        List<String> supportedList = mParameters.getSupportedSceneModes();
        if (supportedList != null && supportedList.indexOf(Camera.Parameters.SCENE_MODE_HDR) > 0) {
            enable = true;
        }
        return enable;
    }

    private boolean isHdrDetectionSupportedInParameters() {
        boolean enable = "true".equals(mParameters.get("hdr-detection-supported"));
        return enable;
    }

    private boolean isVideoHdrSupportedInParameters() {
        boolean enable = getParametersSupportedValues(
                mParameters, KEY_HDR_VIDEO_VALUES).size() > 1;
        return enable;
    }

    private boolean isSingleFrameHDRSupported() {
        boolean enable = "true".equals(mParameters.get("single-frame-cap-hdr-supported"));
        return enable;
    }

    private boolean isHdrSceneModeSupportInCharacteristics() {
        boolean enable = false;
        int[] supportSceneList = mCharacteristics.get(
                CameraCharacteristics.CONTROL_AVAILABLE_SCENE_MODES);
        for (int value : supportSceneList) {
            if (value == CameraMetadata.CONTROL_SCENE_MODE_HDR) {
                enable = true;
                break;
            }
        }
        return enable;
    }

    private boolean isHdrDetectionSupportInCharacteristics() {
        boolean enable = false;
        int[] supportVendorList = null;
        CameraCharacteristics.Key<int[]> keyHdrAvailablePhotoModes = null;
        List<CameraCharacteristics.Key<?>> keyList = mCharacteristics.getKeys();
        for (CameraCharacteristics.Key<?> key : keyList) {
            if (key.getName().equals(HDR_KEY_AVAILABLE_HDR_MODES_PHOTO)) {
                keyHdrAvailablePhotoModes = (CameraCharacteristics.Key<int[]>) key;
            }
        }
        if (keyHdrAvailablePhotoModes != null) {
            supportVendorList = mCharacteristics.get(keyHdrAvailablePhotoModes);
        }
        if (supportVendorList != null) {
            for (int value : supportVendorList) {
                if (value == CAM_HDR_FEATURE_HDR_MODE_AUTO[0]) {
                    enable = true;
                }
            }
        }
        return enable;
    }

    private boolean isVideoHdrSupportInCharacteristics() {
        boolean enable = false;
        int[] supportVendorList = getSupportedVendorList();
        if (supportVendorList != null) {
            for (int value : supportVendorList) {
                if (value == CAM_HDR_FEATURE_HDR_MODE_VIDEO_ON[0]) {
                    enable = true;
                }
            }
        }
        return enable;
    }

    private boolean isVideoHdrDetectionSupportInCharacteristics() {
        boolean enable = false;
        int[] supportVendorList = getSupportedVendorList();
        if (supportVendorList != null) {
            for (int value : supportVendorList) {
                if (value == CAM_HDR_FEATURE_HDR_MODE_VIDEO_AUTO[0]) {
                    enable = true;
                }
            }
        }
        return enable;
    }


    private int[] getSupportedVendorList() {
        int[] supportVendorList = null;
        CameraCharacteristics.Key<int[]> keyHdrAvailableVideoModes = null;
        List<CameraCharacteristics.Key<?>> keyList = mCharacteristics.getKeys();
        for (CameraCharacteristics.Key<?> key : keyList) {
            if (key.getName().equals(HDR_KEY_AVAILABLE_HDR_MODES_VIDEO)) {
                keyHdrAvailableVideoModes = (CameraCharacteristics.Key<int[]>) key;
            }
        }
        if (keyHdrAvailableVideoModes != null) {
            supportVendorList = mCharacteristics.get(keyHdrAvailableVideoModes);
        }
        return supportVendorList;
    }

    private List<String> getParametersSupportedValues(Camera.Parameters parameters, String key) {
        List<String> supportedList = new ArrayList<>();
        if (parameters != null) {
            String str = parameters.get(key);
            supportedList = split(str);
        }
        return supportedList;
    }

    private ArrayList<String> split(String str) {
        ArrayList<String> substrings = new ArrayList<>();
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            for (String s : splitter) {
                substrings.add(s);
            }
        }
        return substrings;
    }

    private String getCurrentMode() {
        if (mIsVHDR) {
            return "video mode";
        } else {
            return "photo mode";
        }
    }

    private String getOptionFromIndex(int index) {
        switch (index) {
            case INDEX_AUTO:
                return "hdr auto";
            case INDEX_ON:
                return "hdr on";
            case INDEX_OFF:
                return "hdr off";
            default:
                return "";
        }
    }

    private String getCameraInfo() {
        if (TestContext.mLatestCameraFacing == INDEX_BACK) {
            return "in back camera";
        } else if (TestContext.mLatestCameraFacing == INDEX_FRONT) {
            return "in front camera";
        }
        return null;
    }

}
