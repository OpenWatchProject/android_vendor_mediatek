package com.mediatek.systemui.ext;


/**
 * Default implementation of Plug-in definition of IMobileIconExt.
 */
public class DefaultMobileIconExt implements IMobileIconExt {
    @Override
    public int customizeWifiNetCondition(int netCondition) {
        return netCondition;
    }
    @Override
    public int customizeMobileNetCondition(int netCondition) {
        return netCondition;
    }
}