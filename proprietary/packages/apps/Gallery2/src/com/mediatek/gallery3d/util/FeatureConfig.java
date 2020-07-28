package com.mediatek.gallery3d.util;


import android.util.Log;

import com.mediatek.galleryportable.SystemPropertyUtils;

/**
 * Get featureOption from FeatureConfig.
 */
public class FeatureConfig {
    private static final String TAG = "MtkGallery2/FeatureConfig";

    public static final boolean SUPPORT_FANCY_HOMEPAGE = true;

    public static final boolean SUPPORT_EMULATOR = SystemPropertyUtils.get("ro.kernel.qemu")
            .equals("1");

    public static final boolean IS_TABLET = SystemPropertyUtils.get("ro.build.characteristics")
            .equals("tablet");

    public static final boolean IS_GMO_RAM_OPTIMIZE =
            SystemPropertyUtils.get("ro.vendor.mtk_gmo_ram_optimize").equals("1");

    public static volatile boolean sIsLowRamDevice;

    static {
        Log.d(TAG, "SUPPORT_FANCY_HOMEPAGE = " + SUPPORT_FANCY_HOMEPAGE);
        Log.d(TAG, "SUPPORT_EMULATOR = " + SUPPORT_EMULATOR);
        Log.d(TAG, "IS_GMO_RAM_OPTIMIZE = " + IS_GMO_RAM_OPTIMIZE);
        Log.d(TAG, "sIsLowRamDevice = " + sIsLowRamDevice);
    }
}