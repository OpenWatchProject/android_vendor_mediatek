package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Switch to vsdof photo mode.
 */
public class SwitchToStereoPhotoModeOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SwitchToStereoPhotoModeOperator.class
            .getSimpleName());

    @Override
    protected void doOperate() {
        new SwitchToModeOperator("Stereo").operate(0);
        new SwitchPhotoVideoOperator().operate(SwitchPhotoVideoOperator.INDEX_PHOTO);
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Switch to stereo photo mode";
    }

    @Override
    public boolean isSupported(int index) {
        if (Utils.isFeatureSupported("com.mediatek.camera.at.vsdof", null) == null) {
            switch (CameraApiHelper.getCameraApiType(null)) {
                case API1:
                    String stereoVsdofModeValues =
                            TestContext.mBackCameraParameters.get("stereo-vsdof-mode-values");
                    LogHelper.d(TAG, "[isSupported] stereo-vsdof-mode-values = "
                            + stereoVsdofModeValues);
                    boolean isSupport = !("off".equals(stereoVsdofModeValues)
                            || stereoVsdofModeValues == null);
                    LogHelper.d(TAG, "[isSupported] = "
                            + isSupport);
                    return isSupport;
                case API2:
                    return false;
                default:
                    return false;
            }
        } else {
            return Utils.isFeatureSupported("com.mediatek.camera.at.vsdof");
        }
    }
}
