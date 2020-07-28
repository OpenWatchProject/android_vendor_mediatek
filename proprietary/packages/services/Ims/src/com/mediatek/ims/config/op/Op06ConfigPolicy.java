package com.mediatek.ims.config.op;

import com.android.ims.ImsConfig;

import com.mediatek.ims.config.ImsConfigPolicy;
import com.mediatek.ims.MtkImsConstants;

public class Op06ConfigPolicy extends ImsConfigPolicy {
    public Op06ConfigPolicy() {
        super("Op06ConfigPolicy");
    }

    public boolean onSetDefaultValue(int configId, ImsConfigPolicy.DefaultConfig config) {
        boolean set = true;
        switch (configId) {
            case ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED:
                config.defVal = "0";
                break;
            case MtkImsConstants.ConfigConstants.EPDG_ADDRESS:
                config.defVal = "";
                break;
            default:
                set = false;
                break;
        }
        return set;
    }
}
