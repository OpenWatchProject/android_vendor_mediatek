package com.mediatek.systemui.ext;

/**
 * M: the interface for Plug-in definition of customize mobile icons.
 */
public interface IMobileIconExt {

    /**
     * Customized WIFI net condition, so even net condition is not connected,
     * can be changed here for operator.
     * @param netCondition original net condition
     * @return 1 for connected, 0 for not
     * @internal
     */
    int customizeWifiNetCondition(int netCondition);
    /**
     * Customized Mobile net condition, so even net condition is not connected,
     * can be changed here for operator.
     * @param netCondition original net condition
     * @return 1 for connected, 0 for not
     * @internal
     */
    int customizeMobileNetCondition(int netCondition);

}
