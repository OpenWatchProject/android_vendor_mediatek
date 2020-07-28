package com.mediatek.ims.config.op;

import com.android.ims.ImsConfig;
import com.mediatek.ims.config.ImsConfigPolicy;
import com.mediatek.ims.MtkImsConstants;

public class Op236ConfigPolicy extends ImsConfigPolicy {
    public Op236ConfigPolicy() {
        super("Op236ConfigPolicy");
    }

    public boolean onSetDefaultValue(int configId, ImsConfigPolicy.DefaultConfig config) {
        boolean set = true;
        switch (configId) {
            // The mobile device must set the Voice_Domain_Preference_E_UTRAN parameter as to
            // CS Voice Only by default.
            case MtkImsConstants.ConfigConstants.VOICE_DOMAIN_PREFERENCE:
                config.defVal = "1";
                break;
            default:
                set = false;
                break;
        }
        return set;
    }
}
