package com.mediatek.ims.config;

import android.content.Context;
import android.os.Handler;


/**
 * Utility class providing an adaption layer between ImsConfig and ContentProvider.
 * Application can register listener ArgsObserver for ImsConfig.ConfigConstants.
 */
public final class ConfigRegister extends Register {
    public ConfigRegister(Context context, int phoneId) {
        this(context, phoneId, null);
    };

    public ConfigRegister(Context context, int phoneId, Handler handler) {
        super(context, phoneId, handler);
        argType = "config";
    };

    @Override
    public Register addArg(int argId) {
        if (ImsConfigContract.Validator.isValidConfigId(argId)) {
            super.addArg(argId);
        } else {
            throw new IllegalArgumentException("Invalid config id " + argId + " to register");
        }
        return this;
    }

    @Override
    public void register(IArgsChangeListener listener) {
        super.register(listener);
        for (Integer argId : mRegArgs) {
            mContext.getContentResolver().registerContentObserver(
                    ImsConfigContract.Master.getUriWithConfigId(mPhoneId, argId),
                    false,
                    mArgsObserver);
        }
    }

    @Override
    public void unregister() {
        super.unregister();
    }
}
