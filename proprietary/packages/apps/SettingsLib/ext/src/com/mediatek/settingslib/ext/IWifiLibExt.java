package com.mediatek.settingslib.ext;


/**
 *  Interface for customize operator request.
 */
public interface IWifiLibExt {

    /**
     * append reason to access point summary.
     * @param summary current summary
     * @param autoJoinStatus Access point's auto join status
     * @param connectFail the disabled fail string
     * @param disabled the generic fail string
     * @internal
     */
    public void appendApSummary(StringBuilder summary, int autoJoinStatus,
        String connectFail, String disabled);
}
