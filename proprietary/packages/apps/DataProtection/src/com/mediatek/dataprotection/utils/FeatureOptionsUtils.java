package com.mediatek.dataprotection.utils;

import android.os.SystemProperties;

public class FeatureOptionsUtils {
    /**
     * Get the option value of FeatureOption.MTK_DRM_APP.
     *
     * @return is MtkDrmApp, or not.
     */
    public static boolean isMtkDrmApp() {
      //  LogUtils.d(TAG, "FeatureOption.MTK_DRM_APP: " + FeatureOption.MTK_DRM_APP);
        //return FeatureOption.MTK_DRM_APP;
        return SystemProperties.getBoolean("ro.vendor.mtk_oma_drm_support", false);
    }
}
