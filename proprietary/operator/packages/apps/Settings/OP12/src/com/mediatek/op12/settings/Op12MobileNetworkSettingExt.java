package com.mediatek.op12.settings;

import android.content.Context;
import android.util.Log;

import com.mediatek.settings.ext.DefaultMobileNetworkSettingsExt;

public class Op12MobileNetworkSettingExt extends DefaultMobileNetworkSettingsExt {

    public Context mContext;
    public static final String TAG = "Op12MobileNetworkSettingExt";

    public Op12MobileNetworkSettingExt(Context context) {
        mContext = context;
    }

    @Override
    public boolean customizeVWZNetworkModePreference(int subId) {
        Log.i(TAG , "customizeVWZNetworkModePreference return true");
        return true;
    }

    @Override
    public boolean customizeVersionFemtoCellSelectionPreference(int subId) {
        Log.i(TAG , "customizeVersionFemtoCellSelectionPreference return true");
        return true;
    }

    @Override
    public boolean customizeVolteState(int subId) {
        Log.i(TAG , "customizeVolteState return true");
        return true;
    }

}