package com.mediatek.op12.settings;

import android.content.Context;
import android.util.Log;

import com.mediatek.settings.ext.IMobileNetworkSettingsExt;
import com.mediatek.settings.ext.IWfcSettingsExt;
import com.mediatek.settings.ext.OpSettingsCustomizationFactoryBase;

public class Op12SettingsCustomizationFactory extends OpSettingsCustomizationFactoryBase {
    private static final String TAG = "Op12SettingsCustomizationFactory";
    private Context mContext;

    public Op12SettingsCustomizationFactory(Context context) {
        super(context);
        mContext = context;
    }

    public IWfcSettingsExt makeWfcSettingsExt() {
        return new Op12WfcSettingsExt(mContext);
    }

    @Override
    public IMobileNetworkSettingsExt makeMobileNetworkSettingsExt(Context context) {
        Log.d(TAG, "IMobileNetworkSettingsExt, context=" + mContext);
        return new Op12MobileNetworkSettingExt(mContext);
    }
}
