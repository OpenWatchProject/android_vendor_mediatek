package com.mediatek.settingslib.ext;

import com.mediatek.settingslib.ext.IWifiLibExt;

/**
 * Default plugin implementation.
 */
public class DefaultWifiLibExt implements IWifiLibExt {

    @Override
    public void appendApSummary(StringBuilder summary, int autoJoinStatus,
        String connectFail, String disabled) {
        summary.append(connectFail);
    }
}
