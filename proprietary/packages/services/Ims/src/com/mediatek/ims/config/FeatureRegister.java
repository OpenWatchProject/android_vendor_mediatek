package com.mediatek.ims.config;

import android.content.Context;
import android.os.Handler;

public final class FeatureRegister extends Register {
    private int mNetworkType;

    public FeatureRegister(Context context, int phoneId, int networkType) {
        this(context, phoneId, networkType, null);
    };

    public FeatureRegister(Context context, int phoneId, int networkType, Handler handler) {
        super(context, phoneId, handler);
        argType = "feature";
        mNetworkType = networkType;
    };

    @Override
    public Register addArg(int argId) {
        if (ImsConfigContract.Validator.isValidFeatureId(argId)) {
            super.addArg(argId);
        } else {
            throw new IllegalArgumentException("Invalid feature id " + argId + " to register");
        }
        return this;
    }

    @Override
    public void register(IArgsChangeListener listener) {
        super.register(listener);
        for (Integer argId : mRegArgs) {
            mContext.getContentResolver().registerContentObserver(
                    ImsConfigContract.Feature.getUriWithFeatureId(mPhoneId, argId, mNetworkType),
                    false,
                    mArgsObserver);
        }
    }

    @Override
    public void unregister() {
        super.unregister();
    }
}
