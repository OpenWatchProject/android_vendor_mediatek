package com.mediatek.apmonitor;

import android.os.SystemProperties;

public class ApmServiceUtils {
    static final String DMC_SUPPORT_PROP = "ro.vendor.mtk_dmc_support";
    static final String APM_ACTIVE_PROP = "vendor.dmc.apm.active";

    static public boolean isDmcSupport() {
        return (SystemProperties.getInt(DMC_SUPPORT_PROP, 0) == 1) ? true : false;
    }

    static public boolean isApmActive() {
        return (SystemProperties.getInt(APM_ACTIVE_PROP, 0) == 1) ? true : false;
    }
}