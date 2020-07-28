package com.mediatek.ims.config.op;

import com.mediatek.ims.config.ImsConfigPolicy;


public class DefaultConfigPolicy extends ImsConfigPolicy {
    public DefaultConfigPolicy() {
        super("DefaultConfigPolicy");
    }

    public boolean onSetDefaultValue(int configId, ImsConfigPolicy.DefaultConfig config) {
        // No default values
        return false;
    }
}
