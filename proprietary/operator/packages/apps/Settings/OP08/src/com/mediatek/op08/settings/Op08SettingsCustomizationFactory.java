package com.mediatek.op08.settings;

import android.content.Context;
import android.util.Log;

import com.mediatek.settings.ext.IMobileNetworkSettingsExt;
import com.mediatek.settings.ext.IWfcSettingsExt;
import com.mediatek.settings.ext.OpSettingsCustomizationFactoryBase;

public class Op08SettingsCustomizationFactory extends OpSettingsCustomizationFactoryBase {
    private static final String TAG = "Op08SettingsCustomizationFactory";
    private Context mContext;

    public Op08SettingsCustomizationFactory(Context context) {
        super(context);
        mContext = context;
    }

    public IWfcSettingsExt makeWfcSettingsExt() {
        return new Op08WfcSettingsExt(mContext);
    }
    @Override
    public IMobileNetworkSettingsExt makeMobileNetworkSettingsExt(Context context) {
        Log.d(TAG, "IMobileNetworkSettingsExt, context=" + mContext);
        return new Op08MobileNetworkSettingExt(mContext);
    }
}