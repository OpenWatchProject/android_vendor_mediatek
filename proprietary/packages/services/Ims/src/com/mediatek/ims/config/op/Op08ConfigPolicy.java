package com.mediatek.ims.config.op;

import com.android.ims.ImsConfig;
import com.mediatek.ims.config.ImsConfigPolicy;

public class Op08ConfigPolicy extends ImsConfigPolicy {
    public Op08ConfigPolicy() {
        super("Op08ConfigPolicy");
    }

    public boolean onSetDefaultValue(int configId, ImsConfigPolicy.DefaultConfig config) {
        boolean set = true;
        switch (configId) {
            case ImsConfig.ConfigConstants.VLT_SETTING_ENABLED:
                config.defVal = "0";
                break;
            default:
                set = false;
                break;
        }
        return set;
    }
}
