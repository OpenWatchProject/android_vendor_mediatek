package com.mediatek.emailcommon;

import android.os.SystemProperties;
import android.text.TextUtils;

import com.android.emailcommon.Logging;

/**
 * M: Email Feature Options
 */
public class EmailFeatureOptions {
    /*
     * [LBE] Large Body Enhancement:
     * 1. support write body file from EAS process, avoid binder size limitation.
     * 2. truncated body for Reply/Forward mail, avoid Parcel size limitation.
     */
    public static boolean LAEGE_BODY_ENHANCEMENT = true;
    static {
        String product = SystemProperties.get("ro.build.product");
        if (!TextUtils.isEmpty(product)
                // Disable in k35v1 product.
                && product.contains("k35v1")) {
            LAEGE_BODY_ENHANCEMENT = false;
        }
        Logging.d("LBE LAEGE_BODY_ENHANCEMENT feature: " + LAEGE_BODY_ENHANCEMENT);
    }
}
