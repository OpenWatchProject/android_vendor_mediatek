
package com.android.mms.util;

import android.os.SystemProperties;

public final class FeatureOption {

    /**
     * check if MTK_WAPPUSH_SUPPORT is turned on or not
     */
    public static final boolean MTK_WAPPUSH_SUPPORT = true;
//        SystemProperties.get("ro.vendor.mtk_wappush_support").equals("1");

    /**
     * check if MTK_DRM_APP is turned on or not
     */
    public static final boolean MTK_DRM_APP =
        SystemProperties.get("ro.vendor.mtk_oma_drm_support").equals("1");

    /**
     * C2K feature option
     */
    public static final boolean MTK_C2K_SUPPORT = SystemProperties.get("ro.vendor.mtk_ps1_rat")
            .contains("C"); //.equals("1");

    /**
     * VoLTE feature option
     */
    public static final boolean MTK_IMS_SUPPORT = SystemProperties.
            get("persist.vendor.ims_support").equals("1");
    public static final boolean MTK_VOLTE_SUPPORT = SystemProperties.
            get("persist.vendor.volte_support").equals("1");
    public static final boolean MTK_MWI_SUPPORT = MTK_IMS_SUPPORT && MTK_VOLTE_SUPPORT;

    public static final boolean MTK_LOW_RAM_SUPPORT = SystemProperties.get("ro.config.low_ram").equals("true");
}
