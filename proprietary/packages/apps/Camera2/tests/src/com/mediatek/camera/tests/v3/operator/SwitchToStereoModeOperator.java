package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Switch to stereo capture feature mode.
 */
public class SwitchToStereoModeOperator extends SwitchToModeOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SwitchToStereoModeOperator.class
            .getSimpleName());
    private static final String VSDOF_FEATURE = "stereo-vsdof-mode-values";
    private static final String STEREO_CAPTURE = "stereo-image-refocus-values";

    public SwitchToStereoModeOperator() {
        super("Stereo");
    }

    @Override
    public boolean isSupported(int index) {
        if (Utils.isFeatureSupported("com.mediatek.camera.at.stereo-capture", null) == null) {
            switch (CameraApiHelper.getCameraApiType(null)) {
                case API1:
                    String stereoVsdofModeValues =
                            TestContext.mBackCameraParameters.get(VSDOF_FEATURE);
                    String stereoCaptureModeValues
                            = TestContext.mBackCameraParameters.get(STEREO_CAPTURE);
                    LogHelper.d(TAG, "[isSupported] vsdof = "
                            + stereoVsdofModeValues + ", stereo capture = "
                            + stereoCaptureModeValues);
                    // Stereo capture support and vsdof not support.
                    boolean isVsdofNotSupport = "off".equals(stereoVsdofModeValues)
                            || stereoVsdofModeValues == null;
                    boolean isStereoCaptureSupport = !("off".equals(stereoVsdofModeValues)
                            || stereoVsdofModeValues == null);
                    return isStereoCaptureSupport && isVsdofNotSupport;
                case API2:
                    return false;
                default:
                    return false;
            }
        } else {
            return Utils.isFeatureSupported("com.mediatek.camera.at.stereo-capture");
        }
    }
}
