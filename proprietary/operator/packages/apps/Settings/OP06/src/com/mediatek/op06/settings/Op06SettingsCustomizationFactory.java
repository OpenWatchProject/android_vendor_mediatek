package com.mediatek.op06.settings;

import android.content.Context;

import com.mediatek.settings.ext.OpSettingsCustomizationFactoryBase;

public class Op06SettingsCustomizationFactory extends OpSettingsCustomizationFactoryBase {
    private Context mContext;

    public Op06SettingsCustomizationFactory(Context context) {
        super(context);
        mContext = context;
    }
}
